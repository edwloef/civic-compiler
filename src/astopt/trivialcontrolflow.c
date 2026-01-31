#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "macros.h"
#include "utils.h"

void AOTCFinit(void) {}
void AOTCFfini(void) {}

node_st *AOTCFprogram(node_st *node) {
    DATA_AOTCF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOTCFfunbody(node_st *node) {
    DATA_AOTCF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOTCF_GET()->funtable = DATA_AOTCF_GET()->funtable->parent;

    return node;
}

node_st *AOTCFstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);

    if (!stmt) {
        TAKE(STMTS_NEXT(node));
        if (!node) {
            return node;
        }
    } else if (NODE_TYPE(stmt) == NT_STMTS) {
        STMTS_STMT(node) = NULL;
        TAKE(STMTS_NEXT(node));
        node = inline_stmts(node, stmt);
    }

    stmt = STMTS_STMT(node);

    STMTS_DIVERGES(node) =
        STMT_DIVERGES(stmt) ||
        (STMTS_NEXT(node) && STMTS_DIVERGES(STMTS_NEXT(node)));

    if (STMT_DIVERGES(stmt) && STMTS_NEXT(node)) {
        STMTS_NEXT(node) = CCNfree(STMTS_NEXT(node));
        CCNcycleNotify();
    }

    if (NODE_TYPE(stmt) == NT_CALL) {
        funtable_ref r = {CALL_N(stmt), CALL_L(stmt)};
        funtable_entry *e = funtable_get(DATA_AOTCF_GET()->funtable, r);
        if (!e->side_effects) {
            TRAVpush(TRAV_ES);

            TRAVexprs(stmt);

            TAKE(STMTS_NEXT(node));
            node = inline_stmts(node, DATA_ES_GET()->stmts);

            TRAVpop();
        }
    }

    return node;
}

node_st *AOTCFifelse(node_st *node) {
    TRAVchildren(node);

    IFELSE_DIVERGES(node) =
        IFELSE_IF_BLOCK(node) && STMTS_DIVERGES(IFELSE_IF_BLOCK(node)) &&
        IFELSE_ELSE_BLOCK(node) && STMTS_DIVERGES(IFELSE_ELSE_BLOCK(node));

    if (NODE_TYPE(IFELSE_EXPR(node)) == NT_BOOL) {
        node_st *stmts;
        if (BOOL_VAL(IFELSE_EXPR(node)) == true) {
            stmts = IFELSE_IF_BLOCK(node);
            IFELSE_IF_BLOCK(node) = NULL;
        } else {
            stmts = IFELSE_ELSE_BLOCK(node);
            IFELSE_ELSE_BLOCK(node) = NULL;
        }

        CCNfree(node);
        node = stmts;
    } else if (IFELSE_IF_BLOCK(node)) {
        if (IFELSE_ELSE_BLOCK(node)) {
            if (NODE_TYPE(IFELSE_EXPR(node)) == NT_MONOP) {
                SWAP(IFELSE_EXPR(node), MONOP_EXPR(tmp));

                node_st *tmp = IFELSE_IF_BLOCK(node);
                IFELSE_IF_BLOCK(node) = IFELSE_ELSE_BLOCK(node);
                IFELSE_ELSE_BLOCK(node) = tmp;
            }
        } else if (!STMTS_NEXT(IFELSE_IF_BLOCK(node)) &&
                   NODE_TYPE(STMTS_STMT(IFELSE_IF_BLOCK(node))) == NT_IFELSE &&
                   !IFELSE_ELSE_BLOCK(STMTS_STMT(IFELSE_IF_BLOCK(node)))) {
            IFELSE_EXPR(node) = ASTbinop(
                IFELSE_EXPR(node),
                IFELSE_EXPR(STMTS_STMT(IFELSE_IF_BLOCK(node))), BO_and);
            BINOP_RESOLVED_TY(IFELSE_EXPR(node)) = TY_bool;
            IFELSE_EXPR(STMTS_STMT(IFELSE_IF_BLOCK(node))) = NULL;

            SWAP(IFELSE_IF_BLOCK(node), IFELSE_IF_BLOCK(STMTS_STMT(tmp)));
        }
    } else if (IFELSE_ELSE_BLOCK(node)) {
        IFELSE_EXPR(node) = ASTmonop(IFELSE_EXPR(node), MO_not);
        MONOP_RESOLVED_TY(IFELSE_EXPR(node)) = TY_bool;

        node_st *tmp = IFELSE_IF_BLOCK(node);
        IFELSE_IF_BLOCK(node) = IFELSE_ELSE_BLOCK(node);
        IFELSE_ELSE_BLOCK(node) = tmp;

        CCNcycleNotify();
    } else {
        TRAVpush(TRAV_ES);

        TRAVexpr(node);

        CCNfree(node);
        node = DATA_ES_GET()->stmts;

        TRAVpop();
    }

    return node;
}

node_st *AOTCFdowhile(node_st *node) {
    TRAVchildren(node);

    DOWHILE_DIVERGES(node) =
        (DOWHILE_STMTS(node) && STMTS_DIVERGES(DOWHILE_STMTS(node))) ||
        (NODE_TYPE(DOWHILE_EXPR(node)) == NT_BOOL &&
         BOOL_VAL(DOWHILE_EXPR(node)) == true);

    if (NODE_TYPE(DOWHILE_EXPR(node)) == NT_BOOL &&
        BOOL_VAL(DOWHILE_EXPR(node)) == false) {
        node_st *stmts = DOWHILE_STMTS(node);
        DOWHILE_STMTS(node) = NULL;
        CCNfree(node);
        node = stmts;
    }

    return node;
}

node_st *AOTCFreturn(node_st *node) {
    TRAVchildren(node);

    RETURN_DIVERGES(node) = true;

    return node;
}
