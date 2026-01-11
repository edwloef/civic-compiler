#include "ccn/ccn.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void OCTCTinit(void) {}

void OCTCTfini(void) {}

node_st *OCTCTprogram(node_st *node) {
    DATA_OCTCT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_OCTCT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *OCTCTexprs(node_st *node) {
    TRAVchildren(node);

    EXPRS_TRANSP(node) = EXPR_TRANSP(EXPRS_EXPR(node)) &&
                         (!EXPRS_NEXT(node) || EXPRS_TRANSP(EXPRS_NEXT(node)));

    return node;
}

node_st *OCTCTarrexprs(node_st *node) {
    TRAVchildren(node);

    ARREXPRS_TRANSP(node) =
        ARREXPR_TRANSP(ARREXPRS_EXPR(node)) &&
        (!ARREXPRS_NEXT(node) || ARREXPRS_TRANSP(ARREXPRS_NEXT(node)));

    return node;
}

node_st *OCTCTfundecl(node_st *node) {
    int prev = DATA_OCTCT_GET()->min_nesting_level;
    DATA_OCTCT_GET()->min_nesting_level = DATA_OCTCT_GET()->nesting_level + 1;
    DATA_OCTCT_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);
    int min_nesting_level = DATA_OCTCT_GET()->min_nesting_level;

    DATA_OCTCT_GET()->min_nesting_level = MIN(prev, min_nesting_level);
    DATA_OCTCT_GET()->vartable = DATA_OCTCT_GET()->vartable->parent;

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_OCTCT_GET()->funtable, r);
    if (min_nesting_level != e->min_nesting_level) {
        e->min_nesting_level = min_nesting_level;
        CCNcycleNotify();
    }

    return node;
}

node_st *OCTCTfunbody(node_st *node) {
    DATA_OCTCT_GET()->nesting_level++;
    DATA_OCTCT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_OCTCT_GET()->nesting_level--;
    DATA_OCTCT_GET()->funtable = DATA_OCTCT_GET()->funtable->parent;

    return node;
}

node_st *OCTCTassign(node_st *node) {
    TRAVchildren(node);

    DATA_OCTCT_GET()->min_nesting_level =
        MIN(DATA_OCTCT_GET()->min_nesting_level,
            DATA_OCTCT_GET()->nesting_level - VARREF_N(ASSIGN_REF(node)));

    return node;
}

node_st *OCTCTmonop(node_st *node) {
    TRAVchildren(node);
    MONOP_TRANSP(node) = EXPR_TRANSP(MONOP_EXPR(node));
    return node;
}

node_st *OCTCTbinop(node_st *node) {
    TRAVchildren(node);
    BINOP_TRANSP(node) =
        EXPR_TRANSP(BINOP_LEFT(node)) && EXPR_TRANSP(BINOP_RIGHT(node));
    return node;
}

node_st *OCTCTcast(node_st *node) {
    TRAVchildren(node);
    CAST_TRANSP(node) = EXPR_TRANSP(CAST_EXPR(node));
    return node;
}

node_st *OCTCTcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    bool fun_transp = funtable_transp(DATA_OCTCT_GET()->funtable, r);

    CALL_TRANSP(node) =
        fun_transp && (!CALL_EXPRS(node) || EXPRS_TRANSP(CALL_EXPRS(node)));

    if (!fun_transp) {
        DATA_OCTCT_GET()->min_nesting_level =
            MIN(DATA_OCTCT_GET()->min_nesting_level,
                DATA_OCTCT_GET()->nesting_level - CALL_N(node));
    }

    return node;
}

node_st *OCTCTvarref(node_st *node) {
    TRAVchildren(node);
    VARREF_TRANSP(node) =
        !VARREF_EXPRS(node) || EXPRS_TRANSP(VARREF_EXPRS(node));
    return node;
}

node_st *OCTCTint(node_st *node) {
    INT_TRANSP(node) = true;
    return node;
}

node_st *OCTCTfloat(node_st *node) {
    FLOAT_TRANSP(node) = true;
    return node;
}

node_st *OCTCTbool(node_st *node) {
    BOOL_TRANSP(node) = true;
    return node;
}
