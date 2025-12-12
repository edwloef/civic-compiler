/**
 * @file
 *
 * This file contains the code for the CountOps traversal.
 * The traversal has the uid: SCO
 *
 *
 */

#include "ccn/ccn.h"
#include "ccngen/ast.h"
#include "ccngen/enum.h"
#include "ccngen/trav.h"
#include "ccngen/trav_data.h"

void SCOinit(void) {}
void SCOfini(void) {}

node_st *SCOprogram(node_st *node) {
    TRAVdecls(node);

    PROGRAM_ADD(node) = DATA_SCO_GET()->add;
    PROGRAM_SUB(node) = DATA_SCO_GET()->sub;
    PROGRAM_MUL(node) = DATA_SCO_GET()->mul;
    PROGRAM_DIV(node) = DATA_SCO_GET()->div;
    PROGRAM_MOD(node) = DATA_SCO_GET()->mod;

    return node;
}

node_st *SCObinop(node_st *node) {
    TRAVleft(node);
    TRAVright(node);

    switch (BINOP_OP(node)) {
    case BO_add:
        DATA_SCO_GET()->add++;
        break;
    case BO_sub:
        DATA_SCO_GET()->sub++;
        break;
    case BO_mul:
        DATA_SCO_GET()->mul++;
        break;
    case BO_div:
        DATA_SCO_GET()->div++;
        break;
    case BO_mod:
        DATA_SCO_GET()->mod++;
        break;
    default:
        break;
    }

    return node;
}
