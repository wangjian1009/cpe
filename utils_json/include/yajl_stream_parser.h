#ifndef CPE_UTILS_YAJL_STREAM_PARSER_H
#define CPE_UTILS_YAJL_STREAM_PARSER_H
#include "yajl_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

cpe_yajl_stream_parser_t cpe_yajl_stream_parser_create(mem_allocrator_t alloc, error_monitor_t em);
void cpe_yajl_stream_parser_free(cpe_yajl_stream_parser_t parser);

int cpe_yajl_stream_parser_append(cpe_yajl_stream_parser_t parser, void const * data, uint32_t data_size);
int cpe_yajl_stream_parser_complete(cpe_yajl_stream_parser_t parser);
    
uint8_t cpe_yajl_stream_parser_is_complete(cpe_yajl_stream_parser_t parser);
uint8_t cpe_yajl_stream_parser_is_ok(cpe_yajl_stream_parser_t parser);

const char * cpe_yajl_stream_parser_error_msg(cpe_yajl_stream_parser_t parser);

yajl_val cpe_yajl_stream_parser_result(cpe_yajl_stream_parser_t parser);
yajl_val cpe_yajl_stream_parser_result_retrieve(cpe_yajl_stream_parser_t parser);
    
#ifdef __cplusplus
}
#endif

#endif
