#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_backend.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_entry_info.h"
#include "test_vfs_backend.h"
#include "test_vfs_entry.h"

/*file*/
void test_vfs_file_cleaar_entry(test_vfs_file_t f) {
    if (f->m_entry) {
        TAILQ_REMOVE(&f->m_entry->m_file.m_opend_files, f, m_next_for_entry);
        f->m_entry = NULL;
    }
}

static int test_vfs_file_open(void * ctx, void * mount_env, vfs_file_t file, const char * path, const char * mode) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t mount_root = mount_env;

    test_vfs_file_t fp = vfs_file_data(file);
    fp->m_env = env;
    fp->m_r_pos = 0;
    fp->m_w_pos = 0;
    fp->m_entry = test_vfs_entry_find_child_by_path(mount_root, path, path + strlen(path));
    if (fp->m_entry == NULL) {
        if (strchr(mode, 'w')) {
            fp->m_entry = test_vfs_entry_create(env, mount_root, path, path + strlen(path), test_vfs_entry_file);
            if (fp->m_entry == NULL) {
                CPE_ERROR(env->m_em, "test_vfs_file_open: create entry fail");
                return -1;
            }
        } else {
            CPE_ERROR(env->m_em, "test_vfs_file_open: open %s fail", path);
            return -1;
        }
    } else if (fp->m_entry->m_type == test_vfs_entry_dir) {
        CPE_ERROR(env->m_em, "test_vfs_file_open: %s is not file", path);
        return -1;
    }

    if (strchr(mode, 'w')) {
        fp->m_w_pos = 0;
    } else {
        fp->m_w_pos = -1;
    }
    
    if (strchr(mode, 'r')) {
        fp->m_r_pos = 0;
    } else {
        fp->m_r_pos = -1;
    }
    
    TAILQ_INSERT_TAIL(&fp->m_entry->m_file.m_opend_files, fp, m_next_for_entry);

    return 0;
}

static void test_vfs_file_close(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;
}

static ssize_t test_vfs_file_read(void * ctx, vfs_file_t file, void * buf, size_t size) {
    test_vfs_testenv_t env = ctx;
    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_read: entry alreay removed");
        return -1;
    }
    
    if (fp->m_r_pos < 0) {
        CPE_ERROR(env->m_em, "test_vfs_file_read: can`t read, check open mode");
        return -1;
    }

    assert(fp->m_entry->m_type == test_vfs_entry_file);

    uint32_t total_size = mem_buffer_size(&fp->m_entry->m_file.m_content);
    if (fp->m_r_pos > total_size) {
        CPE_ERROR(
            env->m_em, "test_vfs_file_read: pos %d overflow, size=%d",
            fp->m_r_pos, total_size);
        return -1;
    }
    
    if (fp->m_r_pos == total_size) return 0;
    
    uint32_t read_size = total_size - (uint32_t)fp->m_r_pos;
    if (read_size > size) read_size = size;

    struct mem_buffer_pos buf_pos;
    mem_pos_at(&buf_pos, &fp->m_entry->m_file.m_content, fp->m_r_pos);
    mem_pos_read(&buf_pos, buf, read_size);

    fp->m_r_pos += read_size;

    return (ssize_t)read_size;
}

static ssize_t test_vfs_file_write(void * ctx, vfs_file_t file, const void * buf, size_t size) {
    test_vfs_testenv_t env = ctx;
    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_write: entry alreay removed");
        return -1;
    }
    
    if (fp->m_r_pos < 0) {
        CPE_ERROR(env->m_em, "test_vfs_file_write: can`t write, check open mode");
        return -1;
    }

    assert(fp->m_entry->m_type == test_vfs_entry_file);

    uint32_t total_size = mem_buffer_size(&fp->m_entry->m_file.m_content);
    if (fp->m_r_pos > total_size) {
        CPE_ERROR(
            env->m_em, "test_vfs_file_write: pos %d overflow, size=%d",
            fp->m_r_pos, total_size);
        return -1;
    }
    
    return -1;
}

