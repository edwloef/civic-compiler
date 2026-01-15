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
    TRAVchildren(node);

    node_st *decl = DECLS_DECL(node);
    if (NODE_TYPE(decl) == NT_VARDECL) {
        node_st *root = NULL;
        node_st *decls = NULL;

        for (node_st *expr = TYPE_EXPRS(VARDECL_TY(decl)); expr;
             expr = EXPRS_NEXT(expr)) {
            if (NODE_TYPE(EXPRS_EXPR(expr)) == NT_VARREF &&
                !VARREF_EXPRS(EXPRS_EXPR(expr))) {
                continue;
            }

            node_st *ref = vartable_temp_var(DATA_DDE_GET()->vartable,
                                             (vartype){TY_int, 0});
            node_st *decl =
                ASTvardecl(ASTtype(NULL, TY_int), CCNcopy(VARREF_ID(ref)),
                           EXPRS_EXPR(expr));
            EXPRS_EXPR(expr) = ref;

            if (root) {
                DECLS_NEXT(decls) = ASTdecls(decl, NULL);
                decls = DECLS_NEXT(decls);
            } else {
                decls = ASTdecls(decl, NULL);
                root = decls;
            }
        }

        if (decls) {
            DECLS_NEXT(decls) = node;
            node = root;
        }
    }

    return node;
}

node_st *DDEfundecl(node_st *node) {
    DATA_DDE_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DDE_GET()->vartable = DATA_DDE_GET()->vartable->parent;

    return node;
}
