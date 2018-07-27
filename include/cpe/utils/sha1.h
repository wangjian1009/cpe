#ifndef CPE_UTILS_SHA1_H
#define CPE_UTILS_SHA1_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPE_SHA1_VALUE_DIGEST_COUNT (20)

struct cpe_sha1_value {
    uint8_t digest[CPE_SHA1_VALUE_DIGEST_COUNT];
};

struct cpe_sha1_ctx {
    uint32_t total[2];    /*!< The number of Bytes processed.  */
    uint32_t state[5];    /*!< The intermediate digest state.  */
    uint8_t buffer[64];   /*!< The data block being processed. */
};

void cpe_sha1_init(cpe_sha1_ctx_t ctx);
void cpe_sha1_fini(cpe_sha1_ctx_t ctx);
void cpe_sha1_copy(cpe_sha1_ctx_t dst, cpe_sha1_ctx_t src);

int cpe_sha1_starts(cpe_sha1_ctx_t ctx);
int cpe_sha1_update(cpe_sha1_ctx_t ctx, const void * input, size_t len);
int cpe_sha1_finish(cpe_sha1_ctx_t ctx, cpe_sha1_value_t output);
    
int cpe_sha1_encode_rs(cpe_sha1_value_t output, read_stream_t input);
int cpe_sha1_encode_str(cpe_sha1_value_t output, const char * input);
int cpe_sha1_encode_buf(cpe_sha1_value_t output, const void * input, size_t input_len);

#ifdef __cplusplus
}
#endif

#endif
