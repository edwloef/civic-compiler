#include "ccn/ccn.h"
#include "ccngen/trav.h"

void CPLinit(void) {
    DATA_CPL_GET()->can_prop = true;
}
void CPLfini(void) {}

node_st *CPLstmts(node_st *node) {
    TRAVstmt(node);

    if (DATA_CPL_GET()->can_prop) {
        TRAVnext(node);
    }

    return node;
}

node_st *CPLcall(node_st *node) {
    TRAVchildren(node);

    if (CALL_N(node) <= VARREF_N(DATA_CPL_GET()->ref)) {
        vartable_ref r = {VARREF_N(DATA_CPL_GET()->ref),
                          VARREF_L(DATA_CPL_GET()->ref)};
        vartable_entry *e = vartable_get(DATA_CPL_GET()->vartable, r);
        if (e->escapes && e->write_count > 1) {
            DATA_CPL_GET()->can_prop = false;
        }
    }

    if (NODE_TYPE(DATA_CPL_GET()->expr) == NT_VARREF &&
        CALL_N(node) <= VARREF_N(DATA_CPL_GET()->expr)) {
        vartable_ref r = {VARREF_N(DATA_CPL_GET()->expr),
                          VARREF_L(DATA_CPL_GET()->expr)};
        vartable_entry *e = vartable_get(DATA_CPL_GET()->vartable, r);
        if (e->escapes) {
            DATA_CPL_GET()->can_prop = false;
        }
    }

    return node;
}

node_st *CPLvarref(node_st *node) {
    TRAVchildren(node);

    if (VARREF_WRITE(node)) {
        if (VARREF_N(node) == VARREF_N(DATA_CPL_GET()->ref) &&
            VARREF_L(node) == VARREF_L(DATA_CPL_GET()->ref)) {
            DATA_CPL_GET()->can_prop = false;
        }

        if (NODE_TYPE(DATA_CPL_GET()->expr) == NT_VARREF &&
            VARREF_N(node) == VARREF_N(DATA_CPL_GET()->expr) &&
            VARREF_L(node) == VARREF_L(DATA_CPL_GET()->expr)) {
            DATA_CPL_GET()->can_prop = false;
        }
    }

    return node;
}
