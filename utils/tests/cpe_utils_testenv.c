#include "cmocka_all.h"
#include "cpe_utils_testenv.h"

cpe_utils_testenv_t cpe_utils_testenv_create() {
    cpe_utils_testenv_t env = mem_alloc(test_allocrator(), sizeof(struct cpe_utils_testenv));
    env->m_tem = test_error_monitor_create();
    env->m_em = test_error_monitor_em(env->m_tem);
    mem_buffer_init(&env->m_tmp_buffer, test_allocrator());
    return env;
}

void cpe_utils_testenv_free(cpe_utils_testenv_t env) {
    mem_buffer_clear(&env->m_tmp_buffer);
    test_error_monitor_free(env->m_tem);
    mem_free(test_allocrator(), env);
}
