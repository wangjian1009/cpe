#ifndef CPE_PAL_DIRENT_H
#define CPE_PAL_DIRENT_H

#if defined _WIN32
#include "win32_dirent.h"
#else
#include <dirent.h>
#if defined ANDROID
# define readdir_r(__dir, __buf, __dp) ((*(__dp) = readdir(__dir)) == NULL ? -1 : 0)
#endif
#endif

#if defined _WIN32
# define MAXNAMLEN (256)
#endif

#endif
