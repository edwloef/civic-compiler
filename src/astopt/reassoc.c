#include "ccn/ccn.h"

node_st *AORbinop(node_st *node) {
    TRAVchildren(node);

    node_st *left = BINOP_LEFT(node);
    node_st *right = BINOP_RIGHT(node);

    switch (BINOP_OP(node)) {
    case BO_add:
    case BO_sub:
        if (NODE_TYPE(left) == NT_BINOP) {
            if (BINOP_OP(left) == BO_add || BINOP_OP(left) == BO_sub) {
                if (NODE_TYPE(right) == NT_BINOP) {
                    if (BINOP_OP(right) == BO_add ||
                        BINOP_OP(right) == BO_sub) {
                        BINOP_RIGHT(node) = BINOP_LEFT(right);
                        BINOP_LEFT(right) = node;
                        BINOP_OP(node) =
                            BINOP_OP(node) != BINOP_OP(right) ? BO_sub : BO_add;
                        node = right;
                    }
                } else if (NODE_TYPE(right) == NT_INT ||
                           NODE_TYPE(right) == NT_FLOAT ||
                           NODE_TYPE(right) == NT_BOOL) {
                    node_st *tmp = BINOP_RIGHT(left);
                    BINOP_RIGHT(left) = right;
                    BINOP_RIGHT(node) = tmp;

                    enum BinOpKind op = BINOP_OP(node);
                    BINOP_OP(node) = BINOP_OP(left);
                    BINOP_OP(left) = op;
                }
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;

            if (BINOP_OP(node) == BO_sub) {
                BINOP_LEFT(node) = ASTmonop(BINOP_LEFT(node), MO_neg);
                BINOP_OP(node) = BO_add;
            }
        }
        break;
    case BO_mul:
        if (NODE_TYPE(left) == NT_BINOP) {
            if (BINOP_OP(left) == BO_mul) {
                if (NODE_TYPE(right) == NT_BINOP) {
                    if (BINOP_OP(right) == BO_mul) {
                        BINOP_RIGHT(node) = BINOP_LEFT(right);
                        BINOP_LEFT(right) = node;
                        node = right;
                    }
                } else if (NODE_TYPE(right) == NT_INT ||
                           NODE_TYPE(right) == NT_FLOAT ||
                           NODE_TYPE(right) == NT_BOOL) {
                    node_st *tmp = BINOP_RIGHT(left);
                    BINOP_RIGHT(left) = right;
                    BINOP_RIGHT(node) = tmp;
                }
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;
        }
        break;
    default:
        break;
    }

    return node;
}