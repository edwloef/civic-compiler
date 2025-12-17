#pragma once

#include "ccngen/enum.h"
#include "palm/hash_table.h"

typedef htable_st *htable_st_ptr;

typedef struct {
	enum BasicType ty;
	int dims;
} type;

typedef struct funtable funtable;
typedef funtable *funtable_ptr;
struct funtable {
    char *name;
    enum BasicType ret_ty;
    int param_count;
    type *param_tys;
    int level;
    funtable_ptr prev;
};

typedef struct symtable symtable;
typedef symtable *symtable_ptr;
struct symtable {
    char *name;
    type ty;
    int level;
    symtable_ptr prev;
};
