#include "ccn/ccn.h"

void DAPinit(void) {}
void DAPfini(void) {}

node_st *DAPprogram(node_st *node) {
    DATA_DAP_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DAPfundecl(node_st *node) {
    DATA_DAP_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DAP_GET()->vartable = DATA_DAP_GET()->vartable->parent;

    return node;
}

node_st *DAPparams(node_st *node) {
    TRAVchildren(node);

    node_st *head = NULL;
    node_st *params = NULL;

    node_st *param = PARAMS_PARAM(node);

    for (node_st *expr = TYPE_EXPRS(PARAM_TY(param)); expr;
         expr = EXPRS_NEXT(expr)) {
        node_st *param = ASTparams(
            ASTparam(ASTtype(NULL, TY_int, false), VARREF_ID(EXPRS_EXPR(expr))),
            NULL);
        VARREF_ID(EXPRS_EXPR(expr)) = NULL;

        if (params) {
            PARAMS_NEXT(params) = param;
        } else {
            head = param;
        }
        params = param;
    }

    if (params) {
        PARAMS_NEXT(params) = node;
        node = head;
    }

    TYPE_EXPRS(PARAM_TY(param)) = CCNfree(TYPE_EXPRS(PARAM_TY(param)));

    return node;
}

node_st *DAPexprs(node_st *node) {
    TRAVchildren(node);

    node_st *head = NULL;
    node_st *params = NULL;

    if (EXPR_RESOLVED_DIMS(EXPRS_EXPR(node)) == 0) {
        return node;
    }

    vartable_ref r = {VARREF_N(EXPRS_EXPR(node)), VARREF_L(EXPRS_EXPR(node))};
    vartable_entry *e = vartable_get(DATA_DAP_GET()->vartable, r);

    for (int i = 0; i < e->ty.len; i++) {
        vartable_ref dr = e->ty.buf[i];
        dr.n += r.n;
        node_st *param =
            ASTexprs(vartable_get_ref(DATA_DAP_GET()->vartable, dr), NULL);

        if (params) {
            EXPRS_NEXT(params) = param;
        } else {
            head = param;
        }
        params = param;
    }

    if (params) {
        EXPRS_NEXT(params) = node;
        node = head;
    }

    return node;
}
