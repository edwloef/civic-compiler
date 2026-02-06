#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "ccn/ccn.h"
#include "globals/globals.h"

static void Usage(char *program) {
    char *program_bin = strrchr(program, '/');
    if (program_bin) {
        program = program_bin + 1;
    }

    printf("Usage: %s [OPTION...] <input>\n", program);
    printf("Options:\n");
    printf("  -h --help                     This help message.\n");
    printf("  -o --output <output>          Output assembly to output file "
           "instead of STDOUT.\n");
    printf(
        "  -q --quiet                    Make compiler output less verbose.\n");
    printf("  -b --breakpoint <breakpoint>  Set a breakpoint.\n");
    printf("  -s --structure                Pretty print the structure of the "
           "compiler.\n");
    printf("     --[no-]preprocessor        Disable the C preprocessor.\n");
    printf("     --[no-]optimize            Disable compiler optimizations.\n");
    printf("     --unroll-limit <limit>     Set the number of operations "
           "allowed to be unrolled in a loop. Default value: 100\n");
    printf("     --[no-]associative-math    Allow re-association of "
           "floating-point operations. Requires --no-signed-zeros.\n");
    printf("     --[no-]finite-math-only    Allow optimizations for "
           "floating-point arithmetic that assume arguments and results are "
           "not NaNs or +-Infs.\n");
    printf("     --[no-]signed-zeros        Allow optimizations for "
           "floating-point arithmetic that ignore the signedness of zero.\n");
    printf("  <input>                       Optional input file. Defaults to "
           "stdin.\n");
}

enum {
    preprocessor = 256,
    no_preprocessor,
    optimize,
    no_optimize,
    unroll_limit,
    associative_math,
    no_associative_math,
    finite_math_only,
    no_finite_math_only,
    signed_zeros,
    no_signed_zeros
};

static struct option options[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {"quiet", no_argument, 0, 'q'},
    {"breakpoint", required_argument, 0, 'b'},
    {"structure", no_argument, 0, 's'},
    {"preprocessor", no_argument, 0, preprocessor},
    {"no-preprocessor", no_argument, 0, no_preprocessor},
    {"optimize", no_argument, 0, optimize},
    {"no-optimize", no_argument, 0, no_optimize},
    {"unroll-limit", required_argument, 0, unroll_limit},
    {"associative-math", no_argument, 0, associative_math},
    {"no-associative-math", no_argument, 0, no_associative_math},
    {"finite-math-only", no_argument, 0, finite_math_only},
    {"no-finite-math-only", no_argument, 0, no_finite_math_only},
    {"signed-zeros", no_argument, 0, signed_zeros},
    {"no-signed-zeros", no_argument, 0, no_signed_zeros},
    {0, 0, 0, 0}};

static void ProcessArgs(int argc, char *argv[]) {
    int c;

    while ((c = getopt_long(argc, argv, "ho:qb:s", options, NULL)) != -1) {
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
        case preprocessor:
            globals.preprocessor = true;
            break;
        case no_preprocessor:
            globals.preprocessor = false;
            break;
        case optimize:
            globals.optimize = true;
            break;
        case no_optimize:
            globals.optimize = false;
            break;
        case unroll_limit:
            globals.unroll_limit = atoi(optarg);
            break;
        case associative_math:
            globals.associative_math = true;
            break;
        case no_associative_math:
            globals.associative_math = false;
            break;
        case finite_math_only:
            globals.finite_math_only = true;
            break;
        case no_finite_math_only:
            globals.finite_math_only = false;
            break;
        case signed_zeros:
            globals.signed_zeros = true;
            break;
        case no_signed_zeros:
            globals.signed_zeros = false;
            break;
        default:
            Usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (globals.associative_math && globals.signed_zeros) {
        emit_message(L_ERROR, "--associative-math requires --no-signed-zeros");
        abort_on_error();
    }

    if (optind + 1 < argc) {
        Usage(argv[0]);
        exit(EXIT_FAILURE);
    } else if (optind + 1 == argc) {
        globals.input_file = argv[optind];
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
