#include "ccn/ccn.h"

static funtable *AOSFshrink_funtable(void) {
    funtable *funtable = funtable_new(DATA_AOST_GET()->new_funtable);

    for (int i = 0; i < DATA_AOST_GET()->funtable->len; i++) {
        funtable_entry *e = &DATA_AOST_GET()->funtable->buf[i];
        if (e->call_count > 0) {
            e->new_l = funtable_push(funtable, *e).l;
        } else {
            funtable_entry_free(*e);
        }
    }

    return funtable;
}

static vartable *AOSFshrink_vartable(void) {
    vartable *vartable = vartable_new(DATA_AOST_GET()->new_vartable);

    for (int i = 0; i < DATA_AOST_GET()->vartable->len; i++) {
        vartable_entry *e = &DATA_AOST_GET()->vartable->buf[i];
        if (e->read_count > 0 || e->write_count > 0) {
            e->new_l = vartable_push(vartable, *e).l;
        } else {
            vartable_entry_free(*e);
        }
    }

    return vartable;
}

void AOSTinit(void) {}
void AOSTfini(void) {}

node_st *AOSTprogram(node_st *node) {
    DATA_AOST_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AOST_GET()->new_funtable = AOSFshrink_funtable();

    DATA_AOST_GET()->vartable = PROGRAM_VARTABLE(node);
    DATA_AOST_GET()->new_vartable = AOSFshrink_vartable();

    TRAVchildren(node);

    funtable_shallow_free(PROGRAM_FUNTABLE(node));
    PROGRAM_FUNTABLE(node) = DATA_AOST_GET()->new_funtable;

    vartable_shallow_free(PROGRAM_VARTABLE(node));
    PROGRAM_VARTABLE(node) = DATA_AOST_GET()->new_vartable;

    return node;
}

node_st *AOSTfundecl(node_st *node) {
    DATA_AOST_GET()->vartable = FUNDECL_VARTABLE(node);
    vartable *prev = DATA_AOST_GET()->new_vartable;
    DATA_AOST_GET()->new_vartable = AOSFshrink_vartable();

    TRAVchildren(node);

    DATA_AOST_GET()->vartable = DATA_AOST_GET()->vartable->parent;

    vartable_shallow_free(FUNDECL_VARTABLE(node));
    FUNDECL_VARTABLE(node) = DATA_AOST_GET()->new_vartable;
    DATA_AOST_GET()->new_vartable = prev;

    funtable_ref r = {0, FUNDECL_L(node)};
    FUNDECL_L(node) = funtable_get(DATA_AOST_GET()->funtable, r)->new_l;

    return node;
}

node_st *AOSTfunbody(node_st *node) {
    DATA_AOST_GET()->funtable = FUNBODY_FUNTABLE(node);
    funtable *prev = DATA_AOST_GET()->new_funtable;
    DATA_AOST_GET()->new_funtable = AOSFshrink_funtable();

    TRAVchildren(node);

    DATA_AOST_GET()->funtable = DATA_AOST_GET()->funtable->parent;

    funtable_shallow_free(FUNBODY_FUNTABLE(node));
    FUNBODY_FUNTABLE(node) = DATA_AOST_GET()->new_funtable;
    DATA_AOST_GET()->new_funtable = prev;

    return node;
}

node_st *AOSTcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOST_GET()->funtable, r);

    CALL_L(node) = e->new_l;

    return node;
}

node_st *AOSTvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {VARREF_N(node), VARREF_L(node)};
    vartable_entry *e = vartable_get(DATA_AOST_GET()->vartable, r);

    VARREF_L(node) = e->new_l;

    return node;
}
