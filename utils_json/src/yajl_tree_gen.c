#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "yajl/yajl_tree.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "yajl_tree_gen.h"

typedef struct cpe_yajl_val * cpe_yajl_val_t;

struct cpe_yajl_val {
    uint16_t m_capacity;
    struct yajl_val_s m_data;
};

#define _to_i(__v) ((cpe_yajl_val_t)(((uint8_t *)(__v)) - CPE_ENTRY_START(cpe_yajl_val, m_data)))
#define _to_r(__v) (&((__v)->m_data))

static char * cpe_yajl_tree_gen_dup_str(cpe_yajl_tree_gen_t gen, const char * value) {
    char * v = mem_buffer_strdup(gen->m_buffer, value);
    if (v == NULL) {
        CPE_ERROR(gen->m_em, "yajl_tree_gen: dup string %s fail!", value);
        cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "out of memory");
        return NULL;
    }

    return v;
}

static cpe_yajl_val_t cpe_yajl_tree_gen_alloc(cpe_yajl_tree_gen_t gen, yajl_type type) {
    cpe_yajl_val_t v = mem_buffer_alloc(gen->m_buffer, sizeof(struct cpe_yajl_val));
    if (v == NULL) {
        CPE_ERROR(gen->m_em, "yajl_tree_gen: gen node fail!");
        cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "out of memory");
        return NULL;
    }

    v->m_capacity = 0;
    v->m_data.type = type;
    
    return v;
}

yajl_val cpe_yajl_tree_dup(cpe_yajl_tree_gen_t gen, yajl_val value) {
    if (value == NULL) return NULL;

    switch(value->type) {
    case yajl_t_string:
        return cpe_yajl_tree_gen_string(gen, yajl_get_string(value));
    case yajl_t_number: {
        cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_number);
        if (v == NULL) return NULL;

        v->m_data.u.number.i = value->u.number.i;
        v->m_data.u.number.d = value->u.number.d;
        v->m_data.u.number.flags = value->u.number.flags;
        v->m_data.u.number.r = cpe_yajl_tree_gen_dup_str(gen, value->u.number.r);
        if (v->m_data.u.number.r == NULL) return NULL;

        return _to_r(v);
    }
    case yajl_t_object: {
        yajl_val obj = cpe_yajl_tree_gen_object(gen, value->u.object.len);
        if (obj == NULL) return NULL;

        uint32_t i;
        for(i = 0; i < value->u.object.len; ++i) {
            yajl_val child_value = cpe_yajl_tree_dup(gen, value->u.object.values[i]);
            if (child_value) {
                cpe_yajl_tree_object_add(gen, obj, value->u.object.keys[i], child_value);
            }
        }

        return obj;
    }
    case yajl_t_array: {
        yajl_val array = cpe_yajl_tree_gen_array(gen, value->u.array.len);
        if (array == NULL) return NULL;

        uint32_t i;
        for(i = 0; i < value->u.array.len; ++i) {
            yajl_val child_value = cpe_yajl_tree_dup(gen, value->u.array.values[i]);
            if (child_value) {
                cpe_yajl_tree_array_add(gen, array, child_value);
            }
        }
        
        return array;
    }
    case yajl_t_true:
        return cpe_yajl_tree_gen_bool(gen, 1);
    case yajl_t_false:
        return cpe_yajl_tree_gen_bool(gen, 0);
    case yajl_t_null:
        return cpe_yajl_tree_gen_null(gen);
    default:
        return NULL;
    }
}

yajl_val cpe_yajl_tree_gen_object(cpe_yajl_tree_gen_t gen, uint32_t capacity) {
    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_object);
    if (v == NULL) return NULL;

    if (capacity == 0) capacity = 8;

    v->m_capacity = capacity;
    v->m_data.u.object.keys = mem_buffer_alloc(gen->m_buffer, sizeof(v->m_data.u.object.keys[0]) * capacity);
    v->m_data.u.object.values = mem_buffer_alloc(gen->m_buffer, sizeof(v->m_data.u.object.values[0]) * capacity);

    if (v->m_data.u.object.keys == NULL || v->m_data.u.object.values == NULL) {
        CPE_ERROR(gen->m_em, "yajl_tree_gen: gen object value buf fail, capacity=%d!", capacity);
        cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "out of memory");
        return NULL;
    }
    
    v->m_data.u.object.len = 0;
    
    return _to_r(v);
}

int cpe_yajl_tree_object_add_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, yajl_val value) {
    if (value == NULL) return 0;
    return cpe_yajl_tree_object_add(gen, o, key, value);
}