static int test_vfs_file_seek(void * ctx, vfs_file_t file, ssize_t off, vfs_file_seek_op_t op) {
    test_vfs_testenv_t env = ctx;
    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_seek: entry alreay removed");
        return -1;
    }
    
    ssize_t new_pos;

    assert(fp->m_entry->m_type == test_vfs_entry_file);
    uint32_t total_size = mem_buffer_size(&fp->m_entry->m_file.m_content);

    switch(op) {
    case vfs_file_seek_set:
        new_pos = off;
        break;
    case vfs_file_seek_cur:
        new_pos = fp->m_r_pos + off;
        break;
    case vfs_file_seek_end:
        new_pos = (ssize_t)total_size + off;
        break;
    default:
        CPE_ERROR(env->m_em, "test_vfs_file_seek: unknown seek op %d", op);
        return -1;
    }

    if (new_pos < 0 || new_pos > ((ssize_t)total_size)) {
        CPE_ERROR(env->m_em, "test_vfs_file_seek: pos %d overflow, size=%d", (int)new_pos, (int)total_size);
        return -1;
    }

    fp->m_r_pos = new_pos;
    return 0;
}

static ssize_t test_vfs_file_tell(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;
/*     test_vfs_file_t fp = vfs_file_data(file); */
/*     return fp->m_pos; */
    return 0;
}

static uint8_t test_vfs_file_eof(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;
    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_eof: entry alreay removed");
        return -1;
    }

    assert(fp->m_entry->m_type == test_vfs_entry_file);
    uint32_t total_size = mem_buffer_size(&fp->m_entry->m_file.m_content);
    return fp->m_r_pos >= (ssize_t)total_size ? 1 : 0;
}

static int test_vfs_file_flush(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;

    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_flush: entry already removed");
        return -1;
    }

    assert(fp->m_entry->m_type == test_vfs_entry_file);
    return 0;
}

static int test_vfs_file_error(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;

    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_error: entry already removed");
        return -1;
    }

    assert(fp->m_entry->m_type == test_vfs_entry_file);
    return 0;
}

static ssize_t test_vfs_file_size(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;

    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_size: entry already removed");
        return -1;
    }
    
    assert(fp->m_entry->m_type == test_vfs_entry_file);
    return (ssize_t)mem_buffer_size(&fp->m_entry->m_file.m_content);
}

static ssize_t test_vfs_file_size_by_path(void * ctx, void * mount_env, const char * path) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t mount_root = mount_env;

    test_vfs_entry_t entry = test_vfs_entry_find_child_by_path(mount_root, path, path + strlen(path));
    if (entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_size_by_path: file %s not exist!", path);
        return -1;
    }

    if (entry->m_type != test_vfs_entry_file) {
        CPE_ERROR(env->m_em, "test_vfs_file_size_by_path: %s is not file!", path);
        return -1;
    }
    
    return (ssize_t)mem_buffer_size(&entry->m_file.m_content);
}

static void const * test_vfs_file_inline_data(void * ctx, vfs_file_t file) {
    test_vfs_testenv_t env = ctx;

    test_vfs_file_t fp = vfs_file_data(file);
    if (fp->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_file_inline_data: entry already removed");
        return NULL;
    }

    assert(fp->m_entry->m_type == test_vfs_entry_file);
    
    return mem_buffer_size(&fp->m_entry->m_file.m_content) == 0
        ? ""
        : mem_buffer_make_continuous(&fp->m_entry->m_file.m_content, 0);
}

static uint8_t test_vfs_file_exist(void * ctx, void * mount_env, const char * path) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t mount_root = mount_env;
    test_vfs_entry_t entry = test_vfs_entry_find_child_by_path(mount_root, path, path + strlen(path));
    return entry && entry->m_type == test_vfs_entry_file ? 1 : 0;
}

