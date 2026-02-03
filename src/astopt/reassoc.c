#include "ccn/ccn.h"
#include "globals/globals.h"

#define CHECK_ASSOCIATIVE_MATH()                                               \
    if (EXPR_RESOLVED_TY(node) == TY_float && !globals.associative_math)       \
        break;

node_st *AORbinop(node_st *node) {
    node_st *left = BINOP_LEFT(node);
    node_st *right = BINOP_RIGHT(node);

    switch (BINOP_OP(node)) {
    case BO_add:
    case BO_sub:
        if (NODE_TYPE(right) == NT_BINOP) {
            if (BINOP_OP(right) == BO_add || BINOP_OP(right) == BO_sub) {
                CHECK_ASSOCIATIVE_MATH();
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
                CCNcycleNotify();
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            if (NODE_TYPE(left) == NT_BINOP &&
                (BINOP_OP(left) == BO_add || BINOP_OP(left) == BO_sub)) {
                CHECK_ASSOCIATIVE_MATH();
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
                CCNcycleNotify();
            } else {
                // (x + 7) => (7 + x)
                // (x - 7) => ((-7) + x)
                if (BINOP_OP(node) == BO_sub) {
                    BINOP_OP(node) = BO_add;
                    right = ASTmonop(right, MO_neg, EXPR_RESOLVED_TY(right));
                }
                BINOP_LEFT(node) = right;
                BINOP_RIGHT(node) = left;
                CCNcycleNotify();
            }
        }
        break;
    case BO_mul:
        if (NODE_TYPE(right) == NT_BINOP) {
            if (BINOP_OP(right) == BO_mul) {
                CHECK_ASSOCIATIVE_MATH();
                // (x * (y * z)) => ((x * y) * z)
                BINOP_RIGHT(node) = BINOP_LEFT(right);
                BINOP_LEFT(right) = node;
                node = right;
                CCNcycleNotify();
            }
        } else if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT ||
                   NODE_TYPE(right) == NT_BOOL) {
            if (NODE_TYPE(left) == NT_BINOP && BINOP_OP(left) == BO_mul) {
                CHECK_ASSOCIATIVE_MATH();
                // ((x * y) * 7) => ((x * 7) * y)
                node_st *tmp = BINOP_RIGHT(left);
                BINOP_RIGHT(left) = right;
                BINOP_RIGHT(node) = tmp;
                CCNcycleNotify();
            } else {
                // (x * 7) => (7 * x)
                BINOP_LEFT(node) = right;
                BINOP_RIGHT(node) = left;
                CCNcycleNotify();
            }
        }
        break;
    case BO_lt:
    case BO_le:
        if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT) {
            // (x < 7) => (7 > x)
            // (x <= 7) => (7 >= x)
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;
            BINOP_OP(node) = BINOP_OP(node) == BO_lt ? BO_gt : BO_ge;
            CCNcycleNotify();
        }
        break;
    case BO_gt:
    case BO_ge:
        if (NODE_TYPE(right) == NT_INT || NODE_TYPE(right) == NT_FLOAT) {
            // (x > 7) => (7 < x)
            // (x >= 7) => (7 <= x)
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;
            BINOP_OP(node) = BINOP_OP(node) == BO_gt ? BO_lt : BO_le;
            CCNcycleNotify();
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
            CCNcycleNotify();
        }
        break;
    case BO_and:
    case BO_or:
        if (NODE_TYPE(right) == NT_BOOL &&
            (EXPR_SIDE_EFFECTS(left) ||
             BOOL_VAL(right) == (BINOP_OP(node) == BO_and))) {
            // (x && true) => (true && x)
            // (x && false) => (false && x) | x no side effects
            // (x || true) => (true || x) | x no side effects
            // (x || false) => (false || x)
            BINOP_LEFT(node) = right;
            BINOP_RIGHT(node) = left;
            CCNcycleNotify();
        }
        break;
    default:
        break;
    }

    TRAVchildren(node);

    return node;
}
