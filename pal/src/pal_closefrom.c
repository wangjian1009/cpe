#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_limits.h"
#include "cpe/pal/pal_dirent.h"

#if CPE_OS_LINUX
#define HAVE_DIRFD
#endif

#if CPE_OS_ANDROID
#define HAVE_SYSCONF
#endif

void closefrom(int lowfd) {
    long fd, maxfd;
#ifdef HAVE_DIRFD
    char fdpath[PATH_MAX], *endp;
    struct dirent *dent;
    DIR *dirp;
    int len;

    /* Check for a /proc/$$/fd directory. */
    len = snprintf(fdpath, sizeof(fdpath), "/proc/%ld/fd", (long)getpid());
    if (len != -1 && len <= sizeof(fdpath) && (dirp = opendir(fdpath))) {
	while ((dent = readdir(dirp)) != NULL) {
	    fd = strtol(dent->d_name, &endp, 10);
	    if (dent->d_name != endp && *endp == '\0' &&
		fd >= 0 && fd < INT_MAX && fd >= lowfd && fd != dirfd(dirp))
		(void) close((int) fd);
	}
	(void) closedir(dirp);
    } else
#endif
    {
	/*
	 * Fall back on sysconf() or getdtablesize().  We avoid checking
	 * resource limits since it is possible to open a file descriptor
	 * and then drop the rlimit such that it is below the open fd.
	 */
#ifdef HAVE_SYSCONF
	maxfd = sysconf(_SC_OPEN_MAX);
#else
	//maxfd = getdtablesize();
#endif /* HAVE_SYSCONF */
	/* if (maxfd < 0) */
	/*     maxfd = OPEN_MAX; */

	for (fd = lowfd; fd < maxfd; fd++)
	    (void) close((int) fd);
    }
}
