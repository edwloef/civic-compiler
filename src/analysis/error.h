#include "ccngen/ast.h"
#include "palm/ctinfo.h"
#include "palm/memory.h"
#include "palm/str.h"

#define SPAN(node)                                                             \
    {NODE_BLINE(node), NODE_BCOL(node), NODE_ELINE(node), NODE_ECOL(node)}

#define ERROR(node, format, ...)                                               \
    {                                                                          \
        char *error = STRfmt(format, ##__VA_ARGS__);                           \
        span span = SPAN(node);                                                \
        emit_message_at_node(CTI_ERROR, span, error);                          \
        MEMfree(error);                                                        \
    }

typedef struct {
    int bl, bc, el, ec;
} span;

void emit_message_at_node(enum cti_type type, span span, char *error);
