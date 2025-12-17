#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "ccngen/ast.h"
#include "ccngen/trav_data.h"
#include "palm/ctinfo.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "user_types.h"

void LACFinit(void) {}

void LACFfini(void) {}

node_st *LACFprogram(node_st *node) {
    TRAVchildren(node);

    PROGRAM_FUNTABLE(node) = DATA_LACF_GET()->funtable;

    return node;
}

node_st *LACFfundecl(node_st *node) {
    node_st *arg = FUNDECL_PARAMS(node);
    int arity = 0;
    while (arg) {
        arity++;
        arg = PARAMS_NEXT(arg);
    }

    char *name = ID_VAL(FUNDECL_ID(node));
    funtable_ptr entry = DATA_LACF_GET()->funtable;
    while (entry && !(entry->level == DATA_LACF_GET()->level &&
                      entry->arity == arity && STReq(name, entry->name)))
        entry = entry->prev;

    if (entry != NULL) {
        CTI(CTI_ERROR, true, "couldn't re-declare function '%s' with arity %d",
            name, arity);
        CTIabortOnError();
    }

    funtable_ptr next = MEMmalloc(sizeof(funtable));
    next->name = name;
    next->arity = arity;
    next->level = DATA_LACF_GET()->level;
    next->prev = DATA_LACF_GET()->funtable;
    DATA_LACF_GET()->funtable = next;

    TRAVchildren(node);

    return node;
}

node_st *LACFfunbody(node_st *node) {
    funtable_ptr prev = DATA_LACF_GET()->funtable;
    DATA_LACF_GET()->level++;

    TRAVchildren(node);
    FUNBODY_FUNTABLE(node) = DATA_LACF_GET()->funtable;

    DATA_LACF_GET()->funtable = prev;
    DATA_LACF_GET()->level--;

    return node;
}
