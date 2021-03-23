#include "cmocka_all.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/vfs/vfs_backend.h"
#include "cpe/vfs/vfs_mount_point.h"
#include "test_vfs_testenv.h"
#include "test_vfs_backend.h"
#include "test_vfs_dir.h"

test_vfs_testenv_t
test_vfs_testenv_create(error_monitor_t em) {
    test_vfs_testenv_t env = mem_alloc(test_allocrator(), sizeof(struct test_vfs_testenv));
    env->m_em = em;
    env->m_mgr = vfs_mgr_create(test_allocrator(), em);
    env->m_backend = vfs_testenv_create_backend(env);
    env->m_current_dir = test_vfs_dir_create(env, NULL, "current");
    env->m_root_dir = test_vfs_dir_create(env, NULL, "root");

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
    test_vfs_dir_free(env->m_current_dir);
    test_vfs_dir_free(env->m_root_dir);
    vfs_mgr_free(env->m_mgr);
    mem_free(test_allocrator(), env);
}

int test_vfs_testenv_install_file_str(test_vfs_testenv_t env, const char * path, const char * data) {
    test_vfs_dir_t dir = env->m_current_dir;

    if (path[0] == '/') {
        path++;
        dir = env->m_root_dir;
    }

    return 0;
}
