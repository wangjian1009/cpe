#ifndef CPE_UTILS_ARRAY_H
#define CPE_UTILS_ARRAY_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_array_priv;

struct cpe_raw_array {
    mem_allocrator_t m_alloc;
    void  *items;
    size_t  size;
    struct cpe_array_priv  *priv;
};

void cpe_raw_array_init(cpe_raw_array_t array, mem_allocrator_t alloc, size_t element_size);
void cpe_raw_array_fini(cpe_raw_array_t array);
void cpe_raw_array_set_callback_data(cpe_raw_array_t array, void *user_data, void (*free_user_data)(mem_allocrator_t, void *));

void cpe_raw_array_set_init(cpe_raw_array_t array, void (*init)(void *user_data, void *value));
void cpe_raw_array_set_fini(cpe_raw_array_t array, void (*done)(void *user_data, void *value));
void cpe_raw_array_set_reuse(cpe_raw_array_t array, void (*reuse)(void *user_data, void *value));
void cpe_raw_array_set_remove(cpe_raw_array_t array, void (*remote)(void *user_data, void *value));

size_t cpe_raw_array_element_size(cpe_raw_array_t array);
void cpe_raw_array_clear(cpe_raw_array_t array);
void * cpe_raw_array_elements(cpe_raw_array_t array);
void * cpe_raw_array_at(cpe_raw_array_t array, size_t index);
size_t cpe_raw_array_size(cpe_raw_array_t array);
uint8_t cpe_raw_array_is_empty(cpe_raw_array_t array);
void cpe_raw_array_ensure_size(cpe_raw_array_t array, size_t count);
void * cpe_raw_array_append(cpe_raw_array_t array);
int cpe_raw_array_copy(
    cpe_raw_array_t dest, cpe_raw_array_t src,
    int (*copy)(void *user_data, void *dest, const void *src), void *user_data);

#define cpe_array(T) \
    struct { \
        mem_allocrator_t m_alloc; \
        T  *items; \
        size_t  size; \
        struct cpe_array_priv  *priv; \
    }

#define cpe_array_element_size(arr)  (sizeof((arr)->items[0]))
#define cpe_array_elements(arr)  ((arr)->items)
#define cpe_array_at(arr, i)     ((arr)->items[(i)])
#define cpe_array_size(arr)      ((arr)->size)
#define cpe_array_is_empty(arr)  ((arr)->size == 0)
#define cpe_array_to_raw(arr)    ((cpe_raw_array_t ) (void *) (arr))

#define cpe_array_init(arr, alloc)                                            \
    (cpe_raw_array_init(cpe_array_to_raw(arr), (alloc), cpe_array_element_size(arr)))
#define cpe_array_fini(arr) \
    (cpe_raw_array_fini(cpe_array_to_raw(arr)))

#define cpe_array_set_callback_data(arr, ud, fud) \
    (cpe_raw_array_set_callback_data(cpe_array_to_raw(arr), (ud), (fud)))
#define cpe_array_set_init(arr, i) \
    (cpe_raw_array_set_init(cpe_array_to_raw(arr), (i)))
#define cpe_array_set_fini(arr, d) \
    (cpe_raw_array_set_fini(cpe_array_to_raw(arr), (d)))
#define cpe_array_set_reuse(arr, r) \
    (cpe_raw_array_set_reuse(cpe_array_to_raw(arr), (r)))
#define cpe_array_set_remove(arr, r) \
    (cpe_raw_array_set_remove(cpe_array_to_raw(arr), (r)))

#define cpe_array_clear(arr) \
    (cpe_raw_array_clear(cpe_array_to_raw(arr)))
#define cpe_array_copy(d, s, c, ud) \
    (cpe_raw_array_copy(cpe_array_to_raw(d), cpe_array_to_raw(s), (c), (ud)))

#define cpe_array_ensure_size(arr, count) \
    (cpe_raw_array_ensure_size(cpe_array_to_raw(arr), (count)))

#define cpe_array_append(arr, element) \
    (cpe_raw_array_append(cpe_array_to_raw(arr)), \
     ((arr)->items[(arr)->size - 1] = (element), (void) 0))

#define cpe_array_append_get(arr) \
    (cpe_raw_array_append(cpe_array_to_raw(arr)), \
     &(arr)->items[(arr)->size - 1])


void cpe_raw_pointer_array_init(cpe_raw_array_t array, mem_allocrator_t alloc, void (*free)(mem_allocrator_t, void *));
#define cpe_pointer_array_init(arr, alloc, f)                    \
    (cpe_raw_pointer_array_init(cpe_array_to_raw(arr), (alloc), (f)))

struct cpe_string_array {
    mem_allocrator_t m_alloc;
    const char  **items;
    size_t  size;
    struct cpe_array_priv  *priv;
};

void cpe_string_array_init(cpe_string_array_t array, mem_allocrator_t alloc);
void cpe_string_array_append(cpe_string_array_t array, const char *str);
void cpe_string_array_copy(cpe_string_array_t dest, cpe_string_array_t src);

#ifdef __cplusplus
}
#endif

#endif
