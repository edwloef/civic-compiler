#include "ccn/ccn.h"
#include "table/vartable.h"

void DDRinit(void) {}
void DDRfini(void) {}

node_st *DDRprogram(node_st *node) {
    DATA_DDR_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DDRfundecl(node_st *node) {
    DATA_DDR_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DDR_GET()->vartable = DATA_DDR_GET()->vartable->parent;

    return node;
}

node_st *DDRmalloc(node_st *node) {
    TRAVchildren(node);

    node_st *exprs = MALLOC_EXPRS(node);
    if (!exprs) {
        return node;
    }

    for (node_st *right = EXPRS_NEXT(exprs); right; right = EXPRS_NEXT(right)) {
        EXPRS_EXPR(exprs) =
            ASTbinop(EXPRS_EXPR(exprs), EXPRS_EXPR(right), BO_mul);
        BINOP_RESOLVED_TY(EXPRS_EXPR(exprs)) = TY_int;
        EXPRS_EXPR(right) = NULL;
    }

    EXPRS_NEXT(exprs) = CCNfree(EXPRS_NEXT(exprs));

    return node;
}

node_st *DDRvarref(node_st *node) {
    TRAVchildren(node);

    node_st *exprs = VARREF_EXPRS(node);
    if (!exprs) {
        return node;
    }

    vartable_ref r = {VARREF_N(node), VARREF_L(node)};
    vartable_entry *e = vartable_get(DATA_DDR_GET()->vartable, r);

    int i = e->ty.len - 1;
    for (node_st *right = EXPRS_NEXT(exprs); right;
         right = EXPRS_NEXT(right), i--) {
        vartable_ref dr = e->ty.buf[i];
        dr.n += r.n;

        EXPRS_EXPR(exprs) = ASTbinop(
            ASTbinop(EXPRS_EXPR(exprs),
                     vartable_get_ref(DATA_DDR_GET()->vartable, dr), BO_mul),
            EXPRS_EXPR(right), BO_add);
        BINOP_RESOLVED_TY(BINOP_LEFT(EXPRS_EXPR(exprs))) = TY_int;
        BINOP_RESOLVED_TY(EXPRS_EXPR(exprs)) = TY_int;
        EXPRS_EXPR(right) = NULL;
    }

    EXPRS_NEXT(exprs) = CCNfree(EXPRS_NEXT(exprs));

    return node;
}
