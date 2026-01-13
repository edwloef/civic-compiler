#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "error.h"
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
        exit(1);
    }
}

static void va_emit_message(level level, char *format, va_list ap) {
    if (level == L_ERROR)
        error_count++;

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

void emit_message_with_span(span span, level level, char *format, ...) {
    va_list ap;

    va_start(ap, format);
    va_emit_message(level, format, ap);
    va_end(ap);

    FILE *file = fopen(globals.input_file, "r");
    if (file == NULL)
        return;

    int lineno_width = log10(span.el + 1) + 1;

    fprintf(stderr,
            ANSI_BRIGHT_BLUE "%*s-->" ANSI_RESET " %s:%d:%d\n" ANSI_BRIGHT_BLUE
                             "%*s|\n",
            lineno_width, "", globals.input_file, span.bl + 1, span.bc + 1,
            lineno_width + 1, "");

    int lineno = 0;
    char line[256];

    while (lineno <= span.el && fgets(line, sizeof(line), file)) {
        if (lineno >= span.bl) {
            fprintf(stderr, "%*d |" ANSI_RESET " %s" ANSI_BRIGHT_BLUE,
                    lineno_width, lineno + 1, line);
        }

        if (line[STRlen(line) - 1] == '\n')
            lineno++;
    }

    fclose(file);

    fprintf(stderr, "%*s|", lineno_width + 1, "");

    char *color = color_of_level(level);
    if (span.bl == span.el) {
        fprintf(stderr, "%*s%s", span.bc + 1, "", color);
        for (int i = 0; i <= span.ec - span.bc; i++)
            fputc('^', stderr);
    } else {
        fprintf(stderr, "\n%*s|", lineno_width + 1, "");
    }

    fprintf(stderr, " %s", color);

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, ANSI_BRIGHT_BLUE "\n%*s\n" ANSI_RESET, lineno_width + 2,
            "|");
}
