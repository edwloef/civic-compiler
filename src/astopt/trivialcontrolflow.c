#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "utils.h"

static void AOTCFdiverges(node_st *node) {
    if (STMTS_NEXT(node)) {
        STMTS_NEXT(node) = CCNfree(STMTS_NEXT(node));
        CCNcycleNotify();
    }
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

            node = inline_stmts(node, stmts);
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
        } else if (!IFELSE_IF_BLOCK(stmt) && !IFELSE_ELSE_BLOCK(stmt)) {
            TRAVpush(TRAV_EC);

            TRAVexpr(stmt);

            node = inline_stmts(node, DATA_EC_GET()->stmts);

            TRAVpop();
        }
        break;
    case NT_DOWHILE:
        if (NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BOOL) {
            if (BOOL_VAL(DOWHILE_EXPR(stmt)) == true) {
                AOTCFdiverges(node);
            } else {
                node_st *stmts = DOWHILE_STMTS(stmt);
                DOWHILE_STMTS(stmt) = NULL;
                node = inline_stmts(node, stmts);
            }
        }
        break;
    case NT_RETURN:
        AOTCFdiverges(node);
        break;
    default:
        break;
    }

    return node;
}
