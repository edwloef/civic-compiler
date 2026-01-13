#pragma once

#include "table/vartable.h"

typedef struct {
    int len;
    int cap;
    vartype *buf;
    enum BasicType ty;
} funtype;

funtype funtype_new(enum BasicType);

void funtype_push(funtype *self, vartype e);

void funtype_free(funtype self);

typedef struct {
    char *name;
    funtype ty;
    bool transp;
    int min_level;
    span span;
} funtable_entry;

typedef struct funtable funtable;
struct funtable {
    int len;
    int cap;
    funtable_entry *buf;
    funtable *parent;
};

typedef struct {
    int n;
    int l;
} funtable_ref;

funtable *funtable_new(funtable *parent);

void funtable_push(funtable *self, funtable_entry e);

funtable_entry *funtable_get(funtable *self, funtable_ref r);

void funtable_free(funtable *self);
