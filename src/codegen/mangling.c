#include "ccn/ccn.h"
#include "ccngen/trav_data.h"
#include "palm/str.h"
#include "table/funtable.h"

void CGMinit(void) {}
void CGMfini(void) {}

node_st *CGMprogram(node_st *node) {
    DATA_CGM_GET()->funtable = PROGRAM_FUNTABLE(node);
    TRAVchildren(node);
    return node;
}

node_st *CGMfundecl(node_st *node) {
    if (FUNDECL_EXTERNAL(node)) {
        return node;
    }

    funtable *parent_table = DATA_CGM_GET()->funtable;
    char *parent_name = DATA_CGM_GET()->name;

    funtable_entry *e = &parent_table->buf[FUNDECL_L(node)];
    if (parent_name == NULL) {
        e->mangled_name = STRfmt("%s+%d", e->name, e->ty.len);
    } else {
        e->mangled_name = STRfmt("%s+%s+%d", parent_name, e->name, e->ty.len);
    }
    ID_VAL(FUNDECL_ID(node)) = e->mangled_name;

    DATA_CGM_GET()->name = e->mangled_name;
    DATA_CGM_GET()->funtable = FUNBODY_FUNTABLE(FUNDECL_BODY(node));
    TRAVchildren(node);
    DATA_CGM_GET()->funtable = parent_table;
    DATA_CGM_GET()->name = parent_name;

    return node;
}
