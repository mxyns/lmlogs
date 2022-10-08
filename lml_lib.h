//
// Created by Maxence Younsi on 07/10/22.
//

#ifndef LMLOGS_LML_LIB_H
#define LMLOGS_LML_LIB_H

#define LML_COND(bool, ...) LML_COND_##bool(__VA_ARGS__)
#define LML_COND_0(...)
#define LML_COND_1(...) __VA_ARGS__

#define LML_PREV_OPT 1
#define LML_RECORD_MEMSIZE_OPT 1
#define LML_NO_OPT 0

#define LML_DECLARE_LOG(PREFIX, TYPE, PREV_OPT, MEMSIZE_OPT, FIELDS...) \
\
TYPE##_entry FIELDS;\
\
struct PREFIX##_stack {\
    size_t stack_cap;\
    size_t stack_size;\
    TYPE##_entry *entries;\
    struct PREFIX##_stack *next;                           \
    LML_COND(PREV_OPT,                                     \
        struct PREFIX##_stack *prev;                        \
        )   \
};\
\
struct PREFIX##_log {                                                   \
    LML_COND(MEMSIZE_OPT,                                                                    \
        size_t alloc_size;                                                  \
        )                                                                   \
    size_t stack_count;\
    struct PREFIX##_stack *head;\
    struct PREFIX##_stack *tail;\
};\
\
struct PREFIX##_stack *PREFIX##_stack_new(size_t stack_cap LML_COND(MEMSIZE_OPT, , size_t* alloc_size_ref)) {\
\
    size_t alloc_size = sizeof(struct PREFIX##_stack) + sizeof(TYPE##_entry) * stack_cap;\
    struct PREFIX##_stack *stack = malloc(alloc_size);\
    stack->stack_cap = stack_cap;\
    stack->stack_size = 0;\
    stack->entries = (TYPE##_entry *) (stack + 1);\
    stack->next = NULL;\
    LML_COND(PREV_OPT,                                     \
        stack->prev = NULL;                                 \
        )                                                               \
    LML_COND(MEMSIZE_OPT,                                               \
        if (alloc_size_ref)\
            *alloc_size_ref = alloc_size;                                   \
        )\
    return stack;\
}\
\
struct PREFIX##_log *PREFIX##_log_new(size_t stack_cap) {\
\
    size_t alloc_size = sizeof(struct PREFIX##_log) + sizeof(struct PREFIX##_stack) + sizeof(TYPE##_entry) * stack_cap;\
    struct PREFIX##_log *log = malloc(alloc_size);\
    struct PREFIX##_stack *stack = (struct PREFIX##_stack *) (log + 1);\
    stack->stack_cap = stack_cap;\
    stack->stack_size = 0;\
    stack->entries = (TYPE##_entry *) (stack + 1);\
    stack->next = NULL;\
    LML_COND(PREV_OPT,                                     \
        stack->prev = NULL;                                 \
        )\
    LML_COND(MEMSIZE_OPT,                                               \
        log->alloc_size = alloc_size;                                   \
    )                                                                   \
    log->stack_count = 1;\
    log->head = stack;\
    log->tail = stack;\
\
    return log;\
}\
\
struct PREFIX##_stack *PREFIX##_log_expand(struct PREFIX##_log *log) {\
\
    LML_COND(MEMSIZE_OPT,                                               \
        size_t alloc_size = 0;                                          \
        )\
    struct PREFIX##_stack *stack = PREFIX##_stack_new(log->tail->stack_cap LML_COND(MEMSIZE_OPT, , &alloc_size));\
    log->tail->next = stack;\
    LML_COND(PREV_OPT,                                                        \
        stack->prev = log->tail;\
        )                                     \
    log->tail = stack;\
    log->stack_count++;\
    LML_COND(MEMSIZE_OPT,                                               \
        log->alloc_size += alloc_size;                                  \
        )\
    return log->tail;\
}\
\
TYPE##_entry *PREFIX##_log_push(struct PREFIX##_log *log, TYPE##_entry entry) {\
\
    struct PREFIX##_stack *stack = log->tail;\
    if (stack->stack_size == stack->stack_cap)\
        stack = PREFIX##_log_expand(log);\
\
    stack->entries[stack->stack_size++] = entry;\
    return &stack->entries[stack->stack_size++];\
}\
\
void PREFIX##_log_dump(struct PREFIX##_log *log, void (*consumer)(struct PREFIX##_log* log, struct PREFIX##_stack* stack, size_t index, TYPE##_entry *entry, void *extra), void *extra) {\
\
    struct PREFIX##_stack *stack = log->head;\
    do {\
        for (size_t i = 0; i < stack->stack_size; ++i) {\
            consumer(log, stack, i, &stack->entries[i], extra);\
        }\
        stack = stack->next;\
    } while (stack != NULL);\
}\
\
void PREFIX##_log_free(struct PREFIX##_log *log) {\
\
\
    struct PREFIX##_stack* stack = log->head->next;\
    while (stack != NULL) {\
        struct PREFIX##_stack* to_free = stack;\
        stack = stack->next;\
        free(to_free);\
    }\
\
    free(log);\
}\

#endif //LMLOGS_LML_LIB_H
