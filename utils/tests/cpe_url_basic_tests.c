#include "cmocka_all.h"
#include "cpe/utils/url.h"
#include "cpe_utils_tests.h"

static void cpe_url_basic(void **state) {
}

int cpe_url_basic_tests() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(cpe_url_basic),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

