#include "ccn/ccn.h"

void ACinit(void) { DATA_AC_GET()->funtable = funtable_new(NULL); }

void ACfini(void) {}

node_st *ACprogram(node_st *node) {
    TRAVchildren(node);

    PROGRAM_FUNTABLE(node) = DATA_AC_GET()->funtable;

    return node;
}

node_st *ACfundecl(node_st *node) {
    funtype ty = funtype_new(FUNDECL_TY(node));
    node_st *arg = FUNDECL_PARAMS(node);
    while (arg) {
        int dims = 0;
        node_st *id = PARAM_IDS(PARAMS_PARAM(arg));
        while (id) {
            dims++;
            id = IDS_NEXT(id);
        }

        vartype e = {PARAM_TY(PARAMS_PARAM(arg)), dims};
        funtype_push(&ty, e);
        arg = PARAMS_NEXT(arg);
    }

    funtable_entry e = {ID_VAL(FUNDECL_ID(node)), ty};
    funtable_insert(DATA_AC_GET()->funtable, e);

    TRAVchildren(node);

    return node;
}

node_st *ACfunbody(node_st *node) {
    DATA_AC_GET()->funtable = funtable_new(DATA_AC_GET()->funtable);

    TRAVchildren(node);
    FUNBODY_FUNTABLE(node) = DATA_AC_GET()->funtable;

    DATA_AC_GET()->funtable = DATA_AC_GET()->funtable->parent;

    return node;
}
