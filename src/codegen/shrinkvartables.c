#include "ccn/ccn.h"

static vartable *CGSVshrink(void) {
    vartable *vartable = vartable_new(DATA_CGSV_GET()->new);

    for (int i = 0; i < DATA_CGSV_GET()->vartable->len; i++) {
        vartable_entry *e = &DATA_CGSV_GET()->vartable->buf[i];
        if (e->read_count > 0 || e->write_count > 0) {
            vartable_entry ne = *e;
            ne.ty = vartype_copy(&e->ty);
            e->new_l = vartable_push(vartable, ne).l;
        }
    }

    return vartable;
}

void CGSVinit(void) {}
void CGSVfini(void) {}

node_st *CGSVprogram(node_st *node) {
    DATA_CGSV_GET()->vartable = PROGRAM_VARTABLE(node);
    DATA_CGSV_GET()->new = CGSVshrink();

    TRAVchildren(node);

    vartable_free(PROGRAM_VARTABLE(node));
    PROGRAM_VARTABLE(node) = DATA_CGSV_GET()->new;

    return node;
}

node_st *CGSVfundecl(node_st *node) {
    DATA_CGSV_GET()->vartable = FUNDECL_VARTABLE(node);
    vartable *prev = DATA_CGSV_GET()->new;
    DATA_CGSV_GET()->new = CGSVshrink();

    TRAVchildren(node);

    DATA_CGSV_GET()->vartable = DATA_CGSV_GET()->vartable->parent;

    vartable_free(FUNDECL_VARTABLE(node));
    FUNDECL_VARTABLE(node) = DATA_CGSV_GET()->new;
    DATA_CGSV_GET()->new = prev;

    return node;
}

node_st *CGSVvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {VARREF_N(node), VARREF_L(node)};
    vartable_entry *e = vartable_get(DATA_CGSV_GET()->vartable, r);

    VARREF_L(node) = e->new_l;

    return node;
}
