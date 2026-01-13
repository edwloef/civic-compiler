#pragma once

#define SPAN(node)                                                             \
    {NODE_BLINE(node), NODE_BCOL(node), NODE_ELINE(node), NODE_ECOL(node)}

typedef struct {
    int bl, bc, el, ec;
} span;
