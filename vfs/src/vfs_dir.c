#include <assert.h>
#include "cpe/vfs/vfs_entry_info.h"
#include "vfs_dir_i.h"
#include "vfs_backend_i.h"
#include "vfs_mount_point_i.h"

vfs_dir_t vfs_dir_open(vfs_mgr_t mgr, const char * input_path) {
    vfs_dir_t d;
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;
    const char * path = input_path;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_dir_open: mount point of path %s not exist!", input_path);
        return NULL;
    }

    backend = mount_point->m_backend;
    assert(backend);
    
    if (backend->m_dir_open == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_dir_open: backend %s not support open dir!", backend->m_name);
        return NULL;
    }
    
    d = mem_alloc(mgr->m_alloc, sizeof(struct vfs_dir) + backend->m_dir_capacity);
    if (d == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_dir_open: alloc fail!");
        return NULL;
    }

    d->m_mgr = mgr;
    d->m_backend = backend;
    
    if (backend->m_dir_open(backend->m_ctx, mount_point->m_backend_env, d, path) != 0) {
        mem_free(mgr->m_alloc, d);
        return NULL;
    }

    return d;
}

void vfs_dir_close(vfs_dir_t d) {
    vfs_backend_t backend = d->m_backend;

    backend->m_dir_close(backend->m_ctx, d);

    mem_free(d->m_mgr->m_alloc, d);
}

void * vfs_dir_data(vfs_dir_t d) {
    return d + 1;
}

void vfs_dir_entries(vfs_dir_t d, vfs_entry_info_it_t it) {
    vfs_backend_t backend = d->m_backend;

    if (backend->m_dir_read) {
        backend->m_dir_read(backend->m_ctx, d, it);
    }
    else {
        it->next = NULL;
    }
}

uint8_t vfs_dir_exist(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return 0;

    backend = mount_point->m_backend;
    assert(backend);

    if (path[0] == 0) return 1;
    
    return backend->m_dir_exist ? backend->m_dir_exist(backend->m_ctx, mount_point->m_backend_env, path) : 0;
}

int vfs_dir_rm(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return -1;

    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_dir_rm ? backend->m_dir_rm(backend->m_ctx, mount_point->m_backend_env, path, 0) : -1;
}

int vfs_dir_rm_recursion(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return -1;

    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_dir_rm ? backend->m_dir_rm(backend->m_ctx, mount_point->m_backend_env, path, 1) : -1;
}

int vfs_dir_mk(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return -1;

    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_dir_mk ? backend->m_dir_mk(backend->m_ctx, mount_point->m_backend_env, path, 0) : -1;
}

int vfs_dir_mk_recursion(vfs_mgr_t mgr, const char * path) {
    vfs_mount_point_t mount_point;
    vfs_backend_t backend;

    mount_point = vfs_mount_point_find_by_path(mgr, &path);
    if (mount_point == NULL) return -1;

    backend = mount_point->m_backend;
    assert(backend);

    return backend->m_dir_mk ? backend->m_dir_mk(backend->m_ctx, mount_point->m_backend_env, path, 1) : -1;
}

int vfs_dir_mv(vfs_mgr_t mgr, const char * to, const char * from) {
    const char * from_path = from;
    vfs_mount_point_t from_mount_point = vfs_mount_point_find_by_path(mgr, &from_path);
    if (from_mount_point == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_dir_mv: mount point of path %s not exist!", from_path);
        return -1;
    }

    const char * to_path = to;
    vfs_mount_point_t to_mount_point = vfs_mount_point_find_by_path(mgr, &to_path);
    if (to_mount_point == NULL) {
        CPE_ERROR(mgr->m_em, "vfs_dir_mv: mount point of path %s not exist!", to_path);
        return -1;
    }
    
    if (from_mount_point->m_backend != to_mount_point->m_backend) {
        CPE_ERROR(
            mgr->m_em, "vfs_dir_mv: can`t move file from backend %s to backend %s!",
            from_mount_point->m_backend->m_name, to_mount_point->m_backend->m_name);
        return -1;
    }

    vfs_backend_t backend = from_mount_point->m_backend;
    if (backend->m_file_mv) {
        return from_mount_point->m_backend->m_file_mv(
            backend->m_ctx, from_mount_point->m_backend_env, from_path, to_mount_point->m_backend_env, to_path);
    } else {
        CPE_ERROR(mgr->m_em, "vfs_file_mv: backend %s not support mv!", from_mount_point->m_backend->m_name);
        return -1;
    }
}
