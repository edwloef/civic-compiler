#include "ccn/ccn.h"

void AOCWinit(void) {}
void AOCWfini(void) {}

node_st *AOCWprogram(node_st *node) {
    for (int i = 0; i < PROGRAM_VARTABLE(node)->len; i++) {
        vartable_entry *e = &PROGRAM_VARTABLE(node)->buf[i];
        if (e->external || e->exported) {
            e->write_count = 2;
        } else {
            e->write_count = 0;
        }
    }

    DATA_AOCW_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOCWfundecl(node_st *node) {
    for (int i = 0; i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *e = &FUNDECL_VARTABLE(node)->buf[i];
        if (e->param || e->loopvar) {
            e->write_count = 1;
        } else {
            e->write_count = 0;
        }
    }

    DATA_AOCW_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AOCW_GET()->vartable = DATA_AOCW_GET()->vartable->parent;

    return node;
}

node_st *AOCWvarref(node_st *node) {
    if (VARREF_WRITE(node)) {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_AOCW_GET()->vartable, r);
        e->write_count++;
    }

    return node;
}