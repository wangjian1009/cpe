#include "yaml_parse.h"

int yajl_document_parse_one(yaml_document_t *document, const char * input, char * error_buf, size_t error_buf_size) {
    yaml_parser_t parser;
    
    if (yaml_parser_initialize(&parser) != 1) {
        if (error_buf) snprintf(error_buf, error_buf_size, "init parser fail!");
        return -1;
    }

    yaml_parser_set_input_string(&parser, (const unsigned char *)input, strlen(input));
    yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
    
    if (yaml_parser_load(&parser, document) != 1) {
        if (error_buf) {
            snprintf(error_buf, error_buf_size, "parse fail!");
        }
        return -1;
    }

    yaml_parser_delete(&parser);
    return 0;
}
