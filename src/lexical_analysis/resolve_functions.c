#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "ccngen/ast.h"
#include "ccngen/trav_data.h"
#include "palm/ctinfo.h"
#include "palm/str.h"
#include "user_types.h"
#include <stdbool.h>

void LARFinit(void) {}

void LARFfini(void) {}

node_st *LARFprogram(node_st *node) {
    DATA_LARF_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

int count_exprs(node_st *next) {
    int arity = 0;
    while (next) {
        arity++;
        next = EXPRS_NEXT(next);
    }
    return arity;
}

bool resolve(char *name, int arity) {
    funtable_ptr prev = DATA_LARF_GET()->funtable;
    while (prev && prev->arity != arity && !STReq(name, prev->name))
        prev = prev->prev;
    return prev != NULL;
}

node_st *LARFcall(node_st *node) {
    char *name = ID_VAL(CALL_ID(node));
    int arity = count_exprs(CALL_EXPRS(node));

    bool resolved = resolve(name, arity);

    if (!resolved) {
        CTI(CTI_ERROR, true, "couldn't resolve function '%s' with arity '%d'",
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
