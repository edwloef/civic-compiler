#include "analysis/error.h"
#include "globals/globals.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "palm/str.h"

#include <math.h>
#include <stdio.h>

#define ANSI_RESET "\033[0m"
#define ANSI_BRIGHT_RED "\033[30;91m"
#define ANSI_BRIGHT_BLUE "\033[30;94m"

void emit_message_at_node(enum cti_type type, span span, char *error) {
    if (CTIgetErrors())
        fprintf(stderr, "\n");

    CTI(type, false, "%s", error);

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

    char *color;
    if (type == CTI_ERROR) {
        color = ANSI_BRIGHT_RED;
    } else if (type == CTI_NOTE) {
        color = ANSI_BRIGHT_BLUE;
    } else {
        DBUG_ASSERT(false, "Unreachable.");
    }

    if (span.bl == span.el) {
        fprintf(stderr, "%*s%s", span.bc + 1, "", color);
        for (int i = 0; i <= span.ec - span.bc; i++)
            putc('^', stderr);
        fprintf(stderr, "\n" ANSI_BRIGHT_BLUE "%*s%s %*s%s\n", lineno_width + 2,
                "|", color, span.bc, "", error);
    } else {
        fprintf(stderr, "%*s%s %s\n", lineno_width + 2, "|", color, error);
    }

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s\n" ANSI_RESET, lineno_width + 2, "|");
}
