#include "yajl/yajl_gen.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "yajl_tree_dump.h"

static void yajl_tree_print_i(yajl_gen gen, yajl_val data) {
    switch(data->type) {
    case yajl_t_string:
        yajl_gen_string2(gen, yajl_get_string(data));
        break;
    case yajl_t_number:
        yajl_gen_number(gen, data->u.number.r, strlen(data->u.number.r));
        break;
    case yajl_t_object: {
        uint32_t i;
        yajl_gen_map_open(gen);
        for(i = 0; i < data->u.object.len; i++) {
            yajl_gen_string2(gen, data->u.object.keys[i]);
            yajl_tree_print_i(gen, data->u.object.values[i]);
        }
        yajl_gen_map_close(gen);
        break;
    }
    case yajl_t_array: {
        uint32_t i;
        yajl_gen_array_open(gen);
        for(i = 0; i < data->u.array.len; i++) {
            yajl_tree_print_i(gen, data->u.array.values[i]);
        }
        yajl_gen_array_close(gen);
        break;
    }
    case yajl_t_true:
        yajl_gen_bool(gen, 1);
        break;
    case yajl_t_false:
        yajl_gen_bool(gen, 0);
        break;
    case yajl_t_null:
        yajl_gen_null(gen);
        break;
    default:
        break;
    }
}

void yajl_tree_print(write_stream_t ws, yajl_val data) {
    yajl_gen gen = yajl_gen_alloc(NULL);
    if (gen == NULL) return;

    yajl_gen_config(gen, yajl_gen_beautify, 0);
    yajl_gen_config(gen, yajl_gen_validate_utf8, 1);
    yajl_gen_config(gen, yajl_gen_print_callback, cpe_yajl_gen_print_to_stream, ws);
    yajl_tree_print_i(gen, data);
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
