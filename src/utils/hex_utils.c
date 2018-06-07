#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/hex_utils.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream_mem.h" 
#include "cpe/utils/stream_buffer.h" 

char * cpe_hex_dup(read_stream_t input, mem_buffer_t buffer) {
    unsigned char buf[16];
    size_t size;
    int first_line;
    struct write_stream_buffer output = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

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
