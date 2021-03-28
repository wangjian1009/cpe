#ifndef CPE_TEST_UTILS_CMOCHA_ALL_H
#define CPE_TEST_UTILS_CMOCHA_ALL_H
#include <setjmp.h>
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_stdarg.h"
#include "cmocka.h"

#define assert_string_match(regex, check) \
    _assert_string_match((const char*)(check), (const char*)(regex), __FILE__, __LINE__)

void _assert_string_match(
    const char * const check, const char * const regex,
    const char * const file, const int line);

#endif
