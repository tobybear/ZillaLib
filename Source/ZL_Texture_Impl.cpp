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

#include "ZL_Texture_Impl.h"
#include <map>
#include <assert.h>
#include "stb/stb_image.h"

static int stdio_read(void *user, char *data, int size) { return (int)ZL_RWread((ZL_RWops*)user, data, 1, size); }
static void stdio_skip(void *user, int n) { ZL_RWseektell((ZL_RWops*)user, n, RW_SEEK_CUR); }
static int stdio_eof(void *user) { return ZL_RWeof((ZL_RWops*)user); }
static const stbi_io_callbacks stbi_zlrwops_callbacks = { stdio_read, stdio_skip, stdio_eof };

static std::map<ZL_FileLink, ZL_Texture_Impl*>* pLoadedTextures = NULL;
#ifdef ZL_VIDEO_WEAKCONTEXT
static std::vector<ZL_Texture_Impl*> *pLoadedFrameBufferTextures = NULL;
#endif

// Returns the smallest power-of-two value in which 'val' fits, with a max size of 'max'
static GLint OGL_GetPOT(GLint val, GLint max)
{
	--val;
	for (GLint i = 1; i < 32; i <<= 1) val |= val >> i;
	return (val + 1 < max ? val + 1 : max);
}
#define OGL_IsPOT(v) ((v & (v - 1)) == 0)

// If possible, probes the OpenGL driver with a specified texture size and format. Returns true if the size and format is ok. */
#if defined(ZL_VIDEO_OPENGL1)
static bool OGL_ProbeTexture(GLint w, GLint h, GLint maxSize, GLint bpp, GLenum format)
{
	GLint size;
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, bpp, w, h, 0, format, GL_UNSIGNED_BYTE, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size);
	if(!size) return w == OGL_GetPOT(w, maxSize) && h == OGL_GetPOT(h, maxSize);
	if (size != w) return false;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size);
	return size == h;
}
#elif defined(ZL_VIDEO_OPENGL_ES1)
static bool OGL_ProbeTexture(GLint w, GLint h, GLint maxSize, GLint, GLenum)
{
	return OGL_IsPOT(w) && OGL_IsPOT(h);
}
#else
#define OGL_ProbeTexture(a,b,c,d,e) true
#endif

ZL_Texture_Impl* ZL_Texture_Impl::LoadTextureRef(const ZL_FileLink& file, ZL_BitmapSurface* out_surface)
{
	if (!pLoadedTextures) pLoadedTextures = new std::map<ZL_FileLink, ZL_Texture_Impl*>();
	std::map<ZL_FileLink, ZL_Texture_Impl*>::iterator it = pLoadedTextures->find(file);
	if (it != pLoadedTextures->end())
	{
		it->second->AddRef();
		if (out_surface) it->second->LoadSurfaceData(file.Open(), out_surface);
		return it->second;
	}
	ZL_Texture_Impl* t = new ZL_Texture_Impl(file.Open(), out_surface);
	pLoadedTextures->operator[](file) = t;
	return t;
}

bool ZL_Texture_Impl::LoadPixelsRGBA(const ZL_File& file, unsigned char** pixels, int *w, int *h)
{
	ZL_File_Impl* fileimpl = ZL_ImplFromOwner<ZL_File_Impl>(file);
	if (!fileimpl) { return false; }
	*pixels = stbi_load_from_callbacks(&stbi_zlrwops_callbacks, fileimpl->src, w, h, NULL, 4);
	if (!pixels || !w || !h) { ZL_LOG2("TEXTURE", "Cannot load image file: %s (err: %s)", fileimpl->filename.c_str(), stbi_failure_reason()); return false; }
	return true;
}

ZL_Texture_Impl::ZL_Texture_Impl(const ZL_File& file, ZL_BitmapSurface* out_surface) : gltexid(0), wraps(GL_CLAMP_TO_EDGE), wrapt(GL_CLAMP_TO_EDGE), filtermin(GL_LINEAR), filtermag(GL_LINEAR), pFrameBuffer(NULL)
{
	LoadSurfaceAndTexture(file, out_surface);
}

