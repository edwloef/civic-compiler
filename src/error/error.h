#pragma once

#define SPAN(node)                                                             \
    (span) {                                                                   \
        NODE_BLINE(node), NODE_BCOL(node), NODE_ELINE(node), NODE_ECOL(node),  \
            NODE_FILENAME(node)                                                \
    }

#define ERROR(node, format, ...) MESSAGE(L_ERROR, node, format, ##__VA_ARGS__)

#define WARNING(node, format, ...)                                             \
    MESSAGE(L_WARNING, node, format, ##__VA_ARGS__)

#define INFO(node, format, ...) MESSAGE(L_INFO, node, format, ##__VA_ARGS__)

#define MESSAGE(level, node, format, ...)                                      \
    emit_message_with_span(SPAN(node), level, format, ##__VA_ARGS__)

typedef enum { L_ERROR, L_WARNING, L_INFO } level;

typedef struct {
    int bl, bc, el, ec;
    char *file;
} span;

void abort_on_error(void);

void emit_message(level level, char *format, ...);

void emit_message_with_span(span span, level level, char *format, ...)
    __attribute__((format(printf, 3, 4)));
