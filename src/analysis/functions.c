#include "ccn/ccn.h"
#include "table/insert.h"

void AFinit(void) {}
void AFfini(void) {}

node_st *AFprogram(node_st *node) {
    DATA_AF_GET()->funtable = funtable_new(NULL);

    TRAVchildren(node);

    PROGRAM_FUNTABLE(node) = DATA_AF_GET()->funtable;

    return node;
}

node_st *AFfundecl(node_st *node) {
    funtype ty = funtype_new(FUNDECL_TY(node));
    node_st *arg = FUNDECL_PARAMS(node);
    while (arg) {
        int dims = 0;
        node_st *id = TYPE_EXPRS(PARAM_TY(PARAMS_PARAM(arg)));
        while (id) {
            dims++;
            id = EXPRS_NEXT(id);
        }

        vartype e = {TYPE_TY(PARAM_TY(PARAMS_PARAM(arg))), dims};
        funtype_push(&ty, e);
        arg = PARAMS_NEXT(arg);
    }

    funtable_entry e = {ID_VAL(FUNDECL_ID(node)), ty, false, 0,
                        SPAN(FUNDECL_ID(node))};
    funtable_insert(DATA_AF_GET()->funtable, e, FUNDECL_ID(node));
    FUNDECL_L(node) = DATA_AF_GET()->funtable->len - 1;

    TRAVchildren(node);

    return node;
}

node_st *AFfunbody(node_st *node) {
    DATA_AF_GET()->funtable = funtable_new(DATA_AF_GET()->funtable);

    TRAVchildren(node);
    FUNBODY_FUNTABLE(node) = DATA_AF_GET()->funtable;

    DATA_AF_GET()->funtable = DATA_AF_GET()->funtable->parent;

    return node;
}
