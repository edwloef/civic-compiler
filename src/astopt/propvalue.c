#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/dbug.h"

#define PROP_INTO_LOOP(e)                                                      \
    {                                                                          \
        if (DATA_PV_GET()->expr && DATA_PV_GET()->write_count > 1) {           \
            int write_count = DATA_PV_GET()->write_count;                      \
            int n = DATA_PV_GET()->n;                                          \
            int l = DATA_PV_GET()->l;                                          \
            TRAVpush(TRAV_CP);                                                 \
            DATA_CP_GET()->write_count = write_count;                          \
            DATA_CP_GET()->n = n;                                              \
            DATA_CP_GET()->l = l;                                              \
            e = TRAVopt(e);                                                    \
            bool can_prop = DATA_CP_GET()->can_prop;                           \
            TRAVpop();                                                         \
            if (!can_prop) {                                                   \
                DATA_PV_GET()->expr = NULL;                                    \
            }                                                                  \
        }                                                                      \
        if (DATA_PV_GET()->expr) {                                             \
            node_st *expr = DATA_PV_GET()->expr;                               \
            int n = DATA_PV_GET()->n;                                          \
            int l = DATA_PV_GET()->l;                                          \
            TRAVpush(TRAV_PIL);                                                \
            DATA_PIL_GET()->expr = expr;                                       \
            DATA_PIL_GET()->n = n;                                             \
            DATA_PIL_GET()->l = l;                                             \
            e = TRAVopt(e);                                                    \
            TRAVpop();                                                         \
        }                                                                      \
    }

void PVinit(void) {}
void PVfini(void) {}

node_st *PVstmts(node_st *node) {
    TRAVstmt(node);

    if (DATA_PV_GET()->expr) {
        TRAVnext(node);
    }

    return node;
}

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
    PROP_INTO_LOOP(node);

    return node;
}

node_st *PVfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    PROP_INTO_LOOP(FOR_STMTS(node));

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