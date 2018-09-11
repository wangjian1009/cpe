#include "cpe/pal/pal_string.h"
#include "cpe/utils_sock/getdnssvraddrs.h"
#include "cpe/utils_sock/sock_utils.h"

#ifdef ANDROID

#include <sys/system_properties.h>

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count, error_monitor_t em) {
    uint8_t addr_capacity = *addr_count;
    *addr_count = 0;

    unsigned int i;
    for (i = 1; i <= MAX_DNS_PROPERTIES && *addr_count < addr_capacity; i++) {
        char propname[PROP_NAME_MAX];
        char propvalue[PROP_VALUE_MAX];
        snprintf(propname, sizeof(propname), "%s%u", DNS_PROP_NAME_PREFIX, i);
        if (__system_property_get(propname, propvalue) < 1) {
            continue;
        }

        struct sockaddr_storage * addr = dnssevraddrs + *addr_count;
        socklen_t addr_len = sizeof(*addr);
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

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count, error_monitor_t em) {
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

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count, error_monitor_t em) {
	uint8_t addr_capacity = *addr_count;
	*addr_count = 0;

	ULONG ulOutBufLen = sizeof(FIXED_INFO);
	FIXED_INFO *pFixedInfo = malloc(ulOutBufLen);
	if (pFixedInfo == NULL)
		return -1;

	DWORD dwError = GetNetworkParams(pFixedInfo, &ulOutBufLen);
	if (dwError == ERROR_BUFFER_OVERFLOW && ulOutBufLen > sizeof(FIXED_INFO)) {
		pFixedInfo = (FIXED_INFO *)realloc(pFixedInfo, ulOutBufLen);
		dwError = GetNetworkParams(pFixedInfo, &ulOutBufLen);
	}
	if (dwError != ERROR_SUCCESS) {
		if (pFixedInfo != NULL)
			free(pFixedInfo);
		return -1;
	}

	IP_ADDR_STRING* pIPAddr = &pFixedInfo->DnsServerList;
	while (pIPAddr != NULL && addr_capacity) {
		socklen_t addr_len = sizeof(dnssevraddrs[0]);
		if (sock_ip_init((struct sockaddr *)&dnssevraddrs[*addr_count], &addr_len, pIPAddr->IpAddress.String, 0, NULL) == 0) {
			*addr_count = (*addr_count)++;
		}
		pIPAddr = pIPAddr->Next;
	}
	if (pFixedInfo != NULL)
		free(pFixedInfo);

	return 0;
}
#else
#include <errno.h>
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"

static int getdnssvraddrs_cfg_resolve(
    struct sockaddr_storage * dnssevraddrs, uint8_t addr_capacity, uint8_t * addr_count,
    const char * path, error_monitor_t em)
{
    FILE *fp = file_stream_open(path, "r", NULL);
    if (fp == NULL) {
        CPE_ERROR(
            em, "getdnssvraddrs: load from %s: open fail!, errno=%d (%s)",
            path, errno, strerror(errno));
        return -1;
    }

    int rv = 0;
    char * line = NULL;
    size_t data_len = 0;

    struct mem_buffer buffer;
    mem_buffer_init(&buffer, NULL);
    do {
        if (file_stream_read_line(&buffer, &line, &data_len, fp, NULL) != 0) {
            CPE_ERROR(
                em, "getdnssvraddrs: load from %s: read line fail, errno=%d (%s)",
                path, errno, strerror(errno));
            rv = -1;
            break;
        }

        if (line == NULL) break;

        if (cpe_str_start_with(line, "ns")) {
            //TODO:
            //if (net_dns_manage_load_ns_by_str(manage, driver, scope, cpe_str_trim_head(line + strlen("ns"))) != 0) rv = -1;
        }
        else if (cpe_str_start_with(line, "domain")) {
        }
        else if (cpe_str_start_with(line, "lookup")) {
        }
        else if (cpe_str_start_with(line, "search")) {
        }
        else if (cpe_str_start_with(line, "sortlist")) {
        }
        else if (cpe_str_start_with(line, "options")) {
        }
    } while(1);

    mem_buffer_clear(&buffer);
    file_stream_close(fp, em);
    
    return rv;
}

int getdnssvraddrs(struct sockaddr_storage * dnssevraddrs, uint8_t * addr_count, error_monitor_t em) {
    int rv = 0;
    uint8_t addr_capacity = *addr_count;
    *addr_count = 0;
    
    if (file_exist("/etc/resolv.conf", em)) {
        if (getdnssvraddrs_cfg_resolve(dnssevraddrs, addr_capacity, addr_count, "/etc/resolv.conf", em) != 0) rv = -1;
    }
    
    return rv;
}
#endif
