#include <assert.h>
#include "yaml_utils.h"

struct yaml_node_sequence_it_data {
    yaml_document_t * m_document;
    yaml_node_item_t * m_next;
    yaml_node_item_t * m_end;
};

static yaml_node_t * yaml_node_sequence_childs_next(yaml_node_it_t it) {
    struct yaml_node_sequence_it_data * data = (struct yaml_node_sequence_it_data *)it->data;

    yaml_node_item_t * current = data->m_next;
    if (current == data->m_end) return NULL;
    
    data->m_next++;

    return yaml_document_get_node(data->m_document, *current);
}

void yaml_node_sequence_childs(yaml_document_t * document, yaml_node_t * node_sequence, yaml_node_it_t it) {
    struct yaml_node_sequence_it_data * data = (struct yaml_node_sequence_it_data *)it->data;
    data->m_document = document;
    data->m_next = NULL;
    data->m_end = NULL;

    if (node_sequence->type == YAML_SEQUENCE_NODE) {
        data->m_next = node_sequence->data.sequence. items.start;
        data->m_end = node_sequence->data.sequence.items.top;
    }
    
    it->next = yaml_node_sequence_childs_next;
}
