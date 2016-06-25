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

#include "ZL_Platform.h"
#include "ZL_File_Impl.h"
#include "ZL_Application.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <string.h>

struct ZL_RWopsFile : public ZL_RWops
{
	FILE *fp;
	static ZL_RWops* Open(const char *file, const char *mode)
	{
		if (!file[0]) return NULL;
		FILE *f = fopen(file, mode);
		if (!f) return NULL;
		ZL_RWopsFile *rw = new ZL_RWopsFile();
		rw->fp = f;
		return rw;
	}
	ptrdiff_t seektell(ptrdiff_t offset, int mode) { return (fseek(fp, (long)offset, mode) ? -1 : ftell(fp)); }
	size_t read(void *data, size_t size, size_t maxnum) { return fread(data, size, maxnum, fp); }
	size_t write(const void *data, size_t size, size_t num) { return fwrite(data, size, num, fp); }
	size_t size() { long p = ftell(fp), s; fseek(fp, 0, RW_SEEK_END); s = ftell(fp); if (p != s) fseek(fp, p, RW_SEEK_SET); return s; }
	int eof() { return feof(fp); }
	int close() { int ret = (fclose(fp) ? -1 : 0); delete this; return ret; }
};

struct ZL_RWopsMem : public ZL_RWops
{
	unsigned char *base, *here, *stop;
	static ZL_RWops* Open(void* mem, size_t size)
	{
		ZL_RWopsMem *rw = new ZL_RWopsMem();
		rw->base = (unsigned char*)mem;
		rw->here = rw->base;
		rw->stop = rw->base + size;
		return rw;
	}
	ptrdiff_t seektell(ptrdiff_t offset, int mode)
	{
		switch (mode)
		{
			case RW_SEEK_SET: here = base + offset; break;
			case RW_SEEK_CUR: here = here + offset; break;
			case RW_SEEK_END: here = stop + offset; break;
			default: return -1;
		}
		if (here > stop) here = stop;
		if (here < base) here = base;
		return (long)(here - base);
	}
	size_t read(void *data, size_t size, size_t maxnum)
	{
		size_t total_bytes = (maxnum * size);
		if (!maxnum || !size || ((total_bytes / maxnum) != size)) return 0;
		if (total_bytes > (size_t)(stop - here)) total_bytes = (stop - here);
		memcpy(data, here, total_bytes);
		here += total_bytes;
		return (total_bytes / size);
	}
	size_t write(const void *data, size_t size, size_t num)
	{
		if ((here + (num * size)) > stop) num = (stop - here) / size;
		memcpy(here, data, num * size);
		here += num * size;
		return num;
	}
	size_t size() { return stop - base; }
	int eof() { return (here == stop); }
	int close() { delete this; return 0; }
};

struct ZL_RWopsConstMem : public ZL_RWopsMem
{
	static ZL_RWops* Open(const void* mem, size_t size)
	{
		ZL_RWopsMem *rw = new ZL_RWopsMem();
		rw->base = (unsigned char*)mem;
		rw->here = rw->base;
		rw->stop = rw->base + size;
		return rw;
	}
	size_t write(const void* /*data*/, size_t /*size*/, size_t /*num*/) { return 0; }
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_FileContainer)

ZL_File ZL_FileContainer::Open(const char *file, const char *mode) const
{
	if (!impl) return ZL_File();
	return impl->OpenFile(file, mode);
}

ZL_FileContainer ZL_File::DefaultReadFileContainer;

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_File)

ZL_File::ZL_File(const char *file, const char *mode, const ZL_FileContainer& archive) : impl(NULL)
{
	//ZL_LOG1("FILE", "Open File %s", file);
	if (archive.impl && (archive.impl != DefaultReadFileContainer.impl || strchr(mode, 'r') != NULL))
		*this = archive.Open(file, mode);
	if (!impl || !impl->src)
	{
		if (impl) delete impl;
		ZL_RWops *rwop = ZL_RWopsFile::Open(file, mode);
		if (rwop) impl = new ZL_File_Impl(file, rwop);
		if (!impl || !impl->src) { ZL_LOG1("FILE", "Could not open file %s", file); }
	}
}

ZL_File::ZL_File(const ZL_String& file, const char *mode, const ZL_FileContainer& archive) : impl(NULL)
{
	//ZL_LOG1("FILE", "Open File %s", file.c_str());
	if (archive.impl && (archive.impl != DefaultReadFileContainer.impl || strchr(mode, 'r') != NULL))
		*this = archive.Open(file.c_str(), mode);
	if (!impl || !impl->src)
	{
		if (impl) delete impl;
		ZL_RWops *rwop = ZL_RWopsFile::Open(file.c_str(), mode);
		if (rwop) impl = new ZL_File_Impl(file.c_str(), rwop);
		if (!impl || !impl->src) { ZL_LOG1("FILE", "Could not open file %s", file.c_str()); }
	}
}
ZL_File::ZL_File(void *mem, size_t size) : impl(new ZL_File_Impl("", ZL_RWopsMem::Open(mem, size))) { }

ZL_File::ZL_File(const void *mem, size_t size) : impl(new ZL_File_Impl("", ZL_RWopsConstMem::Open(mem, size))) { }

ZL_File::operator bool () const { return (impl && impl->src); }

size_t ZL_File::SetContents(const ZL_String& data)
{
	if (!impl || !impl->src) return -2;
	ZL_RWrewind(impl->src);
	return ZL_RWwrite(impl->src, data.c_str(), sizeof(ZL_String::chr), data.size());
}

