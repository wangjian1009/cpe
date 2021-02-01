#ifndef TEST_CPE_UTILS_TESTENV_H_INCLEDED
#define TEST_CPE_UTILS_TESTENV_H_INCLEDED
#include "test_memory.h"
#include "test_error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/utils_types.h"

typedef struct cpe_utils_testenv * cpe_utils_testenv_t;

struct cpe_utils_testenv {
    test_error_monitor_t m_tem;
    error_monitor_t m_em;
    struct mem_buffer m_tmp_buffer;
};

cpe_utils_testenv_t cpe_utils_testenv_create();
void cpe_utils_testenv_free(cpe_utils_testenv_t env);

#endif
