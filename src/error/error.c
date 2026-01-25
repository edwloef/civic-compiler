#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include "error/error.h"
#include "globals/globals.h"
#include "palm/dbug.h"
#include "palm/str.h"

static bool color_supported(void) {
    static bool init = false;
    static bool support;
    if (!init) {
        init = true;
        support = !getenv("NO_COLOR") && isatty(fileno(stderr));
    }
    return support;
}

#define RESET (color_supported() ? "\033[0m" : "")
#define BRIGHT_RED (color_supported() ? "\033[30;91m" : "")
#define BRIGHT_YELLOW (color_supported() ? "\033[30;93m" : "")
#define BRIGHT_BLUE (color_supported() ? "\033[30;94m" : "")
#define BRIGHT_CYAN (color_supported() ? "\033[30;96m" : "")

static char *color_of_level(level level) {
    switch (level) {
    case L_ERROR:
        return BRIGHT_RED;
    case L_WARNING:
        return BRIGHT_YELLOW;
    case L_INFO:
        return BRIGHT_CYAN;
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
    error_count += level == L_ERROR;
    fprintf(stderr, "%s%s%s", color_of_level(level), message_of_level(level),
            RESET);
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
                                   va_list ap) {
    int lineno_width = log10(span.el + 1) + 1;
    char *color = color_of_level(level);

    htable_st *file = HTlookup(globals.linemap, span.file);
    char *line = HTlookup(file, &span.bl);

    fprintf(stderr, "%s%*s--> %s%s:%d:%d\n%s%*s|\n%*d | %s%s\n%s%*s|%*s%s",
            BRIGHT_BLUE, lineno_width, "", RESET, span.file, span.bl + 1,
            span.bc + 1, BRIGHT_BLUE, lineno_width + 1, "", lineno_width,
            span.bl + 1, RESET, line, BRIGHT_BLUE, lineno_width + 1, "",
            span.bc + 1, "", color);

    for (int i = 0; i <= span.ec - span.bc; i++) {
        fputc('^', stderr);
    }
    fputc(' ', stderr);

    vfprintf(stderr, format, ap);

    fprintf(stderr, "\n%s%*s|%s\n", BRIGHT_BLUE, lineno_width + 1, "", RESET);
}

static void multi_line_annotation(span span, level level, char *format,
                                  va_list ap) {
    int lineno_width = log10(span.el + 1) + 1;
    char *color = color_of_level(level);

    htable_st *file = HTlookup(globals.linemap, span.file);
    char *line = HTlookup(file, &span.bl);

    fprintf(stderr, "%s%*s--> %s%s:%d:%d\n%s%*s|\n%*d |   %s%s\n%s%*s|  %s",
            BRIGHT_BLUE, lineno_width, "", RESET, span.file, span.bl + 1,
            span.bc + 1, BRIGHT_BLUE, lineno_width + 1, "", lineno_width,
            span.bl + 1, RESET, line, BRIGHT_BLUE, lineno_width + 1, "", color);

    for (int i = 0; i <= span.bc; i++) {
        fputc('_', stderr);
    }
    fputs("^\n", stderr);

    if (span.el - span.bl < 3) {
        int lineno = span.bl + 1;
        line = HTlookup(file, &lineno);

        fprintf(stderr, "%s%*d | %s| %s%s\n", BRIGHT_BLUE, lineno_width,
                lineno + 1, color, RESET, line);
    } else {
        fprintf(stderr, "%s...%*s%s|\n", BRIGHT_BLUE, lineno_width, "", color);
    }

    line = HTlookup(file, &span.el);

    fprintf(stderr, "%s%*d | %s| %s%s\n%s%*s | %s|", BRIGHT_BLUE, lineno_width,
            span.el + 1, color, RESET, line, BRIGHT_BLUE, lineno_width, "",
            color);

    for (int i = 0; i <= span.ec; i++) {
        fputc('_', stderr);
    }
    fputs("^ ", stderr);

    vfprintf(stderr, format, ap);

    fprintf(stderr, "\n%s%*s|%s\n", BRIGHT_BLUE, lineno_width + 1, "", RESET);
}

void emit_message_with_span(span span, level level, char *format, ...) {
    if (level != L_ERROR && !STReq(globals.input_file, span.file)) {
        return;
    }

    if (globals.quiet || !globals.linemap) {
        fprintf(stderr, "%s:%d:%d: ", span.file, span.bl + 1, span.bc + 1);
    }

    va_list ap;

    va_start(ap, format);
    va_emit_message(level, format, ap);
    va_end(ap);

    if (!(globals.quiet || !globals.linemap)) {
        va_start(ap, format);
        if (span.bl == span.el) {
            single_line_annotation(span, level, format, ap);
        } else {
            multi_line_annotation(span, level, format, ap);
        }
        va_end(ap);
    }
}