unsigned char* ZL_Texture_Impl::LoadSurfaceData(const ZL_File& file, ZL_BitmapSurface* out_surface)
{
	ZL_File_Impl* fileimpl = ZL_ImplFromOwner<ZL_File_Impl>(file);
	if (!fileimpl) { return NULL; }

	int BytesPerPixel;
	unsigned char* pixels = stbi_load_from_callbacks(&stbi_zlrwops_callbacks, fileimpl->src, &w, &h, &BytesPerPixel, 0);
	if (!pixels || !w || !h) { ZL_LOG2("TEXTURE", "Cannot load image file: %s (err: %s)", fileimpl->filename.c_str(), stbi_failure_reason()); return NULL; }
	//ZL_LOG4("SURFACE", "Loaded bitmap: %s - x: %d - y: %d - bpp: %d", fileimpl->filename.c_str(), surface->w, surface->h, surface->BytesPerPixel);

	size_t pitch = w * BytesPerPixel;
	unsigned char StackTempRow[1024], *TempRow = (pitch > 1024 ? (unsigned char*)malloc(pitch) : StackTempRow);
	for (unsigned char *rowTop = pixels, *rowTopEnd = rowTop + (h/2)*pitch, *rowBottom = rowTop + (h-1)*pitch; rowTop != rowTopEnd; rowTop += pitch, rowBottom -= pitch)
	{
		memcpy(TempRow, rowTop, pitch);
		memcpy(rowTop, rowBottom, pitch);
		memcpy(rowBottom, TempRow, pitch);
	}
	if (pitch > 1024) free(TempRow);

	wRep = wTex = w;
	hRep = hTex = h;

	/* Get the color format of the surface */
	if      (BytesPerPixel==4) format = GL_RGBA;
	else if (BytesPerPixel==3) format = GL_RGB;
	else if (BytesPerPixel==2) format = GL_LUMINANCE_ALPHA;
	else if (BytesPerPixel==1) format = GL_LUMINANCE;
	else { ZL_LOG2("SURFACE", "Cannot load image file %s with unsupported %d bytes per pixel", fileimpl->filename.c_str(), BytesPerPixel); free(pixels); return NULL; }

	static GLint maxSize = 0;
	if (!maxSize) glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	if (wTex > maxSize || hTex > maxSize)
	{
		if (wTex > maxSize) wTex = maxSize;
		if (hTex > maxSize) hTex = maxSize;
	}
	else if (!OGL_ProbeTexture(wTex, hTex, maxSize, surface->BytesPerPixel, format))
	{
		wTex = OGL_GetPOT(wTex, maxSize);
		hTex = OGL_GetPOT(hTex, maxSize);
		if(!OGL_ProbeTexture(wTex, hTex, maxSize, 4, GL_RGBA)) { ZL_LOG3("SURFACE", "Cannot load image file %s with size %d x %d due to being unsupported by GPU", fileimpl->filename.c_str(), wTex, hTex); free(pixels); return NULL; }
	}

	bool reBlit = false;
	#ifdef ZL_VIDEO_DIRECT3D
	if (format != GL_RGBA)
	{
		//Direct3D only supports RGBA format
		reBlit = true;
	}
	#elif !defined(ZL_VIDEO_OPENGL_ES1)
	if (format != GL_RGBA && ((wTex&1)||(hTex&1)||(wTex&2)||(hTex&2)))
	{
		//somehow non RGBA textures need to be dividable by 4 sizes or RGBA on PC OpenGL and GLES2
		reBlit = true; //reblit to RGBA format
	}
	#endif

	if (reBlit)
	{
		// The original surface size and/or color format wasn't ok for direct conversion. Do an RGBA-copy of it with the suggested size
		ZL_LOG3("SURFACE", (w > wTex || h > hTex ? "Sizing image from file %s down to %d x %d due to being over maximum GPU supported size - Image should be scaled down" : (w < wTex || h < hTex ? "Adding borders to texture from file %s to increase size to %d x %d due to unsupported format - Image should be stored in this size to avoid slow loading" : "Converting texture from file %s to RGBA due to its size %d x %d not being dividable by 4 - Image should be stored in such a size or as RGBA to avoid slow loading")), fileimpl->filename.c_str(), wTex, hTex);
		format = GL_RGBA;
		int srcbpp = BytesPerPixel, srcw = w;
		unsigned char * const srcpixels = pixels;
		if (wTex < w) w = wTex;
		if (hTex < h) h = hTex;
		BytesPerPixel = 4;
		pixels = (unsigned char*)malloc(w*h*4);
		unsigned char *src = (unsigned char *)srcpixels;
		unsigned int *dst = (unsigned int *)pixels;
		for (int height = h, srcskip = (srcw>w?(srcw-w)*srcbpp:0); height--; src+=srcskip)
		{
			int c = w;
			switch (srcbpp)
			{
				case 1:
					for (; c--; src += srcbpp)
						*dst++ = src[0] + (src[0] << 8) + (src[0] << 16) + (0xFF << 24);
					break;
				case 2:
					for (; c--; src += srcbpp)
						*dst++ = src[0] + (src[0] << 8) + (src[0] << 16) + (src[1] << 24);
					break;
				case 3:
					for (; c--; src += srcbpp)
						#ifndef ZL_USE_BIGENDIAN
						*dst++ = src[0] + (src[1] << 8) + (src[2] << 16) + (0xFF << 24);
						#else
						*dst++ = (src[0] << 16) + (src[1] << 8) + src[2] + (0xFF << 24);
						#endif
					break;
				case 4:
					memcpy(dst, src, 4*w);
					dst += w;
					src += w*4;
					break;
			}
		}
		free(srcpixels);
	}

	if (out_surface)
	{
		out_surface->BytesPerPixel = BytesPerPixel;
		out_surface->w = wTex;
		out_surface->h = hTex;
		out_surface->pixels = pixels;
	}
	return pixels;
}

