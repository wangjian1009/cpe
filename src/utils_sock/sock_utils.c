#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_platform.h"
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
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
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
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
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
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
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
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }
}

static const char * sock_ipv4_parse(struct sockaddr * addr, socklen_t * addr_len, const char *str, error_monitor_t em) {
    const char * ch;
    uint8_t seen_digit_in_octet = 0;
    unsigned int  octets = 0;
    unsigned int  digit = 0;
    uint8_t  result[4];

    for (ch = str; *ch != '\0'; ch++) {
        switch (*ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            seen_digit_in_octet = 1;
            digit *= 10;
            digit += (*ch - '0');
            if (digit > 255) {
                goto parse_error;
            }
            break;

        case '.':
            if (octets == 3) {
                goto parse_error;
            }
            result[octets] = digit;
            digit = 0;
            octets++;
            seen_digit_in_octet = 0;
            break;

        default:
            goto parse_error;
        }
    }

    if (seen_digit_in_octet && octets == 3) {
        result[octets] = digit;
        if (*addr_len < sizeof(struct sockaddr_in)) {
            CPE_ERROR(em, "add len %d too small", (int)*addr_len);
            return NULL;
        }

        struct sockaddr_in * in_addr = (struct sockaddr_in*)addr;
        bzero(in_addr, sizeof(*in_addr));
        in_addr->sin_family = AF_INET;
        memcpy(&in_addr->sin_addr, result, sizeof(in_addr->sin_addr));
        return ch;
    }

parse_error:
    CPE_ERROR(em, "parse ipaddress %s fail", str);
    return NULL;
}

int sock_ipv4_init(struct sockaddr * addr, socklen_t * addr_len, const char *str, error_monitor_t em) {
    return sock_ipv4_parse(addr, addr_len, str, em) == NULL ? -1 : 0;
}

