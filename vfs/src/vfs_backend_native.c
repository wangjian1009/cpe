#include <sys/stat.h>
#include <errno.h>
#include "cpe/pal/pal_dirent.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_limits.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_entry_info.h"
#include "vfs_backend_i.h"
#include "vfs_file_i.h"
#include "vfs_dir_i.h"

struct vfs_backend_native_dir_data {
    char * m_path;
    DIR * m_dirp;
};

extern DIR * dir_open(const char * path, int ignoreError, error_monitor_t em);
extern void dir_close(DIR * dirp, error_monitor_t em);
extern int inode_stat_by_fileno(int fno, struct stat * buf, int ignoreError, error_monitor_t em);

static const char * vfs_backend_make_path(vfs_mgr_t mgr, void * env, const char * path) {
    if (env) {
        mem_buffer_clear_data(&mgr->m_tmp_buffer);
        mem_buffer_strcat(&mgr->m_tmp_buffer, (const char *)env);
        mem_buffer_strcat(&mgr->m_tmp_buffer, "/");
        mem_buffer_strcat(&mgr->m_tmp_buffer, path);
        return (const char *)mem_buffer_make_continuous(&mgr->m_tmp_buffer, 0);
    }
    else {
        return path;
    }
}

static void vfs_backend_env_clear(void * ctx, void * env) {
    vfs_mgr_t mgr = ctx;
    mem_free(mgr->m_alloc, env);
}

static int vfs_native_file_open(void * ctx, void * env, vfs_file_t file, const char * path, const char * mode) {
    vfs_mgr_t mgr = ctx;
    FILE * fp = file_stream_open(vfs_backend_make_path(mgr, env, path), mode, mgr->m_em);
    if (fp == NULL) return -1;

    *(FILE **)vfs_file_data(file) = fp;
        
    return 0;
}

static void vfs_native_file_close(void * ctx, vfs_file_t file) {
    vfs_mgr_t mgr = ctx;
    FILE * fp = *(FILE **)vfs_file_data(file);
    file_stream_close(fp, mgr->m_em);
}

static ssize_t vfs_native_file_read(void * ctx, vfs_file_t file, void * buf, size_t size) {
    vfs_mgr_t mgr = ctx;
    FILE * fp = *(FILE **)vfs_file_data(file);
    ssize_t readed_size = 0;
    
    readed_size = 0;
    while(readed_size < (ssize_t)size) {
        size_t rv = fread(((char*)buf) + readed_size, 1, size - readed_size, fp);
        if (rv == 0) break;
        readed_size += rv;
    }

    if (ferror(fp)) {
        CPE_ERROR(mgr->m_em, "vfs_native_file_read: read data fail, errno=%d (%s)", errno, strerror(errno));
        return -1;
    }

    return readed_size;
}

static ssize_t vfs_native_file_write(void * ctx, vfs_file_t file, const void * buf, size_t size) {
    vfs_mgr_t mgr = ctx;
    FILE * fp = *(FILE **)vfs_file_data(file);
    ssize_t writed_size;

    writed_size = 0;
    while(writed_size < (ssize_t)size) {
        size_t rv = fwrite(((char*)buf) + writed_size, 1, size - writed_size, fp);
        if (rv == 0) break;
        
        writed_size += rv;
    }

    if (ferror(fp)) {
        CPE_ERROR(mgr->m_em, "vfs_native_file_read: write data fail, errno=%d (%s)", errno, strerror(errno));
        return -1;
    }
    else {
        return writed_size;
    }
}

static int vfs_native_file_seek(void * ctx, vfs_file_t file, ssize_t off, vfs_file_seek_op_t op) {
    vfs_mgr_t mgr = ctx;
    FILE * fp = *(FILE **)vfs_file_data(file);

    switch(op) {
    case vfs_file_seek_set:
        return (int)fseek(fp, off, SEEK_SET);
    case vfs_file_seek_cur:
        return (int)fseek(fp, off, SEEK_CUR);
    case vfs_file_seek_end:
        return (int)fseek(fp, off, SEEK_END);
    default:
        CPE_ERROR(mgr->m_em, "vfs_native_file_seek: unknown seek op %d", op);
        return -1;
    }
}

