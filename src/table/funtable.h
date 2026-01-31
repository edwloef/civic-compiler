#pragma once

#include <stdbool.h>

#include "ccngen/enum.h"
#include "error/error.h"

typedef struct ccn_node node_st;

typedef struct {
    enum BasicType ty;
    int dims;
} thin_vartype;

typedef struct {
    int n, l;
} funtable_ref;

typedef struct {
    int len, cap;
    thin_vartype *buf;
    enum BasicType ty;
} funtype;

funtype funtype_new(enum BasicType ty);

void funtype_push(funtype *self, thin_vartype e);

funtype funtype_copy(funtype *self);

void funtype_free(funtype self);

typedef struct {
    char *name;
    char *mangled_name;
    funtype ty;
    span span;
    int new_l, call_count, read_capture, write_capture, scalar_read_capture,
        scalar_write_capture;
    bool external, exported, side_effects;
} funtable_entry;

typedef struct funtable funtable;
struct funtable {
    int len, cap;
    funtable_entry *buf;
    funtable *parent;
};

funtable *funtable_new(funtable *parent);

funtable_ref funtable_insert(funtable *self, funtable_entry e, node_st *id);

funtable_ref funtable_push(funtable *self, funtable_entry e);

funtable_ref funtable_resolve(funtable *self, node_st *call);

funtable_entry *funtable_get(funtable *self, funtable_ref r);

void funtable_entry_free(funtable_entry e);

void funtable_shallow_free(funtable *self);

void funtable_free(funtable *self);
