#include "ccn/ccn.h"

void AOFSFinit(void) {}
void AOFSFfini(void) {}

node_st *AOFSFprogram(node_st *node) {
    DATA_AOFSF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOFSFfundecl(node_st *node) {
    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOFSF_GET()->funtable, r);
    e->side_effects = e->write_capture > 0;

    for (int i = 0; !e->side_effects && i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *ve = &FUNDECL_VARTABLE(node)->buf[i];
        e->side_effects |= ve->param && ve->write_count > 1 && ve->ty.len > 0;
    }

    TRAVchildren(node);

    return node;
}

node_st *AOFSFfunbody(node_st *node) {
    DATA_AOFSF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOFSF_GET()->funtable = DATA_AOFSF_GET()->funtable->parent;

    return node;
}
