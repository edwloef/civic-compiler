#include "ccn/ccn.h"
#include "table/vartable.h"

void AGinit(void) {}
void AGfini(void) {}

node_st *AGprogram(node_st *node) {
    DATA_AG_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AG_GET()->vartable = vartable_new(NULL);

    TRAVchildren(node);

    PROGRAM_VARTABLE(node) = DATA_AG_GET()->vartable;

    return node;
}

node_st *AGvardecl(node_st *node) {
    if (!VARDECL_GLOBAL(node)) {
        return node;
    }

    if (!VARDECL_EXTERNAL(node)) {
        funtable *funtable = DATA_AG_GET()->funtable;
        vartable *vartable = DATA_AG_GET()->vartable;

        TRAVpush(TRAV_AR);

        DATA_AR_GET()->globals = true;
        DATA_AR_GET()->funtable = funtable;
        DATA_AR_GET()->vartable = vartable;

        node = TRAVdo(node);

        TRAVpop();

        return node;
    }

    vartype ty = vartype_new(TYPE_TY(VARDECL_TY(node)));

    for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
         expr = EXPRS_NEXT(expr)) {
        vartable_entry e = {ID_VAL(VARREF_ID(EXPRS_EXPR(expr))),
                            vartype_new(TY_int),
                            SPAN(VARREF_ID(EXPRS_EXPR(expr))),
                            0,
                            0,
                            0,
                            true,
                            false,
                            false,
                            false,
                            false};
        vartable_ref r = vartable_insert(DATA_AG_GET()->vartable, e,
                                         VARREF_ID(EXPRS_EXPR(expr)));
        vartype_push(&ty, r);
    }

    vartable_entry e = {ID_VAL(VARDECL_ID(node)),
                        ty,
                        SPAN(VARDECL_ID(node)),
                        0,
                        0,
                        0,
                        true,
                        false,
                        false,
                        false,
                        false};
    VARDECL_L(node) =
        vartable_insert(DATA_AG_GET()->vartable, e, VARDECL_ID(node)).l;

    return node;
}
