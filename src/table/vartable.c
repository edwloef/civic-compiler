#include "error/error.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "table/table.h"

vartable *vartable_new(vartable *parent) {
    vartable *n = MEMmalloc(sizeof(vartable));
    n->len = 0;
    n->cap = 0;
    n->buf = NULL;
    n->parent = parent;
    return n;
}

void vartable_insert(vartable *self, vartable_entry e, node_st *id) {
    for (int l = self->len - 1; l >= 0; l--) {
        vartable_entry entry = self->buf[l];
        if (!entry.loopvar && STReq(entry.name, e.name)) {
            ERROR(id, "can't re-declare variable '%s'", e.name);
            emit_message_with_span(entry.span, L_INFO,
                                   "variable '%s' previously declared here",
                                   e.name);
            return;
        }
    }

    vartable_push(self, e);
}

void vartable_push(vartable *self, vartable_entry e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(vartable_entry));
    }

    self->buf[self->len++] = e;
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

node_st *vartable_temp_var(vartable *self, vartype ty) {
    int n = 0;
    for (vartable *parent = self->parent; parent; parent = parent->parent)
        n++;
    char *name = STRfmt("_%d_%d", n, self->len);
    vartable_entry e = {name,  ty,    0,    0, {0, 0, 0, 0, NULL},
                        false, false, false};
    vartable_push(self, e);
    return ASTvarref(ASTid(name), NULL);
}

static vartable_entry error = {.name = "error", .ty = {.ty = TY_error}};
vartable_entry *vartable_get(vartable *self, vartable_ref r) {
    if (r.n == -1 && r.l == -1)
        return &error;
    for (int i = 0; i < r.n; i++)
        self = self->parent;
    return &self->buf[r.l];
}

void vartable_free(vartable *self) {
    MEMfree(self->buf);
    MEMfree(self);
}
