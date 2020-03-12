#ifdef _WIN32
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/win32_dirent.h"

/* Use the new safe string functions introduced in Visual Studio 2005 */
#if defined(_MSC_VER) && _MSC_VER >= 1400
#define DIRENT_STRNCPY(dest, src, size) strncpy_s((dest), (size), (src), _TRUNCATE)
#else
#define DIRENT_STRNCPY(dest, src, size) strncpy((dest), (src), (size))
#endif

/* Set errno variable */
#if defined(_MSC_VER)
#define DIRENT_SET_ERRNO(x) _set_errno(x)
#else
#define DIRENT_SET_ERRNO(x) (errno = (x))
#endif

/*****************************************************************************
 * Open directory stream DIRNAME for read and return a pointer to the
 * internal working area that is used to retrieve individual directory
 * entries.
 */
DIR * opendir(const char * dirname) {
    DIR * dirp;

    /* ensure that the resulting search pattern will be a valid file name */
    if (dirname == NULL) {
        DIRENT_SET_ERRNO(ENOENT);
        return NULL;
    }
    if (strlen(dirname) + 3 >= MAX_PATH) {
        DIRENT_SET_ERRNO(ENAMETOOLONG);
        return NULL;
    }

    /* construct new DIR structure */
    dirp = (DIR *)malloc(sizeof(struct DIR));
    if (dirp != NULL) {
        int error;

        /*
       * Convert relative directory name to an absolute one.  This
       * allows rewinddir() to function correctly when the current working
       * directory is changed between opendir() and rewinddir().
       */
        if (GetFullPathNameA(dirname, MAX_PATH, dirp->patt, NULL)) {
            char * p;

            /* append the search pattern "\\*\0" to the directory name */
            p = strchr(dirp->patt, '\0');
            if (dirp->patt < p && *(p - 1) != '\\' && *(p - 1) != ':') {
                *p++ = '\\';
            }
            *p++ = '*';
            *p = '\0';

            /* open directory stream and retrieve the first entry */
            dirp->search_handle = FindFirstFileA(dirp->patt, &dirp->find_data);
            if (dirp->search_handle != INVALID_HANDLE_VALUE) {
                /* a directory entry is now waiting in memory */
                dirp->cached = 1;
                error = 0;
            } else {
                /* search pattern is not a directory name? */
                DIRENT_SET_ERRNO(ENOENT);
                error = 1;
            }
        } else {
            /* buffer too small */
            DIRENT_SET_ERRNO(ENOMEM);
            error = 1;
        }

        if (error) {
            free(dirp);
            dirp = NULL;
        }
    }

    return dirp;
}

/*****************************************************************************
 * Read a directory entry, and return a pointer to a dirent structure
 * containing the name of the entry in d_name field.  Individual directory
 * entries returned by this very function include regular files,
 * sub-directories, pseudo-directories "." and "..", but also volume labels,
 * hidden files and system files may be returned.
 */

struct dirent * readdir(DIR * dirp) {
    struct dirent * r = NULL;
    int rv = readdir_r(dirp, &dirp->curentry, &r);
    return rv == 0 ? r : NULL;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
    DWORD attr;
    if (dirp == NULL) {
        /* directory stream did not open */
        DIRENT_SET_ERRNO(EBADF);
        return -1;
    }

    /* get next directory entry */
    if (dirp->cached != 0) {
        /* a valid directory entry already in memory */
        dirp->cached = 0;
    } else {
        /* get the next directory entry from stream */
        if (dirp->search_handle == INVALID_HANDLE_VALUE) {
            *result = NULL;
            return 0;
        }
        if (FindNextFileA(dirp->search_handle, &dirp->find_data) == FALSE) {
            DWORD err = GetLastError();

            FindClose(dirp->search_handle);
            dirp->search_handle = INVALID_HANDLE_VALUE;
            *result = NULL;
            return err == ERROR_NO_MORE_FILES ? 0 : -1;
        }
    }

    /* copy as a multibyte character string */
    DIRENT_STRNCPY(entry->d_name, dirp->find_data.cFileName, sizeof(entry->d_name));
    entry->d_name[MAX_PATH] = '\0';

    /* compute the length of name */
    entry->d_namlen = strlen(entry->d_name);

    /* determine file type */
    attr = dirp->find_data.dwFileAttributes;
    if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
        entry->d_type = DT_CHR;
    } else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        entry->d_type = DT_DIR;
    } else {
        entry->d_type = DT_REG;
    }

    *result = entry;
    return 0;
}

/*****************************************************************************
 * Close directory stream opened by opendir() function.  Close of the
 * directory stream invalidates the DIR structure as well as any previously
 * read directory entry.
 */
int closedir(DIR * dirp) {
    if (dirp == NULL) {
        /* invalid directory stream */
        DIRENT_SET_ERRNO(EBADF);
        return -1;
    }

    /* release search handle */
    if (dirp->search_handle != INVALID_HANDLE_VALUE) {
        FindClose(dirp->search_handle);
        dirp->search_handle = INVALID_HANDLE_VALUE;
    }

    /* release directory structure */
    free(dirp);
    return 0;
}

/*****************************************************************************
 * Resets the position of the directory stream to which dirp refers to the
 * beginning of the directory.  It also causes the directory stream to refer
 * to the current state of the corresponding directory, as a call to opendir()
 * would have done.  If dirp does not refer to a directory stream, the effect
 * is undefined.
 */
void rewinddir(DIR * dirp) {
    if (dirp != NULL) {
        /* release search handle */
        if (dirp->search_handle != INVALID_HANDLE_VALUE) {
            FindClose(dirp->search_handle);
        }

        /* open new search handle and retrieve the first entry */
        dirp->search_handle = FindFirstFileA(dirp->patt, &dirp->find_data);
        if (dirp->search_handle != INVALID_HANDLE_VALUE) {
            /* a directory entry is now waiting in memory */
            dirp->cached = 1;
        } else {
            /* failed to re-open directory: no directory entry in memory */
            dirp->cached = 0;
        }
    }
}

#endif
