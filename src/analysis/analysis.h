#pragma once

#include "ccngen/ast.h"
#include "funtable.h"
#include "vartable.h"

void vartable_insert(vartable *self, vartable_entry e, node_st *id);

vartable_ref vartable_resolve(vartable *self, node_st *id);

void funtable_insert(funtable *self, funtable_entry e, node_st *id);

funtable_ref funtable_resolve(funtable *self, node_st *call);
