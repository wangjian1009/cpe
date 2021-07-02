#ifndef CPE_PAL_UNISTD_H
#define CPE_PAL_UNISTD_H

#if defined __MINGW32__
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#define pipe(fds) _pipe(fds,4096, _O_BINARY)
extern pid_t w32_fork(void);
#define fork w32_fork
#elif defined _WIN32
#include <io.h>
#include <process.h>
#include <direct.h>
#define mkdir(__f, __m) _mkdir(__f)
#define rmdir _rmdir
#define unlink _unlink
#define getcwd _getcwd
#define chdir _chdir
#define fileno _fileno
#define pipe _pipe
#else
#include <unistd.h>
#endif

void closefrom(int lowfd);

#endif
