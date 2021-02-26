#ifndef CPE_UTILS_YAJL_TYPES_H
#define CPE_UTILS_YAJL_TYPES_H
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * cpe_yajl_tree_type_str(int type);

void cpe_yajl_stream_escape_str_range(write_stream_t ws, const char * msg, const char * msg_end);
void cpe_yajl_stream_escape_str(write_stream_t ws, const char * msg);
void cpe_yajl_gen_print_to_stream(void * ctx, const char * str, size_t len);

#ifdef __cplusplus
}
#endif

#endif
