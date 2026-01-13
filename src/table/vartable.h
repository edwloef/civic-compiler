#pragma once

#include <stdbool.h>

#include "ccngen/enum.h"
#include "error/span.h"

typedef struct {
    enum BasicType ty;
    int dims;
} vartype;

typedef struct {
    char *name;
    vartype ty;
    span span;
    bool loopvar;
} vartable_entry;

typedef struct vartable vartable;
struct vartable {
    int len;
    int cap;
    vartable_entry *buf;
    vartable *parent;
};

typedef struct {
    int n;
    int l;
} vartable_ref;

vartable *vartable_new(vartable *parent);

void vartable_push(vartable *self, vartable_entry e);

vartable_entry *vartable_get(vartable *self, vartable_ref r);

void vartable_free(vartable *self);
