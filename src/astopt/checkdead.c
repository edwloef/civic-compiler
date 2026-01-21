#include "ccn/ccn.h"
#include "ccngen/trav.h"

void CDinit(void) {
    DATA_CD_GET()->is_dead = true;
}
void CDfini(void) {}

node_st *CDstmts(node_st *node) {
    TRAVstmt(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_ASSIGN &&
        VARREF_N(ASSIGN_REF(stmt)) == DATA_CD_GET()->n &&
        VARREF_L(ASSIGN_REF(stmt)) == DATA_CD_GET()->l) {
        return node;
    }

    if (DATA_CD_GET()->is_dead) {
        TRAVnext(node);
    }

    return node;
}

node_st *CDcall(node_st *node) {
    TRAVchildren(node);

    DATA_CD_GET()->is_dead = false;

    return node;
}

node_st *CDvarref(node_st *node) {
    TRAVchildren(node);

    if (DATA_CD_GET()->n == VARREF_N(node) &&
        DATA_CD_GET()->l == VARREF_L(node) && !VARREF_WRITE(node)) {
        DATA_CD_GET()->is_dead = false;
    }

    return node;
}
