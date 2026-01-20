#include "ccn/ccn.h"
#include "palm/str.h"

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        node = tmp;                                                            \
    }

void DVDinit(void) {}
void DVDfini(void) {}

node_st *DVDprogram(node_st *node) {
    DATA_DVD_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    node_st *decl = ASTfundecl(ASTid(STRcpy("__init")), NULL, TY_void);
    node_st *body = ASTfunbody(NULL, DATA_DVD_GET()->stmts);

    FUNDECL_BODY(decl) = body;

    FUNDECL_VARTABLE(decl) = vartable_new(PROGRAM_VARTABLE(node));
    FUNBODY_FUNTABLE(body) = funtable_new(PROGRAM_FUNTABLE(node));

    funtable_push(PROGRAM_FUNTABLE(node), (funtable_entry){"__init",
                                                           funtype_new(TY_void),
                                                           {0, 0, 0, 0, NULL},
                                                           1,
                                                           0,
                                                           false,
                                                           false,
                                                           false});

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
    TRAVchildren(node);

    node_st *decl = DECLS_DECL(node);
    if (NODE_TYPE(decl) == NT_VARDECL) {
        vartable_ref r = {0, VARDECL_L(decl)};
        vartable_entry *e = vartable_get(DATA_DVD_GET()->vartable, r);
        r.n = DATA_DVD_GET()->vartable->parent == NULL;

        if (VARDECL_EXPR(decl)) {
            node_st *ref = ASTvarref(CCNcopy(VARDECL_ID(decl)), NULL);
            VARREF_N(ref) = r.n;
            VARREF_L(ref) = r.l;
            VARREF_RESOLVED_TY(ref) = e->ty.ty;
            VARREF_RESOLVED_DIMS(ref) = e->ty.len;
            VARREF_WRITE(ref) = true;

            node_st *assign = ASTassign(ref, VARDECL_EXPR(decl));
            VARDECL_EXPR(decl) = NULL;

            DATA_DVD_GET()->stmts = ASTstmts(assign, DATA_DVD_GET()->stmts);
        }

        if (TYPE_EXPRS(VARDECL_TY(decl))) {
            node_st *ref = ASTvarref(CCNcopy(VARDECL_ID(decl)), NULL);
            VARREF_N(ref) = r.n;
            VARREF_L(ref) = r.l;
            VARREF_RESOLVED_TY(ref) = e->ty.ty;
            VARREF_RESOLVED_DIMS(ref) = e->ty.len;
            VARREF_WRITE(ref) = true;

            node_st *malloc = ASTmalloc(TYPE_EXPRS(VARDECL_TY(decl)));
            EXPR_RESOLVED_TY(malloc) = TYPE_TY(VARDECL_TY(decl));
            TYPE_EXPRS(VARDECL_TY(decl)) = NULL;

            node_st *assign = ASTassign(ref, malloc);

            DATA_DVD_GET()->stmts = ASTstmts(assign, DATA_DVD_GET()->stmts);
        }

        TAKE(DECLS_NEXT(node));
    }

    return node;
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
