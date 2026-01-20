#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/dbug.h"

void AOCPinit(void) {}
void AOCPfini(void) {}

node_st *AOCPprogram(node_st *node) {
    TRAVstart(node, TRAV_CW);

    DATA_AOCP_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOCPfundecl(node_st *node) {
    DATA_AOCP_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AOCP_GET()->vartable = DATA_AOCP_GET()->vartable->parent;

    return node;
}

node_st *AOCPstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_ASSIGN && !VARREF_EXPRS(ASSIGN_REF(stmt)) &&
        (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_INT ||
         NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_FLOAT ||
         NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_BOOL ||
         (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF &&
          !VARREF_EXPRS(ASSIGN_EXPR(stmt))))) {
        vartable *vartable = DATA_AOCP_GET()->vartable;

        TRAVpush(TRAV_PV);

        DATA_PV_GET()->expr = ASSIGN_EXPR(stmt);

        if (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF) {
            vartable_ref r = {VARREF_N(ASSIGN_EXPR(stmt)),
                              VARREF_L(ASSIGN_EXPR(stmt))};
            vartable_entry *e = vartable_get(vartable, r);
            DATA_PV_GET()->write_count = e->write_count;
        }

        DATA_PV_GET()->n = VARREF_N(ASSIGN_REF(stmt));
        DATA_PV_GET()->l = VARREF_L(ASSIGN_REF(stmt));

        TRAVnext(node);

        TRAVpop();
    }

    return node;
}

void CWinit(void) {}
void CWfini(void) {}

node_st *CWprogram(node_st *node) {
    DATA_CW_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *CWfundecl(node_st *node) {
    for (int i = 0; i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *e = &FUNDECL_VARTABLE(node)->buf[i];
        if (e->external || e->exported) {
            e->write_count = 2;
        } else if (e->param) {
            e->write_count = 1;
        }
    }

    DATA_CW_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_CW_GET()->vartable = DATA_CW_GET()->vartable->parent;

    return node;
}

node_st *CWvarref(node_st *node) {
    if (VARREF_WRITE(node)) {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_CW_GET()->vartable, r);
        e->write_count++;
    }

    return node;
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
    if (DATA_PV_GET()->expr && DATA_PV_GET()->write_count > 1) {
        DATA_PV_GET()->expr = NULL;
    }

    TRAVchildren(node);

    return node;
}

node_st *PVfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    if (DATA_PV_GET()->expr && DATA_PV_GET()->write_count > 1) {
        DATA_PV_GET()->expr = NULL;
    }

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
