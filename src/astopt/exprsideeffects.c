#include "ccn/ccn.h"
#include "utils.h"

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

node_st *AOESFarrexprs(node_st *node) {
    TRAVchildren(node);

    ARREXPRS_SIDE_EFFECTS(node) =
        EXPR_SIDE_EFFECTS(ARREXPRS_EXPR(node)) ||
        (ARREXPRS_NEXT(node) && ARREXPRS_SIDE_EFFECTS(ARREXPRS_NEXT(node)));

    return node;
}

node_st *AOESFmonop(node_st *node) {
    TRAVchildren(node);

    MONOP_SIDE_EFFECTS(node) = EXPR_SIDE_EFFECTS(MONOP_EXPR(node));

    return node;
}

node_st *AOESFbinop(node_st *node) {
    TRAVchildren(node);

    BINOP_SIDE_EFFECTS(node) = EXPR_SIDE_EFFECTS(BINOP_LEFT(node)) ||
                               EXPR_SIDE_EFFECTS(BINOP_RIGHT(node));

    return node;
}

node_st *AOESFcast(node_st *node) {
    TRAVchildren(node);

    CAST_SIDE_EFFECTS(node) = EXPR_SIDE_EFFECTS(CAST_EXPR(node));

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

node_st *AOESFvarref(node_st *node) {
    TRAVchildren(node);

    VARREF_SIDE_EFFECTS(node) =
        VARREF_EXPRS(node) && EXPRS_SIDE_EFFECTS(VARREF_EXPRS(node));

    return node;
}

node_st *AOESFint(node_st *node) {
    return node;
}

node_st *AOESFfloat(node_st *node) {
    return node;
}

node_st *AOESFbool(node_st *node) {
    return node;
}

node_st *AOESFmalloc(node_st *node) {
    TRAVchildren(node);

    MALLOC_SIDE_EFFECTS(node) = EXPRS_SIDE_EFFECTS(MALLOC_EXPRS(node));

    return node;
}

node_st *AOESFconstref(node_st *node) {
    OUT_OF_LIFETIME();
}
