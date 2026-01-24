#include "ccn/ccn.h"

void VPLinit(void) {}
void VPLfini(void) {}

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