int sock_ipv6_init(struct sockaddr * addr, socklen_t * addr_len, const char *str, error_monitor_t em) {
    const char  *ch;

    uint16_t  digit = 0;
    unsigned int  before_count = 0;
    uint16_t  before_double_colon[8];
    uint16_t  after_double_colon[8];
    uint16_t  *dest = before_double_colon;

    unsigned int  digits_seen = 0;
    unsigned int  hextets_seen = 0;
    uint8_t  another_required = 1;
    uint8_t  digit_allowed = 1;
    uint8_t  colon_allowed = 1;
    uint8_t  double_colon_allowed = 1;
    uint8_t  just_saw_colon = 0;

    for (ch = str; *ch != '\0'; ch++) {
        switch (*ch) {
#define process_digit(base)                                             \
            /* Make sure a digit is allowed here. */                    \
                if (!digit_allowed) {                                   \
                    goto parse_error;                                   \
                }                                                       \
                /* If we've already seen 4 digits, it's a parse error. */ \
                if (digits_seen == 4) {                                 \
                    goto parse_error;                                   \
                }                                                       \
                                                                        \
                digits_seen++;                                          \
                colon_allowed = 1;                                   \
                just_saw_colon = 0;                                 \
                digit <<= 4;                                            \
                digit |= (*ch - (base));

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            process_digit('0');
            break;

        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            process_digit('a'-10);
            break;

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            process_digit('A'-10);
            break;

#undef process_digit

        case ':':
            /* We can only see a colon immediately after a hextet or as part
             * of a double-colon. */
            if (!colon_allowed) {
                goto parse_error;
            }

            /* If this is a double-colon, start parsing hextets into our
             * second array. */
            if (just_saw_colon) {
                colon_allowed = 0;
                digit_allowed = 1;
                another_required = 0;
                double_colon_allowed = 0;
                before_count = hextets_seen;
                dest = after_double_colon;
                continue;
            }

            /* If this would end the eighth hextet (regardless of the
             * placement of a double-colon), then there can't be a trailing
             * colon. */
            if (hextets_seen == 8) {
                goto parse_error;
            }

            /* If this is the very beginning of the string, then we can only
             * have a double-colon, not a single colon. */
            if (digits_seen == 0 && hextets_seen == 0) {
                colon_allowed = 1;
                digit_allowed = 0;
                just_saw_colon = 1;
                another_required = 1;
                continue;
            }

            /* Otherwise this ends the current hextet. */
            CPE_COPY_HTON16(dest, &digit);
            dest++;
            digit = 0;
            hextets_seen++;
            digits_seen = 0;
            colon_allowed = double_colon_allowed;
            just_saw_colon = 1;
            another_required = 1;
            break;

        case '.':
        {
            /* If we see a period, then we must be in the middle of an IPv4
             * address at the end of the IPv6 address. */
            struct cork_ipv4  *ipv4 = (struct cork_ipv4 *) dest;

            /* Ensure that we have space for the two hextets that the IPv4
             * address will take up. */
            if (hextets_seen >= 7) {
                goto parse_error;
            }

            /* Parse the IPv4 address directly into our current hextet
             * buffer. */
            ch = sock_ipv4_parse(addr, addr_len, ch - digits_seen, em);
            if (ch != NULL) {
                hextets_seen += 2;
                digits_seen = 0;
                another_required = 0;

                /* ch now points at the NUL terminator, but we're about to
                 * increment ch. */
                ch--;
                break;
            }

            /* The IPv4 parse failed, so we have an IPv6 parse error. */
            goto parse_error;
        }

        default:
            /* Any other character is a parse error. */
            goto parse_error;
        }
    }

    /* If we have a valid hextet at the end, and we've either seen a
     * double-colon, or we have eight hextets in total, then we've got a valid
     * final parse. */
    if (digits_seen > 0) {
        /* If there are trailing digits that would form a ninth hextet
         * (regardless of the placement of a double-colon), then we have a parse
         * error. */
        if (hextets_seen == 8) {
            goto parse_error;
        }

        CPE_COPY_HTON16(dest, &digit);
        dest++;
        hextets_seen++;
    }
    else if (another_required) {
        goto parse_error;
    }

    if (!double_colon_allowed) {
        /* We've seen a double-colon, so use 0000 for any hextets that weren't
         * present. */
        unsigned int  after_count = hextets_seen - before_count;
        if (*addr_len < sizeof(struct sockaddr_in6)) {
            CPE_ERROR(em, "address len %d too small", (int)*addr_len);
            return -1;
        }

        struct sockaddr_in6 * in6_addr = (struct sockaddr_in6*)addr;
        bzero(in6_addr, sizeof(*in6_addr));
        in6_addr->sin6_family = AF_INET6;
        memcpy(&in6_addr->sin6_addr, before_double_colon, sizeof(uint16_t) * before_count);
        memcpy(((uint16_t*)&in6_addr->sin6_addr) + (8 - after_count), after_double_colon, sizeof(uint16_t) * after_count);
        return 0;
    }
    else if (hextets_seen == 8) {
        unsigned int  after_count = hextets_seen - before_count;
        if (*addr_len < sizeof(struct sockaddr_in6)) {
            CPE_ERROR(em, "address len %d too small", (int)*addr_len);
            return -1;
        }

        struct sockaddr_in6 * in6_addr = (struct sockaddr_in6*)addr;
        bzero(in6_addr, sizeof(*in6_addr));
        in6_addr->sin6_family = AF_INET6;
        memcpy(&in6_addr->sin6_addr, before_double_colon, sizeof(uint16_t) * before_count);
        return 0;
    }

parse_error:
    CPE_ERROR(em, "Invalid IPv6 address: \"%s\"", str);
    return -1;
}

int sock_ip_init(struct sockaddr * addr, socklen_t * addr_len, const char *str, error_monitor_t em) {
    if (sock_ipv4_init(addr, addr_len, str, NULL) == 0) return 0;
    if (sock_ipv6_init(addr, addr_len, str, NULL) == 0) return 0;
    
    CPE_ERROR(em, "Invalid IP address: \"%s\"", str);
    return -1;
}
