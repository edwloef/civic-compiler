#include "ccn/ccn.h"

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        CCNcycleNotify();                                                      \
        node = tmp;                                                            \
    }

static void AOTCFdiverges(node_st *node) {
    if (STMTS_NEXT(node)) {
        STMTS_NEXT(node) = CCNfree(STMTS_NEXT(node));
        CCNcycleNotify();
    }
}

static node_st *AOTCFinlinestmts(node_st *node, node_st *stmts) {
    TAKE(STMTS_NEXT(node));

    if (stmts) {
        node_st *tmp = stmts;
        while (STMTS_NEXT(tmp)) {
            tmp = STMTS_NEXT(tmp);
        }
        STMTS_NEXT(tmp) = node;
        return stmts;
    } else {
        return node;
    }
}

static node_st *AOTCFnoop(node_st *node) {
    STMTS_STMT(node) = CCNfree(STMTS_STMT(node));
    return AOTCFinlinestmts(node, NULL);
}

node_st *AOTCFstmts(node_st *node) {
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

            node = AOTCFinlinestmts(node, stmts);
        } else if (NODE_TYPE(IFELSE_EXPR(stmt)) == NT_MONOP &&
                   MONOP_OP(IFELSE_EXPR(stmt)) == MO_not) {
            node_st *tmp = IFELSE_EXPR(stmt);
            IFELSE_EXPR(stmt) = MONOP_EXPR(tmp);
            MONOP_EXPR(tmp) = NULL;
            CCNfree(tmp);

            tmp = IFELSE_IF_BLOCK(stmt);
            IFELSE_IF_BLOCK(stmt) = IFELSE_ELSE_BLOCK(stmt);
            IFELSE_ELSE_BLOCK(stmt) = tmp;

            CCNcycleNotify();
        } else if (ARREXPR_TRANSP(IFELSE_EXPR(stmt)) &&
                   !IFELSE_IF_BLOCK(stmt) && !IFELSE_ELSE_BLOCK(stmt)) {
            node = AOTCFinlinestmts(node, NULL);
        }
        break;
    case NT_WHILE:
        if (NODE_TYPE(WHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(WHILE_EXPR(stmt)) == true) {
                AOTCFdiverges(node);
            } else {
                node = AOTCFnoop(node);
            }
        }
        break;
    case NT_DOWHILE:
        if (NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(DOWHILE_EXPR(stmt)) == true) {
                AOTCFdiverges(node);
            } else {
                node_st *stmts = DOWHILE_STMTS(stmt);
                DOWHILE_STMTS(stmt) = NULL;
                node = AOTCFinlinestmts(node, stmts);
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
                STMTS_STMT(node) = CCNfree(STMTS_STMT(node));
                node = AOTCFinlinestmts(node, NULL);
            } else if (INT_VAL(FOR_LOOP_END(stmt)) -
                           INT_VAL(FOR_LOOP_START(stmt)) <=
                       INT_VAL(FOR_LOOP_STEP(stmt))) {
                node_st *stmts = FOR_STMTS(stmt);
                FOR_STMTS(stmt) = NULL;
                node = AOTCFinlinestmts(node, stmts);
            }
        } else if (EXPR_TRANSP(FOR_LOOP_START(stmt)) &&
                   EXPR_TRANSP(FOR_LOOP_END(stmt)) &&
                   EXPR_TRANSP(FOR_LOOP_STEP(stmt)) && !FOR_STMTS(stmt)) {
            node = AOTCFnoop(node);
        }
        break;
    case NT_RETURN:
        AOTCFdiverges(node);
        break;
    case NT_CALL:
        if (CALL_TRANSP(stmt)) {
            node = AOTCFnoop(node);
        }
        break;
    default:
        break;
    }

    return node;
}
