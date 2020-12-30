#include "test_error.h"
#include "test_memory.h"

struct test_error_monitor {
    struct error_monitor m_em;
};

test_error_monitor_t test_error_monitor_create() {
    test_error_monitor_t tem = mem_alloc(test_allocrator(), sizeof(struct test_error_monitor));
    cpe_error_monitor_init(
        &tem->m_em, cpe_error_log_to_consol_and_flush,
        NULL);
    return tem;
}

void test_error_monitor_free(test_error_monitor_t  tem) {
    mem_free(test_allocrator(), tem);
}

error_monitor_t test_error_monitor_em(test_error_monitor_t  tem) {
    return &tem->m_em;
}
