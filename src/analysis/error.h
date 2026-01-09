#include "ccngen/ast.h"

#define ERROR(node, format, ...)                                               \
    {                                                                          \
        char *error = STRfmt(format, __VA_ARGS__);                             \
        emit_error_at_node(node, error);                                       \
        MEMfree(error);                                                        \
    }

void emit_error_at_node(node_st *node, char *error);
