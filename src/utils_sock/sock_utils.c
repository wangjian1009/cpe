#include "cpe/pal/pal_strings.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream.h"
#include "cpe/utils_sock/sock_utils.h"

int sock_set_reuseport(int socket) {
    int opt = 1;
    return setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

size_t sock_get_sockaddr_len(struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        return sizeof(struct sockaddr_in);
    }
    else if (addr->sa_family == AF_INET6) {
        return sizeof(struct sockaddr_in6);
    }
    else {
        return 0;
    }
}

char * sock_get_addr(
    char * buf, size_t buf_capacity,
    struct sockaddr_storage * addr, socklen_t addr_len, uint8_t with_port, error_monitor_t em)
{
    int port;
    const char * str_addr;

    buf[0] = 0;
    
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)addr;
        str_addr = inet_ntop(AF_INET, &s->sin_addr, buf, buf_capacity);
        port = ntohs(s->sin_port);
    }
    else if (addr->ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)addr;
        str_addr = inet_ntop(AF_INET6, &s->sin6_addr, buf, buf_capacity);
        port = ntohs(s->sin6_port);
    }
    else {
        return "unknown-family";
    }

    if (with_port) {
        size_t used_len = str_addr ? strlen(str_addr) : 0;
        snprintf(buf + used_len, buf_capacity - used_len, ":%d", port);
    }
    
    return buf;
}


char * sock_get_remote_addr(char * buf, size_t buf_capacity, int fd, uint8_t with_port, error_monitor_t em) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(struct sockaddr_storage);
    memset(&addr, 0, len);
    int err = getpeername(fd, (struct sockaddr *)&addr, &len);
    if (err == 0) {
        return sock_get_addr(buf, buf_capacity, &addr, len, with_port, em);
    }
    else {
        CPE_ERROR(
            em, "sock_get_local_name_by_fd: getpeername fail, %d (%s)",
            cpe_sock_errno(), strerror(cpe_sock_errno()));
        return NULL;
    }
}

char * sock_get_local_addr(char * buf, size_t buf_capacity, int fd, uint8_t with_port, error_monitor_t em) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(struct sockaddr_storage);
    memset(&addr, 0, len);
    int err = getsockname(fd, (struct sockaddr *)&addr, &len);
    if (err == 0) {
        return sock_get_addr(buf, buf_capacity, &addr, len, with_port, em);
    }
    else {
        CPE_ERROR(
            em, "sock_get_local_addr: getsockname fail, %d (%s)",
            cpe_sock_errno(), strerror(cpe_sock_errno()));
        return NULL;
    }
}

int sock_print_addr(write_stream_t ws, struct sockaddr_storage * addr, socklen_t addr_len, uint8_t with_port, error_monitor_t em) {
    static char buf[INET6_ADDRSTRLEN];
    const char * str_addr;
    int port;

    if (addr->ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)addr;
        str_addr = inet_ntop(AF_INET, &s->sin_addr, buf, sizeof(buf));
        port = ntohs(s->sin_port);
    }
    else if (addr->ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)addr;
        str_addr = inet_ntop(AF_INET6, &s->sin6_addr, buf, sizeof(buf));
        port = ntohs(s->sin6_port);
    }
    else {
        return -1;
    }

    if (str_addr) {
        size_t used_len = strlen(str_addr);
        stream_write(ws, str_addr, used_len);
    }

    if (with_port) {
        stream_printf(ws, ":%d", port);
    }
    
    return 0;
    
}

int sock_print_peer_addr(write_stream_t ws, int fd, uint8_t with_port, error_monitor_t em) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(struct sockaddr_storage);
    memset(&addr, 0, len);
    int err = getpeername(fd, (struct sockaddr *)&addr, &len);
    if (err == 0) {
        return sock_print_addr(ws, &addr, len, with_port, em);
    }
    else {
        CPE_ERROR(
            em, "sock_print_local_addr: getpeername fail, %d (%s)",
            cpe_sock_errno(), strerror(cpe_sock_errno()));
        return -1;
    }
}

int sock_print_local_addr(write_stream_t ws, int fd, uint8_t with_port, error_monitor_t em) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(struct sockaddr_storage);
    memset(&addr, 0, len);
    int err = getsockname(fd, (struct sockaddr *)&addr, &len);
    if (err == 0) {
        return sock_print_addr(ws, &addr, len, with_port, em);
    }
    else {
        CPE_ERROR(
            em, "sock_print_local_addr: getsockname fail, %d (%s)",
            cpe_sock_errno(), strerror(cpe_sock_errno()));
        return -1;
    }
}
