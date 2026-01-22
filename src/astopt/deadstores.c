#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "utils.h"

void AODSinit(void) {}
void AODSfini(void) {}

node_st *AODSprogram(node_st *node) {
    DATA_AODS_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AODSfundecl(node_st *node) {
    DATA_AODS_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AODS_GET()->vartable = DATA_AODS_GET()->vartable->parent;

    return node;
}

node_st *AODSstmts(node_st *node) {
    TRAVnext(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_ASSIGN && !VARREF_EXPRS(ASSIGN_REF(stmt)) &&
        NODE_TYPE(ASSIGN_EXPR(stmt)) != NT_MALLOC) {
        vartable_ref r = {VARREF_N(ASSIGN_REF(stmt)),
                          VARREF_L(ASSIGN_REF(stmt))};
        vartable_entry *e = vartable_get(DATA_AODS_GET()->vartable, r);

        bool assign_is_dead = e->read_count == 0;

        if (!assign_is_dead && VARREF_N(ASSIGN_REF(stmt)) == 0) {
            node_st *parent = DATA_AODS_GET()->parent;
            node_st *trav = DATA_AODS_GET()->outer_loop;
            trav = trav ? trav : node;

            TRAVpush(TRAV_CD);

            DATA_CD_GET()->ref = ASSIGN_REF(stmt);

            TRAVopt(trav);
            TRAVopt(parent);

            assign_is_dead = DATA_CD_GET()->assign_is_dead;

            TRAVpop();
        }

        if (assign_is_dead) {
            TRAVpush(TRAV_EC);

            TRAVexpr(stmt);
            node = inline_stmts(node, DATA_EC_GET()->stmts);

            TRAVpop();
        }
    } else if (NODE_TYPE(stmt) == NT_DOWHILE) {
        bool outer = DATA_AODS_GET()->outer_loop == NULL;
        if (outer) {
            DATA_AODS_GET()->outer_loop =
                ASTscope(DOWHILE_EXPR(stmt), DOWHILE_STMTS(stmt), NULL);
        }

        TRAVstmt(node);

        if (outer) {
            SCOPE_EXPR(DATA_AODS_GET()->outer_loop) = NULL;
            SCOPE_STMTS(DATA_AODS_GET()->outer_loop) = NULL;
            DATA_AODS_GET()->outer_loop = CCNfree(DATA_AODS_GET()->outer_loop);
        }
    } else if (NODE_TYPE(stmt) == NT_FOR) {
        bool outer = DATA_AODS_GET()->outer_loop == NULL;
        if (outer) {
            DATA_AODS_GET()->outer_loop = ASTscope(NULL, FOR_STMTS(stmt), NULL);
        }

        TRAVstmt(node);

        if (outer) {
            SCOPE_STMTS(DATA_AODS_GET()->outer_loop) = NULL;
            DATA_AODS_GET()->outer_loop = CCNfree(DATA_AODS_GET()->outer_loop);
        }
    } else if (NODE_TYPE(stmt) == NT_IFELSE && !DATA_AODS_GET()->outer_loop) {
        node_st *parent =
            ASTscope(NULL, STMTS_NEXT(node), DATA_AODS_GET()->parent);
        DATA_AODS_GET()->parent = parent;

        TRAVstmt(node);

        DATA_AODS_GET()->parent = SCOPE_PARENT(parent);
        SCOPE_STMTS(parent) = NULL;
        SCOPE_PARENT(parent) = NULL;
        CCNfree(parent);
    } else {
        TRAVstmt(node);
    }

    return node;
}
