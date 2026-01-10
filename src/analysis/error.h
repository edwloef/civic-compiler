#pragma once

#include <stdbool.h>

#include "analysis/span.h"

#define ERROR(node, format, ...)                                               \
    {                                                                          \
        span span = SPAN(node);                                                \
        emit_message_with_span(span, true, format, ##__VA_ARGS__);             \
    }

#define NOTE(span, format, ...)                                                \
    emit_message_with_span(span, false, format, ##__VA_ARGS__);

void abort_on_error(void);
void emit_message(bool error, char *format, ...);
void emit_message_with_span(span span, bool error, char *format, ...);