bool ZL_Texture_Impl::LoadSurfaceAndTexture(const ZL_File& file, ZL_BitmapSurface* out_surface)
{
	unsigned char* pixels = LoadSurfaceData(file, out_surface);
	if (!pixels) return false;
	glGenTextures(1, &gltexid);
	glBindTexture(GL_TEXTURE_2D, gltexid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtermin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtermag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, (!(w&7) ? 8 : (!(w&3) ? 4 : (!(w&1) ? 2 : 1))));
	if (wTex != w || hTex != h)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, wTex, hTex, 0, format, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, pixels);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, format, wTex, hTex, 0, format, GL_UNSIGNED_BYTE, pixels);
	if (!out_surface) free(pixels);
	return true;
}

void ZL_Texture_Impl::SetTextureFilter(GLint newfiltermin, GLint newfiltermag)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtermin = newfiltermin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtermag = newfiltermag);
}

void ZL_Texture_Impl::SetTextureWrap(GLint newwraps, GLint newwrapt)
{
	#if defined(ZL_VIDEO_OPENGL_ES1) || defined(ZL_VIDEO_OPENGL_ES2)
	if ((newwraps == GL_REPEAT || newwrapt == GL_REPEAT)
		#ifdef ZL_VIDEO_OPENGL_ES1
			&& (wTex != w || hTex != h)
		#else
			&& (!OGL_IsPOT(wTex) || !OGL_IsPOT(hTex))
		#endif
	)
	{
		#ifdef ZL_VIDEO_OPENGL_ES2
		//GLscalar texcoordw = w / ((GLscalar)wTex), texcoordh = h / ((GLscalar)hTex);
		wTex = OGL_GetPOT(wTex, 0x7FFFFFFF); hTex = OGL_GetPOT(hTex, 0x7FFFFFFF);
		#endif
		//ZL_LOG2("SURFACE", "                 scale for repeat: x: %d - y: %d", wTex, hTex);
		GLuint gltexidfill, glfb; //, oldglfb;
		glGenFramebuffers(1, &glfb);
		glGenTextures(1, &gltexidfill);
		glBindTexture(GL_TEXTURE_2D, gltexidfill);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, format, wTex, hTex, 0, format, GL_UNSIGNED_BYTE, NULL);
		//glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&oldglfb);
		glBindFramebuffer(GL_FRAMEBUFFER, glfb);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gltexidfill, 0);
		//#ifdef GL_VIEWPORT_BIT
		//glPushAttrib(GL_VIEWPORT_BIT);
		//#else
		//GLint viewport[4];
		//glGetIntegerv(GL_VIEWPORT, viewport);
		//#endif
		glViewport(0, 0, wTex, hTex);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, gltexid);
		#ifdef ZL_VIDEO_OPENGL_ES1
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		GLscalar texcoordw = w / ((GLscalar)wTex);
		GLscalar texcoordh = h / ((GLscalar)hTex);
		GLscalar texcoordbox[8] = { 0 , texcoordh , texcoordw , texcoordh , 0 , 0 , texcoordw , 0 };
		GLscalar verticesbox[8] = { -1,1 , 1,1 , -1,-1 , 1,-1 };
		glColor4(1,1,1,1);
		glTexCoordPointer(2, GL_SCALAR, 0, texcoordbox);
		glVertexPointer(2, GL_SCALAR, 0, verticesbox);
		glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		#else
		GLscalar fullbox[8] = { 0,1 , 1,1 , 0,0 , 1,0 };
		GLscalar matrix[16] = { 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, -1, 0, -1, -1, 0, 1 };
		//The buffers below would resize a POT texture with a NONPOT texture on the topleft. But so far all GLES2 platforms support NONPOT textures
		//GLscalar veccoordbox[8] = { -1,1 , 1,1 , -1,-1 , 1,-1 };
		//GLscalar texcoordbox[8] = { 0 , texcoordh , texcoordw , texcoordh , 0 , 0 , texcoordw , 0 };
		//GLscalar matrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		ZLGLSL::_TEXTURE_PROGRAM_ACTIVATE();
		glVertexAttrib4(ZLGLSL::ATTR_COLOR, 1, 1, 1, 1);
		glUniformMatrix4v(ZLGLSL::UNI_MVP, 1, GL_FALSE, matrix);
		ZLGL_VERTEXTPOINTER(2, GL_SCALAR, 0, fullbox);
		ZLGL_TEXCOORDPOINTER(2, GL_SCALAR, 0, fullbox);
		glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
		ZLGLSL::ActiveProgram = ZLGLSL::NONE; //reset matrix on next draw
		#endif
		//#ifdef GL_VIEWPORT_BIT
		//glPopAttrib();
		//#else
		//glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		//#endif
		glViewport(active_viewport[0], active_viewport[1], active_viewport[2], active_viewport[3]);
		glBindFramebuffer(GL_FRAMEBUFFER, active_framebuffer);
		glDeleteFramebuffers(1, &glfb);
		glDeleteTextures(1, &gltexid);
		gltexid = gltexidfill; w = wTex; h = hTex;
		if (pFrameBuffer) { pFrameBuffer->viewport[2] = wTex; pFrameBuffer->viewport[3] = hTex; }
	}
	#endif
	glBindTexture(GL_TEXTURE_2D, gltexid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps = newwraps);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapt = newwrapt);
}

