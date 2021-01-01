#include "cmocka_all.h"
#include "test_error.h"
#include "test_memory.h"

struct test_error_monitor {
    struct error_monitor m_em;
};

void test_error_monitor_log(struct error_info * info, void * context, const char * fmt, va_list args) {
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

test_error_monitor_t test_error_monitor_create() {
    test_error_monitor_t tem = mem_alloc(test_allocrator(), sizeof(struct test_error_monitor));
    cpe_error_monitor_init(
        &tem->m_em, test_error_monitor_log,
        NULL);
    return tem;
}

void test_error_monitor_free(test_error_monitor_t  tem) {
    mem_free(test_allocrator(), tem);
}

error_monitor_t test_error_monitor_em(test_error_monitor_t  tem) {
    return &tem->m_em;
}
