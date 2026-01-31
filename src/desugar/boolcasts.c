#include "ccn/ccn.h"
#include "macros.h"

node_st *DBCcast(node_st *node) {
    TRAVchildren(node);

    if (CAST_TY(node) == TY_bool) {
        TAKE(CAST_EXPR(node));

        if (EXPR_RESOLVED_TY(node) == TY_int) {
            node = ASTbinop(node, ASTint(0), BO_ne);
            INT_RESOLVED_TY(BINOP_RIGHT(node)) = TY_int;
            BINOP_RESOLVED_TY(node) = TY_bool;
        } else if (EXPR_RESOLVED_TY(node) == TY_float) {
            node = ASTbinop(node, ASTfloat(0.0), BO_ne);
            FLOAT_RESOLVED_TY(BINOP_RIGHT(node)) = TY_float;
            BINOP_RESOLVED_TY(node) = TY_bool;
        }
    }

    return node;
}
