#include "analysis/funtable.h"
#include "analysis/error.h"
#include "analysis/resolve.h"
#include "palm/memory.h"
#include "palm/str.h"

funtype funtype_new(enum BasicType ret_ty) {
    funtype n;
    n.len = 0;
    n.cap = 0;
    n.buf = NULL;
    n.ret_ty = ret_ty;
    return n;
}

void funtype_push(funtype *self, vartype e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(funtable_entry));
    }

    self->buf[self->len++] = e;
}

void funtype_free(funtype self) { MEMfree(self.buf); }

funtable *funtable_new(funtable *parent) {
    funtable *n = MEMmalloc(sizeof(funtable));
    n->len = 0;
    n->cap = 0;
    n->buf = NULL;
    n->parent = parent;
    return n;
}

void funtable_insert(funtable *self, funtable_entry e, node_st *id) {
    for (int l = self->len - 1; l >= 0; l--) {
        funtable_entry entry = self->buf[l];
        if (entry.ty.len == e.ty.len && STReq(entry.name, e.name)) {
            ERROR(id, "couldn't re-declare function '%s' with %d parameters",
                  e.name, e.ty.len);
            return;
        }
    }

    funtable_push(self, e);
}

void funtable_push(funtable *self, funtable_entry e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(funtable_entry));
    }

    self->buf[self->len++] = e;
}

funtable_ref funtable_resolve(funtable *self, node_st *id, int param_count) {
    int n = 0;
    while (self) {
        for (int l = self->len - 1; l >= 0; l--) {
            funtable_entry entry = self->buf[l];
            if (entry.ty.len == param_count && STReq(entry.name, ID_VAL(id))) {
                funtable_ref r = {n, l};
                return r;
            }
        }

        self = self->parent;
        n++;
    }

    ERROR(id, "couldn't resolve function '%s' with %d parameters", ID_VAL(id),
          param_count);
    funtable_ref r = {-1, -1};
    return r;
}

funtable_entry funtable_get(funtable *self, funtable_ref e) {
    if (e.n == -1 && e.l == -1) {
        funtable_entry e = {"error", {0, 0, NULL, TY_error}};
        return e;
    }

    for (int i = 0; i < e.n; i++)
        self = self->parent;
    return self->buf[e.l];
}

void funtable_free(funtable *self) {
    for (int i = 0; i < self->len; i++)
        funtype_free(self->buf[i].ty);
    MEMfree(self->buf);
    MEMfree(self);
}
