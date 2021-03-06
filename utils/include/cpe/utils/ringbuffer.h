#ifndef CPE_RINGBUFFER_H
#define CPE_RINGBUFFER_H
#include "stream.h"
#include "buffer.h"

/* 
  这段代码是从云凤的代码中拿来的,但是为了和这里的代码风格一致，做了部分修改。
  原始代码参见https://github.com/cloudwu/mread
 */

struct ringbuffer_block {
	int capacity;
	int length;
	int offset;
	int id;
    int prev;
	int next;
};

ringbuffer_t ringbuffer_new(int size, error_monitor_t em);
void ringbuffer_delete(ringbuffer_t rb);

ringbuffer_block_t ringbuffer_alloc(ringbuffer_t rb, int size);
void ringbuffer_link(ringbuffer_t rb , ringbuffer_block_t prev, ringbuffer_block_t next);
ringbuffer_block_t ringbuffer_unlink(ringbuffer_t rb , ringbuffer_block_t * head);

typedef int (*ringbuffer_move_block_fun_t)(void * ctx, ringbuffer_t rb, ringbuffer_block_t old_block, ringbuffer_block_t new_block);
int ringbuffer_gc(ringbuffer_t rb, void * move_block_ctx, ringbuffer_move_block_fun_t move_block_fun);

int ringbuffer_collect(ringbuffer_t rb);
void ringbuffer_shrink(ringbuffer_t rb, ringbuffer_block_t blk, int size);
void ringbuffer_free(ringbuffer_t rb, ringbuffer_block_t blk);
int ringbuffer_block_data(ringbuffer_t rb, ringbuffer_block_t blk, int skip, void **ptr);
int ringbuffer_block_len(ringbuffer_t rb, ringbuffer_block_t blk, int skip);
ringbuffer_block_t ringbuffer_block_link_next(ringbuffer_t rb, ringbuffer_block_t blk);
void ringbuffer_block_set_id(ringbuffer_t rb, ringbuffer_block_t blk, int id);
int ringbuffer_block_total_len(ringbuffer_t rb, ringbuffer_block_t blk);
int ringbuffer_data(ringbuffer_t rb, ringbuffer_block_t blk, int size, int skip, void **ptr);
void * ringbuffer_copy(ringbuffer_t rb, ringbuffer_block_t from, int skip, ringbuffer_block_t to);
ringbuffer_block_t ringbuffer_yield(ringbuffer_t rb, ringbuffer_block_t blk, int skip);

const char * ringbuffer_dump_to_bufffer(mem_buffer_t buff, ringbuffer_t rb);
void ringbuffer_dump(ringbuffer_t rb);

#endif

