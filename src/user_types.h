#pragma once

#include "ccngen/enum.h"
#include "palm/hash_table.h"

typedef htable_st *htable_st_ptr;

typedef struct funtable funtable;
typedef funtable *funtable_ptr;
struct funtable {
    char *name;
    int arity;
    enum BasicType ret_ty;
    int level;
    funtable_ptr prev;
};

typedef struct symtable symtable;
typedef symtable *symtable_ptr;
struct symtable {
    char *name;
    enum BasicType ty;
    int dims;
    int level;
    symtable_ptr prev;
};
