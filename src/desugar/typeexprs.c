#include "ccn/ccn.h"
#include "palm/str.h"
#include "table/vartable.h"

static node_st *DTEtempvar(void) {
    char *name = STRfmt("_%d", DATA_DTE_GET()->vartable->len);
    vartable_entry e = {name, {TY_int, 0}, {0, 0, 0, 0}, false};
    vartable_push(DATA_DTE_GET()->vartable, e);

    return ASTid(name);
}

void DTEinit(void) {}
void DTEfini(void) {}

node_st *DTEprogram(node_st *node) {
    DATA_DTE_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DTEdecls(node_st *node) {
    TRAVchildren(node);

    node_st *decl = DECLS_DECL(node);
    if (NODE_TYPE(decl) == NT_VARDECL) {
        for (node_st *expr = TYPE_EXPRS(VARDECL_TY(decl)); expr;
             expr = EXPRS_NEXT(expr)) {
            if (NODE_TYPE(EXPRS_EXPR(expr)) == NT_INT ||
                (NODE_TYPE(EXPRS_EXPR(expr)) == NT_VARREF)) {
                continue;
            }

            node_st *id = DTEtempvar();
            node_st *assign =
                ASTvardecl(ASTtype(NULL, TY_int), id, EXPRS_EXPR(expr));
            EXPRS_EXPR(expr) = ASTvarref(CCNcopy(id), NULL);
            node = ASTdecls(assign, node);
        }
    }

    return node;
}

node_st *DTEfundecl(node_st *node) {
    DATA_DTE_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DTE_GET()->vartable = DATA_DTE_GET()->vartable->parent;

    return node;
}
