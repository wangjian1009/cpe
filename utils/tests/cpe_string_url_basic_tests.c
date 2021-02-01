#include "cmocka_all.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_url.h"
#include "cpe/utils/stream_mem.h"
#include "cpe_utils_tests.h"
#include "cpe_utils_testenv.h"

static void cpe_string_url_encode_basic(void **state) {
    char output[128];
    
    const char * input = "开服最强战机自选大礼包";

    struct write_stream_mem ws = CPE_WRITE_STREAM_MEM_INITIALIZER(output, sizeof(output));
    
    assert_int_equal(
        99,
        cpe_url_encode_from_buf((write_stream_t)&ws, input, strlen(input)));
    
    assert_string_equal(
        "%E5%BC%80%E6%9C%8D%E6%9C%80%E5%BC%BA%E6%88%98%E6%9C%BA%E8%87%AA%E9%80%89%E5%A4%A7%E7%A4%BC%E5%8C%85",
        output);
}

int cpe_string_url_basic_tests() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(cpe_string_url_encode_basic),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

