#include "ccn/ccn.h"

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
        } else if (e->param || e->loopvar) {
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