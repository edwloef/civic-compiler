#include <math.h>

#include "ccn/ccn.h"
#include "globals/globals.h"
#include "macros.h"

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
        if ((NODE_TYPE(left) == NT_INT && INT_VAL(left) == 0) ||
            (NODE_TYPE(left) == NT_FLOAT && FLOAT_VAL(left) == 0.0 &&
             (!globals.fsigned_zeros || signbit(FLOAT_VAL(left)))) ||
            (NODE_TYPE(left) == NT_BOOL && BOOL_VAL(left) == false)) {
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
        if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_not &&
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
        if (NODE_TYPE(left) == NT_MONOP && MONOP_OP(left) == MO_neg) {
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

node_st *AOIcast(node_st *node) {
    TRAVchildren(node);

    if (CAST_TY(node) == EXPR_RESOLVED_TY(CAST_EXPR(node))) {
        // ((int) intval) => intval
        // ((float) floatval) => floatval
        // ((bool) boolval) => boolval
        TAKE(CAST_EXPR(node));
    } else if (CAST_TY(node) == TY_int) {
        if (NODE_TYPE(CAST_EXPR(node)) == NT_CAST &&
            CAST_TY(CAST_EXPR(node)) == TY_float) {
            // ((int) ((float) x)) => (int) x
            SWAP(CAST_EXPR(node), CAST_EXPR(tmp));
        }
    }

    return node;
}
