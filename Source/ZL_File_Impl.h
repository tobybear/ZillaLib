/*
  ZillaLib
  Copyright (C) 2010-2016 Bernhard Schelling

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

#ifndef __ZL_FILE_IMPL__
#define __ZL_FILE_IMPL__

#include "ZL_String.h"
#include "ZL_Impl.h"
#include "ZL_File.h"
#include <stddef.h>

struct ZL_RWops
{
	virtual ptrdiff_t seektell(ptrdiff_t offset, int mode) = 0;
	virtual size_t read(void *data, size_t size, size_t maxnum) = 0;
	virtual size_t write(const void *data, size_t size, size_t num) = 0;
	virtual size_t size() = 0;
	virtual int eof() = 0;
	virtual int close() = 0; //also deletes this
protected:
	inline ZL_RWops() {}
	inline virtual ~ZL_RWops() {}
};

#define RW_SEEK_SET 0
#define RW_SEEK_CUR 1
#define RW_SEEK_END 2
#define ZL_RWseek(rw, offset, mode)    ((rw)->seektell(offset, mode) == -1)
#define ZL_RWseektell(rw, offset, mode) (rw)->seektell(offset, mode)
#define ZL_RWtell(rw)                   (rw)->seektell(0, RW_SEEK_CUR)
#define ZL_RWread(rw, ptr, size, n)     (rw)->read(ptr, size, n)
#define ZL_RWclose(rw)                  (rw)->close()
#define ZL_RWwrite(rw, ptr, size, n)    (rw)->write(ptr, size, n)
#define ZL_RWrewind(rw)                 (rw)->seektell(0, RW_SEEK_SET)
#define ZL_RWsize(rw)                   (rw)->size()
#define ZL_RWeof(rw)                    (rw)->eof()

struct ZL_FileContainer_Impl : ZL_Impl
{
	virtual ~ZL_FileContainer_Impl() { }
	virtual ZL_File OpenFile(const char *filename, const char *mode) = 0;
};

struct ZL_File_Impl : ZL_Impl
{
	ZL_File_Impl(const char *pfn, ZL_RWops *src) : filename(pfn), src(src), archive(NULL) { /*ZL_LOG1("FILE", "Open File %s", (filename.length() ? filename.c_str() : "Memory File"));*/ }
	ZL_File_Impl(const char *pfn, ZL_RWops *src, ZL_FileContainer_Impl* archive) : filename(pfn), src(src), archive(archive) { archive->AddRef(); /*ZL_LOG1("FILE", "Open Managed File %s", filename.c_str());*/ }
	~ZL_File_Impl() { if (src) ZL_RWclose(src); if (archive) archive->DelRef(); /*ZL_LOG1("FILE", (archive ? "Close Archive File %s" : "Close File %s"), (filename.length() ? filename.c_str() : "Memory File"));*/ }

	ZL_String filename;
	ZL_RWops* src;
	ZL_FileContainer_Impl* archive;
};

struct ZL_RWopsZIP : public ZL_RWops
{
	static ZL_RWops *Open(const ZL_FileLink& zipfile, const struct unz_s* dir_uf, unsigned long index);
	ptrdiff_t seektell(ptrdiff_t offset, int mode);
	size_t read(void *data, size_t size, size_t maxnum);
	size_t write(const void *data, size_t size, size_t num);
	size_t size();
	int eof();
	int close();
	bool is_stored_raw();
	unsigned long get_data_offset();

	//read the nth file from zip into new allocated buffer (needs to be free()'d later)
	static unsigned char* ReadSingle(ZL_RWops* rwops, unsigned int file_index = 0, int *out_size = NULL);

private:
	struct unz_s* unz_file;
	ZL_File zl_file;
};

#endif //__ZL_FILE_IMPL__
