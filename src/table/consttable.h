#pragma once

#include <stdbool.h>

#include "ccngen/enum.h"

bool double_biteq(double lhs, double rhs);

typedef struct {
    int l;
} consttable_ref;

typedef struct {
    enum BasicType ty;
    union {
        int intval;
        double floatval;
    } val;
} consttable_entry;

typedef struct {
    int len, cap;
    consttable_entry *buf;
} consttable;

consttable *consttable_new(void);

consttable_ref consttable_insert_int(consttable *self, int intval);

consttable_ref consttable_insert_float(consttable *self, double floatval);

consttable_ref consttable_push(consttable *self, consttable_entry e);

void consttable_free(consttable *self);
