#include <assert.h>
#include "cpe/pal/pal_errno.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_stream.h"
#include "zip_unzip_file_i.h"
#include "zip_unzip_dir_i.h"

struct cpe_unzip_extract_ctx {
    error_monitor_t m_em;
    vfs_mgr_t m_vfs;
    const char * m_path;
    uint8_t m_debug;
    uint8_t m_have_error;
};

dir_visit_next_op_t
cpe_unzip_extract_on_enter(const char * full, cpe_unzip_dir_t d, void * i_ctx) {
    struct cpe_unzip_extract_ctx * ctx = i_ctx;

    char output_path[256];
    snprintf(output_path, sizeof(output_path), "%s/%s", ctx->m_path, full);

    if (vfs_dir_mk(ctx->m_vfs, output_path) != 0) {
        CPE_ERROR(ctx->m_em, "zip: extract: create dir %s fail, errno=%d (%s)", output_path, errno, strerror(errno));
        ctx->m_have_error = 1;
        return dir_visit_next_exit;
    }

    if (ctx->m_debug) {
        CPE_INFO(ctx->m_em, "zip: extract: dir %s: created", output_path);
    }
    
    return dir_visit_next_go;
}

dir_visit_next_op_t
cpe_unzip_extract_on_leave(const char * full, cpe_unzip_dir_t d, void * ctx) {
    return dir_visit_next_go;
}

dir_visit_next_op_t
cpe_unzip_extract_on_file(const char * full, cpe_unzip_file_t f, void * i_ctx) {
    struct cpe_unzip_extract_ctx * ctx = i_ctx;

    char output_path[256];
    snprintf(output_path, sizeof(output_path), "%s/%s", ctx->m_path, full);

    vfs_file_t of = vfs_file_open(ctx->m_vfs, output_path, "wb+");
    if (of == NULL) {
        CPE_ERROR(
            ctx->m_em, "zip: extract: create file %s fail, errno=%d (%s)",
            output_path, errno, strerror(errno));
        ctx->m_have_error = 1;
        return dir_visit_next_exit;
    }
    uint16_t file_fa = cpe_unzip_file_fa(f) >> 16;

    if (vfs_file_set_attributes(ctx->m_vfs, output_path, file_fa) != 0) {
        CPE_ERROR(
            ctx->m_em, "zip: extract: set file %s attrigutes fail, errno=%d (%s)",
            output_path, errno, strerror(errno));
        ctx->m_have_error = 1;
        return dir_visit_next_exit;
    }
    
    struct vfs_write_stream ws = VFS_WRITE_STREAM_INITIALIZER(of);
    
    ssize_t sz = cpe_unzip_file_load_to_stream((write_stream_t)&ws, f, ctx->m_em);
    if (sz < 0) {
        CPE_ERROR(
            ctx->m_em, "zip: extract: write fail %s fail, errno=%d (%s)",
            output_path, errno, strerror(errno));
        vfs_file_close(of);
        ctx->m_have_error = 1;
        return dir_visit_next_exit;
    }
    vfs_file_close(of);

    if (ctx->m_debug) {
        char fa_buf[64];
        struct write_stream_mem ws = CPE_WRITE_STREAM_MEM_INITIALIZER(fa_buf, sizeof(fa_buf));
        file_attribute_print((write_stream_t)&ws, cpe_unzip_file_fa(f) >> 16);
        stream_putc((write_stream_t)&ws, 0);

        CPE_INFO(
            ctx->m_em, "zip: extract: save file %s success, size=%d, fa=%s",
            output_path, (int)sz, fa_buf);
    }

    return dir_visit_next_go;
}

int cpe_unzip_dir_extract_to(
    vfs_mgr_t vfs, const char * path, cpe_unzip_dir_t from,
    uint8_t debug, error_monitor_t em, mem_allocrator_t talloc)
{
    if (vfs_dir_mk_recursion(vfs, path) != 0) {
        CPE_ERROR(em, "zip: extract: create target dir %s fail, errno=%d (%s)", path, errno, strerror(errno));
        return -1;
    }

    struct cpe_unzip_extract_ctx ctx;
    ctx.m_em = em;
    ctx.m_vfs = vfs;
    ctx.m_path = path;
    ctx.m_debug = debug;
    ctx.m_have_error = 0;

    struct cpe_unzip_file_visitor visitor;
    visitor.on_dir_enter = cpe_unzip_extract_on_enter;
    visitor.on_dir_leave = cpe_unzip_extract_on_leave;
    visitor.on_file = cpe_unzip_extract_on_file;
    cpe_unzip_dir_search(&visitor, &ctx, from, 100, em, talloc);

    return ctx.m_have_error ? -1 : 0;
}

int cpe_unzip_extract_to(
    vfs_mgr_t vfs, const char * path, const char * from,
    uint8_t debug, error_monitor_t em, mem_allocrator_t talloc)
{
    cpe_unzip_context_t context = cpe_unzip_context_create(vfs, from, talloc, em);
    if (context == NULL) {
        CPE_ERROR(em, "zip: extract: create unzip context of %s fail, errno=%d (%s)", from, errno, strerror(errno));
        return -1;
    }

    assert(context->m_root);

    if (debug) {
        CPE_INFO(em, "zip: extract: open input %s success", from);
    }

    int rv = cpe_unzip_dir_extract_to(vfs, path, context->m_root, debug, em, talloc);

    cpe_unzip_context_free(context);

    return rv;
}
