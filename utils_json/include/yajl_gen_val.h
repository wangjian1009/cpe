#ifndef CPE_UTILS_YAJL_GEN_VAL_H
#define CPE_UTILS_YAJL_GEN_VAL_H
#include "yajl/yajl_gen.h"
#include "yajl/yajl_tree.h"
#include "yajl_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

yajl_gen_status yajl_gen_val(yajl_gen gen, yajl_val val);

#ifdef __cplusplus
}
#endif

#endif
