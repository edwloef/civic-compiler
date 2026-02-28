#include <math.h>

#include "ccn/ccn.h"
#include "globals/globals.h"

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
            vartable_entry ne = *e;
            ne.new_l = i;
            e->new_l = vartable_push(vartable, ne).l;
        } else {
            vartable_entry_free(*e);
        }
    }

    for (int i = 0; i < vartable->len; i++) {
        vartable_entry *ie = &vartable->buf[i];
        vartable_entry *max_je = ie;

        if (ie->param) {
            continue;
        }

        for (int j = i + 1; j < vartable->len; j++) {
            vartable_entry *je = &vartable->buf[j];
            if (max_je->read_count < je->read_count) {
                max_je = je;
            }
        }

        if (ie == max_je) {
            continue;
        }

        vartable_entry tmp_e = *ie;
        *ie = *max_je;
        *max_je = tmp_e;

        vartable_entry *prev_ie = &DATA_AOST_GET()->vartable->buf[ie->new_l];
        vartable_entry *prev_max_je =
            &DATA_AOST_GET()->vartable->buf[max_je->new_l];

        int tmp_l = prev_ie->new_l;
        prev_ie->new_l = prev_max_je->new_l;
        prev_max_je->new_l = tmp_l;
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

node_st *AOSTbinop(node_st *node) {
    TRAVchildren(node);

    node_st *left = BINOP_LEFT(node);
    if (BINOP_OP(node) == BO_add && NODE_TYPE(left) == NT_FLOAT &&
        (double_biteq(FLOAT_VAL(left), -0.0) ||
         double_biteq(FLOAT_VAL(left), -1.0))) {
        BINOP_OP(node) = BO_sub;
        node_st *tmp = BINOP_LEFT(node);
        FLOAT_VAL(tmp) *= -1.0;
        BINOP_LEFT(node) = BINOP_RIGHT(node);
        BINOP_RIGHT(node) = tmp;
    }

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

node_st *AOSTfloat(node_st *node) {
    if (isnan(FLOAT_VAL(node))) {
        FLOAT_VAL(node) = NAN;
    } else if (double_biteq(FLOAT_VAL(node), -0.0) && !globals.signed_zeros) {
        FLOAT_VAL(node) = 0.0;
    }

    return node;
}
