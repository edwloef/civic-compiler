#include "ccn/ccn.h"

node_st *DDRvarref(node_st *node) {
    TRAVchildren(node);

    node_st *exprs = VARREF_EXPRS(node);
    if (!exprs)
        return node;

    for (node_st *right = EXPRS_NEXT(exprs); right; right = EXPRS_NEXT(right)) {
        EXPRS_EXPR(exprs) =
            ASTbinop(EXPRS_EXPR(exprs), EXPRS_EXPR(right), BO_mul);
        EXPRS_EXPR(right) = NULL;
    }

    EXPRS_NEXT(exprs) = CCNfree(EXPRS_NEXT(exprs));

    return node;
}
