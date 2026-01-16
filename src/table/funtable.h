#pragma once

#include "table/vartable.h"

typedef struct {
    int n;
    int l;
} funtable_ref;

typedef struct {
    int len;
    int cap;
    thin_vartype *buf;
    enum BasicType ty;
} funtype;

funtype funtype_new(enum BasicType ty);

void funtype_push(funtype *self, thin_vartype e);

void funtype_free(funtype self);

typedef struct {
    char *name;
    funtype ty;
    bool exported;
    bool transp;
    int min_level;
    int call_count;
    span span;
} funtable_entry;

typedef struct funtable funtable;
struct funtable {
    int len;
    int cap;
    funtable_entry *buf;
    funtable *parent;
};

funtable *funtable_new(funtable *parent);

funtable_ref funtable_push(funtable *self, funtable_entry e);

funtable_entry *funtable_get(funtable *self, funtable_ref r);

void funtable_free(funtable *self);
