#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/str.h"
#include "utils.h"

void DVDinit(void) {}
void DVDfini(void) {}

node_st *DVDprogram(node_st *node) {
    DATA_DVD_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    node_st *decl = ASTfundecl(NULL, NULL, TY_void);
    node_st *body = ASTfunbody(NULL, DATA_DVD_GET()->stmts);

    FUNDECL_EXPORTED(decl) = true;
    FUNDECL_BODY(decl) = body;

    FUNDECL_VARTABLE(decl) = vartable_new(PROGRAM_VARTABLE(node));
    FUNBODY_FUNTABLE(body) = funtable_new(PROGRAM_FUNTABLE(node));

    funtable_entry e = {STRcpy("__init"),
                        NULL,
                        funtype_new(TY_void),
                        {0, 0, 0, 0, NULL},
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        false,
                        true,
                        false};
    FUNDECL_L(decl) = funtable_push(PROGRAM_FUNTABLE(node), e).l;

    PROGRAM_DECLS(node) = ASTdecls(decl, PROGRAM_DECLS(node));

    return node;
}

node_st *DVDfundecl(node_st *node) {
    DATA_DVD_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DVD_GET()->vartable = DATA_DVD_GET()->vartable->parent;

    return node;
}

node_st *DVDfunbody(node_st *node) {
    node_st *prev = DATA_DVD_GET()->stmts;
    DATA_DVD_GET()->stmts = FUNBODY_STMTS(node);

    TRAVchildren(node);

    FUNBODY_STMTS(node) = DATA_DVD_GET()->stmts;
    DATA_DVD_GET()->stmts = prev;

    return node;
}

node_st *DVDdecls(node_st *node) {
    TRAVnext(node);
    TRAVdecl(node);

    if (!DECLS_DECL(node)) {
        TAKE(DECLS_NEXT(node));
    }

    return node;
}

node_st *DVDvardecl(node_st *node) {
    TRAVchildren(node);

    node_st *root = NULL;
    node_st *stmts = NULL;

    vartable_ref r = {0, VARDECL_L(node)};

    int i = 0;
    for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
         expr = EXPRS_NEXT(expr), i++) {
        vartable_entry *e = vartable_get(DATA_DVD_GET()->vartable, r);

        node_st *ref;
        vartable_ref tr;
        if (VARDECL_EXTERNAL(node)) {
            ref = EXPRS_EXPR(expr);

            EXPRS_EXPR(expr) = vartable_named_temp_var(
                DATA_DVD_GET()->vartable, TY_int, STRfmt("%s+%d", e->name, i));
            tr = (vartable_ref){0, VARREF_L(EXPRS_EXPR(expr))};
            vartable_get(DATA_DVD_GET()->vartable, tr)->external = true;

            VARREF_N(EXPRS_EXPR(expr)) =
                DATA_DVD_GET()->vartable->parent == NULL;
        } else if (VARDECL_EXPORTED(node)) {
            ref = vartable_named_temp_var(DATA_DVD_GET()->vartable, TY_int,
                                          STRfmt("%s+%d", e->name, i));
            tr = (vartable_ref){0, VARREF_L(ref)};
            vartable_get(DATA_DVD_GET()->vartable, tr)->exported = true;
        } else {
            ref = vartable_temp_var(DATA_DVD_GET()->vartable, TY_int);
            tr = (vartable_ref){0, VARREF_L(ref)};
        }

        vartable_get(DATA_DVD_GET()->vartable, r)->ty.buf[i] = tr;

        VARREF_N(ref) = DATA_DVD_GET()->vartable->parent == NULL;
        node_st *decl = ASTassign(CCNcopy(ref), EXPRS_EXPR(expr));
        VARREF_WRITE(ASSIGN_REF(decl)) = true;
        EXPRS_EXPR(expr) = ref;

        if (root) {
            STMTS_NEXT(stmts) = ASTstmts(decl, NULL);
            stmts = STMTS_NEXT(stmts);
        } else {
            stmts = ASTstmts(decl, NULL);
            root = stmts;
        }
    }

    node_st *ref = vartable_get_ref(DATA_DVD_GET()->vartable, r);
    VARREF_N(ref) = DATA_DVD_GET()->vartable->parent == NULL;
    VARREF_WRITE(ref) = true;

    if (VARDECL_EXPR(node)) {
        node_st *assign = ASTassign(CCNcopy(ref), VARDECL_EXPR(node));
        VARDECL_EXPR(node) = NULL;

        DATA_DVD_GET()->stmts = ASTstmts(assign, DATA_DVD_GET()->stmts);
    }

    if (TYPE_EXPRS(VARDECL_TY(node)) && !VARDECL_EXTERNAL(node)) {
        node_st *malloc = ASTmalloc(TYPE_EXPRS(VARDECL_TY(node)));
        MALLOC_RESOLVED_TY(malloc) = TYPE_TY(VARDECL_TY(node));
        TYPE_EXPRS(VARDECL_TY(node)) = NULL;

        node_st *assign = ASTassign(ref, malloc);

        DATA_DVD_GET()->stmts = ASTstmts(assign, DATA_DVD_GET()->stmts);
    } else {
        CCNfree(ref);
    }

    if (stmts) {
        STMTS_NEXT(stmts) = DATA_DVD_GET()->stmts;
        DATA_DVD_GET()->stmts = root;
    }

    return CCNfree(node);
}

node_st *DVDvarref(node_st *node) {
    if (!DATA_DVD_GET()->vartable->parent) {
        VARREF_N(node)++;
    }
    return node;
}

node_st *DVDcall(node_st *node) {
    if (!DATA_DVD_GET()->vartable->parent) {
        CALL_N(node)++;
    }
    return node;
}
