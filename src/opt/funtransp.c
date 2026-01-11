#include "ccn/ccn.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void OCTCTinit(void) {}

void OCTCTfini(void) {}

node_st *OCTCTprogram(node_st *node) {
    DATA_OCTCT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_OCTCT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *OCTCTfundecl(node_st *node) {
    int prev = DATA_OCTCT_GET()->min_nesting_level;
    DATA_OCTCT_GET()->min_nesting_level = DATA_OCTCT_GET()->nesting_level + 1;
    DATA_OCTCT_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);
    int min_nesting_level = DATA_OCTCT_GET()->min_nesting_level;

    DATA_OCTCT_GET()->min_nesting_level = MIN(prev, min_nesting_level);
    DATA_OCTCT_GET()->vartable = DATA_OCTCT_GET()->vartable->parent;

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_OCTCT_GET()->funtable, r);
    if (min_nesting_level != e->min_nesting_level) {
        e->min_nesting_level = min_nesting_level;
        CCNcycleNotify();
    }

    return node;
}

node_st *OCTCTfunbody(node_st *node) {
    DATA_OCTCT_GET()->nesting_level++;
    DATA_OCTCT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_OCTCT_GET()->nesting_level--;
    DATA_OCTCT_GET()->funtable = DATA_OCTCT_GET()->funtable->parent;

    return node;
}

node_st *OCTCTassign(node_st *node) {
    DATA_OCTCT_GET()->min_nesting_level =
        MIN(DATA_OCTCT_GET()->min_nesting_level,
            DATA_OCTCT_GET()->nesting_level - VARREF_N(ASSIGN_REF(node)));

    return node;
}

node_st *OCTCTcall(node_st *node) {
    funtable_ref r = {CALL_N(node), CALL_L(node)};

    if (!funtable_transp(DATA_OCTCT_GET()->funtable, r)) {
        DATA_OCTCT_GET()->min_nesting_level =
            MIN(DATA_OCTCT_GET()->min_nesting_level,
                DATA_OCTCT_GET()->nesting_level - CALL_N(node));
    }

    return node;
}
