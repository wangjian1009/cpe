#ifndef CPE_SOCK_UTILS_H
#define CPE_SOCK_UTILS_H
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int sock_set_reuseport(int socket);
size_t sock_get_sockaddr_len(struct sockaddr *addr);

int sock_create_and_bind(const char *addr, const char *port);

#ifdef __cplusplus
}
#endif

#endif
