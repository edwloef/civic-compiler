#include "ccn/ccn.h"
#include "table/table.h"

void DSEinit(void) {}
void DSEfini(void) {}

node_st *DSEprogram(node_st *node) {
    DATA_DSE_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DSEstmts(node_st *node) {
    STMTS_NEXT(node) = TRAVopt(STMTS_NEXT(node));
    STMTS_STMT(node) = TRAVdo(STMTS_STMT(node));

    if (DATA_DSE_GET()->stmts) {
        node_st *tmp;
        for (tmp = DATA_DSE_GET()->stmts; STMTS_NEXT(tmp);
             tmp = STMTS_NEXT(tmp))
            ;
        STMTS_NEXT(tmp) = node;
        node = DATA_DSE_GET()->stmts;
        DATA_DSE_GET()->stmts = NULL;
    }

    return node;
}

node_st *DSEfundecl(node_st *node) {
    DATA_DSE_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DSE_GET()->vartable = DATA_DSE_GET()->vartable->parent;

    return node;
}

node_st *DSEexprs(node_st *node) {
    EXPRS_NEXT(node) = TRAVopt(EXPRS_NEXT(node));
    EXPRS_EXPR(node) = TRAVdo(EXPRS_EXPR(node));

    if (NODE_TYPE(EXPRS_EXPR(node)) == NT_INT ||
        (NODE_TYPE(EXPRS_EXPR(node)) == NT_VARREF &&
         !VARREF_EXPRS(EXPRS_EXPR(node)))) {
        return node;
    }

    vartype ty = {EXPR_RESOLVED_TY(EXPRS_EXPR(node)), 0};
    node_st *ref = vartable_temp_var(DATA_DSE_GET()->vartable, ty);
    node_st *stmt = ASTassign(CCNcopy(ref), EXPRS_EXPR(node));
    EXPRS_EXPR(node) = ref;

    DATA_DSE_GET()->stmts = ASTstmts(stmt, DATA_DSE_GET()->stmts);

    return node;
}
