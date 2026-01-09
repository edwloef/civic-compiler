#include "analysis/error.h"
#include "globals/globals.h"
#include "palm/ctinfo.h"
#include "palm/str.h"

#include <math.h>
#include <stdio.h>

#define ANSI_RESET "\033[0m"
#define ANSI_BRIGHT_RED "\033[30;91m"
#define ANSI_BRIGHT_BLUE "\033[30;94m"

void emit_error_at_node(node_st *node, char *error) {
    if (CTIgetErrors())
        fprintf(stderr, "\n");

    CTI(CTI_ERROR, false, "%s", error);

    FILE *file = fopen(globals.input_file, "r");
    if (file == NULL)
        return;

    int lineno_width = log10(NODE_ELINE(node) + 1) + 1;

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s" ANSI_RESET " %s:%d:%d\n",
            lineno_width + 3, "-->", globals.input_file, NODE_BLINE(node) + 1,
            NODE_BCOL(node) + 1);
    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s\n", lineno_width + 2, "|");

    uint32_t lineno = 0;
    char line[256];

    while (lineno <= NODE_ELINE(node) && fgets(line, sizeof line, file)) {
        if (lineno >= NODE_BLINE(node)) {
            fprintf(stderr, "%*d " ANSI_BRIGHT_BLUE "|" ANSI_RESET " %s",
                    lineno_width, lineno + 1, line);
        }

        if (line[STRlen(line) - 1] == '\n')
            lineno++;
    }

    fclose(file);

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s", lineno_width + 2, "|");

    if (NODE_BLINE(node) == NODE_ELINE(node)) {
        fprintf(stderr, "%*s" ANSI_BRIGHT_RED, NODE_BCOL(node) + 1, "");
        for (uint32_t i = 0; i <= NODE_ECOL(node) - NODE_BCOL(node); i++)
            putc('^', stderr);
        fprintf(stderr, "\n" ANSI_BRIGHT_BLUE "%*s" ANSI_BRIGHT_RED " %*s%s\n",
                lineno_width + 2, "|", NODE_BCOL(node), "", error);
    } else {
        fprintf(stderr, "%*s" ANSI_BRIGHT_RED " %s\n", lineno_width + 2, "|",
                error);
    }

    fprintf(stderr, ANSI_BRIGHT_BLUE "%*s\n" ANSI_RESET, lineno_width + 2, "|");
}
