/*
  The code below is based on unzip from Minizip (also ZLIB license) altered for usage in ZillaLib:
   - CRC, time/date and crypt features were removed
   - File reading was changed to use ZL_RWops directly
*/

#include "ZL_File_Impl.h"

/* unzip.h -- IO for uncompress .zip files using zlib
   Version 1.01h, December 28th, 2009

   Copyright (C) 1998-2009 Gilles Vollant

   This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
     WinZip, InfoZip tools and compatible.

   Multi volume ZipFile (span) are not supported.
   Encryption compatible with pkzip 2.04g only supported
   Old compressions used by old PKZip 1.x are not supported


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"

#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif

typedef unsigned int unzu32;

#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

// unz_global_info structure contain global data about the ZIP file - These data comes from the end of central dir
struct unz_global_info
{
	unzu32 number_entry;         /* total number of entries in the central dir on this disk */
	unzu32 size_comment;         /* size of the global comment of the zipfile */
};

/* unz_file_info contain information about a file in the zipfile */
struct unz_file_info
{
	unzu32 version;              /* version made by                 2 bytes */
	unzu32 version_needed;       /* version needed to extract       2 bytes */
	unzu32 flag;                 /* general purpose bit flag        2 bytes */
	unzu32 compression_method;   /* compression method              2 bytes */
	unzu32 dosDate;              /* last mod file date in Dos fmt   4 bytes */
	unzu32 crc;                  /* crc-32                          4 bytes */
	unzu32 compressed_size;      /* compressed size                 4 bytes */
	unzu32 uncompressed_size;    /* uncompressed size               4 bytes */
	unzu32 size_filename;        /* filename length                 2 bytes */
	unzu32 size_file_extra;      /* extra field length              2 bytes */
	unzu32 size_file_comment;    /* file comment length             2 bytes */

	unzu32 disk_num_start;       /* disk number start               2 bytes */
	unzu32 internal_fa;          /* internal file attributes        2 bytes */
	unzu32 external_fa;          /* external file attributes        4 bytes */
};

struct unz_file_pos
{
	unzu32 pos_in_zip_directory;   /* offset in zip file directory */
	unzu32 num_of_file;            /* # of file */
};

#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) free(p);}
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)

/* unz_file_info_interntal contain internal info about a file in zipfile*/
struct unz_file_info_internal
{
	unzu32 offset_curfile;/* relative offset of local header 4 bytes */
};

/* file_in_zip_read_info_s contain internal information about a file in zipfile, when reading and decompress it */
struct file_in_zip_read_info_s
{
	char  *read_buffer;         /* internal buffer for compressed data */
	z_stream stream;            /* zLib stream structure for inflate */

	unzu32 pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
	unzu32 stream_initialised;   /* flag set if stream structure is initialised*/

	unzu32 offset_local_extrafield;/* offset of the local extra field */
	uInt  size_local_extrafield;/* size of the local extra field */
	unzu32 rest_read_compressed; /* number of byte to be decompressed */
	unzu32 rest_read_uncompressed;/*number of byte to be obtained after decomp*/
	ZL_RWops* filestream;        /* io structore of the zipfile */
	unzu32 compression_method;   /* compression method (0==store) */
	unzu32 byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
	int   raw;
};

/* unz_s contain internal information about the zipfile */
struct unz_s
{
	ZL_RWops* filestream;        /* io structore of the zipfile */
	unz_global_info gi;       /* public global information */
	unzu32 byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
	unzu32 num_file;             /* number of the current file in the zipfile*/
	unzu32 pos_in_central_dir;   /* pos of the current file in the central dir*/
	unzu32 current_file_ok;      /* flag about the usability of the current file*/
	unzu32 central_pos;          /* position of the beginning of the central dir*/

	unzu32 size_central_dir;     /* size of the central directory  */
	unzu32 offset_central_dir;   /* offset of start of central directory with respect to the starting disk number */

	unz_file_info cur_file_info; /* public info about the current file in zip*/
	unz_file_info_internal cur_file_info_internal; /* private info about it*/
	file_in_zip_read_info_s* pfile_in_zip_read; /* structure about the current file if we are decompressing it */
};

