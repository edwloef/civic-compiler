#include "ccn/ccn.h"
#include "globals/globals.h"

#define CHECK_FFINITE_MATH_ONLY()                                              \
    if (ARREXPR_RESOLVED_TY(node) == TY_float && !globals.ffinite_math_only)   \
        return node;

#define CHECK_FNO_SIGNED_ZEROS()                                               \
    if (ARREXPR_RESOLVED_TY(node) == TY_float && globals.fsigned_zeros)        \
        return node;

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        CCNcycleNotify();                                                      \
        node = tmp;                                                            \
    }

#define SWAP(n, e)                                                             \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = e;                                                                 \
        e = NULL;                                                              \
        CCNfree(tmp);                                                          \
        CCNcycleNotify();                                                      \
    }

#define WRAP(o)                                                                \
    {                                                                          \
        node = ASTmonop(node, o);                                              \
        MONOP_TRANSP(node) = ARREXPR_TRANSP(MONOP_EXPR(node));                 \
        CCNcycleNotify();                                                      \
    }

node_st *AOImonop(node_st *node) {
    TRAVchildren(node);

    node_st *expr = MONOP_EXPR(node);

    switch (MONOP_OP(node)) {
    case MO_pos:
        // (+x) => x
        TAKE(MONOP_EXPR(node));
        break;
    case MO_neg:
        if (NODE_TYPE(expr) == NT_MONOP && MONOP_OP(expr) == MO_neg) {
            // (-(-x)) => x
            TAKE(MONOP_EXPR(MONOP_EXPR(node)));
        } else if (NODE_TYPE(expr) == NT_BINOP) {
            node_st *left = BINOP_LEFT(expr);
            node_st *right = BINOP_RIGHT(expr);

            switch (BINOP_OP(expr)) {
            case BO_add:
            case BO_sub:
                if (NODE_TYPE(left) == NT_INT || NODE_TYPE(left) == NT_FLOAT) {
                    // (-(7 - x)) => ((-7) + x)
                    TAKE(MONOP_EXPR(node));
                    BINOP_LEFT(node) = ASTmonop(BINOP_LEFT(node), MO_neg);
                    BINOP_OP(node) = BINOP_OP(node) == BO_add ? BO_sub : BO_add;
                }
                break;
            case BO_mul:
                if (NODE_TYPE(left) == NT_INT || NODE_TYPE(left) == NT_FLOAT) {
                    // (-(7 * x)) => ((-7) * x)
                    TAKE(MONOP_EXPR(node));
                    BINOP_LEFT(node) = ASTmonop(BINOP_LEFT(node), MO_neg);
                }
                break;
            case BO_div:
                if (NODE_TYPE(left) == NT_INT || NODE_TYPE(left) == NT_FLOAT) {
                    // (-(7 / x)) => ((-7) / x)
                    TAKE(MONOP_EXPR(node));
                    BINOP_LEFT(node) = ASTmonop(BINOP_LEFT(node), MO_neg);
                } else if (NODE_TYPE(right) == NT_INT ||
                           NODE_TYPE(right) == NT_FLOAT) {
                    // (-(x / 7)) => (x / (-7))
                    TAKE(MONOP_EXPR(node));
                    BINOP_RIGHT(node) = ASTmonop(BINOP_RIGHT(node), MO_neg);
                }
                break;
            case BO_mod:
                if (NODE_TYPE(left) == NT_INT || NODE_TYPE(left) == NT_FLOAT) {
                    // (-(7 % x)) => ((-7) % x)
                    TAKE(MONOP_EXPR(node));
                    BINOP_LEFT(node) = ASTmonop(BINOP_LEFT(node), MO_neg);
                }
            default:
                break;
            }
        }
        break;
    case MO_not:
        if (NODE_TYPE(expr) == NT_MONOP && MONOP_OP(expr) == MO_not) {
            TAKE(MONOP_EXPR(MONOP_EXPR(node)))
        } else if (NODE_TYPE(expr) == NT_BINOP) {
            switch (BINOP_OP(expr)) {
            case BO_lt:
                // (!(x < y)) => (x >= y)
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_ge;
                break;
            case BO_le:
                // (!(x <= y)) => (x > y)
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_gt;
                break;
            case BO_gt:
                // (!(x > y)) => (x <= y)
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_le;
                break;
            case BO_ge:
                // (!(x >= y)) => (x < y)
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_lt;
                break;
            case BO_eq:
                // (!(x == y)) => (x != y)
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_ne;
                break;
            case BO_ne:
                // (!(x != y)) => (x == y)
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_eq;
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    return node;
}

node_st *AOIbinop(node_st *node) {
    TRAVchildren(node);

    node_st *left = BINOP_LEFT(node);
    node_st *right = BINOP_RIGHT(node);

    switch (BINOP_OP(node)) {
    case BO_add:
    case BO_sub:
        if (ARREXPR_TRANSP(right) && NODE_TYPE(left) == NT_BOOL &&
            BOOL_VAL(left) == true) {
            // (true + x) => true | x no side effects
            TAKE(BINOP_LEFT(node));
            break;
        } else if ((NODE_TYPE(left) == NT_INT && INT_VAL(left) == 0) ||
                   (NODE_TYPE(left) == NT_FLOAT && FLOAT_VAL(left) == 0.0) ||
                   (NODE_TYPE(left) == NT_BOOL && BOOL_VAL(left) == false)) {
            CHECK_FNO_SIGNED_ZEROS();
            // ({0, 0.0, false} + x) => x
            // ({0, 0.0} - x) => (-x)
            if (BINOP_OP(node) == BO_sub) {
                BINOP_RIGHT(node) = ASTmonop(BINOP_RIGHT(node), MO_neg);
            }
            TAKE(BINOP_RIGHT(node));
        } else if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_not &&
                   NODE_TYPE(right) == NT_MONOP && MONOP_OP(right) == MO_not) {
            // ((!x) + (!x)) => (!(x * x))
            BINOP_OP(node) = BO_mul;
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            WRAP(MO_not);
        } else if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_neg) {
            // ((-x) + y) => (-(x - y))
            // ((-x) - y) => (-(x + y))
            BINOP_OP(node) = BINOP_OP(node) == BO_add ? BO_sub : BO_add;
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            WRAP(MO_neg);
        } else if (NODE_TYPE(right) == NT_MONOP && MONOP_OP(right) == MO_neg) {
            // (x + (-y)) => (x - y)
            // (x - (-y)) => (x + y)
            BINOP_OP(node) = BINOP_OP(node) == BO_add ? BO_sub : BO_add;
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
        }
        break;
    case BO_mul:
        if (ARREXPR_TRANSP(right) &&
            ((NODE_TYPE(left) == NT_INT && INT_VAL(left) == 0) ||
             (NODE_TYPE(left) == NT_FLOAT && FLOAT_VAL(left) == 0.0) ||
             (NODE_TYPE(left) == NT_BOOL && BOOL_VAL(left) == false))) {
            CHECK_FFINITE_MATH_ONLY();
            CHECK_FNO_SIGNED_ZEROS();
            // ({0, 0.0, false} * x) => {0, 0.0, false} | x no side effects
            TAKE(BINOP_LEFT(node));
            break;
        } else if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_not &&
                   NODE_TYPE(right) == NT_MONOP && MONOP_OP(right) == MO_not) {
            // ((!x) * (!y)) => (!(x + y))
            BINOP_OP(node) = BO_add;
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            WRAP(MO_not);
            break;
        } else if ((NODE_TYPE(left) == NT_INT && INT_VAL(left) == 1) ||
                   (NODE_TYPE(left) == NT_FLOAT && FLOAT_VAL(left) == 1.0) ||
                   (NODE_TYPE(left) == NT_BOOL && BOOL_VAL(left) == true)) {
            // ({1, 1.0, true} * x) => x
            TAKE(BINOP_RIGHT(node));
            break;
        } else if ((NODE_TYPE(left) == NT_INT && INT_VAL(left) == -1) ||
                   (NODE_TYPE(left) == NT_FLOAT && FLOAT_VAL(left) == -1.0)) {
            // ({-1, -1.0} * x) => (-x)
            TAKE(BINOP_RIGHT(node));
            WRAP(MO_neg);
            break;
        } else if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_neg) {
            // ((-x) * y) => (-(x * y))
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            WRAP(MO_neg);
        } else if (NODE_TYPE(right) == NT_MONOP && MONOP_OP(right) == MO_neg) {
            // (x * (-y)) => (-(x * y))
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            WRAP(MO_neg);
        }
        break;
    case BO_div:
        if ((NODE_TYPE(right) == NT_INT && INT_VAL(right) == 1) ||
            (NODE_TYPE(right) == NT_FLOAT && FLOAT_VAL(right) == 1.0)) {
            // (x / {1, 1.0}) => x
            TAKE(BINOP_LEFT(node));
        } else if ((NODE_TYPE(right) == NT_INT && INT_VAL(right) == -1) ||
                   (NODE_TYPE(right) == NT_FLOAT && FLOAT_VAL(right) == -1.0)) {
            // (x / {-1, -1.0}) => -x
            TAKE(BINOP_LEFT(node));
            WRAP(MO_neg);
        } else if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_neg) {
            // ((-x) / y) => (-(x / y))
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            WRAP(MO_neg);
        } else if (NODE_TYPE(right) == NT_MONOP && MONOP_OP(right) == MO_neg) {
            // (x / (-y)) => (-(x / y))
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            WRAP(MO_neg);
        }
        break;
    case BO_mod:
        if (ARREXPR_TRANSP(left) && NODE_TYPE(right) == NT_INT &&
            INT_VAL(BINOP_RIGHT(node)) == 1) {
            // (x % 1) => 1 | x no side effects
            TAKE(BINOP_RIGHT(node));
        } else if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_neg) {
            // ((-x) % y) => (-(x % y))
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            WRAP(MO_neg);
        } else if (NODE_TYPE(right) == NT_MONOP && MONOP_OP(right) == MO_neg) {
            // (x % (-y)) => (x % y)
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
        }
        break;
    case BO_and:
        if (NODE_TYPE(left) == NT_BOOL) {
            if (BOOL_VAL(left) == true) {
                // (true && x) => x
                TAKE(BINOP_RIGHT(node));
            } else {
                // (false && x) => false
                TAKE(BINOP_LEFT(node));
            }
        }
        break;
    case BO_or:
        if (NODE_TYPE(left) == NT_BOOL) {
            if (BOOL_VAL(left) == true) {
                // (true || x) => true
                TAKE(BINOP_LEFT(node));
            } else {
                // (false || x) => x
                TAKE(BINOP_RIGHT(node));
            }
        }
        break;
    case BO_eq:
        if (NODE_TYPE(left) == NT_BOOL) {
            if (BOOL_VAL(left) == true) {
                // (true == x) => x
                TAKE(BINOP_RIGHT(node));
            } else {
                // (false == x) => (!x)
                TAKE(BINOP_RIGHT(node));
                WRAP(MO_not);
            }
        }
        break;
    case BO_ne:
        if (NODE_TYPE(left) == NT_BOOL) {
            if (BOOL_VAL(left) == true) {
                // (true != x) => (!x)
                TAKE(BINOP_RIGHT(node));
                WRAP(MO_not);
            } else {
                // (false != x) => x
                TAKE(BINOP_RIGHT(node));
            }
        }
        break;
    default:
        break;
    }

    return node;
}
