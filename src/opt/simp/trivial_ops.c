#include "ccn/ccn.h"

static vartype RESOLVED_TY(node_st *node) {
    vartype ty = {ARREXPR_RESOLVED_TY(node), ARREXPR_RESOLVED_DIMS(node)};
    return ty;
}

#define TAKE(e)                                                                \
    {                                                                          \
        node_st *tmp = e;                                                      \
        e = NULL;                                                              \
        CCNfree(node);                                                         \
        CCNcycleNotify();                                                      \
        node = tmp;                                                            \
    }

node_st *OSTOmonop(node_st *node) {
    TRAVchildren(node);

    if (MONOP_OP(node) == MO_pos)
        TAKE(MONOP_EXPR(node));

    return node;
}

node_st *OSTObinop(node_st *node) {
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
            TAKE(BINOP_RIGHT(node));
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

node_st *OSTOcast(node_st *node) {
    TRAVchildren(node);

    if (CAST_TY(node) == RESOLVED_TY(CAST_EXPR(node)).ty)
        TAKE(CAST_EXPR(node));

    return node;
}

node_st *OSTOinlinestmts(node_st *l, node_st *r) {
    CCNcycleNotify();

    node_st *tmp = STMTS_NEXT(l);
    STMTS_NEXT(l) = NULL;
    CCNfree(l);
    l = tmp;

    if (r) {
        node_st *tmp = r;
        while (STMTS_NEXT(tmp))
            tmp = STMTS_NEXT(tmp);
        STMTS_NEXT(tmp) = l;
        return r;
    } else {
        return l;
    }
}

node_st *OSTOstmts(node_st *node) {
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

            node = OSTOinlinestmts(node, stmts);
        }
    } break;
    case NT_WHILE:
        if (NODE_TYPE(WHILE_EXPR(stmt)) == NT_BOOL &&
            BOOL_VAL(WHILE_EXPR(stmt)) == false) {
            CCNfree(stmt);

            node = OSTOinlinestmts(node, NULL);
        }
        break;
    case NT_DOWHILE:
        if (NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BOOL &&
            BOOL_VAL(DOWHILE_EXPR(stmt)) == false) {
            node_st *stmts = DOWHILE_STMTS(stmt);
            DOWHILE_STMTS(stmt) = NULL;

            node = OSTOinlinestmts(node, stmts);
        }
        break;
    default:
        break;
    }

    return node;
}