ZL_Texture_Impl::~ZL_Texture_Impl()
{
	if (!pFrameBuffer)
		for (std::map<ZL_FileLink, ZL_Texture_Impl*>::iterator it = pLoadedTextures->begin(); it != pLoadedTextures->end(); ++it)
			if (it->second == this) { pLoadedTextures->erase(it); break; }
	#ifdef ZL_VIDEO_WEAKCONTEXT
	if (pFrameBuffer)
	{
		if (pFrameBuffer->pStorePixelData) free(pFrameBuffer->pStorePixelData);
		for (std::vector<ZL_Texture_Impl*>::iterator it = pLoadedFrameBufferTextures->begin(); it != pLoadedFrameBufferTextures->end(); ++it)
			if (*it == this) { pLoadedFrameBufferTextures->erase(it); break; }
	}
	#endif
	if (pFrameBuffer) { glDeleteFramebuffers(1, &pFrameBuffer->glFB); delete pFrameBuffer; }
	if (gltexid) glDeleteTextures(1, &gltexid);
}

ZL_Texture_Impl::ZL_Texture_Impl(int width, int height, bool use_alpha) : gltexid(0), wraps(GL_CLAMP_TO_EDGE), wrapt(GL_CLAMP_TO_EDGE), filtermin(GL_LINEAR), filtermag(GL_LINEAR), pFrameBuffer(NULL)
{
	#ifdef ZL_VIDEO_OPENGL2
	if (glGenFramebuffers == NULL) return;
	#endif
	format = (use_alpha ? GL_RGBA : GL_RGB);
	pFrameBuffer = new ZL_TextureFrameBuffer();
	#ifdef ZL_VIDEO_WEAKCONTEXT
	pFrameBuffer->pStorePixelData = NULL;
	#endif
	SetupFrameBuffer(width, height);
	#ifdef ZL_VIDEO_WEAKCONTEXT
	if (!pLoadedFrameBufferTextures) pLoadedFrameBufferTextures = new std::vector<ZL_Texture_Impl*>();
	pLoadedFrameBufferTextures->push_back(this);
	#endif
}

