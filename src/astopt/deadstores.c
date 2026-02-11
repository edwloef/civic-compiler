#include "ccn/ccn.h"
#include "ccngen/trav.h"
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

    DATA_AODS_GET()->next = STMTS_NEXT(node);

    TRAVstmt(node);

    node_st *stmt = STMTS_STMT(node);
    if (!stmt || NODE_TYPE(stmt) == NT_STMTS) {
        STMTS_STMT(node) = NULL;
        TAKE(STMTS_NEXT(node));
        node = inline_stmts(node, stmt);
    }

    return node;
}

node_st *AODSassign(node_st *node) {
    if (VARREF_EXPRS(ASSIGN_REF(node)) ||
        EXPR_RESOLVED_DIMS(ASSIGN_REF(node))) {
        return node;
    }

    vartable_ref r = {VARREF_N(ASSIGN_REF(node)), VARREF_L(ASSIGN_REF(node))};
    vartable_entry *e = vartable_get(DATA_AODS_GET()->vartable, r);
    bool assign_is_dead = e->read_count == 0;

    if (!assign_is_dead && NODE_TYPE(ASSIGN_EXPR(node)) == NT_VARREF) {
        assign_is_dead = r.n == VARREF_N(ASSIGN_EXPR(node)) &&
                         r.l == VARREF_L(ASSIGN_EXPR(node));
    }

    if (!assign_is_dead && VARREF_N(ASSIGN_REF(node)) == 0) {
        funtable *funtable = DATA_AODS_GET()->funtable;
        vartable *vartable = DATA_AODS_GET()->vartable;

        node_st *next = DATA_AODS_GET()->next;
        node_st *parent = DATA_AODS_GET()->parent;
        node_st *outer_loop = DATA_AODS_GET()->outer_loop;

        TRAVpush(TRAV_CD);

        DATA_CD_GET()->ref = ASSIGN_REF(node);
        DATA_CD_GET()->funtable = funtable;
        DATA_CD_GET()->vartable = vartable;

        if (outer_loop) {
            TRAVdo(outer_loop);

            bool prev = DATA_CD_GET()->ref_is_dead;

            TRAVdo(outer_loop);

            DATA_CD_GET()->ref_is_dead = prev;
        } else {
            TRAVdo(node);
            TRAVopt(next);
        }

        TRAVopt(parent);

        assign_is_dead = DATA_CD_GET()->assign_is_dead;

        TRAVpop();
    }

    if (assign_is_dead) {
        TRAVpush(TRAV_ES);

        TRAVexpr(node);

        node = CCNfree(node);
        node = DATA_ES_GET()->stmts;

        TRAVpop();
    }

    return node;
}

node_st *AODSifelse(node_st *node) {
    bool outer = DATA_AODS_GET()->outer_loop == NULL;

    if (outer) {
        DATA_AODS_GET()->parent =
            ASTscope(NULL, DATA_AODS_GET()->next, DATA_AODS_GET()->parent);
    }

    TRAVchildren(node);

    if (outer) {
        node_st *parent = DATA_AODS_GET()->parent;
        DATA_AODS_GET()->parent = SCOPE_PARENT(parent);
        SCOPE_STMTS(parent) = NULL;
        SCOPE_PARENT(parent) = NULL;
        CCNfree(parent);
    }

    return node;
}

node_st *AODSdowhile(node_st *node) {
    bool outer = DATA_AODS_GET()->outer_loop == NULL;

    if (outer) {
        DATA_AODS_GET()->outer_loop =
            ASTscope(DOWHILE_EXPR(node), DOWHILE_STMTS(node), NULL);
    }

    TRAVchildren(node);

    if (outer) {
        SCOPE_EXPR(DATA_AODS_GET()->outer_loop) = NULL;
        SCOPE_STMTS(DATA_AODS_GET()->outer_loop) = NULL;
        DATA_AODS_GET()->outer_loop = CCNfree(DATA_AODS_GET()->outer_loop);
    }

    return node;
}