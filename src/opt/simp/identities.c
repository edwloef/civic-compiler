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

    if (MONOP_OP(node) == MO_pos) {
        TAKE(MONOP_EXPR(node));
    } else if ((MONOP_OP(node) == MO_neg &&
                NODE_TYPE(MONOP_EXPR(node)) == NT_MONOP &&
                MONOP_OP(MONOP_EXPR(node)) == MO_neg) ||
               (MONOP_OP(node) == MO_not &&
                NODE_TYPE(MONOP_EXPR(node)) == NT_MONOP &&
                MONOP_OP(MONOP_EXPR(node)) == MO_not)) {
        TAKE(MONOP_EXPR(MONOP_EXPR(node)))
    }

    return node;
}

node_st *OSIbinop(node_st *node) {
    TRAVchildren(node);

    switch (BINOP_OP(node)) {
    case BO_sub:
        if ((NODE_TYPE(BINOP_LEFT(node)) == NT_INT &&
             INT_VAL(BINOP_LEFT(node)) == 0) ||
            (NODE_TYPE(BINOP_LEFT(node)) == NT_FLOAT &&
             FLOAT_VAL(BINOP_LEFT(node)) == 0.0)) {
            TAKE(BINOP_RIGHT(node));
        } else if ((NODE_TYPE(BINOP_RIGHT(node)) == NT_INT &&
                    INT_VAL(BINOP_RIGHT(node)) == 0) ||
                   (NODE_TYPE(BINOP_RIGHT(node)) == NT_FLOAT &&
                    FLOAT_VAL(BINOP_RIGHT(node)) == 0.0)) {
            TAKE(BINOP_LEFT(node));
            node = ASTmonop(node, MO_neg);
        } else if (NODE_TYPE(BINOP_RIGHT(node)) == NT_MONOP &&
                   MONOP_OP(BINOP_RIGHT(node)) == MO_neg) {
            BINOP_OP(node) = BO_add;
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
        }
        break;
    case BO_div:
        if ((NODE_TYPE(BINOP_RIGHT(node)) == NT_INT &&
             INT_VAL(BINOP_RIGHT(node)) == 1) ||
            (NODE_TYPE(BINOP_RIGHT(node)) == NT_FLOAT &&
             FLOAT_VAL(BINOP_RIGHT(node)) == 1.0)) {
            TAKE(BINOP_LEFT(node));
        }
        break;
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
            BINOP_OP(node) = BO_sub;
            SWAP(BINOP_RIGHT(node), MONOP_EXPR(tmp));
        }
        break;
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

node_st *OSIinlinestmts(node_st *node, node_st *stmts) {
    TAKE(STMTS_NEXT(node));

    if (stmts) {
        node_st *tmp = stmts;
        while (STMTS_NEXT(tmp))
            tmp = STMTS_NEXT(tmp);
        STMTS_NEXT(tmp) = node;
        return stmts;
    } else {
        return node;
    }
}

node_st *OSIstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    switch (NODE_TYPE(stmt)) {
    case NT_IFELSE: {
        if (NODE_TYPE(IFELSE_EXPR(stmt)) == NT_BOOL) {
            node_st *stmts;
            if (BOOL_VAL(IFELSE_EXPR(stmt)) == true) {
                stmts = IFELSE_IF_BLOCK(stmt);
                IFELSE_IF_BLOCK(stmt) = NULL;
            } else {
                stmts = IFELSE_ELSE_BLOCK(stmt);
                IFELSE_ELSE_BLOCK(stmt) = NULL;
            }

            node = OSIinlinestmts(node, stmts);
        }
    } break;
    case NT_WHILE:
        if (NODE_TYPE(WHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(WHILE_EXPR(stmt)) == true) {
                CCNfree(STMTS_NEXT(node));
                STMTS_NEXT(node) = NULL;
                CCNcycleNotify();
            } else {
                CCNfree(stmt);
                node = OSIinlinestmts(node, NULL);
            }
        }
        break;
    case NT_DOWHILE:
        if (NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(DOWHILE_EXPR(stmt)) == true) {
                STMTS_STMT(node) =
                    ASTwhile(DOWHILE_EXPR(stmt), DOWHILE_STMTS(stmt));
                DOWHILE_EXPR(stmt) = NULL;
                DOWHILE_STMTS(stmt) = NULL;
                CCNfree(stmt);
                CCNcycleNotify();
            } else {
                node_st *stmts = DOWHILE_STMTS(stmt);
                DOWHILE_STMTS(stmt) = NULL;
                node = OSIinlinestmts(node, stmts);
            }
        }
        break;
    case NT_RETURN:
        CCNfree(STMTS_NEXT(node));
        STMTS_NEXT(node) = NULL;
        CCNcycleNotify();
    default:
        break;
    }

    return node;
}
