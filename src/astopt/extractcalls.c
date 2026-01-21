#include "ccn/ccn.h"

void ECinit(void) {}
void ECfini(void) {}

node_st *ECcall(node_st *node) {
    DATA_EC_GET()->stmts = ASTstmts(node, DATA_EC_GET()->stmts);
    return NULL;
}
