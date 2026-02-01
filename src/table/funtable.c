#include "ccngen/ast.h"
#include "error/error.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "table/funtable.h"

funtype funtype_new(enum BasicType ty) {
    return (funtype){0, 0, NULL, ty};
}

void funtype_push(funtype *self, thin_vartype e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(thin_vartype));
    }

    self->buf[self->len++] = e;
}

funtype funtype_copy(funtype *self) {
    return (funtype){self->len, self->len,
                     MEMcopy(self->len * sizeof(thin_vartype), self->buf),
                     self->ty};
}

void funtype_free(funtype self) {
    MEMfree(self.buf);
}

funtable *funtable_new(funtable *parent) {
    funtable *n = MEMmalloc(sizeof(funtable));
    *n = (funtable){0, 0, NULL, parent};
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
            return (funtable_ref){0, -1};
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

    return (funtable_ref){0, self->len - 1};
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
                return (funtable_ref){n, l};
            }
        }

        self = self->parent;
        n++;
    }

    ERROR(call, "can't resolve function '%s' with %d parameters",
          ID_VAL(CALL_ID(call)), param_count);
    return (funtable_ref){0, -1};
}

static funtable_entry error = {.name = "error", .ty = {.ty = TY_error}};
funtable_entry *funtable_get(funtable *self, funtable_ref r) {
    if (r.l == -1) {
        return &error;
    }

    for (int i = 0; i < r.n; i++) {
        self = self->parent;
    }

    return &self->buf[r.l];
}

void funtable_entry_free(funtable_entry e) {
    MEMfree(e.name);
    MEMfree(e.mangled_name);
    funtype_free(e.ty);
}

void funtable_shallow_free(funtable *self) {
    MEMfree(self->buf);
    MEMfree(self);
}

void funtable_free(funtable *self) {
    for (int i = 0; i < self->len; i++) {
        funtable_entry_free(self->buf[i]);
    }
    funtable_shallow_free(self);
}
