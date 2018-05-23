#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash_standalone.h"
#include "cpe/utils/hash_algo.h"
#include "cpe/utils/memory.h"

struct cpe_hash_table_standalone_entry_priv {
    struct cpe_hash_table_standalone_entry public;
    TAILQ_ENTRY(cpe_hash_table_standalone_entry_priv) insertion_order;
    TAILQ_ENTRY(cpe_hash_table_standalone_entry_priv) in_bucket;
};

typedef TAILQ_HEAD(cpe_hash_table_standalone_entry_priv_list, cpe_hash_table_standalone_entry_priv) * cpe_hash_table_standalone_entry_priv_list_t;

struct cpe_hash_table_standalone {
    mem_allocrator_t m_alloc;
    struct cpe_hash_table_standalone_entry_priv_list * bins;
    struct cpe_hash_table_standalone_entry_priv_list insertion_order;
    uint32_t bin_count;
    uint32_t bin_mask;
    uint32_t entry_count;
    void * user_data;
    void (*free_user_data)(void*);
    uint32_t (*hash)(void *, const void *);
    uint8_t (*equals)(void *_data, const void *, const void *);
    void (*free_key)(void*);
    void (*free_value)(void *);
};

static uint32_t cpe_hash_table_standalone__default_hash(void *user_data, const void *key) {
    return (uint32_t) (ptr_int_t)key;
}

static uint8_t cpe_hash_table_standalone__default_equals(void *user_data, const void *key1, const void *key2) {
    return key1 == key2;
}

#define CPE_HASH_TABLE_DEFAULT_INITIAL_SIZE  8
#define CPE_HASH_TABLE_MAX_DENSITY  5

static inline uint32_t cpe_hash_table_standalone_new_size(uint32_t desired_count) {
    uint32_t  v = desired_count;
    uint32_t  r = 1;
    while (v >>= 1) {
        r <<= 1;
    }
    if (r != desired_count) {
        r <<= 1;
    }
    return r;
}

#define bin_index(table, hash)  ((hash) & (table)->bin_mask)

static void cpe_hash_table_standalone_allocate_bins(cpe_hash_table_standalone_t table, uint32_t desired_count) {
    uint32_t  i;

    table->bin_count = cpe_hash_table_standalone_new_size(desired_count);
    table->bin_mask = table->bin_count - 1;

    uint32_t bins_size = table->bin_count * sizeof(struct cpe_hash_table_standalone_entry_priv_list);
    table->bins = mem_alloc(table->m_alloc, bins_size);
    bzero(table->bins, bins_size);
}

static struct cpe_hash_table_standalone_entry_priv *
cpe_hash_table_standalone_new_entry(cpe_hash_table_standalone_t table, uint32_t hash, void *key, void *value) {
    struct cpe_hash_table_standalone_entry_priv  *entry = mem_alloc(table->m_alloc, sizeof(struct cpe_hash_table_standalone_entry_priv));
    TAILQ_INSERT_TAIL(&table->insertion_order, entry, insertion_order);
    entry->public.hash = hash;
    entry->public.key = key;
    entry->public.value = value;
    return entry;
}

static void cpe_hash_table_standalone_free_entry(cpe_hash_table_standalone_t table, struct cpe_hash_table_standalone_entry_priv *entry) {
    if (table->free_key != NULL) {
        table->free_key(entry->public.key);
    }
    if (table->free_value != NULL) {
        table->free_value(entry->public.value);
    }

    TAILQ_REMOVE(&table->insertion_order, entry, insertion_order);

    mem_free(table->m_alloc, entry);
}

cpe_hash_table_standalone_t
cpe_hash_table_standalone_create(mem_allocrator_t alloc, uint32_t initial_size) {
    cpe_hash_table_standalone_t table = mem_alloc(alloc, sizeof(struct cpe_hash_table_standalone));
    table->m_alloc = alloc;
    table->entry_count = 0;
    table->user_data = NULL;
    table->free_user_data = NULL;
    table->hash = cpe_hash_table_standalone__default_hash;
    table->equals = cpe_hash_table_standalone__default_equals;
    table->free_key = NULL;
    table->free_value = NULL;
    TAILQ_INIT(&table->insertion_order);
    
    if (initial_size < CPE_HASH_TABLE_DEFAULT_INITIAL_SIZE) {
        initial_size = CPE_HASH_TABLE_DEFAULT_INITIAL_SIZE;
    }
    
    cpe_hash_table_standalone_allocate_bins(table, initial_size);
    return table;
}

