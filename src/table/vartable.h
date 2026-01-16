#pragma once

#include <stdbool.h>

#include "ccngen/enum.h"
#include "error/span.h"

typedef struct {
    int n;
    int l;
} vartable_ref;

typedef struct {
    int len;
    int cap;
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
    int read_count;
    int write_count;
    span span;
    bool external;
    bool exported;
    bool loopvar;
} vartable_entry;

typedef struct vartable vartable;
struct vartable {
    int len;
    int cap;
    vartable_entry *buf;
    vartable *parent;
};

vartable *vartable_new(vartable *parent);

vartable_ref vartable_push(vartable *self, vartable_entry e);

vartable_entry *vartable_get(vartable *self, vartable_ref r);

void vartable_free(vartable *self);
