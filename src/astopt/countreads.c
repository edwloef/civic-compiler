#include "ccn/ccn.h"

static void CRreset(void) {
    for (int i = 0; i < DATA_CR_GET()->vartable->len; i++) {
        vartable_entry *e = &DATA_CR_GET()->vartable->buf[i];
        e->read_count = (e->exported || e->external);
    }
}

void CRinit(void) {}
void CRfini(void) {}

node_st *CRprogram(node_st *node) {
    DATA_CR_GET()->vartable = PROGRAM_VARTABLE(node);

    CRreset();

    TRAVchildren(node);

    return node;
}

node_st *CRfundecl(node_st *node) {
    DATA_CR_GET()->vartable = FUNDECL_VARTABLE(node);

    CRreset();

    TRAVchildren(node);

    DATA_CR_GET()->vartable = DATA_CR_GET()->vartable->parent;

    return node;
}

node_st *CRvarref(node_st *node) {
    TRAVchildren(node);

    if (!VARREF_WRITE(node) || VARREF_EXPRS(node) != NULL) {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_CR_GET()->vartable, r);
        e->read_count++;
    }

    return node;
}
