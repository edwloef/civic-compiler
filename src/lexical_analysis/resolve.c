#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "ccngen/ast.h"
#include "ccngen/enum.h"
#include "ccngen/trav.h"
#include "ccngen/trav_data.h"
#include "palm/ctinfo.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "user_types.h"

void LARinit(void) {}

void LARfini(void) {}

node_st *LARprogram(node_st *node) {
    DATA_LAR_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

void LARtrydeclaresym(char *name, enum BasicType ty, int dims) {
    symtable_ptr entry = DATA_LAR_GET()->symtable;
    while (entry &&
           !(entry->level == DATA_LAR_GET()->level && STReq(name, entry->name)))
        entry = entry->prev;

    if (entry != NULL) {
        CTI(CTI_ERROR, true, "couldn't re-declare variable '%s'", name);
        CTIabortOnError();
    }

    symtable_ptr next = MEMmalloc(sizeof(symtable));
    next->name = name;
    next->ty = ty;
    next->dims = dims;
    next->level = DATA_LAR_GET()->level;
    next->prev = DATA_LAR_GET()->symtable;
    DATA_LAR_GET()->symtable = next;
}

node_st *LARvardecl(node_st *node) {
    TRAVopt(VARDECL_EXPR(node));

    int dims = 0;
    node_st *expr = TYPE_EXPRS(VARDECL_TY(node));
    while (expr) {
        dims++;
        expr = EXPRS_NEXT(expr);
    }

    LARtrydeclaresym(ID_VAL(VARDECL_ID(node)), TYPE_TY(VARDECL_TY(node)), dims);

    return node;
}

node_st *LARparam(node_st *node) {
    int dims = 0;
    node_st *id = PARAM_IDS(node);
    while (id) {
        dims++;
        LARtrydeclaresym(ID_VAL(IDS_ID(id)), TY_int, 0);
        id = IDS_NEXT(id);
    }

    LARtrydeclaresym(ID_VAL(PARAM_ID(node)), PARAM_TY(node), dims);

    return node;
}

node_st *LARfundecl(node_st *node) {
    symtable_ptr prev = DATA_LAR_GET()->symtable;
    DATA_LAR_GET()->level++;

    TRAVchildren(node);
    FUNDECL_SYMTABLE(node) = DATA_LAR_GET()->symtable;

    DATA_LAR_GET()->symtable = prev;
    DATA_LAR_GET()->level--;

    return node;
}

node_st *LARfunbody(node_st *node) {
    funtable_ptr prev = DATA_LAR_GET()->funtable;
    DATA_LAR_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_LAR_GET()->funtable = prev;

    return node;
}

node_st *LARfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    symtable_ptr next = MEMmalloc(sizeof(symtable));
    next->name = ID_VAL(FOR_ID(node));
    next->level = DATA_LAR_GET()->level;
    next->prev = DATA_LAR_GET()->symtable;
    DATA_LAR_GET()->symtable = next;

    symtable_ptr prev = next->prev;

    TRAVopt(FOR_STMTS(node));

    DATA_LAR_GET()->symtable = prev;

    return node;
}

node_st *LARcall(node_st *node) {
    node_st *arg = CALL_EXPRS(node);
    int arity = 0;
    while (arg) {
        arity++;
        arg = EXPRS_NEXT(arg);
    }

    char *name = ID_VAL(CALL_ID(node));
    funtable_ptr entry = DATA_LAR_GET()->funtable;
    while (entry && !(entry->arity == arity && STReq(name, entry->name)))
        entry = entry->prev;

    if (entry == NULL) {
        CTI(CTI_ERROR, true, "couldn't resolve function '%s' with arity %d",
            name, arity);
        CTIabortOnError();
    }

    return node;
}

node_st *LARvarref(node_st *node) {
    char *name = ID_VAL(VARREF_ID(node));
    symtable_ptr entry = DATA_LAR_GET()->symtable;
    while (entry && !STReq(name, entry->name))
        entry = entry->prev;

    if (entry == NULL) {
        CTI(CTI_ERROR, true, "couldn't resolve variable '%s'", name);
        CTIabortOnError();
    }

    TRAVopt(VARREF_EXPRS(node));

    return node;
}
