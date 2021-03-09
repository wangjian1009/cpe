#include <assert.h>
#include "yaml_utils.h"

struct yaml_node_mapping_item {
    yaml_document_t * m_document;
    yaml_node_pair_t * m_current;
    yaml_node_pair_t * m_next;
    yaml_node_pair_t * m_end;
};

const char * yaml_node_mapping_item_name(yaml_node_mapping_item_t item) {
    assert(item);
    assert(item->m_current != item->m_end);

    yaml_node_t* key_node = yaml_document_get_node(item->m_document, item->m_current->key);
    if (key_node == NULL) NULL;
    if (key_node->type != YAML_SCALAR_NODE) return NULL;

    return (const char *)key_node->data.scalar.value;
}

yaml_node_t * yaml_node_mapping_item_value(yaml_node_mapping_item_t item) {
    assert(item->m_current != item->m_end);
    return yaml_document_get_node(item->m_document, item->m_current->value);
}

static yaml_node_mapping_item_t yaml_node_mapping_childs_next(yaml_node_mapping_item_it_t it) {
    struct yaml_node_mapping_item * data = (struct yaml_node_mapping_item *)it->data;

    if (data->m_current == data->m_end) return NULL;
    
    data->m_current = data->m_next;
    data->m_next++;

    return data->m_current == data->m_end ? NULL : data;
}

void yaml_node_mapping_childs(yaml_document_t * document, yaml_node_t * node_mapping, yaml_node_mapping_item_it_t it) {
    struct yaml_node_mapping_item * data = (struct yaml_node_mapping_item *)it->data;
    data->m_document = document;
    data->m_current = NULL;
    data->m_next = NULL;
    data->m_end = NULL;

    if (node_mapping->type == YAML_MAPPING_NODE) {
        data->m_next = node_mapping->data.mapping.pairs.start;
        data->m_end = node_mapping->data.mapping.pairs.top;
    }
    
    it->next = yaml_node_mapping_childs_next;
}

yaml_node_t * yaml_node_mapping_find_child(
    yaml_document_t * document, yaml_node_t * node_mapping, const char * name)
{
    if (node_mapping == NULL) return NULL;
    if (node_mapping->type != YAML_MAPPING_NODE) return NULL;

    struct yaml_node_mapping_item_it item_it;
    yaml_node_mapping_childs(document, node_mapping, &item_it);

    yaml_node_mapping_item_t item;
    while((item = yaml_node_mapping_item_it_next(&item_it))) {
        if (strcmp(yaml_node_mapping_item_name(item), name) == 0) {
            return yaml_node_mapping_item_value(item);
        }
    }
    
    return NULL;
}
