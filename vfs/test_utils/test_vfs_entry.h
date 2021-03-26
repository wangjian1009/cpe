#ifndef TESTS_VFS_TESTENV_ENTRY_H_INCLEDED
#define TESTS_VFS_TESTENV_ENTRY_H_INCLEDED
#include "cpe/utils/buffer.h"
#include "test_vfs_testenv.h"

#ifdef __cplusplus
extern "C" {
#endif

enum test_vfs_entry_type {
    test_vfs_entry_file,
    test_vfs_entry_dir,
};
    
struct test_vfs_entry {
    test_vfs_testenv_t m_env;
    test_vfs_entry_t m_parent;
    TAILQ_ENTRY(test_vfs_entry) m_next;
    char * m_name;
    test_vfs_entry_type_t m_type;
    union {
        struct {
            test_vfs_entry_list_t m_childs;
            test_vfs_dir_list_t m_opend_dirs;
        } m_dir;
        struct {
            struct mem_buffer m_content;
            test_vfs_file_list_t m_opend_files;
        } m_file;
    };
};

test_vfs_entry_t
test_vfs_entry_create(
    test_vfs_testenv_t env, test_vfs_entry_t parent,
    const char * name, const char * name_end, test_vfs_entry_type_t type);

test_vfs_entry_t
test_vfs_entry_create2(
    test_vfs_testenv_t env, test_vfs_entry_t parent,
    const char * name, test_vfs_entry_type_t type);

void test_vfs_entry_free(test_vfs_entry_t entry);

test_vfs_entry_t test_vfs_entry_find_child_by_name(test_vfs_entry_t parent, const char * name, const char * name_end);
test_vfs_entry_t test_vfs_entry_find_child_by_path(test_vfs_entry_t parent, const char * name, const char * path_end);    

test_vfs_entry_t test_vfs_entry_create_recursive(test_vfs_testenv_t env, const char * path, test_vfs_entry_type_t type);

const char * test_vfs_entry_type_str(test_vfs_entry_type_t type);

#ifdef __cplusplus
}
#endif

#endif
