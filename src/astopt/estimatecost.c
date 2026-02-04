#include "ccn/ccn.h"
#include "utils.h"

#define COST(n)                                                                \
    {                                                                          \
        TRAVchildren(node);                                                    \
        DATA_EC_GET()->cost += (n);                                            \
        return node;                                                           \
    }

void ECinit(void) {}
void ECfini(void) {}

node_st *ECstmts(node_st *node) {
    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_CALL && CALL_RESOLVED_TY(stmt) != TY_void) {
        COST(1);
    } else {
        COST(0);
    }
}

node_st *ECassign(node_st *node) {
    COST(0);
}

node_st *ECifelse(node_st *node) {
    if (IFELSE_ELSE_BLOCK(node)) {
        COST(2);
    } else {
        COST(1);
    }
}

node_st *ECwhile(node_st *node) {
    OUT_OF_LIFETIME();
}

node_st *ECdowhile(node_st *node) {
    COST(1);
}

node_st *ECfor(node_st *node) {
    OUT_OF_LIFETIME();
}

node_st *ECreturn(node_st *node) {
    COST(1);
}

node_st *ECarrexprs(node_st *node) {
    OUT_OF_LIFETIME();
}

node_st *ECmonop(node_st *node) {
    COST(1);
}

node_st *ECbinop(node_st *node) {
    if (BINOP_OP(node) == BO_and || BINOP_OP(node) == BO_or) {
        COST(3)
    } else {
        COST(1);
    }
}

node_st *ECcast(node_st *node) {
    if (EXPR_RESOLVED_TY(CAST_EXPR(node)) == TY_bool) {
        COST(4);
    } else {
        COST(1);
    }
}

node_st *ECcall(node_st *node) {
    COST(2);
}

node_st *ECvarref(node_st *node) {
    if (VARREF_WRITE(node) && VARREF_EXPRS(node)) {
        COST(2);
    } else {
        COST(1);
    }
}

node_st *ECint(node_st *node) {
    COST(1);
}

node_st *ECfloat(node_st *node) {
    COST(1);
}

node_st *ECbool(node_st *node) {
    COST(1);
}

node_st *ECmalloc(node_st *node) {
    COST(1);
}

node_st *ECconstref(node_st *node) {
    OUT_OF_LIFETIME();
}
