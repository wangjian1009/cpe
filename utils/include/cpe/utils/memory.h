#ifndef CPE_MEM_ALLOCRATOR_H
#define CPE_MEM_ALLOCRATOR_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*mem_allocrator_alloc_t)(size_t size, struct mem_allocrator * allocrator, const char* file, const int line);
typedef void (*mem_allocrator_free_t)(void * p, struct mem_allocrator * allocrator, const char* file, const int line);

struct mem_allocrator {
    mem_allocrator_alloc_t m_alloc;
    mem_allocrator_alloc_t m_calloc;
    mem_allocrator_free_t m_free;
    void * m_ctx;
};

void * cpe_mem_alloc(struct mem_allocrator * alloc, size_t size, const char* file, const int line);
void * cpe_mem_calloc(struct mem_allocrator * alloc, size_t size, const char* file, const int line);
void cpe_mem_free(struct mem_allocrator * alloc, void * p, const char* file, const int line);

struct mem_allocrator * mem_allocrator_null(void);

#define mem_alloc(__alloc, __size) cpe_mem_alloc((__alloc), (__size), __FILE__, __LINE__)
#define mem_calloc(__alloc, __size) cpe_mem_calloc((__alloc), (__size), __FILE__, __LINE__)
#define mem_free(__alloc, __p) cpe_mem_free((__alloc), (__p), __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif
