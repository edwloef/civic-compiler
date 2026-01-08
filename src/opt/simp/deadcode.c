#include "ccn/ccn.h"

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        CCNcycleNotify();                                                      \
        node = tmp;                                                            \
    }

node_st *OSDCinlinestmts(node_st *node, node_st *stmts) {
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

node_st *OSDCstmts(node_st *node) {
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

            node = OSDCinlinestmts(node, stmts);
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
                node = OSDCinlinestmts(node, NULL);
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
                node = OSDCinlinestmts(node, stmts);
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
