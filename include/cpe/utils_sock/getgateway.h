#ifndef CPE_UTILS_SOCK_GETGETWARY_H
#define CPE_UTILS_SOCK_GETGETWARY_H
#include "cpe/pal/pal_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/* getdefaultgateway() :
 * return value :
 *    0 : success
 *   -1 : failure    */
int getdefaultgateway(struct in_addr * addr);
int getdefaultgateway6(struct in6_addr * addr);

#ifdef __cplusplus
}
#endif

#endif
