#pragma once

#include <stdbool.h>

#include "ccngen/enum.h"
#include "error/span.h"

typedef struct {
    int n, l;
} vartable_ref;

typedef struct {
    int len, cap;
    vartable_ref *buf;
    enum BasicType ty;
} vartype;

typedef struct {
    enum BasicType ty;
    int dims;
} thin_vartype;

vartype vartype_new(enum BasicType ty);

void vartype_push(vartype *self, vartable_ref e);

void vartype_free(vartype self);

typedef struct {
    char *name;
    vartype ty;
    span span;
    int read_count, write_count;
    bool external, exported, param, loopvar;
} vartable_entry;

typedef struct vartable vartable;
struct vartable {
    int len, cap;
    vartable_entry *buf;
    vartable *parent;
};

vartable *vartable_new(vartable *parent);

vartable_ref vartable_push(vartable *self, vartable_entry e);

vartable_entry *vartable_get(vartable *self, vartable_ref r);

void vartable_free(vartable *self);
