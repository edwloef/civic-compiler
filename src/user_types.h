#pragma once

#include <stdio.h>

#include "palm/hash_table.h"
#include "table/consttable.h"
#include "table/funtable.h"
#include "table/vartable.h"

typedef htable_st *htable_st_ptr;
typedef consttable *consttable_ptr;
typedef funtable *funtable_ptr;
typedef vartable *vartable_ptr;

typedef FILE *FILE_ptr;

#define CCN_USES_UNSAFE_ACKNOWLEDGE
typedef char *tablestring;
