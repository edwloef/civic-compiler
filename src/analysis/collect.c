#include "ccn/ccn.h"
#include "resolve.h"

void ACinit(void) {
    DATA_AC_GET()->funtable = funtable_new(NULL);
    DATA_AC_GET()->vartable = vartable_new(NULL);
}

void ACfini(void) {}

node_st *ACprogram(node_st *node) {
    TRAVchildren(node);

    PROGRAM_FUNTABLE(node) = DATA_AC_GET()->funtable;
    PROGRAM_VARTABLE(node) = DATA_AC_GET()->vartable;

    return node;
}

node_st *ACfundecl(node_st *node) {
    funtype ty = funtype_new(FUNDECL_TY(node));
    node_st *arg = FUNDECL_PARAMS(node);
    while (arg) {
        int dims = 0;
        node_st *id = TYPE_EXPRS(PARAM_TY(PARAMS_PARAM(arg)));
        while (id) {
            dims++;
            id = EXPRS_NEXT(id);
        }

        vartype e = {TYPE_TY(PARAM_TY(PARAMS_PARAM(arg))), dims};
        funtype_push(&ty, e);
        arg = PARAMS_NEXT(arg);
    }

    funtable_entry e = {ID_VAL(FUNDECL_ID(node)), ty};
    funtable_insert(DATA_AC_GET()->funtable, e, FUNDECL_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *ACfunbody(node_st *node) {
    DATA_AC_GET()->funtable = funtable_new(DATA_AC_GET()->funtable);

    TRAVchildren(node);
    FUNBODY_FUNTABLE(node) = DATA_AC_GET()->funtable;

    DATA_AC_GET()->funtable = DATA_AC_GET()->funtable->parent;

    return node;
}

node_st *ACvardecl(node_st *node) {
    if (VARDECL_GLOBAL(node)) {
        if (VARDECL_EXTERNAL(node)) {
            for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
                 expr = EXPRS_NEXT(expr)) {
                vartable_entry e = {
                    ID_VAL(VARREF_ID(EXPRS_EXPR(expr))), {TY_int, 0}, false};
                vartable_insert(DATA_AC_GET()->vartable, e,
                                VARREF_ID(EXPRS_EXPR(expr)));
            }
        } else {
            TRAVpush(TRAV_AR);
            TRAVchildren(node);
            TRAVpop();
        }

        int dims = 0;
        for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
             expr = EXPRS_NEXT(expr)) {
            dims++;
        }

        vartable_entry e = {
            ID_VAL(VARDECL_ID(node)), {TYPE_TY(VARDECL_TY(node)), dims}, false};
        vartable_insert(DATA_AC_GET()->vartable, e, VARDECL_ID(node));

        VARDECL_L(node) = DATA_AC_GET()->vartable->len - 1;
    }

    return node;
}
