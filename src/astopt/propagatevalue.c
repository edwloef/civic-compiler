#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/dbug.h"

#define CHECK_PROPAGATABLE()                                                   \
    {                                                                          \
        int write_count = DATA_PV_GET()->write_count;                          \
        int n = DATA_PV_GET()->n;                                              \
        int l = DATA_PV_GET()->l;                                              \
        TRAVpush(TRAV_VP);                                                     \
        DATA_VP_GET()->write_count = write_count;                              \
        DATA_VP_GET()->n = n;                                                  \
        DATA_VP_GET()->l = l;                                                  \
        TRAVstmts(node);                                                       \
        bool propagatable = DATA_VP_GET()->propagatable;                       \
        TRAVpop();                                                             \
        if (!propagatable) {                                                   \
            DATA_PV_GET()->expr = NULL;                                        \
        }                                                                      \
    }

void PVinit(void) {}
void PVfini(void) {}

node_st *PVassign(node_st *node) {
    TRAVexpr(node);
    TRAVref(node);

    return node;
}

node_st *PVifelse(node_st *node) {
    TRAVexpr(node);

    node_st *before = DATA_PV_GET()->expr;

    TRAVif_block(node);

    node_st *after = DATA_PV_GET()->expr;
    DATA_PV_GET()->expr = before;

    TRAVelse_block(node);

    if (DATA_PV_GET()->expr) {
        DATA_PV_GET()->expr = after;
    }

    return node;
}

node_st *PVwhile(node_st *node) {
    DBUG_ASSERT(false, "Unreachable.");

    return node;
}

node_st *PVdowhile(node_st *node) {
    CHECK_PROPAGATABLE();

    TRAVchildren(node);

    return node;
}

node_st *PVfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    CHECK_PROPAGATABLE();

    TRAVstmts(node);

    return node;
}

node_st *PVreturn(node_st *node) {
    TRAVchildren(node);

    return node;
}

node_st *PVcall(node_st *node) {
    TRAVchildren(node);

    if (DATA_PV_GET()->expr && DATA_PV_GET()->write_count > 1) {
        DATA_PV_GET()->expr = NULL;
    }

    return node;
}

node_st *PVvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_PV_GET()->expr && DATA_PV_GET()->n == VARREF_N(node) &&
        DATA_PV_GET()->l == VARREF_L(node)) {
        if (VARREF_WRITE(node)) {
            DATA_PV_GET()->expr = NULL;
        } else {
            CCNfree(node);
            node = CCNcopy(DATA_PV_GET()->expr);
            CCNcycleNotify();
        }
    }

    return node;
}