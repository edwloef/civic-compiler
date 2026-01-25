#include "ccn/ccn.h"
#include "table/funtable.h"

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
    for (node_st *param = FUNDECL_PARAMS(node); param;
         param = PARAMS_NEXT(param)) {
        int dims = 0;
        for (node_st *expr = TYPE_EXPRS(PARAM_TY(PARAMS_PARAM(param))); expr;
             expr = EXPRS_NEXT(expr)) {
            dims++;
        }

        thin_vartype e = {TYPE_TY(PARAM_TY(PARAMS_PARAM(param))), dims};
        funtype_push(&ty, e);
    }

    funtable_entry e = {ID_VAL(FUNDECL_ID(node)),
                        ty,
                        SPAN(FUNDECL_ID(node)),
                        0,
                        0,
                        0,
                        0,
                        0,
                        FUNDECL_EXTERNAL(node),
                        FUNDECL_EXPORTED(node),
                        false};
    FUNDECL_L(node) =
        funtable_insert(DATA_AF_GET()->funtable, e, FUNDECL_ID(node)).l;

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
