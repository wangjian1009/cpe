#include "cpe/pal/pal_string.h"
#include "zip_unzip_context_i.h"
#include "zip_unzip_file_i.h"
#include "zip_unzip_dir_i.h"

int cpe_unzip_context_build(cpe_unzip_context_t context, error_monitor_t em);

static cpe_unzip_dir_t cpe_unzip_dir_find_or_create(cpe_unzip_dir_t parent, const char * name, error_monitor_t em) {
    cpe_unzip_dir_t r;

    TAILQ_FOREACH(r, &parent->m_child_dirs, m_next_dir) {
        if (strcmp(r->m_name, name) == 0) return r;
    }

    return cpe_unzip_dir_create(parent->m_context, parent, name, em);
}


cpe_unzip_context_t
cpe_unzip_context_create(vfs_mgr_t vfs, const char * path, mem_allocrator_t alloc, error_monitor_t em) {
    cpe_unzip_context_t r;
    int rv;

    r = (cpe_unzip_context_t)mem_alloc(alloc, sizeof(struct cpe_unzip_context));
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_context_create: malloc fail!");
        return NULL;
    }

    r->m_zip_file = cpe_unzOpen64(em, vfs, path);
    if (r->m_zip_file == NULL) {
        CPE_ERROR(em, "cpe_unzip_context_create: open zip file %s fail!", path);
        mem_free(alloc, r);
        return NULL;
    }

    rv = cpe_unzGetGlobalInfo64(r->m_zip_file, &r->m_global_info);
    if (rv != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_context_create: zip file %s: get global info fail!", path);
        cpe_unzClose(r->m_zip_file);
        mem_free(alloc, r);
        return NULL;
    }

    r->m_alloc = alloc;
    r->m_root = NULL;

    if (cpe_unzip_context_build(r, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_context_create: zip file %s: build zip file info fail!", path);
        cpe_unzip_context_free(r);
        return NULL;
    }

    return r;
}

void cpe_unzip_context_free(cpe_unzip_context_t unzc) {
    if (unzc->m_root) cpe_unzip_dir_free(unzc->m_root);
    cpe_unzClose(unzc->m_zip_file);
    mem_free(unzc->m_alloc, unzc);
}

int cpe_unzip_context_build(cpe_unzip_context_t context, error_monitor_t em) {
    uLong i;
    int err;

    if (context->m_root != NULL) return 0;

    context->m_root = cpe_unzip_dir_create(context, NULL, "", em);
    if (context->m_root == NULL) {
        CPE_ERROR(em, "cpe_unzip_context_build: create root dir fail");
        return -1;
    }

    err = cpe_unzGoToFirstFile(context->m_zip_file);
    if (err != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_context_build: unzGoToFirstFile error, error=%d", err);
        return -1;
    }

    for (i = 0; i < context->m_global_info.number_entry; i++) {
        char filename_inzip[256];
        unz_file_info64 file_info;
        cpe_unzip_dir_t cur_dir;
        char * start;
        char * end;

        err = cpe_unzGetCurrentFileInfo64(context->m_zip_file, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err != UNZ_OK) {
            CPE_ERROR(em, "cpe_unzip_context_build: unzGetCurrentFileInfo error, error=%d", err);
            return -1;
        }

        cur_dir = context->m_root;
        start = filename_inzip;
        while((end = strchr(start, '/'))) {
            *end = 0;
            cur_dir = cpe_unzip_dir_find_or_create(cur_dir, start, em);
            if (cur_dir == NULL) {
                CPE_ERROR(em, "cpe_unzip_context_build: create dir fail");
                return -1;
            }
            start = end + 1;
        }

        if (start[0] != 0) {
            if (cpe_unzip_file_create(cur_dir, start, &file_info, em) == NULL) {
                CPE_ERROR(em, "cpe_unzip_context_build: create file error, error=%d", err);
                return -1;
            }
        }

        if ((i + 1) < context->m_global_info.number_entry) {
            err = cpe_unzGoToNextFile(context->m_zip_file);
            if (err != UNZ_OK) {
                CPE_ERROR(em, "cpe_unzip_context_build: unzGoToNextFile error, error=%d",err);
                return -1;
            }
        }
    }

    return 0;
}
