#include "ccn/ccn.h"
#include "ccngen/trav.h"

void AOCPinit(void) {}
void AOCPfini(void) {}

node_st *AOCPprogram(node_st *node) {
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

node_st *AOCPstmts(node_st *node) {
    TRAVnext(node);

    node_st *stmt = STMTS_STMT(node);
    switch (NODE_TYPE(stmt)) {
    case NT_ASSIGN:
        if (!VARREF_EXPRS(ASSIGN_REF(stmt)) &&
            (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_INT ||
             NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_FLOAT ||
             NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_BOOL ||
             (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF &&
              !VARREF_EXPRS(ASSIGN_EXPR(stmt))))) {
            node_st *parent = DATA_AOCP_GET()->parent;
            vartable_ref r = {VARREF_N(ASSIGN_REF(stmt)),
                              VARREF_L(ASSIGN_REF(stmt))};
            vartable_entry *e = vartable_get(DATA_AOCP_GET()->vartable, r);

            TRAVpush(TRAV_VP);

            DATA_VP_GET()->expr = ASSIGN_EXPR(stmt);
            DATA_VP_GET()->write_count = e->write_count;
            DATA_VP_GET()->n = r.n;
            DATA_VP_GET()->l = r.l;

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
    case NT_FOR:
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

    return node;
}
