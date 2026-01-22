#include "ccn/ccn.h"
#include "ccngen/trav.h"

void CDinit(void) {
    DATA_CD_GET()->assign_is_dead = true;
}
void CDfini(void) {}

node_st *CDstmts(node_st *node) {
    TRAVstmt(node);

    if (DATA_CD_GET()->ref_is_dead) {
        return node;
    }

    if (DATA_CD_GET()->assign_is_dead) {
        TRAVnext(node);
    }

    return node;
}

node_st *CDifelse(node_st *node) {
    TRAVexpr(node);

    bool prev = DATA_CD_GET()->ref_is_dead;

    TRAVif_block(node);

    bool ref_is_dead = DATA_CD_GET()->ref_is_dead;
    DATA_CD_GET()->ref_is_dead = prev;

    TRAVelse_block(node);

    DATA_CD_GET()->ref_is_dead &= ref_is_dead;

    return node;
}

node_st *CDfor(node_st *node) {
    TRAVchildren(node);

    DATA_CD_GET()->ref_is_dead = false;

    return node;
}

node_st *CDcall(node_st *node) {
    TRAVchildren(node);

    if (CALL_N(node) == 0) {
        DATA_CD_GET()->assign_is_dead = false;
    }

    return node;
}

node_st *CDvarref(node_st *node) {
    TRAVchildren(node);

    if (node == DATA_CD_GET()->ref) {
        DATA_CD_GET()->seen = true;
    } else if (VARREF_N(node) == 0 &&
               VARREF_L(node) == VARREF_L(DATA_CD_GET()->ref)) {
        if (VARREF_WRITE(node)) {
            if (DATA_CD_GET()->seen) {
                DATA_CD_GET()->ref_is_dead = true;
            }
        } else {
            DATA_CD_GET()->assign_is_dead = false;
        }
    }

    return node;
}
