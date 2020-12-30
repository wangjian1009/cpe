#include <setjmp.h>
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_stdarg.h"
#include "cmocka.h"
#include "test_memory.h"

void * test_mem_alloc(size_t size, mem_allocrator_t allocrator, const char* file, const int line) {
    return _test_malloc(size, file, line);
}

void * test_mem_calloc(size_t size, mem_allocrator_t allocrator, const char* file, const int line) {
    return _test_calloc(1, size, file, line);
}

void test_mem_free(void * p, mem_allocrator_t allocrator, const char* file, const int line) {
    _test_free(p, file, line);
}

mem_allocrator_t test_allocrator() {
    static struct mem_allocrator s_ins = {
        test_mem_alloc,
        test_mem_calloc,
        test_mem_free,
        NULL,
    };

    return &s_ins;
}

