#ifndef TESTS_VFS_FILE_H_INCLEDED
#define TESTS_VFS_FILE_H_INCLEDED
#include "cpe/utils/buffer.h"
#include "test_vfs_testenv.h"

struct test_vfs_file {
    test_vfs_testenv_t m_env;
    test_vfs_dir_t m_dir;
    TAILQ_ENTRY(test_vfs_file) m_next_for_dir;
    char * m_name;
    struct mem_buffer m_content;
};

test_vfs_file_t test_vfs_file_create(test_vfs_testenv_t env, test_vfs_dir_t dir, const char * name);
void test_vfs_file_free(test_vfs_file_t file);

#endif
