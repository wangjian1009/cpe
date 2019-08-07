#ifndef CPE_UTILS_YAML_UTILS_H
#define CPE_UTILS_YAML_UTILS_H
#include "yaml.h"
#include "yaml_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * yaml_error_type_str(yaml_error_type_t err);
const char * yaml_node_type_str(yaml_node_type_t node_type);

yaml_node_t * yaml_tree_get(yaml_document_t * document, yaml_node_t * n, const char ** path);
yaml_node_t * yaml_tree_get_2(yaml_document_t * document, yaml_node_t * n, const char * path);

const char * yaml_get_string(yaml_node_t * n);

#ifdef __cplusplus
}
#endif

#endif
