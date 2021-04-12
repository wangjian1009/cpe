#ifndef CPE_UTILS_YAJL_COPY_TYPES_H
#define CPE_UTILS_YAJL_COPY_TYPES_H
#include "yajl/yajl_tree.h"
#include "yajl_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

yajl_val cpe_yajl_tree_copy(yajl_val val);
    
#ifdef __cplusplus
}
#endif

#endif
