#ifndef CPE_ZIP_UNZIP_CONTEXT_TYPES_H_INCLEDED
#define CPE_ZIP_UNZIP_CONTEXT_TYPES_H_INCLEDED
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/zip/zip_types.h"
#include "zip_pro_unzip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(cpe_unzip_dir_list, cpe_unzip_dir) cpe_unzip_dir_list_t;
typedef TAILQ_HEAD(cpe_unzip_file_list, cpe_unzip_file) cpe_unzip_file_list_t;

struct cpe_unzip_context {
    mem_allocrator_t m_alloc;
    unzFile m_zip_file;
    unz_global_info64 m_global_info;
    cpe_unzip_dir_t m_root;
};

#ifdef __cplusplus
}
#endif

#endif
