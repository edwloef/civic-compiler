#include "ccn/ccn.h"
#include "palm/dbug.h"

node_st *AOCPstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_ASSIGN && !VARREF_EXPRS(ASSIGN_REF(stmt)) &&
        (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_INT ||
         NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_FLOAT ||
         NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_BOOL ||
         (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF &&
          !VARREF_EXPRS(ASSIGN_EXPR(stmt))))) {
        TRAVpush(TRAV_PV);

        DATA_PV_GET()->expr = ASSIGN_EXPR(stmt);
        DATA_PV_GET()->n = VARREF_N(ASSIGN_REF(stmt));
        DATA_PV_GET()->l = VARREF_L(ASSIGN_REF(stmt));

        STMTS_NEXT(node) = TRAVopt(STMTS_NEXT(node));

        TRAVpop();
    }

    return node;
}

void PVinit(void) {}
void PVfini(void) {}

node_st *PVassign(node_st *node) {
    if (DATA_PV_GET()->expr &&
        (NODE_TYPE(DATA_PV_GET()->expr) != NT_VARREF ||
         VARREF_N(DATA_PV_GET()->expr) != VARREF_N(ASSIGN_REF(node)) ||
         VARREF_L(DATA_PV_GET()->expr) != VARREF_L(ASSIGN_REF(node)))) {
        TRAVchildren(node);
    }

    return node;
}

node_st *PVifelse(node_st *node) {
    IFELSE_EXPR(node) = TRAVdo(IFELSE_EXPR(node));

    node_st *before = DATA_PV_GET()->expr;

    IFELSE_IF_BLOCK(node) = TRAVopt(IFELSE_IF_BLOCK(node));

    node_st *after = DATA_PV_GET()->expr;
    DATA_PV_GET()->expr = before;

    IFELSE_ELSE_BLOCK(node) = TRAVopt(IFELSE_ELSE_BLOCK(node));

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
    if (DATA_PV_GET()->expr &&
        (NODE_TYPE(DATA_PV_GET()->expr) != NT_VARREF ||
         ID_VAL(VARREF_ID(DATA_PV_GET()->expr))[0] == '_')) {
        TRAVchildren(node);
    }

    return node;
}

node_st *PVfor(node_st *node) {
    if (DATA_PV_GET()->expr &&
        (NODE_TYPE(DATA_PV_GET()->expr) != NT_VARREF ||
         ID_VAL(VARREF_ID(DATA_PV_GET()->expr))[0] == '_')) {
        TRAVchildren(node);
    }

    return node;
}

node_st *PVreturn(node_st *node) {
    TRAVchildren(node);

    return node;
}

node_st *PVcall(node_st *node) {
    TRAVchildren(node);

    if (DATA_PV_GET()->expr && NODE_TYPE(DATA_PV_GET()->expr) == NT_VARREF &&
        ID_VAL(VARREF_ID(DATA_PV_GET()->expr))[0] != '_') {
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
