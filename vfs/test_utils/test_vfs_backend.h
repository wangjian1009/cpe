#ifndef TESTS_VFS_TESTFS_BACKEND_H_INCLEDED
#define TESTS_VFS_TESTFS_BACKEND_H_INCLEDED
#include "test_vfs_testenv.h"

vfs_backend_t vfs_testenv_create_backend(test_vfs_testenv_t env);

struct test_vfs_file {
    test_vfs_testenv_t m_env;
    test_vfs_entry_t m_entry;
    TAILQ_ENTRY(test_vfs_file) m_next_for_entry;
    int32_t m_r_pos;
    int32_t m_w_pos;
};
void test_vfs_file_cleaar_entry(test_vfs_file_t f);

struct test_vfs_dir {
    test_vfs_testenv_t m_env;
    test_vfs_entry_t m_entry;
    TAILQ_ENTRY(test_vfs_dir) m_next_for_entry;
};
void test_vfs_dir_cleaar_entry(test_vfs_dir_t f);

#endif
