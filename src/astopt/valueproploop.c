#include "ccn/ccn.h"

void VPLinit(void) {}
void VPLfini(void) {}

node_st *VPLvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_VPL_GET()->expr && DATA_VPL_GET()->n == VARREF_N(node) &&
        DATA_VPL_GET()->l == VARREF_L(node)) {
        CCNfree(node);
        node = CCNcopy(DATA_VPL_GET()->expr);
        CCNcycleNotify();
    }

    return node;
}
