/* unzip.c -- IO for uncompress .zip files using zlib
   Version 1.1, February 14h, 2010
   part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

         For more info read MiniZip_info.txt


  ------------------------------------------------------------------------------------
  Decryption code comes from crypt.c by Info-ZIP but has been greatly reduced in terms of
  compatibility with older software. The following is from the original crypt.c.
  Code woven in by Terry Thorsen 1/2003.

  Copyright (c) 1990-2000 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html

        crypt.c (full version) by Info-ZIP.      Last revised:  [see crypt.h]

  The encryption/decryption parts of this source code (as opposed to the
  non-echoing password parts) were originally written in Europe.  The
  whole source package can be freely distributed, including from the USA.
  (Prior to January 2000, re-export from the US was a violation of US law.)

        This encryption code is a direct transcription of the algorithm from
  Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
  file (appnote.txt) is distributed with the PKZIP program (even in the
  version without encryption capabilities).

        ------------------------------------------------------------------------------------

        Changes in unzip.c

        2007-2008 - Even Rouault - Addition of cpl_unzGetCurrentFileZStreamPos
  2007-2008 - Even Rouault - Decoration of symbol names unz* -> cpl_unz*
  2007-2008 - Even Rouault - Remove old C style function prototypes
  2007-2008 - Even Rouault - Add unzip support for ZIP64

        Copyright (C) 2007-2008 Even Rouault


        Oct-2009 - Mathias Svensson - Removed cpl_* from symbol names (Even Rouault added them but since this is now moved to a new project (minizip64) I renamed them again).
  Oct-2009 - Mathias Svensson - Fixed problem if uncompressed size was > 4G and compressed size was <4G
                                should only read the compressed/uncompressed size from the Zip64 format if
                                the size from normal header was 0xFFFFFFFF
  Oct-2009 - Mathias Svensson - Applied some bug fixes from paches recived from Gilles Vollant
        Oct-2009 - Mathias Svensson - Applied support to unzip files with compression mathod BZIP2 (bzip2 lib is required)
                                Patch created by Daniel Borca

  Jan-2010 - back to unzip and minizip 1.0 name scheme, with compatibility layer

  Copyright (C) 1998 - 2010 Gilles Vollant, Even Rouault, Mathias Svensson

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include "zip_pro_unzip.h"
#include "cpe/pal/pal_errno.h"
#include "cpe/pal/pal_limits.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_file.h"

/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef CASESENSITIVITYDEFAULT_NO
#if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES)
#define CASESENSITIVITYDEFAULT_NO
#endif
#endif

#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
#define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
#define TRYFREE(p)      \
    {                   \
        if (p) free(p); \
    }
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)

static const char unz_copyright[] = " unzip 1.01 Copyright 1998-2004 Gilles Vollant - http://www.winimage.com/zLibDll";

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct unz_file_info64_internal_s {
    ssize_t offset_curfile; /* relative offset of local header 8 bytes */
} unz_file_info64_internal;

/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
typedef struct {
    char * read_buffer; /* internal buffer for compressed data */
    z_stream stream; /* zLib stream structure for inflate */

#ifdef HAVE_BZIP2
    bz_stream bstream; /* bzLib stream structure for bziped */
#endif

    ssize_t pos_in_zipfile; /* position in byte on the zipfile, for fseek*/
    uLong stream_initialised; /* flag set if stream structure is initialised*/

    ssize_t offset_local_extrafield; /* offset of the local extra field */
    uInt size_local_extrafield; /* size of the local extra field */
    ssize_t pos_local_extrafield; /* position in the local extra field in read*/
    ssize_t total_out_64;

    uLong crc32; /* crc32 of all data uncompressed */
    uLong crc32_wait; /* crc32 we must obtain after decompress all */
    ssize_t rest_read_compressed; /* number of byte to be decompressed */
    ssize_t rest_read_uncompressed; /*number of byte to be obtained after decomp*/
    vfs_file_t filestream; /* io structore of the zipfile */
    uLong compression_method; /* compression method (0==store) */
    ssize_t byte_before_the_zipfile; /* byte before the zipfile, (>0 for sfx)*/
    int raw;
} file_in_zip64_read_info_s;

/* unz64_s contain internal information about the zipfile
*/
typedef struct {
    int is64bitOpenFunction;
    error_monitor_t em;
    vfs_file_t filestream; /* io structore of the zipfile */
    unz_global_info64 gi; /* public global information */
    ssize_t byte_before_the_zipfile; /* byte before the zipfile, (>0 for sfx)*/
    ssize_t num_file; /* number of the current file in the zipfile*/
    ssize_t pos_in_central_dir; /* pos of the current file in the central dir*/
    ssize_t current_file_ok; /* flag about the usability of the current file*/
    ssize_t central_pos; /* position of the beginning of the central dir*/

    ssize_t size_central_dir; /* size of the central directory  */
    ssize_t offset_central_dir; /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_internal cur_file_info_internal; /* private info about it*/
    file_in_zip64_read_info_s * pfile_in_zip_read; /* structure about the current
                                        file if we are decompressing it */
    int encrypted;

    int isZip64;

    unsigned long keys[3]; /* keys defining the pseudo-random sequence */
    const z_crc_t * pcrc_32_tab;
} unz64_s;

#include "zip_pro_crypt.h"

/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/

int cpe_unz64local_getByte(vfs_file_t filestream, int * pi) {
    unsigned char c;
    int err = (int)vfs_file_read(filestream, &c, 1);
    if (err == 1) {
        *pi = (int)c;
        return UNZ_OK;
    } else {
        if (vfs_file_error(filestream))
            return UNZ_ERRNO;
        else
            return UNZ_EOF;
    }
}

