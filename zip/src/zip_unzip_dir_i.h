#ifndef CPE_ZIP_UNZIP_DIR_I_H_INCLEDED
#define CPE_ZIP_UNZIP_DIR_I_H_INCLEDED
#include "cpe/zip/zip_unzip_file.h"
#include "zip_unzip_context_i.h"

struct cpe_unzip_dir {
    cpe_unzip_context_t m_context;
    cpe_unzip_dir_t m_parent_dir;
    TAILQ_ENTRY(cpe_unzip_dir) m_next_dir;
    cpe_unzip_dir_list_t m_child_dirs;
    cpe_unzip_file_list_t m_child_files;
    char m_name[64];
};

cpe_unzip_dir_t
cpe_unzip_dir_create(cpe_unzip_context_t context, cpe_unzip_dir_t parent, const char * name, error_monitor_t em);

void cpe_unzip_dir_free(cpe_unzip_dir_t d);

void cpe_unzip_dir_build_path(mem_buffer_t buffer, cpe_unzip_dir_t d);

#endif
