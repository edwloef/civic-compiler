#include "ccn/ccn.h"

void AOESFinit(void) {}
void AOESFfini(void) {}

node_st *AOESFprogram(node_st *node) {
    DATA_AOESF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOESFfunbody(node_st *node) {
    DATA_AOESF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOESF_GET()->funtable = DATA_AOESF_GET()->funtable->parent;

    return node;
}

node_st *AOESFexprs(node_st *node) {
    TRAVchildren(node);

    EXPRS_SIDE_EFFECTS(node) =
        EXPR_SIDE_EFFECTS(EXPRS_EXPR(node)) ||
        (EXPRS_NEXT(node) && EXPRS_SIDE_EFFECTS(EXPRS_NEXT(node)));

    return node;
}

node_st *AOESFcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOESF_GET()->funtable, r);
    CALL_SIDE_EFFECTS(node) =
        e->side_effects ||
        (CALL_EXPRS(node) && EXPRS_SIDE_EFFECTS(CALL_EXPRS(node)));

    return node;
}
