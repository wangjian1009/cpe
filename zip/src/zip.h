#ifndef CPE_ZIP_H_INCLEDED
#define CPE_ZIP_H_INCLEDED
#include "cpe/vfs/vfs_types.h"
#include "zip_defs.h"

typedef voidp zipFile;

#define ZIP_OK (0)
#define ZIP_EOF (0)
#define ZIP_ERRNO (Z_ERRNO)
#define ZIP_PARAMERROR (-102)
#define ZIP_BADZIPFILE (-103)
#define ZIP_INTERNALERROR (-104)

typedef struct {
    cpe_zip_tm tmz_date; /* date in understandable format           */
    uLong dosDate; /* if dos_date == 0, tmu_date is used      */
    /*    uLong       flag;        */ /* general purpose bit flag        2 bytes */

    uLong internal_fa; /* internal file attributes        2 bytes */
    uLong external_fa; /* external file attributes        4 bytes */
} zip_fileinfo;

typedef const char * zipcharpc;

#define APPEND_STATUS_CREATE (0)
#define APPEND_STATUS_CREATEAFTER (1)
#define APPEND_STATUS_ADDINZIP (2)

zipFile cpe_zipOpen(vfs_mgr_t vfs, const char * pathname, int append);
zipFile cpe_zipOpen64(vfs_mgr_t vfs, const void * pathname, int append);
/*
  Create a zipfile.
     pathname contain on Windows XP a filename like "c:\\zlib\\zlib113.zip" or on
       an Unix computer "zlib/zlib113.zip".
     if the file pathname exist and append==APPEND_STATUS_CREATEAFTER, the zip
       will be created at the end of the file.
         (useful if the file contain a self extractor code)
     if the file pathname exist and append==APPEND_STATUS_ADDINZIP, we will
       add files in existing zip (be sure you don't add file that doesn't exist)
     If the zipfile cannot be opened, the return value is NULL.
     Else, the return value is a zipFile Handle, usable with other function
       of this zip package.
*/

/* Note : there is no delete function into a zipfile.
   If you want delete file into a zipfile, you must open a zipfile, and create another
   Of couse, you can use RAW reading and writing to copy the file you did not want delte
*/

zipFile cpe_zipOpen2(vfs_mgr_t vfs, const char * pathname, int append, zipcharpc * globalcomment);

zipFile cpe_zipOpen2_64(vfs_mgr_t vfs, const void * pathname, int append, zipcharpc * globalcomment);

int cpe_zipOpenNewFileInZip(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level);

int cpe_zipOpenNewFileInZip64(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int zip64);

/*
  Open a file in the ZIP for writing.
  filename : the filename in zip (if NULL, '-' without quote will be used
  *zipfi contain supplemental information
  if extrafield_local!=NULL and size_extrafield_local>0, extrafield_local
    contains the extrafield data the the local header
  if extrafield_global!=NULL and size_extrafield_global>0, extrafield_global
    contains the extrafield data the the local header
  if comment != NULL, comment contain the comment string
  method contain the compression method (0 for store, Z_DEFLATED for deflate)
  level contain the level of compression (can be Z_DEFAULT_COMPRESSION)
  zip64 is set to 1 if a zip64 extended information block should be added to the local file header.
                    this MUST be '1' if the uncompressed size is >= 0xffffffff.

*/

int cpe_zipOpenNewFileInZip2(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int raw);

int cpe_zipOpenNewFileInZip2_64(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int raw,
    int zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

int cpe_zipOpenNewFileInZip3(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int raw,
    int windowBits,
    int memLevel,
    int strategy,
    const char * password,
    uLong crcForCrypting);

int cpe_zipOpenNewFileInZip3_64(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int raw,
    int windowBits,
    int memLevel,
    int strategy,
    const char * password,
    uLong crcForCrypting,
    int zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

int cpe_zipOpenNewFileInZip4(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int raw,
    int windowBits,
    int memLevel,
    int strategy,
    const char * password,
    uLong crcForCrypting,
    uLong versionMadeBy,
    uLong flagBase);

int cpe_zipOpenNewFileInZip4_64(zipFile file,
    const char * filename,
    const zip_fileinfo * zipfi,
    const void * extrafield_local,
    uInt size_extrafield_local,
    const void * extrafield_global,
    uInt size_extrafield_global,
    const char * comment,
    int method,
    int level,
    int raw,
    int windowBits,
    int memLevel,
    int strategy,
    const char * password,
    uLong crcForCrypting,
    uLong versionMadeBy,
    uLong flagBase,
    int zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */

int cpe_zipWriteInFileInZip(zipFile file, const void * buf, unsigned len);
/*
  Write data in the zipfile
*/

int cpe_zipCloseFileInZip(zipFile file);
/*
  Close the current file in the zipfile
*/

int cpe_zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32);

int cpe_zipCloseFileInZipRaw64(zipFile file, ssize_t uncompressed_size, uLong crc32);

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
*/

int cpe_zipClose(zipFile file, const char * global_comment);
/*
  Close the zipfile
*/

int cpe_zipRemoveExtraInfoBlock(char * pData, int * dataLen, short sHeader);
/*
  zipRemoveExtraInfoBlock -  Added by Mathias Svensson

  Remove extra information block from a extra information data for the local file header or central directory header

  It is needed to remove ZIP64 extra information blocks when before data is written if using RAW mode.

  0x0001 is the signature header for the ZIP64 extra information blocks

  usage.
                        Remove ZIP64 Extra information from a central director extra field data
              zipRemoveExtraInfoBlock(pCenDirExtraFieldData, &nCenDirExtraFieldDataLen, 0x0001);

                        Remove ZIP64 Extra information from a Local File Header extra field data
        zipRemoveExtraInfoBlock(pLocalHeaderExtraFieldData, &nLocalHeaderExtraFieldDataLen, 0x0001);
*/

#endif