void cpe_hash_table_standalone_clear(struct cpe_hash_table_standalone *table) {
    uint32_t  i;

    while(!TAILQ_EMPTY(&table->insertion_order)) {
        cpe_hash_table_standalone_free_entry(table, TAILQ_FIRST(&table->insertion_order));
    }

    for (i = 0; i < table->bin_count; i++) {
        TAILQ_INIT(&table->bins[i]);
    }

    table->entry_count = 0;
}

void cpe_hash_table_standalone_free(cpe_hash_table_standalone_t table) {
    cpe_hash_table_standalone_clear(table);

    if (table->bins) {
        mem_free(table->m_alloc, table->bins);
        table->bins = NULL;
    }

    if (table->free_user_data) {
        table->free_user_data(table->user_data);
    }
    
    mem_free(table->m_alloc, table);
}

uint32_t cpe_hash_table_standalone_size(cpe_hash_table_standalone_t table) {
    return table->entry_count;
}

void cpe_hash_table_standalone_set_user_data(cpe_hash_table_standalone_t table, void *user_data, void (*free_user_data)(void*)) {
    table->user_data = user_data;
    table->free_user_data = free_user_data;
}

void cpe_hash_table_standalone_set_hash(cpe_hash_table_standalone_t table, uint32_t (*hash)(void *, const void *)) {
    table->hash = hash;
}

void cpe_hash_table_standalone_set_equals(cpe_hash_table_standalone_t table, uint8_t (*equals)(void *_data, const void *, const void *)) {
    table->equals = equals;
}

void cpe_hash_table_standalone_set_free_key(cpe_hash_table_standalone_t table, void (*free)(void*)) {
    table->free_key = free;
}

void cpe_hash_table_standalone_set_free_value(cpe_hash_table_standalone_t table, void (*free)(void*)) {
    table->free_value = free;
}

void cpe_hash_table_standalone_ensure_size(struct cpe_hash_table_standalone *table, uint32_t desired_count) {
    if (desired_count > table->bin_count) {
        struct cpe_hash_table_standalone_entry_priv_list * old_bins = table->bins;
        uint32_t old_bin_count = table->bin_count;

        cpe_hash_table_standalone_allocate_bins(table, desired_count);

        if (old_bins != NULL) {
            for (uint32_t i = 0; i < old_bin_count; i++) {
                struct cpe_hash_table_standalone_entry_priv_list * old_bin = &old_bins[i];
                struct cpe_hash_table_standalone_entry_priv_list * new_bin = &table->bins[i];
                while(TAILQ_EMPTY(old_bin)) {
                    struct cpe_hash_table_standalone_entry_priv * entry = TAILQ_FIRST(old_bin);

                    TAILQ_REMOVE(old_bin, entry, in_bucket);
                    
                    uint32_t  bin_index = bin_index(table, entry->public.hash);

                    TAILQ_INSERT_TAIL(&new_bin[bin_index], entry, in_bucket);
                }
            }

            mem_free(table->m_alloc, old_bins);
        }
    }
}

static void cpe_hash_table_standalone_rehash(cpe_hash_table_standalone_t table) {
    cpe_hash_table_standalone_ensure_size(table, table->bin_count + 1);
}

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_entry_hash(cpe_hash_table_standalone_t table, uint32_t hash, const void *key) {
    if (table->bin_count == 0) return NULL;

    uint32_t bin_index = bin_index(table, hash);

    struct cpe_hash_table_standalone_entry_priv_list * bin = &table->bins[bin_index];
    struct cpe_hash_table_standalone_entry_priv * curr;
    TAILQ_FOREACH(curr, bin, in_bucket) {
        if (table->equals(table->user_data, key, curr->public.key)) {
            return &curr->public;
        }
    }

    return NULL;
}

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_entry(cpe_hash_table_standalone_t table, const void *key) {
    uint32_t hash = table->hash(table->user_data, key);
    return cpe_hash_table_standalone_get_entry_hash(table, hash, key);
}

