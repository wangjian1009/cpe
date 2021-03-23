#ifndef TESTS_VFS_DIR_H_INCLEDED
#define TESTS_VFS_DIR_H_INCLEDED
#include "test_vfs_testenv.h"

struct test_vfs_dir {
    test_vfs_testenv_t m_env;
    test_vfs_dir_t m_parent;
    TAILQ_ENTRY(test_vfs_dir) m_next_for_parent;
    char * m_name;
    test_vfs_dir_list_t m_child_dirs;
    test_vfs_file_list_t m_files;
};

test_vfs_dir_t
test_vfs_dir_create(test_vfs_testenv_t env, test_vfs_dir_t parent, const char * name);

void test_vfs_dir_free(test_vfs_dir_t dir);

#endif
