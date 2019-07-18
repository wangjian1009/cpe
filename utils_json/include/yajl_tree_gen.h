#ifndef CPE_UTILS_JSON_TYPES_H
#define CPE_UTILS_JSON_TYPES_H
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct yajl_val_s * yajl_val;
typedef struct cpe_yajl_tree_gen * cpe_yajl_tree_gen_t;
    
struct cpe_yajl_tree_gen {
    mem_buffer_t m_buffer;
    error_monitor_t m_em;
    char m_error_msg[64];
};

#define CPE_YAJL_TREE_GEN_INIT(buffer, em) { (buffer), (em), "" }

yajl_val cpe_yajl_tree_gen_object(cpe_yajl_tree_gen_t gen, uint32_t capaciity);
int cpe_yajl_tree_object_add(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, yajl_val value);
int cpe_yajl_tree_object_add_opt(cpe_yajl_tree_gen_t gen, yajl_val o, const char * key, yajl_val value);

yajl_val cpe_yajl_tree_gen_array(cpe_yajl_tree_gen_t gen, uint32_t capaciity);
int cpe_yajl_tree_array_add(cpe_yajl_tree_gen_t gen, yajl_val o, yajl_val value);
int cpe_yajl_tree_array_add_opt(cpe_yajl_tree_gen_t gen, yajl_val o, yajl_val value);

yajl_val cpe_yajl_tree_gen_str(cpe_yajl_tree_gen_t gen, const char * value);
yajl_val cpe_yajl_tree_gen_null(cpe_yajl_tree_gen_t gen);
yajl_val cpe_yajl_tree_gen_integer(cpe_yajl_tree_gen_t gen, int64_t value);
yajl_val cpe_yajl_tree_gen_double(cpe_yajl_tree_gen_t gen, double value);
yajl_val cpe_yajl_tree_gen_bool(cpe_yajl_tree_gen_t gen, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
