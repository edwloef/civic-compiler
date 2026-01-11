#include "analysis/resolve.h"
#include "analysis/span.h"
#include "ccn/ccn.h"
#include "ccngen/trav.h"

void ARinit(void) {}

void ARfini(void) {}

node_st *ARprogram(node_st *node) {
    DATA_AR_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AR_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *ARfundecl(node_st *node) {
    DATA_AR_GET()->vartable = vartable_new(DATA_AR_GET()->vartable);

    TRAVchildren(node);
    FUNDECL_VARTABLE(node) = DATA_AR_GET()->vartable;

    DATA_AR_GET()->vartable = DATA_AR_GET()->vartable->parent;

    return node;
}

node_st *ARfunbody(node_st *node) {
    DATA_AC_GET()->nesting_level++;
    DATA_AR_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AC_GET()->nesting_level--;
    DATA_AR_GET()->funtable = DATA_AR_GET()->funtable->parent;

    return node;
}

node_st *ARparam(node_st *node) {
    for (node_st *expr = TYPE_EXPRS(PARAM_TY(node)); expr;
         expr = EXPRS_NEXT(expr)) {
        vartable_entry e = {ID_VAL(VARREF_ID(EXPRS_EXPR(expr))),
                            {TY_int, 0},
                            DATA_AR_GET()->nesting_level,
                            SPAN(VARREF_ID(EXPRS_EXPR(expr))),
                            false};
        vartable_insert(DATA_AR_GET()->vartable, e,
                        VARREF_ID(EXPRS_EXPR(expr)));
    }

    int dims = 0;
    for (node_st *expr = TYPE_EXPRS(PARAM_TY(node)); expr;
         expr = EXPRS_NEXT(expr)) {
        dims++;
    }

    vartable_entry e = {ID_VAL(PARAM_ID(node)),
                        {TYPE_TY(PARAM_TY(node)), dims},
                        DATA_AR_GET()->nesting_level,
                        SPAN(PARAM_ID(node)),
                        false};
    vartable_insert(DATA_AR_GET()->vartable, e, PARAM_ID(node));

    return node;
}

node_st *ARvardecl(node_st *node) {
    if (!VARDECL_GLOBAL(node)) {
        TRAVchildren(node);

        int dims = 0;
        for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
             expr = EXPRS_NEXT(expr)) {
            dims++;
        }

        vartable_entry e = {ID_VAL(VARDECL_ID(node)),
                            {TYPE_TY(VARDECL_TY(node)), dims},
                            DATA_AR_GET()->nesting_level,
                            SPAN(VARDECL_ID(node)),
                            false};
        vartable_insert(DATA_AR_GET()->vartable, e, VARDECL_ID(node));

        VARDECL_L(node) = DATA_AR_GET()->vartable->len - 1;
    }

    return node;
}

node_st *ARfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    vartable_entry e = {ID_VAL(FOR_ID(node)),
                        {TY_int, 0},
                        DATA_AR_GET()->nesting_level,
                        SPAN(FOR_ID(node)),
                        false};
    vartable_push(DATA_AR_GET()->vartable, e);

    vartable_ref r = {0, DATA_AR_GET()->vartable->len - 1};

    TRAVstmts(node);

    vartable_get(DATA_AR_GET()->vartable, r)->loopvar = true;

    return node;
}

node_st *ARcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = funtable_resolve(DATA_AR_GET()->funtable, node);
    CALL_N(node) = r.n;
    CALL_L(node) = r.l;

    return node;
}

node_st *ARvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = vartable_resolve(DATA_AR_GET()->vartable, VARREF_ID(node));
    VARREF_N(node) = r.n;
    VARREF_L(node) = r.l;

    return node;
}
