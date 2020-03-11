#ifndef CPE_UNZIP_H_INCLEDED
#define CPE_UNZIP_H_INCLEDED
#include "cpe/utils/utils_types.h"
#include "cpe/vfs/vfs_types.h"
#include "zip_defs.h"

typedef voidp unzFile;

#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
typedef struct unz_global_info64_s {
    ssize_t number_entry;         /* total number of entries in the central dir on this disk */
    uLong size_comment;         /* size of the global comment of the zipfile */
} unz_global_info64;

typedef struct unz_global_info_s {
    uLong number_entry;         /* total number of entries in
                                     the central dir on this disk */
    uLong size_comment;         /* size of the global comment of the zipfile */
} unz_global_info;

/* unz_file_info contain information about a file in the zipfile */
typedef struct unz_file_info64_s {
    uLong version;              /* version made by                 2 bytes */
    uLong version_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_method;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                  /* crc-32                          4 bytes */
    ssize_t compressed_size;   /* compressed size                 8 bytes */
    ssize_t uncompressed_size; /* uncompressed size               8 bytes */
    uLong size_filename;        /* filename length                 2 bytes */
    uLong size_file_extra;      /* extra field length              2 bytes */
    uLong size_file_comment;    /* file comment length             2 bytes */

    uLong disk_num_start;       /* disk number start               2 bytes */
    uLong internal_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    cpe_zip_tm tmu_date;
} unz_file_info64;

typedef struct unz_file_info_s {
    uLong version;              /* version made by                 2 bytes */
    uLong version_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_method;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                  /* crc-32                          4 bytes */
    uLong compressed_size;      /* compressed size                 4 bytes */
    uLong uncompressed_size;    /* uncompressed size               4 bytes */
    uLong size_filename;        /* filename length                 2 bytes */
    uLong size_file_extra;      /* extra field length              2 bytes */
    uLong size_file_comment;    /* file comment length             2 bytes */

    uLong disk_num_start;       /* disk number start               2 bytes */
    uLong internal_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    cpe_zip_tm tmu_date;
} unz_file_info;

unzFile cpe_unzOpen(error_monitor_t em, vfs_mgr_t vfs, const char *path);
unzFile cpe_unzOpen64(error_monitor_t em, vfs_mgr_t vfs, const char * path);
/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
     "zlib/zlib113.zip".
     If the zipfile cannot be opened (file don't exist or in not valid), the
       return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
     the "64" function take a const void* pointer, because the path is just the
       value passed to the open64_file_func callback.
     Under Windows, if UNICODE is defined, using fill_fopen64_filefunc, the path
       is a pointer to a wide unicode string (LPCTSTR is LPCWSTR), so const char*
       does not describe the reality
*/

int cpe_unzClose(unzFile file);
/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */

int cpe_unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info);
int cpe_unzGetGlobalInfo64(unzFile file, unz_global_info64 *pglobal_info);
/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */


int cpe_unzGetGlobalComment(unzFile file, char *szComment, uLong uSizeBuf);
/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/


/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

int cpe_unzGoToFirstFile(unzFile file);
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

int cpe_unzGoToNextFile(unzFile file);
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

int cpe_unzLocateFile(unzFile file, const char *szFileName, int iCaseSensitivity);
/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/


/* ****************************************** */
/* Ryan supplied functions */
/* unz_file_info contain information about a file in the zipfile */
typedef struct unz_file_pos_s {
    uLong pos_in_zip_directory;   /* offset in zip file directory */
    uLong num_of_file;            /* # of file */
} unz_file_pos;

int cpe_unzGetFilePos(unzFile file, unz_file_pos* file_pos);

int cpe_unzGoToFilePos(unzFile file, unz_file_pos* file_pos);

typedef struct unz64_file_pos_s {
    ssize_t pos_in_zip_directory;   /* offset in zip file directory */
    ssize_t num_of_file;            /* # of file */
} unz64_file_pos;

int cpe_unzGetFilePos64(unzFile file, unz64_file_pos* file_pos);
int cpe_unzGoToFilePos64(unzFile file, const unz64_file_pos* file_pos);

/* ****************************************** */

int cpe_unzGetCurrentFileInfo64(
    unzFile file,
    unz_file_info64 *pfile_info,
    char *szFileName,
    uLong fileNameBufferSize,
    void *extraField,
    uLong extraFieldBufferSize,
    char *szComment,
    uLong commentBufferSize);

int cpe_unzGetCurrentFileInfo(
    unzFile file,
    unz_file_info *pfile_info,
    char *szFileName,
    uLong fileNameBufferSize,
    void *extraField,
    uLong extraFieldBufferSize,
    char *szComment,
    uLong commentBufferSize);
/*
  Get Info about the current file
  if pfile_info!=NULL, the *pfile_info structure will contain somes info about
        the current file
  if szFileName!=NULL, the filemane string will be copied in szFileName
            (fileNameBufferSize is the size of the buffer)
  if extraField!=NULL, the extra field information will be copied in extraField
            (extraFieldBufferSize is the size of the buffer).
            This is the Central-header version of the extra field
  if szComment!=NULL, the comment string of the file will be copied in szComment
            (commentBufferSize is the size of the buffer)
*/


/** Addition for GDAL : START */

ssize_t cpe_unzGetCurrentFileZStreamPos64(unzFile file);

/** Addition for GDAL : END */


/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
   */

int cpe_unzOpenCurrentFile(unzFile file);
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

int cpe_unzOpenCurrentFilePassword(unzFile file, const char* password);
/*
  Open for reading data the current file in the zipfile.
  password is a crypting password
  If there is no error, the return value is UNZ_OK.
*/

int cpe_unzOpenCurrentFile2(unzFile file, int* method, int* level, int raw);
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as NULL (if you did not want known level,
         but you CANNOT set method parameter as NULL
*/

int cpe_unzOpenCurrentFile3(
    unzFile file, int* method, int* level, int raw, const char* password);
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as NULL (if you did not want known level,
         but you CANNOT set method parameter as NULL
*/


int cpe_unzCloseCurrentFile(unzFile file);
/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/

int cpe_unzReadCurrentFile(unzFile file, voidp buf, unsigned len);
/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

z_off_t cpe_unztell(unzFile file);

ssize_t cpe_unztell64(unzFile file);
/*
  Give the current position in uncompressed data
*/

int cpe_unzeof(unzFile file);
/*
  return 1 if the end of file was reached, 0 elsewhere
*/

int cpe_unzGetLocalExtrafield(unzFile file, voidp buf, unsigned len);
/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
    more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
  the return value is the number of bytes copied in buf, or (if <0)
    the error code
*/

/***************************************************************************/

/* Get the current file offset */
ssize_t cpe_unzGetOffset64(unzFile file);
uLong cpe_unzGetOffset(unzFile file);

/* Set the current file offset */
int cpe_unzSetOffset64(unzFile file, ssize_t pos);
int cpe_unzSetOffset(unzFile file, uLong pos);

#endif
