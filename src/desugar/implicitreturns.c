#include "ccn/ccn.h"

node_st *DIRfundecl(node_st *node) {
    TRAVchildren(node);

    if (!FUNDECL_EXTERNAL(node) && FUNDECL_TY(node) == TY_void &&
        (!FUNBODY_STMTS(FUNDECL_BODY(node)) ||
         !STMTS_DIVERGES(FUNBODY_STMTS(FUNDECL_BODY(node))))) {
        if (FUNBODY_STMTS(FUNDECL_BODY(node))) {
            node_st *tmp = FUNBODY_STMTS(FUNDECL_BODY(node));
            while (STMTS_NEXT(tmp)) {
                tmp = STMTS_NEXT(tmp);
            }
            STMTS_NEXT(tmp) = ASTstmts(ASTreturn(NULL), NULL);
        } else {
            FUNBODY_STMTS(FUNDECL_BODY(node)) = ASTstmts(ASTreturn(NULL), NULL);
        }
    }

    return node;
}
