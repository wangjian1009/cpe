#ifndef CPE_PAL_LIMITS_H
#define CPE_PAL_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined MSVC /*windows*/
#include <stdint.h>
#define PATH_MAX MAX_PATH
#else /*ux*/
#include <limits.h>
#endif

#ifndef UINT16_MAX
#define UINT16_MAX  0xFFFFu
#endif

#ifndef UINT32_MAX
#define UINT32_MAX  0xFFFFFFFFUL
#endif
    
#ifdef __cplusplus
}
#endif

#endif

