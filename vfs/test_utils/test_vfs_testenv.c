#include "cmocka_all.h"
#include "cpe/pal/pal_string.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/vfs/vfs_backend.h"
#include "cpe/vfs/vfs_mount_point.h"
#include "test_vfs_testenv.h"
#include "test_vfs_backend.h"
#include "test_vfs_entry.h"

test_vfs_testenv_t
test_vfs_testenv_create(error_monitor_t em) {
    test_vfs_testenv_t env = mem_alloc(test_allocrator(), sizeof(struct test_vfs_testenv));
    env->m_em = em;
    env->m_mgr = vfs_mgr_create(test_allocrator(), em);
    env->m_backend = vfs_testenv_create_backend(env);
    env->m_current_dir = test_vfs_entry_create2(env, NULL, "current", test_vfs_entry_dir);
    env->m_root_dir = test_vfs_entry_create2(env, NULL, "root", test_vfs_entry_dir);

    vfs_mount_point_t mount_current =
        vfs_mount_point_mount(
            vfs_mgr_current_point(env->m_mgr), "", env->m_current_dir, env->m_backend);
    assert_true(mount_current);

    vfs_mount_point_t mount_root =
        vfs_mount_point_mount(
            vfs_mgr_root_point(env->m_mgr), "", env->m_root_dir, env->m_backend);
    assert_true(mount_root);
    
    return env;
}

void test_vfs_testenv_free(test_vfs_testenv_t env) {
    vfs_backend_free(env->m_backend);
    test_vfs_entry_free(env->m_current_dir);
    test_vfs_entry_free(env->m_root_dir);
    vfs_mgr_free(env->m_mgr);
    mem_free(test_allocrator(), env);
}

int test_vfs_testenv_install_file_str(test_vfs_testenv_t env, const char * path, const char * data) {
    test_vfs_entry_t entry = test_vfs_entry_create_recursive(env, path, test_vfs_entry_file);
    if (entry == NULL) return -1;

    mem_buffer_clear_data(&entry->m_file.m_content);
    mem_buffer_append(&entry->m_file.m_content, data, strlen(data));
    return 0;
}
