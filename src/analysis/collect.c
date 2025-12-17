#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "ccngen/ast.h"
#include "ccngen/trav_data.h"
#include "analysis/funtable.h"
#include "palm/memory.h"

void ACinit(void) { DATA_AC_GET()->funtable = funtable_new(NULL); }

void ACfini(void) {}

node_st *ACprogram(node_st *node) {
    TRAVchildren(node);

    PROGRAM_FUNTABLE(node) = DATA_AC_GET()->funtable;

    return node;
}

node_st *ACfundecl(node_st *node) {
    node_st *arg = FUNDECL_PARAMS(node);
    int param_count = 0;
    while (arg) {
        param_count++;
        arg = PARAMS_NEXT(arg);
    }

    vartype *param_tys = MEMmalloc(param_count * sizeof(vartype));
    arg = FUNDECL_PARAMS(node);
    for (int i = 0; i < param_count; i++) {
        int dims = 0;
        node_st *id = PARAM_IDS(PARAMS_PARAM(arg));
        while (id) {
            dims++;
            id = IDS_NEXT(id);
        }

        vartype ty = {PARAM_TY(PARAMS_PARAM(arg)), dims};
        param_tys[i] = ty;
        arg = PARAMS_NEXT(arg);
    }

    funtype ty = {FUNDECL_TY(node), param_count, param_tys};
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