#define unzlocal_getU32(filestream, valptr) (ZL_RWread(filestream, valptr, 1, 4) ? 0 : 1)

static int unzlocal_getU16(ZL_RWops* filestream, unzu32 *pX)
{
	unsigned short x;
	if (ZL_RWread(filestream, &x, 1, 2) != 2) { *pX = 0; return UNZ_ERRNO; }
	*pX = x;
	return UNZ_OK;
}

/*
  Locate the Central directory of a zipfile (at the end, just before the global comment)
  */
static unzu32 unzlocal_SearchCentralDir(ZL_RWops* filestream)
{
	unzu32 uSizeFile;
	unzu32 uBackRead;
	unzu32 uMaxBack = 0xffff; /* maximum size of global comment */

	if (ZL_RWseek(filestream, 0, RW_SEEK_END) != 0)
		return 0;

	uSizeFile = (unzu32)ZL_RWtell(filestream);

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	enum { BUFREADCOMMENT = 512 - 4 };
	unsigned char buf[BUFREADCOMMENT + 4];

	uBackRead = 4;
	while (uBackRead < uMaxBack)
	{
		unzu32 uReadSize, uReadPos;
		if (uBackRead + BUFREADCOMMENT > uMaxBack) uBackRead = uMaxBack;
		else uBackRead += BUFREADCOMMENT;
		uReadPos = uSizeFile - uBackRead;

		uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ? (BUFREADCOMMENT + 4) : (uSizeFile - uReadPos);
		if (ZL_RWseek(filestream, uReadPos, RW_SEEK_SET) != 0)
			break;

		if (ZL_RWread(filestream, buf, 1, uReadSize) != uReadSize)
			break;

		for (int i = (int)uReadSize - 4; (i--) > 0;)
			if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) && ((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3)) == 0x06))
				return uReadPos + i;
	}
	return 0;
}

/*
  Get Info about the current file in the zipfile, with internal only info
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
  */
