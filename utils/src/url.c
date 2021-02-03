#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/string_url.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/url.h"

struct cpe_url_param {
    char * m_name;
    char * m_value;
};

struct cpe_url {
    mem_allocrator_t m_alloc;
    char * m_protocol;
    char * m_host;
    uint16_t m_port;
    char * m_path;

    uint16_t m_query_param_count;
    struct cpe_url_param m_query_param[8];
    uint16_t m_query_param_addition_capacity;
    struct cpe_url_param * m_query_param_addition;
};

cpe_url_t cpe_url_create(mem_allocrator_t alloc) {
    cpe_url_t url = mem_alloc(alloc, sizeof(struct cpe_url));
    if (url == NULL) return NULL;

    url->m_alloc = alloc;
    url->m_protocol = NULL;
    url->m_host = NULL;
    url->m_port = 0;
    url->m_path = NULL;
    url->m_query_param_count = 0;
    url->m_query_param_addition_capacity = 0;
    url->m_query_param_addition = NULL;
    
    return url;
}

void cpe_url_free(cpe_url_t url) {
    mem_allocrator_t alloc = url->m_alloc;

    if (url->m_protocol) {
        mem_free(alloc, url->m_protocol);
        url->m_protocol = NULL;
    }

    if (url->m_host) {
        mem_free(alloc, url->m_host);
        url->m_host = NULL;
    }

    if (url->m_path) {
        mem_free(alloc, url->m_path);
        url->m_path = NULL;
    }

    uint16_t i;
    for(i = 0; i < url->m_query_param_count; ++i) {
        if (i < CPE_ARRAY_SIZE(url->m_query_param)) {
            mem_free(alloc, url->m_query_param[i].m_name);
            mem_free(alloc, url->m_query_param[i].m_value);
        }
        else {
            uint16_t addition_pos = i - CPE_ARRAY_SIZE(url->m_query_param);
            mem_free(alloc, url->m_query_param_addition[addition_pos].m_name);
            mem_free(alloc, url->m_query_param_addition[addition_pos].m_value);
        }
    }
    url->m_query_param_count = 0;

    if (url->m_query_param_addition) {
        mem_free(alloc, url->m_query_param_addition);
        url->m_query_param_addition = NULL;
    }
    url->m_query_param_addition_capacity = 0;

    mem_free(alloc, url);
}

cpe_url_t cpe_url_clone(mem_allocrator_t alloc, cpe_url_t from) {
    cpe_url_t url = cpe_url_create(alloc);
    if (url == NULL) return NULL;

    if (from->m_protocol) {
        url->m_protocol = cpe_str_mem_dup(url->m_alloc, from->m_protocol);
        if (url->m_protocol == NULL) goto COPY_ERROR;
    }

    if (from->m_host) {
        url->m_host = cpe_str_mem_dup(url->m_alloc, from->m_host);
        if (url->m_host == NULL) goto COPY_ERROR;
    }

    url->m_port = from->m_port;

    if (from->m_path) {
        url->m_path = cpe_str_mem_dup(url->m_alloc, from->m_path);
        if (url->m_path == NULL) goto COPY_ERROR;
    }

    uint16_t i;
    for(i = 0; i < from->m_query_param_count; ++i) {
        const char * name = cpe_url_query_param_name_at(from, i);
        const char * value = cpe_url_query_param_value_at(from, i);
        if (cpe_url_query_param_add(url, name, value) != 0) goto COPY_ERROR;
    }
    
    return url;

COPY_ERROR:
    cpe_url_free(url);
    return NULL;
}

const char * cpe_url_protocol(cpe_url_t url) {
    return url->m_protocol;
}

int cpe_url_set_protocol(cpe_url_t url, const char * protocol) {
    char * new_protocol = NULL;
    if (protocol) {
        new_protocol = cpe_str_mem_dup(url->m_alloc, protocol);
        if (new_protocol == NULL) return -1;
    }

    if (url->m_protocol) {
        mem_free(url->m_alloc, url->m_protocol);
        url->m_protocol = NULL;
    }
    url->m_protocol = new_protocol;

    return 0;
}

