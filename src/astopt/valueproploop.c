#include "ccn/ccn.h"

void VPLinit(void) {}
void VPLfini(void) {}

node_st *VPLdowhile(node_st *node) {
    node_st *expr = DOWHILE_EXPR(node);
    if (NODE_TYPE(DATA_VPL_GET()->expr) == NT_INT &&
        NODE_TYPE(expr) == NT_BINOP && NODE_TYPE(BINOP_LEFT(expr)) == NT_INT &&
        NODE_TYPE(BINOP_RIGHT(expr)) == NT_VARREF &&
        VARREF_N(BINOP_RIGHT(expr)) == VARREF_N(DATA_VPL_GET()->ref) &&
        VARREF_L(BINOP_RIGHT(expr)) == VARREF_L(DATA_VPL_GET()->ref)) {
        DOWHILE_KNOWN_START(node) = true;
        DOWHILE_UNROLL_START(node) = INT_VAL(DATA_VPL_GET()->expr);
    }

    TRAVchildren(node);

    return node;
}

node_st *VPLvarref(node_st *node) {
    TRAVchildren(node);

    if (VARREF_N(DATA_VPL_GET()->ref) == VARREF_N(node) &&
        VARREF_L(DATA_VPL_GET()->ref) == VARREF_L(node)) {
        CCNfree(node);
        node = CCNcopy(DATA_VPL_GET()->expr);
        CCNcycleNotify();
    }

    return node;
}
