#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/dbug.h"

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
    vartable_ref r = {VARREF_N(DATA_VP_GET()->ref),
                      VARREF_L(DATA_VP_GET()->ref)};
    vartable_entry *e = vartable_get(DATA_VP_GET()->vartable, r);

    if (DATA_VP_GET()->expr && e->write_count > 1) {
        node_st *ref = DATA_VP_GET()->ref;
        node_st *expr = DATA_VP_GET()->expr;
        funtable *funtable = DATA_VP_GET()->funtable;
        vartable *vartable = DATA_VP_GET()->vartable;

        TRAVpush(TRAV_CPL);

        DATA_CPL_GET()->ref = ref;
        DATA_CPL_GET()->expr = expr;
        DATA_CPL_GET()->funtable = funtable;
        DATA_CPL_GET()->vartable = vartable;

        node = TRAVopt(node);

        bool can_prop = DATA_CPL_GET()->can_prop;

        TRAVpop();

        if (!can_prop) {
            DATA_VP_GET()->expr = NULL;
        }
    }

    if (DATA_VP_GET()->expr) {
        node_st *ref = DATA_VP_GET()->ref;
        node_st *expr = DATA_VP_GET()->expr;

        TRAVpush(TRAV_VPL);

        DATA_VPL_GET()->ref = ref;
        DATA_VPL_GET()->expr = expr;

        node = TRAVopt(node);

        TRAVpop();
    }

    return node;
}

node_st *VPfor(node_st *node) {
    DBUG_ASSERT(false, "Unreachable.");

    return node;
}

node_st *VPreturn(node_st *node) {
    TRAVchildren(node);

    return node;
}

node_st *VPcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_VP_GET()->funtable, r);

    if (CALL_N(node) <= VARREF_N(DATA_VP_GET()->ref) &&
        CALL_N(node) + e->scalar_write_capture > VARREF_N(DATA_VP_GET()->ref)) {
        vartable_ref r = {VARREF_N(DATA_VP_GET()->ref),
                          VARREF_L(DATA_VP_GET()->ref)};
        vartable_entry *e = vartable_get(DATA_VP_GET()->vartable, r);
        if (e->escapes && e->write_count > 1) {
            DATA_VP_GET()->expr = NULL;
        }
    }

    if (DATA_VP_GET()->expr && NODE_TYPE(DATA_VP_GET()->expr) == NT_VARREF &&
        CALL_N(node) <= VARREF_N(DATA_VP_GET()->expr) &&
        CALL_N(node) + e->scalar_write_capture >
            VARREF_N(DATA_VP_GET()->expr)) {
        vartable_ref r = {VARREF_N(DATA_VP_GET()->expr),
                          VARREF_L(DATA_VP_GET()->expr)};
        vartable_entry *e = vartable_get(DATA_VP_GET()->vartable, r);
        if (e->escapes) {
            DATA_VP_GET()->expr = NULL;
        }
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
        } else if (VARREF_N(node) == VARREF_N(DATA_VP_GET()->ref) &&
                   VARREF_L(node) == VARREF_L(DATA_VP_GET()->ref)) {
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