/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
int cpe_unz64local_getShort(vfs_file_t filestream, uLong * pX) {
    uLong x;
    int i = 0;
    int err;

    err = cpe_unz64local_getByte(filestream, &i);
    x = (uLong)i;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((uLong)i) << 8;

    if (err == UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

int cpe_unz64local_getLong(vfs_file_t filestream, uLong * pX) {
    uLong x;
    int i = 0;
    int err;

    err = cpe_unz64local_getByte(filestream, &i);
    x = (uLong)i;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((uLong)i) << 8;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((uLong)i) << 16;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x += ((uLong)i) << 24;

    if (err == UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

int cpe_unz64local_getLong64(vfs_file_t filestream, ssize_t * pX) {
    ssize_t x;
    int i = 0;
    int err;

    err = cpe_unz64local_getByte(filestream, &i);
    x = (ssize_t)i;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 8;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 16;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 24;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 32;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 40;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 48;

    if (err == UNZ_OK)
        err = cpe_unz64local_getByte(filestream, &i);
    x |= ((ssize_t)i) << 56;

    if (err == UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
ssize_t cpe_unz64local_SearchCentralDir(vfs_file_t filestream) {
    unsigned char * buf;
    ssize_t uSizeFile;
    ssize_t uBackRead;
    ssize_t uMaxBack = 0xffff; /* maximum size of global comment */
    ssize_t uPosFound = 0;

    if (vfs_file_seek(filestream, 0, vfs_file_seek_end) != 0)
        return 0;

    uSizeFile = vfs_file_tell(filestream);

    if (uMaxBack > uSizeFile)
        uMaxBack = uSizeFile;

    buf = (unsigned char *)ALLOC(BUFREADCOMMENT + 4);
    if (buf == NULL)
        return 0;

    uBackRead = 4;
    while (uBackRead < uMaxBack) {
        uLong uReadSize;
        ssize_t uReadPos;
        int i;
        if (uBackRead + BUFREADCOMMENT > uMaxBack)
            uBackRead = uMaxBack;
        else
            uBackRead += BUFREADCOMMENT;
        uReadPos = uSizeFile - uBackRead;

        uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ? (BUFREADCOMMENT + 4) : (uLong)(uSizeFile - uReadPos);
        if (vfs_file_seek(filestream, uReadPos, vfs_file_seek_set) != 0)
            break;

        if (vfs_file_read(filestream, buf, uReadSize) != uReadSize)
            break;

        for (i = (int)uReadSize - 3; (i--) > 0;)
            if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) && ((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3)) == 0x06)) {
                uPosFound = uReadPos + i;
                break;
            }

        if (uPosFound != 0)
            break;
    }
    TRYFREE(buf);
    return uPosFound;
}

/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
ssize_t cpe_unz64local_SearchCentralDir64(vfs_file_t filestream) {
    unsigned char * buf;
    ssize_t uSizeFile;
    ssize_t uBackRead;
    ssize_t uMaxBack = 0xffff; /* maximum size of global comment */
    ssize_t uPosFound = 0;
    uLong uL;
    ssize_t relativeOffset;

    if (vfs_file_seek(filestream, 0, vfs_file_seek_end) != 0)
        return 0;

    uSizeFile = vfs_file_tell(filestream);

    if (uMaxBack > uSizeFile)
        uMaxBack = uSizeFile;

    buf = (unsigned char *)ALLOC(BUFREADCOMMENT + 4);
    if (buf == NULL)
        return 0;

    uBackRead = 4;
    while (uBackRead < uMaxBack) {
        uLong uReadSize;
        ssize_t uReadPos;
        int i;
        if (uBackRead + BUFREADCOMMENT > uMaxBack)
            uBackRead = uMaxBack;
        else
            uBackRead += BUFREADCOMMENT;
        uReadPos = uSizeFile - uBackRead;

        uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ? (BUFREADCOMMENT + 4) : (uLong)(uSizeFile - uReadPos);
        if (vfs_file_seek(filestream, uReadPos, vfs_file_seek_set) != 0)
            break;

        if (vfs_file_read(filestream, buf, uReadSize) != uReadSize)
            break;

        for (i = (int)uReadSize - 3; (i--) > 0;)
            if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) && ((*(buf + i + 2)) == 0x06) && ((*(buf + i + 3)) == 0x07)) {
                uPosFound = uReadPos + i;
                break;
            }

        if (uPosFound != 0)
            break;
    }
    TRYFREE(buf);
    if (uPosFound == 0)
        return 0;

    /* Zip64 end of central directory locator */
    if (vfs_file_seek(filestream, uPosFound, vfs_file_seek_set) != 0)
        return 0;

    /* the signature, already checked */
    if (cpe_unz64local_getLong(filestream, &uL) != UNZ_OK)
        return 0;

    /* number of the disk with the start of the zip64 end of  central directory */
    if (cpe_unz64local_getLong(filestream, &uL) != UNZ_OK)
        return 0;
    if (uL != 0)
        return 0;

    /* relative offset of the zip64 end of central directory record */
    if (cpe_unz64local_getLong64(filestream, &relativeOffset) != UNZ_OK)
        return 0;

    /* total number of disks */
    if (cpe_unz64local_getLong(filestream, &uL) != UNZ_OK)
        return 0;
    if (uL != 1)
        return 0;

    /* Goto end of central directory record */
    if (vfs_file_seek(filestream, relativeOffset, vfs_file_seek_set) != 0)
        return 0;

    /* the signature */
    if (cpe_unz64local_getLong(filestream, &uL) != UNZ_OK)
        return 0;

    if (uL != 0x06064b50)
        return 0;

    return relativeOffset;
}

/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer
     "zlib/zlib114.zip".
     If the zipfile cannot be opened (file doesn't exist or in not valid), the
       return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
*/
unzFile cpe_unzOpenInternal(error_monitor_t em, vfs_mgr_t vfs, const void * path, int is64bitOpenFunction) {
    uLong uL;

    uLong number_disk; /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uLong number_disk_with_CD; /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    ssize_t number_entry_CD; /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    int err = UNZ_OK;

    if (unz_copyright[0] != ' ') {
        CPE_ERROR(em, "unzip: open file %s fail, validate copyright fail", (const char *)path);
        return NULL;
    }

    unz64_s us;
    us.is64bitOpenFunction = is64bitOpenFunction;
    us.em = em;
    us.filestream = vfs_file_open(vfs, path, "r+b");
    if (us.filestream == NULL) {
        CPE_ERROR(em, "unzip: open file %s fail, error=%d (%s)", (const char *)path, errno, strerror(errno));
        return NULL;
    }

    ssize_t central_pos = cpe_unz64local_SearchCentralDir64(us.filestream);
    if (central_pos) {
        uLong uS;
        ssize_t uL64;

        us.isZip64 = 1;

        if (vfs_file_seek(us.filestream, central_pos, vfs_file_seek_set) != 0) {
            err = UNZ_ERRNO;
        }

        /* the signature, already checked */
        if (cpe_unz64local_getLong(us.filestream, &uL) != UNZ_OK) {
            err = UNZ_ERRNO;
        }

        /* size of zip64 end of central directory record */
        if (cpe_unz64local_getLong64(us.filestream, &uL64) != UNZ_OK) {
            err = UNZ_ERRNO;
        }

        /* version made by */
        if (cpe_unz64local_getShort(us.filestream, &uS) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        /* version needed to extract */
        if (cpe_unz64local_getShort(us.filestream, &uS) != UNZ_OK) {
            err = UNZ_ERRNO;
        }

        /* number of this disk */
        if (cpe_unz64local_getLong(us.filestream, &number_disk) != UNZ_OK) {
            err = UNZ_ERRNO;
        }

        /* number of the disk with the start of the central directory */
        if (cpe_unz64local_getLong(us.filestream, &number_disk_with_CD) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        /* total number of entries in the central directory on this disk */
        if (cpe_unz64local_getLong64(us.filestream, &us.gi.number_entry) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        /* total number of entries in the central directory */
        if (cpe_unz64local_getLong64(us.filestream, &number_entry_CD) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        if ((number_entry_CD != us.gi.number_entry) || (number_disk_with_CD != 0) || (number_disk != 0)) {
            err = UNZ_BADZIPFILE;
        }

        /* size of the central directory */
        if (cpe_unz64local_getLong64(us.filestream, &us.size_central_dir) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        /* offset of start of central directory with respect to the
          starting disk number */
        if (cpe_unz64local_getLong64(us.filestream, &us.offset_central_dir) != UNZ_OK) {
            err = UNZ_ERRNO;
        }

        us.gi.size_comment = 0;
    } else {
        central_pos = cpe_unz64local_SearchCentralDir(us.filestream);
        if (central_pos == 0) {
            err = UNZ_ERRNO;
        }
        
        us.isZip64 = 0;

        if (vfs_file_seek(us.filestream, central_pos, vfs_file_seek_set) != 0) {
            err = UNZ_ERRNO;
        }

        /* the signature, already checked */
        if (cpe_unz64local_getLong(us.filestream, &uL) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        /* number of this disk */
        if (cpe_unz64local_getShort(us.filestream, &number_disk) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        
        /* number of the disk with the start of the central directory */
        if (cpe_unz64local_getShort(us.filestream, &number_disk_with_CD) != UNZ_OK) {
            err = UNZ_ERRNO;
        }

        /* total number of entries in the central dir on this disk */
        if (cpe_unz64local_getShort(us.filestream, &uL) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        us.gi.number_entry = uL;

        /* total number of entries in the central dir */
        if (cpe_unz64local_getShort(us.filestream, &uL) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        number_entry_CD = uL;

        if ((number_entry_CD != us.gi.number_entry) || (number_disk_with_CD != 0) || (number_disk != 0)) {
            err = UNZ_BADZIPFILE;
        }
        
        /* size of the central directory */
        if (cpe_unz64local_getLong(us.filestream, &uL) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        us.size_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        if (cpe_unz64local_getLong(us.filestream, &uL) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
        us.offset_central_dir = uL;

        /* zipfile comment length */
        if (cpe_unz64local_getShort(us.filestream, &us.gi.size_comment) != UNZ_OK) {
            err = UNZ_ERRNO;
        }
    }

    if ((central_pos < us.offset_central_dir + us.size_central_dir) && (err == UNZ_OK))
        err = UNZ_BADZIPFILE;

    if (err != UNZ_OK) {
        if (err == UNZ_ERRNO) {
            CPE_ERROR(em, "unzip: %s: read data error, err=%d (%s)", (const char *)path, errno, strerror(errno));
        }
        else {
            CPE_ERROR(em, "unzip: %s: lib error, err=%d", (const char *)path, err);
        }
        vfs_file_close(us.filestream);
        return NULL;
    }

    us.byte_before_the_zipfile = central_pos - (us.offset_central_dir + us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;
    us.encrypted = 0;

    unz64_s * s = (unz64_s *)ALLOC(sizeof(unz64_s));
    if (s != NULL) {
        *s = us;
        cpe_unzGoToFirstFile((unzFile)s);
    }
    return (unzFile)s;
}

unzFile cpe_unzOpen(error_monitor_t em, vfs_mgr_t vfs, const char * path) {
    return cpe_unzOpenInternal(em, vfs, path, 0);
}

unzFile cpe_unzOpen64(error_monitor_t em, vfs_mgr_t vfs, const char * path) {
    return cpe_unzOpenInternal(em, vfs, path, 1);
}

/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */
int cpe_unzClose(unzFile file) {
    unz64_s * s;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;

    if (s->pfile_in_zip_read != NULL)
        cpe_unzCloseCurrentFile(file);

    vfs_file_close(s->filestream);
    s->filestream = NULL;

    TRYFREE(s);
    return UNZ_OK;
}

/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
int cpe_unzGetGlobalInfo64(unzFile file, unz_global_info64 * pglobal_info) {
    unz64_s * s;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    *pglobal_info = s->gi;
    return UNZ_OK;
}

int cpe_unzGetGlobalInfo(unzFile file, unz_global_info * pglobal_info32) {
    unz64_s * s;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->number_entry = (uLong)s->gi.number_entry;
    pglobal_info32->size_comment = s->gi.size_comment;
    return UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easilty)
*/
void cpe_unz64local_DosDateToTmuDate(ssize_t ulDosDate, cpe_zip_tm * ptm) {
    ssize_t uDate;
    uDate = (ssize_t)(ulDosDate >> 16);
    ptm->tm_mday = (uInt)(uDate & 0x1f);
    ptm->tm_mon = (uInt)((((uDate)&0x1E0) / 0x20) - 1);
    ptm->tm_year = (uInt)(((uDate & 0x0FE00) / 0x0200) + 1980);

    ptm->tm_hour = (uInt)((ulDosDate & 0xF800) / 0x800);
    ptm->tm_min = (uInt)((ulDosDate & 0x7E0) / 0x20);
    ptm->tm_sec = (uInt)(2 * (ulDosDate & 0x1f));
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
int cpe_unz64local_GetCurrentFileInfoInternal(unzFile file,
    unz_file_info64 * pfile_info,
    unz_file_info64_internal * pfile_info_internal,
    char * szFileName,
    uLong fileNameBufferSize,
    void * extraField,
    uLong extraFieldBufferSize,
    char * szComment,
    uLong commentBufferSize) {
    unz64_s * s;
    unz_file_info64 file_info;
    unz_file_info64_internal file_info_internal;
    int err = UNZ_OK;
    uLong uMagic;
    long lSeek = 0;
    uLong uL;

    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    if (vfs_file_seek(s->filestream, s->pos_in_central_dir + s->byte_before_the_zipfile, vfs_file_seek_set) != 0)
        err = UNZ_ERRNO;

    /* we check the magic */
    if (err == UNZ_OK) {
        if (cpe_unz64local_getLong(s->filestream, &uMagic) != UNZ_OK)
            err = UNZ_ERRNO;
        else if (uMagic != 0x02014b50)
            err = UNZ_BADZIPFILE;
    }

    if (cpe_unz64local_getShort(s->filestream, &file_info.version) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.version_needed) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.flag) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.compression_method) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getLong(s->filestream, &file_info.dosDate) != UNZ_OK)
        err = UNZ_ERRNO;

    cpe_unz64local_DosDateToTmuDate(file_info.dosDate, &file_info.tmu_date);

    if (cpe_unz64local_getLong(s->filestream, &file_info.crc) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getLong(s->filestream, &uL) != UNZ_OK)
        err = UNZ_ERRNO;
    file_info.compressed_size = uL;

    if (cpe_unz64local_getLong(s->filestream, &uL) != UNZ_OK)
        err = UNZ_ERRNO;
    file_info.uncompressed_size = uL;

    if (cpe_unz64local_getShort(s->filestream, &file_info.size_filename) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.size_file_extra) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.size_file_comment) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.disk_num_start) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &file_info.internal_fa) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getLong(s->filestream, &file_info.external_fa) != UNZ_OK)
        err = UNZ_ERRNO;

    // relative offset of local header
    if (cpe_unz64local_getLong(s->filestream, &uL) != UNZ_OK)
        err = UNZ_ERRNO;
    file_info_internal.offset_curfile = uL;

    lSeek += file_info.size_filename;
    if ((err == UNZ_OK) && (szFileName != NULL)) {
        uLong uSizeRead;
        if (file_info.size_filename < fileNameBufferSize) {
            *(szFileName + file_info.size_filename) = '\0';
            uSizeRead = file_info.size_filename;
        } else
            uSizeRead = fileNameBufferSize;

        if ((file_info.size_filename > 0) && (fileNameBufferSize > 0))
            if (vfs_file_read(s->filestream, szFileName, uSizeRead) != uSizeRead)
                err = UNZ_ERRNO;
        lSeek -= uSizeRead;
    }

    // Read extrafield
    if ((err == UNZ_OK) && (extraField != NULL)) {
        ssize_t uSizeRead;
        if (file_info.size_file_extra < extraFieldBufferSize)
            uSizeRead = file_info.size_file_extra;
        else
            uSizeRead = extraFieldBufferSize;

        if (lSeek != 0) {
            if (vfs_file_seek(s->filestream, lSeek, vfs_file_seek_cur) == 0)
                lSeek = 0;
            else
                err = UNZ_ERRNO;
        }

        if ((file_info.size_file_extra > 0) && (extraFieldBufferSize > 0))
            if (vfs_file_read(s->filestream, extraField, (uLong)uSizeRead) != uSizeRead)
                err = UNZ_ERRNO;

        lSeek += file_info.size_file_extra - (uLong)uSizeRead;
    } else
        lSeek += file_info.size_file_extra;

    if ((err == UNZ_OK) && (file_info.size_file_extra != 0)) {
        uLong acc = 0;

        // since lSeek now points to after the extra field we need to move back
        lSeek -= file_info.size_file_extra;

        if (lSeek != 0) {
            if (vfs_file_seek(s->filestream, lSeek, vfs_file_seek_cur) == 0)
                lSeek = 0;
            else
                err = UNZ_ERRNO;
        }

        while (acc < file_info.size_file_extra) {
            uLong headerId;
            uLong dataSize;

            if (cpe_unz64local_getShort(s->filestream, &headerId) != UNZ_OK)
                err = UNZ_ERRNO;

            if (cpe_unz64local_getShort(s->filestream, &dataSize) != UNZ_OK)
                err = UNZ_ERRNO;

            /* ZIP64 extra fields */
            if (headerId == 0x0001) {
                uLong uL;

                if (file_info.uncompressed_size == UINT32_MAX) {
                    if (cpe_unz64local_getLong64(s->filestream, &file_info.uncompressed_size) != UNZ_OK)
                        err = UNZ_ERRNO;
                }

                if (file_info.compressed_size == UINT32_MAX) {
                    if (cpe_unz64local_getLong64(s->filestream, &file_info.compressed_size) != UNZ_OK)
                        err = UNZ_ERRNO;
                }

                if (file_info_internal.offset_curfile == UINT32_MAX) {
                    /* Relative Header offset */
                    if (cpe_unz64local_getLong64(s->filestream, &file_info_internal.offset_curfile) != UNZ_OK)
                        err = UNZ_ERRNO;
                }

                if (file_info.disk_num_start == UINT32_MAX) {
                    /* Disk Start Number */
                    if (cpe_unz64local_getLong(s->filestream, &uL) != UNZ_OK)
                        err = UNZ_ERRNO;
                }

            } else {
                if (vfs_file_seek(s->filestream, dataSize, vfs_file_seek_cur) != 0)
                    err = UNZ_ERRNO;
            }

            acc += 2 + 2 + dataSize;
        }
    }

    if ((err == UNZ_OK) && (szComment != NULL)) {
        uLong uSizeRead;
        if (file_info.size_file_comment < commentBufferSize) {
            *(szComment + file_info.size_file_comment) = '\0';
            uSizeRead = file_info.size_file_comment;
        } else
            uSizeRead = commentBufferSize;

        if (lSeek != 0) {
            if (vfs_file_seek(s->filestream, lSeek, vfs_file_seek_cur) == 0)
                lSeek = 0;
            else
                err = UNZ_ERRNO;
        }

        if ((file_info.size_file_comment > 0) && (commentBufferSize > 0))
            if (vfs_file_read(s->filestream, szComment, uSizeRead) != uSizeRead)
                err = UNZ_ERRNO;
        lSeek += file_info.size_file_comment - uSizeRead;
    } else
        lSeek += file_info.size_file_comment;

    if ((err == UNZ_OK) && (pfile_info != NULL))
        *pfile_info = file_info;

    if ((err == UNZ_OK) && (pfile_info_internal != NULL))
        *pfile_info_internal = file_info_internal;

    return err;
}

/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
int cpe_unzGetCurrentFileInfo64(unzFile file,
    unz_file_info64 * pfile_info,
    char * szFileName, uLong fileNameBufferSize,
    void * extraField, uLong extraFieldBufferSize,
    char * szComment, uLong commentBufferSize) {
    return cpe_unz64local_GetCurrentFileInfoInternal(file, pfile_info, NULL,
        szFileName, fileNameBufferSize,
        extraField, extraFieldBufferSize,
        szComment, commentBufferSize);
}

int cpe_unzGetCurrentFileInfo(unzFile file,
    unz_file_info * pfile_info,
    char * szFileName, uLong fileNameBufferSize,
    void * extraField, uLong extraFieldBufferSize,
    char * szComment, uLong commentBufferSize) {
    int err;
    unz_file_info64 file_info64;
    err = cpe_unz64local_GetCurrentFileInfoInternal(file, &file_info64, NULL,
        szFileName, fileNameBufferSize,
        extraField, extraFieldBufferSize,
        szComment, commentBufferSize);
    if ((err == UNZ_OK) && (pfile_info != NULL)) {
        pfile_info->version = file_info64.version;
        pfile_info->version_needed = file_info64.version_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_method = file_info64.compression_method;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->crc = file_info64.crc;

        pfile_info->size_filename = file_info64.size_filename;
        pfile_info->size_file_extra = file_info64.size_file_extra;
        pfile_info->size_file_comment = file_info64.size_file_comment;

        pfile_info->disk_num_start = file_info64.disk_num_start;
        pfile_info->internal_fa = file_info64.internal_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;

        pfile_info->compressed_size = (uLong)file_info64.compressed_size;
        pfile_info->uncompressed_size = (uLong)file_info64.uncompressed_size;
    }
    return err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
int cpe_unzGoToFirstFile(unzFile file) {
    int err = UNZ_OK;
    unz64_s * s;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    s->pos_in_central_dir = s->offset_central_dir;
    s->num_file = 0;
    err = cpe_unz64local_GetCurrentFileInfoInternal(file, &s->cur_file_info,
        &s->cur_file_info_internal,
        NULL, 0, NULL, 0, NULL, 0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
int cpe_unzGoToNextFile(unzFile file) {
    unz64_s * s;
    int err;

    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;
    if (s->gi.number_entry != 0xffff) /* 2^16 files overflow hack */
        if (s->num_file + 1 == s->gi.number_entry)
            return UNZ_END_OF_LIST_OF_FILE;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename + s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment;
    s->num_file++;
    err = cpe_unz64local_GetCurrentFileInfoInternal(file, &s->cur_file_info,
        &s->cur_file_info_internal,
        NULL, 0, NULL, 0, NULL, 0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzipStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
int cpe_unzLocateFile(unzFile file, const char * szFileName, int iCaseSensitivity) {
    unz64_s * s;
    int err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_internal cur_file_info_internalSaved;
    ssize_t num_fileSaved;
    ssize_t pos_in_central_dirSaved;

    if (file == NULL)
        return UNZ_PARAMERROR;

    if (strlen(szFileName) >= UNZ_MAXFILENAMEINZIP)
        return UNZ_PARAMERROR;

    s = (unz64_s *)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;

    /* Save the current state */
    num_fileSaved = s->num_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_internalSaved = s->cur_file_info_internal;

    err = cpe_unzGoToFirstFile(file);

    while (err == UNZ_OK) {
        char szCurrentFileName[UNZ_MAXFILENAMEINZIP + 1];
        err = cpe_unzGetCurrentFileInfo64(file, NULL,
            szCurrentFileName, sizeof(szCurrentFileName) - 1,
            NULL, 0, NULL, 0);
        if (err == UNZ_OK) {
            if (cpe_zip_fname_cmp(szCurrentFileName, szFileName, iCaseSensitivity) == 0)
                return UNZ_OK;
            err = cpe_unzGoToNextFile(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->num_file = num_fileSaved;
    s->pos_in_central_dir = pos_in_central_dirSaved;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_internal = cur_file_info_internalSaved;
    return err;
}

/*
///////////////////////////////////////////
// Contributed by Ryan Haksi (mailto://cryogen@infoserve.net)
// I need random access
//
// Further optimization could be realized by adding an ability
// to cache the directory in memory. The goal being a single
// comprehensive file read to put the file I need in a memory.
*/

/*
typedef struct unz_file_pos_s
{
    ssize_t pos_in_zip_directory;   // offset in file
    ssize_t num_of_file;            // # of file
} unz_file_pos;
*/

int cpe_unzGetFilePos64(unzFile file, unz64_file_pos * file_pos) {
    unz64_s * s;

    if (file == NULL || file_pos == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    if (!s->current_file_ok)
        return UNZ_END_OF_LIST_OF_FILE;

    file_pos->pos_in_zip_directory = s->pos_in_central_dir;
    file_pos->num_of_file = s->num_file;

    return UNZ_OK;
}

int cpe_unzGetFilePos(
    unzFile file,
    unz_file_pos * file_pos) {
    unz64_file_pos file_pos64;
    int err = cpe_unzGetFilePos64(file, &file_pos64);
    if (err == UNZ_OK) {
        file_pos->pos_in_zip_directory = (uLong)file_pos64.pos_in_zip_directory;
        file_pos->num_of_file = (uLong)file_pos64.num_of_file;
    }
    return err;
}

int cpe_unzGoToFilePos64(unzFile file, const unz64_file_pos * file_pos) {
    unz64_s * s;
    int err;

    if (file == NULL || file_pos == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;

    /* jump to the right spot */
    s->pos_in_central_dir = file_pos->pos_in_zip_directory;
    s->num_file = file_pos->num_of_file;

    /* set the current file */
    err = cpe_unz64local_GetCurrentFileInfoInternal(file, &s->cur_file_info,
        &s->cur_file_info_internal,
        NULL, 0, NULL, 0, NULL, 0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

int cpe_unzGoToFilePos(
    unzFile file,
    unz_file_pos * file_pos) {
    unz64_file_pos file_pos64;
    if (file_pos == NULL)
        return UNZ_PARAMERROR;

    file_pos64.pos_in_zip_directory = file_pos->pos_in_zip_directory;
    file_pos64.num_of_file = file_pos->num_of_file;
    return cpe_unzGoToFilePos64(file, &file_pos64);
}

/*
// Unzip Helper Functions - should be here?
///////////////////////////////////////////
*/

/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
        directory about this file
  store in *piSizeVar the size of extra info in local header
        (filename and size of extra field data)
*/
int cpe_unz64local_CheckCurrentFileCoherencyHeader(unz64_s * s, uInt * piSizeVar,
    ssize_t * poffset_local_extrafield,
    uInt * psize_local_extrafield) {
    uLong uMagic, uData, uFlags;
    uLong size_filename;
    uLong size_extra_field;
    int err = UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    if (vfs_file_seek(s->filestream, s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile, vfs_file_seek_set) != 0)
        return UNZ_ERRNO;

    if (err == UNZ_OK) {
        if (cpe_unz64local_getLong(s->filestream, &uMagic) != UNZ_OK)
            err = UNZ_ERRNO;
        else if (uMagic != 0x04034b50)
            err = UNZ_BADZIPFILE;
    }

    if (cpe_unz64local_getShort(s->filestream, &uData) != UNZ_OK)
        err = UNZ_ERRNO;
    /*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    if (cpe_unz64local_getShort(s->filestream, &uFlags) != UNZ_OK)
        err = UNZ_ERRNO;

    if (cpe_unz64local_getShort(s->filestream, &uData) != UNZ_OK)
        err = UNZ_ERRNO;
    else if ((err == UNZ_OK) && (uData != s->cur_file_info.compression_method))
        err = UNZ_BADZIPFILE;

    if ((err == UNZ_OK) && (s->cur_file_info.compression_method != 0) &&
        /* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_method != Z_BZIP2ED) &&
        /* #endif */
        (s->cur_file_info.compression_method != Z_DEFLATED))
        err = UNZ_BADZIPFILE;

    if (cpe_unz64local_getLong(s->filestream, &uData) != UNZ_OK) /* date/time */
        err = UNZ_ERRNO;

    if (cpe_unz64local_getLong(s->filestream, &uData) != UNZ_OK) /* crc */
        err = UNZ_ERRNO;
    else if ((err == UNZ_OK) && (uData != s->cur_file_info.crc) && ((uFlags & 8) == 0))
        err = UNZ_BADZIPFILE;

    if (cpe_unz64local_getLong(s->filestream, &uData) != UNZ_OK) /* size compr */
        err = UNZ_ERRNO;
    else if (uData != 0xFFFFFFFF && (err == UNZ_OK) && (uData != s->cur_file_info.compressed_size) && ((uFlags & 8) == 0))
        err = UNZ_BADZIPFILE;

    if (cpe_unz64local_getLong(s->filestream, &uData) != UNZ_OK) /* size uncompr */
        err = UNZ_ERRNO;
    else if (uData != 0xFFFFFFFF && (err == UNZ_OK) && (uData != s->cur_file_info.uncompressed_size) && ((uFlags & 8) == 0))
        err = UNZ_BADZIPFILE;

    if (cpe_unz64local_getShort(s->filestream, &size_filename) != UNZ_OK)
        err = UNZ_ERRNO;
    else if ((err == UNZ_OK) && (size_filename != s->cur_file_info.size_filename))
        err = UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    if (cpe_unz64local_getShort(s->filestream, &size_extra_field) != UNZ_OK)
        err = UNZ_ERRNO;
    *poffset_local_extrafield = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
int cpe_unzOpenCurrentFile3(unzFile file, int * method,
    int * level, int raw, const char * password) {
    int err = UNZ_OK;
    uInt iSizeVar;
    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    ssize_t offset_local_extrafield; /* offset of the local extra field */
    uInt size_local_extrafield; /* size of the local extra field */
#ifndef NOUNCRYPT
    char source[12];
#else
    if (password != NULL)
        return UNZ_PARAMERROR;
#endif

    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    if (!s->current_file_ok)
        return UNZ_PARAMERROR;

    if (s->pfile_in_zip_read != NULL)
        cpe_unzCloseCurrentFile(file);

    if (cpe_unz64local_CheckCurrentFileCoherencyHeader(s, &iSizeVar, &offset_local_extrafield, &size_local_extrafield) != UNZ_OK)
        return UNZ_BADZIPFILE;

    pfile_in_zip_read_info = (file_in_zip64_read_info_s *)ALLOC(sizeof(file_in_zip64_read_info_s));
    if (pfile_in_zip_read_info == NULL)
        return UNZ_INTERNALERROR;

    pfile_in_zip_read_info->read_buffer = (char *)ALLOC(UNZ_BUFSIZE);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield = 0;
    pfile_in_zip_read_info->raw = raw;

    if (pfile_in_zip_read_info->read_buffer == NULL) {
        TRYFREE(pfile_in_zip_read_info);
        return UNZ_INTERNALERROR;
    }

    pfile_in_zip_read_info->stream_initialised = 0;

    if (method != NULL)
        *method = (int)s->cur_file_info.compression_method;

    if (level != NULL) {
        *level = 6;
        switch (s->cur_file_info.flag & 0x06) {
        case 6:
            *level = 1;
            break;
        case 4:
            *level = 2;
            break;
        case 2:
            *level = 9;
            break;
        }
    }

    if ((s->cur_file_info.compression_method != 0) &&
        /* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_method != Z_BZIP2ED) &&
        /* #endif */
        (s->cur_file_info.compression_method != Z_DEFLATED))

        err = UNZ_BADZIPFILE;

    pfile_in_zip_read_info->crc32_wait = s->cur_file_info.crc;
    pfile_in_zip_read_info->crc32 = 0;
    pfile_in_zip_read_info->total_out_64 = 0;
    pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
    pfile_in_zip_read_info->filestream = s->filestream;
    pfile_in_zip_read_info->byte_before_the_zipfile = s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

    if ((s->cur_file_info.compression_method == Z_BZIP2ED) && (!raw)) {
#ifdef HAVE_BZIP2
        pfile_in_zip_read_info->bstream.bzalloc = (void * (*)(void *, int, int))0;
        pfile_in_zip_read_info->bstream.bzfree = (free_func)0;
        pfile_in_zip_read_info->bstream.opaque = (voidpf)0;
        pfile_in_zip_read_info->bstream.state = (voidpf)0;

        pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
        pfile_in_zip_read_info->stream.zfree = (free_func)0;
        pfile_in_zip_read_info->stream.opaque = (voidpf)0;
        pfile_in_zip_read_info->stream.next_in = (voidpf)0;
        pfile_in_zip_read_info->stream.avail_in = 0;

        err = BZ2_bzDecompressInit(&pfile_in_zip_read_info->bstream, 0, 0);
        if (err == Z_OK)
            pfile_in_zip_read_info->stream_initialised = Z_BZIP2ED;
        else {
            TRYFREE(pfile_in_zip_read_info);
            return err;
        }
#else
        pfile_in_zip_read_info->raw = 1;
#endif
    } else if ((s->cur_file_info.compression_method == Z_DEFLATED) && (!raw)) {
        pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
        pfile_in_zip_read_info->stream.zfree = (free_func)0;
        pfile_in_zip_read_info->stream.opaque = (voidpf)0;
        pfile_in_zip_read_info->stream.next_in = 0;
        pfile_in_zip_read_info->stream.avail_in = 0;

        err = inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
        if (err == Z_OK)
            pfile_in_zip_read_info->stream_initialised = Z_DEFLATED;
        else {
            TRYFREE(pfile_in_zip_read_info);
            return err;
        }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size;
    pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size;

    pfile_in_zip_read_info->pos_in_zipfile = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar;

    pfile_in_zip_read_info->stream.avail_in = (uInt)0;

    s->pfile_in_zip_read = pfile_in_zip_read_info;
    s->encrypted = 0;

    if (password != NULL) {
        int i;
        s->pcrc_32_tab = get_crc_table();
        cpe_zip_crypto_init_keys(password, s->keys, s->pcrc_32_tab);
        if (vfs_file_seek(s->filestream,
                s->pfile_in_zip_read->pos_in_zipfile + s->pfile_in_zip_read->byte_before_the_zipfile,
                SEEK_SET)
            != 0)
            return UNZ_INTERNALERROR;
        if (vfs_file_read(s->filestream, source, 12) < 12)
            return UNZ_INTERNALERROR;

        for (i = 0; i < 12; i++) {
            zdecode(s->keys, s->pcrc_32_tab, source[i]);
        }
        
        s->pfile_in_zip_read->pos_in_zipfile += 12;
        s->encrypted = 1;
    }

    return UNZ_OK;
}

int cpe_unzOpenCurrentFile(unzFile file) {
    return cpe_unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

int cpe_unzOpenCurrentFilePassword(unzFile file, const char * password) {
    return cpe_unzOpenCurrentFile3(file, NULL, NULL, 0, password);
}

int cpe_unzOpenCurrentFile2(unzFile file, int * method, int * level, int raw) {
    return cpe_unzOpenCurrentFile3(file, method, level, raw, NULL);
}

/** Addition for GDAL : START */

ssize_t cpe_unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    s = (unz64_s *)file;
    if (file == NULL)
        return 0; //UNZ_PARAMERROR;
    pfile_in_zip_read_info = s->pfile_in_zip_read;
    if (pfile_in_zip_read_info == NULL)
        return 0; //UNZ_PARAMERROR;
    return pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile;
}

/** Addition for GDAL : END */

/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
int cpe_unzReadCurrentFile(unzFile file, voidp buf, unsigned len) {
    int err = UNZ_OK;
    uInt iRead = 0;
    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    pfile_in_zip_read_info = s->pfile_in_zip_read;

    if (pfile_in_zip_read_info == NULL)
        return UNZ_PARAMERROR;

    if (pfile_in_zip_read_info->read_buffer == NULL)
        return UNZ_END_OF_LIST_OF_FILE;
    if (len == 0)
        return 0;

    pfile_in_zip_read_info->stream.next_out = (Bytef *)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)len;

    if ((len > pfile_in_zip_read_info->rest_read_uncompressed) && (!(pfile_in_zip_read_info->raw)))
        pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    if ((len > pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in) && (pfile_in_zip_read_info->raw))
        pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in;

    while (pfile_in_zip_read_info->stream.avail_out > 0) {
        if ((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed > 0)) {
            uInt uReadThis = UNZ_BUFSIZE;
            if (pfile_in_zip_read_info->rest_read_compressed < uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            if (uReadThis == 0)
                return UNZ_EOF;
            if (vfs_file_seek(
                    pfile_in_zip_read_info->filestream,
                    pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile,
                    vfs_file_seek_set)
                != 0)
                return UNZ_ERRNO;
            if (vfs_file_read(
                    pfile_in_zip_read_info->filestream,
                    pfile_in_zip_read_info->read_buffer,
                    uReadThis)
                != uReadThis)
                return UNZ_ERRNO;

#ifndef NOUNCRYPT
            if (s->encrypted) {
                uInt i;
                for (i = 0; i < uReadThis; i++)
                    pfile_in_zip_read_info->read_buffer[i] = zdecode(s->keys, s->pcrc_32_tab,
                        pfile_in_zip_read_info->read_buffer[i]);
            }
#endif

            pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

            pfile_in_zip_read_info->rest_read_compressed -= uReadThis;

            pfile_in_zip_read_info->stream.next_in = (Bytef *)pfile_in_zip_read_info->read_buffer;
            pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
        }

        if ((pfile_in_zip_read_info->compression_method == 0) || (pfile_in_zip_read_info->raw)) {
            uInt uDoCopy, i;

            if ((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed == 0))
                return (iRead == 0) ? UNZ_EOF : iRead;

            if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out;
            else
                uDoCopy = pfile_in_zip_read_info->stream.avail_in;

            for (i = 0; i < uDoCopy; i++)
                *(pfile_in_zip_read_info->stream.next_out + i) = *(pfile_in_zip_read_info->stream.next_in + i);

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uDoCopy;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                pfile_in_zip_read_info->stream.next_out,
                uDoCopy);
            pfile_in_zip_read_info->rest_read_uncompressed -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
            pfile_in_zip_read_info->stream.next_out += uDoCopy;
            pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        } else if (pfile_in_zip_read_info->compression_method == Z_BZIP2ED) {
#ifdef HAVE_BZIP2
            uLong uTotalOutBefore, uTotalOutAfter;
            const Bytef * bufBefore;
            uLong uOutThis;

            pfile_in_zip_read_info->bstream.next_in = (char *)pfile_in_zip_read_info->stream.next_in;
            pfile_in_zip_read_info->bstream.avail_in = pfile_in_zip_read_info->stream.avail_in;
            pfile_in_zip_read_info->bstream.total_in_lo32 = pfile_in_zip_read_info->stream.total_in;
            pfile_in_zip_read_info->bstream.total_in_hi32 = 0;
            pfile_in_zip_read_info->bstream.next_out = (char *)pfile_in_zip_read_info->stream.next_out;
            pfile_in_zip_read_info->bstream.avail_out = pfile_in_zip_read_info->stream.avail_out;
            pfile_in_zip_read_info->bstream.total_out_lo32 = pfile_in_zip_read_info->stream.total_out;
            pfile_in_zip_read_info->bstream.total_out_hi32 = 0;

            uTotalOutBefore = pfile_in_zip_read_info->bstream.total_out_lo32;
            bufBefore = (const Bytef *)pfile_in_zip_read_info->bstream.next_out;

            err = BZ2_bzDecompress(&pfile_in_zip_read_info->bstream);

            uTotalOutAfter = pfile_in_zip_read_info->bstream.total_out_lo32;
            uOutThis = uTotalOutAfter - uTotalOutBefore;

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, bufBefore, (uInt)(uOutThis));
            pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;
            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            pfile_in_zip_read_info->stream.next_in = (Bytef *)pfile_in_zip_read_info->bstream.next_in;
            pfile_in_zip_read_info->stream.avail_in = pfile_in_zip_read_info->bstream.avail_in;
            pfile_in_zip_read_info->stream.total_in = pfile_in_zip_read_info->bstream.total_in_lo32;
            pfile_in_zip_read_info->stream.next_out = (Bytef *)pfile_in_zip_read_info->bstream.next_out;
            pfile_in_zip_read_info->stream.avail_out = pfile_in_zip_read_info->bstream.avail_out;
            pfile_in_zip_read_info->stream.total_out = pfile_in_zip_read_info->bstream.total_out_lo32;

            if (err == BZ_STREAM_END)
                return (iRead == 0) ? UNZ_EOF : iRead;
            if (err != BZ_OK)
                break;
#endif
        } // end Z_BZIP2ED
        else {
            ssize_t uTotalOutBefore, uTotalOutAfter;
            const Bytef * bufBefore;
            ssize_t uOutThis;
            int flush = Z_SYNC_FLUSH;

            uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
            bufBefore = pfile_in_zip_read_info->stream.next_out;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err = inflate(&pfile_in_zip_read_info->stream, flush);

            if ((err >= 0) && (pfile_in_zip_read_info->stream.msg != NULL))
                err = Z_DATA_ERROR;

            uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
            uOutThis = uTotalOutAfter - uTotalOutBefore;

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, bufBefore,
                (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;

            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            if (err == Z_STREAM_END)
                return (iRead == 0) ? UNZ_EOF : iRead;
            if (err != Z_OK)
                break;
        }
    }

    if (err == Z_OK)
        return iRead;
    return err;
}

/*
  Give the current position in uncompressed data
*/
z_off_t cpe_unztell(unzFile file) {
    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    pfile_in_zip_read_info = s->pfile_in_zip_read;

    if (pfile_in_zip_read_info == NULL)
        return UNZ_PARAMERROR;

    return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}

ssize_t cpe_unztell64(unzFile file) {

    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    if (file == NULL)
        return (ssize_t)-1;
    s = (unz64_s *)file;
    pfile_in_zip_read_info = s->pfile_in_zip_read;

    if (pfile_in_zip_read_info == NULL)
        return (ssize_t)-1;

    return pfile_in_zip_read_info->total_out_64;
}

/*
  return 1 if the end of file was reached, 0 elsewhere
*/
int cpe_unzeof(unzFile file) {
    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    pfile_in_zip_read_info = s->pfile_in_zip_read;

    if (pfile_in_zip_read_info == NULL)
        return UNZ_PARAMERROR;

    if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
        return 1;
    else
        return 0;
}

/*
Read extra field from the current file (opened by unzOpenCurrentFile)
This is the local-header version of the extra field (sometimes, there is
more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field that can be read

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
  the return value is the number of bytes copied in buf, or (if <0)
    the error code
*/
int cpe_unzGetLocalExtrafield(unzFile file, voidp buf, unsigned len) {
    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    uInt read_now;
    ssize_t size_to_read;

    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    pfile_in_zip_read_info = s->pfile_in_zip_read;

    if (pfile_in_zip_read_info == NULL)
        return UNZ_PARAMERROR;

    size_to_read = (pfile_in_zip_read_info->size_local_extrafield - pfile_in_zip_read_info->pos_local_extrafield);

    if (buf == NULL)
        return (int)size_to_read;

    if (len > size_to_read)
        read_now = (uInt)size_to_read;
    else
        read_now = (uInt)len;

    if (read_now == 0)
        return 0;

    if (vfs_file_seek(
            pfile_in_zip_read_info->filestream,
            pfile_in_zip_read_info->offset_local_extrafield + pfile_in_zip_read_info->pos_local_extrafield,
            vfs_file_seek_set)
        != 0)
        return UNZ_ERRNO;

    if (vfs_file_read(
            pfile_in_zip_read_info->filestream,
            buf, read_now)
        != read_now)
        return UNZ_ERRNO;

    return (int)read_now;
}

/*
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
int cpe_unzCloseCurrentFile(unzFile file) {
    int err = UNZ_OK;

    unz64_s * s;
    file_in_zip64_read_info_s * pfile_in_zip_read_info;
    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;
    pfile_in_zip_read_info = s->pfile_in_zip_read;

    if (pfile_in_zip_read_info == NULL)
        return UNZ_PARAMERROR;

    if ((pfile_in_zip_read_info->rest_read_uncompressed == 0) && (!pfile_in_zip_read_info->raw)) {
        if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
            err = UNZ_CRCERROR;
    }

    TRYFREE(pfile_in_zip_read_info->read_buffer);
    pfile_in_zip_read_info->read_buffer = NULL;
    if (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
        inflateEnd(&pfile_in_zip_read_info->stream);
#ifdef HAVE_BZIP2
    else if (pfile_in_zip_read_info->stream_initialised == Z_BZIP2ED)
        BZ2_bzDecompressEnd(&pfile_in_zip_read_info->bstream);
#endif

    pfile_in_zip_read_info->stream_initialised = 0;
    TRYFREE(pfile_in_zip_read_info);

    s->pfile_in_zip_read = NULL;

    return err;
}

/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
int cpe_unzGetGlobalComment(unzFile file, char * szComment, uLong uSizeBuf) {
    unz64_s * s;
    uLong uReadThis;
    if (file == NULL)
        return (int)UNZ_PARAMERROR;
    s = (unz64_s *)file;

    uReadThis = uSizeBuf;
    if (uReadThis > s->gi.size_comment)
        uReadThis = s->gi.size_comment;

    if (vfs_file_seek(s->filestream, s->central_pos + 22, vfs_file_seek_set) != 0)
        return UNZ_ERRNO;

    if (uReadThis > 0) {
        *szComment = '\0';
        if (vfs_file_read(s->filestream, szComment, uReadThis) != uReadThis)
            return UNZ_ERRNO;
    }

    if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment))
        *(szComment + s->gi.size_comment) = '\0';
    return (int)uReadThis;
}

/* Additions by RX '2004 */
ssize_t cpe_unzGetOffset64(unzFile file) {
    unz64_s * s;

    if (file == NULL)
        return 0; //UNZ_PARAMERROR;
    s = (unz64_s *)file;
    if (!s->current_file_ok)
        return 0;
    if (s->gi.number_entry != 0 && s->gi.number_entry != 0xffff)
        if (s->num_file == s->gi.number_entry)
            return 0;
    return s->pos_in_central_dir;
}

uLong cpe_unzGetOffset(unzFile file) {
    ssize_t offset64;

    if (file == NULL)
        return 0; //UNZ_PARAMERROR;
    offset64 = cpe_unzGetOffset64(file);
    return (uLong)offset64;
}

int cpe_unzSetOffset64(unzFile file, ssize_t pos) {
    unz64_s * s;
    int err;

    if (file == NULL)
        return UNZ_PARAMERROR;
    s = (unz64_s *)file;

    s->pos_in_central_dir = pos;
    s->num_file = s->gi.number_entry; /* hack */
    err = cpe_unz64local_GetCurrentFileInfoInternal(file, &s->cur_file_info,
        &s->cur_file_info_internal,
        NULL, 0, NULL, 0, NULL, 0);
    s->current_file_ok = (err == UNZ_OK);
    return err;
}

int cpe_unzSetOffset(unzFile file, uLong pos) {
    return cpe_unzSetOffset64(file, pos);
}
