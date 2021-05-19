#include <assert.h>
#include "cpe/pal/pal_errno.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "yajl/yajl_tree.h"
#include "yajl/yajl_parse.h"
#include "yajl_stream_parser.h"

#define STATUS_CONTINUE 1
#define STATUS_ABORT    0

extern long long yajl_parse_integer(const unsigned char *number, unsigned int length);
static const yajl_callbacks s_callbacks;

struct stack_elem_s;
typedef struct stack_elem_s stack_elem_t;

struct stack_elem_s {
    char * key;
    yajl_val value;
    stack_elem_t *next;
};

struct cpe_yajl_stream_parser {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    yajl_handle m_handle;
    stack_elem_t * m_stack;
    uint8_t m_completed;
    yajl_status m_status;
    yajl_val m_root;
    char m_errbuf[128];
};

static yajl_val cpe_yajl_stream_parser_pop(cpe_yajl_stream_parser_t parser);
static int cpe_yajl_stream_parser_push(cpe_yajl_stream_parser_t parser, yajl_val v);

cpe_yajl_stream_parser_t
cpe_yajl_stream_parser_create(mem_allocrator_t alloc, error_monitor_t em) {
    cpe_yajl_stream_parser_t parser = mem_alloc(alloc, sizeof(struct cpe_yajl_stream_parser));
    if (parser == NULL) {
        CPE_ERROR(em, "yajl stream parser: create: alloc fail!");
        return NULL;
    }

    parser->m_alloc = alloc;
    parser->m_em = em;

    parser->m_stack = NULL;
    
    parser->m_handle = yajl_alloc(&s_callbacks, NULL, parser);
    if (parser->m_handle == NULL) {
        CPE_ERROR(em, "yajl stream parser: create: create handle fail!");
        mem_free(alloc, parser);
        return NULL;
    }
    
    yajl_config(parser->m_handle, yajl_allow_comments, 1);

    parser->m_completed = 0;
    parser->m_status = yajl_status_ok;
    parser->m_root = NULL;
	parser->m_errbuf[0] = 0;

    return parser;
}

void cpe_yajl_stream_parser_free(cpe_yajl_stream_parser_t parser) {
    if (parser->m_handle) {
        yajl_free(parser->m_handle);
        parser->m_handle = NULL;
    }

    if (parser->m_root) {
        yajl_tree_free(parser->m_root);
        parser->m_root = NULL;
    }

    while(parser->m_stack) {
        yajl_val val = cpe_yajl_stream_parser_pop(parser);
        if (val) yajl_tree_free(val);
    }
    
    mem_free(parser->m_alloc, parser);
}

uint8_t cpe_yajl_stream_parser_is_complete(cpe_yajl_stream_parser_t parser) {
    return parser->m_completed;
}

uint8_t cpe_yajl_stream_parser_is_ok(cpe_yajl_stream_parser_t parser) {
    return parser->m_status == yajl_status_ok;
}

const char * cpe_yajl_stream_parser_error_msg(cpe_yajl_stream_parser_t parser) {
    return parser->m_errbuf[0] ? parser->m_errbuf : NULL;
}

yajl_val cpe_yajl_stream_parser_result(cpe_yajl_stream_parser_t parser) {
    if (parser->m_status != yajl_status_ok || !parser->m_completed) return NULL;
    return parser->m_root;
}

yajl_val cpe_yajl_stream_parser_result_retrieve(cpe_yajl_stream_parser_t parser) {
    if (parser->m_status != yajl_status_ok || !parser->m_completed) return NULL;

    yajl_val r = parser->m_root;
    parser->m_root = NULL;
    return r;
}

int cpe_yajl_stream_parser_append(cpe_yajl_stream_parser_t parser, void const * data, uint32_t data_size) {
    if (parser->m_status != yajl_status_ok || parser->m_completed) return -1;
    parser->m_status = yajl_parse(parser->m_handle, (unsigned char *) data, data_size);
    return parser->m_status == yajl_status_ok ? 0 : -1;
}

int cpe_yajl_stream_parser_complete(cpe_yajl_stream_parser_t parser) {
    if (parser->m_status != yajl_status_ok || parser->m_completed) return -1;

    parser->m_status = yajl_complete_parse(parser->m_handle);
    parser->m_completed = 1;

    return parser->m_status == yajl_status_ok ? 0 : -1;
}

