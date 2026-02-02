#include "ccn/ccn.h"
#include "utils.h"

node_st *DIRfundecl(node_st *node) {
    TRAVchildren(node);

    if (!FUNDECL_EXTERNAL(node) && FUNDECL_TY(node) == TY_void &&
        (!FUNBODY_STMTS(FUNDECL_BODY(node)) ||
         !STMTS_DIVERGES(FUNBODY_STMTS(FUNDECL_BODY(node))))) {
        FUNBODY_STMTS(FUNDECL_BODY(node)) = inline_stmts(
            ASTstmts(ASTreturn(NULL), NULL), FUNBODY_STMTS(FUNDECL_BODY(node)));
    }

    return node;
}
