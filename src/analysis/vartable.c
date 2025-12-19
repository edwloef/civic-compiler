#include "analysis/vartable.h"
#include "palm/ctinfo.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "print/print.h"

vartable *vartable_new(vartable *parent) {
    vartable *n = MEMmalloc(sizeof(vartable));
    n->len = 0;
    n->cap = 0;
    n->buf = NULL;
    n->parent = parent;
    return n;
}

void vartable_insert(vartable *self, vartable_entry e) {
    for (int l = self->len - 1; l >= 0; l--) {
        vartable_entry entry = self->buf[l];
        if (entry.valid && STReq(entry.name, e.name)) {
            CTI(CTI_ERROR, true, "couldn't re-declare variable '%s'", e.name);
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

vartable_ref vartable_resolve(vartable *self, char *name) {
    int n = 0;
    while (self) {
        for (int l = self->len - 1; l >= 0; l--) {
            vartable_entry entry = self->buf[l];
            if (entry.valid && STReq(entry.name, name)) {
                vartable_ref r = {n, l};
                return r;
            }
        }

        self = self->parent;
        n++;
    }

    CTI(CTI_ERROR, true, "couldn't resolve variable '%s'", name);
    CTIabortOnError();
    exit(EXIT_FAILURE);
}

vartable_entry vartable_get(vartable *self, vartable_ref e) {
    for (int i = 0; i < e.n; i++)
        self = self->parent;
    return self->buf[e.l];
}

void vartable_free(vartable *self) {
    MEMfree(self->buf);
    MEMfree(self);
}