static int unzlocal_GetCurrentFileInfoInternal(unz_s* file, unz_file_info *pfile_info, unz_file_info_internal *pfile_info_internal, char *szFileName, unzu32 fileNameBufferSize,
	void *extraField, unzu32 extraFieldBufferSize,
	char *szComment, unzu32 commentBufferSize)
{
	unz_s* s;
	unz_file_info file_info;
	unz_file_info_internal file_info_internal;
	int err = UNZ_OK;
	unzu32 uMagic;
	long lSeek = 0;

	if (file == NULL)
		return UNZ_PARAMERROR;
	s = (unz_s*)file;
	if (ZL_RWseek(s->filestream, s->pos_in_central_dir + s->byte_before_the_zipfile, RW_SEEK_SET) != 0)
		err = UNZ_ERRNO;


	/* we check the magic */
	if (err == UNZ_OK)
	{
		if (unzlocal_getU32(s->filestream, &uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if (uMagic != 0x02014b50)
			err = UNZ_BADZIPFILE;
	}

	if (unzlocal_getU16(s->filestream, &file_info.version) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.version_needed) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.flag) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.compression_method) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU32(s->filestream, &file_info.dosDate) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU32(s->filestream, &file_info.crc) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU32(s->filestream, &file_info.compressed_size) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU32(s->filestream, &file_info.uncompressed_size) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.size_filename) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.size_file_extra) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.size_file_comment) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.disk_num_start) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU16(s->filestream, &file_info.internal_fa) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU32(s->filestream, &file_info.external_fa) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getU32(s->filestream, &file_info_internal.offset_curfile) != UNZ_OK)
		err = UNZ_ERRNO;

	lSeek += file_info.size_filename;
	if ((err == UNZ_OK) && (szFileName != NULL))
	{
		unzu32 uSizeRead;
		if (file_info.size_filename < fileNameBufferSize)
		{
			*(szFileName + file_info.size_filename) = '\0';
			uSizeRead = file_info.size_filename;
		}
		else
			uSizeRead = fileNameBufferSize;

		if ((file_info.size_filename>0) && (fileNameBufferSize > 0))
			if (ZL_RWread(s->filestream, szFileName, 1, uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
		lSeek -= uSizeRead;
	}


	if ((err == UNZ_OK) && (extraField != NULL))
	{
		unzu32 uSizeRead;
		if (file_info.size_file_extra < extraFieldBufferSize)
			uSizeRead = file_info.size_file_extra;
		else
			uSizeRead = extraFieldBufferSize;

		if (lSeek != 0)
		{
			if (ZL_RWseek(s->filestream, lSeek, RW_SEEK_CUR) == 0)
				lSeek = 0;
			else
				err = UNZ_ERRNO;
		}

		if ((file_info.size_file_extra>0) && (extraFieldBufferSize > 0))
			if (ZL_RWread(s->filestream, extraField, 1, uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
		lSeek += file_info.size_file_extra - uSizeRead;
	}
	else
		lSeek += file_info.size_file_extra;


	if ((err == UNZ_OK) && (szComment != NULL))
	{
		unzu32 uSizeRead;
		if (file_info.size_file_comment < commentBufferSize)
		{
			*(szComment + file_info.size_file_comment) = '\0';
			uSizeRead = file_info.size_file_comment;
		}
		else
			uSizeRead = commentBufferSize;

		if (lSeek != 0)
		{
			if (ZL_RWseek(s->filestream, lSeek, RW_SEEK_CUR) == 0)
				lSeek = 0;
			else
				err = UNZ_ERRNO;
		}

		if ((file_info.size_file_comment>0) && (commentBufferSize > 0))
			if (ZL_RWread(s->filestream, szComment, 1, uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
		lSeek += file_info.size_file_comment - uSizeRead;
	}
	else
		lSeek += file_info.size_file_comment;

	if ((err == UNZ_OK) && (pfile_info != NULL))
		*pfile_info = file_info;

	if ((err == UNZ_OK) && (pfile_info_internal != NULL))
		*pfile_info_internal = file_info_internal;

	return err;
}

/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
  */
static int unzGoToFirstFile(unz_s* file)
{
	int err = UNZ_OK;
	unz_s* s;
	if (file == NULL)
		return UNZ_PARAMERROR;
	s = (unz_s*)file;
	s->pos_in_central_dir = s->offset_central_dir;
	s->num_file = 0;
	err = unzlocal_GetCurrentFileInfoInternal(file, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}

/*
  Open a Zip file with ZL_RWops.
  If the zipfile cannot be opened (file doesn't exist or in not valid), the
  return value is NULL.
  Else, the return value is a unzFile Handle, usable with other function
  of this unzip package.
  */
static unz_s* unzOpenZL(ZL_RWops* filestream)
{
	unz_s us;
	unz_s *s;
	unzu32 central_pos, uL;

	unzu32 number_disk;          /* number of the current dist, used for
								   spaning ZIP, unsupported, always 0*/
	unzu32 number_disk_with_CD;  /* number the the disk with central dir, used
								   for spaning ZIP, unsupported, always 0*/
	unzu32 number_entry_CD;      /* total number of entries in
								   the central dir
								   (same than number_entry on nospan) */

	int err = UNZ_OK;

	us.filestream = filestream;
	if (us.filestream == NULL)
		return NULL;

	central_pos = unzlocal_SearchCentralDir(us.filestream);
	if (central_pos == 0)
		err = UNZ_ERRNO;

	if (ZL_RWseek(us.filestream, central_pos, RW_SEEK_SET) != 0)
		err = UNZ_ERRNO;

	/* the signature, already checked */
	if (unzlocal_getU32(us.filestream, &uL) != UNZ_OK)
		err = UNZ_ERRNO;

	/* number of this disk */
	if (unzlocal_getU16(us.filestream, &number_disk) != UNZ_OK)
		err = UNZ_ERRNO;

	/* number of the disk with the start of the central directory */
	if (unzlocal_getU16(us.filestream, &number_disk_with_CD) != UNZ_OK)
		err = UNZ_ERRNO;

	/* total number of entries in the central dir on this disk */
	if (unzlocal_getU16(us.filestream, &us.gi.number_entry) != UNZ_OK)
		err = UNZ_ERRNO;

	/* total number of entries in the central dir */
	if (unzlocal_getU16(us.filestream, &number_entry_CD) != UNZ_OK)
		err = UNZ_ERRNO;

	if ((number_entry_CD != us.gi.number_entry) ||
		(number_disk_with_CD != 0) ||
		(number_disk != 0))
		err = UNZ_BADZIPFILE;

	/* size of the central directory */
	if (unzlocal_getU32(us.filestream, &us.size_central_dir) != UNZ_OK)
		err = UNZ_ERRNO;

	/* offset of start of central directory with respect to the
		  starting disk number */
	if (unzlocal_getU32(us.filestream, &us.offset_central_dir) != UNZ_OK)
		err = UNZ_ERRNO;

	/* zipfile comment length */
	if (unzlocal_getU16(us.filestream, &us.gi.size_comment) != UNZ_OK)
		err = UNZ_ERRNO;

	if ((central_pos < us.offset_central_dir + us.size_central_dir) &&
		(err == UNZ_OK))
		err = UNZ_BADZIPFILE;

	if (err != UNZ_OK)
	{
		return NULL;
	}

	us.byte_before_the_zipfile = central_pos -
		(us.offset_central_dir + us.size_central_dir);
	us.central_pos = central_pos;
	us.pfile_in_zip_read = NULL;

	s = (unz_s*)ALLOC(sizeof(unz_s));
	if (s != NULL)
	{
		*s = us;
		unzGoToFirstFile((unz_s*)s);
	}
	return (unz_s*)s;
}

/*
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
  */
static int unzCloseCurrentFile(unz_s* file)
{
	int err = UNZ_OK;

	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file == NULL)
		return UNZ_PARAMERROR;
	s = (unz_s*)file;
	pfile_in_zip_read_info = s->pfile_in_zip_read;

	if (pfile_in_zip_read_info == NULL)
		return UNZ_PARAMERROR;

	TRYFREE(pfile_in_zip_read_info->read_buffer);
	pfile_in_zip_read_info->read_buffer = NULL;
	if (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
		inflateEnd(&pfile_in_zip_read_info->stream);

	pfile_in_zip_read_info->stream_initialised = 0;
	TRYFREE(pfile_in_zip_read_info);

	s->pfile_in_zip_read = NULL;

	return err;
}

/*
  Close a ZipFile opened with unzipOpen.
  return UNZ_OK if there is no problem. */
static int unzClose(unz_s* file)
{
	if (file == NULL) return UNZ_PARAMERROR;
	if (file->pfile_in_zip_read != NULL) unzCloseCurrentFile(file);
	TRYFREE(file);
	return UNZ_OK;
}

/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
  directory about this file
  store in *piSizeVar the size of extra info in local header
  (filename and size of extra field data)
  */
static int unzlocal_CheckCurrentFileCoherencyHeader(unz_s* s, uInt* piSizeVar, unzu32 *poffset_local_extrafield, uInt  *psize_local_extrafield)
{
	unzu32 uMagic, uData, uFlags;
	unzu32 size_filename;
	unzu32 size_extra_field;
	int err = UNZ_OK;

	*piSizeVar = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield = 0;

	if (ZL_RWseek(s->filestream, s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile, RW_SEEK_SET) != 0)
		return UNZ_ERRNO;

	if (err == UNZ_OK)
	{
		if (unzlocal_getU32(s->filestream, &uMagic) != UNZ_OK) err = UNZ_ERRNO;
		else if (uMagic != 0x04034b50) err = UNZ_BADZIPFILE;
	}

	if (unzlocal_getU16(s->filestream, &uData) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getU16(s->filestream, &uFlags) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getU16(s->filestream, &uData) != UNZ_OK) err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.compression_method)) err = UNZ_BADZIPFILE;

	if ((err == UNZ_OK) && (s->cur_file_info.compression_method != 0) && (s->cur_file_info.compression_method != Z_DEFLATED)) err = UNZ_BADZIPFILE;
	if (unzlocal_getU32(s->filestream, &uData) != UNZ_OK) err = UNZ_ERRNO; /* date/time */

	if (unzlocal_getU32(s->filestream, &uData) != UNZ_OK) err = UNZ_ERRNO; /* crc */
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.crc) && ((uFlags & 8) == 0)) err = UNZ_BADZIPFILE;

	if (unzlocal_getU32(s->filestream, &uData) != UNZ_OK) err = UNZ_ERRNO; /* size compr */
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.compressed_size) && ((uFlags & 8) == 0)) err = UNZ_BADZIPFILE;

	if (unzlocal_getU32(s->filestream, &uData) != UNZ_OK) err = UNZ_ERRNO; /* size uncompr */
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.uncompressed_size) && ((uFlags & 8) == 0)) err = UNZ_BADZIPFILE;

	if (unzlocal_getU16(s->filestream, &size_filename) != UNZ_OK) err = UNZ_ERRNO;
	else if ((err == UNZ_OK) && (size_filename != s->cur_file_info.size_filename)) err = UNZ_BADZIPFILE;

	*piSizeVar += (uInt)size_filename;

	if (unzlocal_getU16(s->filestream, &size_extra_field) != UNZ_OK) err = UNZ_ERRNO;
	*poffset_local_extrafield = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + size_filename;
	*psize_local_extrafield = (uInt)size_extra_field;

	*piSizeVar += (uInt)size_extra_field;
	return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
  */
