#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "palm/hash_table.h"

struct globals {
    int line;
    int col;
    char *file;

    char *input_file;
    char *output_file;

    htable_st *linemap; // file => (lineno => line)

    bool quiet;
    bool preprocessor;
    bool optimize;

    int unroll_limit;

    bool associative_math;
    bool finite_math_only;
    bool signed_zeros;

    FILE *output_stream;
};

extern struct globals globals;