const char * cpe_url_host(cpe_url_t url) {
    return url->m_host;
}

int cpe_url_set_host(cpe_url_t url, const char * host) {
    char * new_host = NULL;
    if (host) {
        new_host = cpe_str_mem_dup(url->m_alloc, host);
        if (new_host == NULL) return -1;
    }

    if (url->m_host) {
        mem_free(url->m_alloc, url->m_host);
        url->m_host = NULL;
    }
    url->m_host = new_host;

    return 0;
}

uint16_t cpe_url_port(cpe_url_t url) {
    return url->m_port;
}

void cpe_url_set_port(cpe_url_t url, uint16_t port) {
    url->m_port = port;
}

const char * cpe_url_path(cpe_url_t url) {
    return url->m_path;
}

int cpe_url_set_path(cpe_url_t url, const char * path) {
    char * new_path = NULL;
    if (path) {
        new_path = cpe_str_mem_dup(url->m_alloc, path);
        if (new_path == NULL) return -1;
    }

    if (url->m_path) {
        mem_free(url->m_alloc, url->m_path);
        url->m_path = NULL;
    }
    url->m_path = new_path;

    return 0;
}

uint16_t cpe_url_query_param_count(cpe_url_t url) {
    return url->m_query_param_count;
}

const char * cpe_url_query_param_name_at(cpe_url_t url, uint16_t pos) {
    assert(pos < url->m_query_param_count);

    if (pos < CPE_ARRAY_SIZE(url->m_query_param)) {
        return url->m_query_param[pos].m_name;
    }
    else {
        uint16_t addition_pos = pos - CPE_ARRAY_SIZE(url->m_query_param);
        return url->m_query_param_addition[addition_pos].m_name;
    }
}

const char * cpe_url_query_param_value_at(cpe_url_t url, uint16_t pos) {
    assert(pos < url->m_query_param_count);

    if (pos < CPE_ARRAY_SIZE(url->m_query_param)) {
        return url->m_query_param[pos].m_value;
    }
    else {
        uint16_t addition_pos = pos - CPE_ARRAY_SIZE(url->m_query_param);
        return url->m_query_param_addition[addition_pos].m_value;
    }
}

const char * cpe_url_query_param_value_find(cpe_url_t url, const char * name) {
    uint16_t i;
    for(i = 0; i < url->m_query_param_count; ++i) {
        const char * arg_name = cpe_url_query_param_name_at(url, i);
        if (cpe_str_cmp_opt(arg_name, name) == 0) return cpe_url_query_param_value_at(url, i);
    }
    
    return NULL;
}

struct cpe_url_param * cpe_url_query_param_confirm_next(cpe_url_t url) {
    if (url->m_query_param_count < CPE_ARRAY_SIZE(url->m_query_param)) {
        return &url->m_query_param[url->m_query_param_count];
    }

    uint16_t addition_pos = url->m_query_param_count - CPE_ARRAY_SIZE(url->m_query_param);

    if (addition_pos >= url->m_query_param_addition_capacity) {
        uint16_t new_capacity = url->m_query_param_addition_capacity < 8 ? 8 : url->m_query_param_addition_capacity * 2;

        struct cpe_url_param * new_addition = mem_alloc(url->m_alloc, sizeof(struct cpe_url_param) * new_capacity);
        if (new_addition == NULL) return NULL;

        if (url->m_query_param_addition_capacity > 0) {
            assert(url->m_query_param_addition);
            memcpy(
                new_addition, url->m_query_param_addition,
                sizeof(struct cpe_url_param) * url->m_query_param_addition_capacity);
        }

        if (url->m_query_param_addition) {
            mem_free(url->m_alloc, url->m_query_param_addition);
        }

        url->m_query_param_addition_capacity = new_capacity;
        url->m_query_param_addition = new_addition;
    }

    assert(addition_pos < url->m_query_param_addition_capacity);
    return &url->m_query_param_addition[addition_pos];
}

