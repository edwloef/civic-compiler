#include "ccn/ccn.h"
#include "palm/str.h"

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
        char *name = vartable_get(DATA_DDR_GET()->vartable, dr)->name;

        node_st *dim = ASTvarref(ASTid(STRcpy(name)), NULL);
        VARREF_N(dim) = dr.n;
        VARREF_L(dim) = dr.l;
        VARREF_RESOLVED_TY(dim) = TY_int;

        EXPRS_EXPR(exprs) = ASTbinop(ASTbinop(EXPRS_EXPR(exprs), dim, BO_mul),
                                     EXPRS_EXPR(right), BO_add);
        EXPRS_EXPR(right) = NULL;
    }

    EXPRS_NEXT(exprs) = CCNfree(EXPRS_NEXT(exprs));

    return node;
}
