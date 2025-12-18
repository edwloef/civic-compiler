#include "ccn/ccn.h"
#include "ccngen/ast.h"
#include "ccngen/enum.h"
#include "ccngen/trav.h"
#include "globals/globals.h"

node_st *strength_reduce(node_st *base, int count) {
    node_st *res = CCNcopy(base);
    for (; count > 1; count--) {
        res = ASTbinop(res, CCNcopy(base), BO_add);
    }
    return res;
}

node_st *OSRbinop(node_st *node) {
    TRAVleft(node);
    TRAVright(node);

    if (BINOP_OP(node) == BO_mul) {
        if (NODE_TYPE(BINOP_LEFT(node)) == NT_ID &&
            NODE_TYPE(BINOP_RIGHT(node)) == NT_INT) {
            int val = INT_VAL(BINOP_RIGHT(node));
            if (val == 0) {
                node = CCNfree(node);
                node = ASTint(0);
            } else if (val >= 1 && val <= globals.strength_reduction_limit) {
                node_st *res = strength_reduce(BINOP_LEFT(node), val);
                node = CCNfree(node);
                node = res;
            }
        } else if (NODE_TYPE(BINOP_LEFT(node)) == NT_ID &&
                   NODE_TYPE(BINOP_RIGHT(node)) == NT_INT) {
            int val = INT_VAL(BINOP_LEFT(node));
            if (val == 0) {
                node = CCNfree(node);
                node = ASTint(0);
            } else if (val >= 1 && val <= globals.strength_reduction_limit) {
                node_st *res = strength_reduce(BINOP_RIGHT(node), val);
                node = CCNfree(node);
                node = res;
            }
        }
    }

    return node;
}
