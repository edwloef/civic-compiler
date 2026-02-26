#include "ccn/ccn.h"

void DIinit(void) {}
void DIfini(void) {}

node_st *DIprogram(node_st *node) {
    DATA_DI_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_DI_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

static void keep_table_id(char *table_name, node_st *id) {
    if (ID_VAL(id) == table_name) {
        ID_VAL(id) = NULL;
    }
}

node_st *DIfundecl(node_st *node) {
    funtable_entry *e = &DATA_DI_GET()->funtable->buf[FUNDECL_L(node)];
    keep_table_id(e->name, FUNDECL_ID(node));

    DATA_DI_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DI_GET()->vartable = DATA_DI_GET()->vartable->parent;

    return node;
}

node_st *DIfunbody(node_st *node) {
    DATA_DI_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_DI_GET()->funtable = DATA_DI_GET()->funtable->parent;

    return node;
}

node_st *DIvardecl(node_st *node) {
    vartable_entry *e = &DATA_DI_GET()->vartable->buf[VARDECL_L(node)];
    keep_table_id(e->name, VARDECL_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DIparam(node_st *node) {
    vartable_entry *e =
        vartable_get(DATA_DI_GET()->vartable, (vartable_ref){0, PARAM_L(node)});
    keep_table_id(e->name, PARAM_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DIcall(node_st *node) {
    funtable_entry *e = funtable_get(
        DATA_DI_GET()->funtable, (funtable_ref){CALL_N(node), CALL_L(node)});
    keep_table_id(e->name, CALL_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DIvarref(node_st *node) {
    vartable_entry *e =
        vartable_get(DATA_DI_GET()->vartable,
                     (vartable_ref){VARREF_N(node), VARREF_L(node)});
    keep_table_id(e->name, VARREF_ID(node));

    TRAVchildren(node);

    return node;
}

node_st *DIid(node_st *node) {
    return CCNfree(node);
}
