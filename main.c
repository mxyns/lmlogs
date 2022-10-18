#include <stdio.h>

#include <inttypes.h>
#include <malloc.h>
#include <sys/time.h>
#include "lml_lib.h"
#include "lml_time.h"
#include <time.h>

enum EVENT_CODE {
    IN,
    OUT
};

LML_DECLARE_ALL(lml, struct lml_entry, LML_PREV_OPT, LML_RECORD_MEMSIZE_OPT, LML_CLEAR_OPT);

LML_DEFINE_TYPES(lml, struct lml_entry, LML_PREV_OPT, LML_RECORD_MEMSIZE_OPT, LML_CLEAR_OPT,  {
    size_t current_alloc_size;
    uint64_t timestamp;
    uint64_t data;
    enum EVENT_CODE event_code;
});

LML_DEFINE_FUNCS(lml, struct lml_entry, LML_PREV_OPT, LML_RECORD_MEMSIZE_OPT, LML_CLEAR_OPT);


void lml_put_entry_to_file(struct lml_log *log, struct lml_stack *stack, size_t index, struct lml_entry* entry, void *file_extra);
void lml_put_entry_to_file(struct lml_log *log, struct lml_stack *stack, size_t index, struct lml_entry* entry, void *file_extra) {
    FILE *file = (FILE *)file_extra;
    if (!(stack == log->head && index == 0)
        && entry->current_alloc_size == ((index == 0 ? stack->prev : stack)->entries[(index == 0 ? stack->prev->stack_cap : index) - 1]).current_alloc_size) {
            fprintf(file, "[-][%lu] event=%s, data=%lu\n", entry->timestamp, entry->event_code ? "IN" : "OUT", entry->data);
    } else {
        fprintf(file, "[%lu][%lu] event=%s, data=%lu\n", entry->current_alloc_size, entry->timestamp, entry->event_code ? "IN" : "OUT", entry->data);
    }
}

int main(void) {

    uint64_t N = 128;
    {
        struct timespec ts;
        FILE *naive_file = fopen("./naive.txt", "w");
        LML_TIME(naive_duration, {

                     for (uint64_t i = 0; i < N; i++) {
                         LML_TIME_FUNC(&ts);
                         fprintf(naive_file, "[%lu] event=%s, data=%lu\n", ts.tv_sec*1000000000+ts.tv_nsec, i % 2 == 0 ? "IN" : "OUT", i);
                     }
                 });

        printf(LML_TIME_PFORMAT"\n", LML_TIME_PPARAMS(naive_duration));
        fclose(naive_file);
    }

    {
        struct timespec ts;

        FILE *smart_file = fopen("./smart.txt", "w+");
        struct lml_log* log = lml_log_new(4096*2);
        LML_TIME(smart_duration, {
            for (uint64_t i = 0; i < N; i++) {
                LML_TIME_FUNC(&ts);
                struct lml_entry *entry = lml_log_push(log, (struct lml_entry) {
                        .current_alloc_size = 0,
                        .timestamp = ts.tv_sec * 1000000000 + ts.tv_nsec,
                        .event_code = i % 2 == 0 ? IN : OUT,
                        .data = i
                })->current_alloc_size = log->alloc_size;
            }
        });
        printf(LML_TIME_PFORMAT"\n", LML_TIME_PPARAMS(smart_duration));

        printf("Dumping %lu blocks\n", log->stack_count);
        fprintf(smart_file, "==== AFTER INSERT ==== Dumping %lu blocks\n", log->stack_count);
        LML_TIME(smart_dump_duration, {
            lml_log_dump(log, lml_put_entry_to_file, (void *)smart_file);
        });
        printf(LML_TIME_PFORMAT"\n", LML_TIME_PPARAMS(smart_dump_duration));

        lml_log_clear(log);
        printf("Dumping %lu blocks\n", log->stack_count);
        fprintf(smart_file, "==== AFTER CLEAR ==== Dumping %lu blocks\n", log->stack_count);
        LML_TIME(cleared_smart_dump_duration, {
            lml_log_dump(log, lml_put_entry_to_file, (void *)smart_file);
        });

        for (uint64_t i = 0; i < log->head->stack_cap + 1; i++) {
            LML_TIME_FUNC(&ts);
            lml_log_push(log, (struct lml_entry) {
                                      .current_alloc_size = 0,
                                      .timestamp = ts.tv_sec * 1000000000 + ts.tv_nsec,
                                      .event_code = i%2?IN:OUT,
                                      .data = i
                              })->current_alloc_size = log->alloc_size;
        }

        printf(LML_TIME_PFORMAT"\n", LML_TIME_PPARAMS(cleared_smart_dump_duration));
        printf("Dumping %lu blocks\n", log->stack_count);
        fprintf(smart_file, "==== AFTER REINSERT ==== Dumping %lu blocks\n", log->stack_count);
        LML_TIME(reinsert_cleared_smart_dump_duration, {
            lml_log_dump(log, lml_put_entry_to_file, (void *)smart_file);
        });
        printf(LML_TIME_PFORMAT"\n", LML_TIME_PPARAMS(reinsert_cleared_smart_dump_duration));


        fclose(smart_file);
        lml_log_free(log);
    }

    return 0;
}
