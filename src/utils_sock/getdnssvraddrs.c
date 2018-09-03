#include "cpe/pal/pal_string.h"
#include "cpe/utils_sock/getdnssvraddrs.h"

#ifdef ANDROID

#include <sys/system_properties.h>

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count) {
    uint8_t addr_capacity = *addr_count;
    *addr_count = 0;
    
    if (*addr_count < addr_capacity) {
        struct sockaddr_storage * addr = dnssevraddrs + *addr_count;
        socklen_t addr_len = sizeof(*addr);
        
        char buf[PROP_VALUE_MAX];
        __system_property_get("net.dns1", buf);

        if (sock_ipv4_init((struct sockaddr *)addr, &addr_len, buf1, 0, NULL) == 0) {
            *addr_count++;
        }
        else if (sock_ipv6_init((struct sockaddr *)addr, &addr_len, url, port, NULL) == 0) {
            *addr_count++;
        }
    }

    
    if (*addr_count < addr_capacity) {
        struct sockaddr_storage * addr = dnssevraddrs + *addr_count;
        socklen_t addr_len = sizeof(*addr);
        
        char buf[PROP_VALUE_MAX];
        __system_property_get("net.dns2", buf);

        if (sock_ipv4_init((struct sockaddr *)addr, &addr_len, buf1, 0, NULL) == 0) {
            *addr_count++;
        }
        else if (sock_ipv6_init((struct sockaddr *)addr, &addr_len, url, port, NULL) == 0) {
            *addr_count++;
        }
    }

    return 0;
}

#elif defined __APPLE__
#include <TargetConditionals.h>
#include <resolv.h>

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count) {
    uint8_t addr_capacity = *addr_count;
    *addr_count = 0;
    
    struct __res_state stat = {0};
    res_ninit(&stat);
    union res_sockaddr_union addrs[MAXNS] = {0};
    int count = res_getservers(&stat, addrs, MAXNS);
    if (count > addr_capacity) count = addr_capacity;
    for (*addr_count = 0; *addr_count < count; ++addr_count) {
        if (AF_INET == addrs[*addr_count].sin.sin_family) {
            memcpy(dnssevraddrs + *addr_count, &addrs[*addr_count].sin, sizeof(addrs[*addr_count].sin));
        }
        else if (AF_INET6 == addrs[*addr_count].sin.sin_family) {
            memcpy(dnssevraddrs + *addr_count, &addrs[*addr_count].sin6, sizeof(addrs[*addr_count].sin6));
        }
    }
    
    res_ndestroy(&stat);

    return 0;
}

#elif defined _WIN32
#include <stdio.h>
#include <windows.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count) {
    FIXED_INFO fi;
    ULONG ulOutBufLen = sizeof(fi);
    
    if (::GetNetworkParams(&fi, &ulOutBufLen) != ERROR_SUCCESS) {
        return;
    }
    
    IP_ADDR_STRING* pIPAddr = fi.DnsServerList.Next;
    
    while (pIPAddr != NULL) {
		_dnssvraddrs.push_back(socket_address(pIPAddr->IpAddress.String) );
        pIPAddr = pIPAddr->Next;
    }
    
    return;
}
#else
int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count) {
    *addr_count = 0;
    return -1;
}
#endif
