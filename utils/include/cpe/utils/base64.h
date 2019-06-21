#ifndef CPE_UTILS_BASE64_H
#define CPE_UTILS_BASE64_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t cpe_base64_encode(write_stream_t output, read_stream_t input);
size_t cpe_base64_decode(write_stream_t output, read_stream_t input);

const char * cpe_base64_dump(mem_buffer_t buffer, void const * data, size_t data_len);
    
#define cpe_base64_size(x)  (((x)+2) / 3 * 4 + 1)

#ifdef __cplusplus
}
#endif

#endif
