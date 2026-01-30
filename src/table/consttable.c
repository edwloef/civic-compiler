#include "palm/memory.h"
#include "table/consttable.h"

bool double_biteq(double lhs, double rhs) {
    typedef union {
        int64_t intval;
        double floatval;
    } bitcast;
    bitcast l = {.floatval = lhs};
    bitcast r = {.floatval = rhs};
    return l.intval == r.intval;
}

consttable *consttable_new(void) {
    consttable *n = MEMmalloc(sizeof(consttable));
    *n = (consttable){0, 0, NULL};
    return n;
}

consttable_ref consttable_insert_int(consttable *self, int64_t intval) {
    for (int i = 0; i < self->len; i++) {
        if (self->buf[i].ty == TY_int && self->buf[i].intval == intval) {
            return (consttable_ref){i};
        }
    }

    return consttable_push(self, (consttable_entry){TY_int, .intval = intval});
}

consttable_ref consttable_insert_float(consttable *self, double floatval) {
    for (int i = 0; i < self->len; i++) {
        if (self->buf[i].ty == TY_float &&
            double_biteq(self->buf[i].floatval, floatval)) {
            return (consttable_ref){i};
        }
    }

    return consttable_push(self,
                           (consttable_entry){TY_float, .floatval = floatval});
}

consttable_ref consttable_push(consttable *self, consttable_entry e) {
    if (self->len == self->cap) {
        self->cap = self->cap ? self->cap * 2 : 4;
        self->buf = MEMrealloc(self->buf, self->cap * sizeof(consttable_entry));
    }

    self->buf[self->len++] = e;

    return (consttable_ref){self->len - 1};
}

void consttable_free(consttable *self) {
    MEMfree(self->buf);
    MEMfree(self);
}