int cpe_yajl_tree_object_add(cpe_yajl_tree_gen_t gen, yajl_val i_o, const char * key, yajl_val i_value) {
    assert(i_o);
    assert(i_o->type == yajl_t_object);
    assert(key);

    if (i_value == NULL) {
        CPE_ERROR(gen->m_em, "yajl_tree_gen: %s append empty value!", key);
        cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "object append empty object");
        return -1;
    }
    
    cpe_yajl_val_t o = _to_i(i_o);
    cpe_yajl_val_t vaule = _to_i(i_value);

    if ((uint32_t)o->m_data.u.object.len >= o->m_capacity) {
        uint32_t new_capacity = o->m_capacity < 8 ? 8 : o->m_capacity * 2;
        const char ** new_keys = mem_buffer_alloc(gen->m_buffer, sizeof(o->m_data.u.object.keys[0]) * new_capacity);
        yajl_val * new_values = mem_buffer_alloc(gen->m_buffer, sizeof(o->m_data.u.object.values[0]) * new_capacity);

        if (new_keys == NULL || new_values == NULL) {
            CPE_ERROR(gen->m_em, "yajl_tree_gen: gen object value buf fail, capacity=%d!", new_capacity);
            cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "out of memory");
            return -1;
        }

        memcpy((void*)new_keys, o->m_data.u.object.keys, sizeof(o->m_data.u.object.keys[0]) * o->m_data.u.object.len);
        memcpy(new_values, o->m_data.u.object.values, sizeof(o->m_data.u.object.values[0]) * o->m_data.u.object.len);
        
        o->m_capacity = new_capacity;
        o->m_data.u.object.keys = new_keys;
        o->m_data.u.object.values = new_values;
    }

    o->m_data.u.object.keys[o->m_data.u.object.len] = cpe_yajl_tree_gen_dup_str(gen, key);
    if (o->m_data.u.object.keys[o->m_data.u.object.len] == NULL) return -1;

    o->m_data.u.object.values[o->m_data.u.object.len] = i_value;

    o->m_data.u.object.len++;
    
    return 0;
}

yajl_val cpe_yajl_tree_object_add_object(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, uint32_t capacity) {
    yajl_val v = cpe_yajl_tree_gen_object(gen, capacity);
    if (v == NULL) return NULL;

    if (cpe_yajl_tree_object_add(gen, o, key, v) != 0) return NULL;
    
    return v;
}

yajl_val cpe_yajl_tree_object_add_array(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, uint32_t capacity) {
    yajl_val v = cpe_yajl_tree_gen_array(gen, capacity);
    if (v == NULL) return NULL;

    if (cpe_yajl_tree_object_add(gen, o, key, v) != 0) return NULL;
    
    return v;
}

int cpe_yajl_tree_object_add_string(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, const char * value) {
    yajl_val v = cpe_yajl_tree_gen_string(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_object_add(gen, o, key, v);
}

int cpe_yajl_tree_object_add_string_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, const char * value) {
    if (value) {
        return cpe_yajl_tree_object_add_string(gen, o, key, value);
    }
    else {
        return cpe_yajl_tree_object_add_null(gen, o, key);
    }
}

int cpe_yajl_tree_object_add_integer(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, int64_t value) {
    yajl_val v = cpe_yajl_tree_gen_integer(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_object_add(gen, o, key, v);
}

int cpe_yajl_tree_object_add_double(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, double value) {
    yajl_val v = cpe_yajl_tree_gen_double(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_object_add(gen, o, key, v);
}

int cpe_yajl_tree_object_add_bool(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, uint8_t value) {
    yajl_val v = cpe_yajl_tree_gen_bool(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_object_add(gen, o, key, v);
}

int cpe_yajl_tree_object_add_null(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key) {
    yajl_val v = cpe_yajl_tree_gen_null(gen);
    if (v == NULL) return -1;
    return cpe_yajl_tree_object_add(gen, o, key, v);
}

yajl_val cpe_yajl_tree_gen_array(cpe_yajl_tree_gen_t gen, uint32_t capacity) {
    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_array);
    if (v == NULL) return NULL;

    if (capacity == 0) capacity = 8;

    v->m_capacity = capacity;
    v->m_data.u.array.values = mem_buffer_alloc(gen->m_buffer, sizeof(v->m_data.u.array.values[0]) * capacity);

    if (v->m_data.u.array.values == NULL) {
        CPE_ERROR(gen->m_em, "yajl_tree_gen: gen array value buf fail, capacity=%d!", capacity);
        cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "out of memory");
        return NULL;
    }
    
    v->m_data.u.array.len = 0;
    
    return _to_r(v);
}

int cpe_yajl_tree_array_add_opt(cpe_yajl_tree_gen_t gen, yajl_val o, yajl_val value) {
    if (value == NULL) return 0;
    return cpe_yajl_tree_array_add(gen, o, value);
}

int cpe_yajl_tree_array_add(cpe_yajl_tree_gen_t gen, yajl_val i_o, yajl_val i_value) {
    assert(i_o);
    assert(i_o->type == yajl_t_array);

    if (i_value == NULL) {
        CPE_ERROR(gen->m_em, "yajl_tree_gen: array append empty value!");
        cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "array append empty object");
        return -1;
    }

    cpe_yajl_val_t o = _to_i(i_o);
    cpe_yajl_val_t vaule = _to_i(i_value);

    if ((uint32_t)o->m_data.u.array.len >= o->m_capacity) {
        uint32_t new_capacity = o->m_capacity < 8 ? 8 : o->m_capacity * 2;
        yajl_val * new_values = mem_buffer_alloc(gen->m_buffer, sizeof(o->m_data.u.array.values[0]) * new_capacity);

        if (new_values == NULL) {
            CPE_ERROR(gen->m_em, "yajl_tree_gen: gen array value buf fail, capacity=%d!", new_capacity);
            cpe_str_dup(gen->m_error_msg, sizeof(gen->m_error_msg), "out of memory");
            return -1;
        }

        memcpy(new_values, o->m_data.u.array.values, sizeof(o->m_data.u.array.values[0]) * o->m_data.u.array.len);
        
        o->m_capacity = new_capacity;
        o->m_data.u.array.values = new_values;
    }

    o->m_data.u.array.values[o->m_data.u.array.len] = i_value;

    o->m_data.u.array.len++;
    
    return 0;
}

