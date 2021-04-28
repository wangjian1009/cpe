#ifndef CPE_TEST_UTILS_CMOCHA_ALL_H
#define CPE_TEST_UTILS_CMOCHA_ALL_H
#include <setjmp.h>
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_stdarg.h"
#include "cpe/pal/pal_platform.h"
#include "cmocka.h"

#define assert_string_match(regex, check) \
    _assert_string_match((const char*)(check), (const char*)(regex), __FILE__, __LINE__)

void _assert_string_match(
    const char * const check, const char * const regex,
    const char * const file, const int line);

/*测试套定义宏 */
#define CPE_DECLARE_TEST_SUIT(__case_name) \
    extern const struct CMUnitTest __case_name ## _cases[]; \
    extern const uint16_t __case_name ## _count

#define CPE_BEGIN_TEST_SUIT(__case_name) \
    const struct CMUnitTest __case_name ## _cases[] = {

#define CPE_END_TEST_SUIT(__case_name) \
    }; \
    const uint16_t __case_name ## _count = CPE_ARRAY_SIZE(__case_name ## _cases);

/*测试套执行函数 */

struct _CPE_TEST_SUIT {
    const struct CMUnitTest * m_cases;
    uint16_t m_count;
};
int cpe_cmocak_run_test_suits(const struct _CPE_TEST_SUIT * test_groups, uint16_t test_group_count);

#define CPE_BEGIN_RUN_TESTS()                   \
    struct _CPE_TEST_SUIT _test_groups[] = {

#define CPE_ADD_TEST_SUIT(__case_name) { __case_name ## _cases, __case_name ## _count }

#define CPE_END_RUN_TESTS()                     \
    }; \
    return cpe_cmocak_run_test_suits(_test_groups, CPE_ARRAY_SIZE(_test_groups))

#endif
