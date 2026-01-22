#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "error/error.h"
#include "globals/globals.h"
#include "palm/dbug.h"
#include "palm/str.h"

#define ANSI_RESET "\033[0m"
#define ANSI_BRIGHT_RED "\033[30;91m"
#define ANSI_BRIGHT_YELLOW "\033[30;93m"
#define ANSI_BRIGHT_BLUE "\033[30;94m"
#define ANSI_BRIGHT_CYAN "\033[30;96m"

static char *color_of_level(level level) {
    switch (level) {
    case L_ERROR:
        return ANSI_BRIGHT_RED;
    case L_WARNING:
        return ANSI_BRIGHT_YELLOW;
    case L_INFO:
        return ANSI_BRIGHT_CYAN;
    default:
        DBUG_ASSERT(false, "Unknown level");
        return "";
    }
}

static char *message_of_level(level level) {
    switch (level) {
    case L_ERROR:
        return "error: ";
    case L_WARNING:
        return "warning: ";
    case L_INFO:
        return "info: ";
    default:
        DBUG_ASSERT(false, "Unknown level");
        return "";
    }
}

static int error_count = 0;

void abort_on_error(void) {
    if (error_count > 0) {
        emit_message(
            L_ERROR, "couldn't compile '%s' due to %d previous error%s\n",
            globals.input_file, error_count, error_count > 1 ? "s" : "");
        exit(EXIT_FAILURE);
    }
}

static void va_emit_message(level level, char *format, va_list ap) {
    if (level == L_ERROR) {
        error_count++;
    }

    fprintf(stderr, "%s%s" ANSI_RESET, color_of_level(level),
            message_of_level(level));
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
}

void emit_message(level level, char *format, ...) {
    va_list ap;

    va_start(ap, format);
    va_emit_message(level, format, ap);
    va_end(ap);
}

static void single_line_annotation(span span, level level, char *format,
                                   FILE *file, va_list ap) {
    int lineno_width = log10(span.el + 1) + 1;

    fprintf(stderr,
            ANSI_BRIGHT_BLUE "%*s-->" ANSI_RESET " %s:%d:%d\n" ANSI_BRIGHT_BLUE
                             "%*s|\n" ANSI_RESET,
            lineno_width, "", span.file, span.bl + 1, span.bc + 1,
            lineno_width + 1, "");

    bool prev_nl = true;
    int lineno = 0;
    char line[256];

    while (lineno <= span.el && fgets(line, sizeof(line), file)) {
        int len = STRlen(line);
        bool new_nl = line[len - 1] == '\n';

        if (lineno == span.bl) {
            if (prev_nl) {
                fprintf(stderr, ANSI_BRIGHT_BLUE "%*d |" ANSI_RESET " ",
                        lineno_width, lineno + 1);
            }
            fputs(line, stderr);
        }

        prev_nl = new_nl;
        lineno += prev_nl;

        if (prev_nl && lineno == span.bl) {
            break;
        }
    }

    if (!prev_nl) {
        fputc('\n', stderr);
    }

    char *color = color_of_level(level);
    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s|%*s%s", lineno_width + 1, "",
            span.bc + 1, "", color);
    for (int i = 0; i <= span.ec - span.bc; i++) {
        fputc('^', stderr);
    }

    fputc(' ', stderr);

    vfprintf(stderr, format, ap);

    fprintf(stderr, ANSI_BRIGHT_BLUE "\n%*s\n" ANSI_RESET, lineno_width + 2,
            "|");
}

static void multi_line_annotation(span span, level level, char *format,
                                  FILE *file, va_list ap) {
    int lineno_width = log10(span.el + 1) + 2;
    lineno_width = lineno_width < 3 ? 3 : lineno_width;

    fprintf(stderr,
            ANSI_BRIGHT_BLUE "%*s-->" ANSI_RESET " %s:%d:%d\n" ANSI_BRIGHT_BLUE
                             "%*s|\n",
            lineno_width, "", span.file, span.bl + 1, span.bc + 1,
            lineno_width + 1, "");

    bool prev_nl = true;
    int lineno = 0;
    char line[256];

    int bl_end = -1;

    char *color = color_of_level(level);
    while (lineno <= span.el && fgets(line, sizeof(line), file)) {
        int len = STRlen(line);
        bool new_nl = line[len - 1] == '\n';

        if (lineno == span.bl) {
            if (prev_nl) {
                fprintf(stderr, ANSI_BRIGHT_BLUE "%*d |" ANSI_RESET " ",
                        lineno_width, lineno + 1);
            }
            fputs(line, stderr);
            bl_end += len;
            if (new_nl) {
                fprintf(stderr, ANSI_BRIGHT_BLUE "%*s|%s┌", lineno_width + 1,
                        "", color);
                for (int i = 0; i < span.bc; i++) {
                    fputc('-', stderr);
                }
                for (int i = span.bc; i < bl_end; i++) {
                    fputc('^', stderr);
                }
                fputc('\n', stderr);
                if (span.el - span.bl >= 3) {
                    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s |%s|" ANSI_RESET "\n",
                            lineno_width, "...", color);
                }
            }
        } else if (lineno == span.el) {
            if (prev_nl) {
                fprintf(stderr, ANSI_BRIGHT_BLUE "%*d |%s|" ANSI_RESET " ",
                        lineno_width, lineno + 1, color);
            }
            fputs(line, stderr);
        } else if (lineno > span.bl && lineno < span.el &&
                   span.el - span.bl < 3) {
            if (prev_nl) {
                fprintf(stderr, ANSI_BRIGHT_BLUE "%*d |%s|" ANSI_RESET " ",
                        lineno_width, lineno + 1, color);
            }
            fputs(line, stderr);
        }

        prev_nl = new_nl;
        lineno += prev_nl;

        if (lineno == span.el && prev_nl) {
            break;
        }
    }

    if (!prev_nl) {
        fputc('\n', stderr);
    }

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s |%s└", lineno_width, "", color);
    for (int i = 0; i <= span.ec + 1; i++) {
        fputc('^', stderr);
    }
    fputc(' ', stderr);

    vfprintf(stderr, format, ap);

    fprintf(stderr, ANSI_BRIGHT_BLUE "\n%*s\n" ANSI_RESET, lineno_width + 2,
            "|");
}

void emit_message_with_span(span span, level level, char *format, ...) {
    if (!(level == L_ERROR || STReq(globals.input_file, span.file)) ||
        (globals.quiet && level == L_INFO)) {
        return;
    }

    FILE *file = NULL;

    if (!globals.quiet) {
        file = fopen(span.file, "r");
    }

    if (!file) {
        fprintf(stderr, "%s:%d:%d: ", span.file, span.bl + 1, span.bc + 1);
    }

    va_list ap;

    va_start(ap, format);
    va_emit_message(level, format, ap);
    va_end(ap);

    if (!file) {
        return;
    }

    va_start(ap, format);
    if (span.bl == span.el) {
        single_line_annotation(span, level, format, file, ap);
    } else {
        multi_line_annotation(span, level, format, file, ap);
    }
    va_end(ap);

    fclose(file);
}
