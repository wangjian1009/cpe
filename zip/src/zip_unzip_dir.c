#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "zip_unzip_dir_i.h"
#include "zip_unzip_file_i.h"

cpe_unzip_dir_t
cpe_unzip_dir_create(cpe_unzip_context_t context, cpe_unzip_dir_t parent, const char * name, error_monitor_t em) {
    cpe_unzip_dir_t r;

    r = mem_alloc(context->m_alloc, sizeof(struct cpe_unzip_dir));
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_dir_create: malloc fail");
        return NULL;
    }

    r->m_context = context;
    r->m_parent_dir = parent;
    TAILQ_INIT(&r->m_child_dirs);
    TAILQ_INIT(&r->m_child_files);
    cpe_str_dup(r->m_name, sizeof(r->m_name), name);

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_child_dirs, r, m_next_dir);
    }

    return r;
}

void cpe_unzip_dir_free(cpe_unzip_dir_t d) {
    while(!TAILQ_EMPTY(&d->m_child_files)) {
        cpe_unzip_file_free(TAILQ_FIRST(&d->m_child_files));
    }

    while(!TAILQ_EMPTY(&d->m_child_dirs)) {
        cpe_unzip_dir_free(TAILQ_FIRST(&d->m_child_dirs));
    }

    if (d->m_parent_dir) {
        TAILQ_REMOVE(&d->m_parent_dir->m_child_dirs, d, m_next_dir);
    }
    
    mem_free(d->m_context->m_alloc, d);
}

void cpe_unzip_dir_build_path(mem_buffer_t buffer, cpe_unzip_dir_t d) {
    if (d->m_parent_dir && d->m_parent_dir != d->m_context->m_root) {
        cpe_unzip_dir_build_path(buffer, d->m_parent_dir);
        mem_buffer_strcat(buffer, "/");
    }

    mem_buffer_strcat(buffer, d->m_name);
}

const char * cpe_unzip_dir_name(cpe_unzip_dir_t d) {
    return d->m_name;
}

const char * cpe_unzip_dir_path(mem_buffer_t buffer, cpe_unzip_dir_t d) {
    cpe_unzip_dir_build_path(buffer, d);
    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

cpe_unzip_dir_t
cpe_unzip_dir_find(cpe_unzip_context_t context, const char * path, error_monitor_t em) {
    const char * start;
    const char * end;
    cpe_unzip_dir_t cur_dir;
    cpe_unzip_dir_t f;

    assert(path);
    assert(context);
    assert(context->m_root);

    cur_dir = context->m_root;

    start = path;
    while((end = strchr(start, '/'))) {
        cpe_unzip_dir_t c;

        TAILQ_FOREACH(c, &cur_dir->m_child_dirs, m_next_dir) {
            size_t len = end - start;
            size_t name_len = strlen(c->m_name);
            if (name_len == len && memcmp(c->m_name, start, len) == 0) {
                break;
            }
        }

        if (c == TAILQ_END(&cur_dir->m_child_dirs)) return NULL;
        
        cur_dir = c;
        start = end + 1;
    }

    assert(start);
    if (start[0] == 0) {
        return cur_dir;
    }

    TAILQ_FOREACH(f, &cur_dir->m_child_dirs, m_next_dir) {
        if (strcmp(f->m_name, start) == 0) return f;
    }

    return NULL;
}
