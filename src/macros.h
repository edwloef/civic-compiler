#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

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

#define WRAP(o)                                                                \
    {                                                                          \
        node = ASTmonop(node, o);                                              \
        MONOP_RESOLVED_TY(node) = EXPR_RESOLVED_TY(MONOP_EXPR(node));          \
        CCNcycleNotify();                                                      \
    }
