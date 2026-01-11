#include "ccn/ccn.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void OCTTinit(void) {}

void OCTTfini(void) {}

node_st *OCTTprogram(node_st *node) {
    DATA_OCTT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_OCTT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *OCTTexprs(node_st *node) {
    TRAVchildren(node);

    EXPRS_TRANSP(node) = EXPR_TRANSP(EXPRS_EXPR(node)) &&
                         (!EXPRS_NEXT(node) || EXPRS_TRANSP(EXPRS_NEXT(node)));

    return node;
}

node_st *OCTTarrexprs(node_st *node) {
    TRAVchildren(node);

    ARREXPRS_TRANSP(node) =
        ARREXPR_TRANSP(ARREXPRS_EXPR(node)) &&
        (!ARREXPRS_NEXT(node) || ARREXPRS_TRANSP(ARREXPRS_NEXT(node)));

    return node;
}

node_st *OCTTfundecl(node_st *node) {
    int prev = DATA_OCTT_GET()->min_nesting_level;
    DATA_OCTT_GET()->min_nesting_level = DATA_OCTT_GET()->nesting_level;
    DATA_OCTT_GET()->vartable = FUNDECL_VARTABLE(node);

    if (!FUNDECL_EXTERNAL(node)) {
        DATA_OCTT_GET()->min_nesting_level++;
        TRAVchildren(node);
    }

    int min_nesting_level = DATA_OCTT_GET()->min_nesting_level;
    DATA_OCTT_GET()->min_nesting_level = MIN(prev, min_nesting_level);
    DATA_OCTT_GET()->vartable = DATA_OCTT_GET()->vartable->parent;

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_OCTT_GET()->funtable, r);
    if (min_nesting_level != e->min_nesting_level) {
        e->min_nesting_level = min_nesting_level;
        CCNcycleNotify();
    }

    return node;
}

node_st *OCTTfunbody(node_st *node) {
    DATA_OCTT_GET()->nesting_level++;
    DATA_OCTT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_OCTT_GET()->nesting_level--;
    DATA_OCTT_GET()->funtable = DATA_OCTT_GET()->funtable->parent;

    return node;
}

node_st *OCTTassign(node_st *node) {
    TRAVchildren(node);

    DATA_OCTT_GET()->min_nesting_level =
        MIN(DATA_OCTT_GET()->min_nesting_level,
            DATA_OCTT_GET()->nesting_level - VARREF_N(ASSIGN_REF(node)));

    return node;
}

node_st *OCTTmonop(node_st *node) {
    TRAVchildren(node);

    MONOP_TRANSP(node) = EXPR_TRANSP(MONOP_EXPR(node));

    return node;
}

node_st *OCTTbinop(node_st *node) {
    TRAVchildren(node);

    BINOP_TRANSP(node) =
        EXPR_TRANSP(BINOP_LEFT(node)) && EXPR_TRANSP(BINOP_RIGHT(node));

    return node;
}

node_st *OCTTcast(node_st *node) {
    TRAVchildren(node);

    CAST_TRANSP(node) = EXPR_TRANSP(CAST_EXPR(node));

    return node;
}

node_st *OCTTcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    bool fun_transp = funtable_transp(DATA_OCTT_GET()->funtable, r);

    CALL_TRANSP(node) =
        fun_transp && (!CALL_EXPRS(node) || EXPRS_TRANSP(CALL_EXPRS(node)));

    if (!fun_transp) {
        DATA_OCTT_GET()->min_nesting_level =
            MIN(DATA_OCTT_GET()->min_nesting_level,
                DATA_OCTT_GET()->nesting_level - CALL_N(node));
    }

    return node;
}

node_st *OCTTvarref(node_st *node) {
    TRAVchildren(node);

    VARREF_TRANSP(node) =
        !VARREF_EXPRS(node) || EXPRS_TRANSP(VARREF_EXPRS(node));

    return node;
}

node_st *OCTTint(node_st *node) {
    INT_TRANSP(node) = true;

    return node;
}

node_st *OCTTfloat(node_st *node) {
    FLOAT_TRANSP(node) = true;

    return node;
}

node_st *OCTTbool(node_st *node) {
    BOOL_TRANSP(node) = true;

    return node;
}
