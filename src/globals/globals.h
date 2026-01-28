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

    bool fassociative_math;
    bool ffinite_math_only;
    bool fsigned_zeros;
};

extern struct globals globals;
