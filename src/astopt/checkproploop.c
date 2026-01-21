#include "ccn/ccn.h"
#include "ccngen/trav.h"

void CPLinit(void) {
    DATA_CPL_GET()->can_prop = true;
}
void CPLfini(void) {}

node_st *CPLstmts(node_st *node) {
    TRAVstmt(node);

    if (DATA_CPL_GET()->can_prop) {
        TRAVnext(node);
    }

    return node;
}

node_st *CPLcall(node_st *node) {
    TRAVchildren(node);

    if (DATA_CPL_GET()->write_count > 1) {
        DATA_CPL_GET()->can_prop = false;
    }

    return node;
}

node_st *CPLvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_CPL_GET()->n == VARREF_N(node) &&
        DATA_CPL_GET()->l == VARREF_L(node) && VARREF_WRITE(node)) {
        DATA_CPL_GET()->can_prop = false;
    }

    return node;
}
