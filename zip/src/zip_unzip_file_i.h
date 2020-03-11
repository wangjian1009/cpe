#ifndef CPE_ZIP_UNZIP_FILE_I_H_INCLEDED
#define CPE_ZIP_UNZIP_FILE_I_H_INCLEDED
#include "cpe/zip/zip_unzip_file.h"
#include "zip_unzip_context_i.h"

struct cpe_unzip_file {
    cpe_unzip_context_t m_context;
    cpe_unzip_dir_t m_parent_dir;
    TAILQ_ENTRY(cpe_unzip_file) m_next_file;
    char m_name[64];
    unz_file_info64 m_file_info;
};

#endif