static int test_vfs_file_rm(void * ctx, void * mount_env, const char * path) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t mount_root = mount_env;

    test_vfs_entry_t entry = test_vfs_entry_find_child_by_path(mount_root, path, path + strlen(path));
    if (entry == NULL) return 0;

    if (entry->m_type != test_vfs_entry_file) {
        CPE_ERROR(env->m_em, "test_vfs_file_rm: %s is not file", path);
        return -1;
    }

    test_vfs_entry_free(entry);
    return 0;
}

static int test_vfs_file_mv(void * ctx, void * from_env, const char * from_path, void * to_env, const char * to_path) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t from_root = from_env;
    test_vfs_entry_t to_root = to_env;

    test_vfs_entry_t from_entry = test_vfs_entry_find_child_by_path(from_root, from_path, from_path + strlen(from_path));
    if (from_entry == NULL) {
        CPE_ERROR(
            env->m_em, "test_vfs_file_mv: from %s %s not eixst",
            from_root->m_name, from_path);
        return -1;
    }

    test_vfs_entry_t to_dir = NULL;
    const char * to_name = NULL;
    
    const char * to_path_last_sep = strrchr(to_path, '/');
    if (to_path_last_sep) {
        to_dir = test_vfs_entry_find_child_by_path(to_root, to_path, to_path_last_sep);
        if (to_dir == NULL) {
            CPE_ERROR(
                env->m_em, "test_vfs_file_mv: to %s %.*s not eixst",
                to_root->m_name, (int)(to_path_last_sep - to_path), to_path);
            return -1;
        }

        if (to_dir->m_type != test_vfs_entry_dir) {
            CPE_ERROR(
                env->m_em, "test_vfs_file_mv: to %s %.*s is not dir",
                to_root->m_name, (int)(to_path_last_sep - to_path), to_path);
            return -1;
        }
    }
    else {
        to_dir = to_root;
        to_name = to_path;
    }

    if (test_vfs_entry_find_child_by_name(to_dir, to_name, to_name + strlen(to_name)) != NULL) {
        CPE_ERROR(
            env->m_em, "test_vfs_file_mv: to %s %.*s already exists",
            to_root->m_name, (int)(to_path_last_sep - to_path), to_path);
        return -1;
    }

    test_vfs_entry_set_name(from_entry, to_name);
    test_vfs_entry_set_parent(from_entry, to_dir);
    
    return 0;
}

static int test_vfs_file_set_attributes(void * ctx, void * mount_env, const char * path, uint16_t fa) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t mount_root = mount_env;

    test_vfs_entry_t entry = test_vfs_entry_find_child_by_path(mount_root, path, path + strlen(path));
    if (entry == NULL) return 0;

    if (entry->m_type != test_vfs_entry_file) {
        CPE_ERROR(env->m_em, "test_vfs_file_set_attributes: %s is not file", path);
        return -1;
    }
    
    CPE_ERROR(env->m_em, "test_vfs_file_set_attributes: not support");
    return -1;
}

/*dir*/
void test_vfs_dir_cleaar_entry(test_vfs_dir_t f) {
    if (f->m_entry) {
        TAILQ_REMOVE(&f->m_entry->m_dir.m_opend_dirs, f, m_next_for_entry);
        f->m_entry = NULL;
    }
}

static int test_vfs_dir_open(void * ctx, void * mount_env, vfs_dir_t dir, const char * path) {
    test_vfs_testenv_t env = ctx;
    test_vfs_entry_t mount_root = mount_env;
    
    test_vfs_dir_t vfs_dir = vfs_dir_data(dir);

    vfs_dir->m_entry = test_vfs_entry_find_child_by_path(mount_root, path, path + strlen(path));
    if (vfs_dir->m_entry == NULL) {
        CPE_ERROR(env->m_em, "test_vfs_dir_open: %s %s not exist", mount_root->m_name, path);
        return -1;
    }

    if (vfs_dir->m_entry->m_type != test_vfs_entry_dir) {
        CPE_ERROR(env->m_em, "test_vfs_dir_open: %s %s is not dir", mount_root->m_name, path);
        return -1;
    }

    vfs_dir->m_next_child = TAILQ_FIRST(&vfs_dir->m_entry->m_dir.m_childs);

    TAILQ_INSERT_TAIL(&vfs_dir->m_entry->m_dir.m_opend_dirs, vfs_dir, m_next_for_entry);
    
    return 0;
}

