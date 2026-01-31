#include "ccn/ccn.h"
#include "palm/memory.h"
#include "table/funtable.h"
#include "table/vartable.h"

void DOTinit(void) {}
void DOTfini(void) {}

node_st *DOTprogram(node_st *node) {
    DATA_DOT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_DOT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

static void use_table_name(char *table_name, node_st *id) {
    if (ID_VAL(id) != table_name) {
        MEMfree(ID_VAL(id));
        ID_VAL(id) = table_name;
    }
}

node_st *DOTfundecl(node_st *node) {
    vartable *parent_vt = DATA_DOT_GET()->vartable;
    DATA_DOT_GET()->vartable = FUNDECL_VARTABLE(node);

    if (FUNDECL_BODY(node)) {
        funtable *parent_ft = DATA_DOT_GET()->funtable;
        DATA_DOT_GET()->funtable = FUNBODY_FUNTABLE(FUNDECL_BODY(node));

        funtable_entry *e = &parent_ft->buf[FUNDECL_L(node)];
        use_table_name(e->name, FUNDECL_ID(node));

        TRAVchildren(node);

        DATA_DOT_GET()->funtable = parent_ft;
    }

    DATA_DOT_GET()->vartable = parent_vt;

    return node;
}

node_st *DOTvarref(node_st *node) {
    vartable_entry *e =
        vartable_get(DATA_DOT_GET()->vartable,
                     (vartable_ref){VARREF_N(node), VARREF_L(node)});
    use_table_name(e->name, VARREF_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DOTvardecl(node_st *node) {
    vartable_entry *e = vartable_get(DATA_DOT_GET()->vartable,
                                     (vartable_ref){0, VARDECL_L(node)});
    use_table_name(e->name, VARDECL_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DOTparam(node_st *node) {
    vartable_entry *e = vartable_get(DATA_DOT_GET()->vartable,
                                     (vartable_ref){0, PARAM_L(node)});
    use_table_name(e->name, PARAM_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DOTcall(node_st *node) {
    funtable_entry *e = funtable_get(
        DATA_DOT_GET()->funtable, (funtable_ref){CALL_N(node), CALL_L(node)});
    use_table_name(e->name, CALL_ID(node));

    TRAVchildren(node);

    return node;
}
