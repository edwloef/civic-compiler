#include "ccn/ccn.h"

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        node = tmp;                                                            \
    }

node_st *DCcast(node_st *node) {
    TRAVchildren(node);

    if (CAST_TY(node) == ARREXPR_RESOLVED_TY(CAST_EXPR(node))) {
        TAKE(CAST_EXPR(node));
    } else if (CAST_TY(node) == TY_bool) {
        if (ARREXPR_RESOLVED_TY(CAST_EXPR(node)) == TY_int) {
            TAKE(CAST_EXPR(node));
            node = ASTbinop(node, ASTint(0), BO_ne);
        } else if (ARREXPR_RESOLVED_TY(CAST_EXPR(node)) == TY_float) {
            TAKE(CAST_EXPR(node));
            node = ASTbinop(node, ASTfloat(0.0), BO_ne);
        }
    }

    return node;
}
