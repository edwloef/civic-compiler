#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "macros.h"
#include "utils.h"

void AODSinit(void) {}
void AODSfini(void) {}

node_st *AODSprogram(node_st *node) {
    DATA_AODS_GET()->funtable = PROGRAM_FUNTABLE(node);
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

node_st *AODSfunbody(node_st *node) {
    DATA_AODS_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AODS_GET()->funtable = DATA_AODS_GET()->funtable->parent;

    return node;
}

node_st *AODSstmts(node_st *node) {
    TRAVnext(node);

    bool outer = DATA_AODS_GET()->outer_loop == NULL;

    node_st *stmt = STMTS_STMT(node);
    switch (NODE_TYPE(stmt)) {
    case NT_ASSIGN:
        if (!VARREF_EXPRS(ASSIGN_REF(stmt)) &&
            !EXPR_RESOLVED_DIMS(ASSIGN_REF(stmt))) {
            vartable_ref r = {VARREF_N(ASSIGN_REF(stmt)),
                              VARREF_L(ASSIGN_REF(stmt))};
            vartable_entry *e = vartable_get(DATA_AODS_GET()->vartable, r);
            bool assign_is_dead = e->read_count == 0;

            if (!assign_is_dead && NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF) {
                vartable_ref er = {VARREF_N(ASSIGN_EXPR(stmt)),
                                   VARREF_L(ASSIGN_EXPR(stmt))};
                assign_is_dead = r.n == er.n && r.l == er.l;
            }

            if (!assign_is_dead && VARREF_N(ASSIGN_REF(stmt)) == 0) {
                funtable *funtable = DATA_AODS_GET()->funtable;
                vartable *vartable = DATA_AODS_GET()->vartable;

                node_st *parent = DATA_AODS_GET()->parent;
                node_st *trav = DATA_AODS_GET()->outer_loop;
                trav = trav ? trav : node;

                TRAVpush(TRAV_CD);

                DATA_CD_GET()->ref = ASSIGN_REF(stmt);
                DATA_CD_GET()->funtable = funtable;
                DATA_CD_GET()->vartable = vartable;

                TRAVopt(trav);
                TRAVopt(parent);

                assign_is_dead = DATA_CD_GET()->assign_is_dead;

                TRAVpop();
            }

            if (assign_is_dead) {
                TRAVpush(TRAV_ES);

                TRAVexpr(stmt);

                TAKE(STMTS_NEXT(node));
                node = inline_stmts(node, DATA_ES_GET()->stmts);

                TRAVpop();
            }
        }
        break;
    case NT_DOWHILE:
        if (outer) {
            if (NODE_TYPE(stmt) == NT_DOWHILE) {
                DATA_AODS_GET()->outer_loop =
                    ASTscope(DOWHILE_EXPR(stmt), DOWHILE_STMTS(stmt), NULL);
            } else {
                DATA_AODS_GET()->outer_loop =
                    ASTscope(NULL, FOR_STMTS(stmt), NULL);
            }
        }
        TRAVstmt(node);
        if (outer) {
            SCOPE_EXPR(DATA_AODS_GET()->outer_loop) = NULL;
            SCOPE_STMTS(DATA_AODS_GET()->outer_loop) = NULL;
            DATA_AODS_GET()->outer_loop = CCNfree(DATA_AODS_GET()->outer_loop);
        }
        break;
    case NT_IFELSE:
        if (outer) {
            DATA_AODS_GET()->parent =
                ASTscope(NULL, STMTS_NEXT(node), DATA_AODS_GET()->parent);
        }
        TRAVstmt(node);
        if (outer) {
            node_st *parent = DATA_AODS_GET()->parent;
            DATA_AODS_GET()->parent = SCOPE_PARENT(parent);
            SCOPE_STMTS(parent) = NULL;
            SCOPE_PARENT(parent) = NULL;
            CCNfree(parent);
        }
        break;
    default:
        TRAVstmt(node);
        break;
    }

    return node;
}
