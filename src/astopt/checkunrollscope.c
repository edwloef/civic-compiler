#include "ccn/ccn.h"

void CUSinit(void) {
    DATA_CUS_GET()->can_unroll = true;
}
void CUSfini(void) {}

node_st *CUSassign(node_st *node) {
    if (!DATA_CUS_GET()->can_unroll) {
        return node;
    }

    TRAVchildren(node);

    node_st *ref = ASSIGN_REF(node);

    if (VARREF_N(ref) == VARREF_N(DATA_CUS_GET()->ref) &&
        VARREF_L(ref) == VARREF_L(DATA_CUS_GET()->ref)) {
        DATA_CUS_GET()->can_unroll = false;
    }

    return node;
}

node_st *CUScall(node_st *node) {
    if (!DATA_CUS_GET()->can_unroll) {
        return node;
    }

    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_CUS_GET()->funtable, r);

    if (CALL_N(node) <= VARREF_N(DATA_CUS_GET()->ref) &&
        CALL_N(node) + e->scalar_write_capture >
            VARREF_N(DATA_CUS_GET()->ref)) {
        vartable_ref r = {VARREF_N(DATA_CUS_GET()->ref),
                          VARREF_L(DATA_CUS_GET()->ref)};
        vartable_entry *e = vartable_get(DATA_CUS_GET()->vartable, r);
        if (e->write_escapes && e->write_count > 1) {
            DATA_CUS_GET()->can_unroll = false;
        }
    }

    return node;
}
