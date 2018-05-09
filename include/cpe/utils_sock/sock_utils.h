#ifndef CPE_SOCK_UTILS_H
#define CPE_SOCK_UTILS_H
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int sock_set_reuseport(int socket);
size_t sock_get_sockaddr_len(struct sockaddr *addr);

char * sock_get_addr(
    char * buf, size_t buf_capacity,
    struct sockaddr_storage * addr, socklen_t addr_len,
    uint8_t with_port, error_monitor_t em);

char * sock_get_peer_addr(
    char * buf, size_t buf_capacity,
    int fd,
    uint8_t with_port, error_monitor_t em);

char * sock_get_local_addr(
    char * buf, size_t buf_capacity,
    int fd,
    uint8_t with_port, error_monitor_t em);

int sock_print_addr(write_stream_t ws, struct sockaddr_storage * addr, socklen_t addr_len, uint8_t with_port, error_monitor_t em);
int sock_print_peer_addr(write_stream_t ws, int fd, uint8_t with_port, error_monitor_t em);
int sock_print_local_addr(write_stream_t ws, int fd, uint8_t with_port, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
