#include "cmocka_all.h"
#include "cpe/utils/url.h"
#include "cpe_utils_tests.h"
#include "cpe_utils_testenv.h"

static int setup(void **state) {
    cpe_utils_testenv_t env = cpe_utils_testenv_create();
    *state = env;
    return 0;
}

static int teardown(void **state) {
    cpe_utils_testenv_t env = *state;
    cpe_utils_testenv_free(env);
    return 0;
}

static void cpe_url_assert_reparse(cpe_utils_testenv_t env, const char * str_url) {
    cpe_url_t url = cpe_url_parse(test_allocrator(), env->m_em, str_url);
    assert_true(url);

    assert_string_equal(cpe_url_dump(&env->m_tmp_buffer, url, cpe_url_print_full), str_url);

    cpe_url_free(url);
}

static void cpe_url_basic(void **state) {
    cpe_utils_testenv_t env = *state;
    cpe_url_assert_reparse(env, "http://1.2.3.4:5678/a/b?arg1=v1&arg2=v2");
}

int cpe_url_basic_tests() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(cpe_url_basic, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

