#pragma once

#include "analysis/vartable.h"
#include "ccngen/enum.h"

typedef struct {
    enum BasicType ret_ty;
    int param_count;
    vartype *param_tys;
} funtype;

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

funtable_ref funtable_resolve(funtable *self, char *name, int param_count);

void funtable_free(funtable *self);
