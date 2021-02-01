#include "cpe/pal/pal_ctype.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/string_url.h"

static unsigned char hexchars[] = "0123456789ABCDEF";

size_t cpe_url_encode(write_stream_t output, read_stream_t input) {
    uint8_t input_buf[128];
    int input_buf_len;

    size_t output_total_len = 0;
    for(input_buf_len = stream_read(input, input_buf, sizeof(input_buf));
        input_buf_len > 0;
        input_buf_len = stream_read(input, input_buf, sizeof(input_buf)))
    {
        uint8_t output_buf[CPE_ARRAY_SIZE(input_buf) * 3];
        uint32_t output_pos = 0;

        uint16_t input_pos;
        for(input_pos = 0; input_pos < input_buf_len; input_pos++) {
            char ch = input_buf[input_pos];

            if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')
                || ch == '.' || ch == '-' || ch == '_' || ch == '*' )
            {
                output_buf[output_pos++] = ch;
            }
            else if (ch == ' ') {
                output_buf[output_pos++] = '+';
            }
            else {
                output_buf[output_pos++] = '%';
                output_buf[output_pos++] = hexchars[(((uint8_t)ch) & 0xF0) >> 4];
                output_buf[output_pos++] = hexchars[((uint8_t)ch) & 0x0F];
            }
        }

        stream_write(output, output_buf, output_pos);
        
        output_total_len += output_pos;
    }

    return output_total_len;
}

size_t cpe_url_decode(write_stream_t output, read_stream_t input) {
    char input_buf[128];
    int input_buf_len;
    int input_read_len;
    char lpCode[4];

    size_t output_total_len = 0;
    for(input_buf_len = 0, input_read_len = stream_read(input, input_buf, sizeof(input_buf));
        input_buf_len + input_read_len > 0;
        input_read_len = stream_read(input, input_buf + input_buf_len, sizeof(input_buf) - input_buf_len))
    {
        
    }
    
/*     ssize_t j = 0; */

/*     while(input_size > 0) { */
/*         char ch = *input; */
/*         int used; */
        
/*         if (j + 1 > result_capacity) { */
/*             CPE_ERROR(em, "cpe_url_decode: result overflow!"); */
/*             return -1; */
/*         } */
            
/*         if (ch == '+') { */
/*             result[j++] = ' '; */
/*             used = 1; */
/*         } */
/*         else if (ch == '%' && input_size >= 2 && isxdigit((int)input[1]) && isxdigit((int)input[2])) { */
/*             int value; */
            
/*             ch = input[j + 1]; */
/*             if (isupper(ch)) ch = tolower(ch); */
/*             value = (ch >= '0' && ch <= '9' ? ch - '0' : ch - 'a' + 10) * 16; */

/*             ch = input[j + 2]; */
/*             if (isupper(ch)) ch = tolower(ch); */
/*             value += ch >= '0' && ch <= '9' ? ch - '0' : ch - 'a' + 10; */

/*             result[j++] = (char)value; */
/*             used = 3; */
/*         } */
/*         else { */
/*             result[j++] = ch; */
/*             used = 1; */
/*         } */

/*         input_size -= used; */
/*         input += used; */
/*     } */
    
/*     if (j + 1 > result_capacity) { */
/*         CPE_ERROR(em, "cpe_url_encode: result overflow!"); */
/*         return -1; */
/*     } */
            
/*     result[j] = '\0'; */
    
/*     return j; */
    return output_total_len;
}

ssize_t cpe_url_encode_from_buf(write_stream_t output, const char * input, size_t input_size) {
    struct read_stream_mem is = CPE_READ_STREAM_MEM_INITIALIZER(input, input_size);
    return cpe_url_encode(output, (read_stream_t)&is);
}

ssize_t cpe_url_decode_from_buf(write_stream_t output, const char * input, size_t input_size) {
    struct read_stream_mem is = CPE_READ_STREAM_MEM_INITIALIZER(input, input_size);
    return cpe_url_decode(output, (read_stream_t)&is);
}

size_t cpe_url_decode_inline(char * str, size_t len) {
    char *dest = str;
    char *data = str;

    int value;
    int c;

    while(len--) {
        if (*data == '+') {
            *dest = ' ';
        }
        else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
                 && isxdigit((int) *(data + 2)))
        {

            c = ((unsigned char *)(data+1))[0];
            if (isupper(c))
                c = tolower(c);
            value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;
            c = ((unsigned char *)(data+1))[1];
            if (isupper(c))
                c = tolower(c);
            value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

            *dest = (char)value ;
            data += 2;
            len -= 2;
        } else {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';
    return dest - str;
}
