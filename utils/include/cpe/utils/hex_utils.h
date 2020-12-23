#ifndef CPE_UTILS_HEX_H
#define CPE_UTILS_HEX_H
#include "stream.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

char * cpe_hex_dup(read_stream_t input, mem_buffer_t buffer);
char * cpe_hex_dup_buf(const void * buf, size_t size, mem_buffer_t buffer);

int cpe_hex_2_bin(uint8_t * p, const char * hexstr, size_t len, error_monitor_t em);
void cpe_bin_2_hex(char * o, uint8_t const * p, size_t len);

void cpe_hex_print_bin_2_hex(write_stream_t ws, uint8_t const * p, size_t len);

void cpe_uint32_2_hex(char * o, uint32_t v);
int cpe_hex_to_uint32(uint32_t *v, const char * str_value);
int cpe_hex_str_to_uint32(uint32_t *v, const char * str_value);

void cpe_uint64_2_hex(char * o, uint64_t v);
int cpe_hex_to_uint64(uint64_t *v, const char * str_value);
int cpe_hex_str_to_uint64(uint64_t *v, const char * str_value);
    
#ifdef __cplusplus
}
#endif

#endif
