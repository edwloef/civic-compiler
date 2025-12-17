#include "analysis/funtable.h"
#include "palm/ctinfo.h"
#include "palm/memory.h"
#include "palm/str.h"

funtable *funtable_new(funtable *parent) {
    funtable *n = MEMmalloc(sizeof(funtable));
    n->len = 0;
    n->cap = 0;
    n->buf = NULL;
    n->parent = parent;
    return n;
}

void funtable_insert(funtable *self, funtable_entry e) {
    for (int i = 0; i < self->len; i++) {
        funtable_entry entry = self->buf[i];
        if (entry.ty.param_count == e.ty.param_count &&
            STReq(entry.name, e.name)) {
            CTI(CTI_ERROR, true,
                "couldn't re-declare function '%s' with arity %d", e.name,
                e.ty.param_count);
            CTIabortOnError();
        }
    }

    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(funtable_entry));
    }

    self->buf[self->len++] = e;
}

funtable_ref funtable_resolve(funtable *self, char *name, int param_count) {
    int n = 0;
    while (self) {
        for (int l = 0; l < self->len; l++) {
            funtable_entry entry = self->buf[l];
            if (entry.ty.param_count == param_count &&
                STReq(entry.name, name)) {
                funtable_ref r = {n, l};
                return r;
            }
        }

        self = self->parent;
    }

    CTI(CTI_ERROR, true, "couldn't resolve function '%s' with arity %d", name,
        param_count);
    CTIabortOnError();
    exit(1);
}

void funtable_free(funtable *self) {
    for (int i = 0; i < self->len; i++)
        MEMfree(self->buf[i].ty.param_tys);
    MEMfree(self->buf);
    MEMfree(self);
}
