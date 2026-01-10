#pragma once

#include "analysis/span.h"
#include "palm/memory.h"
#include "palm/str.h"

#define ERROR(node, format, ...)                                               \
    {                                                                          \
        span span = SPAN(node);                                                \
        char *message = STRfmt(format, ##__VA_ARGS__);                         \
        emit_message_at_span(span, message, true);                             \
        MEMfree(message);                                                      \
    }

#define NOTE(span, format, ...)                                                \
    {                                                                          \
        char *message = STRfmt(format, ##__VA_ARGS__);                         \
        emit_message_at_span(span, message, false);                            \
        MEMfree(message);                                                      \
    }

void abort_on_error(void);

void emit_message_at_span(span span, char *message, bool err);