int cpe_url_query_param_add(cpe_url_t url, const char * name, const char * value) {
    struct cpe_url_param * param = cpe_url_query_param_confirm_next(url);
    if (param == NULL) return -1;

    param->m_name = cpe_str_mem_dup(url->m_alloc, name);
    if (param->m_name == NULL) return -1;

    param->m_value = cpe_str_mem_dup(url->m_alloc, value);
    if (param->m_value == NULL) {
        mem_free(url->m_alloc, param->m_name);
        return -1;
    }

    url->m_query_param_count++;
    return 0;
}

int cpe_url_cmp(cpe_url_t l, cpe_url_t r) {
    int rv;
    
    if (l->m_port != r->m_port) {
        return (int)l->m_port - (int)r->m_port;
    }

    if ((rv = cpe_str_cmp_opt(l->m_protocol, r->m_protocol)) != 0) return rv;
    if ((rv = cpe_str_cmp_opt(l->m_host, r->m_host)) != 0) return rv;
    if ((rv = cpe_str_cmp_opt(l->m_path, r->m_path)) != 0) return rv;

    if (l->m_query_param_count != r->m_query_param_count) {
        return (int)l->m_query_param_count - (int)r->m_query_param_count;
    }

    uint16_t i;
    for(i = 0; i < l->m_query_param_count; ++i) {
        const char * arg_name = cpe_url_query_param_name_at(l, i);
        const char * l_value = cpe_url_query_param_value_at(l, i);
        const char * r_value = cpe_url_query_param_value_find(r, arg_name);
        if ((rv = cpe_str_cmp_opt(l_value, r_value)) != 0) return rv;
    }
    
    return 0;
}

int cpe_url_cmp_opt(cpe_url_t l, cpe_url_t r) {
    if (l == NULL) {
        if (r == NULL) {
            return 0;
        }
        else {
            return -1;
        }
    }
    else {
        if (r == NULL) {
            return 1;
        }
        else {
            return cpe_url_cmp(l, r);
        }
    }
}

void cpe_url_print(write_stream_t ws, cpe_url_t url) {
    stream_printf(ws, "%s://", cpe_str_opt(url->m_protocol, ""));

    if (url->m_host) {
        if (url->m_port) {
            stream_printf(ws, "%s:%d", url->m_host, url->m_port);
        }
        else {
            stream_printf(ws, "%s", url->m_host);
        }
    }
    else {
        stream_printf(ws, ":%d", url->m_port);
    }

    if (url->m_path) {
        stream_printf(ws, "%s", url->m_path);
    }

    if (url->m_query_param_count) {
        stream_printf(ws, "%s", "?");
        
        uint16_t i;
        for(i = 0; i < url->m_query_param_count; ++i) {
            const char * name = cpe_url_query_param_name_at(url, i);
            const char * value = cpe_url_query_param_value_at(url, i);

            if (i > 0) stream_printf(ws, "&");
            
            stream_printf(ws, "%s=%s", name, value);
        }
    }
}

const char * cpe_url_dump(mem_buffer_t buffer, cpe_url_t url) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);
    
    cpe_url_print((write_stream_t)&stream, url);
    stream_putc((write_stream_t)&stream, 0);
    
    return mem_buffer_make_continuous(buffer, 0);
}

