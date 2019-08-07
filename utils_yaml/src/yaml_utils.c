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
        for(c = n->data.mapping.pairs.start; c != n->data.mapping.pairs.end; c++) {
            //if (!strcmp(*path, c->key) {
            /*     n = n->u.object.values[i]; */
            /*     break; */
            /* } */
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

        for(c = n->data.mapping.pairs.start; c != n->data.mapping.pairs.end; c++) {
            yaml_node_t * key_node = yaml_document_get_node(document, c->key);
            if (key_node == NULL) continue;
            if (key_node->type != YAML_SCALAR_NODE) continue;
            if (key_node->data.scalar.length != (sep - path)) continue;

            if (memcmp(key_node->data.scalar.value, path, key_node->data.scalar.length) != 0) continue;
            
            n = document->nodes.start + c->value;
            path = sep + 1;
            break;
        }

        if (c == n->data.mapping.pairs.end) return NULL;
        path++;
    }

    if (n->type != YAML_MAPPING_NODE) return NULL;
    
    size_t left_length = strlen(path);

    for (c = n->data.mapping.pairs.start; c != n->data.mapping.pairs.end; c++) {
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

const char * yaml_get_string(yaml_node_t * n) {
    if (n == NULL) return NULL;
    if (n->type != YAML_SCALAR_NODE) return NULL;
    return (const char *)n->data.scalar.value;
}
