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
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_ASSIGN && !VARREF_EXPRS(ASSIGN_REF(stmt)) &&
        (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_INT ||
         NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_FLOAT ||
         NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_BOOL ||
         (NODE_TYPE(ASSIGN_EXPR(stmt)) == NT_VARREF &&
          !VARREF_EXPRS(ASSIGN_EXPR(stmt))))) {
        vartable *vartable = DATA_AOCP_GET()->vartable;

        TRAVpush(TRAV_PV);

        DATA_PV_GET()->expr = ASSIGN_EXPR(stmt);

        vartable_ref r = {VARREF_N(ASSIGN_REF(stmt)),
                          VARREF_L(ASSIGN_REF(stmt))};
        vartable_entry *e = vartable_get(vartable, r);
        DATA_PV_GET()->write_count = e->write_count;

        DATA_PV_GET()->n = VARREF_N(ASSIGN_REF(stmt));
        DATA_PV_GET()->l = VARREF_L(ASSIGN_REF(stmt));

        TRAVnext(node);

        TRAVpop();
    }

    return node;
}