void ZL_Texture_Impl::SetupFrameBuffer(int width, int height)
{
	w = wTex = wRep = width;
	h = hTex = hRep = height;
	glGenTextures(1, &gltexid);
	glGenFramebuffers(1, &pFrameBuffer->glFB);
	pFrameBuffer->viewport[0] = pFrameBuffer->viewport[1] = 0;
	glBindTexture(GL_TEXTURE_2D, gltexid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtermin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtermag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	#ifndef ZL_VIDEO_WEAKCONTEXT
	glTexImage2D(GL_TEXTURE_2D, 0, format, (pFrameBuffer->viewport[2] = w), (pFrameBuffer->viewport[3] = h), 0, format, GL_UNSIGNED_BYTE, NULL);
	#else
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format, (pFrameBuffer->viewport[2] = w), (pFrameBuffer->viewport[3] = h), 0, format, GL_UNSIGNED_BYTE, pFrameBuffer->pStorePixelData);
	if (pFrameBuffer->pStorePixelData) { free(pFrameBuffer->pStorePixelData); pFrameBuffer->pStorePixelData = NULL; }
	#endif
	glBindFramebuffer(GL_FRAMEBUFFER, pFrameBuffer->glFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gltexid, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, active_framebuffer);
}

static ZL_TextureFrameBuffer *pActiveFrameBuffer = NULL;

