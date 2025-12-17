#pragma once

#include "palm/hash_table.h"

typedef htable_st *htable_st_ptr;

typedef struct funtable funtable;
typedef funtable *funtable_ptr;
struct funtable {
    char *name;
    int arity;
    int level;
    funtable_ptr prev;
};
