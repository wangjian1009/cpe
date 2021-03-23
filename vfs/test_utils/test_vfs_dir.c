#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "test_vfs_dir.h"
#include "test_vfs_file.h"

test_vfs_dir_t
test_vfs_dir_create(test_vfs_testenv_t env, test_vfs_dir_t parent, const char * name) {
    test_vfs_dir_t dir = mem_alloc(test_allocrator(), sizeof(struct test_vfs_dir));
    assert(dir);

    dir->m_env = env;
    dir->m_parent = parent;
    dir->m_name = cpe_str_mem_dup(test_allocrator(), name);
    assert(dir->m_name);

    TAILQ_INIT(&dir->m_child_dirs);
    TAILQ_INIT(&dir->m_files);

    if (dir->m_parent) {
        TAILQ_INSERT_TAIL(&dir->m_parent->m_child_dirs, dir, m_next_for_parent);
    }
    
    return dir;
}

void test_vfs_dir_free(test_vfs_dir_t dir) {
    while(!TAILQ_EMPTY(&dir->m_files)) {
        test_vfs_file_free(TAILQ_FIRST(&dir->m_files));
    }

    while(!TAILQ_EMPTY(&dir->m_child_dirs)) {
        test_vfs_dir_free(TAILQ_FIRST(&dir->m_child_dirs));
    }

    if (dir->m_parent) {
        TAILQ_REMOVE(&dir->m_parent->m_child_dirs, dir, m_next_for_parent);
        dir->m_parent = NULL;
    }

    if (dir->m_name) {
        mem_free(test_allocrator(), dir->m_name);
    }
    
    mem_free(test_allocrator(), dir);
}