void ZL_Texture_Impl::FrameBufferBegin(bool clear)
{
	#if defined(ZL_VIDEO_OPENGL_ES1) || defined(ZL_VIDEO_OPENGL_ES2)
	//Some (older?) GLES hardware implementations require an "empty" render call like this glClear with no bits set
	//Because if the framebuffer was to be used while the main window render buffer was still processing the last frame,
	//artifacts would appear inside this framebuffer. With this call we wait until the GPU is ready for more (FBO) rendering.
	//glFlush or glFinish might seem the more appropriate solution, but tests have shown them to not work.
	glClear(0);
	#endif
	pFrameBuffer->pPrevFrameBuffer = pActiveFrameBuffer;
	pActiveFrameBuffer = pFrameBuffer;
	active_viewport = pFrameBuffer->viewport;
	active_framebuffer = pFrameBuffer->glFB;
	glBindFramebuffer(GL_FRAMEBUFFER, active_framebuffer);
	glViewport(active_viewport[0], active_viewport[1], active_viewport[2], active_viewport[3]);
	if (clear)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void ZL_Texture_Impl::FrameBufferEnd()
{
	assert(pActiveFrameBuffer == pFrameBuffer);
	#if defined(ZL_VIDEO_OPENGL_ES1) || defined(ZL_VIDEO_OPENGL_ES2)
	if (format == GL_RGB)
	{
		//on GLES textures are internally always RGBA, thus clear alpha channel on actual RGB only framebuffer textures
		//it would be easier with glClear(GL_COLOR_BUFFER_BIT) but some hardware implementations dont like clearing only alpha with it
		//A different approach would be during the rendering of this ZL_Texture_Impl by checking pFrameBuffer and format==GL_RGB, then glDisable GL_BLEND
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
		ZLGL_DISABLE_TEXTURE();
		glVertexAttrib4(ZLGLSL::ATTR_COLOR, 0, 0, 0, 1);
		GLPUSHMATRIX();
		GLLOADIDENTITY();
		GLscalar vtx[8] = { -1 , 1 , 1 , 1 , -1 , -1 , 1 , -1 };
		ZLGL_VERTEXTPOINTER(2, GL_SCALAR, 0, vtx);
		glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
		GLPOPMATRIX();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	#endif
	pActiveFrameBuffer = pFrameBuffer->pPrevFrameBuffer;
	active_viewport = (pActiveFrameBuffer ? pActiveFrameBuffer->viewport : window_viewport);
	active_framebuffer = (pActiveFrameBuffer ? pActiveFrameBuffer->glFB : window_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, active_framebuffer);
	glViewport(active_viewport[0], active_viewport[1], active_viewport[2], active_viewport[3]);
}

#ifdef ZL_VIDEO_WEAKCONTEXT
#ifndef ZL_VIDEO_USE_GLSL
bool CheckTexturesIfContextLost()
{
	return (pLoadedTextures && pLoadedTextures->size() && !glIsTexture(pLoadedTextures->begin()->second->gltexid));
}
#endif
void RecreateAllTexturesOnContextLost()
{
	if (pLoadedTextures)
	{
		//for (std::map<ZL_FileLink, ZL_Texture_Impl*>::iterator itx = pLoadedTextures->begin(); itx != pLoadedTextures->end(); ++itx) glDeleteTextures(1, &itx->second->gltexid); //DEBUG
		ZL_LOG1("TEXTURE", "RecreateAllTexturesIfContextLost with %d textures to reload", pLoadedTextures->size());
		for (std::map<ZL_FileLink, ZL_Texture_Impl*>::iterator it = pLoadedTextures->begin(); it != pLoadedTextures->end(); ++it)
		{
			ZL_LOG2("TEXTURE", "   Reload Tex ID: %d (%s)", it->second->gltexid, it->first.Name().c_str());
			if (!it->second->LoadSurfaceAndTexture(it->first.Open())) continue;
			it->second->SetTextureWrap(it->second->wraps, it->second->wrapt);
		}
	}
	if (pLoadedFrameBufferTextures)
	{
		ZL_LOG1("TEXTURE", "RecreateAllTexturesIfContextLost with %d framebuffer textures to reload", pLoadedFrameBufferTextures->size());
		for (std::vector<ZL_Texture_Impl*>::iterator it = pLoadedFrameBufferTextures->begin(); it != pLoadedFrameBufferTextures->end(); ++it)
		{
			ZL_LOG4("TEXTURE", "   Reload Framebuffer Tex ID: %d (size: %d x %d - has buffer: %d)", (*it)->gltexid, (*it)->wRep, (*it)->hRep, ((*it)->pFrameBuffer->pStorePixelData != NULL));
			(*it)->SetupFrameBuffer((*it)->wRep, (*it)->hRep);
			(*it)->SetTextureWrap((*it)->wraps, (*it)->wrapt);
		}
	}
}
void StoreAllFrameBufferTexturesOnDeactivate()
{
	if (!pLoadedFrameBufferTextures) return;
	ZL_LOG1("TEXTURE", "StoreAllFrameBufferTexturesOnDeactivate with %d framebuffer textures to store", pLoadedFrameBufferTextures->size());
	for (std::vector<ZL_Texture_Impl*>::iterator it = pLoadedFrameBufferTextures->begin(); it != pLoadedFrameBufferTextures->end(); ++it)
	{
		ZL_LOG4("TEXTURE", "Storing framebuffer textures with size %d x %d x %d bpp (has already: %d)", (*it)->wRep, (*it)->hRep, ((*it)->format == GL_RGBA ? 4 : 3), ((*it)->pFrameBuffer->pStorePixelData != NULL));
		if ((*it)->pFrameBuffer->pStorePixelData) continue;
		//glReadPixels on OpenGLES is limited to RGBA reads only!
		(*it)->pFrameBuffer->pStorePixelData = malloc((*it)->wRep * (*it)->hRep * 4);
		(*it)->FrameBufferBegin(false);
		glReadPixels(0, 0, (*it)->wRep, (*it)->hRep, GL_RGBA, GL_UNSIGNED_BYTE, (*it)->pFrameBuffer->pStorePixelData);
		(*it)->FrameBufferEnd();
		if ((*it)->format == GL_RGB)
		{
			//realign RGBA to RGB
			unsigned char *p = ((unsigned char*)((*it)->pFrameBuffer->pStorePixelData)) + 3, *pr = ((unsigned char*)((*it)->pFrameBuffer->pStorePixelData)) + 4;
			for (int i = (*it)->wRep * (*it)->hRep; i > 0; i--, p+=3, pr+=4) { p[0] = pr[0]; p[1] = pr[1]; p[2] = pr[2]; }
		}
	}
}
#endif
