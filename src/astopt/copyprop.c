#include "ccn/ccn.h"
#include "ccngen/trav.h"

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

    node_st *stmt = STMTS_STMT(node);
    switch (NODE_TYPE(stmt)) {
    case NT_ASSIGN:
        ASSIGN_EXPR(stmt) = TRAVstart(ASSIGN_EXPR(stmt), TRAV_AOCF);
        if (!VARREF_EXPRS(ASSIGN_REF(stmt)) &&
            (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_INT ||
             NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_FLOAT ||
             NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_BOOL ||
             (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF &&
              !VARREF_EXPRS(ASSIGN_EXPR(stmt))))) {
            funtable *funtable = DATA_AOCP_GET()->funtable;
            vartable *vartable = DATA_AOCP_GET()->vartable;
            node_st *parent = DATA_AOCP_GET()->parent;

            TRAVpush(TRAV_VP);

            DATA_VP_GET()->ref = ASSIGN_REF(stmt);
            DATA_VP_GET()->expr = ASSIGN_EXPR(stmt);
            DATA_VP_GET()->funtable = funtable;
            DATA_VP_GET()->vartable = vartable;

            TRAVnext(node);
            TRAVopt(parent);

            TRAVpop();
        }
        break;
    case NT_DOWHILE: {
        node_st *parent = ASTscope(DOWHILE_EXPR(stmt), STMTS_NEXT(node),
                                   DATA_AOCP_GET()->parent);
        DATA_AOCP_GET()->parent = parent;

        TRAVstmt(node);

        DATA_AOCP_GET()->parent = SCOPE_PARENT(parent);
        SCOPE_EXPR(parent) = NULL;
        SCOPE_STMTS(parent) = NULL;
        SCOPE_PARENT(parent) = NULL;
        CCNfree(parent);
        break;
    }
    case NT_IFELSE: {
        node_st *parent = DATA_AOCP_GET()->parent;
        DATA_AOCP_GET()->parent = NULL;

        TRAVstmt(node);

        DATA_AOCP_GET()->parent = parent;
        break;
    }
    default:
        TRAVstmt(node);
        break;
    }

    TRAVnext(node);

    return node;
}
