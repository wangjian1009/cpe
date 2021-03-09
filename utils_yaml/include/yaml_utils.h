#ifndef CPE_UTILS_YAML_UTILS_H
#define CPE_UTILS_YAML_UTILS_H
#include "yaml.h"
#include "yaml_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yaml_node_mapping_item_it {
    yaml_node_mapping_item_t (*next)(yaml_node_mapping_item_it_t it);
    char data[64];
};

struct yaml_node_it {
    yaml_node_t * (*next)(yaml_node_it_t it);
    char data[64];
};

const char * yaml_error_type_str(yaml_error_type_t err);
const char * yaml_node_type_str(yaml_node_type_t node_type);

yaml_node_t * yaml_tree_get(yaml_document_t * document, yaml_node_t * n, const char ** path);
yaml_node_t * yaml_tree_get_2(yaml_document_t * document, yaml_node_t * n, const char * path);

const char * yaml_node_tag(yaml_node_t * n);
const char * yaml_node_value(yaml_node_t * n);

const char * yaml_node_mapping_item_name(yaml_node_mapping_item_t item);
yaml_node_t * yaml_node_mapping_item_value(yaml_node_mapping_item_t item);
void yaml_node_mapping_childs(yaml_document_t * document, yaml_node_t * node_mapping, yaml_node_mapping_item_it_t it);
    
void yaml_node_sequence_childs(yaml_document_t * document, yaml_node_t * node_sequence, yaml_node_it_t it);
    
#define yaml_node_mapping_item_it_next(__it) ((__it)->next(__it))
#define yaml_node_it_next(__it) ((__it)->next(__it))

#ifdef __cplusplus
}
#endif

#endif
