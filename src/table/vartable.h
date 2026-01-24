#pragma once

#include <stdbool.h>

#include "ccngen/enum.h"
#include "error/error.h"

typedef struct ccn_node node_st;

typedef struct {
    int n, l;
} vartable_ref;

typedef struct {
    int len, cap;
    vartable_ref *buf;
    enum BasicType ty;
} vartype;

vartype vartype_new(enum BasicType ty);

void vartype_push(vartype *self, vartable_ref e);

void vartype_free(vartype self);

typedef struct {
    char *name;
    vartype ty;
    span span;
    int read_count, write_count;
    bool external, exported, param, loopvar, escapes;
} vartable_entry;

typedef struct vartable vartable;
struct vartable {
    int len, cap;
    vartable_entry *buf;
    vartable *parent;
};

vartable *vartable_new(vartable *parent);

vartable_ref vartable_insert(vartable *self, vartable_entry e, node_st *id);

vartable_ref vartable_push_loopvar(vartable *self, vartable_entry e);

vartable_ref vartable_push(vartable *self, vartable_entry e);

vartable_ref vartable_resolve(vartable *self, node_st *id);

vartable_entry *vartable_get(vartable *self, vartable_ref r);

node_st *vartable_get_ref(vartable *self, vartable_ref r);

node_st *vartable_temp_var(vartable *self, enum BasicType ty);

void vartable_free(vartable *self);
