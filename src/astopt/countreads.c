#include "ccn/ccn.h"

void AOCRinit(void) {}
void AOCRfini(void) {}

node_st *AOCRprogram(node_st *node) {
    DATA_AOCR_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOCRfundecl(node_st *node) {
    for (int i = 0; i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *e = &FUNDECL_VARTABLE(node)->buf[i];
        e->read_count = 0;
    }

    DATA_AOCR_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AOCR_GET()->vartable = DATA_AOCR_GET()->vartable->parent;

    return node;
}

node_st *AOCRvarref(node_st *node) {
    if (!VARREF_WRITE(node)) {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_AOCR_GET()->vartable, r);
        e->read_count++;
    }

    return node;
}