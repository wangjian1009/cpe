#ifndef CPE_UTILS_HASH_STANDALONE_H
#define CPE_UTILS_HASH_STANDALONE_H
#include "utils_types.h"

struct cpe_hash_table_standalone_entry {
    uint32_t hash;
    void * key;
    void * value;
};

cpe_hash_table_standalone_t cpe_hash_table_standalone_create(mem_allocrator_t alloc, uint32_t initial_size);
void cpe_hash_table_standalone_free(cpe_hash_table_standalone_t table);

void cpe_hash_table_standalone_set_user_data(cpe_hash_table_standalone_t table, void *user_data, void (*free_user_data)(void*));
void cpe_hash_table_standalone_set_equals(cpe_hash_table_standalone_t table, uint8_t (*equals)(void *_data, const void *, const void *));
void cpe_hash_table_standalone_set_free_key(cpe_hash_table_standalone_t table, void (*free)(void *));
void cpe_hash_table_standalone_set_free_value(cpe_hash_table_standalone_t table, void (*free)(void *));
void cpe_hash_table_standalone_set_hash(cpe_hash_table_standalone_t table, uint32_t (*hash)(void *, const void *));
void cpe_hash_table_standalone_clear(cpe_hash_table_standalone_t table);
void cpe_hash_table_standalone_ensure_size(cpe_hash_table_standalone_t table, uint32_t desired_count);

uint32_t cpe_hash_table_standalone_size(cpe_hash_table_standalone_t table);
void * cpe_hash_table_standalone_get(cpe_hash_table_standalone_t table, const void *key);
void * cpe_hash_table_standalone_get_hash(cpe_hash_table_standalone_t table, uint32_t hash, const void *key);

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_entry(
    cpe_hash_table_standalone_t table, const void *key);

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_entry_hash(
    cpe_hash_table_standalone_t table, uint32_t hash, const void *key);

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_or_create(
    cpe_hash_table_standalone_t table, void *key, uint8_t *is_new);

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_or_create_hash(
    cpe_hash_table_standalone_t table, uint32_t hash, void *key, uint8_t *is_new);

void cpe_hash_table_standalone_put(
    cpe_hash_table_standalone_t table, void *key, void *value, uint8_t *is_new, void **old_key, void **old_value);

void cpe_hash_table_standalone_put_hash(
    cpe_hash_table_standalone_t table,
    uint32_t hash, void *key, void *value,
    uint8_t *is_new, void **old_key, void **old_value);

void cpe_hash_table_standalone_delete_entry(
    cpe_hash_table_standalone_t table, struct cpe_hash_table_standalone_entry *entry);

uint8_t cpe_hash_table_standalone_delete(
    cpe_hash_table_standalone_t table, const void *key,
    void **deleted_key, void **deleted_value);

uint8_t cpe_hash_table_standalone_delete_hash(
    cpe_hash_table_standalone_t table, uint32_t hash, const void *key,
    void **deleted_key, void **deleted_value);

typedef enum cpe_hash_table_standalone_map_result {
    cpe_hash_table_map_abort = 0,
    cpe_hash_table_map_continue = 1,
    cpe_hash_table_map_delete = 2
} cpe_hash_table_standalone_map_result_t;

typedef cpe_hash_table_standalone_map_result_t (*cpe_hash_table_standalone_map_f)(void *user_data, cpe_hash_table_standalone_entry_t entry);

void cpe_hash_table_standalone_map(cpe_hash_table_standalone_t table, void *user_data, cpe_hash_table_standalone_map_f mapper);

struct cpe_hash_table_standalone_iterator {
    cpe_hash_table_standalone_t table;
    void * priv;
};

void cpe_hash_table_standalone_iterator_init(cpe_hash_table_standalone_t table, cpe_hash_table_standalone_iterator_t iterator);
cpe_hash_table_standalone_entry_t cpe_hash_table_standalone_iterator_next(cpe_hash_table_standalone_iterator_t iterator);

cpe_hash_table_standalone_t cpe_string_hash_table_create(mem_allocrator_t alloc, uint32_t initial_size);
cpe_hash_table_standalone_t cpe_pointer_hash_table_create(mem_allocrator_t alloc, uint32_t initial_size);

#endif
