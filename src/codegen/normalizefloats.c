#include <math.h>

#include "ccn/ccn.h"
#include "globals/globals.h"

node_st *CGNFfloat(node_st *node) {
    if (isnan(FLOAT_VAL(node))) {
        FLOAT_VAL(node) = NAN;
    } else if (FLOAT_VAL(node) == 0.0 && !globals.fsigned_zeros) {
        FLOAT_VAL(node) = 0.0;
    }

    return node;
}
