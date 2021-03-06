#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "test_vfs_entry.h"
#include "test_vfs_backend.h"

test_vfs_entry_t
test_vfs_entry_create(
    test_vfs_testenv_t env, test_vfs_entry_t parent,
    const char * name, const char * name_end, test_vfs_entry_type_t type)
{
    test_vfs_entry_t entry;
    
    entry = mem_alloc(test_allocrator(), sizeof(struct test_vfs_entry));
    if (entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_entry_create: alloc fail!");
        return NULL;
    }

    entry->m_env = env;
    entry->m_parent = parent;
    entry->m_name = cpe_str_mem_dup_range(test_allocrator(), name, name_end);
    entry->m_type = type;

    switch(type) {
    case test_vfs_entry_file:
        mem_buffer_init(&entry->m_file.m_content,test_allocrator());
        TAILQ_INIT(&entry->m_file.m_opend_files);
        break;
    case test_vfs_entry_dir:
        TAILQ_INIT(&entry->m_dir.m_childs);
        TAILQ_INIT(&entry->m_dir.m_opend_dirs);
        break;
    }
    
    if (parent) {
        assert(parent->m_type == test_vfs_entry_dir);
        TAILQ_INSERT_TAIL(&parent->m_dir.m_childs, entry, m_next);
    }
    
    return entry;
}

test_vfs_entry_t
test_vfs_entry_create2(
    test_vfs_testenv_t env, test_vfs_entry_t parent, const char * name, test_vfs_entry_type_t type)
{
    return test_vfs_entry_create(env, parent, name, name + strlen(name), type);
}

void test_vfs_entry_free(test_vfs_entry_t entry) {
    switch(entry->m_type) {
    case test_vfs_entry_dir:
        while(!TAILQ_EMPTY(&entry->m_dir.m_childs)) {
            test_vfs_entry_free(TAILQ_FIRST(&entry->m_dir.m_childs));
        }

        while(!TAILQ_EMPTY(&entry->m_dir.m_opend_dirs)) {
            test_vfs_dir_cleaar_entry(TAILQ_FIRST(&entry->m_dir.m_opend_dirs));
        }

        break;
    case test_vfs_entry_file:
        mem_buffer_clear(&entry->m_file.m_content);

        while(!TAILQ_EMPTY(&entry->m_file.m_opend_files)) {
            test_vfs_file_cleaar_entry(TAILQ_FIRST(&entry->m_file.m_opend_files));
        }

        break;
    }    

    if (entry->m_parent) {
        assert(entry->m_parent->m_type == test_vfs_entry_dir);
        TAILQ_REMOVE(&entry->m_parent->m_dir.m_childs, entry, m_next);
    }

    mem_free(test_allocrator(), entry);
}

void test_vfs_entry_set_name(test_vfs_entry_t entry, const char * name) {
    if (entry->m_name) {
        mem_free(test_allocrator(), entry->m_name);
    }

    entry->m_name = cpe_str_mem_dup(test_allocrator(), name);
    assert(entry->m_name);
}

void test_vfs_entry_set_parent(test_vfs_entry_t entry, test_vfs_entry_t parent) {
    if (entry->m_parent) {
        TAILQ_REMOVE(&entry->m_parent->m_dir.m_childs, entry, m_next);
    }

    entry->m_parent = parent;

    if (entry->m_parent) {
        TAILQ_INSERT_TAIL(&entry->m_parent->m_dir.m_childs, entry, m_next);
    }
}

test_vfs_entry_t
test_vfs_entry_find_child_by_name(test_vfs_entry_t parent, const char * name, const char * name_end) {
    test_vfs_entry_t entry;

    if (parent->m_type != test_vfs_entry_dir) return NULL;

    size_t name_len = name_end - name;
    TAILQ_FOREACH(entry, &parent->m_dir.m_childs, m_next) {
        if (strlen(entry->m_name) != name_len) continue;
        if (memcmp(name, entry->m_name, name_len) == 0) return entry;
    }

    return NULL;
}

test_vfs_entry_t
test_vfs_entry_find_child_by_path(test_vfs_entry_t parent, const char * path, const char * path_end) {
    test_vfs_testenv_t env = parent->m_env;
    const char * sep;

    for(sep = cpe_str_char_range(path, path_end, '/');
        sep;
        path = sep + 1, sep = cpe_str_char_range(path, path_end, '/'))
    {
        assert(sep < path_end);
        if (sep > path) {
            parent = test_vfs_entry_find_child_by_name(parent, path, sep);
            if (parent == NULL) return NULL;
        }
    }

    if (path < path_end) {
        parent = test_vfs_entry_find_child_by_name(parent, path, path_end);
    }

    return parent;
}

test_vfs_entry_t
test_vfs_entry_create_recursive(
    test_vfs_testenv_t env, const char * path, test_vfs_entry_type_t type, uint8_t auto_create)
{
    test_vfs_entry_t parent;
    test_vfs_entry_t cur;
    const char * sep;

    parent = path[0] == '/' ? env->m_root_dir : env->m_current_dir;
    while((sep = strchr(path, '/'))) {
        if (sep > path) {
            cur = test_vfs_entry_find_child_by_name(parent, path, sep);
            if (cur == NULL) {
                if (!auto_create) {
                    CPE_ERROR(
                        env->m_em, "test_vfs_entry_create_recursive: %s/%.*s not exist",
                        parent->m_name, (int)(sep - path), path);
                    return NULL;
                }
                
                cur = test_vfs_entry_create(env, parent, path, sep, 1);
                if (cur == NULL) {
                    CPE_ERROR(env->m_em, "test_vfs_entry_create_recursive: create entry fail");
                    return NULL;
                }
            }
            else if (cur->m_type != test_vfs_entry_dir) {
                CPE_ERROR(env->m_em, "test_vfs_entry_create_recursive: %s is not dir", path);
                return NULL;
            }
        
            parent = cur;
        }
        
        path = sep + 1;
    }

    if (path[0] == 0) {
        CPE_ERROR(env->m_em, "test_vfs_entry_create_recursive: no file name");
        return NULL;
    }

    if ((cur = test_vfs_entry_find_child_by_name(parent, path, path + strlen(path)))) {
        if (auto_create) {
            return cur;
        }
        else {
            CPE_ERROR(env->m_em, "test_vfs_entry_create_recursive: %s already exist", path);
            return NULL;
        }
    }
    
    cur = test_vfs_entry_create2(env, parent, path, type);
    if (cur == NULL) {
        CPE_ERROR(
            env->m_em, "test_vfs_entry_create_recursive: create %s entry fail",
            test_vfs_entry_type_str(type));
        return NULL;
    }

    return cur;
}

const char * test_vfs_entry_type_str(test_vfs_entry_type_t type) {
    switch(type) {
    case test_vfs_entry_file:
        return "file";
    case test_vfs_entry_dir:
        return "dir";
    }
}
