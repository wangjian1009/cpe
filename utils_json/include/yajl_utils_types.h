#ifndef CPE_UTILS_YAJL_TYPES_H
#define CPE_UTILS_YAJL_TYPES_H
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cpe_yajl_tree_gen * cpe_yajl_tree_gen_t;

const char * cpe_yajl_tree_type_str(int type);

void cpe_yajl_stream_escape_str_range(write_stream_t ws, const char * msg, const char * msg_end);
void cpe_yajl_stream_escape_str(write_stream_t ws, const char * msg);

#ifdef __cplusplus
}
#endif

#endif
