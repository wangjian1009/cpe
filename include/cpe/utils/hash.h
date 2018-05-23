#ifndef CPE_UTILS_HASH_H
#define CPE_UTILS_HASH_H
#include "cpe/pal/pal_types.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*cpe_hash_fun_t)(const void * obj, void * user_data);
typedef int (*cpe_hash_eq_t)(const void * obj_l, const void * obj_r, void * user_data);
typedef void (*cpe_hash_destory_t)(void * obj, void * context);

typedef struct cpe_hash_entry {
    uint32_t m_value;
    struct cpe_hash_entry * m_next;
    struct cpe_hash_entry ** m_prev;
} cpe_hash_entry;

struct cpe_hash_bucket;

typedef struct cpe_hash_table {
    mem_allocrator_t m_alloc;
    cpe_hash_fun_t m_hash_fun;
    cpe_hash_eq_t  m_eq_fun;
    void * m_user_data;

     /*delta between object end entry*/
    int32_t m_obj2Entry;

    /*auto destory object in fini*/
    cpe_hash_destory_t m_destory_fun;
    void * m_destory_context;

    /*bulckets*/
    struct cpe_hash_bucket * m_buckets;
    uint32_t m_bucket_capacity;

    int32_t m_count;
} * cpe_hash_table_t;

typedef struct cpe_hash_it {
    cpe_hash_table_t m_hstable;
    struct cpe_hash_entry * m_entry;
} cpe_hash_it_t;

#define cpe_hash_entry_init(__entry) (__entry)->m_prev = NULL

int cpe_hash_table_init(
    cpe_hash_table_t hstable,
    mem_allocrator_t alloc,
    cpe_hash_fun_t hash_fun,
    cpe_hash_eq_t eq_fun,
    int32_t obj2Entry,
    int32_t suggestCapacity);

void * cpe_hash_table_user_data(cpe_hash_table_t hstable);
void cpe_hash_table_set_user_data(cpe_hash_table_t hstable, void * ctx);

void cpe_hash_table_set_destory_fun(
    cpe_hash_table_t hstable,
    cpe_hash_destory_t destory_fun,
    void * destory_context);

void cpe_hash_table_fini(cpe_hash_table_t hstable);

int cpe_hash_table_count(cpe_hash_table_t hstable);

int cpe_hash_table_insert_unique(cpe_hash_table_t hstable, void * obj);
int cpe_hash_table_insert(cpe_hash_table_t hstable, void * obj);

int cpe_hash_table_resize(cpe_hash_table_t hstable, int32_t suggestCapacity);
int cpe_hash_table_check_resize(cpe_hash_table_t hstable, int32_t suggestCapacity);

void * cpe_hash_table_find(cpe_hash_table_t hstable, const void * obj);
void * cpe_hash_table_find_next(cpe_hash_table_t hstable, const void * obj);
int cpe_hash_table_remove_by_ins(cpe_hash_table_t hstable, void * obj);
int cpe_hash_table_remove_by_key(cpe_hash_table_t hstable, const void * obj);
int cpe_hash_table_remove_all_by_key(cpe_hash_table_t hstable, const void * obj);

/*iterator operation*/
void cpe_hash_it_init(cpe_hash_it_t * it, cpe_hash_table_t hstable);
void * cpe_hash_it_next(cpe_hash_it_t * it);

uint32_t cpe_hash_str(const void * str, size_t len);
uint32_t cpe_hash_uint32(uint32_t h);
uint64_t cpe_hash_uint64(uint64_t h);
uint32_t cpe_hash_buffer(uint32_t seed, const void *src, size_t len);
uint32_t cpe_stable_hash_buffer(uint32_t seed, const void *src, size_t len);

#define cpe_hash_variable(seed, val) \
    (cpe_hash_buffer((seed), &(val), sizeof((val))))
#define cork_stable_hash_variable(seed, val) \
    (cork_stable_hash_buffer((seed), &(val), sizeof((val))))
#define cork_big_hash_variable(seed, val) \
    (cork_big_hash_buffer((seed), &(val), sizeof((val))))

#define CPE_HASH_UINT32(__h) do {               \
        __h ^= __h >> 16;                       \
        __h *= 0x85ebca6b;                      \
        __h ^= __h >> 13;                       \
        __h *= 0xc2b2ae35;                      \
        __h ^= __h >> 16;                       \
    } while(0)

#define CPE_HASH_UINT64(__k) do {               \
        __k ^= __k >> 33;                       \
        __k *= (uint64_t)(0xff51afd7ed558ccd);  \
        __k ^= __k >> 33;                       \
        __k *= (uint64_t)(0xc4ceb9fe1a85ec53);  \
        __k ^= __k >> 33;                       \
    } while(0)

#define CPE_HASH_OBJ2ENTRY(__typeName, __itemName)  (((char*)(&((struct __typeName *)1000)->__itemName)) - ((char*)1000))

#ifdef __cplusplus
}
#endif

#endif
