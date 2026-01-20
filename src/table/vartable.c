#include "error/error.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "table/table.h"

vartype vartype_new(enum BasicType ty) {
    vartype n = {0, 0, NULL, ty};
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
    *n = (vartable){0, 0, NULL, parent};
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
            return (vartable_ref){0, -1};
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

    return (vartable_ref){0, self->len - 1};
}

vartable_ref vartable_resolve(vartable *self, node_st *id) {
    int n = 0;
    while (self) {
        for (int l = self->len - 1; l >= 0; l--) {
            vartable_entry entry = self->buf[l];
            if (!entry.loopvar && STReq(entry.name, ID_VAL(id))) {
                return (vartable_ref){n, l};
            }
        }

        self = self->parent;
        n++;
    }

    ERROR(id, "can't resolve variable '%s'", ID_VAL(id));
    return (vartable_ref){0, -1};
}

node_st *vartable_temp_var(vartable *self, enum BasicType ty) {
    static int temp_var_index = 0;
    char *name = STRfmt("_%d", temp_var_index++);
    vartable_entry e = {
        name, vartype_new(ty), {0, 0, 0, 0, NULL}, 0, 0, false, false, false,
        false};
    node_st *ref = ASTvarref(ASTid(name), NULL);
    VARREF_L(ref) = vartable_push(self, e).l;
    return ref;
}

static vartable_entry error = {.name = "error", .ty = {.ty = TY_error}};
vartable_entry *vartable_get(vartable *self, vartable_ref r) {
    if (r.l == -1) {
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
