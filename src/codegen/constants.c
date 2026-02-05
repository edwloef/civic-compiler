#include "ccn/ccn.h"

void CGCinit(void) {}
void CGCfini(void) {}

node_st *CGCprogram(node_st *node) {
    PROGRAM_CONSTTABLE(node) = consttable_new();

    DATA_CGC_GET()->consttable = PROGRAM_CONSTTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *CGCint(node_st *node) {
    if (INT_VAL(node) == 1 || INT_VAL(node) == 0 || INT_VAL(node) == -1) {
        return node;
    }

    consttable_ref r =
        consttable_insert_int(DATA_CGC_GET()->consttable, INT_VAL(node));

    CCNfree(node);
    node = ASTconstref(r.l, TY_int);

    return node;
}

node_st *CGCfloat(node_st *node) {
    if (double_biteq(FLOAT_VAL(node), 0.0) ||
        double_biteq(FLOAT_VAL(node), 1.0)) {
        return node;
    }

    consttable_ref r =
        consttable_insert_float(DATA_CGC_GET()->consttable, FLOAT_VAL(node));

    CCNfree(node);
    node = ASTconstref(r.l, TY_float);

    return node;
}
