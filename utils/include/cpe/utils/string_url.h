#ifndef CPE_UTILS_STRING_URL_H
#define CPE_UTILS_STRING_URL_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t cpe_url_encode(write_stream_t output, read_stream_t input);
size_t cpe_url_decode(write_stream_t output, read_stream_t input);
    
ssize_t cpe_url_encode_from_buf(write_stream_t output, const char * input, size_t input_size, error_monitor_t em);
ssize_t cpe_url_decode_from_buf(write_stream_t output, const char * input, size_t input_size, error_monitor_t em);

size_t cpe_url_decode_inline(char * str, size_t len);
    
#ifdef __cplusplus
}
#endif

#endif
