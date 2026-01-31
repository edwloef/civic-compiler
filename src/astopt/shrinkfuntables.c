#include "ccn/ccn.h"

static funtable *CGSFshrink(void) {
    funtable *funtable = funtable_new(DATA_CGSF_GET()->new);

    for (int i = 0; i < DATA_CGSF_GET()->funtable->len; i++) {
        funtable_entry *e = &DATA_CGSF_GET()->funtable->buf[i];
        if (e->call_count > 0) {
            e->new_l = funtable_push(funtable, *e).l;
        } else {
            funtable_entry_free(*e);
        }
    }

    return funtable;
}

void CGSFinit(void) {}
void CGSFfini(void) {}

node_st *CGSFprogram(node_st *node) {
    DATA_CGSF_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_CGSF_GET()->new = CGSFshrink();

    TRAVchildren(node);

    funtable_shallow_free(PROGRAM_FUNTABLE(node));
    PROGRAM_FUNTABLE(node) = DATA_CGSF_GET()->new;

    return node;
}

node_st *CGSFfundecl(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {0, FUNDECL_L(node)};
    FUNDECL_L(node) = funtable_get(DATA_CGSF_GET()->funtable, r)->new_l;

    return node;
}

node_st *CGSFfunbody(node_st *node) {
    DATA_CGSF_GET()->funtable = FUNBODY_FUNTABLE(node);
    funtable *prev = DATA_CGSF_GET()->new;
    DATA_CGSF_GET()->new = CGSFshrink();

    TRAVchildren(node);

    DATA_CGSF_GET()->funtable = DATA_CGSF_GET()->funtable->parent;

    funtable_shallow_free(FUNBODY_FUNTABLE(node));
    FUNBODY_FUNTABLE(node) = DATA_CGSF_GET()->new;
    DATA_CGSF_GET()->new = prev;

    return node;
}

node_st *CGSFcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_CGSF_GET()->funtable, r);

    CALL_L(node) = e->new_l;

    return node;
}
