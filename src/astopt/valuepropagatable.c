#include "ccn/ccn.h"

void VPinit(void) {
    DATA_VP_GET()->propagatable = true;
}
void VPfini(void) {}

node_st *VPcall(node_st *node) {
    TRAVchildren(node);

    if (DATA_VP_GET()->write_count > 1) {
        DATA_VP_GET()->propagatable = false;
    }

    return node;
}

node_st *VPvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_VP_GET()->n == VARREF_N(node) &&
        DATA_VP_GET()->l == VARREF_L(node) && VARREF_WRITE(node)) {
        DATA_VP_GET()->propagatable = false;
    }

    return node;
}