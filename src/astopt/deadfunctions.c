#include "ccn/ccn.h"
#include "palm/str.h"
#include "utils.h"

void AODFinit(void) {}
void AODFfini(void) {}

node_st *AODFprogram(node_st *node) {
    DATA_AODF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AODFdecls(node_st *node) {
    TRAVchildren(node);

    if (!DECLS_DECL(node)) {
        TAKE(DECLS_NEXT(node));
    }

    return node;
}

node_st *AODFfundecl(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_AODF_GET()->funtable, r);

    if (e->call_count == 0) {
        TRAVstart(node, TRAV_F);
        node = CCNfree(node);
    } else if (STReq(e->name, "__init") &&
               NODE_TYPE(STMTS_STMT(FUNBODY_STMTS(FUNDECL_BODY(node)))) ==
                   NT_RETURN) {
        e->exported = false;
        TRAVstart(node, TRAV_F);
        node = CCNfree(node);
    }

    return node;
}

node_st *AODFfunbody(node_st *node) {
    DATA_AODF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AODF_GET()->funtable = DATA_AODF_GET()->funtable->parent;

    return node;
}
