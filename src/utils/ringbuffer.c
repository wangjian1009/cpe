#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream_buffer.h"

#define ALIGN(s) (((s) + 3 ) & ~3)

struct ringbuffer {
    error_monitor_t m_em;
    int size;
    int head;
};

INLINE int
block_offset(ringbuffer_t rb, ringbuffer_block_t blk) {
    char * start = (char *)(rb + 1);
    return (int)((char *)blk - start);
}

INLINE ringbuffer_block_t
block_ptr(ringbuffer_t rb, int offset) {
    char * start = (char *)(rb + 1);
    return (ringbuffer_block_t)(start + offset);
}

INLINE ringbuffer_block_t
block_next(ringbuffer_t rb, ringbuffer_block_t blk) {
    assert(ALIGN(blk->capacity) == blk->capacity);

    int head = block_offset(rb, blk);

    assert(head + blk->capacity <= rb->size);
    
    if (head + blk->capacity == rb->size) {
        return NULL;
    }
    else {
        return block_ptr(rb, head + blk->capacity);
    }
}

INLINE ringbuffer_block_t
link_follow_free_block(ringbuffer_t rb, ringbuffer_block_t blk) {
    ringbuffer_block_t next = NULL;
    for(next = block_next(rb, blk); next; next = block_next(rb, blk)) {
        /* CPE_ERROR(rb->m_em, "          blk(capacity=%d, length=%d, offset=%d, id=%d, next=%d)", */
        /*           blk->capacity, blk->length, blk->offset, blk->id, blk->next); */
        /* CPE_ERROR(rb->m_em, "               next(capacity=%d, length=%d, offset=%d, id=%d, next=%d)", */
        /*           blk->capacity, next->length, next->offset, next->id, next->next); */
                
        if (next->id != -1) {
            break;
        }

        assert(ALIGN(blk->capacity) == blk->capacity);
        assert(ALIGN(next->capacity) == next->capacity);
        
        blk->capacity += next->capacity;
    }
    return next;
}

ringbuffer_t
ringbuffer_new(int size, error_monitor_t em) {
    ringbuffer_block_t blk;
    ringbuffer_t rb;

    assert(size == ALIGN(size));

    rb = malloc(sizeof(*rb) + size);
    rb->m_em = em;
    rb->size = size;
    rb->head = 0;

    blk = block_ptr(rb, 0);
    blk->length = size;
    blk->capacity = size;
    blk->id = -1;
    return rb;
}

void
ringbuffer_delete(ringbuffer_t rb) {
    free(rb);
}

void
ringbuffer_link(ringbuffer_t rb , ringbuffer_block_t head, ringbuffer_block_t next) {
    assert(head);
    while (head->next >=0) {
        head = block_ptr(rb, head->next);
    }
    ringbuffer_block_set_id(rb, next, head->id);
    head->next = block_offset(rb, next);
}

ringbuffer_block_t
ringbuffer_unlink(ringbuffer_t rb , ringbuffer_block_t * head) {
    ringbuffer_block_t r = *head;

    if (r == NULL) return NULL;

    *head = r->next >= 0 ? block_ptr(rb, r->next) : NULL;
    r->next = -1;
    
    return r;
}

ringbuffer_block_t
ringbuffer_alloc(ringbuffer_t rb, int size) {
    ringbuffer_block_t blk;

TRY_AGAIN:

    blk = block_ptr(rb, rb->head);
    if (blk->id != -1) {
        if (rb->head == 0) {
            return NULL;
        }
        else { 
            rb->head = 0;
            CPE_ERROR(rb->m_em, "ringbuffer_block: move head to begin(1)");
            goto TRY_AGAIN;
        }
    }

    ringbuffer_block_t next = link_follow_free_block(rb, blk);
    blk->length = blk->capacity;
    
    if (size == 0) {
        rb->head = next ? block_offset(rb, next) : 0;
    }
    else {
        int blk_capacity = blk->capacity;
        int total_size = size + sizeof(struct ringbuffer_block);
        
        if (total_size > blk_capacity) {
            if (rb->head == 0) {
                return NULL;
            }
            else { 
                rb->head = 0;
                CPE_ERROR(rb->m_em, "ringbuffer_block: move head to begin(2)");
                goto TRY_AGAIN;
            }
        }
        else {
            blk->length = total_size;

            int blk_at_least_capacity = ALIGN(blk->length);
            
            int left_capacity = blk_capacity - blk_at_least_capacity;
            if (left_capacity > sizeof(struct ringbuffer_block)) {
                /*生成一个新的块 */
                blk->capacity = blk_at_least_capacity;

                ringbuffer_block_t new_next = block_next(rb, blk);
                new_next->capacity = left_capacity;
                new_next->length = left_capacity;
                new_next->offset = 0;
                new_next->id = -1;
                new_next->next = -1;
                rb->head = block_offset(rb, new_next);
            }
            else {
                rb->head = next ? block_offset(rb, next) : 0;
            }
        }
    }

    blk->next = -1;
    blk->offset = 0;
    
    /* CPE_ERROR( */
    /*     rb->m_em, "ringbuffer_alloc success: length=%d, capacity=%d, offset=%d, id=%d, next=%d, head=%d", */
    /*     blk->length, blk->capacity, blk->offset, blk->id, blk->next, rb->head); */
    
    return blk;
}

