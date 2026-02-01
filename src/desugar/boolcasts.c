#include "ccn/ccn.h"
#include "utils.h"

node_st *DBCcast(node_st *node) {
    TRAVchildren(node);

    if (CAST_RESOLVED_TY(node) == TY_bool) {
        TAKE(CAST_EXPR(node));

        if (EXPR_RESOLVED_TY(node) == TY_int) {
            node = ASTbinop(node, ASTint(0, TY_int), BO_ne, TY_bool);
        } else if (EXPR_RESOLVED_TY(node) == TY_float) {
            node = ASTbinop(node, ASTfloat(0.0, TY_float), BO_ne, TY_bool);
        }
    }

    return node;
}
