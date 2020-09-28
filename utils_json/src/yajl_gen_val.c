#include "cpe/pal/pal_string.h"
#include "yajl_gen_val.h"

yajl_gen_status yajl_gen_val(yajl_gen gen, yajl_val value) {
    uint8_t i;
    yajl_gen_status s;
    
    switch(value->type) {
    case yajl_t_string:
        return yajl_gen_string2(gen, value->u.string);
    case yajl_t_number:
        return yajl_gen_number(gen,value->u.number.r, strlen(value->u.number.r));
    case yajl_t_object:
        if ((s = yajl_gen_map_open(gen)) != yajl_gen_status_ok) return s;
        for(i = 0; i < value->u.object.len; i++) {
            if ((s = yajl_gen_string2(gen, value->u.object.keys[i])) != yajl_gen_status_ok) return s;
            if ((s = yajl_gen_val(gen, value->u.object.values[i])) != yajl_gen_status_ok) return s;
        }
        if ((s = yajl_gen_map_close(gen)) != yajl_gen_status_ok) return s;
        return yajl_gen_status_ok;
    case yajl_t_array:
        if ((s = yajl_gen_array_open(gen)) != yajl_gen_status_ok) return s;
        for(i = 0; i < value->u.array.len; i++) {
            if ((s = yajl_gen_val(gen, value->u.array.values[i])) != yajl_gen_status_ok) return s;
        }
        if ((s = yajl_gen_array_close(gen)) != yajl_gen_status_ok) return s;
        return yajl_gen_status_ok;
    case yajl_t_true:
        return yajl_gen_bool(gen, 1);
    case yajl_t_false:
        return yajl_gen_bool(gen, 0);
    case yajl_t_null:
    case yajl_t_any:
        return yajl_gen_null(gen);
    }
}


