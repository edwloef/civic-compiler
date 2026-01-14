#pragma once

#define SPAN(node)                                                             \
    {NODE_BLINE(node), NODE_BCOL(node), NODE_ELINE(node), NODE_ECOL(node),     \
     NODE_FILE(node)}

typedef struct {
    int bl, bc, el, ec;
    char *file;
} span;