static int
_last_id(ringbuffer_t rb) {
    int i;
    for (i=0;i<2;i++) {
        ringbuffer_block_t blk = block_ptr(rb, rb->head);
        do {
            if (blk->length >= sizeof(struct ringbuffer_block) && blk->id >= 0)
                return blk->id;
            blk = block_next(rb, blk);
        } while(blk);
        rb->head = 0;
    }
    return -1;
}

int
ringbuffer_collect(ringbuffer_t rb) {
    int id = _last_id(rb);
    ringbuffer_block_t blk = block_ptr(rb, 0);

    if (id < 0) return -1;

    do {
        if (blk->length >= sizeof(struct ringbuffer_block) && blk->id == id) {
            blk->id = -1;
        }
        blk = block_next(rb, blk);
    } while(blk);
    return id;
}

void
ringbuffer_shrink(ringbuffer_t rb, ringbuffer_block_t blk, int size) {
    /* CPE_ERROR(rb->m_em, "ringbuffer_shrink: blk.start=%d, blk.capacity=%d, head=%d", */
    /*           block_offset(rb, blk), blk->capacity, rb->head); */

    assert(
        (rb->head == 0 && (block_offset(rb, blk) + blk->capacity == rb->size))
        || (block_offset(rb, blk) + blk->capacity == rb->head));
    
    if (size == 0) {
        rb->head = block_offset(rb, blk);
        CPE_ERROR(rb->m_em, "ringbuffer_shrink: return all blk, head=%d");
        return;
    }

    ringbuffer_block_t next = link_follow_free_block(rb, blk);
        
    int blk_capacity = blk->capacity;
    
    int total_size = size + sizeof(struct ringbuffer_block);
    assert(total_size <= blk->length);

    int blk_at_least_capacity = ALIGN(total_size);
    blk->length = total_size;
    
    int left_capacity = blk_capacity - blk_at_least_capacity;

    if (left_capacity > sizeof(struct ringbuffer_block)) {
        /*生成一个新的块 */
        blk->capacity = blk_at_least_capacity;

        ringbuffer_block_t new_next = block_next(rb, blk);
        new_next->capacity = left_capacity;
        new_next->length = left_capacity;
        new_next->offset = 0;
        new_next->id = -1;
        new_next->next = -1;
        rb->head = block_offset(rb, new_next);
    }
    else {
        rb->head = next ? block_offset(rb, next) : 0;
    }

    CPE_ERROR(rb->m_em, "ringbuffer_shrink success: head=%d", rb->head);
}

static int
_block_id(ringbuffer_block_t blk) {
    int id;

    assert(blk->length >= sizeof(struct ringbuffer_block));
    id = blk->id;
    //assert(id>=0);
    return id;
}

void
ringbuffer_free(ringbuffer_t rb, ringbuffer_block_t blk) {
    int id;

    if (blk == NULL)
        return;
    id = _block_id(blk);
    blk->id = -1;
    while (blk->next >= 0) {
        blk = block_ptr(rb, blk->next);
        assert(_block_id(blk) == id);
        blk->id = -1;
    }
}

int
ringbuffer_block_data(ringbuffer_t rb, ringbuffer_block_t blk, int skip, void **ptr) {
    int length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;

    if (length >= skip) {
        char * start = (char *)(blk + 1);
        *ptr = (start + blk->offset + skip);
        return length - skip;
    }
    else {
        *ptr = NULL;
        return -1;
    }
}

int ringbuffer_block_len(ringbuffer_t rb, ringbuffer_block_t blk, int skip) {
    return blk->length - sizeof(struct ringbuffer_block) - blk->offset - skip;
}

void ringbuffer_block_set_id(ringbuffer_t rb, ringbuffer_block_t blk, int id) {
    while(blk) {
        blk->id = id;
        blk = ringbuffer_block_link_next(rb, blk);
    }
}

