#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "ccngen/ast.h"
#include "ccngen/trav_data.h"
#include "palm/ctinfo.h"
#include "palm/str.h"
#include "user_types.h"

void LARFinit(void) {}

void LARFfini(void) {}

node_st *LARFprogram(node_st *node) {
    DATA_LARF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *LARFcall(node_st *node) {
    node_st *arg = CALL_EXPRS(node);
    int arity = 0;
    while (arg) {
        arity++;
        arg = EXPRS_NEXT(arg);
    }

    char *name = ID_VAL(CALL_ID(node));
    funtable_ptr entry = DATA_LARF_GET()->funtable;
    while (entry && entry->arity != arity && !STReq(name, entry->name))
        entry = entry->prev;

    if (entry == NULL) {
        CTI(CTI_ERROR, true, "couldn't resolve function '%s' with arity %d",
            name, arity);
        CTIabortOnError();
    }

    return node;
}

node_st *LARFfunbody(node_st *node) {
    funtable_ptr prev = DATA_LARF_GET()->funtable;
    DATA_LARF_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_LARF_GET()->funtable = prev;

    return node;
}
