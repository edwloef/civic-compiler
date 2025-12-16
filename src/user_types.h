#pragma once

#include "palm/hash_table.h"

typedef htable_st *htable_st_ptr;

typedef struct funtable funtable;
typedef funtable *funtable_ptr;
struct funtable {
	char *name;
    int arity;
    funtable_ptr prev;
    funtable_ptr next;
};
