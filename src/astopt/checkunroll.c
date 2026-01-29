#include "ccn/ccn.h"

void CUinit(void) {
    DATA_CU_GET()->can_unroll = true;
}
void CUfini(void) {}

node_st *CUifelse(node_st *node) {
    if (!DATA_CU_GET()->can_unroll) {
        return node;
    }

    node_st *ref = DATA_CU_GET()->ref;
    funtable *funtable = DATA_CU_GET()->funtable;
    vartable *vartable = DATA_CU_GET()->vartable;

    TRAVpush(TRAV_CUS);

    DATA_CUS_GET()->ref = ref;
    DATA_CUS_GET()->funtable = funtable;
    DATA_CUS_GET()->vartable = vartable;

    TRAVdo(node);

    bool can_unroll = DATA_CUS_GET()->can_unroll;

    TRAVpop();

    DATA_CU_GET()->can_unroll = can_unroll;

    return node;
}

node_st *CUdowhile(node_st *node) {
    if (!DATA_CU_GET()->can_unroll) {
        return node;
    }

    node_st *ref = DATA_CU_GET()->ref;
    funtable *funtable = DATA_CU_GET()->funtable;
    vartable *vartable = DATA_CU_GET()->vartable;

    TRAVpush(TRAV_CUS);

    DATA_CUS_GET()->ref = ref;
    DATA_CUS_GET()->funtable = funtable;
    DATA_CUS_GET()->vartable = vartable;

    TRAVdo(node);

    bool can_unroll = DATA_CUS_GET()->can_unroll;

    TRAVpop();

    DATA_CU_GET()->can_unroll = can_unroll;

    return node;
}

node_st *CUassign(node_st *node) {
    if (!DATA_CU_GET()->can_unroll) {
        return node;
    }

    TRAVchildren(node);

    node_st *ref = ASSIGN_REF(node);
    node_st *expr = ASSIGN_EXPR(node);

    if (VARREF_N(ref) == VARREF_N(DATA_CU_GET()->ref) &&
        VARREF_L(ref) == VARREF_L(DATA_CU_GET()->ref)) {
        if (NODE_TYPE(expr) == NT_BINOP &&
            (BINOP_OP(expr) == BO_add || BINOP_OP(expr) == BO_sub) &&
            NODE_TYPE(BINOP_LEFT(expr)) == NT_INT &&
            NODE_TYPE(BINOP_RIGHT(expr)) == NT_VARREF &&
            (VARREF_N(BINOP_RIGHT(expr)) == VARREF_N(DATA_CU_GET()->ref) &&
             VARREF_L(BINOP_RIGHT(expr)) == VARREF_L(DATA_CU_GET()->ref))) {
            if (BINOP_OP(expr) == BO_add) {
                DATA_CU_GET()->step += INT_VAL(BINOP_LEFT(expr));
            } else {
                DATA_CU_GET()->step -= INT_VAL(BINOP_LEFT(expr));
            }
        } else {
            DATA_CU_GET()->can_unroll = false;
        }
    }

    return node;
}

node_st *CUcall(node_st *node) {
    if (!DATA_CU_GET()->can_unroll) {
        return node;
    }

    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_CU_GET()->funtable, r);

    if (CALL_N(node) <= VARREF_N(DATA_CU_GET()->ref) &&
        CALL_N(node) + e->scalar_write_capture > VARREF_N(DATA_CU_GET()->ref)) {
        vartable_ref r = {VARREF_N(DATA_CU_GET()->ref),
                          VARREF_L(DATA_CU_GET()->ref)};
        vartable_entry *e = vartable_get(DATA_CU_GET()->vartable, r);
        if (e->escapes && e->write_count > 1) {
            DATA_CU_GET()->can_unroll = false;
        }
    }

    return node;
}
