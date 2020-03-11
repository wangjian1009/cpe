#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "zip_unzip_file_i.h"
#include "zip_unzip_dir_i.h"

cpe_unzip_file_t
cpe_unzip_file_create(cpe_unzip_dir_t parent, const char * name, unz_file_info64 * file_info, error_monitor_t em) {
    cpe_unzip_file_t r;

    r = mem_alloc(parent->m_context->m_alloc, sizeof(struct cpe_unzip_file));
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_file_create: malloc fail");
        return NULL;
    }

    r->m_context = parent->m_context;
    r->m_parent_dir = parent;
    cpe_str_dup(r->m_name, sizeof(r->m_name), name);
    memcpy(&r->m_file_info, file_info, sizeof(r->m_file_info));

    TAILQ_INSERT_TAIL(&parent->m_child_files, r, m_next_file);

    return r;
}

void cpe_unzip_file_free(cpe_unzip_file_t f) {
    assert(f);
    assert(f->m_parent_dir);

    TAILQ_REMOVE(&f->m_parent_dir->m_child_files, f, m_next_file);

    mem_free(f->m_context->m_alloc, f);
}

cpe_unzip_file_t
cpe_unzip_file_find(cpe_unzip_context_t context, const char * file, error_monitor_t em) {
    const char * start;
    const char * end;
    cpe_unzip_dir_t cur_dir;
    cpe_unzip_file_t f;

    assert(context);
    assert(context->m_root);

    cur_dir = context->m_root;

    start = file;
    while((end = strchr(start, '/'))) {
        cpe_unzip_dir_t c;

        TAILQ_FOREACH(c, &cur_dir->m_child_dirs, m_next_dir) {
            size_t len = end - start;
            if (memcmp(c->m_name, start, len) == 0 && c->m_name[len] == 0) {
                break;
            }
        }

        if (c == TAILQ_END(&cur_dir->m_child_dirs)) return NULL;
        
        cur_dir = c;
        start = end + 1;
    }

    TAILQ_FOREACH(f, &cur_dir->m_child_files, m_next_file) {
        if (strcmp(f->m_name, start) == 0) return f;
    }

    return NULL;
}

const char * cpe_unzip_file_name(cpe_unzip_file_t zf) {
    return zf->m_name;
}

uint32_t cpe_unzip_file_fa(cpe_unzip_file_t zf) {
    return zf->m_file_info.external_fa;
}

const char * cpe_unzip_file_path(mem_buffer_t buffer, cpe_unzip_file_t zf) {
    if (zf->m_parent_dir && zf->m_parent_dir != zf->m_context->m_root) {
        cpe_unzip_dir_build_path(buffer, zf->m_parent_dir);
        mem_buffer_strcat(buffer, "/");
    }

    mem_buffer_strcat(buffer, zf->m_name);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

ssize_t cpe_unzip_file_load_to_buffer(mem_buffer_t buffer, cpe_unzip_file_t zf, error_monitor_t em) {
    ssize_t len;
    size_t start;
    char * buf;
    int r;

    assert(buffer);

    len = zf->m_file_info.uncompressed_size;
    start = mem_buffer_size(buffer);
    mem_buffer_set_size(buffer, start + len);

    buf = mem_buffer_make_continuous(buffer, 0);

    if (buf == NULL) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buffer: alloc buff fail!");
        return -1;
    }

    if (cpe_unzip_file_open(zf, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buffer: open zip file fail!");
        return -1;
    }

    mem_buffer_set_size(buffer, len + start);

    assert(buf == mem_buffer_make_continuous(buffer, 0));

    r = cpe_unzReadCurrentFile(zf->m_context->m_zip_file, buf + start, (unsigned)len);
    assert(r == len);
    
    return len;
}

ssize_t cpe_unzip_file_load_to_buf(char * buf, size_t size, cpe_unzip_file_t zf, error_monitor_t em) {
    ssize_t read_size;

    if (cpe_unzip_file_open(zf, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: open zip file fail!");
        return -1;
    }

    read_size = cpe_unzReadCurrentFile(zf->m_context->m_zip_file, buf, (unsigned)size);
    if (read_size < 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: read fail!");
        cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
        return -1;
    }

    cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
    return read_size;
}

ssize_t cpe_unzip_file_load_to_stream(write_stream_t ws, cpe_unzip_file_t zf, error_monitor_t em) {
    if (cpe_unzip_file_open(zf, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_stream: open zip file fail!");
        return -1;
    }

    ssize_t read_size = 0;
    
    char read_block[1024];
    ssize_t block_read_size;
    do {
        block_read_size = cpe_unzReadCurrentFile(zf->m_context->m_zip_file, read_block, sizeof(read_block));
        if (block_read_size < 0) {
            CPE_ERROR(em, "cpe_unzip_file_load_to_stream: read fail!");
            cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
            return -1;
        }

        if (stream_write(ws, read_block, block_read_size) != block_read_size) {
            CPE_ERROR(em, "cpe_unzip_file_load_to_stream: write to stream fail, size=%d!", (int)block_read_size);
            cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
            return -1;
        }
        read_size += block_read_size;
    } while (block_read_size > 0);

    cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
    return read_size;
}

int cpe_unzip_file_open(cpe_unzip_file_t zf, error_monitor_t em) {
    struct mem_buffer name_buffer;
    const char * path;
    int rv;

    mem_buffer_init(&name_buffer, zf->m_context->m_alloc);

    path = cpe_unzip_file_path(&name_buffer, zf);
    if (path == NULL) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: make path fail!");
        mem_buffer_clear(&name_buffer);
        return -1;
    }

    rv = cpe_unzLocateFile(zf->m_context->m_zip_file, path, 0);
    if (rv != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: zip file %s: locate file fail!", path);
        mem_buffer_clear(&name_buffer);
        return -1;
    }

    rv = cpe_unzOpenCurrentFile(zf->m_context->m_zip_file);
    if (rv != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: zip file %s: open zip fail!", path);
        mem_buffer_clear(&name_buffer);
        return -1;
    }

    mem_buffer_clear(&name_buffer);
    return 0;
}

