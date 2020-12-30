#ifndef CPE_TEST_UTILS_ERROR_H
#define CPE_TEST_UTILS_ERROR_H
#include "cpe/utils/error.h"

typedef struct test_error_monitor * test_error_monitor_t;

test_error_monitor_t test_error_monitor_create();
void test_error_monitor_free(test_error_monitor_t  tem);

error_monitor_t test_error_monitor_em(test_error_monitor_t  tem);

#endif
