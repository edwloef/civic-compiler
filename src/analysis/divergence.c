#include "ccn/ccn.h"
#include "error/error.h"
#include "print/print.h"

node_st *ADprogram(node_st *node) {
    TRAVchildren(node);

    abort_on_error();

    return node;
}

node_st *ADstmts(node_st *node) {
    TRAVchildren(node);

    if (STMT_DIVERGES(STMTS_STMT(node)) && STMTS_NEXT(node)) {
        WARNING(STMTS_STMT(node),
                "any code following this statement is unreachable");
        INFO(STMTS_STMT(STMTS_NEXT(node)), "first unreachable statement");
    }

    STMTS_DIVERGES(node) =
        STMT_DIVERGES(STMTS_STMT(node)) ||
        (STMTS_NEXT(node) && STMTS_DIVERGES(STMTS_NEXT(node)));

    return node;
}

node_st *ADfundecl(node_st *node) {
    TRAVchildren(node);

    if (!FUNDECL_EXTERNAL(node) && FUNDECL_TY(node) != TY_void &&
        (!FUNBODY_STMTS(FUNDECL_BODY(node)) ||
         !STMTS_DIVERGES(FUNBODY_STMTS(FUNDECL_BODY(node))))) {
        ERROR(
            FUNDECL_ID(node),
            "function '%s' returning value of type '%s' doesn't diverge in all "
            "paths",
            ID_VAL(FUNDECL_ID(node)), fmt_BasicType(FUNDECL_TY(node)));
    }

    return node;
}

node_st *ADifelse(node_st *node) {
    TRAVchildren(node);

    IFELSE_DIVERGES(node) =
        IFELSE_IF_BLOCK(node) && STMTS_DIVERGES(IFELSE_IF_BLOCK(node)) &&
        IFELSE_ELSE_BLOCK(node) && STMTS_DIVERGES(IFELSE_ELSE_BLOCK(node));

    return node;
}

node_st *ADwhile(node_st *node) {
    TRAVchildren(node);

    WHILE_EXPR(node) = TRAVstart(WHILE_EXPR(node), TRAV_AOCF);
    WHILE_DIVERGES(node) = NODE_TYPE(WHILE_EXPR(node)) == NT_BOOL &&
                           BOOL_VAL(WHILE_EXPR(node)) == true;

    return node;
}

node_st *ADdowhile(node_st *node) {
    TRAVchildren(node);

    DOWHILE_EXPR(node) = TRAVstart(DOWHILE_EXPR(node), TRAV_AOCF);
    DOWHILE_DIVERGES(node) =
        (DOWHILE_STMTS(node) && STMTS_DIVERGES(DOWHILE_STMTS(node))) ||
        (NODE_TYPE(DOWHILE_EXPR(node)) == NT_BOOL &&
         BOOL_VAL(DOWHILE_EXPR(node)) == true);

    return node;
}

node_st *ADreturn(node_st *node) {
    TRAVchildren(node);

    RETURN_DIVERGES(node) = true;

    return node;
}