static ssize_t vfs_native_file_tell(void * ctx, vfs_file_t file) {
    FILE * fp = *(FILE **)vfs_file_data(file);
    return (ssize_t)ftell(fp);
}

static uint8_t vfs_native_file_eof(void * ctx, vfs_file_t file) {
    FILE * fp = *(FILE **)vfs_file_data(file);
    return feof(fp) ? 1 : 0;
}

static int vfs_native_file_flush(void * ctx, vfs_file_t file) {
    FILE * fp = *(FILE **)vfs_file_data(file);
    return (int)fflush(fp);
}

static int vfs_native_file_error(void * ctx, vfs_file_t file) {
    FILE * fp = *(FILE **)vfs_file_data(file);
    return ferror(fp);
}

static ssize_t vfs_native_file_size(void * ctx, vfs_file_t file) {
    vfs_mgr_t mgr = ctx;

    FILE * fp = *(FILE **)vfs_file_data(file);
    struct stat buffer;
    int status;
    status = inode_stat_by_fileno(fileno(fp), &buffer, 0, mgr->m_em);
    if (status != 0) {
        return -1;
    }

    return (ssize_t)buffer.st_size;
}

static ssize_t vfs_native_file_size_by_path(void * ctx, void * env, const char * path) {
    vfs_mgr_t mgr = ctx;
    return file_size(vfs_backend_make_path(mgr, env, path), mgr->m_em);
}

static uint8_t vfs_native_file_exist(void * ctx, void * env, const char * path) {
    vfs_mgr_t mgr = ctx;
    return (uint8_t)file_exist(vfs_backend_make_path(mgr, env, path), mgr->m_em);
}

static int vfs_native_file_rm(void * ctx, void * env, const char * path) {
    vfs_mgr_t mgr = ctx;
    return file_rm(vfs_backend_make_path(mgr, env, path), mgr->m_em);
}

static int vfs_native_file_set_fa(void * ctx, void * env, const char * path, uint16_t fa) {
    vfs_mgr_t mgr = ctx;
    return file_set_fa(vfs_backend_make_path(mgr, env, path), fa, mgr->m_em);
}

static int vfs_native_file_mv(void * ctx, void * from_env, const char * from_path, void * to_env, const char * to_path) {
    vfs_mgr_t mgr = ctx;
    
    char from_path_full[PATH_MAX];
    cpe_str_dup(from_path_full, sizeof(from_path_full), vfs_backend_make_path(mgr, from_env, from_path));
    
    const char * to_path_full = vfs_backend_make_path(mgr, to_env, to_path);

    return file_mv(to_path_full, from_path_full, mgr->m_em);
}

static int vfs_native_dir_open(void * ctx, void * env, vfs_dir_t dir, const char * path) {
    vfs_mgr_t mgr = ctx;
    struct vfs_backend_native_dir_data * dir_data = vfs_dir_data(dir);
    const char * full_path = vfs_backend_make_path(mgr, env, path);
    
    DIR * dirp = dir_open(full_path, 0, mgr->m_em);
    if (dirp == NULL) return -1;

    dir_data->m_dirp = dirp;
    dir_data->m_path = cpe_str_mem_dup(mgr->m_alloc, full_path);
    return 0;
}

static void vfs_native_dir_close(void * ctx, vfs_dir_t dir) {
    vfs_mgr_t mgr = ctx;
    struct vfs_backend_native_dir_data * dir_data = vfs_dir_data(dir);

    dir_close(dir_data->m_dirp, mgr->m_em);
    mem_free(mgr->m_alloc, dir_data->m_path);
}

struct vfs_native_dir_it_data {
    vfs_mgr_t m_mgr;
    DIR * m_dirp;
    char * m_path;    
    struct vfs_entry_info m_entry;
};

