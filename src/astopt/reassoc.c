#include "ccn/ccn.h"

node_st *AORbinop(node_st *node) {
    TRAVchildren(node);

    node_st *left = BINOP_LEFT(node);
    node_st *right = BINOP_RIGHT(node);

    switch (BINOP_OP(node)) {
    case BO_add:
    case BO_sub:
        if (NODE_TYPE(right) == NT_BINOP) {
            if (BINOP_OP(right) == BO_add || BINOP_OP(right) == BO_sub) {
                BINOP_RIGHT(node) = BINOP_LEFT(right);
                BINOP_LEFT(right) = node;
                BINOP_OP(node) =
                    BINOP_OP(node) != BINOP_OP(right) ? BO_sub : BO_add;
                node = right;
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            if (NODE_TYPE(left) == NT_BINOP &&
                (BINOP_OP(left) == BO_add || BINOP_OP(left) == BO_sub)) {
                node_st *tmp = BINOP_RIGHT(left);
                BINOP_RIGHT(left) = right;
                BINOP_RIGHT(node) = tmp;

                enum BinOpKind op = BINOP_OP(node);
                BINOP_OP(node) = BINOP_OP(left);
                BINOP_OP(left) = op;
            } else {
                BINOP_LEFT(node) = right;
                BINOP_RIGHT(node) = left;
            }
        }
        break;
    case BO_mul:
        if (NODE_TYPE(right) == NT_BINOP) {
            if (BINOP_OP(right) == BO_mul) {
                BINOP_RIGHT(node) = BINOP_LEFT(right);
                BINOP_LEFT(right) = node;
                node = right;
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            if (NODE_TYPE(left) == NT_BINOP && BINOP_OP(left) == BO_mul) {
                node_st *tmp = BINOP_RIGHT(left);
                BINOP_RIGHT(left) = right;
                BINOP_RIGHT(node) = tmp;
            } else {
                BINOP_LEFT(node) = right;
                BINOP_RIGHT(node) = left;
            }
        }
        break;
    default:
        break;
    }

    return node;
}
