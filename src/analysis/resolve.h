#include "analysis/funtable.h"
#include "analysis/vartable.h"
#include "ccngen/ast.h"

void vartable_insert(vartable *self, vartable_entry e, node_st *id);

vartable_ref vartable_resolve(vartable *self, node_st *id);

void funtable_insert(funtable *self, funtable_entry e, node_st *id);

funtable_ref funtable_resolve(funtable *self, node_st *call);
