#include "ccn/ccn.h"

void Einit(void) {}
void Efini(void) {}

node_st *Eprogram(node_st *node) {
    for (int i = 0; i < PROGRAM_VARTABLE(node)->len; i++) {
        vartable_entry *e = &PROGRAM_VARTABLE(node)->buf[i];
        e->escapes = false;
    }

    DATA_E_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *Efundecl(node_st *node) {
    for (int i = 0; i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *e = &FUNDECL_VARTABLE(node)->buf[i];
        e->escapes = false;
    }

    DATA_E_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_E_GET()->vartable = DATA_E_GET()->vartable->parent;

    return node;
}

node_st *Evarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {VARREF_N(node), VARREF_L(node)};
    vartable_entry *e = vartable_get(DATA_E_GET()->vartable, r);
    e->escapes |= r.n > 0;

    return node;
}