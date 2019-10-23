#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream.h"
#include "yajl_utils_types.h"

void cpe_yajl_stream_escape_str_range(write_stream_t ws, const char * msg, const char * msg_end) {
    const char * block_begin = msg;
    const char * block_end = msg;
    
    while(block_end < msg_end) {
        switch(*block_end) {
        case '"':
            if (block_begin < block_end) {
                stream_write(ws, block_begin, block_end - block_begin);
            }
            stream_putc(ws, '\\');
            stream_putc(ws, *block_end);
            block_end++;
            block_begin = block_end;
            break;
        default:
            block_end++;
            break;
        }
    }
    
    if (block_begin < block_end) {
        stream_write(ws, block_begin, block_end - block_begin);
    }
}

void cpe_yajl_stream_escape_str(write_stream_t ws, const char * msg) {
    cpe_yajl_stream_escape_str_range(ws, msg, msg + strlen(msg));
}

void cpe_yajl_gen_print_to_stream(void * ctx, const char * str, size_t len) {
    stream_write((write_stream_t)ctx, str, len);
}