size_t ZL_File::SetContents(const unsigned char* data, size_t size)
{
	if (!impl || !impl->src) return -2;
	ZL_RWrewind(impl->src);
	return ZL_RWwrite(impl->src, data, 1, size);
}

ZL_String ZL_File::GetContents() const
{
	if (!impl || !impl->src) return "";
	size_t size = ZL_RWsize(impl->src);
	ZL_RWrewind(impl->src);
	ZL_String res(size, '\0');
	ZL_RWread(impl->src, &(char&)res.operator[](0), size, 1);
	return res;
}

size_t ZL_File::GetContents(ZL_String& out) const
{
	if (!impl || !impl->src) return 0;
	size_t size = ZL_RWsize(impl->src);
	ZL_RWrewind(impl->src);
	out.resize(size);
	ZL_RWread(impl->src, &(char&)out.operator[](0), size, 1);
	return size;
}

size_t ZL_File::GetContents(std::vector<unsigned char>& out) const
{
	if (!impl || !impl->src) return 0;
	size_t size = ZL_RWsize(impl->src);
	ZL_RWrewind(impl->src);
	out.resize(out.size() + size);
	ZL_RWread(impl->src, &out[out.size() - size], size, 1);
	return size;
}

size_t ZL_File::GetContents(unsigned char* buf, size_t bufsize) const
{
	if (!impl || !impl->src) return 0;
	size_t readsize = MIN(ZL_RWsize(impl->src), bufsize);
	ZL_RWrewind(impl->src);
	ZL_RWread(impl->src, buf, readsize, 1);
	return readsize;
}

char* ZL_File::GetContentsMalloc(size_t* size) const
{
	if (!impl || !impl->src) { size = 0; return NULL; }
	*size = ZL_RWsize(impl->src);
	ZL_RWrewind(impl->src);
	char* ret = (char*)malloc(*size);
	ZL_RWread(impl->src, ret, *size, 1);
	return ret;
}

size_t ZL_File::Size() const
{
	return (impl ? ZL_RWsize(impl->src) : 0);
}

const ZL_String& ZL_File::GetFileName() const
{
	return (impl ? impl->filename : ZL_String::EmptyString);
}

bool ZL_File::Exists(const char *file, const ZL_FileContainer& archive)
{
	if (archive.impl && archive.impl != DefaultReadFileContainer.impl && archive.Open(file)) return true;
	FILE* f = fopen(file, "rb");
	if (!f) return false;
	fclose(f);
	return true;
}

struct ZL_FileLink_Impl : ZL_Impl
{
	ZL_String filename;
	ZL_FileContainer_Impl *filecontainer;
	ZL_File_Impl *filecopy; //only used for files with "no filename" (i.e. files opened from memory)

	ZL_FileLink_Impl(const ZL_File &linksource) : filecopy(NULL)
	{
		ZL_File_Impl* fileimpl = ZL_ImplFromOwner<ZL_File_Impl>(linksource);
		if (!fileimpl) { filecontainer = NULL; return; }
		filename = fileimpl->filename;
		filecontainer = fileimpl->archive;
		if (filecontainer) filecontainer->AddRef();
		if (!filename.length()) { filecopy = fileimpl; filecopy->AddRef(); }
	}

	ZL_FileLink_Impl(const char *file, const ZL_FileContainer& archive) : filename(file), filecontainer(NULL), filecopy(NULL)
	{
		filecontainer = ZL_ImplFromOwner<ZL_FileContainer_Impl>(archive);
		if (filecontainer) filecontainer->AddRef();
	}

	ZL_FileLink_Impl(const char *file) : filename(file), filecontainer(ZL_ImplFromOwner<ZL_FileContainer_Impl>(ZL_File::DefaultReadFileContainer)), filecopy(NULL)
	{
		if (filecontainer) filecontainer->AddRef();
	}

	~ZL_FileLink_Impl()
	{
		if (filecopy) filecopy->DelRef();
		if (filecontainer) filecontainer->DelRef();
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_FileLink)

ZL_FileLink::ZL_FileLink(const ZL_File &linksource) : impl(new ZL_FileLink_Impl(linksource)) { }

ZL_FileLink::ZL_FileLink(const ZL_String &file, const ZL_FileContainer& archive) : impl(new ZL_FileLink_Impl(file, archive)) { }

ZL_FileLink::ZL_FileLink(const char *file) : impl(new ZL_FileLink_Impl(file)) { }

ZL_File ZL_FileLink::Open() const
{
	ZL_File file;
	if (!impl) return file;
	if (impl->filecopy)
	{
		impl->filecopy->AddRef();
		file.impl = impl->filecopy;
	}
	else if (impl->filecontainer)
	{
		file = impl->filecontainer->OpenFile(impl->filename, "rb");
		if (!file.impl) { ZL_LOG1("FILE", "Could not open managed file %s", impl->filename.c_str()); }
	}
	else
	{
		ZL_RWops *rwop = ZL_RWopsFile::Open(impl->filename.c_str(), "rb");
		if (rwop) file.impl = new ZL_File_Impl(impl->filename, rwop);
		if (!file.impl || !file.impl->src) { ZL_LOG1("FILE", "Could not open file %s", impl->filename.c_str()); }
	}
	return file;
}

ZL_String ZL_FileLink::Name() const
{
	return (impl->filename.length() ? impl->filename : ZL_String("Memory File"));
}

int ZL_FileLink::RefCount() { return (impl ? impl->GetRefCount() : 0); }

bool ZL_FileLink::operator<(const ZL_FileLink& b) const
{
	if (!impl || !b.impl) return false;
	if (impl->filecopy != b.impl->filecopy) return impl->filecopy < b.impl->filecopy;
	return (impl->filename < b.impl->filename);
}
