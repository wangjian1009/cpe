#ifndef CPE_UTILS_TYPES_H
#define CPE_UTILS_TYPES_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct error_monitor * error_monitor_t;
typedef struct error_info * error_info_t;
typedef struct write_stream * write_stream_t;
typedef struct read_stream * read_stream_t;
typedef struct mem_allocrator * mem_allocrator_t;

typedef struct cpe_graph * cpe_graph_t;
typedef struct cpe_graph_node * cpe_graph_node_t;
typedef struct cpe_graph_node_it * cpe_graph_node_it_t;
typedef struct cpe_graph_edge * cpe_graph_edge_t;
typedef struct cpe_graph_edge_it * cpe_graph_edge_it_t;
typedef struct cpe_graph_weight_path * cpe_graph_weight_path_t;
typedef struct cpe_graph_weight_path_node * cpe_graph_weight_path_node_t;    
    
typedef struct cpe_priority_queue * cpe_priority_queue_t;

typedef struct cpe_url * cpe_url_t;
    
typedef struct mem_buffer * mem_buffer_t;
typedef struct mem_buffer_pos * mem_buffer_pos_t;
typedef struct mem_buffer_trunk * mem_buffer_trunk_t;

typedef struct cpe_hash_string * cpe_hash_string_t;

typedef struct cpe_str_ucs4 * cpe_str_ucs4_t;

typedef struct binpack_rect_size  * binpack_rect_size_t;
typedef struct binpack_rect * binpack_rect_t;
typedef struct binpack_maxrects_ctx * binpack_maxrects_ctx_t;
    
typedef struct cpe_md5_value * cpe_md5_value_t;
typedef struct cpe_md5_ctx * cpe_md5_ctx_t;

typedef struct cpe_sha1_value * cpe_sha1_value_t;
typedef struct cpe_sha1_ctx * cpe_sha1_ctx_t;

typedef struct cpe_http_arg * cpe_http_arg_t;

typedef struct ringbuffer * ringbuffer_t;
typedef struct ringbuffer_block * ringbuffer_block_t;

typedef struct cpe_raw_array * cpe_raw_array_t;
typedef struct cpe_string_array * cpe_string_array_t;

typedef struct cpe_traffic_bps * cpe_traffic_bps_t;

typedef struct cpe_hash_table_standalone * cpe_hash_table_standalone_t;
typedef struct cpe_hash_table_standalone_entry * cpe_hash_table_standalone_entry_t;
typedef struct cpe_hash_table_standalone_iterator * cpe_hash_table_standalone_iterator_t;

typedef struct cpe_yajl_tree_gen * cpe_yajl_tree_gen_t;

#ifdef __cplusplus
}
#endif

#endif
