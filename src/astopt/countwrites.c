#include "ccn/ccn.h"

static void CWreset(void) {
    for (int i = 0; i < DATA_CW_GET()->vartable->len; i++) {
        vartable_entry *e = &DATA_CW_GET()->vartable->buf[i];
        e->write_count = 2 * (e->external || e->exported) + e->param;
    }
}

void CWinit(void) {}
void CWfini(void) {}

node_st *CWprogram(node_st *node) {
    DATA_CW_GET()->vartable = PROGRAM_VARTABLE(node);

    CWreset();

    TRAVchildren(node);

    return node;
}

node_st *CWfundecl(node_st *node) {
    DATA_CW_GET()->vartable = FUNDECL_VARTABLE(node);

    CWreset();

    TRAVchildren(node);

    DATA_CW_GET()->vartable = DATA_CW_GET()->vartable->parent;

    return node;
}

node_st *CWvarref(node_st *node) {
    TRAVchildren(node);

    if (VARREF_WRITE(node)) {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_CW_GET()->vartable, r);
        e->write_count++;
    }

    return node;
}