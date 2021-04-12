#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "yajl/yajl_tree.h"
#include "cpe/pal/pal_string.h"
#include "yajl_tree_copy.h"

yajl_val cpe_yajl_tree_copy(yajl_val val) {
    yajl_val copy_val = malloc(sizeof(*val));
    if (copy_val == NULL) return NULL;

    bzero(copy_val, sizeof(*val));
    copy_val->type = val->type;

    size_t i;
    switch (val->type) {
    case yajl_t_string: {
        size_t len = strlen(val->u.string);
        copy_val->u.string = malloc(len + 1);
        if (copy_val->u.string == NULL) {
            free(copy_val);
            return NULL;
        }
        memcpy(copy_val->u.string, val->u.string, len);
        copy_val->u.string[len] = 0;
        break;
    }
    case yajl_t_number: {
        size_t len = strlen(val->u.number.r);
        copy_val->u.number.r = malloc(len + 1);
        if (copy_val->u.number.r == NULL) {
            free(copy_val);
            return NULL;
        }
        memcpy(copy_val->u.number.r, val->u.number.r, len);
        copy_val->u.number.r[len] = 0;

        copy_val->u.number.i = val->u.number.i;
        copy_val->u.number.d = val->u.number.d;
        copy_val->u.number.flags = val->u.number.flags;
        break;
    }
    case yajl_t_object: {
        copy_val->u.object.keys = malloc(sizeof(const char *) * val->u.object.len);
        if (copy_val->u.object.keys == NULL) {
            free(copy_val);
            return NULL;
        }
        
        copy_val->u.object.values = malloc(sizeof(yajl_val) * val->u.object.len);
        if (copy_val->u.object.values == NULL) {
            free(copy_val->u.object.keys);
            copy_val->u.object.keys = NULL;
            free(copy_val);
            return NULL;
        }

        while(copy_val->u.object.len < val->u.object.len) {
            size_t len = strlen(val->u.object.keys[copy_val->u.object.len]);
            char * key = malloc(len +1);
            if (key == NULL) goto FREE_OBJ_ALL;
            memcpy(key, val->u.object.keys[copy_val->u.object.len], len);
            key[len] = 0;
            
            yajl_val child_val = cpe_yajl_tree_copy(val->u.object.values[copy_val->u.object.len]);
            if (child_val == NULL) {
                free(key);
                goto FREE_OBJ_ALL;
            }
            
            copy_val->u.object.keys[copy_val->u.object.len] = key;
            copy_val->u.object.values[copy_val->u.object.len] = child_val;
            copy_val->u.object.len++;
        }

        break;
    }
    case yajl_t_array: {
        copy_val->u.array.values = malloc(sizeof(yajl_val) * val->u.array.len);
        if (copy_val->u.array.values == NULL) {
            free(copy_val);
            return NULL;
        }

        while(copy_val->u.object.len < val->u.object.len) {
            yajl_val child_val = cpe_yajl_tree_copy(val->u.object.values[copy_val->u.object.len]);
            if (child_val == NULL) goto FREE_ARRAY_ALL;
            
            copy_val->u.object.values[copy_val->u.object.len] = child_val;
            copy_val->u.object.len++;
        }
        
        break;
    }
    case yajl_t_true:
    case yajl_t_false:
    case yajl_t_null:
        break;
    default:
        free(copy_val);
        return NULL;
    }

    return copy_val;
FREE_OBJ_ALL:
    for (i = 0; i < copy_val->u.object.len; i++) {
        free((void *)copy_val->u.object.keys[i]);
        yajl_tree_free(copy_val->u.object.values[i]);
    }

    free(copy_val->u.object.keys);
    free(copy_val->u.object.values);
    free(copy_val);
    return NULL;

FREE_ARRAY_ALL:
    for (i = 0; i < copy_val->u.array.len; i++) {
        yajl_tree_free(copy_val->u.array.values[i]);
    }
    free(copy_val->u.array.values);
    free(copy_val);
    return NULL;
}