static int unzOpenCurrentFile3(unz_s* file, int* method, int* level, int raw, const char* password)
{
	int err = UNZ_OK;
	uInt iSizeVar;
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	unzu32 offset_local_extrafield;  /* offset of the local extra field */
	uInt  size_local_extrafield;    /* size of the local extra field */
	if (password != NULL)
		return UNZ_PARAMERROR;

	if (file == NULL)
		return UNZ_PARAMERROR;
	s = (unz_s*)file;
	if (!s->current_file_ok)
		return UNZ_PARAMERROR;

	if (s->pfile_in_zip_read != NULL)
		unzCloseCurrentFile(file);

	if (unzlocal_CheckCurrentFileCoherencyHeader(s, &iSizeVar,
		&offset_local_extrafield, &size_local_extrafield) != UNZ_OK)
		return UNZ_BADZIPFILE;

	pfile_in_zip_read_info = (file_in_zip_read_info_s*)
		ALLOC(sizeof(file_in_zip_read_info_s));
	if (pfile_in_zip_read_info == NULL)
		return UNZ_INTERNALERROR;

	pfile_in_zip_read_info->read_buffer = (char*)ALLOC(UNZ_BUFSIZE);
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
	pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
	pfile_in_zip_read_info->raw = raw;

	if (pfile_in_zip_read_info->read_buffer == NULL)
	{
		TRYFREE(pfile_in_zip_read_info);
		return UNZ_INTERNALERROR;
	}

	pfile_in_zip_read_info->stream_initialised = 0;

	if (method != NULL)
		*method = (int)s->cur_file_info.compression_method;

	if (level != NULL)
	{
		*level = 6;
		switch (s->cur_file_info.flag & 0x06)
		{
			case 6: *level = 1; break;
			case 4: *level = 2; break;
			case 2: *level = 9; break;
		}
	}

	if ((s->cur_file_info.compression_method != 0) && (s->cur_file_info.compression_method != Z_DEFLATED))
		err = UNZ_BADZIPFILE;

	pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
	pfile_in_zip_read_info->filestream = s->filestream;
	pfile_in_zip_read_info->byte_before_the_zipfile = s->byte_before_the_zipfile;

	pfile_in_zip_read_info->stream.total_out = 0;

	if ((s->cur_file_info.compression_method == Z_DEFLATED) &&
		(!raw))
	{
		pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
		pfile_in_zip_read_info->stream.zfree = (free_func)0;
		pfile_in_zip_read_info->stream.opaque = NULL;
		pfile_in_zip_read_info->stream.next_in = NULL;
		pfile_in_zip_read_info->stream.avail_in = 0;

		err = inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
		if (err == Z_OK)
			pfile_in_zip_read_info->stream_initialised = Z_DEFLATED;
		else
		{
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

	return UNZ_OK;
}

static int unzOpenCurrentFile(unz_s* file)
{
	return unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
  (UNZ_ERRNO for IO error, or zLib error for uncompress error)
  */
static int unzReadCurrentFile(unz_s* file, voidp buf, unsigned len)
{
	int err = UNZ_OK;
	uInt iRead = 0;
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file == NULL)
		return UNZ_PARAMERROR;
	s = (unz_s*)file;
	pfile_in_zip_read_info = s->pfile_in_zip_read;

	if (pfile_in_zip_read_info == NULL) return UNZ_PARAMERROR;
	if (pfile_in_zip_read_info->read_buffer == NULL) return UNZ_END_OF_LIST_OF_FILE;
	if (len == 0) return 0;

	pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

	pfile_in_zip_read_info->stream.avail_out = (uInt)len;

	if ((len > pfile_in_zip_read_info->rest_read_uncompressed) && (!(pfile_in_zip_read_info->raw)))
		pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

	if ((len > pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in) && (pfile_in_zip_read_info->raw))
		pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in;

	while (pfile_in_zip_read_info->stream.avail_out > 0)
	{
		if ((pfile_in_zip_read_info->stream.avail_in == 0) &&
			(pfile_in_zip_read_info->rest_read_compressed > 0))
		{
			uInt uReadThis = UNZ_BUFSIZE;
			if (pfile_in_zip_read_info->rest_read_compressed < uReadThis)
				uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
			if (uReadThis == 0)
				return UNZ_EOF;
			if (ZL_RWseek(pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile, RW_SEEK_SET) != 0)
				return UNZ_ERRNO;
			if (ZL_RWread(pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->read_buffer, 1, uReadThis) != uReadThis)
				return UNZ_ERRNO;

			pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

			pfile_in_zip_read_info->rest_read_compressed -= uReadThis;

			pfile_in_zip_read_info->stream.next_in =
				(Bytef*)pfile_in_zip_read_info->read_buffer;
			pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
		}

		if ((pfile_in_zip_read_info->compression_method == 0) || (pfile_in_zip_read_info->raw))
		{
			uInt uDoCopy;

			if ((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed == 0))
				return (iRead == 0) ? UNZ_EOF : iRead;

			if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in)
				uDoCopy = pfile_in_zip_read_info->stream.avail_out;
			else
				uDoCopy = pfile_in_zip_read_info->stream.avail_in;

			memcpy(pfile_in_zip_read_info->stream.next_out, pfile_in_zip_read_info->stream.next_in, uDoCopy);

			pfile_in_zip_read_info->rest_read_uncompressed -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
			pfile_in_zip_read_info->stream.next_out += uDoCopy;
			pfile_in_zip_read_info->stream.next_in += uDoCopy;
			pfile_in_zip_read_info->stream.total_out += uDoCopy;
			iRead += uDoCopy;
		}
		else
		{
			size_t uTotalOutBefore, uTotalOutAfter;
			int flush = Z_SYNC_FLUSH;

			uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;

			err = inflate(&pfile_in_zip_read_info->stream, flush);

			if ((err >= 0) && (pfile_in_zip_read_info->stream.msg != NULL))
				err = Z_DATA_ERROR;

			uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;

			pfile_in_zip_read_info->rest_read_uncompressed -= (unzu32)(uTotalOutAfter - uTotalOutBefore);

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

static int unzSetOffset(unz_s* s, unzu32 pos)
{
	if (s == NULL) return UNZ_PARAMERROR;
	s->pos_in_central_dir = pos;
	s->num_file = s->gi.number_entry;      /* hack */
	int err = unzlocal_GetCurrentFileInfoInternal(s, &s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <map>
#include <ZL_Application.h>

ZL_RWops *ZL_RWopsZIP::Open(const ZL_FileLink& zipfilelink, const unz_s* dir_uf, unsigned long index)
{
	unz_s* s = (unz_s*)malloc(sizeof(unz_s));
	if (!s) return NULL;

	ZL_File zipfile = zipfilelink.Open();
	*s = *dir_uf; //copy pre-loaded zip structure
	s->filestream = ZL_ImplFromOwner<ZL_File_Impl>(zipfile)->src;
	s->pfile_in_zip_read = NULL;

	if ((unzSetOffset(s, (unzu32)index) != UNZ_OK) || (unzOpenCurrentFile(s) != UNZ_OK))
	{
		unzClose(s);
		return NULL;
	}

	ZL_RWopsZIP* rwops = new ZL_RWopsZIP();
	rwops->zl_file = zipfile;
	rwops->unz_file = s;
	return rwops;
}

ptrdiff_t ZL_RWopsZIP::seektell(ptrdiff_t offset, int mode)
{
	file_in_zip_read_info_s* f = unz_file->pfile_in_zip_read;
	if (mode == RW_SEEK_END) offset += unz_file->cur_file_info.uncompressed_size;
	else if (mode == RW_SEEK_CUR) offset += f->stream.total_out;
	size_t pos = (size_t)offset;
	if (pos == f->stream.total_out) return pos;
	if (pos > unz_file->cur_file_info.uncompressed_size) return -1;

	if (pos == unz_file->cur_file_info.uncompressed_size)
	{
		f->stream.total_in += f->rest_read_compressed;
		f->stream.total_out += f->rest_read_uncompressed;
		f->stream.avail_in = 0;
		f->stream.avail_out = 0;
		f->pos_in_zipfile += f->rest_read_compressed;
		f->rest_read_compressed = f->rest_read_uncompressed = 0;
		return pos;
	}

	if (pos < f->stream.total_out)
	{
		f->pos_in_zipfile -= (unz_file->cur_file_info.compressed_size - f->rest_read_compressed);
		f->rest_read_compressed = unz_file->cur_file_info.compressed_size;
		f->rest_read_uncompressed = unz_file->cur_file_info.uncompressed_size;
		if (!f->raw && f->compression_method) inflateReset(&f->stream);
		f->stream.total_in = f->stream.total_out = f->stream.avail_in = f->stream.avail_out = 0;
	}

	if (pos > f->stream.total_out)
	{
		char tmpbuf[1024];
		for (size_t i = f->stream.total_out; i < pos; i += 1024)
			unzReadCurrentFile(unz_file, tmpbuf, (unsigned)(i + 1024 > pos ? pos - i : 1024));
	}

	return pos;
}

size_t ZL_RWopsZIP::read(void *data, size_t size, size_t maxnum)
{
	return unzReadCurrentFile(unz_file, data, (unsigned)(size*maxnum)) / size;
}

size_t ZL_RWopsZIP::write(const void*, size_t, size_t) { return 0; }

size_t ZL_RWopsZIP::size()
{
	return unz_file->cur_file_info.uncompressed_size;
}

int ZL_RWopsZIP::eof()
{
	const unz_s *s = (unz_s*)unz_file;
	file_in_zip_read_info_s* f = s->pfile_in_zip_read;
	return (f->stream.total_out == s->cur_file_info.uncompressed_size);
}

int ZL_RWopsZIP::close()
{
	unzCloseCurrentFile(unz_file);
	unzClose(unz_file);
	delete this;
	return 0;
}

bool ZL_RWopsZIP::is_stored_raw()
{
	return (unz_file->cur_file_info.compression_method == 0 &&
		unz_file->cur_file_info.uncompressed_size == unz_file->cur_file_info.compressed_size);
}

unsigned long ZL_RWopsZIP::get_data_offset()
{
	file_in_zip_read_info_s* f = unz_file->pfile_in_zip_read;
	return f->pos_in_zipfile + f->rest_read_compressed - unz_file->cur_file_info.compressed_size;
}

unsigned char* ZL_RWopsZIP::ReadSingle(ZL_RWops* rwops, unsigned int file_index, int *out_size)
{
	unz_s* unz_file = unzOpenZL(rwops);
	if (!unz_file) return NULL;
	while (file_index--)
	{
		if (unzlocal_GetCurrentFileInfoInternal(unz_file, &unz_file->cur_file_info, NULL, NULL,0, NULL,0,NULL,0) != UNZ_OK) break;
		unzSetOffset(unz_file, unz_file->pos_in_central_dir + SIZECENTRALDIRITEM + unz_file->cur_file_info.size_filename + unz_file->cur_file_info.size_file_extra + unz_file->cur_file_info.size_file_comment);
	}
	unsigned char* buf = NULL;
	if (unzOpenCurrentFile(unz_file) == UNZ_OK)
	{
		unzu32 size = unz_file->cur_file_info.uncompressed_size;
		buf = (unsigned char*)malloc(size);
		unzReadCurrentFile(unz_file, buf, (unsigned)size);
		unzCloseCurrentFile(unz_file);
		if (out_size) *out_size = (int)size;
	}
	unzClose(unz_file);
	return buf;
}

struct ZL_FileContainer_ZIP_Impl : ZL_FileContainer_Impl
{
	ZL_String strLoadPrefix;
	unz_s* uf;
	const ZL_FileLink zipfilelink;
	std::map<ZL_String, unsigned long> structure;

	ZL_FileContainer_ZIP_Impl(const ZL_File &zipfile) : uf(NULL), zipfilelink(zipfile)
	{
		if (!zipfile) return;
		ZL_LOG1("ZIP", "Reading ZIP structure (filesize: %d)", zipfile.Size());

		//this constructor actually closes the file handle after reading the zip index
		ZL_RWops* rwops = ZL_ImplFromOwner<ZL_File_Impl>(zipfile)->src;
		uf = unzOpenZL(rwops);
		if (!uf) return;

		char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
		while (true)
		{
			if (unzlocal_GetCurrentFileInfoInternal(uf, &uf->cur_file_info, NULL, szCurrentFileName,UNZ_MAXFILENAMEINZIP, NULL,0,NULL,0) != UNZ_OK) break;
			structure[szCurrentFileName] = uf->pos_in_central_dir;
			//ZL_LOG1("ZIP", "Got File Entry: %s", szCurrentFileName);
			uf->pos_in_central_dir += SIZECENTRALDIRITEM + uf->cur_file_info.size_filename + uf->cur_file_info.size_file_extra + uf->cur_file_info.size_file_comment;
			uf->num_file++;
		}

		//for (std::map<ZL_String, unsigned long>::iterator it = structure.begin(); it != structure.end(); ++it)
		//	ZL_LOG2("ZIP", "Structure entry [%s]: %d", it->first.c_str(), it->second);
	}

	ZL_File OpenFile(const char *filename, const char * /*mode*/)
	{
		//ZL_LOG1("ZIP", "Looking up filename %s in zip", filename.c_str());
		const ZL_String &fullfile = (strLoadPrefix.length() ? strLoadPrefix + filename : ZL_String(filename));
		std::map<ZL_String, unsigned long>::iterator it = structure.find(fullfile);
		if (it != structure.end())
		{
			ZL_RWops *rwop = ZL_RWopsZIP::Open(zipfilelink, uf, it->second);
			if (rwop) return ZL_ImplMakeOwner<ZL_File>(new ZL_File_Impl(filename, rwop, this), false);
		}
		return ZL_File();
	}

	~ZL_FileContainer_ZIP_Impl()
	{
		//just free the resources of the unzFile, the actual file handle is unrelated
		unzClose(uf);
	}
};

ZL_FileContainer_ZIP::ZL_FileContainer_ZIP(const ZL_File& zipfile)
{
	ZL_FileContainer_ZIP_Impl* i; impl = i = new ZL_FileContainer_ZIP_Impl(zipfile);
	if (i->uf == NULL) { delete impl; impl = NULL; }
}

ZL_FileContainer_ZIP::ZL_FileContainer_ZIP(const ZL_File& zipfile, const char *defaultFolder)
{
	ZL_FileContainer_ZIP_Impl* i; impl = i = new ZL_FileContainer_ZIP_Impl(zipfile);
	if (i->uf == NULL) { delete impl; impl = NULL; return; }
	i->strLoadPrefix = defaultFolder;
}
