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
