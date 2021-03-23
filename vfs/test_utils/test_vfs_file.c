#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "test_vfs_file.h"
#include "test_vfs_dir.h"

test_vfs_file_t
test_vfs_file_create(test_vfs_testenv_t env, test_vfs_dir_t dir, const char * name) {
    test_vfs_file_t file = mem_alloc(test_allocrator(), sizeof(struct test_vfs_file));

    file->m_env = env;
    file->m_name = cpe_str_mem_dup(test_allocrator(), name);
    file->m_dir = dir;

    TAILQ_INSERT_TAIL(&dir->m_files, file, m_next_for_dir);

    mem_buffer_init(&file->m_content, test_allocrator());
    
    return file;
}

void test_vfs_file_free(test_vfs_file_t file) {
    mem_free(test_allocrator(), file->m_name);

    mem_buffer_clear(&file->m_content);

    assert(file->m_dir);
    TAILQ_REMOVE(&file->m_dir->m_files, file, m_next_for_dir);

    mem_free(test_allocrator(), file);
}
