#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_token.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_data_entry.h"
#include "../dr_internal_types.h"

xtoken_t dr_create_token_from_ctype(xcomputer_t computer, uint8_t type, void * data) {
    xtoken_t r;

    r = xcomputer_alloc_token(computer);
    if (r == NULL) return NULL;
    
    switch(type) {
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_INT8:
        xcomputer_set_token_int(computer, r, (int64_t)*(int8_t*)data);
        break;
    case CPE_DR_TYPE_UCHAR:
    case CPE_DR_TYPE_UINT8:
        xcomputer_set_token_int(computer, r, (int64_t)*(uint8_t*)data);
        break;
    case CPE_DR_TYPE_INT16:
        xcomputer_set_token_int(computer, r, (int64_t)*(int16_t*)data);
        break;
    case CPE_DR_TYPE_UINT16:
        xcomputer_set_token_int(computer, r, (int64_t)*(uint16_t*)data);
        break;
    case CPE_DR_TYPE_INT32:
        xcomputer_set_token_int(computer, r, (int64_t)*(int32_t*)data);
        break;
    case CPE_DR_TYPE_UINT32:
        xcomputer_set_token_int(computer, r, (int64_t)*(uint32_t*)data);
        break;
    case CPE_DR_TYPE_INT64:
        xcomputer_set_token_int(computer, r, (int64_t)*(int64_t*)data);
        break;
    case CPE_DR_TYPE_UINT64:
        xcomputer_set_token_int(computer, r, (int64_t)*(uint64_t*)data);
        break;
    case CPE_DR_TYPE_FLOAT:
        xcomputer_set_token_float(computer, r, (double)*(float*)data);
        break;
    case CPE_DR_TYPE_DOUBLE:
        xcomputer_set_token_float(computer, r, *(double*)data);
        break;
    case CPE_DR_TYPE_STRING:
        if (xcomputer_set_token_str(computer, r, (const char *)data) != 0) {
            xcomputer_free_token(computer, r);
            return NULL;
        }
        break;
    default:
        xcomputer_free_token(computer, r);
        return NULL;
    }

    return r;
}

xtoken_t dr_create_token_from_entry(xcomputer_t computer, dr_data_entry_t entry) {
    return dr_create_token_from_ctype(computer, entry->m_entry->m_type, entry->m_data);
}

xtoken_t dr_create_token_from_value(xcomputer_t computer, dr_value_t value) {
    return dr_create_token_from_ctype(computer, value->m_type, value->m_data);
}

static xtoken_t dr_calc_find_value_in_source(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em) {
    dr_data_source_t data_source = input_ctx;
    struct dr_data_entry from_attr_buf;
    dr_data_entry_t from_attr;

    from_attr = dr_data_entry_search_in_source(&from_attr_buf, data_source, attr_name);
    
    return from_attr ? dr_create_token_from_entry(computer, from_attr) : NULL;
}

uint8_t dr_calc_bool_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int8_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    uint8_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_bool(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int8_t dr_calc_int8_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int8_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint8_t dr_calc_uint8_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint8_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint8_t)r;
}

int16_t dr_calc_int16_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int16_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint16_t dr_calc_uint16_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint16_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint16_t)r;
}

int32_t dr_calc_int32_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int32_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint32_t dr_calc_uint32_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint32_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int64_t dr_calc_int64_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, int64_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint64_t dr_calc_uint64_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, uint64_t dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    uint64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

float dr_calc_float_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, float dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (float)r;
}

double dr_calc_double_with_dft(xcomputer_t computer, const char * def, dr_data_source_t data_source, double dft) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

