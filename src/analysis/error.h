#pragma once

#include <stdbool.h>

#include "analysis/span.h"

#define ERROR(node, format, ...) MESSAGE(L_ERROR, node, format, ##__VA_ARGS__)
#define WARNING(node, format, ...)                                             \
    MESSAGE(L_WARNING, node, format, ##__VA_ARGS__)
#define INFO(node, format, ...) MESSAGE(L_INFO, node, format, ##__VA_ARGS__)
#define MESSAGE(level, node, format, ...)                                      \
    {                                                                          \
        span span = SPAN(node);                                                \
        emit_message_with_span(span, level, format, ##__VA_ARGS__);            \
    }

typedef enum { L_ERROR, L_WARNING, L_INFO } level;

void abort_on_error(void);
void emit_message(level level, char *format, ...);
void emit_message_with_span(span span, level level, char *format, ...);
