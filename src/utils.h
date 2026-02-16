#pragma once

#include <stdio.h>

#include "ccngen/ast.h"
#include "palm/dbug.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define NOOP()                                                                 \
    {                                                                          \
        TRAVchildren(node);                                                    \
        return node;                                                           \
    }

#define OUT_OF_LIFETIME()                                                      \
    {                                                                          \
        DBUG_ASSERT(false, "Unreachable.");                                    \
        return node;                                                           \
    }

#define TYPE(n)                                                                \
    (thin_vartype) {                                                           \
        EXPR_RESOLVED_TY(n), EXPR_RESOLVED_DIMS(n)                             \
    }

#define TAKE(n)                                                                \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = NULL;                                                              \
        CCNfree(node);                                                         \
        node = tmp;                                                            \
        CCNcycleNotify();                                                      \
    }

#define SWAP(n, e)                                                             \
    {                                                                          \
        node_st *tmp = n;                                                      \
        n = e;                                                                 \
        e = NULL;                                                              \
        CCNfree(tmp);                                                          \
        CCNcycleNotify();                                                      \
    }

bool is_trivial_scalar(node_st *node);

node_st *inline_stmts(node_st *node, node_st *stmts);

FILE *spawn_command(FILE *in, const char *cmd, char *const argv[]);
