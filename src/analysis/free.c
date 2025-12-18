#include "ccn/ccn.h"

node_st *AFprogram(node_st *node) {
    TRAVchildren(node);

    funtable_free(PROGRAM_FUNTABLE(node));
    PROGRAM_FUNTABLE(node) = NULL;

    vartable_free(PROGRAM_VARTABLE(node));
    PROGRAM_VARTABLE(node) = NULL;

    return node;
}

node_st *AFfundecl(node_st *node) {
    TRAVchildren(node);

    vartable_free(FUNDECL_VARTABLE(node));
    FUNDECL_VARTABLE(node) = NULL;

    return node;
}

node_st *AFfunbody(node_st *node) {
    TRAVchildren(node);

    funtable_free(FUNBODY_FUNTABLE(node));
    FUNBODY_FUNTABLE(node) = NULL;

    return node;
}
