#pragma once

#include <stdbool.h>

struct globals {
    int line;
    int col;
    char *file;

    char *input_file;
    char *output_file;

    bool fassociative_math;
    bool ffinite_math_only;
    bool fsigned_zeros;
};

extern struct globals globals;