void * cpe_hash_table_standalone_get_hash(cpe_hash_table_standalone_t table, uint32_t hash, const void *key) {
    cpe_hash_table_standalone_entry_t entry = cpe_hash_table_standalone_get_entry_hash(table, hash, key);
    return entry ? entry->value : NULL;
}

void * cpe_hash_table_standalone_get(cpe_hash_table_standalone_t table, const void *key) {
    cpe_hash_table_standalone_entry_t entry = cpe_hash_table_standalone_get_entry(table, key);
    return entry ? entry->value : NULL;
}

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_or_create_hash(cpe_hash_table_standalone_t table, uint32_t hash, void *key, uint8_t *is_new) {
    struct cpe_hash_table_standalone_entry_priv * entry;
    uint32_t  bin_index;

    if (table->bin_count > 0) {
        bin_index = bin_index(table, hash);
        
        struct cpe_hash_table_standalone_entry_priv_list * bin = &table->bins[bin_index];
        struct cpe_hash_table_standalone_entry_priv * curr;

        TAILQ_FOREACH(curr, bin, in_bucket) {
            if (table->equals(table->user_data, key, curr->public.key)) {
                *is_new = 0;
                return &curr->public;
            }
        }

        if ((table->entry_count / table->bin_count) > CPE_HASH_TABLE_MAX_DENSITY) {
            cpe_hash_table_standalone_rehash(table);
            bin_index = bin_index(table, hash);
        }
    }
    else {
        cpe_hash_table_standalone_rehash(table);
        bin_index = bin_index(table, hash);
    }

    entry = cpe_hash_table_standalone_new_entry(table, hash, key, NULL);

    TAILQ_INSERT_TAIL(&table->bins[bin_index], entry, in_bucket);

    table->entry_count++;
    *is_new = 1;
    return &entry->public;
}

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_get_or_create(cpe_hash_table_standalone_t table, void *key, uint8_t *is_new) {
    uint32_t  hash = table->hash(table->user_data, key);
    return cpe_hash_table_standalone_get_or_create_hash(table, hash, key, is_new);
}

void cpe_hash_table_standalone_put_hash(
    cpe_hash_table_standalone_t table,
    uint32_t hash, void *key, void *value,
    uint8_t *is_new, void **old_key, void **old_value)
{
    struct cpe_hash_table_standalone_entry_priv  *entry;
    uint32_t  bin_index;

    if (table->bin_count > 0) {
        bin_index = bin_index(table, hash);

        struct cpe_hash_table_standalone_entry_priv_list * bin = &table->bins[bin_index];
        struct cpe_hash_table_standalone_entry_priv * curr;

        TAILQ_FOREACH(curr, bin, in_bucket) {
            if (table->equals(table->user_data, key, curr->public.key)) {
                if (old_key) *old_key = curr->public.key;
                if (old_value) *old_value = curr->public.value;
                if (is_new) *is_new = 0;
                
                entry->public.key = key;
                entry->public.value = value;
                return;
            }
        }

        /* create a new entry */
        if ((table->entry_count / table->bin_count) > CPE_HASH_TABLE_MAX_DENSITY) {
            cpe_hash_table_standalone_rehash(table);
            bin_index = bin_index(table, hash);
        }
    }
    else {
        cpe_hash_table_standalone_rehash(table);
        bin_index = bin_index(table, hash);
    }

    entry = cpe_hash_table_standalone_new_entry(table, hash, key, value);

    TAILQ_INSERT_TAIL(&table->bins[bin_index], entry, in_bucket);

    table->entry_count++;

    if (old_key) *old_key = NULL;
    if (old_value) *old_value = NULL;
    if (is_new) *is_new = 1;
}

void cpe_hash_table_standalone_put(
    cpe_hash_table_standalone_t table,
    void *key, void *value,
    uint8_t *is_new, void **old_key, void **old_value)
{
    uint32_t  hash = table->hash(table->user_data, key);
    cpe_hash_table_standalone_put_hash(table, hash, key, value, is_new, old_key, old_value);
}


