#include <stdlib.h>
#include "cpe/utils/memory.h"

void * cpe_mem_alloc(struct mem_allocrator * alloc, size_t size, const char* file, const int line) {
    if (alloc) {
        return alloc->m_alloc(size, alloc, file, line);
    }
    else {
        return malloc(size);
    }
}

void * cpe_mem_calloc(struct mem_allocrator * alloc, size_t size, const char* file, const int line) {
    if (alloc) {
        return alloc->m_calloc(size, alloc, file, line);
    }
    else {
        return calloc(size, 1);
    }
}

void cpe_mem_free(struct mem_allocrator * alloc, void * p, const char* file, const int line) {
    if (alloc) {
        alloc->m_free(p, alloc, file, line);
    }
    else {
        free(p);
    }
}

static void * mem_do_null_alloc(size_t size, struct mem_allocrator * allocrator, const char* file, const int line) {
    return NULL;
}

static void mem_do_null_free(void * p, struct mem_allocrator * allocrator, const char* file, const int line) {
}

struct mem_allocrator * mem_allocrator_null(void) {
    static struct mem_allocrator s_ins = {
        mem_do_null_alloc,
        mem_do_null_alloc,
        mem_do_null_free,
        NULL,
    };

    return &s_ins;
}
