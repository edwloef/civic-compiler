#include "ccn/ccn.h"

node_st *Fprogram(node_st *node) {
    TRAVchildren(node);

    consttable_free(PROGRAM_CONSTTABLE(node));
    PROGRAM_CONSTTABLE(node) = NULL;

    funtable_free(PROGRAM_FUNTABLE(node));
    PROGRAM_FUNTABLE(node) = NULL;

    vartable_free(PROGRAM_VARTABLE(node));
    PROGRAM_VARTABLE(node) = NULL;

    return CCNfree(node);
}

node_st *Ffundecl(node_st *node) {
    TRAVchildren(node);

    vartable_free(FUNDECL_VARTABLE(node));
    FUNDECL_VARTABLE(node) = NULL;

    return node;
}

node_st *Ffunbody(node_st *node) {
    TRAVchildren(node);

    funtable_free(FUNBODY_FUNTABLE(node));
    FUNBODY_FUNTABLE(node) = NULL;

    return node;
}
