#include "ccn/ccn.h"
#include "palm/str.h"
#include "table/table.h"

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        node = tmp;                                                            \
    }

static node_st *DAArev_exprs(node_st *root) {
    node_st *curr = root;
    node_st *prev = NULL;

    while (curr) {
        node_st *next = EXPRS_NEXT(curr);
        EXPRS_NEXT(curr) = prev;
        prev = curr;
        curr = next;
    }

    return prev;
}

static node_st *DAAbuild_array_assign(node_st *ref, vartable_ref *dims,
                                      node_st *expr, int ref_dims) {
    if (ref_dims == EXPR_RESOLVED_DIMS(expr)) {
        if (ref_dims == 0) {
            VARREF_EXPRS(ref) = DAArev_exprs(VARREF_EXPRS(ref));
            return ASTstmts(ASTassign(ref, expr), NULL);
        } else {
            node_st *stmts = NULL;
            node_st *head = NULL;

            int i = 0;
            for (node_st *inner = expr; inner;
                 inner = ARREXPRS_NEXT(inner), i++) {
                node_st *inner_ref = CCNcopy(ref);

                VARREF_EXPRS(inner_ref) =
                    ASTexprs(ASTint(i), VARREF_EXPRS(inner_ref));

                node_st *stmt = DAAbuild_array_assign(
                    inner_ref, dims + 1, ARREXPRS_EXPR(inner), ref_dims - 1);
                ARREXPRS_EXPR(inner) = NULL;

                if (stmts) {
                    STMTS_NEXT(stmts) = stmt;
                } else {
                    stmts = stmt;
                    head = stmt;
                }

                while (STMTS_NEXT(stmts)) {
                    stmts = STMTS_NEXT(stmts);
                }
            }

            CCNfree(ref);
            CCNfree(expr);

            return head;
        }
    } else {
        node_st *loopvar = vartable_temp_var(DATA_DAA_GET()->vartable, TY_int);
        VARREF_EXPRS(ref) = ASTexprs(loopvar, VARREF_EXPRS(ref));

        node_st *id = CCNcopy(VARREF_ID(loopvar));

        node_st *stmt =
            DAAbuild_array_assign(ref, dims + 1, expr, ref_dims - 1);

        vartable_entry *e = vartable_get(DATA_DAA_GET()->vartable, dims[0]);
        node_st *ref = ASTvarref(ASTid(STRcpy(e->name)), NULL);
        VARREF_N(ref) = dims[0].n;
        VARREF_L(ref) = dims[0].l;
        VARREF_RESOLVED_TY(ref) = TY_int;

        return ASTstmts(ASTfor(id, ASTint(0), ref, ASTint(1), stmt), NULL);
    }
}

void DAAinit(void) {}
void DAAfini(void) {}

node_st *DAAprogram(node_st *node) {
    DATA_DAA_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DAAfundecl(node_st *node) {
    DATA_DAA_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DAA_GET()->vartable = DATA_DAA_GET()->vartable->parent;

    return node;
}

node_st *DAAstmts(node_st *node) {
    STMTS_NEXT(node) = TRAVopt(STMTS_NEXT(node));
    STMTS_STMT(node) = TRAVdo(STMTS_STMT(node));

    if (!STMTS_STMT(node)) {
        if (STMTS_NEXT(node)) {
            TAKE(STMTS_NEXT(node));
        } else {
            node = CCNfree(node);
        }
    }

    if (DATA_DAA_GET()->stmts) {
        node_st *tmp = DATA_DAA_GET()->stmts;
        while (STMTS_NEXT(tmp)) {
            tmp = STMTS_NEXT(tmp);
        }
        STMTS_NEXT(tmp) = node;

        node = DATA_DAA_GET()->stmts;
        DATA_DAA_GET()->stmts = NULL;
    }

    return node;
}

node_st *DAAassign(node_st *node) {
    if (NODE_TYPE(ASSIGN_EXPR(node)) == NT_MALLOC ||
        EXPR_RESOLVED_DIMS(ASSIGN_REF(node)) == 0) {
        return node;
    }

    TRAVchildren(node);

    if (NODE_TYPE(ASSIGN_EXPR(node)) != NT_ARREXPRS) {
        node_st *expr = ASSIGN_EXPR(node);
        node_st *ref =
            vartable_temp_var(DATA_DAA_GET()->vartable, EXPR_RESOLVED_TY(expr));
        node_st *assign = ASTassign(CCNcopy(ref), expr);
        ASSIGN_EXPR(node) = ref;

        DATA_DAA_GET()->stmts = ASTstmts(assign, NULL);
    }

    vartable_ref r = {VARREF_N(ASSIGN_REF(node)), VARREF_L(ASSIGN_REF(node))};
    vartable_entry *e = vartable_get(DATA_DAA_GET()->vartable, r);

    node_st *stmt = DAAbuild_array_assign(ASSIGN_REF(node), e->ty.buf,
                                          ASSIGN_EXPR(node), e->ty.len);

    if (DATA_DAA_GET()->stmts) {
        node_st *tmp = DATA_DAA_GET()->stmts;
        while (STMTS_NEXT(tmp)) {
            tmp = STMTS_NEXT(tmp);
        }
        STMTS_NEXT(tmp) = stmt;
    } else {
        DATA_DAA_GET()->stmts = stmt;
    }

    ASSIGN_REF(node) = NULL;
    ASSIGN_EXPR(node) = NULL;

    return CCNfree(node);
}

node_st *DAAarrexprs(node_st *node) {
    TRAVchildren(node);

    if (NODE_TYPE(ARREXPRS_EXPR(node)) == NT_ARREXPRS ||
        NODE_TYPE(ARREXPRS_EXPR(node)) == NT_INT ||
        NODE_TYPE(ARREXPRS_EXPR(node)) == NT_FLOAT ||
        NODE_TYPE(ARREXPRS_EXPR(node)) == NT_BOOL ||
        (NODE_TYPE(ARREXPRS_EXPR(node)) == NT_VARREF &&
         !VARREF_EXPRS(ARREXPRS_EXPR(node)))) {
        return node;
    }

    node_st *expr = ARREXPRS_EXPR(node);
    node_st *ref =
        vartable_temp_var(DATA_DAA_GET()->vartable, EXPR_RESOLVED_TY(expr));
    node_st *assign = ASTassign(CCNcopy(ref), expr);
    ARREXPRS_EXPR(node) = ref;

    DATA_DAA_GET()->stmts = ASTstmts(assign, DATA_DAA_GET()->stmts);

    return node;
}
