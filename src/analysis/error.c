#include <math.h>
#include <stdio.h>

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

void emit_message(char *message, bool error) {
    if (error) {
        if (error_count > 0)
            fprintf(stderr, "\n");
        fprintf(stderr, ANSI_BRIGHT_RED "error: " ANSI_RESET "%s\n", message);
        error_count++;
    } else {
        fprintf(stderr, ANSI_BRIGHT_CYAN "note: " ANSI_RESET "%s\n", message);
    }
}

void emit_message_with_span(span span, char *message, bool error) {
    emit_message(message, error);

    FILE *file = fopen(globals.input_file, "r");
    if (file == NULL)
        return;

    int lineno_width = log10(span.el + 1) + 1;

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s" ANSI_RESET " %s:%d:%d\n",
            lineno_width + 3, "-->", globals.input_file, span.bl + 1,
            span.bc + 1);
    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s\n", lineno_width + 2, "|");

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

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s", lineno_width + 2, "|");

    char *color = error ? ANSI_BRIGHT_RED : ANSI_BRIGHT_CYAN;
    if (span.bl == span.el) {
        fprintf(stderr, "%*s%s", span.bc + 1, "", color);
        for (int i = 0; i <= span.ec - span.bc; i++)
            putc('^', stderr);
        fprintf(stderr, "\n" ANSI_BRIGHT_BLUE "%*s%s %*s%s\n", lineno_width + 2,
                "|", color, span.bc, "", message);
    } else {
        fprintf(stderr, "\n%*s%s %s\n", lineno_width + 2, "|", color, message);
    }

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s\n" ANSI_RESET, lineno_width + 2, "|");
}
