#include "ccn/ccn.h"

void CCinit(void) {}
void CCfini(void) {}

node_st *CCprogram(node_st *node) {
    for (int i = 0; i < PROGRAM_FUNTABLE(node)->len; i++) {
        funtable_entry *e = &PROGRAM_FUNTABLE(node)->buf[i];
        if (e->exported) {
            e->call_count = 1;
        } else {
            e->call_count = 0;
        }
    }

    DATA_CC_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *CCfunbody(node_st *node) {
    for (int i = 0; i < FUNBODY_FUNTABLE(node)->len; i++) {
        funtable_entry *e = &FUNBODY_FUNTABLE(node)->buf[i];
        e->call_count = 0;
    }

    DATA_CC_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_CC_GET()->funtable = DATA_CC_GET()->funtable->parent;

    return node;
}

node_st *CCcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_CC_GET()->funtable, r);
    e->call_count++;

    return node;
}
