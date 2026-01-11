#include "ccn/ccn.h"

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        CCNcycleNotify();                                                      \
        node = tmp;                                                            \
    }

static node_st *OTCFinlinestmts(node_st *node, node_st *stmts) {
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

node_st *OTCFstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    switch (NODE_TYPE(stmt)) {
    case NT_IFELSE:
        if (NODE_TYPE(IFELSE_EXPR(stmt)) == NT_BOOL) {
            node_st *stmts;
            if (BOOL_VAL(IFELSE_EXPR(stmt)) == true) {
                stmts = IFELSE_IF_BLOCK(stmt);
                IFELSE_IF_BLOCK(stmt) = NULL;
            } else {
                stmts = IFELSE_ELSE_BLOCK(stmt);
                IFELSE_ELSE_BLOCK(stmt) = NULL;
            }

            node = OTCFinlinestmts(node, stmts);
        } else if (NODE_TYPE(IFELSE_EXPR(stmt)) == NT_MONOP &&
                   MONOP_OP(IFELSE_EXPR(node)) == MO_not) {
            node_st *tmp = IFELSE_EXPR(node);
            IFELSE_EXPR(node) = MONOP_EXPR(tmp);
            MONOP_EXPR(tmp) = NULL;
            CCNfree(tmp);

            tmp = IFELSE_IF_BLOCK(node);
            IFELSE_IF_BLOCK(node) = IFELSE_ELSE_BLOCK(node);
            IFELSE_ELSE_BLOCK(node) = tmp;
        }
        break;
    case NT_WHILE:
        if (NODE_TYPE(WHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(WHILE_EXPR(stmt)) == true) {
                CCNfree(STMTS_NEXT(node));
                STMTS_NEXT(node) = NULL;
                CCNcycleNotify();
            } else {
                CCNfree(STMTS_STMT(node));
                STMTS_STMT(node) = NULL;
                node = OTCFinlinestmts(node, NULL);
            }
        }
        break;
    case NT_DOWHILE:
        if (NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(DOWHILE_EXPR(stmt)) == true) {
                CCNfree(STMTS_NEXT(node));
                STMTS_NEXT(node) = NULL;
                CCNcycleNotify();
            } else {
                node_st *stmts = DOWHILE_STMTS(stmt);
                DOWHILE_STMTS(stmt) = NULL;
                node = OTCFinlinestmts(node, stmts);
            }
        }
        break;
    case NT_FOR:
        if (NODE_TYPE(FOR_LOOP_START(stmt)) == NT_INT &&
            NODE_TYPE(FOR_LOOP_END(stmt)) == NT_INT &&
            NODE_TYPE(FOR_LOOP_STEP(stmt)) == NT_INT) {
            if (INT_VAL(FOR_LOOP_START(stmt)) == INT_VAL(FOR_LOOP_END(stmt)) ||
                ((INT_VAL(FOR_LOOP_START(stmt)) >
                  INT_VAL(FOR_LOOP_END(stmt))) ==
                 (INT_VAL(FOR_LOOP_STEP(stmt)) > 0))) {
                CCNfree(STMTS_STMT(node));
                STMTS_STMT(node) = NULL;
                node = OTCFinlinestmts(node, NULL);
            } else if (INT_VAL(FOR_LOOP_END(stmt)) -
                           INT_VAL(FOR_LOOP_START(stmt)) <=
                       INT_VAL(FOR_LOOP_STEP(stmt))) {
                node_st *stmts = FOR_STMTS(stmt);
                FOR_STMTS(stmt) = NULL;
                node = OTCFinlinestmts(node, stmts);
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
