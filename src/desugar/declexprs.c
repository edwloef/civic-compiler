#include "ccn/ccn.h"
#include "table/table.h"

void DDEinit(void) {}
void DDEfini(void) {}

node_st *DDEprogram(node_st *node) {
    DATA_DDE_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DDEdecls(node_st *node) {
    DECLS_NEXT(node) = TRAVopt(DECLS_NEXT(node));
    DECLS_DECL(node) = TRAVdo(DECLS_DECL(node));

    if (DATA_DDE_GET()->decls) {
        node_st *tmp;
        for (tmp = DATA_DDE_GET()->decls; DECLS_NEXT(tmp);
             tmp = DECLS_NEXT(tmp))
            ;
        DECLS_NEXT(tmp) = node;
        node = DATA_DDE_GET()->decls;
        DATA_DDE_GET()->decls = NULL;
    }

    return node;
}

node_st *DDEfundecl(node_st *node) {
    DATA_DDE_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DDE_GET()->vartable = DATA_DDE_GET()->vartable->parent;

    return node;
}

node_st *DDEfunbody(node_st *node) {
    FUNBODY_DECLS(node) = TRAVopt(FUNBODY_DECLS(node));

    return node;
}

node_st *DDEexprs(node_st *node) {
    EXPRS_NEXT(node) = TRAVopt(EXPRS_NEXT(node));
    EXPRS_EXPR(node) = TRAVdo(EXPRS_EXPR(node));

    if (NODE_TYPE(EXPRS_EXPR(node)) == NT_INT ||
        (NODE_TYPE(EXPRS_EXPR(node)) == NT_VARREF &&
         !VARREF_EXPRS(EXPRS_EXPR(node)))) {
        return node;
    }

    vartype ty = {EXPR_RESOLVED_TY(EXPRS_EXPR(node)), 0};
    node_st *ref = vartable_temp_var(DATA_DDE_GET()->vartable, ty);
    node_st *decl = ASTvardecl(ASTtype(NULL, ty.ty), CCNcopy(VARREF_ID(ref)),
                               EXPRS_EXPR(node));
    EXPRS_EXPR(node) = ref;

    DATA_DDE_GET()->decls = ASTdecls(decl, DATA_DDE_GET()->decls);

    return node;
}
