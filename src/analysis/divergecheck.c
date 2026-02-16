#include "ccn/ccn.h"
#include "print/print.h"
#include "utils.h"

node_st *ADCprogram(node_st *node) {
    TRAVchildren(node);

    abort_on_error();

    return node;
}

node_st *ADCstmts(node_st *node) {
    TRAVchildren(node);

    if (STMT_DIVERGES(STMTS_STMT(node)) && STMTS_NEXT(node)) {
        WARNING(STMTS_STMT(node),
                "any code following this statement is unreachable");
    }

    STMTS_DIVERGES(node) =
        STMT_DIVERGES(STMTS_STMT(node)) ||
        (STMTS_NEXT(node) && STMTS_DIVERGES(STMTS_NEXT(node)));

    return node;
}

node_st *ADCfundecl(node_st *node) {
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

node_st *ADCassign(node_st *node) {
    NOOP();
}

node_st *ADCifelse(node_st *node) {
    TRAVchildren(node);

    IFELSE_EXPR(node) = TRAVstart(IFELSE_EXPR(node), TRAV_AOCF);

    if (NODE_TYPE(IFELSE_EXPR(node)) == NT_BOOL) {
        if (BOOL_VAL(IFELSE_EXPR(node)) == true) {
            IFELSE_DIVERGES(node) =
                IFELSE_IF_BLOCK(node) && STMTS_DIVERGES(IFELSE_IF_BLOCK(node));

            if (IFELSE_ELSE_BLOCK(node)) {
                WARNING(IFELSE_ELSE_BLOCK(node), "unreachable else-branch");
            }
        } else {
            IFELSE_DIVERGES(node) = IFELSE_ELSE_BLOCK(node) &&
                                    STMTS_DIVERGES(IFELSE_ELSE_BLOCK(node));

            if (IFELSE_IF_BLOCK(node)) {
                WARNING(IFELSE_IF_BLOCK(node), "unreachable if-branch");
            }
        }
    } else {
        IFELSE_DIVERGES(node) =
            IFELSE_IF_BLOCK(node) && STMTS_DIVERGES(IFELSE_IF_BLOCK(node)) &&
            IFELSE_ELSE_BLOCK(node) && STMTS_DIVERGES(IFELSE_ELSE_BLOCK(node));
    }

    return node;
}

node_st *ADCwhile(node_st *node) {
    TRAVchildren(node);

    WHILE_EXPR(node) = TRAVstart(WHILE_EXPR(node), TRAV_AOCF);

    if (NODE_TYPE(WHILE_EXPR(node)) == NT_BOOL) {
        if (BOOL_VAL(WHILE_EXPR(node)) == true) {
            WHILE_DIVERGES(node) = true;
        } else if (WHILE_STMTS(node)) {
            WARNING(WHILE_STMTS(node), "unreachable while-loop body");
        }
    }

    return node;
}

node_st *ADCdowhile(node_st *node) {
    TRAVchildren(node);

    DOWHILE_EXPR(node) = TRAVstart(DOWHILE_EXPR(node), TRAV_AOCF);

    DOWHILE_DIVERGES(node) =
        (DOWHILE_STMTS(node) && STMTS_DIVERGES(DOWHILE_STMTS(node))) ||
        (NODE_TYPE(DOWHILE_EXPR(node)) == NT_BOOL &&
         BOOL_VAL(DOWHILE_EXPR(node)) == true);

    return node;
}

node_st *ADCfor(node_st *node) {
    TRAVchildren(node);

    FOR_LOOP_START(node) = TRAVstart(FOR_LOOP_START(node), TRAV_AOCF);
    FOR_LOOP_END(node) = TRAVstart(FOR_LOOP_END(node), TRAV_AOCF);
    FOR_LOOP_STEP(node) = TRAVstart(FOR_LOOP_STEP(node), TRAV_AOCF);

    if (FOR_STMTS(node) && NODE_TYPE(FOR_LOOP_START(node)) == NT_INT &&
        NODE_TYPE(FOR_LOOP_END(node)) == NT_INT &&
        NODE_TYPE(FOR_LOOP_STEP(node)) == NT_INT &&
        (INT_VAL(FOR_LOOP_STEP(node)) > 0
             ? INT_VAL(FOR_LOOP_START(node)) >= INT_VAL(FOR_LOOP_END(node))
             : INT_VAL(FOR_LOOP_START(node)) <= INT_VAL(FOR_LOOP_END(node)))) {
        WARNING(FOR_STMTS(node), "unreachable for-loop body");
    }

    return node;
}

node_st *ADCreturn(node_st *node) {
    TRAVchildren(node);

    RETURN_DIVERGES(node) = true;

    return node;
}

node_st *ADCcall(node_st *node) {
    NOOP();
}
