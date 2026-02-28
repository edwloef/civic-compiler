#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "utils.h"

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

node_st *CDassign(node_st *node) {
    TRAVexpr(node);
    TRAVref(node);

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

node_st *CDwhile(node_st *node) {
    OUT_OF_LIFETIME();
}

node_st *CDdowhile(node_st *node) {
    NOOP();
}

node_st *CDfor(node_st *node) {
    OUT_OF_LIFETIME();
}

node_st *CDreturn(node_st *node) {
    NOOP();
}

node_st *CDcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_CD_GET()->funtable, r);

    if (CALL_N(node) <= VARREF_N(DATA_CD_GET()->ref) &&
        CALL_N(node) + e->scalar_write_capture > VARREF_N(DATA_CD_GET()->ref)) {
        vartable_ref r = {VARREF_N(DATA_CD_GET()->ref),
                          VARREF_L(DATA_CD_GET()->ref)};
        vartable_entry *e = vartable_get(DATA_CD_GET()->vartable, r);
        if (e->read_escapes && !DATA_CD_GET()->ref_is_dead) {
            DATA_CD_GET()->assign_is_dead = false;
        }
    }

    return node;
}

node_st *CDvarref(node_st *node) {
    TRAVchildren(node);

    if (node == DATA_CD_GET()->ref) {
        DATA_CD_GET()->seen = true;
    } else if (DATA_CD_GET()->seen) {
        if (VARREF_N(node) == VARREF_N(DATA_CD_GET()->ref) &&
            VARREF_L(node) == VARREF_L(DATA_CD_GET()->ref)) {
            if (VARREF_WRITE(node)) {
                DATA_CD_GET()->ref_is_dead = true;
            } else if (!DATA_CD_GET()->ref_is_dead) {
                DATA_CD_GET()->assign_is_dead = false;
            }
        }
    }

    return node;
}
