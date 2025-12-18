#include "ccn/ccn.h"
#include "palm/memory.h"

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

node_st *AFarrexprs(node_st *node) {
    TRAVchildren(node);

    MEMfree(ARREXPRS_RESOLVED_TY(node));
    ARREXPRS_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFmonop(node_st *node) {
    TRAVchildren(node);

    MEMfree(MONOP_RESOLVED_TY(node));
    MONOP_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFbinop(node_st *node) {
    TRAVchildren(node);

    MEMfree(BINOP_RESOLVED_TY(node));
    BINOP_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFcast(node_st *node) {
    TRAVchildren(node);

    MEMfree(CAST_RESOLVED_TY(node));
    CAST_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFcall(node_st *node) {
    TRAVchildren(node);

    MEMfree(CALL_RESOLVED_TY(node));
    CALL_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFvarref(node_st *node) {
    TRAVchildren(node);

    MEMfree(VARREF_RESOLVED_TY(node));
    VARREF_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFint(node_st *node) {
    MEMfree(INT_RESOLVED_TY(node));
    INT_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFfloat(node_st *node) {
    MEMfree(FLOAT_RESOLVED_TY(node));
    FLOAT_RESOLVED_TY(node) = NULL;

    return node;
}

node_st *AFbool(node_st *node) {
    MEMfree(BOOL_RESOLVED_TY(node));
    BOOL_RESOLVED_TY(node) = NULL;

    return node;
}
