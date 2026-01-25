#include "ccn/ccn.h"
#include "ccngen/trav.h"

void ESinit(void) {}
void ESfini(void) {}

node_st *ESbinop(node_st *node) {
    switch
        BINOP_OP(node) {
        case BO_and: {
            node_st *prev = DATA_ES_GET()->stmts;

            TRAVright(node);

            DATA_ES_GET()->stmts = ASTstmts(
                ASTifelse(BINOP_LEFT(node), DATA_ES_GET()->stmts, NULL), prev);

            BINOP_LEFT(node) = NULL;
            node = CCNfree(node);

            break;
        }
        case BO_or: {
            node_st *prev = DATA_ES_GET()->stmts;

            TRAVright(node);

            DATA_ES_GET()->stmts = ASTstmts(
                ASTifelse(BINOP_LEFT(node), NULL, DATA_ES_GET()->stmts), prev);

            BINOP_LEFT(node) = NULL;
            node = CCNfree(node);

            break;
        }
        default:
            TRAVchildren(node);
            break;
        }

    return node;
}

node_st *EScall(node_st *node) {
    DATA_ES_GET()->stmts = ASTstmts(node, DATA_ES_GET()->stmts);
    return NULL;
}