cpe_url_t cpe_url_parse(mem_allocrator_t alloc, error_monitor_t em, const char * str_url) {
    cpe_url_t url = cpe_url_create(alloc);
    if (url == NULL) {
        CPE_ERROR(em, "parse url: %s: alloc fail!", str_url);
        return NULL;
    }

    const char * left_url = str_url;
    const char * url_end = left_url + strlen(left_url);

    const char * sep = strstr(left_url, "://");
    if (sep == NULL) {
        CPE_ERROR(em, "parse url: %s: no protocol sep!", str_url);
        goto PARSE_ERROR;
    }

    url->m_protocol = cpe_str_mem_dup_range(alloc, left_url, sep);
    if (url->m_protocol == NULL) {
        CPE_ERROR(em, "parse url: %s: dup protocol %.*s fail!", str_url, (int)(sep - left_url), left_url);
        goto PARSE_ERROR;
    }
    left_url = sep + 3;

    const char * host_port_last = strchr(left_url, '/');
    if (host_port_last == NULL) host_port_last = strchr(left_url, '?');
    if (host_port_last == NULL) host_port_last = url_end;

    sep = cpe_str_char_range(left_url, host_port_last, ':');
    if (sep) {
        if (left_url != sep) {
            url->m_host = cpe_str_mem_dup_range(alloc, left_url, sep);
            if (url->m_host == NULL) {
                CPE_ERROR(em, "parse url: %s: dup host %.*s fail!", str_url, (int)(sep - left_url), left_url);
                goto PARSE_ERROR;
            }
        }

        const char * port_begin = sep + 1;
        uint16_t port_len = host_port_last - port_begin;
        char port_buf[32];
        if (port_len + 1 > CPE_ARRAY_SIZE(port_buf)) {
            CPE_ERROR(em, "parse url: %s: port %.*s too long!", str_url, (int)(port_len), port_begin);
            goto PARSE_ERROR;
        }
        memcpy(port_buf, port_begin, port_len);
        port_buf[port_len] = 0;

        char * port_end_p;
        url->m_port = strtol(port_buf, &port_end_p, 10);
        if (*port_end_p != 0) {
            CPE_ERROR(em, "parse url: %s: port %.*s format error!", str_url, (int)(port_len), port_begin);
            goto PARSE_ERROR;
        }
    } else {
        url->m_host = cpe_str_mem_dup_range(alloc, left_url, host_port_last);
        if (url->m_host == NULL) {
            CPE_ERROR(em, "parse url: %s: dup host %.*s fail!", str_url, (int)(host_port_last - left_url), left_url);
            goto PARSE_ERROR;
        }
    }
    left_url = host_port_last;

    const char * path_end = strchr(left_url, '?');
    if (path_end == NULL) path_end = url_end;

    if (left_url < path_end) {
        url->m_path = cpe_str_mem_dup_range(alloc, left_url, path_end);
        if (url->m_path == NULL) {
            CPE_ERROR(em, "parse url: %s: dup path %.*s fail!", str_url, (int)(path_end - left_url), left_url);
            goto PARSE_ERROR;
        }
    }
    left_url = *path_end ? path_end + 1 : path_end;

    const char * arg_begin = left_url;
    while (arg_begin < url_end) {
        const char * arg_end = strchr(arg_begin, '&');
        if (arg_end == NULL) arg_end = url_end;

        sep = cpe_str_char_range(arg_begin, arg_end, '=');
        if (sep == NULL) {
            CPE_ERROR(em, "parse url: %s: arg %.*s no sep!", str_url, (int)(arg_end - arg_begin), arg_begin);
            goto PARSE_ERROR;
        }

        struct cpe_url_param * param = cpe_url_query_param_confirm_next(url);
        if (param == NULL) {
            CPE_ERROR(em, "parse url: %s: append param fail!", str_url);
            goto PARSE_ERROR;
        }

        param->m_name = cpe_str_mem_dup_range(alloc, arg_begin, sep);
        if (param->m_name == NULL) {
            CPE_ERROR(em, "parse url: %s: append param: dup arg name %.*s fail!", str_url, (int)(sep - arg_begin), arg_begin);
            goto PARSE_ERROR;
        }

        param->m_value = cpe_str_mem_dup_range(alloc, sep + 1, arg_end);
        if (param->m_value == NULL) {
            CPE_ERROR(em, "parse url: %s: append param: dup arg value %.*s fail!", str_url, (int)(sep + 1 - arg_end), sep + 1);
            mem_free(url->m_alloc, param->m_name);
            goto PARSE_ERROR;
        }
        
        url->m_query_param_count++;
        arg_begin = arg_end[0] ? arg_end + 1 : arg_end;
    }
    
    return url;

PARSE_ERROR:
    cpe_url_free(url);
    return NULL;
}

