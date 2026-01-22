#include "ccn/ccn.h"

node_st *DWLwhile(node_st *node) {
    TRAVchildren(node);

    node_st *dowhile = ASTifelse(
        CCNcopy(WHILE_EXPR(node)),
        ASTstmts(ASTdowhile(WHILE_STMTS(node), WHILE_EXPR(node)), NULL), NULL);

    WHILE_EXPR(node) = NULL;
    WHILE_STMTS(node) = NULL;
    CCNfree(node);

    return dowhile;
}
