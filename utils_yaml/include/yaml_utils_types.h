#ifndef CPE_UTILS_YAML_TYPES_H
#define CPE_UTILS_YAML_TYPES_H
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct yaml_document_s yaml_document_t;
typedef struct yaml_node_s yaml_node_t;

typedef struct yaml_node_mapping_item * yaml_node_mapping_item_t;
typedef struct yaml_node_mapping_item_it * yaml_node_mapping_item_it_t;

typedef struct yaml_node_it * yaml_node_it_t;

#ifdef __cplusplus
}
#endif

#endif
