#ifndef CPE_PAL_SOCKET_H
#define CPE_PAL_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#if defined _WINDOWS_
#error Do not include winsock.h or windows.h before pal_socket.h.
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <errno.h>
#include <io.h>

#ifdef EINPROGRESS
#undef EINPROGRESS
#endif
#define EINPROGRESS WSAEINPROGRESS

#ifdef EWOULDBLOCK
#undef EWOULDBLOCK
#endif
#define EWOULDBLOCK WSAEWOULDBLOCK

//extern int cpe_socket_open(int af, int type, int protocol);
#define cpe_sock_open(_af, _type, _protocol) (_open_osfhandle(socket((_af), (_type), (_protocol)), 0))
#define cpe_connect(_fd, _name, _namelen) (connect(_get_osfhandle(_fd), (_name), (_namelen)))
#define cpe_getsockopt(_fd, _level, _optname, _optval, _optlen) (getsockopt(_get_osfhandle(_fd), _level, _optname, _optval, _optlen))
#define cpe_getsockname(_fd, _name, _namelen) (getsockname (_get_osfhandle(_fd), _name, _namelen))
#define cpe_getpeername(_fd, _name, _namelen) (getpeername (_get_osfhandle(_fd), _name, _namelen))
#define cpe_bind(_fd, _addr, _namelen) (bind(_get_osfhandle(_fd), _addr,_namelen))
#define cpe_listen(_fd, _backlog) (listen(_get_osfhandle(_fd), _backlog))
#define cpe_accept(_fd, _addr, _addrlen) (_open_osfhandle(accept(_get_osfhandle(_fd), _addr,_addrlen), 0))
#define cpe_recv(_fd, _buf, _len, _flags) (recv(_get_osfhandle(_fd), _buf, _len, _flags))
#define cpe_recvfrom(_fd, _buf, _len,_flags, _from, _fromlen) (recvfrom(_get_osfhandle(_fd), _buf, _len,_flags, _from, _fromlen))
#define cpe_send(_fd, _buf, _len, _flags) (send(_get_osfhandle(_fd), _buf, _len, _flags))
#define cpe_sendto(_fd, _buf, _len, _flags, _to, _tolen) (sendto(_get_osfhandle(_fd), _buf, _len, _flags, _to, _tolen))
#define cpe_sock_close _close
#define cpe_sock_shutdown(_fd, _how) shutdown(_get_osfhandle(_fd), (_how))

#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
    
#define cpe_sock_errno() WSAGetLastError()
extern const char *cpe_sock_errstr(int n);

#else

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define cpe_sock_pair socketpair
#define cpe_sock_open socket
#define cpe_connect connect
#define cpe_getsockopt getsockopt
#define cpe_getsockname getsockname
#define cpe_getpeername getpeername
#define cpe_bind bind
#define cpe_listen listen
#define cpe_accept accept
#define cpe_recv recv
#define cpe_recvfrom recvfrom
#define cpe_send send
#define cpe_sendto sendto

#define cpe_sock_close close
#define cpe_sock_shutdown shutdown    
#define cpe_sock_errno() errno
#define cpe_sock_errstr(n) strerror(n)

#endif

int cpe_sock_set_none_block(int fd, int is_non_block);
int cpe_sock_set_reuseaddr(int fd, int is_reuseaddr);
int cpe_sock_set_reuseport(int socket, int is_reuseport);
int cpe_sock_set_no_sigpipe(int fd, int is_no_sigpipe);
int cpe_sock_set_no_delay(int fd, int is_no_delay);
int cpe_sock_set_recv_timeout(int fd, int timeout_ms);
int cpe_sock_set_send_timeout(int fd, int timeout_ms);
int cpe_sock_get_tcp_mss(int fd);
    
#ifdef MSG_NOSIGNAL
#define CPE_SOCKET_DEFAULT_SEND_FLAGS MSG_NOSIGNAL
#else
#define CPE_SOCKET_DEFAULT_SEND_FLAGS 0
#endif

#ifdef __cplusplus
}
#endif

#endif

