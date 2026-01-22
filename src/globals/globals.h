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

    bool fassociative_math;
    bool fsigned_zeros;
};

extern struct globals globals;
