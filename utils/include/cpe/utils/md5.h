#ifndef CPE_MD5_H
#define CPE_MD5_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPE_MD5_VALUE_DIGEST_COUNT (16)

struct cpe_md5_value {
    uint8_t digest[CPE_MD5_VALUE_DIGEST_COUNT];
};

struct cpe_md5_ctx {
    uint32_t i[2];
    uint32_t buf[4];
    uint8_t in[64];
    struct cpe_md5_value value;
};

void cpe_md5_ctx_init(cpe_md5_ctx_t ctx);
void cpe_md5_ctx_update(cpe_md5_ctx_t ctx, void const * inBuf, size_t inLen);
void cpe_md5_ctx_final(cpe_md5_ctx_t ctx);

#define cpe_md5_ctx_update_sizeof(__ctx, __v) cpe_md5_ctx_update((__ctx), &(__v), sizeof(__v))
#define cpe_md5_ctx_update_string(__ctx, __v) \
    do {\
        const char * __v_tmp = (__v);\
        if (__v_tmp) cpe_md5_ctx_update((__ctx), __v_tmp, strlen(__v_tmp)); \
    } while(0)
    
int cpe_md5_cmp(cpe_md5_value_t l, cpe_md5_value_t r);
uint32_t cpe_hash_md5(cpe_md5_value_t value);

#define CPE_MD5_STR_BUF_SIZE (33)
void cpe_md5_to_string(char str[CPE_MD5_STR_BUF_SIZE], cpe_md5_value_t value);
void cpe_md5_print(write_stream_t s, cpe_md5_value_t value);
const char * cpe_md5_dump(cpe_md5_value_t value, mem_buffer_t buff);
int cpe_md5_parse(cpe_md5_value_t value, const char * str);

extern struct cpe_md5_value CPE_MD5_INVALID;

#ifdef __cplusplus
}
#endif

#endif
