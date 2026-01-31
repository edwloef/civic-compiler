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
    if (STMT_DIVERGES(STMTS_STMT(node)) && STMTS_NEXT(node)) {
        STMTS_NEXT(node) = CCNfree(STMTS_NEXT(node));
        CCNcycleNotify();
    }

    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    switch (NODE_TYPE(stmt)) {
    case NT_IFELSE:
        if (NODE_TYPE(IFELSE_EXPR(stmt)) == NT_BOOL) {
            node_st *stmts;
            if (BOOL_VAL(IFELSE_EXPR(stmt)) == true) {
                stmts = IFELSE_IF_BLOCK(stmt);
                IFELSE_IF_BLOCK(stmt) = NULL;
            } else {
                stmts = IFELSE_ELSE_BLOCK(stmt);
                IFELSE_ELSE_BLOCK(stmt) = NULL;
            }

            TAKE(STMTS_NEXT(node));
            node = inline_stmts(node, stmts);
        } else if (IFELSE_IF_BLOCK(stmt)) {
            if (IFELSE_ELSE_BLOCK(stmt)) {
                if (NODE_TYPE(IFELSE_EXPR(stmt)) == NT_MONOP) {
                    SWAP(IFELSE_EXPR(stmt), MONOP_EXPR(tmp));

                    node_st *tmp = IFELSE_IF_BLOCK(stmt);
                    IFELSE_IF_BLOCK(stmt) = IFELSE_ELSE_BLOCK(stmt);
                    IFELSE_ELSE_BLOCK(stmt) = tmp;
                }
            } else if (!STMTS_NEXT(IFELSE_IF_BLOCK(stmt)) &&
                       NODE_TYPE(STMTS_STMT(IFELSE_IF_BLOCK(stmt))) ==
                           NT_IFELSE &&
                       !IFELSE_ELSE_BLOCK(STMTS_STMT(IFELSE_IF_BLOCK(stmt)))) {
                IFELSE_EXPR(stmt) = ASTbinop(
                    IFELSE_EXPR(stmt),
                    IFELSE_EXPR(STMTS_STMT(IFELSE_IF_BLOCK(stmt))), BO_and);
                BINOP_RESOLVED_TY(IFELSE_EXPR(stmt)) = TY_bool;
                IFELSE_EXPR(STMTS_STMT(IFELSE_IF_BLOCK(stmt))) = NULL;

                SWAP(IFELSE_IF_BLOCK(stmt), IFELSE_IF_BLOCK(STMTS_STMT(tmp)));
            }
        } else if (IFELSE_ELSE_BLOCK(stmt)) {
            IFELSE_EXPR(stmt) = ASTmonop(IFELSE_EXPR(stmt), MO_not);
            MONOP_RESOLVED_TY(IFELSE_EXPR(stmt)) = TY_bool;

            node_st *tmp = IFELSE_IF_BLOCK(stmt);
            IFELSE_IF_BLOCK(stmt) = IFELSE_ELSE_BLOCK(stmt);
            IFELSE_ELSE_BLOCK(stmt) = tmp;

            CCNcycleNotify();
        } else {
            TRAVpush(TRAV_ES);

            TRAVexpr(stmt);

            TAKE(STMTS_NEXT(node));
            node = inline_stmts(node, DATA_ES_GET()->stmts);

            TRAVpop();
        }
        break;
    case NT_DOWHILE:
        if (NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BOOL &&
            BOOL_VAL(DOWHILE_EXPR(stmt)) == false) {
            node_st *stmts = DOWHILE_STMTS(stmt);
            DOWHILE_STMTS(stmt) = NULL;
            TAKE(STMTS_NEXT(node));
            node = inline_stmts(node, stmts);
        }
        break;
    case NT_CALL: {
        funtable_ref r = {CALL_N(stmt), CALL_L(stmt)};
        funtable_entry *e = funtable_get(DATA_AOTCF_GET()->funtable, r);
        if (!e->side_effects) {
            TRAVpush(TRAV_ES);

            TRAVexprs(stmt);

            TAKE(STMTS_NEXT(node));
            node = inline_stmts(node, DATA_ES_GET()->stmts);

            TRAVpop();
        }
        break;
    }
    default:
        break;
    }

    return node;
}
