#pragma once

#include <stdbool.h>

struct globals {
    int line;
    int col;
    char *file;

    char *input_file;
    char *output_file;

    bool fassociative_math;
};

extern struct globals globals;
