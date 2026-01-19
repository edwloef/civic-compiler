#pragma once

#include "table/vartable.h"

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

void funtype_free(funtype self);

typedef struct {
    char *name;
    funtype ty;
    span span;
    int call_count, min_level;
    bool external, exported, transp;
} funtable_entry;

typedef struct funtable funtable;
struct funtable {
    int len, cap;
    funtable_entry *buf;
    funtable *parent;
};

funtable *funtable_new(funtable *parent);

funtable_ref funtable_push(funtable *self, funtable_entry e);

funtable_entry *funtable_get(funtable *self, funtable_ref r);

void funtable_free(funtable *self);
