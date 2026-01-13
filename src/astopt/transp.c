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

node_st *AOTexprs(node_st *node) {
    TRAVchildren(node);

    EXPRS_TRANSP(node) = EXPR_TRANSP(EXPRS_EXPR(node)) &&
                         (!EXPRS_NEXT(node) || EXPRS_TRANSP(EXPRS_NEXT(node)));

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
        int prev = DATA_AOT_GET()->min_nesting_level;
        DATA_AOT_GET()->vartable = FUNDECL_VARTABLE(node);
        DATA_AOT_GET()->min_nesting_level = DATA_AOT_GET()->nesting_level + 1;

        TRAVchildren(node);
        bool transp =
            DATA_AOT_GET()->min_nesting_level > DATA_AOT_GET()->nesting_level;

        DATA_AOT_GET()->vartable = DATA_AOT_GET()->vartable->parent;
        DATA_AOT_GET()->min_nesting_level = prev;

        funtable_ref r = {0, FUNDECL_L(node)};
        funtable_entry *e = funtable_get(DATA_AOT_GET()->funtable, r);
        if (transp != e->transp) {
            e->transp = transp;
            CCNcycleNotify();
        }
    }

    return node;
}

node_st *AOTfunbody(node_st *node) {
    DATA_AOT_GET()->nesting_level++;
    DATA_AOT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOT_GET()->nesting_level--;
    DATA_AOT_GET()->funtable = DATA_AOT_GET()->funtable->parent;

    return node;
}

node_st *AOTassign(node_st *node) {
    TRAVchildren(node);

    DATA_AOT_GET()->min_nesting_level =
        MIN(DATA_AOT_GET()->min_nesting_level,
            DATA_AOT_GET()->nesting_level - VARREF_N(ASSIGN_REF(node)));

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
    bool fun_transp = funtable_get(DATA_AOT_GET()->funtable, r)->transp;

    CALL_TRANSP(node) =
        fun_transp && (!CALL_EXPRS(node) || EXPRS_TRANSP(CALL_EXPRS(node)));

    if (!fun_transp) {
        DATA_AOT_GET()->min_nesting_level =
            MIN(DATA_AOT_GET()->min_nesting_level,
                DATA_AOT_GET()->nesting_level - CALL_N(node));
    }

    return node;
}

node_st *AOTvarref(node_st *node) {
    TRAVchildren(node);

    VARREF_TRANSP(node) =
        !VARREF_EXPRS(node) || EXPRS_TRANSP(VARREF_EXPRS(node));

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
