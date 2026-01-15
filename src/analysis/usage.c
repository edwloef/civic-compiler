#include "ccn/ccn.h"
#include "error/error.h"

void AUinit(void) {}
void AUfini(void) {
    abort_on_error();
}

static void AUlint_vartable(vartable *vartable) {
    for (int i = 0; i < vartable->len; i++) {
        vartable_entry *e = &vartable->buf[i];
        if (e->loopvar || e->exported) {
            continue;
        }

        if (e->read_count == 0) {
            if (e->write_count != 0) {
                if (!e->external) {
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

static void AUlint_funtable(funtable *funtable) {
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

node_st *AUprogram(node_st *node) {
    DATA_AU_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AU_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    AUlint_vartable(DATA_AU_GET()->vartable);
    AUlint_funtable(DATA_AU_GET()->funtable);

    return node;
}

node_st *AUfundecl(node_st *node) {
    DATA_AU_GET()->vartable = FUNDECL_VARTABLE(node);

    if (FUNDECL_BODY(node)) {
        TRAVchildren(node);
        AUlint_vartable(DATA_AU_GET()->vartable);
    }

    DATA_AU_GET()->vartable = DATA_AU_GET()->vartable->parent;

    return node;
}

node_st *AUfunbody(node_st *node) {
    DATA_AU_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);
    AUlint_funtable(DATA_AU_GET()->funtable);

    DATA_AU_GET()->funtable = DATA_AU_GET()->funtable->parent;

    return node;
}

node_st *AUassign(node_st *node) {
    vartable_ref r = {VARREF_N(ASSIGN_REF(node)), VARREF_L(ASSIGN_REF(node))};
    vartable_entry *e = vartable_get(DATA_AU_GET()->vartable, r);
    if (e->ty.ty != TY_error) {
        e->write_count++;
    }

    ASSIGN_EXPR(node) = TRAVdo(ASSIGN_EXPR(node));
    VARREF_EXPRS(ASSIGN_REF(node)) = TRAVopt(VARREF_EXPRS(ASSIGN_REF(node)));

    return node;
}

node_st *AUcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_AU_GET()->funtable, r);
    if (e->ty.ty != TY_error) {
        e->call_count++;
    }

    return node;
}

node_st *AUvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {VARREF_N(node), VARREF_L(node)};
    vartable_entry *e = vartable_get(DATA_AU_GET()->vartable, r);
    if (e->ty.ty != TY_error) {
        e->read_count++;
    }

    return node;
}
