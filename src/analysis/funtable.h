#pragma once

#include "analysis/vartable.h"

typedef struct {
    int len;
    int cap;
    vartype *buf;
    enum BasicType ret_ty;
} funtype;

funtype funtype_new(enum BasicType);

void funtype_push(funtype *self, vartype e);

void funtype_free(funtype self);

typedef struct {
    char *name;
    funtype ty;
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

funtable_entry funtable_get(funtable *self, funtable_ref e);

void funtable_free(funtable *self);
