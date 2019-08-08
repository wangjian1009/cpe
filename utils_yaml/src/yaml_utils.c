#include <assert.h>
#include "yaml_utils.h"

const char * yaml_error_type_str(yaml_error_type_t err) {
    switch(err) {
    case YAML_NO_ERROR:
        return "no-error";
    case YAML_MEMORY_ERROR:
        return "memory-error";
    case YAML_READER_ERROR:
        return "reader-error";
    case YAML_SCANNER_ERROR:
        return "scanner-error";
    case YAML_PARSER_ERROR:
        return "composer-error";
    case YAML_COMPOSER_ERROR:
        return "composer-error";
    case YAML_WRITER_ERROR:
        return "writer-error";
    case YAML_EMITTER_ERROR:
        return "emitter-error";
    }
}

const char * yaml_node_type_str(yaml_node_type_t node_type) {
    switch(node_type) {
    case YAML_NO_NODE:
        return "none";
    case YAML_SCALAR_NODE:
        return "scalar";
    case YAML_SEQUENCE_NODE:
        return "sequence";
    case YAML_MAPPING_NODE:
        return "mapping";
    }
}

yaml_node_t * yaml_tree_get(yaml_document_t * document, yaml_node_t * n, const char ** path) {
    if (!path) return NULL;
    
    while (n && *path) {
        unsigned int i;

        if (n->type != YAML_MAPPING_NODE) return NULL;

        yaml_node_pair_t * c;
        for(c = n->data.mapping.pairs.start; c != n->data.mapping.pairs.top; c++) {
            yaml_node_t * key_node = yaml_document_get_node(document, c->key);
            assert(key_node);
            if (key_node->type != YAML_SCALAR_NODE) continue;

            if (strcmp((const char *)key_node->data.scalar.value, *path) != 0) continue;

            n = yaml_document_get_node(document, c->value);
            if (n == NULL) return NULL;
        }

        if (c == n->data.mapping.pairs.end) return NULL;
        path++;
    }

    return n;
}

yaml_node_t * yaml_tree_get_2(yaml_document_t * document, yaml_node_t * n, const char * path) {
    const char * sep;
    yaml_node_pair_t * c;

    while (n && (sep = strchr(path, '/'))) {
        unsigned int i;

        if (n->type != YAML_MAPPING_NODE) return NULL;

        for(c = n->data.mapping.pairs.start; c != n->data.mapping.pairs.top; c++) {
            yaml_node_t * key_node = yaml_document_get_node(document, c->key);
            if (key_node == NULL) continue;
            if (key_node->type != YAML_SCALAR_NODE) continue;
            if (key_node->data.scalar.length != (sep - path)) continue;

            if (memcmp(key_node->data.scalar.value, path, key_node->data.scalar.length) != 0) continue;
            
            n = yaml_document_get_node(document, c->value);
            if (n == NULL) return NULL;
            
            path = sep + 1;
            break;
        }

        if (c == n->data.mapping.pairs.end) return NULL;
    }

    if (n->type != YAML_MAPPING_NODE) return NULL;
    
    size_t left_length = strlen(path);

    for (c = n->data.mapping.pairs.start; c != n->data.mapping.pairs.top; c++) {
        yaml_node_t* key_node = yaml_document_get_node(document, c->key);
        if (key_node == NULL) continue;
        if (key_node->type != YAML_SCALAR_NODE) continue;

        if (key_node->data.scalar.length != left_length) continue;
        if (memcmp(key_node->data.scalar.value, path, key_node->data.scalar.length) != 0) continue;

        n = yaml_document_get_node(document, c->value);
        break;
    }

    return n;
}

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

const char * yaml_node_tag(yaml_node_t * n) {
    return (const char *)n->tag;
}

const char * yaml_node_value(yaml_node_t * n) {
    if (n == NULL) return NULL;
    if (n->type != YAML_SCALAR_NODE) return NULL;
    return (const char *)n->data.scalar.value;
}
