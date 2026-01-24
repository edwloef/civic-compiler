#include "ccn/ccn.h"

void CRinit(void) {}
void CRfini(void) {}

node_st *CRprogram(node_st *node) {
    for (int i = 0; i < PROGRAM_VARTABLE(node)->len; i++) {
        vartable_entry *e = &PROGRAM_VARTABLE(node)->buf[i];
        e->read_count = (e->exported || e->external);
    }

    DATA_CR_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *CRfundecl(node_st *node) {
    for (int i = 0; i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *e = &FUNDECL_VARTABLE(node)->buf[i];
        e->read_count = 0;
    }

    DATA_CR_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_CR_GET()->vartable = DATA_CR_GET()->vartable->parent;

    return node;
}

node_st *CRvarref(node_st *node) {
    TRAVchildren(node);

    if (!VARREF_WRITE(node)) {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_CR_GET()->vartable, r);
        e->read_count++;
    }

    return node;
}