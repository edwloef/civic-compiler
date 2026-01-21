#include "ccn/ccn.h"

void PILinit(void) {}
void PILfini(void) {}

node_st *PILvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_PIL_GET()->expr && DATA_PIL_GET()->n == VARREF_N(node) &&
        DATA_PIL_GET()->l == VARREF_L(node)) {
        CCNfree(node);
        node = CCNcopy(DATA_PIL_GET()->expr);
        CCNcycleNotify();
    }

    return node;
}