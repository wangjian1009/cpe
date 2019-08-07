#ifndef CPE_UTILS_YAML_PARSE_H
#define CPE_UTILS_YAML_PARSE_H
#include "yaml.h"
#include "yaml_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int yajl_document_parse_one(yaml_document_t *document, const char * input, char * error_buf, size_t error_buf_size);

#ifdef __cplusplus
}
#endif

#endif
