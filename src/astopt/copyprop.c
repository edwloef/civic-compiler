#include "ccn/ccn.h"

void AOCPinit(void) {}
void AOCPfini(void) {}

node_st *AOCPprogram(node_st *node) {
    DATA_AOCP_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AOCP_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOCPfundecl(node_st *node) {
    DATA_AOCP_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AOCP_GET()->vartable = DATA_AOCP_GET()->vartable->parent;

    return node;
}

node_st *AOCPfunbody(node_st *node) {
    DATA_AOCP_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOCP_GET()->funtable = DATA_AOCP_GET()->funtable->parent;

    return node;
}

node_st *AOCPstmts(node_st *node) {
    DATA_AOCP_GET()->next = STMTS_NEXT(node);

    TRAVchildren(node);

    return node;
}

node_st *AOCPassign(node_st *node) {
    ASSIGN_EXPR(node) = TRAVstart(ASSIGN_EXPR(node), TRAV_AOCF);

    if (!VARREF_EXPRS(ASSIGN_REF(node)) &&
        (NODE_TYPE(ASSIGN_EXPR(node)) == NT_INT ||
         NODE_TYPE(ASSIGN_EXPR(node)) == NT_FLOAT ||
         NODE_TYPE(ASSIGN_EXPR(node)) == NT_BOOL ||
         (NODE_TYPE(ASSIGN_EXPR(node)) == NT_VARREF &&
          !VARREF_EXPRS(ASSIGN_EXPR(node))))) {
        funtable *funtable = DATA_AOCP_GET()->funtable;
        vartable *vartable = DATA_AOCP_GET()->vartable;

        node_st *next = DATA_AOCP_GET()->next;
        node_st *parent = DATA_AOCP_GET()->parent;

        TRAVpush(TRAV_VP);

        DATA_VP_GET()->ref = ASSIGN_REF(node);
        DATA_VP_GET()->expr = ASSIGN_EXPR(node);
        DATA_VP_GET()->funtable = funtable;
        DATA_VP_GET()->vartable = vartable;

        TRAVopt(next);
        TRAVopt(parent);

        TRAVpop();
    }

    return node;
}

node_st *AOCPifelse(node_st *node) {
    node_st *parent = DATA_AOCP_GET()->parent;
    DATA_AOCP_GET()->parent = NULL;

    TRAVchildren(node);

    DATA_AOCP_GET()->parent = parent;

    return node;
}

node_st *AOCPdowhile(node_st *node) {
    node_st *parent = ASTscope(DOWHILE_EXPR(node), DATA_AOCP_GET()->next,
                               DATA_AOCP_GET()->parent);
    DATA_AOCP_GET()->parent = parent;

    TRAVchildren(node);

    DATA_AOCP_GET()->parent = SCOPE_PARENT(parent);
    SCOPE_EXPR(parent) = NULL;
    SCOPE_STMTS(parent) = NULL;
    SCOPE_PARENT(parent) = NULL;
    CCNfree(parent);

    return node;
}
