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
                // (x + (y + z)) => ((x + y) + z)
                // (x + (y - z)) => ((x + y) - z)
                // (x - (y + z)) => ((x - y) - z)
                // (x - (y - z)) => ((x - y) + z)
                if (BINOP_OP(node) == BO_sub) {
                    BINOP_OP(right) =
                        BINOP_OP(right) == BO_add ? BO_sub : BO_add;
                }
                BINOP_RIGHT(node) = BINOP_LEFT(right);
                BINOP_LEFT(right) = node;
                node = right;
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            if (NODE_TYPE(left) == NT_BINOP &&
                (BINOP_OP(left) == BO_add || BINOP_OP(left) == BO_sub)) {
                // ((x + y) + 7) => ((x + 7) + y)
                // ((x + y) - 7) => ((x - 7) + y)
                // ((x - y) + 7) => ((x + 7) - y)
                // ((x - y) - 7) => ((x - 7) - y)
                node_st *tmp = BINOP_RIGHT(left);
                BINOP_RIGHT(left) = right;
                BINOP_RIGHT(node) = tmp;
                enum BinOpKind op = BINOP_OP(node);
                BINOP_OP(node) = BINOP_OP(left);
                BINOP_OP(left) = op;
            } else {
                // (x + 7) => (7 + x)
                // (x - 7) => ((-7) + x)
                if (BINOP_OP(node) == BO_sub) {
                    BINOP_OP(node) = BO_add;
                    right = ASTmonop(right, MO_neg);
                }
                BINOP_LEFT(node) = right;
                BINOP_RIGHT(node) = left;
            }
        }
        break;
    case BO_mul:
        if (NODE_TYPE(right) == NT_BINOP) {
            if (BINOP_OP(right) == BO_mul) {
                // (x * (y * z)) => ((x * y) * z)
                BINOP_RIGHT(node) = BINOP_LEFT(right);
                BINOP_LEFT(right) = node;
                node = right;
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            if (NODE_TYPE(left) == NT_BINOP && BINOP_OP(left) == BO_mul) {
                // ((x * y) * 7) => ((x * 7) * y)
                node_st *tmp = BINOP_RIGHT(left);
                BINOP_RIGHT(left) = right;
                BINOP_RIGHT(node) = tmp;
            } else {
                // (x * 7) => (7 * x)
                BINOP_LEFT(node) = right;
                BINOP_RIGHT(node) = left;
            }
        }
        break;
    case BO_eq:
    case BO_ne:
        if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
            NODE_TYPE(right) == NT_BOOL) {
            // (x == 7) => (7 == x)
            // (x != 7) => (7 != x)
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;
        }
        break;
    case BO_and:
    case BO_or:
        if ((ARREXPR_TRANSP(left) && NODE_TYPE(right) == NT_BOOL)) {
            // (x && {true, false}) => ({true, false} && x) | x no side effects
            // (x || {true, false}) => ({true, false} || x) | x no side effects
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;
        }
        break;
    default:
        break;
    }

    return node;
}
