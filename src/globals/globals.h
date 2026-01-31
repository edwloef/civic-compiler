#pragma once

#include <stdbool.h>

#include "palm/hash_table.h"

struct globals {
    int line;
    int col;
    char *file;

    char *input_file;
    char *output_file;

    htable_st *linemap; // file => (lineno => line)

    bool quiet;
    bool optimize;
    bool preprocessor;

    int unroll_limit;

    bool associative_math;
    bool finite_math_only;
    bool signed_zeros;
};

extern struct globals globals;
