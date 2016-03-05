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

#ifndef __ZL_FILE__
#define __ZL_FILE__

#include "ZL_String.h"

//This reference an actual file on the file system, data in memory, or a file inside a container
struct ZL_File
{
	ZL_File();
	ZL_File(const char *file, const char *mode = "rb", const struct ZL_FileContainer& archive = DefaultReadFileContainer);
	ZL_File(const ZL_String& file, const char *mode = "rb", const struct ZL_FileContainer& archive = DefaultReadFileContainer);
	ZL_File(void *mem, size_t size);
	ZL_File(const void *mem, size_t size);
	~ZL_File();
	ZL_File(const ZL_File &source);
	ZL_File & operator=(const ZL_File &source);
	operator bool () const;
	bool operator==(const ZL_File &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_File &b) const { return (impl!=b.impl); }

	size_t SetContents(const ZL_String& data);
	size_t SetContents(const unsigned char* data, size_t size);
	ZL_String GetContents() const;
	size_t GetContents(ZL_String& out) const;
	size_t GetContents(std::vector<unsigned char>& out) const;
	size_t GetContents(unsigned char* buf, size_t bufsize) const;
	char* GetContentsMalloc(size_t* size) const; //need to call free() on return

	size_t Size() const;
	const ZL_String& GetFileName() const;

	static bool Exists(const char *file, const struct ZL_FileContainer& archive = DefaultReadFileContainer);

	static struct ZL_FileContainer DefaultReadFileContainer;

	private: struct ZL_File_Impl* impl; friend struct ZL_FileLink;
};

//Abstract file container class for file archives, etc.
struct ZL_FileContainer
{
	ZL_FileContainer();
	~ZL_FileContainer();
	ZL_FileContainer(const ZL_FileContainer &source);
	ZL_FileContainer & operator=(const ZL_FileContainer &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_FileContainer &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_FileContainer &b) const { return (impl!=b.impl); }

	struct ZL_File Open(const char *file, const char *mode = "rb") const;

	protected: struct ZL_FileContainer_Impl* impl; friend struct ZL_File;
};

//A way to reference a file without actually opening the file
struct ZL_FileLink
{
	ZL_FileLink();
	ZL_FileLink(const ZL_File &linksource);
	ZL_FileLink(const ZL_String &file, const ZL_FileContainer& archive = ZL_File::DefaultReadFileContainer);
	ZL_FileLink(const char *file);
	~ZL_FileLink();
	ZL_FileLink(const ZL_FileLink &source);
	ZL_FileLink & operator=(const ZL_FileLink &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_FileLink &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_FileLink &b) const { return (impl!=b.impl); }
	bool operator<(const ZL_FileLink& b) const;
	int RefCount();

	ZL_File Open() const;
	ZL_String Name() const;

	private: struct ZL_FileLink_Impl* impl;
};

//An actual file container implementation for ZIP archive files
struct ZL_FileContainer_ZIP : public ZL_FileContainer
{
	ZL_FileContainer_ZIP() { }
	ZL_FileContainer_ZIP(const ZL_File& zipfile);
	ZL_FileContainer_ZIP(const ZL_File& zipfile, const char *defaultFolder);
};

#endif //__ZL_FILE__
