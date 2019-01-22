#ifndef CPE_UTILS_SOCK_GETDNSADDRS_H
#define CPE_UTILS_SOCK_GETDNSADDRS_H
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined ANDROID
typedef int (*dns_resolver_fun_t)(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count, error_monitor_t em);
extern dns_resolver_fun_t g_android_dns_resolver;
#endif

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