static void test_vfs_dir_close(void * ctx, vfs_dir_t dir) {
}

/* struct test_vfs_dir_it_data { */
/*     test_vfs_entry_t m_cur; */
/*     struct vfs_entry_info m_entry; */
/* }; */

/* static vfs_entry_info_t test_vfs_dir_it_next(struct vfs_entry_info_it * it) { */
/*     struct test_vfs_dir_it_data * it_data = (void*)it->m_data; */

/*     if (it_data->m_cur == NULL) return NULL; */

/*     it_data->m_entry.m_name = it_data->m_cur->m_name; */
/*     it_data->m_entry.m_type = it_data->m_cur->m_is_dir ? vfs_entry_dir : vfs_entry_file; */

/*     it_data->m_cur = TAILQ_NEXT(it_data->m_cur, m_next); */
    
/*     return &it_data->m_entry; */
/* } */

static void test_vfs_dir_read(void * ctx, vfs_dir_t dir, vfs_entry_info_it_t it) {
/*     struct test_vfs_dir_it_data * it_data = (void*)it->m_data; */
/*     test_vfs_dir_t rfs_dir = vfs_dir_data(dir); */

/*     it->next = test_vfs_dir_it_next; */
/*     it_data->m_cur = TAILQ_FIRST(&rfs_dir->m_entry->m_dir.m_childs); */
}

static uint8_t test_vfs_dir_exist(void * ctx, void * env, const char * path) {
/*     test_vfs_t rfs = env; */
/*     test_vfs_entry_t entry; */

/*     entry = test_vfs_entry_find_child_by_path(rfs->m_root, path, path + strlen(path)); */
/*     return entry && entry->m_is_dir ? 1 : 0; */
    return 0;
}

static int test_vfs_dir_rm(void * ctx, void * env, const char * path, uint8_t is_recursive) {
/*     test_vfs_backend_t backend = ctx; */
/*     CPE_ERROR(backend->m_em, "test_vfs_dir_rm: rfs not support dir rm"); */
    return -1;
}

static int test_vfs_dir_mk(void * ctx, void * env, const char * path, uint8_t is_recursive) {
/*     test_vfs_t rfs = env; */
/*     CPE_ERROR(rfs->m_em, "test_vfs_dir_mk: rfs not support create dir"); */
    return -1;
}

vfs_backend_t
vfs_testenv_create_backend(test_vfs_testenv_t env) {
    vfs_backend_t backend =
        vfs_backend_create(
            env->m_mgr, "test-fs:",
            /*ctx*/
            env, NULL,
            /*env*/
            NULL,
            /*file*/
            sizeof(struct test_vfs_file),
            test_vfs_file_open, test_vfs_file_close,
            test_vfs_file_read, test_vfs_file_write, test_vfs_file_flush,
            test_vfs_file_seek, test_vfs_file_tell, test_vfs_file_eof,
            test_vfs_file_error,
            test_vfs_file_size,
            test_vfs_file_size_by_path,
            test_vfs_file_inline_data,
            test_vfs_file_exist,
            test_vfs_file_rm,
            test_vfs_file_mv,
            test_vfs_file_set_attributes,
            /*dir*/
            sizeof(struct test_vfs_dir),
            test_vfs_dir_open, test_vfs_dir_close,
            test_vfs_dir_read,
            test_vfs_dir_exist,
            test_vfs_dir_rm,
            test_vfs_dir_mk);
    if (backend == NULL) return NULL;

    return backend;
}
