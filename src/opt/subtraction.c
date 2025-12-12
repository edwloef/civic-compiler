/**
 * @file
 *
 * This file contains the code for the Subtraction traversal.
 * The traversal has the uid: OS
 *
 *
 */

#include "ccn/ccn.h"
#include "ccngen/ast.h"
#include "ccngen/trav.h"
#include "ccngen/enum.h"
#include "palm/str.h"

node_st *OSbinop(node_st *node)
{
    TRAVleft(node);
    TRAVright(node);

    if (BINOP_OP(node) == BO_sub) {
        if ((NODE_TYPE(BINOP_LEFT(node)) == NT_ID) &&
            (NODE_TYPE(BINOP_RIGHT(node)) == NT_ID) &&
            STReq(ID_VAL(BINOP_LEFT(node)), ID_VAL(BINOP_RIGHT(node)))) {
            node = CCNfree(node);
            node = ASTint(0);
        } else if ((NODE_TYPE(BINOP_LEFT(node)) == NT_ID) &&
                   (NODE_TYPE(BINOP_RIGHT(node)) == NT_ID) &&
                   (ID_VAL(BINOP_LEFT(node)) == ID_VAL(BINOP_RIGHT(node)))) {
            node = CCNfree(node);
            node = ASTint(0);
        }
    }

    return node;
}
