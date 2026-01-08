#include "ccn/ccn.h"

static vartype RESOLVED_TY(node_st *node) {
    vartype ty = {ARREXPR_RESOLVED_TY(node), ARREXPR_RESOLVED_DIMS(node)};
    return ty;
}

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

node_st *OSImonop(node_st *node) {
    TRAVchildren(node);

    switch (MONOP_OP(node)) {
    case MO_pos:
        TAKE(MONOP_EXPR(node));
        break;
    case MO_neg:
        if (NODE_TYPE(MONOP_EXPR(node)) == NT_MONOP &&
            MONOP_OP(MONOP_EXPR(node)) == MO_neg) {
            TAKE(MONOP_EXPR(MONOP_EXPR(node)));
        } else if (NODE_TYPE(MONOP_EXPR(node)) == NT_BINOP &&
                   BINOP_OP(MONOP_EXPR(node)) == BO_sub &&
                   NODE_TYPE(BINOP_LEFT(MONOP_EXPR(node))) == NT_MONOP &&
                   MONOP_OP(BINOP_LEFT(MONOP_EXPR(node))) == MO_neg) {
            TAKE(MONOP_EXPR(node));
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            BINOP_OP(node) = BO_add;
        }
        break;
    case MO_not:
        if (NODE_TYPE(MONOP_EXPR(node)) == NT_MONOP &&
            MONOP_OP(MONOP_EXPR(node)) == MO_not) {
            TAKE(MONOP_EXPR(MONOP_EXPR(node)))
        } else if (NODE_TYPE(MONOP_EXPR(node)) == NT_BINOP) {
            switch (BINOP_OP(MONOP_EXPR(node))) {
            case BO_lt:
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_ge;
                break;
            case BO_le:
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_gt;
                break;
            case BO_gt:
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_le;
                break;
            case BO_ge:
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_lt;
                break;
            case BO_eq:
                TAKE(MONOP_EXPR(node));
                BINOP_OP(node) = BO_ne;
                break;
            case BO_ne:
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

node_st *OSIbinop(node_st *node) {
    TRAVchildren(node);

    switch (BINOP_OP(node)) {
    case BO_sub:
    case BO_add:
        if ((NODE_TYPE(BINOP_LEFT(node)) == NT_INT &&
             INT_VAL(BINOP_LEFT(node)) == 0) ||
            (NODE_TYPE(BINOP_LEFT(node)) == NT_FLOAT &&
             FLOAT_VAL(BINOP_LEFT(node)) == 0.0) ||
            (NODE_TYPE(BINOP_LEFT(node)) == NT_BOOL &&
             BOOL_VAL(BINOP_LEFT(node)) == false)) {
            TAKE(BINOP_RIGHT(node));
        } else if ((NODE_TYPE(BINOP_RIGHT(node)) == NT_INT &&
                    INT_VAL(BINOP_RIGHT(node)) == 0) ||
                   (NODE_TYPE(BINOP_RIGHT(node)) == NT_FLOAT &&
                    FLOAT_VAL(BINOP_RIGHT(node)) == 0.0) ||
                   (NODE_TYPE(BINOP_RIGHT(node)) == NT_BOOL &&
                    BOOL_VAL(BINOP_RIGHT(node)) == false)) {
            TAKE(BINOP_LEFT(node));
        } else if (NODE_TYPE(BINOP_LEFT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_LEFT(node)) == MO_not &&
                   NODE_TYPE(BINOP_RIGHT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_RIGHT(node)) == MO_not) {
            BINOP_OP(node) = BO_mul;
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            node = ASTmonop(node, MO_not);
        } else if (NODE_TYPE(BINOP_RIGHT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_RIGHT(node)) == MO_neg) {
            BINOP_OP(node) = BINOP_OP(node) == BO_add ? BO_sub : BO_add;
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
        }
        break;
    case BO_div:
    case BO_mul:
        if ((NODE_TYPE(BINOP_LEFT(node)) == NT_INT &&
             INT_VAL(BINOP_LEFT(node)) == 1) ||
            (NODE_TYPE(BINOP_LEFT(node)) == NT_FLOAT &&
             FLOAT_VAL(BINOP_LEFT(node)) == 1.0) ||
            (NODE_TYPE(BINOP_LEFT(node)) == NT_BOOL &&
             BOOL_VAL(BINOP_LEFT(node)) == true)) {
            TAKE(BINOP_RIGHT(node));
        } else if ((NODE_TYPE(BINOP_RIGHT(node)) == NT_INT &&
                    INT_VAL(BINOP_RIGHT(node)) == 1) ||
                   (NODE_TYPE(BINOP_RIGHT(node)) == NT_FLOAT &&
                    FLOAT_VAL(BINOP_RIGHT(node)) == 1.0) ||
                   (NODE_TYPE(BINOP_RIGHT(node)) == NT_BOOL &&
                    BOOL_VAL(BINOP_RIGHT(node)) == true)) {
            TAKE(BINOP_LEFT(node));
        } else if (NODE_TYPE(BINOP_LEFT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_LEFT(node)) == MO_not &&
                   NODE_TYPE(BINOP_RIGHT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_RIGHT(node)) == MO_not) {
            BINOP_OP(node) = BO_add;
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            node = ASTmonop(node, MO_not);
        } else if (NODE_TYPE(BINOP_LEFT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_LEFT(node)) == MO_neg) {
            SWAP(BINOP_LEFT(node), MONOP_EXPR(tmp));
            node = ASTmonop(node, MO_neg);
        } else if (NODE_TYPE(BINOP_RIGHT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_RIGHT(node)) == MO_neg) {
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
            node = ASTmonop(node, MO_neg);
        }
        break;
    case BO_and:
        if ((NODE_TYPE(BINOP_LEFT(node)) == NT_BOOL &&
             BOOL_VAL(BINOP_LEFT(node)) == false) ||
            (NODE_TYPE(BINOP_RIGHT(node)) == NT_BOOL &&
             BOOL_VAL(BINOP_RIGHT(node)) == true)) {
            TAKE(BINOP_LEFT(node));
        }
        break;
    case BO_or:
        if ((NODE_TYPE(BINOP_LEFT(node)) == NT_BOOL &&
             BOOL_VAL(BINOP_LEFT(node)) == true) ||
            (NODE_TYPE(BINOP_RIGHT(node)) == NT_BOOL &&
             BOOL_VAL(BINOP_RIGHT(node)) == false)) {
            TAKE(BINOP_LEFT(node));
        }
        break;
    default:
        break;
    }

    return node;
}

node_st *OSIcast(node_st *node) {
    TRAVchildren(node);

    if (CAST_TY(node) == RESOLVED_TY(CAST_EXPR(node)).ty)
        TAKE(CAST_EXPR(node));

    return node;
}
