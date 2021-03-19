#ifndef CPE_UTILS_YAJL_DUMP_TYPES_H
#define  CPE_UTILS_YAJL_DUMP_TYPES_H
#include "yajl/yajl_tree.h"
#include "yajl/yajl_gen.h"
#include "yajl_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void yajl_tree_print(write_stream_t ws, yajl_val data);
const char * yajl_tree_dump(mem_buffer_t buff, yajl_val data);

#ifdef __cplusplus
}
#endif

#endif
