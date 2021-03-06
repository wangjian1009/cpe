#ifndef TESTS_VFS_TESTENV_H_INCLEDED
#define TESTS_VFS_TESTENV_H_INCLEDED
#include "test_memory.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/vfs/vfs_types.h"

typedef struct test_vfs_testenv * test_vfs_testenv_t;
typedef enum test_vfs_entry_type test_vfs_entry_type_t;
typedef struct test_vfs_entry * test_vfs_entry_t;
typedef TAILQ_HEAD(test_vfs_entry_list, test_vfs_entry) test_vfs_entry_list_t;
typedef struct test_vfs_file * test_vfs_file_t;
typedef TAILQ_HEAD(test_vfs_file_list, test_vfs_file) test_vfs_file_list_t;
typedef struct test_vfs_dir * test_vfs_dir_t;
typedef TAILQ_HEAD(test_vfs_dir_list, test_vfs_dir) test_vfs_dir_list_t;

struct test_vfs_testenv {
    error_monitor_t m_em;
    vfs_mgr_t m_mgr;
    vfs_backend_t m_backend;
    test_vfs_entry_t m_current_dir;
    test_vfs_entry_t m_root_dir;
};

test_vfs_testenv_t test_vfs_testenv_create(error_monitor_t em);
void test_vfs_testenv_free(test_vfs_testenv_t env);

mem_buffer_t test_vfs_testenv_install_file(test_vfs_testenv_t env, const char * path);
int test_vfs_testenv_install_file_data(test_vfs_testenv_t env, const char * path, void const * data, uint32_t data_size);
int test_vfs_testenv_install_file_str(test_vfs_testenv_t env, const char * path, const char * data);
int test_vfs_testenv_install_dir(test_vfs_testenv_t env, const char * path);

#endif