const char * dr_calc_str_with_dft(
    mem_buffer_t buffer, xcomputer_t computer, const char * def, dr_data_source_t data_source, const char * dft)
{
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    switch(xtoken_data_type(value)) {
    case xtoken_data_none:
        xcomputer_free_token(computer, value);
        return dft;
    case xtoken_data_int: {
        char buf[23];
        char * r;
        int64_t rv;

        if (xtoken_try_to_int64(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return dft;
        }

        snprintf(buf, sizeof(buf), FMT_INT64_T, rv);
        mem_buffer_clear_data(buffer);
        r = mem_buffer_strdup(buffer, buf);

        xcomputer_free_token(computer, value);
        return r ? r : dft;
    }
    case xtoken_data_double: {
        char buf[23];
        char * r;
        double rv;

        if (xtoken_try_to_double(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return dft;
        }

        snprintf(buf, sizeof(buf), "%f", rv);
        mem_buffer_clear_data(buffer);
        r = mem_buffer_strdup(buffer, buf);
        xcomputer_free_token(computer, value);
        return r ? r : dft;
    }
    case xtoken_data_str: {
        const char * r = xtoken_try_to_str(value);
        if (r) {
            r = mem_buffer_strdup(buffer, r);
        }
        xcomputer_free_token(computer, value);
        return r ? r : dft;
    }
    default:
        xcomputer_free_token(computer, value);
        return dft; 
    }
}

int dr_try_calc_bool(uint8_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_bool(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_int8(int8_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;
    int32_t rv;
    
    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_uint8(uint8_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;
    uint32_t rv;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_int16(int16_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;
    int32_t rv;
    
    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_uint16(uint16_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;
    uint32_t rv;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_int32(int32_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_uint32(uint32_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_int64(int64_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_uint64(uint64_t * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_float(float * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_float(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int dr_try_calc_double(double * result, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em) {
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_double(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

const char * dr_try_calc_str(
    mem_buffer_t buffer, xcomputer_t computer, const char * def, dr_data_source_t data_source, error_monitor_t em)
{
    struct xcomputer_args calc_args = { data_source, dr_calc_find_value_in_source };
    xtoken_t value;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return NULL;

    switch(xtoken_data_type(value)) {
    case xtoken_data_none:
        xcomputer_free_token(computer, value);
        return NULL;
    case xtoken_data_int: {
        char buf[23];
        int64_t rv;

        if (xtoken_try_to_int64(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return NULL;
        }

        xcomputer_free_token(computer, value);

        snprintf(buf, sizeof(buf), FMT_INT64_T, rv);
        mem_buffer_clear_data(buffer);
        return mem_buffer_strdup(buffer, buf);
    }
    case xtoken_data_double: {
        char buf[23];
        double rv;

        if (xtoken_try_to_double(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return NULL;
        }

        xcomputer_free_token(computer, value);

        snprintf(buf, sizeof(buf), "%f", rv);
        mem_buffer_clear_data(buffer);
        return mem_buffer_strdup(buffer, buf);
    }
    case xtoken_data_str: {
        const char * r = xtoken_try_to_str(value);
        if (r) {
            r = mem_buffer_strdup(buffer, r);
        }
        xcomputer_free_token(computer, value);
        return r;
    }
    default:
        xcomputer_free_token(computer, value);
        return NULL; 
    }
}

int dr_entry_set_from_token(void * data, LPDRMETAENTRY entry, xtoken_t token, error_monitor_t em) {
    switch(xtoken_data_type(token)) {
    case xtoken_data_int: {
        int64_t v;
        if (xtoken_try_to_int64(token, &v) != 0) {
            CPE_ERROR(
                em, "%s.%s: set from token: read token int64 fail!",
                dr_meta_name(dr_entry_self_meta(entry)), dr_entry_name(entry), dr_entry_type_name(entry));
            return -1;
        }

        if (dr_entry_set_from_int64(data, v, entry, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: set from token: set by int64 " FMT_INT64_T " fail!",
                dr_meta_name(dr_entry_self_meta(entry)), dr_entry_name(entry), dr_entry_type_name(entry), v);
            return -1;
        }

        return 0;
    }
    case xtoken_data_double: {
        double v;
        if (xtoken_try_to_double(token, &v) != 0) {
            CPE_ERROR(
                em, "%s.%s: set from token: read token double fail!",
                dr_meta_name(dr_entry_self_meta(entry)), dr_entry_name(entry), dr_entry_type_name(entry));
            return -1;
        }

        if (dr_entry_set_from_double(data, v, entry, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: set from token: set by double %f fail!",
                dr_meta_name(dr_entry_self_meta(entry)), dr_entry_name(entry), dr_entry_type_name(entry), (float)v);
            return -1;
        }

        return 0;
    }
    case xtoken_data_str:
        assert(xtoken_try_to_str(token));
        if (dr_entry_set_from_string(data, xtoken_try_to_str(token), entry, em) != 0) {
            CPE_ERROR(
                em, "%s.%s: set from token: set by string %s fail!",
                dr_meta_name(dr_entry_self_meta(entry)), dr_entry_name(entry), dr_entry_type_name(entry), xtoken_try_to_str(token));
            return -1;
        }
        return 0;
    default:
        CPE_ERROR(
            em, "%s.%s: set from token: not support type %d!",
            dr_meta_name(dr_entry_self_meta(entry)), dr_entry_name(entry), dr_entry_type_name(entry), xtoken_data_type(token));
        return -1;
    }
}

int dr_meta_setup(
    void * data, size_t size, LPDRMETA meta,
    xcomputer_t computer, char * defs, dr_data_source_t ds, error_monitor_t em)
{
    char * key;
    char * value;
    LPDRMETAENTRY entry;
    int offset;
    struct xcomputer_args calc_args = { ds, dr_calc_find_value_in_source };
    xtoken_t token;

    key = (char*)cpe_str_trim_head(defs);
    
    value = strchr(key, '=');
    if (value == NULL) {
        CPE_ERROR(em, "dr_meta_setup: %s: parse defs %s fail!", dr_meta_name(meta), defs);
        return -1;
    }

    *(char*)cpe_str_trim_tail(value, key) = 0;

    value = (char*)cpe_str_trim_head(value + 1);

    entry = dr_meta_find_entry_by_path_ex(meta, key, &offset);
    if (entry == NULL) {
        CPE_ERROR(em, "dr_meta_setup: %s: no entry %s!", dr_meta_name(meta), key);
        return -1;
    }

    token = xcomputer_compute(computer, value, &calc_args);
    if (token == NULL) {
        CPE_ERROR(em, "dr_meta_setup: %s: entry %s calc from %s fail!", dr_meta_name(meta), key, value);
        return -1;
    }

    if (dr_entry_set_from_token(((char*)data) + offset, entry, token, em) != 0) {
        CPE_ERROR(em, "dr_meta_setup: %s: entry %s calc from %s fail!", dr_meta_name(meta), key, value);
        xcomputer_free_token(computer, token);
        return -1;
    }

    return 0;
}

int dr_meta_bulk_setup(
    void * data, size_t size, LPDRMETA meta,
    xcomputer_t computer, char * defs, dr_data_source_t ds, error_monitor_t em)
{
    char * sep;
    int rv = 0;
    
    defs = (char*)cpe_str_trim_head(defs);

    while((sep = (char*)cpe_str_char_not_in_pair(defs, ',', "{[(", ")]}"))) {
        * cpe_str_trim_tail(sep, defs) = 0;
        
        if (dr_meta_setup(data, size, meta, computer, defs, ds, em) != 0) rv = -1;
        
        defs =  (char*)cpe_str_trim_head(sep + 1);
    }

    * cpe_str_trim_tail(defs + strlen(defs), defs) = 0;

    if (*defs) {
        if (dr_meta_setup(data, size, meta, computer, defs, ds, em) != 0) rv = -1;
    }
    
    return rv;
}   