void cpe_hash_table_standalone_delete_entry(struct cpe_hash_table_standalone *table, struct cpe_hash_table_standalone_entry *ventry) {
    struct cpe_hash_table_standalone_entry_priv * entry = (struct cpe_hash_table_standalone_entry_priv *)ventry;
    uint32_t bin_index = bin_index(table, entry->public.hash);
    TAILQ_REMOVE(&table->bins[bin_index], entry, in_bucket);
    table->entry_count--;
    cpe_hash_table_standalone_free_entry(table, entry);
}

uint8_t cpe_hash_table_standalone_delete_hash(
    struct cpe_hash_table_standalone *table,
    uint32_t hash, const void *key, void **deleted_key, void **deleted_value)
{
    if (table->bin_count == 0) return 0;

    uint32_t bin_index = bin_index(table, hash);
    struct cpe_hash_table_standalone_entry_priv_list * bin = &table->bins[bin_index];
    struct cpe_hash_table_standalone_entry_priv * curr;
    TAILQ_FOREACH(curr, bin, in_bucket) {
        if (table->equals(table->user_data, key, curr->public.key)) {
            if (deleted_key) *deleted_key = curr->public.key;
            if (deleted_value) *deleted_value = curr->public.value;

            TAILQ_REMOVE(bin, curr, in_bucket);
            table->entry_count--;

            cpe_hash_table_standalone_free_entry(table, curr);
            return 1;
        }
    }

    return 0;
}

uint8_t cpe_hash_table_standalone_delete(cpe_hash_table_standalone_t table, const void *key, void **deleted_key, void **deleted_value) {
    uint32_t  hash = table->hash(table->user_data, key);
    return cpe_hash_table_standalone_delete_hash(table, hash, key, deleted_key, deleted_value);
}

void cpe_hash_table_standalone_map(cpe_hash_table_standalone_t table, void *user_data, cpe_hash_table_standalone_map_f map) {
    struct cpe_hash_table_standalone_entry_priv * curr;

    curr = TAILQ_FIRST(&table->insertion_order);
    while (curr != TAILQ_END(&table->insertion_order)) {
        struct cpe_hash_table_standalone_entry_priv * next = TAILQ_NEXT(curr, insertion_order);

        cpe_hash_table_standalone_map_result_t result = map(user_data, &curr->public);

        if (result == cpe_hash_table_map_abort) {
            return;
        }
        else if (result == cpe_hash_table_map_delete) {
            cpe_hash_table_standalone_delete_entry(table, &curr->public);
        }

        curr = next;
    }
}

void cpe_hash_table_standalone_iterator_init(cpe_hash_table_standalone_t table, cpe_hash_table_standalone_iterator_t iterator) {
    iterator->table = table;
    iterator->priv = TAILQ_FIRST(&table->insertion_order);
}

cpe_hash_table_standalone_entry_t
cpe_hash_table_standalone_iterator_next(cpe_hash_table_standalone_iterator_t iterator) {
    cpe_hash_table_standalone_t table = iterator->table;
    struct cpe_hash_table_standalone_entry_priv * curr = iterator->priv;

    if (curr == TAILQ_END(&table->insertion_order)) return NULL;

    iterator->priv = TAILQ_NEXT(curr, insertion_order);
    
    return &curr->public;
}

static uint32_t string_hash(void *user_data, const void *vk) {
    const char  *k = vk;
    uint32_t  len = strlen(k);
    return cpe_hash_buffer(0, k, len);
}

static uint8_t string_equals(void *user_data, const void *vk1, const void *vk2) {
    const char  *k1 = vk1;
    const char  *k2 = vk2;
    return strcmp(k1, k2) == 0;
}

cpe_hash_table_standalone_t cpe_string_hash_table_create(mem_allocrator_t alloc, uint32_t initial_size) {
    cpe_hash_table_standalone_t table = cpe_hash_table_standalone_create(alloc, initial_size);
    if (table) {
        cpe_hash_table_standalone_set_hash(table, string_hash);
        cpe_hash_table_standalone_set_equals(table, string_equals);
    }
    return table;
}

cpe_hash_table_standalone_t cpe_pointer_hash_table_create(mem_allocrator_t alloc, uint32_t initial_size) {
    return cpe_hash_table_standalone_create(alloc, initial_size);
}