int ringbuffer_block_total_len(ringbuffer_t rb, ringbuffer_block_t blk) {
    int length;
    length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;
    while (blk->next >= 0) {
        blk = block_ptr(rb, blk->next);
        assert(blk->offset == 0);
        length += blk->length - sizeof(struct ringbuffer_block);
    }

    return length;
}

ringbuffer_block_t ringbuffer_block_link_next(ringbuffer_t rb, ringbuffer_block_t blk) {
    return blk->next >= 0 ? block_ptr(rb, blk->next) : NULL;
}

int
ringbuffer_data(ringbuffer_t rb, ringbuffer_block_t blk, int size, int skip, void **ptr) {
    int length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;
    for (;;) {
        if (length > skip) {
            int ret;

            if (length - skip >= size) {
                char * start = (char *)(blk + 1);
                *ptr = (start + blk->offset + skip);
                return size;
            }
            *ptr = NULL;
            ret = length - skip;
            while (blk->next >= 0) {
                blk = block_ptr(rb, blk->next);
                ret += blk->length - sizeof(struct ringbuffer_block);
                if (ret >= size)
                    return ret;
            }
            return ret;
        }
        if (blk->next < 0) {
            assert(length == skip);
            *ptr = NULL;
            return 0;
        }
        blk = block_ptr(rb, blk->next);
        assert(blk->offset == 0);
        skip -= length;
        length = blk->length - sizeof(struct ringbuffer_block);
    }
}

void *
ringbuffer_copy(ringbuffer_t rb, ringbuffer_block_t from, int skip, ringbuffer_block_t to) {
    int size = to->length - sizeof(struct ringbuffer_block);
    int length = from->length - sizeof(struct ringbuffer_block) - from->offset;
    char * ptr = (char *)(to+1);
    for (;;) {
        if (length > skip) {
            char * src = (char *)(from + 1);
            src += from->offset + skip;
            length -= skip;
            while (length < size) {
                memcpy(ptr, src, length);
                ptr += length;
                size -= length;

                if (from->next >= 0) {
                    from = block_ptr(rb , from->next);
                    assert(from->offset == 0);
                    length = from->length - sizeof(struct ringbuffer_block);
                    src =  (char *)(from + 1);
                }
                else {
                    to->id = from->id;
                    return (char *)(to + 1);
                }
            }
            memcpy(ptr, src , size);
            to->id = from->id;
            return (char *)(to + 1);
        }

        if(from->next >= 0) {
            from = block_ptr(rb, from->next);
            assert(from->offset == 0);
            skip -= length;
            length = from->length - sizeof(struct ringbuffer_block);
        }
        else {
            to->id = from->id;
            return (char *)(to + 1);
        }
    }
}

ringbuffer_block_t
ringbuffer_yield(ringbuffer_t rb, ringbuffer_block_t blk, int skip) {
    int length = blk->length - sizeof(struct ringbuffer_block) - blk->offset;
    for (;;) {
        if (length > skip) {
            blk->offset += skip;
            return blk;
        }
        blk->id = -1;
        if (blk->next < 0) {
            return NULL;
        }
        blk = block_ptr(rb, blk->next);
        assert(blk->offset == 0);
        skip -= length;
        length = blk->length - sizeof(struct ringbuffer_block);
    }
}

void ringbuffer_dump_i(write_stream_t s, ringbuffer_t rb) {
    if (rb) {
        ringbuffer_block_t blk = block_ptr(rb,0);
        int i = 0;

        stream_printf(s, "ringbuffer: total-size=%d, head=%d\n", rb->size, rb->head);
        while (blk) {
            ++i;
            if (i > 1024) break;

            if (blk->length >= sizeof(*blk)) {
                stream_printf(s, "%d | id=%d, len=%d", block_offset(rb, blk), blk->id, (int)blk->length);
                if (blk->offset > 0) {
                    stream_printf(s, ", data-len=%d+%d", blk->offset, (int)(blk->length - sizeof(*blk) - blk->offset));
                }
                else {
                    stream_printf(s, ", data-len=%d", (int)(blk->length - sizeof(*blk)));
                }
                        
                if (blk->id >=0) {
                    stream_printf(s, ", next=%d", blk->next);
                }
            }
            else {
                stream_printf(s, "%d | ??? len=%d, ", block_offset(rb, blk), blk->length);
            }

            stream_printf(s, "\n");

            blk = block_next(rb, blk);
        }
    }
    else {
        stream_printf(s, "rb null");
    }
}

const char * ringbuffer_dump(mem_buffer_t buffer, ringbuffer_t rb) {
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    mem_buffer_clear_data(buffer);

    ringbuffer_dump_i((write_stream_t)&s, rb);

    mem_buffer_append_char(buffer, 0);

    return mem_buffer_make_continuous(buffer, 0);
}
