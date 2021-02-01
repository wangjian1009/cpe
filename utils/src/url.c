#include <assert.h>
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
        cpe_url_encode_from_buf(ws, url->m_path, strlen(url->m_path), NULL);
    }

    if (url->m_query_param_count) {
        stream_printf(ws, "%s", "?");
        
        uint16_t i;
        for(i = 0; i < url->m_query_param_count; ++i) {
            const char * name = cpe_url_query_param_name_at(url, i);
            const char * value = cpe_url_query_param_value_at(url, i);

            cpe_url_encode_from_buf(ws, name, strlen(name), NULL);
            stream_printf(ws, "=");
            cpe_url_encode_from_buf(ws, value, strlen(value), NULL);
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
    }

    return url;
}

