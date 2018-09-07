#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_fcntl.h"
#include "cpe/pal/pal_string.h"

#ifdef _MSC_VER

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
int cpe_sock_close(int fd)
{
    return closesocket(_get_osfhandle(fd));
}
#endif

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
#ifdef _MSC_VER
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
#if defined _MSC_VER
    return 0;
#else
#if defined CPE_OS_LINUX || defined CPE_OS_CYGWIN
    return 0;
#else
	int opt = is_no_sigpipe ? 1 : 0;
	return setsockopt(fd , SOL_SOCKET , SO_NOSIGPIPE , &opt , sizeof(opt) );
#endif
#endif
}


int cpe_sock_set_reuseaddr(int fd, int is_reuseaddr) {
#ifdef _MSC_VER
    BOOL flag;

    flag = is_reuseaddr ? TRUE : FALSE;
    return setsockopt(_get_osfhandle(fd),  SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag));
#else
	int opt = is_reuseaddr;
	return setsockopt( fd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt) );
#endif
}

int cpe_sock_set_reuseport(int fd, int is_reuseport) {
#ifdef _MSC_VER
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

