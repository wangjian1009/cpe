#ifndef CPE_UTILS_YAJL_GEN_TYPES_H
#define CPE_UTILS_YAJL_GEN_TYPES_H
#include "yajl/yajl_tree.h"
#include "yajl_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_yajl_tree_gen {
    mem_buffer_t m_buffer;
    error_monitor_t m_em;
    char m_error_msg[64];
};

#define CPE_YAJL_TREE_GEN_INIT(buffer, em) { (buffer), (em), "" }

yajl_val cpe_yajl_tree_dup(cpe_yajl_tree_gen_t gen, yajl_val value);
    
yajl_val cpe_yajl_tree_gen_object(cpe_yajl_tree_gen_t gen, uint32_t capaciity);
int cpe_yajl_tree_object_add(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, yajl_val value);
int cpe_yajl_tree_object_add_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, yajl_val value);
yajl_val cpe_yajl_tree_object_add_object(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, uint32_t capacity);
yajl_val cpe_yajl_tree_object_add_array(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, uint32_t capacity);    
int cpe_yajl_tree_object_add_string(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, const char * value);
int cpe_yajl_tree_object_add_string_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, const char * value);
int cpe_yajl_tree_object_add_integer(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, int64_t value);
int cpe_yajl_tree_object_add_double(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, double value);
int cpe_yajl_tree_object_add_bool(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, uint8_t value);
int cpe_yajl_tree_object_add_null(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key);

yajl_val cpe_yajl_tree_gen_array(cpe_yajl_tree_gen_t gen, uint32_t capaciity);
int cpe_yajl_tree_array_add(cpe_yajl_tree_gen_t gen, yajl_val o, yajl_val value);
int cpe_yajl_tree_array_add_opt(cpe_yajl_tree_gen_t gen, yajl_val o, yajl_val value);
yajl_val cpe_yajl_tree_array_add_object(cpe_yajl_tree_gen_t gen, yajl_val o, uint32_t capacity);
yajl_val cpe_yajl_tree_array_add_array(cpe_yajl_tree_gen_t gen, yajl_val o, uint32_t capacity);    
int cpe_yajl_tree_array_add_string(cpe_yajl_tree_gen_t gen, yajl_val o, const char * value);
int cpe_yajl_tree_array_add_string_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * value);
int cpe_yajl_tree_array_add_integer(cpe_yajl_tree_gen_t gen, yajl_val o, int64_t value);
int cpe_yajl_tree_array_add_double(cpe_yajl_tree_gen_t gen, yajl_val o, double value);
int cpe_yajl_tree_array_add_bool(cpe_yajl_tree_gen_t gen, yajl_val o, uint8_t value);
int cpe_yajl_tree_array_add_null(cpe_yajl_tree_gen_t gen, yajl_val o);

yajl_val cpe_yajl_tree_gen_string(cpe_yajl_tree_gen_t gen, const char * value);
yajl_val cpe_yajl_tree_gen_null(cpe_yajl_tree_gen_t gen);
yajl_val cpe_yajl_tree_gen_integer(cpe_yajl_tree_gen_t gen, int64_t value);
yajl_val cpe_yajl_tree_gen_double(cpe_yajl_tree_gen_t gen, double value);
yajl_val cpe_yajl_tree_gen_bool(cpe_yajl_tree_gen_t gen, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
