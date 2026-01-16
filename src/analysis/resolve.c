#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "error/error.h"
#include "error/span.h"
#include "table/table.h"

static void ARlint_vartable_usage(vartable *vartable) {
    for (int i = 0; i < vartable->len; i++) {
        vartable_entry *e = &vartable->buf[i];
        if (e->loopvar || e->exported) {
            continue;
        }

        if (e->read_count == 0) {
            if (e->write_count != 0) {
                if (!e->external && (!e->param || e->ty.len == 0)) {
                    emit_message_with_span(
                        e->span, L_WARNING,
                        "variable '%s' is never read from, only written to",
                        e->name);
                }
            } else {
                emit_message_with_span(
                    e->span, L_WARNING,
                    "variable '%s' is never read from or written to", e->name);
            }
        }
    }
}

static void ARlint_funtable_usage(funtable *funtable) {
    for (int i = 0; i < funtable->len; i++) {
        funtable_entry *e = &funtable->buf[i];
        if (e->exported) {
            continue;
        }

        if (e->call_count == 0) {
            emit_message_with_span(e->span, L_WARNING,
                                   "function '%s' is never called", e->name);
        }
    }
}

void ARinit(void) {}

void ARfini(void) {}

node_st *ARprogram(node_st *node) {
    DATA_AR_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AR_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    ARlint_funtable_usage(DATA_AR_GET()->funtable);
    ARlint_vartable_usage(DATA_AR_GET()->vartable);

    return node;
}

node_st *ARfundecl(node_st *node) {
    DATA_AR_GET()->vartable = vartable_new(DATA_AR_GET()->vartable);

    TRAVchildren(node);
    FUNDECL_VARTABLE(node) = DATA_AR_GET()->vartable;
    if (FUNDECL_BODY(node)) {
        ARlint_vartable_usage(DATA_AR_GET()->vartable);
    }

    DATA_AR_GET()->vartable = DATA_AR_GET()->vartable->parent;

    return node;
}

node_st *ARfunbody(node_st *node) {
    DATA_AR_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);
    ARlint_funtable_usage(DATA_AR_GET()->funtable);

    DATA_AR_GET()->funtable = DATA_AR_GET()->funtable->parent;

    return node;
}

node_st *ARparam(node_st *node) {
    vartype ty = vartype_new(TYPE_TY(PARAM_TY(node)));

    for (node_st *expr = TYPE_EXPRS(PARAM_TY(node)); expr;
         expr = EXPRS_NEXT(expr)) {
        vartable_entry e = {ID_VAL(VARREF_ID(EXPRS_EXPR(expr))),
                            vartype_new(TY_int),
                            SPAN(VARREF_ID(EXPRS_EXPR(expr))),
                            0,
                            0,
                            false,
                            false,
                            true,
                            false};
        vartable_ref r = vartable_insert(DATA_AR_GET()->vartable, e,
                                         VARREF_ID(EXPRS_EXPR(expr)));
        vartype_push(&ty, r);
    }

    vartable_entry e = {ID_VAL(PARAM_ID(node)),
                        ty,
                        SPAN(PARAM_ID(node)),
                        0,
                        0,
                        false,
                        false,
                        true,
                        false};
    vartable_insert(DATA_AR_GET()->vartable, e, PARAM_ID(node));

    return node;
}

node_st *ARvardecl(node_st *node) {
    if (!DATA_AR_GET()->globals && VARDECL_GLOBAL(node)) {
        return node;
    }

    TRAVchildren(node);

    vartype ty = vartype_new(TYPE_TY(VARDECL_TY(node)));
    for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
         expr = EXPRS_NEXT(expr)) {
        vartype_push(&ty, (vartable_ref){-1, -1});
    }

    vartable_entry e = {ID_VAL(VARDECL_ID(node)),
                        ty,
                        SPAN(VARDECL_ID(node)),
                        0,
                        VARDECL_EXPR(node) ? 1 : 0,
                        false,
                        VARDECL_EXPORTED(node),
                        false,
                        false};
    VARDECL_L(node) =
        vartable_insert(DATA_AR_GET()->vartable, e, VARDECL_ID(node)).l;

    return node;
}

node_st *ARfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    vartable_entry e = {ID_VAL(FOR_ID(node)),
                        vartype_new(TY_int),
                        SPAN(FOR_ID(node)),
                        0,
                        0,
                        false,
                        false,
                        false,
                        false};
    vartable_ref r = vartable_push(DATA_AR_GET()->vartable, e);

    TRAVstmts(node);

    vartable_get(DATA_AR_GET()->vartable, r)->loopvar = true;

    return node;
}

node_st *ARcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = funtable_resolve(DATA_AR_GET()->funtable, node);
    CALL_N(node) = r.n;
    CALL_L(node) = r.l;

    funtable_entry *e = funtable_get(DATA_AR_GET()->funtable, r);
    if (e->ty.ty != TY_error) {
        e->call_count++;
    }

    return node;
}

node_st *ARvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = vartable_resolve(DATA_AR_GET()->vartable, VARREF_ID(node));
    VARREF_N(node) = r.n;
    VARREF_L(node) = r.l;

    vartable_entry *e = vartable_get(DATA_AR_GET()->vartable, r);
    if (e->ty.ty != TY_error) {
        if (VARREF_WRITE(node)) {
            e->write_count++;
        } else {
            e->read_count++;
        }
    }

    return node;
}
