#include <ctype.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccn/ccn.h"
#include "globals/globals.h"

static void Usage(char *program) {
    char *program_bin = strrchr(program, '/');
    if (program_bin)
        program = program_bin + 1;

    printf("Usage: %s [OPTION...] <civic file>\n", program);
    printf("Options:\n");
    printf("  -h                            This help message.\n");
    printf("  --output/-o <output_file>     Output assembly to output file "
           "instead of STDOUT.\n");
    printf("  --verbose/-v                  Enable verbose mode.\n");
    printf("  --breakpoint/-b <breakpoint>  Set a breakpoint.\n");
    printf("  --structure/-s                Pretty print the structure of the "
           "compiler.\n");
    printf(
        "  --strength-reduction-limit/-r The maximum number of additions "
        "that will replace a multiplication in the strength reduction pass.\n");
}

static int ProcessArgs(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"output", required_argument, 0, 'o'},
        {"breakpoint", required_argument, 0, 'b'},
        {"structure", no_argument, 0, 's'},
        {"strength-reduction-limit", required_argument, 0, 'r'},
        {0, 0, 0, 0}};

    int option_index;
    int c;

    while (1) {
        c = getopt_long(argc, argv, "hsvor:b:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 'v':
            globals.verbose = 1;
            CCNsetVerbosity(PD_V_MEDIUM);
            break;
        case 'b':
            if (optarg != NULL && isdigit(optarg[0])) {
                CCNsetBreakpointWithID((int)strtol(optarg, NULL, 10));
            } else {
                CCNsetBreakpoint(optarg);
            }
            break;
        case 's':
            CCNshowTree();
            break;
        case 'o':
            globals.output_file = optarg;
            break;
        case 'r':
            globals.strength_reduction_limit = atoi(optarg);
            break;
        case 'h':
            Usage(argv[0]);
            exit(EXIT_SUCCESS);
        case '?':
            Usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (optind < argc) {
        globals.input_file = argv[optind];
    } else {
        Usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    return 0;
}

void BreakpointHandler(node_st *root) {
    TRAVstart(root, TRAV_PRT);
    return;
}

int main(int argc, char **argv) {
    ProcessArgs(argc, argv);
    CCNrun(NULL);
    return 0;
}
