#include "error/error.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "table/table.h"

vartype vartype_new(enum BasicType ty) {
    vartype n;
    n.len = 0;
    n.cap = 0;
    n.buf = NULL;
    n.ty = ty;
    return n;
}

void vartype_push(vartype *self, vartable_ref e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(vartable_ref));
    }

    self->buf[self->len++] = e;
}

void vartype_free(vartype self) {
    MEMfree(self.buf);
}

vartable *vartable_new(vartable *parent) {
    vartable *n = MEMmalloc(sizeof(vartable));
    n->len = 0;
    n->cap = 0;
    n->buf = NULL;
    n->parent = parent;
    return n;
}

vartable_ref vartable_insert(vartable *self, vartable_entry e, node_st *id) {
    for (int l = self->len - 1; l >= 0; l--) {
        vartable_entry entry = self->buf[l];
        if (!entry.loopvar && STReq(entry.name, e.name)) {
            ERROR(id, "can't re-declare variable '%s'", e.name);
            emit_message_with_span(entry.span, L_INFO,
                                   "variable '%s' previously declared here",
                                   e.name);
            vartype_free(e.ty);
            return (vartable_ref){-1, -1};
        }
    }

    return vartable_push(self, e);
}

vartable_ref vartable_push(vartable *self, vartable_entry e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(vartable_entry));
    }

    self->buf[self->len++] = e;

    return (vartable_ref){self->len - 1, 0};
}

vartable_ref vartable_resolve(vartable *self, node_st *id) {
    int n = 0;
    while (self) {
        for (int l = self->len - 1; l >= 0; l--) {
            vartable_entry entry = self->buf[l];
            if (!entry.loopvar && STReq(entry.name, ID_VAL(id))) {
                vartable_ref r = {n, l};
                return r;
            }
        }

        self = self->parent;
        n++;
    }

    ERROR(id, "can't resolve variable '%s'", ID_VAL(id));
    vartable_ref r = {-1, -1};
    return r;
}

node_st *vartable_temp_var(vartable *self, enum BasicType ty) {
    int n = 0;
    for (vartable *parent = self->parent; parent; parent = parent->parent) {
        n++;
    }

    char *name = STRfmt("_%d_%d", n, self->len);
    vartable_entry e = {name,  vartype_new(ty), 0,    0, {0, 0, 0, 0, NULL},
                        false, false,           false};
    vartable_push(self, e);
    node_st *ref = ASTvarref(ASTid(name), NULL);
    VARREF_L(ref) = self->len - 1;
    return ref;
}

static vartable_entry error = {.name = "error", .ty = {.ty = TY_error}};
vartable_entry *vartable_get(vartable *self, vartable_ref r) {
    if (r.n == -1 && r.l == -1) {
        return &error;
    }

    for (int i = 0; i < r.n; i++) {
        self = self->parent;
    }

    return &self->buf[r.l];
}

void vartable_free(vartable *self) {
    for (int i = 0; i < self->len; i++) {
        vartype_free(self->buf[i].ty);
    }

    MEMfree(self->buf);
    MEMfree(self);
}
