#include "ccn/ccn.h"
#include "utils.h"

node_st *inline_stmts(node_st *node, node_st *stmts) {
    if (stmts) {
        node_st *tmp = stmts;
        while (STMTS_NEXT(tmp)) {
            tmp = STMTS_NEXT(tmp);
        }
        STMTS_NEXT(tmp) = node;
        CCNcycleNotify();
        return stmts;
    } else {
        return node;
    }
}
