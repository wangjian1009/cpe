#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_fcntl.h"
#include "cpe/pal/pal_string.h"

#if CPE_OS_ANDROID
#include <linux/tcp.h>
#endif

#if CPE_OS_LINUX
#include <netinet/tcp.h>
#endif

#if CPE_OS_MAC
#include <netinet/tcp.h>
#define SOL_TCP IPPROTO_TCP
#endif

#if _MSC_VER || __MINGW32__

#pragma comment(lib, "ws2_32.lib")

const char* cpe_sock_errstr(int n)
{
    /// <FIXME> not thread safe
    static char buf[1024];
    LPSTR errorText = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,  
        NULL,
        n,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errorText,  // output 
        0, // minimum size for output buffer
        NULL);   // arguments - see note 
    if ( NULL != errorText )
    {
        // ... do something with the string - log it, display it to the user, etc.
        strncpy_s(buf, sizeof(buf), errorText, strlen(errorText) + 1);
        // release memory allocated by FormatMessage()
        LocalFree(errorText);
        errorText = NULL;
        return buf;
    }

    return 0;
}

#endif

int cpe_sock_set_none_block(int fd, int is_non_block) {
#if _MSC_VER || __MINGW32__
    u_long flag;

    flag = is_non_block ? 1 : 0;
    return ioctlsocket(_get_osfhandle(fd), FIONBIO, &flag);
#else
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return -1;
    }

    if (is_non_block) {
        if (flags & O_NONBLOCK) {
            return 0;
        }
        else {
            flags |= O_NONBLOCK;
        }
    }
    else {
        if (!(flags & O_NONBLOCK)) {
            return 0;
        }
        else {
            flags &= ~O_NONBLOCK;
        }
    }

    return fcntl(fd, F_SETFL, flags);
#endif
}

int cpe_sock_set_no_sigpipe(int fd, int is_no_sigpipe) {
#if defined SO_NOSIGPIPE
	int opt = is_no_sigpipe ? 1 : 0;
	return setsockopt(fd , SOL_SOCKET , SO_NOSIGPIPE , &opt , sizeof(opt) );
#else
    return 0;
#endif
}

int cpe_sock_set_no_delay(int fd, int is_no_delay) {
#if _MSC_VER || __MINGW32__
    BOOL flag;

    flag = is_reuseaddr ? TRUE : FALSE;
    return setsockopt(_get_osfhandle(fd),  SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
#else
	int opt = is_no_delay ? 1 : 0;
	return setsockopt(fd , SOL_TCP , TCP_NODELAY , &opt , sizeof(opt));
#endif
}

int cpe_sock_set_reuseaddr(int fd, int is_reuseaddr) {
#if _MSC_VER || __MINGW32__
    BOOL flag;

    flag = is_reuseaddr ? TRUE : FALSE;
    return setsockopt(_get_osfhandle(fd),  SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
#else
	int opt = is_reuseaddr;
	return setsockopt( fd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt) );
#endif
}

int cpe_sock_set_reuseport(int fd, int is_reuseport) {
#if _MSC_VER || __MINGW32__
    BOOL flag;

    flag = is_reuseport ? TRUE : FALSE;
    return setsockopt(_get_osfhandle(fd),  SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
#else
#if defined CPE_OS_CYGWIN
    int opt = is_reuseport;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#else
    int opt = is_reuseport;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
#endif
}

int cpe_sock_set_recv_timeout(int fd, int timeout_ms) {
    struct timeval timeout;
	timeout.tv_sec = (int)(timeout_ms / 1000);
	timeout.tv_usec = timeout_ms - ((int)timeout.tv_sec * 1000);

    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(timeout));
}

int cpe_sock_set_send_timeout(int fd, int timeout_ms) {
    struct timeval timeout;
	timeout.tv_sec = (long)(timeout_ms / 1000);
	timeout.tv_usec = (timeout_ms - ((int)timeout.tv_sec * 1000));

    return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout, sizeof(timeout));
}

int cpe_sock_get_tcp_mss(int fd) {
#define NETWORK_MTU 1500
#define SS_TCP_MSS (NETWORK_MTU - 40)

    int _tcp_mss = SS_TCP_MSS;

    int mss = 0;
    socklen_t len = sizeof(mss);

#if defined(WIN32) || defined(_WIN32)
    getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, (char *)&mss, &len);
#else
    getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len);
#endif
    if (50 < mss && mss <= NETWORK_MTU) {
        _tcp_mss = (size_t) mss;
    }
    
    return _tcp_mss;
}
