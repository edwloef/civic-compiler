#pragma once

struct globals {
    int line;
    int col;
    char *input_file;
    char *output_file;
};

extern struct globals globals;