yajl_val cpe_yajl_tree_array_add_object(cpe_yajl_tree_gen_t gen, yajl_val o, uint32_t capacity) {
    yajl_val v = cpe_yajl_tree_gen_object(gen, capacity);
    if (v == NULL) return NULL;

    if (cpe_yajl_tree_array_add(gen, o, v) != 0) return NULL;
    
    return v;
}

yajl_val cpe_yajl_tree_array_add_array(cpe_yajl_tree_gen_t gen, yajl_val o, uint32_t capacity) {
    yajl_val v = cpe_yajl_tree_gen_array(gen, capacity);
    if (v == NULL) return NULL;

    if (cpe_yajl_tree_array_add(gen, o, v) != 0) return NULL;
    
    return v;
}

int cpe_yajl_tree_array_add_string(cpe_yajl_tree_gen_t gen, yajl_val o, const char * value) {
    yajl_val v = cpe_yajl_tree_gen_string(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_array_add(gen, o, v);
}

int cpe_yajl_tree_array_add_string_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * value) {
    if (value) {
        return cpe_yajl_tree_array_add_string(gen, o, value);
    }
    else {
        return cpe_yajl_tree_array_add_null(gen, o);
    }
}

int cpe_yajl_tree_array_add_integer(cpe_yajl_tree_gen_t gen, yajl_val o, int64_t value) {
    yajl_val v = cpe_yajl_tree_gen_integer(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_array_add(gen, o, v);
}

int cpe_yajl_tree_array_add_double(cpe_yajl_tree_gen_t gen, yajl_val o, double value) {
    yajl_val v = cpe_yajl_tree_gen_double(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_array_add(gen, o, v);
}

int cpe_yajl_tree_array_add_bool(cpe_yajl_tree_gen_t gen, yajl_val o, uint8_t value) {
    yajl_val v = cpe_yajl_tree_gen_bool(gen, value);
    if (v == NULL) return -1;
    return cpe_yajl_tree_array_add(gen, o, v);
}

int cpe_yajl_tree_array_add_null(cpe_yajl_tree_gen_t gen, yajl_val o) {
    yajl_val v = cpe_yajl_tree_gen_null(gen);
    if (v == NULL) return -1;
    return cpe_yajl_tree_array_add(gen, o, v);
}

yajl_val cpe_yajl_tree_gen_string(cpe_yajl_tree_gen_t gen, const char * value) {
    assert(value);

    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_string);
    if (v == NULL) return NULL;

    v->m_data.u.string = cpe_yajl_tree_gen_dup_str(gen, value);
    if (v->m_data.u.string == NULL) return NULL;

    return _to_r(v);
}

yajl_val cpe_yajl_tree_gen_null(cpe_yajl_tree_gen_t gen) {
    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_null);
    if (v == NULL) return NULL;
    return _to_r(v);
}

yajl_val cpe_yajl_tree_gen_integer(cpe_yajl_tree_gen_t gen, int64_t value) {
    char buf[32];
    snprintf(buf, sizeof(buf), FMT_INT64_T, value);

    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_number);
    if (v == NULL) return NULL;

    v->m_data.u.number.i = (long long)value;
    v->m_data.u.number.flags |= YAJL_NUMBER_INT_VALID;
    v->m_data.u.number.d = (double)value;
    v->m_data.u.number.flags |= YAJL_NUMBER_DOUBLE_VALID;
    v->m_data.u.number.r = cpe_yajl_tree_gen_dup_str(gen, buf);
    if (v->m_data.u.number.r == NULL) return NULL;

    return _to_r(v);
}

yajl_val cpe_yajl_tree_gen_double(cpe_yajl_tree_gen_t gen, double value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%f", value);

    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, yajl_t_number);
    if (v == NULL) return NULL;

    v->m_data.u.number.d = (double)value;
    v->m_data.u.number.flags |= YAJL_NUMBER_DOUBLE_VALID;
    v->m_data.u.number.r = cpe_yajl_tree_gen_dup_str(gen, buf);
    if (v->m_data.u.number.r == NULL) return NULL;

    return _to_r(v);
}

yajl_val cpe_yajl_tree_gen_bool(cpe_yajl_tree_gen_t gen, uint8_t value) {
    cpe_yajl_val_t v = cpe_yajl_tree_gen_alloc(gen, value ? yajl_t_true : yajl_t_false);
    if (v == NULL) return NULL;

    return _to_r(v);
}
