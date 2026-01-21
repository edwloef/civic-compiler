#include "ccn/ccn.h"
#include "macros.h"

void AODFinit(void) {}
void AODFfini(void) {}

node_st *AODFprogram(node_st *node) {
    DATA_AODF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AODFdecls(node_st *node) {
    TRAVchildren(node);

    node_st *decl = DECLS_DECL(node);
    funtable_ref r = {0, FUNDECL_L(decl)};
    funtable_entry *e = funtable_get(DATA_AODF_GET()->funtable, r);
    if (e->call_count == 0) {
        TRAVstart(decl, TRAV_F);
        TAKE(DECLS_NEXT(node));
    }

    return node;
}

node_st *AODFfunbody(node_st *node) {
    DATA_AODF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AODF_GET()->funtable = DATA_AODF_GET()->funtable->parent;

    return node;
}
