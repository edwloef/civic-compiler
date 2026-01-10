#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "analysis/error.h"
#include "globals/globals.h"
#include "palm/str.h"

#define ANSI_RESET "\033[0m"
#define ANSI_BRIGHT_RED "\033[30;91m"
#define ANSI_BRIGHT_BLUE "\033[30;94m"
#define ANSI_BRIGHT_CYAN "\033[30;96m"

static int error_count = 0;

void abort_on_error(void) {
    if (error_count > 0) {
        fprintf(stderr,
                ANSI_BRIGHT_RED
                "error: " ANSI_RESET
                "couldn't compile '%s' due to %d previous error%s\n",
                globals.input_file, error_count, error_count > 1 ? "s" : "");
        exit(1);
    }
}

static void va_emit_message(bool error, char *format, va_list ap) {
    if (error) {
        if (error_count > 0)
            fputc('\n', stderr);
        fputs(ANSI_BRIGHT_RED "error: " ANSI_RESET, stderr);
        error_count++;
    } else {
        fputs(ANSI_BRIGHT_CYAN "note: " ANSI_RESET, stderr);
    }

    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
}

void emit_message(bool error, char *format, ...) {
    va_list ap;

    va_start(ap, format);
    va_emit_message(error, format, ap);
    va_end(ap);
}

void emit_message_with_span(span span, bool error, char *format, ...) {
    va_list ap;

    va_start(ap, format);
    va_emit_message(error, format, ap);
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

    while (lineno <= span.el && fgets(line, sizeof line, file)) {
        if (lineno >= span.bl) {
            fprintf(stderr, "%*d " ANSI_BRIGHT_BLUE "|" ANSI_RESET " %s",
                    lineno_width, lineno + 1, line);
        }

        if (line[STRlen(line) - 1] == '\n')
            lineno++;
    }

    fclose(file);

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s|", lineno_width + 1, "");

    char *color = error ? ANSI_BRIGHT_RED : ANSI_BRIGHT_CYAN;

    if (span.bl == span.el) {
        fprintf(stderr, "%*s%s", span.bc + 1, "", color);
        for (int i = 0; i <= span.ec - span.bc; i++)
            fputc('^', stderr);
    }

    fprintf(stderr, "\n" ANSI_BRIGHT_BLUE "%*s|%s %*s", lineno_width + 1, "",
            color, span.bc, "");

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, ANSI_BRIGHT_BLUE "\n%*s\n" ANSI_RESET, lineno_width + 2,
            "|");
}
