#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/array.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/string_utils.h"

/*-----------------------------------------------------------------------
 * Resizable arrays
 */

struct cpe_array_priv {
    size_t  allocated_count;
    size_t  allocated_size;
    size_t  element_size;
    size_t  initialized_count;
    void  *user_data;
    void (*free_user_data)(mem_allocrator_t alloc, void *);
    void (*init)(void *user_data, void *value);
    void (*fini)(void *user_data, void *value);
    void (*reuse)(void *user_data, void *value);
    void (*remove)(void *user_data, void *value);
};

void cpe_raw_array_init(cpe_raw_array_t array, mem_allocrator_t alloc, size_t element_size) {
    array->m_alloc = alloc;
    array->items = NULL;
    array->size = 0;
    array->priv = mem_alloc(alloc, sizeof(struct cpe_array_priv));
    array->priv->allocated_count = 0;
    array->priv->allocated_size = 0;
    array->priv->element_size = element_size;
    array->priv->initialized_count = 0;
    array->priv->user_data = NULL;
    array->priv->free_user_data = NULL;
    array->priv->init = NULL;
    array->priv->fini = NULL;
    array->priv->reuse = NULL;
    array->priv->remove = NULL;
}

void cpe_raw_array_fini(cpe_raw_array_t array) {
    if (array->priv->fini != NULL) {
        size_t  i;
        char  *element = array->items;
        for (i = 0; i < array->priv->initialized_count; i++) {
            array->priv->fini(array->priv->user_data, element);
            element += array->priv->element_size;
        }
    }
    if (array->items != NULL) {
        mem_free(array->m_alloc, array->items);
    }

    if (array->priv->free_user_data) {
        array->priv->free_user_data(array->m_alloc, array->priv->user_data);
    }

    mem_free(array->m_alloc, array->priv);
}

void cpe_raw_array_set_callback_data(cpe_raw_array_t array, void *user_data, void (*free_user_data)(mem_allocrator_t, void *)) {
    array->priv->user_data = user_data;
    array->priv->free_user_data = free_user_data;
}

void cpe_raw_array_set_init(cpe_raw_array_t array, void (*init)(void *user_data, void *value)) {
    array->priv->init = init;
}

void cpe_raw_array_set_fini(cpe_raw_array_t array, void (*fini)(void *user_data, void *value)) {
    array->priv->fini = fini;
}

void cpe_raw_array_set_reuse(cpe_raw_array_t array, void (*reuse)(void *user_data, void *value)) {
    array->priv->reuse = reuse;
}

void cpe_raw_array_set_remove(cpe_raw_array_t array, void (*remove)(void *user_data, void *value)) {
    array->priv->remove = remove;
}

size_t cpe_raw_array_element_size(cpe_raw_array_t array) {
    return array->priv->element_size;
}

void cpe_raw_array_clear(cpe_raw_array_t array) {
    if (array->priv->remove != NULL) {
        size_t  i;
        char  *element = array->items;
        for (i = 0; i < array->priv->initialized_count; i++) {
            array->priv->remove(array->priv->user_data, element);
            element += array->priv->element_size;
        }
    }
    array->size = 0;
}

void * cpe_raw_array_elements(cpe_raw_array_t array) {
    return array->items;
}

void * cpe_raw_array_at(cpe_raw_array_t array, size_t index) {
    return ((char *) array->items) + (array->priv->element_size * index);
}

size_t cpe_raw_array_size(cpe_raw_array_t array) {
    return array->size;
}

uint8_t cpe_raw_array_is_empty(cpe_raw_array_t array) {
    return (array->size == 0);
}

void cpe_raw_array_ensure_size(cpe_raw_array_t array, size_t desired_count) {
    size_t  desired_size;

    desired_size = desired_count * array->priv->element_size;

    if (desired_size > array->priv->allocated_size) {
        size_t  new_count = array->priv->allocated_count * 2;
        size_t  new_size = array->priv->allocated_size * 2;
        if (desired_size > new_size) {
            new_count = desired_count;
            new_size = desired_size;
        }

        void * new_items = mem_alloc(array->m_alloc, new_size);
        if (array->priv->allocated_size) {
            memcpy(new_items, array->items, new_size);
        }

        if (array->items) {
            mem_free(array->m_alloc, array->items);
        }
        array->items = new_items;
        array->priv->allocated_count = new_count;
        array->priv->allocated_size = new_size;
    }
}

void * cpe_raw_array_append(cpe_raw_array_t array) {
    size_t  index;
    void  *element;
    index = array->size++;
    cpe_raw_array_ensure_size(array, array->size);
    element = cpe_raw_array_at(array, index);

    /* Call the init or reset callback, depending on whether this entry has been
     * initialized before. */

    /* Since we can currently only add elements by appending them one at a time,
     * then this entry is either already initialized, or is the first
     * uninitialized entry. */
    assert(index <= array->priv->initialized_count);

    if (index == array->priv->initialized_count) {
        /* This element has not been initialized yet. */
        array->priv->initialized_count++;
        if (array->priv->init != NULL) {
            array->priv->init(array->priv->user_data, element);
        }
    }
    else {
        /* This element has already been initialized. */
        if (array->priv->reuse != NULL) {
            array->priv->reuse(array->priv->user_data, element);
        }
    }

    return element;
}

