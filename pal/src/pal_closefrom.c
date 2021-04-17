#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_limits.h"
/* #include <sys/types.h> */
/* #include <sys/param.h> */
/* #include <stdio.h> */
/* #ifdef STDC_HEADERS */
/* # include <stdlib.h> */
/* # include <stddef.h> */
/* #else */
/* # ifdef HAVE_STDLIB_H */
/* #  include <stdlib.h> */
/* # endif */
/* #endif /\* STDC_HEADERS *\/ */
/* #ifdef HAVE_DIRENT_H */
/* # include <dirent.h> */
/* # define NAMLEN(dirent) strlen((dirent)->d_name) */
/* #else */
/* # define dirent direct */
/* # define NAMLEN(dirent) (dirent)->d_namlen */
/* # ifdef HAVE_SYS_NDIR_H */
/* #  include <sys/ndir.h> */
/* # endif */
/* # ifdef HAVE_SYS_DIR_H */
/* #  include <sys/dir.h> */
/* # endif */
/* # ifdef HAVE_NDIR_H */
/* #  include <ndir.h> */
/* # endif */
/* #endif */

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
