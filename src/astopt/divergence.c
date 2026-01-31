#include "ccn/ccn.h"

node_st *AODstmts(node_st *node) {
    TRAVchildren(node);

    STMTS_DIVERGES(node) =
        STMT_DIVERGES(STMTS_STMT(node)) ||
        (STMTS_NEXT(node) && STMTS_DIVERGES(STMTS_NEXT(node)));

    return node;
}

node_st *AODifelse(node_st *node) {
    TRAVchildren(node);

    IFELSE_DIVERGES(node) =
        IFELSE_IF_BLOCK(node) && STMTS_DIVERGES(IFELSE_IF_BLOCK(node)) &&
        IFELSE_ELSE_BLOCK(node) && STMTS_DIVERGES(IFELSE_ELSE_BLOCK(node));

    return node;
}

node_st *AODdowhile(node_st *node) {
    TRAVchildren(node);

    DOWHILE_DIVERGES(node) =
        (DOWHILE_STMTS(node) && STMTS_DIVERGES(DOWHILE_STMTS(node))) ||
        (NODE_TYPE(DOWHILE_EXPR(node)) == NT_BOOL &&
         BOOL_VAL(DOWHILE_EXPR(node)) == true);

    return node;
}

node_st *AODreturn(node_st *node) {
    TRAVchildren(node);

    RETURN_DIVERGES(node) = true;

    return node;
}
