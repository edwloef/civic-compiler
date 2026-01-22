#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/dbug.h"

#define PROP_INTO_LOOP(e)                                                      \
    {                                                                          \
        if (DATA_VP_GET()->expr && DATA_VP_GET()->write_count > 1) {           \
            int write_count = DATA_VP_GET()->write_count;                      \
            int n = DATA_VP_GET()->n;                                          \
            int l = DATA_VP_GET()->l;                                          \
            TRAVpush(TRAV_CPL);                                                \
            DATA_CPL_GET()->write_count = write_count;                         \
            DATA_CPL_GET()->n = n;                                             \
            DATA_CPL_GET()->l = l;                                             \
            e = TRAVopt(e);                                                    \
            bool can_prop = DATA_CPL_GET()->can_prop;                          \
            TRAVpop();                                                         \
            if (!can_prop) {                                                   \
                DATA_VP_GET()->expr = NULL;                                    \
            }                                                                  \
        }                                                                      \
        if (DATA_VP_GET()->expr) {                                             \
            node_st *expr = DATA_VP_GET()->expr;                               \
            int n = DATA_VP_GET()->n;                                          \
            int l = DATA_VP_GET()->l;                                          \
            TRAVpush(TRAV_VPL);                                                \
            DATA_VPL_GET()->expr = expr;                                       \
            DATA_VPL_GET()->n = n;                                             \
            DATA_VPL_GET()->l = l;                                             \
            e = TRAVopt(e);                                                    \
            TRAVpop();                                                         \
        }                                                                      \
    }

void VPinit(void) {}
void VPfini(void) {}

node_st *VPstmts(node_st *node) {
    TRAVstmt(node);

    if (DATA_VP_GET()->expr) {
        TRAVnext(node);
    }

    return node;
}

node_st *VPassign(node_st *node) {
    TRAVexpr(node);
    TRAVref(node);

    return node;
}

node_st *VPifelse(node_st *node) {
    TRAVexpr(node);

    node_st *before = DATA_VP_GET()->expr;

    TRAVif_block(node);

    node_st *after = DATA_VP_GET()->expr;
    DATA_VP_GET()->expr = before;

    TRAVelse_block(node);

    if (DATA_VP_GET()->expr) {
        DATA_VP_GET()->expr = after;
    }

    return node;
}

node_st *VPwhile(node_st *node) {
    DBUG_ASSERT(false, "Unreachable.");

    return node;
}

node_st *VPdowhile(node_st *node) {
    PROP_INTO_LOOP(node);

    return node;
}

node_st *VPfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    PROP_INTO_LOOP(FOR_STMTS(node));

    return node;
}

node_st *VPreturn(node_st *node) {
    TRAVchildren(node);

    return node;
}

node_st *VPcall(node_st *node) {
    TRAVchildren(node);

    if (DATA_VP_GET()->write_count > 1 && CALL_N(node) <= DATA_VP_GET()->n) {
        DATA_VP_GET()->expr = NULL;
    }

    return node;
}

node_st *VPvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_VP_GET()->expr) {
        if (NODE_TYPE(DATA_VP_GET()->expr) == NT_VARREF &&
            VARREF_N(node) == VARREF_N(DATA_VP_GET()->expr) &&
            VARREF_L(node) == VARREF_L(DATA_VP_GET()->expr)) {
            if (VARREF_WRITE(node)) {
                DATA_VP_GET()->expr = NULL;
            }
        } else if (VARREF_N(node) == DATA_VP_GET()->n &&
                   VARREF_L(node) == DATA_VP_GET()->l) {
            if (VARREF_WRITE(node)) {
                DATA_VP_GET()->expr = NULL;
            } else {
                CCNfree(node);
                node = CCNcopy(DATA_VP_GET()->expr);
                CCNcycleNotify();
            }
        }
    }

    return node;
}