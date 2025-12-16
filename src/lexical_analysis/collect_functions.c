#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "ccngen/ast.h"
#include "ccngen/trav_data.h"
#include "palm/memory.h"
#include "user_types.h"

void LACFinit(void) {}

void LACFfini(void) {}

node_st *LACFprogram(node_st *node) {
    TRAVchildren(node);

    PROGRAM_FUNTABLE(node) = DATA_LACF_GET()->funtable;

    return node;
}

int arity(node_st *next) {
    int arity = 0;
    while (next) {
        arity++;
        next = DECLS_NEXT(next);
    }
    return arity;
}

node_st *LACFfundecl(node_st *node) {
    funtable_ptr prev = DATA_LACF_GET()->funtable;

    funtable_ptr next = MEMmalloc(sizeof(funtable));
    next->name = ID_VAL(FUNDECL_ID(node));
    next->arity = arity(FUNDECL_DECLS(node));
    next->prev = prev;
    prev->next = next;
    DATA_LACF_GET()->funtable = next;

    TRAVchildren(node);

    return node;
}

node_st *LACFfunbody(node_st *node) {
    funtable_ptr prev = DATA_LACF_GET()->funtable;

    TRAVchildren(node);

    FUNBODY_FUNTABLE(node) = DATA_LACF_GET()->funtable;
    DATA_LACF_GET()->funtable = prev;

    return node;
}
