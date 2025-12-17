#include <stdio.h>

#include "ccn/ccn.h"
#include "ccngen/ast.h"
#include "ccngen/trav_data.h"
#include "palm/hash_table.h"
#include "palm/memory.h"
#include "palm/str.h"

void SCIinit(void) { DATA_SCI_GET()->count = HTnew_String(256); }

void SCIfini(void) {
    htable_st_ptr count = DATA_SCI_GET()->count;

    for (htable_iter_st *iter = HTiterate(count); iter != NULL;
         iter = HTiterateNext(iter)) {
        char *name = HTiterKey(iter);
        int *count = HTiterValue(iter);
        printf("%d instances of id '%s'\n", *count, name);
        MEMfree(name);
        MEMfree(count);
    }

    HTdelete(count);
}

node_st *SCIid(node_st *node) {
    char *name = ID_VAL(node);
    htable_st_ptr count = DATA_SCI_GET()->count;

    int *already = HTlookup(count, name);
    if (already == NULL) {
        int *data = MEMmalloc(sizeof(int));
        (*data) = 1;
        HTinsert(count, STRcpy(name), data);
    } else {
        (*already)++;
    }

    return node;
}
