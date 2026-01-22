#pragma once

#include "ccngen/ast.h"
#include "table/funtable.h"
#include "table/vartable.h"

node_st *vartable_entry_ref(vartable_entry *e, vartable_ref r);

vartable_ref vartable_insert(vartable *self, vartable_entry e, node_st *id);

vartable_ref vartable_resolve(vartable *self, node_st *id);

node_st *vartable_temp_var(vartable *self, enum BasicType ty);

funtable_ref funtable_insert(funtable *self, funtable_entry e, node_st *id);

funtable_ref funtable_resolve(funtable *self, node_st *call);
