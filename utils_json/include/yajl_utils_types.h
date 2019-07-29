#ifndef CPE_UTILS_YAJL_TYPES_H
#define CPE_UTILS_YAJL_TYPES_H
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cpe_yajl_tree_gen * cpe_yajl_tree_gen_t;

const char * cpe_yajl_tree_type_str(int type);
    
#ifdef __cplusplus
}
#endif

#endif
