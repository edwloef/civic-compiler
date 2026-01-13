#pragma once

#include "ccngen/ast.h"
#include "table/funtable.h"
#include "table/vartable.h"

void vartable_insert(vartable *self, vartable_entry e, node_st *id);

vartable_ref vartable_resolve(vartable *self, node_st *id);

node_st *vartable_temp_var(vartable *self, vartype ty);

void funtable_insert(funtable *self, funtable_entry e, node_st *id);

funtable_ref funtable_resolve(funtable *self, node_st *call);
