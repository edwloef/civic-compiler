#pragma once

#include "analysis/span.h"
#include "palm/memory.h"
#include "palm/str.h"

#define ERROR(node, format, ...)                                               \
    {                                                                          \
        char *error = STRfmt(format, ##__VA_ARGS__);                           \
        span span = SPAN(node);                                                \
        emit_message_at_node(span, error, true);                               \
        MEMfree(error);                                                        \
    }

#define NOTE(span, format, ...)                                                \
    {                                                                          \
        char *error = STRfmt(format, ##__VA_ARGS__);                           \
        emit_message_at_node(span, error, false);                              \
        MEMfree(error);                                                        \
    }

void abort_on_error(void);

void emit_message_at_node(span span, char *error, bool err);
