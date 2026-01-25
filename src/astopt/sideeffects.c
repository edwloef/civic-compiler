#include "ccn/ccn.h"

void AOSFinit(void) {}
void AOSFfini(void) {}

node_st *AOSFprogram(node_st *node) {
    DATA_AOSF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOSFfundecl(node_st *node) {
    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOSF_GET()->funtable, r);
    e->side_effects = e->write_capture > 0;

    for (int i = 0; !e->side_effects && i < FUNDECL_VARTABLE(node)->len; i++) {
        vartable_entry *ve = &FUNDECL_VARTABLE(node)->buf[i];
        e->side_effects |= ve->param && ve->write_count > 1 && ve->ty.len > 0;
    }

    TRAVchildren(node);

    return node;
}

node_st *AOSFfunbody(node_st *node) {
    DATA_AOSF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOSF_GET()->funtable = DATA_AOSF_GET()->funtable->parent;

    return node;
}
