#ifndef CPE_UTILS_URL_H
#define CPE_UTILS_URL_H
#include "utils_types.h"

cpe_url_t cpe_url_create(mem_allocrator_t alloc);
cpe_url_t cpe_url_clone(mem_allocrator_t alloc, cpe_url_t from);
void cpe_url_free(cpe_url_t url);

const char * cpe_url_protocol(cpe_url_t url);
int cpe_url_set_protocol(cpe_url_t url, const char * protocol);

const char * cpe_url_host(cpe_url_t url);
int cpe_url_set_host(cpe_url_t url, const char * host);

uint16_t cpe_url_port(cpe_url_t url);
void cpe_url_set_port(cpe_url_t url, uint16_t port);

const char * cpe_url_path(cpe_url_t url);
int cpe_url_set_path(cpe_url_t url, const char * path);

uint16_t cpe_url_query_param_count(cpe_url_t url);
const char * cpe_url_query_param_name_at(cpe_url_t url, uint16_t pos);
const char * cpe_url_query_param_value_at(cpe_url_t url, uint16_t pos);
const char * cpe_url_query_param_value_find(cpe_url_t url, const char * name);
int cpe_url_query_param_add(cpe_url_t url, const char * name, const char * value);

int cpe_url_cmp(cpe_url_t l, cpe_url_t r);
int cpe_url_cmp_opt(cpe_url_t l, cpe_url_t r);

void cpe_url_md5_update(cpe_url_t url, cpe_md5_ctx_t ctx);
void cpe_url_md5(cpe_url_t url, cpe_md5_value_t r_value);

enum cpe_url_print_scope {
    cpe_url_print_full,
    cpe_url_print_host,
    cpe_url_print_path_query,
};

void cpe_url_print(write_stream_t ws, cpe_url_t url, enum cpe_url_print_scope scope);
const char * cpe_url_dump(mem_buffer_t buffer, cpe_url_t url, enum cpe_url_print_scope scope);

cpe_url_t cpe_url_parse(mem_allocrator_t alloc, error_monitor_t em, const char * str_url);

#endif
