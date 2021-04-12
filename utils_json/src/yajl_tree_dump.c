#include "yajl/yajl_gen.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/string_utils.h"
#include "yajl_tree_dump.h"
#include "yajl_gen_val.h"

void yajl_tree_print(write_stream_t ws, yajl_val data) {
    yajl_gen gen = yajl_gen_alloc(NULL);
    if (gen == NULL) return;

    yajl_gen_config(gen, yajl_gen_beautify, 0);
    yajl_gen_config(gen, yajl_gen_validate_utf8, 1);
    yajl_gen_config(gen, yajl_gen_print_callback, cpe_yajl_gen_print_to_stream, ws);
    yajl_gen_val(gen, data);
    yajl_gen_free(gen);
}

const char * yajl_tree_dump(mem_buffer_t buff, yajl_val data) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buff);

    mem_buffer_clear_data(buff);
    yajl_tree_print((write_stream_t)&stream, data);
    stream_putc((write_stream_t)&stream, 0);
    
    return mem_buffer_make_continuous(buff, 0);
}

const char * cpe_yajl_tree_type_str(int type) {
    switch(type) {
    case yajl_t_string:
        return "string";
    case yajl_t_number:
        return "number";
    case yajl_t_object:
        return "object";
    case yajl_t_array:
        return "array";
    case yajl_t_true:
        return "true";
    case yajl_t_false:
        return "false";
    case yajl_t_null:
        return "null";
    default:
        return "unknown";
    }
}