int cpe_raw_array_copy(
    cpe_raw_array_t dest, cpe_raw_array_t src,
    int (*copy)(void *user_data, void *dest, const void *src), void *user_data)
{
    size_t  i;
    size_t  reuse_count;
    char  *dest_element;

    assert(dest->priv->element_size == src->priv->element_size);
    cpe_array_clear(dest);
    cpe_array_ensure_size(dest, src->size);

    /* Initialize enough elements to hold the contents of src */
    reuse_count = dest->priv->initialized_count;
    if (src->size < reuse_count) {
        reuse_count = src->size;
    }

    dest_element = dest->items;
    if (dest->priv->reuse != NULL) {
        for (i = 0; i < reuse_count; i++) {
            dest->priv->reuse(dest->priv->user_data, dest_element);
            dest_element += dest->priv->element_size;
        }
    }
    else {
        dest_element += reuse_count * dest->priv->element_size;
    }

    if (dest->priv->init != NULL) {
        for (i = reuse_count; i < src->size; i++) {
            dest->priv->init(dest->priv->user_data, dest_element);
            dest_element += dest->priv->element_size;
        }
    }

    if (src->size > dest->priv->initialized_count) {
        dest->priv->initialized_count = src->size;
    }

    /* If the caller provided a copy function, let it copy each element in turn.
     * Otherwise, bulk copy everything using memcpy. */
    if (copy == NULL) {
        memcpy(dest->items, src->items, src->size * dest->priv->element_size);
    }
    else {
        const char  *src_element = src->items;
        dest_element = dest->items;
        for (i = 0; i < src->size; i++) {
            if (copy(user_data, dest_element, src_element) != 0) {
                return -1;
            }
            
            dest_element += dest->priv->element_size;
            src_element += dest->priv->element_size;
        }
    }

    dest->size = src->size;
    return 0;
}

/*-----------------------------------------------------------------------
 * Pointer arrays
 */

struct cpe_pointer_array {
    mem_allocrator_t alloc;
    void (*free)(mem_allocrator_t alloc, void *);
};

static void pointer__init(void *user_data, void *vvalue) {
    void **value = vvalue;
    *value = NULL;
}

static void pointer__fini(void *user_data, void *vvalue) {
    struct cpe_pointer_array  *ptr_array = user_data;
    void **value = vvalue;
    if (*value != NULL) {
        ptr_array->free(ptr_array->alloc, *value);
    }
}

static void pointer__remove(void *user_data, void *vvalue) {
    struct cpe_pointer_array  *ptr_array = user_data;
    void **value = vvalue;
    if (*value != NULL) {
        ptr_array->free(ptr_array->alloc, *value);
    }
    *value = NULL;
}

static void pointer__free(mem_allocrator_t alloc, void *user_data) {
    struct cpe_pointer_array  *ptr_array = user_data;
    mem_free(alloc, ptr_array);
}

void cpe_raw_pointer_array_init(cpe_raw_array_t array, mem_allocrator_t alloc, void (*free_ptr)(mem_allocrator_t alloc, void*)) {
    struct cpe_pointer_array  *ptr_array = mem_alloc(array->m_alloc, sizeof(struct cpe_pointer_array));
    ptr_array->alloc = alloc;
    ptr_array->free = free_ptr;
    cpe_raw_array_init(array, alloc, sizeof(void *));
    cpe_array_set_callback_data(array, ptr_array, pointer__free);
    cpe_array_set_init(array, pointer__init);
    cpe_array_set_fini(array, pointer__fini);
    cpe_array_set_remove(array, pointer__remove);
}

/*-----------------------------------------------------------------------
 * String arrays
 */
static void cpe_string_array_string_free(mem_allocrator_t alloc, void * data) {
    mem_free(alloc, data);
}

void cpe_string_array_init(cpe_string_array_t array, mem_allocrator_t alloc) {
    cpe_raw_pointer_array_init((cpe_raw_array_t) array, alloc, cpe_string_array_string_free);
}

void cpe_string_array_append(cpe_string_array_t array, const char *str) {
    const char  *copy = cpe_str_mem_dup(array->m_alloc, str);
    cpe_array_append(array, copy);
}

static int string__copy(void *user_data, void *vdest, const void *vsrc) {
    mem_allocrator_t alloc = user_data;
    const char  **dest = vdest;
    const char  **src = (const char **) vsrc;
    *dest = cpe_str_mem_dup(alloc, *src);
    return 0;
}

void cpe_string_array_copy(cpe_string_array_t dest, cpe_string_array_t src) {
    int  rc = cpe_array_copy(dest, src, string__copy, dest->m_alloc);
    /* cpe_array_copy can only fail if the copy callback fails, and ours
     * doesn't! */
    assert(rc == 0);
}