static vfs_entry_info_t vfs_native_dir_it_next(struct vfs_entry_info_it * it) {
    struct vfs_native_dir_it_data * it_data = (void*)it->m_data;
    struct dirent * dp;
    
    while((dp = readdir(it_data->m_dirp))) {
        if (dp->d_type == DT_UNKNOWN) {
            const char * d_path_full = vfs_backend_make_path(it_data->m_mgr, it_data->m_path, dp->d_name);
            struct stat stat_buf;
            if (stat(d_path_full, &stat_buf) < 0) continue;

            if (S_ISDIR(stat_buf.st_mode)) {
                dp->d_type = DT_DIR;
            }
            else if (S_ISREG(stat_buf.st_mode)) {
                dp->d_type = DT_REG;
            }
            else if (S_ISBLK(stat_buf.st_mode)) {
                dp->d_type = DT_BLK;
            }
            else if (S_ISCHR(stat_buf.st_mode)) {
                dp->d_type = DT_CHR;
            }
            else if (S_ISFIFO(stat_buf.st_mode)) {
                dp->d_type = DT_FIFO;
            }
            else if (S_ISLNK(stat_buf.st_mode)) {
                dp->d_type = DT_LNK;
            }
            else if (S_ISSOCK(stat_buf.st_mode)) {
                dp->d_type = DT_SOCK;
            }
#if defined S_ISWHT
            else if (S_ISWHT(stat_buf.st_mode)) {
                dp->d_type = DT_WHT;
            }
#endif
            else {
                continue;
            }
        }

        if (dp->d_type != DT_UNKNOWN) break;
    }
    if (dp == NULL) return NULL;

    it_data->m_entry.m_name = dp->d_name;
    it_data->m_entry.m_type = dp->d_type == DT_DIR ? vfs_entry_dir : vfs_entry_file;
    return &it_data->m_entry;
}

static void vfs_native_dir_read(void * ctx, vfs_dir_t dir, vfs_entry_info_it_t it) {
    vfs_mgr_t mgr = ctx;
    struct vfs_native_dir_it_data * it_data = (void*)it->m_data;
    struct vfs_backend_native_dir_data * dir_data = vfs_dir_data(dir);

    //typedef char check_it_data_size[sizeof(it->m_data) < sizeof(struct vfs_native_dir_it_data) ? -1 : 1];

    it->next = vfs_native_dir_it_next;
    it_data->m_mgr = mgr;
    it_data->m_dirp = dir_data->m_dirp;
    it_data->m_path = dir_data->m_path;
}

static uint8_t vfs_native_dir_exist(void * ctx, void * env, const char * path) {
    vfs_mgr_t mgr = ctx;
    return (uint8_t)dir_exist(vfs_backend_make_path(mgr, env, path), mgr->m_em);
}

static int vfs_native_dir_rm(void * ctx, void * env, const char * path, uint8_t is_recursive) {
    vfs_mgr_t mgr = ctx;
    if (is_recursive) {
        return dir_rm_recursion(vfs_backend_make_path(mgr, env, path), mgr->m_em, mgr->m_alloc);
    }
    else {
        return dir_rm(vfs_backend_make_path(mgr, env, path), mgr->m_em);
    }
}

static int vfs_native_dir_mk(void * ctx, void * env, const char * path, uint8_t is_recursive) {
    vfs_mgr_t mgr = ctx;

    if (is_recursive) {
        return dir_mk_recursion(vfs_backend_make_path(mgr, env, path), DIR_DEFAULT_MODE, mgr->m_em, mgr->m_alloc);
    }
    else {
        return dir_mk(vfs_backend_make_path(mgr, env, path), DIR_DEFAULT_MODE, mgr->m_em);
    }
}

int vfs_backend_native_create(vfs_mgr_t mgr) {
    return
        vfs_backend_create(
            mgr, "native", mgr, NULL,
            /*env*/
            vfs_backend_env_clear,
            /*file*/
            sizeof(FILE*), vfs_native_file_open, vfs_native_file_close,
            vfs_native_file_read, vfs_native_file_write, vfs_native_file_flush,
            vfs_native_file_seek, vfs_native_file_tell, vfs_native_file_eof,
            vfs_native_file_error,
            vfs_native_file_size,
            vfs_native_file_size_by_path,
            NULL,
            vfs_native_file_exist,
            vfs_native_file_rm,
            vfs_native_file_mv,
            vfs_native_file_set_fa,
            /*dir*/
            sizeof(struct vfs_backend_native_dir_data), vfs_native_dir_open, vfs_native_dir_close, vfs_native_dir_read,
            vfs_native_dir_exist,
            vfs_native_dir_rm,
            vfs_native_dir_mk)
        == NULL
        ? -1
        : 0;
}
