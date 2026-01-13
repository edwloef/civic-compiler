#include "ccn/ccn.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void AOTinit(void) {}

void AOTfini(void) {}

node_st *AOTprogram(node_st *node) {
    DATA_AOT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AOT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOTarrexprs(node_st *node) {
    TRAVchildren(node);

    ARREXPRS_TRANSP(node) =
        ARREXPR_TRANSP(ARREXPRS_EXPR(node)) &&
        (!ARREXPRS_NEXT(node) || ARREXPRS_TRANSP(ARREXPRS_NEXT(node)));

    return node;
}

node_st *AOTfundecl(node_st *node) {
    if (!FUNDECL_EXTERNAL(node)) {
        int prev = DATA_AOT_GET()->min_level;
        DATA_AOT_GET()->vartable = FUNDECL_VARTABLE(node);
        DATA_AOT_GET()->min_level = DATA_AOT_GET()->level + 1;

        TRAVchildren(node);
        int min_level = DATA_AOT_GET()->min_level;

        DATA_AOT_GET()->vartable = DATA_AOT_GET()->vartable->parent;
        DATA_AOT_GET()->min_level = prev;

        funtable_ref r = {0, FUNDECL_L(node)};
        funtable_entry *e = funtable_get(DATA_AOT_GET()->funtable, r);
        if (min_level != e->min_level) {
            e->min_level = min_level;
            e->transp = min_level > DATA_AOT_GET()->level;
            CCNcycleNotify();
        }
    }

    return node;
}

node_st *AOTfunbody(node_st *node) {
    DATA_AOT_GET()->level++;
    DATA_AOT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOT_GET()->level--;
    DATA_AOT_GET()->funtable = DATA_AOT_GET()->funtable->parent;

    return node;
}

node_st *AOTassign(node_st *node) {
    TRAVchildren(node);

    DATA_AOT_GET()->min_level =
        MIN(DATA_AOT_GET()->min_level,
            DATA_AOT_GET()->level - VARREF_N(ASSIGN_REF(node)));

    return node;
}

node_st *AOTmonop(node_st *node) {
    TRAVchildren(node);

    MONOP_TRANSP(node) = EXPR_TRANSP(MONOP_EXPR(node));

    return node;
}

node_st *AOTbinop(node_st *node) {
    TRAVchildren(node);

    BINOP_TRANSP(node) =
        EXPR_TRANSP(BINOP_LEFT(node)) && EXPR_TRANSP(BINOP_RIGHT(node));

    return node;
}

node_st *AOTcast(node_st *node) {
    TRAVchildren(node);

    CAST_TRANSP(node) = EXPR_TRANSP(CAST_EXPR(node));

    return node;
}

node_st *AOTcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOT_GET()->funtable, r);

    CALL_TRANSP(node) = e->transp;

    if (!e->transp) {
        DATA_AOT_GET()->min_level =
            MIN(DATA_AOT_GET()->min_level, e->min_level);
    }

    return node;
}

node_st *AOTvarref(node_st *node) {
    TRAVchildren(node);

    VARREF_TRANSP(node) = true;

    return node;
}

node_st *AOTint(node_st *node) {
    INT_TRANSP(node) = true;

    return node;
}

node_st *AOTfloat(node_st *node) {
    FLOAT_TRANSP(node) = true;

    return node;
}

node_st *AOTbool(node_st *node) {
    BOOL_TRANSP(node) = true;

    return node;
}
