#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "ccn/ccn.h"
#include "globals/globals.h"

static void Usage(char *program) {
    char *program_bin = strrchr(program, '/');
    if (program_bin)
        program = program_bin + 1;

    printf("Usage: %s [OPTION...] <civic file>\n", program);
    printf("Options:\n");
    printf("  --help/-h                     This help message.\n");
    printf("  --output/-o <output_file>     Output assembly to output file "
           "instead of STDOUT.\n");
    printf("  --verbose/-v                  Enable verbose mode.\n");
    printf("  --breakpoint/-b <breakpoint>  Set a breakpoint.\n");
    printf("  --structure/-s                Pretty print the structure of the "
           "compiler.\n");
}

static int ProcessArgs(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"output", required_argument, 0, 'o'},
        {"verbose", no_argument, 0, 'v'},
        {"breakpoint", required_argument, 0, 'b'},
        {"structure", no_argument, 0, 's'},
        {0, 0, 0, 0}};

    while (1) {
        int c = getopt_long(argc, argv, "hovb:s", long_options, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'h':
            Usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 'o':
            globals.output_file = optarg;
            break;
        case 'v':
            CCNsetVerbosity(PD_V_HIGH);
            break;
        case 'b':
            if (isdigit(optarg[0])) {
                CCNsetBreakpointWithID(atoi(optarg));
            } else {
                CCNsetBreakpoint(optarg);
            }
            break;
        case 's':
            CCNshowTree();
            break;
        default:
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