static yajl_val value_alloc(yajl_type type) {
    yajl_val v = malloc (sizeof (*v));

    if (v) {
        bzero(v, sizeof (*v));
        v->type = type;
    }

    return v;
}

static int cpe_yajl_stream_parser_push(cpe_yajl_stream_parser_t parser, yajl_val v) {
    stack_elem_t * stack = malloc(sizeof (*stack));
    if (stack == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return ENOMEM;
    }
    bzero(stack, sizeof(*stack));

    assert ((parser->m_stack == NULL)
            || YAJL_IS_OBJECT (v)
            || YAJL_IS_ARRAY (v));

    stack->value = v;
    stack->next = parser->m_stack;
    parser->m_stack = stack;

    return 0;
}

static yajl_val cpe_yajl_stream_parser_pop(cpe_yajl_stream_parser_t parser) {
    stack_elem_t *stack;
    yajl_val v;

    if (parser->m_stack == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf),  "context_pop: Bottom of stack reached prematurely");
        return NULL;
    }

    stack = parser->m_stack;
    parser->m_stack = stack->next;

    v = stack->value;

    free(stack);

    return v;
}

static int object_add_keyval(cpe_yajl_stream_parser_t parser, yajl_val obj, char *key, yajl_val value) {
    const char **tmpk;
    yajl_val *tmpv;

    /* We're checking for NULL in "context_add_value" or its callers. */
    assert (parser != NULL);
    assert (obj != NULL);
    assert (key != NULL);
    assert (value != NULL);

    /* We're assuring that "obj" is an object in "context_add_value". */
    assert(YAJL_IS_OBJECT(obj));

    tmpk = realloc((void *) obj->u.object.keys, sizeof(*(obj->u.object.keys)) * (obj->u.object.len + 1));
    if (tmpk == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return ENOMEM;
    }
    obj->u.object.keys = tmpk;

    tmpv = realloc(obj->u.object.values, sizeof (*obj->u.object.values) * (obj->u.object.len + 1));
    if (tmpv == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return ENOMEM;
    }
    obj->u.object.values = tmpv;
    obj->u.object.keys[obj->u.object.len] = key;
    obj->u.object.values[obj->u.object.len] = value;
    obj->u.object.len++;

    return (0);
}

static int array_add_value(cpe_yajl_stream_parser_t parser, yajl_val array, yajl_val value) {
    /* We're checking for NULL pointers in "context_add_value" or its
     * callers. */
    assert (parser != NULL);
    assert (array != NULL);
    assert (value != NULL);

    /* "context_add_value" will only call us with array values. */
    assert(YAJL_IS_ARRAY(array));
    
    yajl_val * tmp = realloc(array->u.array.values, sizeof(*(array->u.array.values)) * (array->u.array.len + 1));
    if (tmp == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return ENOMEM;
    }

    array->u.array.values = tmp;
    array->u.array.values[array->u.array.len] = value;
    array->u.array.len++;

    return 0;
}

static int cpe_yajl_stream_parser_add_value(cpe_yajl_stream_parser_t parser, yajl_val v) {
    /* We're checking for NULL values in all the calling functions. */
    assert (parser != NULL);
    assert (v != NULL);

    if (parser->m_stack == NULL) {
        assert (parser->m_root == NULL);
        parser->m_root = v;
        return (0);
    }
    else if (YAJL_IS_OBJECT (parser->m_stack->value)) {
        if (parser->m_stack->key == NULL) {
            if (!YAJL_IS_STRING (v)) {
                snprintf(
                    parser->m_errbuf, sizeof(parser->m_errbuf), 
                    "context_add_value: Object key is not a string (%#04x)", v->type);
                return EINVAL;
            }
                    
            parser->m_stack->key = v->u.string;
            v->u.string = NULL;
            free(v);
            return 0;
        }
        else { /* if (ctx->key != NULL) */
            char * key = parser->m_stack->key;
            parser->m_stack->key = NULL;
            return object_add_keyval(parser, parser->m_stack->value, key, v);
        }
    }
    else if (YAJL_IS_ARRAY (parser->m_stack->value)) {
        return array_add_value (parser, parser->m_stack->value, v);
    }
    else {
        snprintf(
            parser->m_errbuf, sizeof(parser->m_errbuf),
            "context_add_value: Cannot add value to a value of type %#04x (not a composite type)",
            parser->m_stack->value->type);
        return EINVAL;
    }
}

