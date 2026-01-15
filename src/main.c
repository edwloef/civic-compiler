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
    printf("  -h --help                     This help message.\n");
    printf("  -o --output <output>          Output assembly to output file "
           "instead of STDOUT.\n");
    printf("  -b --breakpoint <breakpoint>  Set a breakpoint.\n");
    printf("  -s --structure                Pretty print the structure of the "
           "compiler.\n");
    printf("  --f[no]associative-math       Enable/disable floating-point "
           "associativity\n");
}

#define FASSOCIATIVE_MATH 256
#define FNO_ASSOCIATIVE_MATH 257

static struct option options[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {"breakpoint", required_argument, 0, 'b'},
    {"structure", no_argument, 0, 's'},
    {"fassociative-math", no_argument, 0, FASSOCIATIVE_MATH},
    {"fno-associative-math", no_argument, 0, FNO_ASSOCIATIVE_MATH},
    {0, 0, 0, 0}};

static int ProcessArgs(int argc, char *argv[]) {
    int c;

    while ((c = getopt_long_only(argc, argv, "ho:b:s", options, NULL)) != -1) {
        switch (c) {
        case 'h':
            Usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 'o':
            globals.output_file = optarg;
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
            exit(EXIT_SUCCESS);
        case FASSOCIATIVE_MATH:
            globals.fassociative_math = true;
            break;
        case FNO_ASSOCIATIVE_MATH:
            globals.fassociative_math = false;
            break;
        default:
            Usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind + 1 != argc) {
        Usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    globals.input_file = argv[optind];

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
