#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "ccn/ccn.h"
#include "error/error.h"
#include "globals/globals.h"

static void Usage(char *program) {
    char *program_bin = strrchr(program, '/');
    if (program_bin) {
        program = program_bin + 1;
    }

    printf("Usage: %s [OPTION...] <civic file>\n", program);
    printf("Options:\n");
    printf("  -h --help                     This help message.\n");
    printf("  -o --output <output>          Output assembly to output file "
           "instead of STDOUT.\n");
    printf(
        "  -q --quiet                    Make compiler output less verbose.\n");
    printf("  -O --optimize                 Run optimizations on generated "
           "code.\n");
    printf("  -b --breakpoint <breakpoint>  Set a breakpoint.\n");
    printf("  -s --structure                Pretty print the structure of the "
           "compiler.\n");
    printf("     --unroll-limit <limit>     Set the number of operations "
           "allowed to be unrolled in a loop. Default value: 256");
    printf("  -f[no]associative-math        Allow re-association of "
           "floating-point operations. Requires -fno-signed-zeros.\n");
    printf("  -f[no]finite-math-only        Allow optimizations for "
           "floating-point arithmetic that assume arguments and results are "
           "not NaNs or +-Infs.\n");
    printf("  -f[no]signed-zeros            Allow optimizations for "
           "floating-point arithmetic that ignore the signedness of zero.\n");
}

#define UNROLL_LIMIT 256
#define FASSOCIATIVE_MATH 257
#define FNO_ASSOCIATIVE_MATH 258
#define FFINITE_MATH_ONLY 259
#define FNO_FINITE_MATH_ONLY 260
#define FSIGNED_ZEROS 261
#define FNO_SIGNED_ZEROS 262

static struct option options[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {"quiet", no_argument, 0, 'q'},
    {"optimize", no_argument, 0, 'O'},
    {"breakpoint", required_argument, 0, 'b'},
    {"structure", no_argument, 0, 's'},
    {"unroll-limit", required_argument, 0, UNROLL_LIMIT},
    {"fassociative-math", no_argument, 0, FASSOCIATIVE_MATH},
    {"fno-associative-math", no_argument, 0, FNO_ASSOCIATIVE_MATH},
    {"ffinite-math-only", no_argument, 0, FASSOCIATIVE_MATH},
    {"fno-finite-math-only", no_argument, 0, FNO_ASSOCIATIVE_MATH},
    {"fsigned-zeros", no_argument, 0, FASSOCIATIVE_MATH},
    {"fno-signed-zeros", no_argument, 0, FNO_ASSOCIATIVE_MATH},
    {0, 0, 0, 0}};

static void ProcessArgs(int argc, char *argv[]) {
    int c;

    while ((c = getopt_long_only(argc, argv, "ho:qOb:s", options, NULL)) !=
           -1) {
        switch (c) {
        case 'h':
            Usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 'o':
            globals.output_file = optarg;
            break;
        case 'q':
            globals.quiet = true;
            break;
        case 'O':
            globals.optimize = true;
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
        case UNROLL_LIMIT:
            globals.unroll_limit = atoi(optarg);
            break;
        case FASSOCIATIVE_MATH:
            globals.fassociative_math = true;
            break;
        case FNO_ASSOCIATIVE_MATH:
            globals.fassociative_math = false;
            break;
        case FFINITE_MATH_ONLY:
            globals.ffinite_math_only = true;
            break;
        case FNO_FINITE_MATH_ONLY:
            globals.ffinite_math_only = false;
            break;
        case FSIGNED_ZEROS:
            globals.fsigned_zeros = true;
            break;
        case FNO_SIGNED_ZEROS:
            globals.fsigned_zeros = false;
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

    if (globals.fassociative_math && globals.fsigned_zeros) {
        emit_message(L_ERROR, "-fassociative-math requires -fno-signed-zeros");
        abort_on_error();
    }

#ifdef NDEBUG
    CCNsetTreeCheck(false);
#endif
}

void BreakpointHandler(node_st *root) {
    TRAVstart(root, TRAV_PRT);
}

int main(int argc, char **argv) {
    ProcessArgs(argc, argv);
    CCNrun(NULL);
    return 0;
}

bool is_optimize_enabled(void) {
    return globals.optimize;
}