static int handle_string(void *ctx, const unsigned char *string, size_t string_length) {
    cpe_yajl_stream_parser_t parser = ctx;

    yajl_val v = value_alloc(yajl_t_string);
    if (v == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }

    v->u.string = malloc(string_length + 1);
    if (v->u.string == NULL) {
        free (v);
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }
    memcpy(v->u.string, string, string_length);
    v->u.string[string_length] = 0;

    return ((cpe_yajl_stream_parser_add_value(ctx, v) == 0) ? STATUS_CONTINUE : STATUS_ABORT);
}

static int handle_number (void *ctx, const char *string, size_t string_length) {
    cpe_yajl_stream_parser_t parser = ctx;
    
    yajl_val v = value_alloc(yajl_t_number);
    if (v == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }
    
    v->u.number.r = malloc(string_length + 1);
    if (v->u.number.r == NULL) {
        free(v);
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }
    memcpy(v->u.number.r, string, string_length);
    v->u.number.r[string_length] = 0;

    v->u.number.flags = 0;

    errno = 0;
    v->u.number.i = yajl_parse_integer((const unsigned char *) v->u.number.r, strlen(v->u.number.r));
    if (errno == 0) {
        v->u.number.flags |= YAJL_NUMBER_INT_VALID;
    }

    char * endptr = NULL;
    v->u.number.d = strtod(v->u.number.r, &endptr);
    if (endptr != NULL && *endptr == 0) {
        v->u.number.flags |= YAJL_NUMBER_DOUBLE_VALID;
    }
    
    return cpe_yajl_stream_parser_add_value(ctx, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static int handle_start_map (void *ctx) {
    cpe_yajl_stream_parser_t parser = ctx;
    
    yajl_val v = value_alloc(yajl_t_object);
    if (v == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }        

    v->u.object.keys = NULL;
    v->u.object.values = NULL;
    v->u.object.len = 0;

    return cpe_yajl_stream_parser_push(ctx, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static int handle_end_map (void *ctx) {
    cpe_yajl_stream_parser_t parser = ctx;

    yajl_val v = cpe_yajl_stream_parser_pop(ctx);
    if (v == NULL) STATUS_ABORT;

    return cpe_yajl_stream_parser_add_value(ctx, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static int handle_start_array (void *ctx) {
    cpe_yajl_stream_parser_t parser = ctx;

    yajl_val v = value_alloc(yajl_t_array);
    if (v == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }

    v->u.array.values = NULL;
    v->u.array.len = 0;

    return cpe_yajl_stream_parser_push (ctx, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static int handle_end_array(void *ctx) {
    cpe_yajl_stream_parser_t parser = ctx;

    yajl_val v = cpe_yajl_stream_parser_pop(parser);
    if (v == NULL) return STATUS_ABORT;

    return cpe_yajl_stream_parser_add_value(parser, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static int handle_boolean(void *ctx, int boolean_value) {
    cpe_yajl_stream_parser_t parser = ctx;

    yajl_val v = value_alloc(boolean_value ? yajl_t_true : yajl_t_false);
    if (v == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }        

    return cpe_yajl_stream_parser_add_value (ctx, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static int handle_null (void *ctx) {
    cpe_yajl_stream_parser_t parser = ctx;

    yajl_val v = value_alloc(yajl_t_null);
    if (v == NULL) {
        snprintf(parser->m_errbuf, sizeof(parser->m_errbuf), "Out of memory");
        return STATUS_ABORT;
    }

    return cpe_yajl_stream_parser_add_value (ctx, v) == 0 ? STATUS_CONTINUE : STATUS_ABORT;
}

static const yajl_callbacks s_callbacks = {
    /* null        = */ handle_null,
    /* boolean     = */ handle_boolean,
    /* integer     = */ NULL,
    /* double      = */ NULL,
    /* number      = */ handle_number,
    /* string      = */ handle_string,
    /* start map   = */ handle_start_map,
    /* map key     = */ handle_string,
    /* end map     = */ handle_end_map,
    /* start array = */ handle_start_array,
    /* end array   = */ handle_end_array
};
