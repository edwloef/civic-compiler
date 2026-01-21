#include "ccn/ccn.h"

void CPinit(void) {
    DATA_CP_GET()->can_prop = true;
}
void CPfini(void) {}

node_st *CPcall(node_st *node) {
    TRAVchildren(node);

    if (DATA_CP_GET()->write_count > 1) {
        DATA_CP_GET()->can_prop = false;
    }

    return node;
}

node_st *CPvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_CP_GET()->n == VARREF_N(node) &&
        DATA_CP_GET()->l == VARREF_L(node) && VARREF_WRITE(node)) {
        DATA_CP_GET()->can_prop = false;
    }

    return node;
}
