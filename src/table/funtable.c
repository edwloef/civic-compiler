#include "error/error.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "table/table.h"

funtype funtype_new(enum BasicType ty) {
    funtype n;
    n.len = 0;
    n.cap = 0;
    n.buf = NULL;
    n.ty = ty;
    return n;
}

void funtype_push(funtype *self, thin_vartype e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(thin_vartype));
    }

    self->buf[self->len++] = e;
}

void funtype_free(funtype self) {
    MEMfree(self.buf);
}

funtable *funtable_new(funtable *parent) {
    funtable *n = MEMmalloc(sizeof(funtable));
    n->len = 0;
    n->cap = 0;
    n->buf = NULL;
    n->parent = parent;
    return n;
}

funtable_ref funtable_insert(funtable *self, funtable_entry e, node_st *id) {
    for (int l = self->len - 1; l >= 0; l--) {
        funtable_entry entry = self->buf[l];
        if (entry.ty.len == e.ty.len && STReq(entry.name, e.name)) {
            ERROR(id, "can't re-declare function '%s' with %d parameters",
                  e.name, e.ty.len);
            emit_message_with_span(entry.span, L_INFO,
                                   "function '%s' previously declared here",
                                   e.name);
            funtype_free(e.ty);
            return (funtable_ref){-1, -1};
        }
    }

    return funtable_push(self, e);
}

funtable_ref funtable_push(funtable *self, funtable_entry e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(funtable_entry));
    }

    self->buf[self->len++] = e;

    return (funtable_ref){self->len - 1, 0};
}

funtable_ref funtable_resolve(funtable *self, node_st *call) {
    int param_count = 0;
    for (node_st *expr = CALL_EXPRS(call); expr; expr = EXPRS_NEXT(expr)) {
        param_count++;
    }

    int n = 0;
    while (self) {
        for (int l = self->len - 1; l >= 0; l--) {
            funtable_entry entry = self->buf[l];
            if (entry.ty.len == param_count &&
                STReq(entry.name, ID_VAL(CALL_ID(call)))) {
                funtable_ref r = {n, l};
                return r;
            }
        }

        self = self->parent;
        n++;
    }

    ERROR(call, "can't resolve function '%s' with %d parameters",
          ID_VAL(CALL_ID(call)), param_count);
    return (funtable_ref){-1, -1};
}

static funtable_entry error = {.name = "error", .ty = {.ty = TY_error}};
funtable_entry *funtable_get(funtable *self, funtable_ref r) {
    if (r.n == -1 && r.l == -1) {
        return &error;
    }

    for (int i = 0; i < r.n; i++) {
        self = self->parent;
    }

    return &self->buf[r.l];
}

void funtable_free(funtable *self) {
    for (int i = 0; i < self->len; i++) {
        funtype_free(self->buf[i].ty);
    }

    MEMfree(self->buf);
    MEMfree(self);
}
