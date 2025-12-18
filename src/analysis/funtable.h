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

void funtable_insert(funtable *self, funtable_entry e);

void funtable_push(funtable *self, funtable_entry e);

funtable_ref funtable_resolve(funtable *self, char *name, int param_count);

void funtable_free(funtable *self);
