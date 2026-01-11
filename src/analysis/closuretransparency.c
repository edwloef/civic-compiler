#include "ccn/ccn.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void ATCTinit(void) {}

void ATCTfini(void) {}

node_st *ATCTprogram(node_st *node) {
    DATA_ATCT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_ATCT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *ATCTfundecl(node_st *node) {
    int prev = DATA_ATCT_GET()->min_nesting_level;
    DATA_ATCT_GET()->min_nesting_level = DATA_ATCT_GET()->nesting_level + 1;
    DATA_ATCT_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);
    int min_nesting_level = DATA_ATCT_GET()->min_nesting_level;

    DATA_ATCT_GET()->min_nesting_level = MIN(prev, min_nesting_level);
    DATA_ATCT_GET()->vartable = DATA_ATCT_GET()->vartable->parent;

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_ATCT_GET()->funtable, r);
    if (min_nesting_level != e->min_nesting_level) {
        e->min_nesting_level = min_nesting_level;
        CCNcycleNotify();
    }

    return node;
}

node_st *ATCTfunbody(node_st *node) {
    DATA_ATCT_GET()->nesting_level++;
    DATA_ATCT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_ATCT_GET()->nesting_level--;
    DATA_ATCT_GET()->funtable = DATA_ATCT_GET()->funtable->parent;

    return node;
}

node_st *ATCTassign(node_st *node) {
    DATA_ATCT_GET()->min_nesting_level =
        MIN(DATA_ATCT_GET()->min_nesting_level,
            DATA_ATCT_GET()->nesting_level - VARREF_N(ASSIGN_REF(node)));

    return node;
}

node_st *ATCTcall(node_st *node) {
    DATA_ATCT_GET()->min_nesting_level =
        MIN(DATA_ATCT_GET()->min_nesting_level,
            DATA_ATCT_GET()->nesting_level - CALL_N(node));

    return node;
}
