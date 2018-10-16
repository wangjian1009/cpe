#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/hex_utils.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream_mem.h" 
#include "cpe/utils/stream_buffer.h" 

char * cpe_hex_dup(read_stream_t input, mem_buffer_t buffer) {
    unsigned char buf[16];
    size_t size;
    int first_line;
    struct write_stream_buffer output = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    mem_buffer_clear_data(buffer);

    first_line = 1;
    size = stream_read(input, buf, sizeof(buf));
    while(size > 0) {
        size_t pos;

        if (first_line) {
            first_line = 0;
        }
        else {
            stream_putc((write_stream_t)&output, ' ');
        }

        for(pos = 0; pos < size; ++pos) {
            if (pos > 0) {
                stream_putc((write_stream_t)&output, ' ');
            }

            stream_printf((write_stream_t)&output, "0x%02X", buf[pos]);
        }

        if (size == sizeof(buf)) {
            size = stream_read(input, buf, sizeof(buf));
        }
        else {
            size = 0;
        }
    } 
    
    stream_putc((write_stream_t)&output, 0);
    return (char *)mem_buffer_make_continuous(buffer, 0);
}

char * cpe_hex_dup_buf(const void * buf, size_t size, mem_buffer_t buffer) {
    struct read_stream_mem stream = CPE_READ_STREAM_MEM_INITIALIZER(buf, size);

    return cpe_hex_dup((read_stream_t)&stream, buffer);
}

int cpe_hex_2_bin(uint8_t * p, const char * hexstr, size_t len, error_monitor_t em) {
    char hex_byte[3];
    char *ep;

    hex_byte[2] = '\0';

    while (*hexstr && len) {
        if (!hexstr[1]) {
            CPE_ERROR(em, "hex2bin str truncated");
            return -1;
        }
        hex_byte[0] = hexstr[0];
        hex_byte[1] = hexstr[1];
        *p = (uint8_t)strtol(hex_byte, &ep, 16);
        if (*ep) {
            CPE_ERROR(em, "hex2bin failed on '%s'", hex_byte);
            return -1;
        }
        p++;
        hexstr += 2;
        len--;
    }

    return (len == 0 && *hexstr == 0) ? 0 : -1;
}

const char * s_hex_num2char = "0123456789abcdef";

void cpe_bin_2_hex(char * o, uint8_t const * p, size_t len) {
    while (len) {
        o[0] = s_hex_num2char[(*p) >> 4];
        o[1] = s_hex_num2char[(*p) & 0xF];
        p++;
        o += 2;
        len--;
    }
}

void cpe_uint32_2_hex(char * o, uint32_t v) {
    o[0] = s_hex_num2char[(v >> 28) & 0xF];
    o[1] = s_hex_num2char[(v >> 24) & 0xF];
    o[2] = s_hex_num2char[(v >> 20) & 0xF];
    o[3] = s_hex_num2char[(v >> 16) & 0xF];
    o[4] = s_hex_num2char[(v >> 12) & 0xF];
    o[5] = s_hex_num2char[(v >> 8) & 0xF];
    o[6] = s_hex_num2char[(v >> 4) & 0xF];
    o[7] = s_hex_num2char[(v >> 0) & 0xF];
}

int cpe_hex_to_uint32(uint32_t *v, const char * str_value) {
    int i;
    uint32_t r = 0;
    for(i = 0; i < (sizeof(*v) * 2); ++i) {
        char c = str_value[i];

        r <<= 4;
        if (c >= '0' && c <= '9') {
            r += c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            r += (c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F') {
            r += (c - 'A' + 10);
        }
        else {
            return -1;
        }
    }

    *v = r;
    return 0;
}

void cpe_uint64_2_hex(char * o, uint64_t v) {
    o[0] = s_hex_num2char[(v >> 60) & 0xF];
    o[1] = s_hex_num2char[(v >> 56) & 0xF];
    o[2] = s_hex_num2char[(v >> 52) & 0xF];
    o[3] = s_hex_num2char[(v >> 48) & 0xF];
    o[4] = s_hex_num2char[(v >> 44) & 0xF];
    o[5] = s_hex_num2char[(v >> 40) & 0xF];
    o[6] = s_hex_num2char[(v >> 36) & 0xF];
    o[7] = s_hex_num2char[(v >> 32) & 0xF];
    o[8] = s_hex_num2char[(v >> 28) & 0xF];
    o[9] = s_hex_num2char[(v >> 24) & 0xF];
    o[10] = s_hex_num2char[(v >> 20) & 0xF];
    o[11] = s_hex_num2char[(v >> 16) & 0xF];
    o[12] = s_hex_num2char[(v >> 12) & 0xF];
    o[13] = s_hex_num2char[(v >> 8) & 0xF];
    o[14] = s_hex_num2char[(v >> 4) & 0xF];
    o[15] = s_hex_num2char[(v >> 0) & 0xF];
}

int cpe_hex_to_uint64(uint64_t *v, const char * str_value) {
    int i;
    uint64_t r = 0;
    for(i = 0; i < sizeof(*v) * 2; ++i) {
        char c = str_value[i];

        r <<= 4;
        if (c >= '0' && c <= '9') {
            r += c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            r += (c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F') {
            r += (c - 'A' + 10);
        }
        else {
            return -1;
        }
    }

    *v = r;
    return 0;
}

int cpe_hex_str_to_uint64(uint64_t *v, const char * str_value) {
    size_t len = strlen(str_value);
    if (len > (sizeof(*v) * 2)) return -1;
    
    size_t i;
    uint64_t r = 0;
    for(i = 0; i < len; ++i) {
        char c = str_value[i];

        r <<= 4;
        if (c >= '0' && c <= '9') {
            r += c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            r += (c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F') {
            r += (c - 'A' + 10);
        }
        else {
            return -1;
        }
    }

    *v = r;
    return 0;
}

int cpe_hex_str_to_uint32(uint32_t *v, const char * str_value) {
    size_t len = strlen(str_value);
    if (len > (sizeof(*v) * 2)) return -1;

    size_t i;
    uint32_t r = 0;
    for(i = 0; i < len; ++i) {
        char c = str_value[i];

        r <<= 4;
        if (c >= '0' && c <= '9') {
            r += c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            r += (c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F') {
            r += (c - 'A' + 10);
        }
        else {
            return -1;
        }
    }

    *v = r;
    return 0;
}
