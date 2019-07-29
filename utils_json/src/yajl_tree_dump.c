#include "cpe/utils/stream_buffer.h"
#include "yajl_tree_dump.h"

void yajl_tree_print(write_stream_t ws, yajl_val data) {
    switch(data->type) {
    case yajl_t_string:
        stream_printf(ws, "\"%s\"", yajl_get_string(data));
        break;
    case yajl_t_number:
        stream_printf(ws, "\"%s\"", data->u.number.r);
        break;
    case yajl_t_object: {
        size_t i;
        stream_printf(ws, "{");
        for(i = 0; i < data->u.object.len; i++) {
            if (i > 0) stream_printf(ws, ",");
            stream_printf(ws, "\"%s\":", data->u.object.keys[i]);
            yajl_tree_print(ws, data->u.object.values[i]);
        }
        stream_printf(ws, "}");
        break;
    }
    case yajl_t_array: {
        size_t i;
        stream_printf(ws, "[");
        for(i = 0; i < data->u.array.len; i++) {
            if (i > 0) stream_printf(ws, ",");
            yajl_tree_print(ws, data->u.array.values[i]);
        }
        stream_printf(ws, "]");
        break;
    }
    case yajl_t_true:
        stream_printf(ws, "true");
        break;
    case yajl_t_false:
        stream_printf(ws, "false");
        break;
    case yajl_t_null:
        stream_printf(ws, "null");
        break;
    default:
        break;
    }
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
