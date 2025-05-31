/*
  ZillaLib
  Copyright (C) 2010-2020 Bernhard Schelling

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

#include "ZL_Audio.h"
#include "ZL_File_Impl.h"
#include "ZL_Platform.h"
#include "ZL_Math.h"
#include "ZL_File.h"
#include "ZL_File_Impl.h"
#include "ZL_Display_Impl.h"
#include <vector>
#include <string.h>

#include "stb/stb_vorbis.h"
//#include "tremor/ivorbisfile.h"
#if (defined(STB_VORBIS_INCLUDE_STB_VORBIS_H) && defined(_OV_FILE_H_))
#error can only use either STB or TREMOR
#elif (!defined(STB_VORBIS_INCLUDE_STB_VORBIS_H) && !defined(_OV_FILE_H_))
#error include either STB or TREMOR headers
#elif defined(STB_VORBIS_INCLUDE_STB_VORBIS_H)
#define OGG_READ(H, buffer, samples) (stb_vorbis_get_samples_short_interleaved((stb_vorbis*)(H), ((stb_vorbis_info*)(H))->channels, (short*)(buffer), (int)(samples))*((stb_vorbis_info*)(H))->channels)
#define OGG_SEEKRESET(H) stb_vorbis_seek((stb_vorbis*)(H),0) //stb_vorbis_seek_start((stb_vorbis*)(H));
#elif defined(_OV_FILE_H_)
#define OGG_READ(H, buffer, samples) ov_read((OggVorbis_File*)(H), (char*)(buffer), (int)((samples)<<1), NULL)
#define OGG_SEEKRESET(H) ov_raw_seek((OggVorbis_File*)(H), 0);
static size_t ogg_read_func(void *ptr, size_t size, size_t num, void *src) { return ZL_RWread((ZL_RWops*)src, ptr, size, num); }
static int ogg_seek_func(void *src, ogg_int64_t offset, int mode) { return ZL_RWseek((ZL_RWops*)src, (int)offset, mode); }
static long ogg_tell_func(void *src) { return (long)ZL_RWtell((ZL_RWops*)src); }
#endif

struct ZL_AudioPlayingHandle
{
	struct ZL_Sound_Impl *snd;
	unsigned int pos;
	bool loop, paused;
	bool mix_into(short* buf, unsigned int rem, bool add);
};

static std::vector<ZL_AudioPlayingHandle> *ZL_AudioActive = NULL;
static std::vector<bool(*)(short*, unsigned int, bool)> *ZL_AudioMixFuncs = NULL;
static ZL_MutexHandle ZL_AudioActiveMutex;
static float audio_global_factor = 1.0f;

bool ZL_Audio::Init()
{
	if (ZL_AudioActive) return false;
	ZL_AudioActive = new std::vector<ZL_AudioPlayingHandle>();
	ZL_MutexInit(ZL_AudioActiveMutex);
	if (!ZL_AudioOpen()) { ZL_MutexDestroy(ZL_AudioActiveMutex); delete ZL_AudioActive; ZL_AudioActive = NULL; return false; }
	return true;
}

/*
void ZL_Audio::Close()
{
	ZL_MutexDestroy(ZL_AudioActiveMutex);
	delete ZL_AudioActive;
	ZL_AudioActive = NULL;
}
*/

void ZL_Audio::SetGlobalSpeedFactor(scalar factor)
{
	audio_global_factor = MAX(0.001f, (float)factor);
}

void ZL_Audio::HookAudioMix(bool (*pFuncAudioMix)(short* buffer, unsigned int samples, bool need_mix))
{
	if (!ZL_AudioMixFuncs) ZL_AudioMixFuncs = new std::vector<bool(*)(short*, unsigned int, bool)>();
	else if (std::find(ZL_AudioMixFuncs->begin(), ZL_AudioMixFuncs->end(), pFuncAudioMix) != ZL_AudioMixFuncs->end()) return;
	ZL_AudioMixFuncs->push_back(pFuncAudioMix);
}

void ZL_Audio::UnhookAudioMix(bool (*pFuncAudioMix)(short* buffer, unsigned int samples, bool need_mix))
{
	if (!ZL_AudioMixFuncs) return;
	std::vector<bool(*)(short*, unsigned int, bool)>::iterator it = std::find(ZL_AudioMixFuncs->begin(), ZL_AudioMixFuncs->end(), pFuncAudioMix);
	if (it != ZL_AudioMixFuncs->end()) ZL_AudioMixFuncs->erase(it);
	if (!ZL_AudioMixFuncs->size()) { delete ZL_AudioMixFuncs; ZL_AudioMixFuncs = NULL; }
}

void ZL_Audio::LockAudioThread()
{
	if (ZL_AudioActive) ZL_MutexLock(ZL_AudioActiveMutex);
}

void ZL_Audio::UnlockAudioThread()
{
	if (ZL_AudioActive) ZL_MutexUnlock(ZL_AudioActiveMutex);
}

struct ZL_Sound_Impl : ZL_Impl
{
	ZL_File_Impl* stream_fh;
	ZL_Sound_Impl* clone_base;
	void *decoder; //audio decoder handle
	short* audiodata;
	unsigned int totalsamples, numactive, fixshift;
	float audiofactor, audiovol;
	#if defined(__IPHONEOS__)
	void *IOS_AudioPlayer;
	#define ZL_SOUND_IMPL_PLATFORM_INIT , IOS_AudioPlayer(NULL)
	#elif defined(__ANDROID__)
	void *Android_AudioPlayer;
	#define ZL_SOUND_IMPL_PLATFORM_INIT , Android_AudioPlayer(NULL)
	#else
	#define ZL_SOUND_IMPL_PLATFORM_INIT
	#endif

	ZL_Sound_Impl() : stream_fh(NULL), clone_base(NULL), decoder(NULL), audiodata(NULL), totalsamples(0), numactive(0), fixshift(0), audiofactor(1), audiovol(1) ZL_SOUND_IMPL_PLATFORM_INIT {}

	ZL_Sound_Impl(ZL_Sound_Impl* b) : stream_fh(NULL), clone_base(b), decoder(NULL), audiodata(b->audiodata), totalsamples(b->totalsamples), numactive(0), fixshift(b->fixshift), audiofactor(b->audiofactor), audiovol(b->audiovol) ZL_SOUND_IMPL_PLATFORM_INIT { b->AddRef(); }

	~ZL_Sound_Impl()
	{
		if (ZL_AudioActive && numactive) Stop();
		#if defined(__IPHONEOS__)
		if (IOS_AudioPlayer) { ZL_AudioPlayerRelease(IOS_AudioPlayer); IOS_AudioPlayer = NULL; }
		#elif defined(__ANDROID__)
		if (Android_AudioPlayer) { ZL_AudioAndroidRelease(Android_AudioPlayer); Android_AudioPlayer = NULL; return; }
		#endif
		if (clone_base) clone_base->DelRef();
		else if (audiodata) free(audiodata);
		#if defined(STB_VORBIS_INCLUDE_STB_VORBIS_H)
		if ((stb_vorbis*)decoder) stb_vorbis_close((stb_vorbis*)decoder);
		#elif defined(_OV_FILE_H_)
		if ((OggVorbis_File*)decoder) delete (OggVorbis_File*)decoder;
		#endif
		if (stream_fh) stream_fh->DelRef();
	}

	void Play(bool looped, bool startPaused)
	{
		#if defined(__IPHONEOS__)
		if (IOS_AudioPlayer) { ZL_AudioPlayerPlay(IOS_AudioPlayer, looped); if (startPaused) ZL_AudioPlayerPause(IOS_AudioPlayer); return; }
		#elif defined(__ANDROID__)
		if (Android_AudioPlayer) { ZL_AudioAndroidPlay(Android_AudioPlayer, looped); if (startPaused) ZL_AudioAndroidPause(Android_AudioPlayer); return; }
		#endif
		if (decoder && numactive) Stop(); //stop streamed file before playing it again
		numactive++;
		ZL_AudioPlayingHandle a;
		a.snd = this;
		a.loop = looped;
		a.pos = 0;
		a.paused = startPaused;
		ZL_MutexLock(ZL_AudioActiveMutex);
		ZL_AudioActive->push_back(a);
		ZL_MutexUnlock(ZL_AudioActiveMutex);
	}

	void Stop()
	{
		#if defined(__IPHONEOS__)
		if (IOS_AudioPlayer) { ZL_AudioPlayerStop(IOS_AudioPlayer); return; }
		#elif defined(__ANDROID__)
		if (Android_AudioPlayer) { ZL_AudioAndroidStop(Android_AudioPlayer); return; }
		#endif
		const unsigned int old_numactive = numactive;
		ZL_MutexLock(ZL_AudioActiveMutex);
		for (std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin(); it != ZL_AudioActive->end(); )
			if (it->snd == this) { it = ZL_AudioActive->erase(it); numactive--; }
			else ++it;
		ZL_MutexUnlock(ZL_AudioActiveMutex);
		if (numactive < old_numactive && decoder) OGG_SEEKRESET(decoder);
	}

	void Pause()
	{
		#if defined(__IPHONEOS__)
		if (IOS_AudioPlayer) { ZL_AudioPlayerPause(IOS_AudioPlayer); return; }
		#elif defined(__ANDROID__)
		if (Android_AudioPlayer) { ZL_AudioAndroidPause(Android_AudioPlayer); return; }
		#endif
		ZL_MutexLock(ZL_AudioActiveMutex);
		for (std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin(); it != ZL_AudioActive->end(); ++it)
			if (it->snd == this) it->paused = true;
		ZL_MutexUnlock(ZL_AudioActiveMutex);
	}

	void Resume()
	{
		#if defined(__IPHONEOS__)
		if (IOS_AudioPlayer) { ZL_AudioPlayerResume(IOS_AudioPlayer); return; }
		#elif defined(__ANDROID__)
		if (Android_AudioPlayer) { ZL_AudioAndroidResume(Android_AudioPlayer); return; }
		#endif
		ZL_MutexLock(ZL_AudioActiveMutex);
		for (std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin(); it != ZL_AudioActive->end(); ++it)
			if (it->snd == this) it->paused = false;
		ZL_MutexUnlock(ZL_AudioActiveMutex);
	}
};

bool ZL_PlatformAudioMix(short *stream, unsigned int bytes)
{
	if (ZL_WINDOWFLAGS_HAS(ZL_WINDOW_MINIMIZED) && !ZL_WINDOWFLAGS_HAS(ZL_WINDOW_MINIMIZEDAUDIO))
	{
		nothingtomix:
		memset(stream, 0, bytes);
		return false;
	}
	bool didFuncAudioMix = false;
	if (ZL_AudioMixFuncs)
	{
		for (std::vector<bool(*)(short*, unsigned int, bool)>::iterator it = ZL_AudioMixFuncs->begin(); it != ZL_AudioMixFuncs->end(); ++it)
		{
			didFuncAudioMix |= (*it)(stream, bytes >> 2, didFuncAudioMix);
		}
		if (didFuncAudioMix && !ZL_AudioActive->size()) return true;
	}
	if (!didFuncAudioMix && !ZL_AudioActive->size()) goto nothingtomix;

	bool haveNothingToMix = false;
	ZL_MutexLock(ZL_AudioActiveMutex);
	std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin();
	while (it != ZL_AudioActive->end() && (it->paused || !it->snd->audiofactor)) ++it;
	if (it == ZL_AudioActive->end()) haveNothingToMix = true;
	else
	{
		unsigned int total_samples = bytes >> 1; //shorts
		if (!it->mix_into(stream, total_samples, didFuncAudioMix)) ++it;
		else { it->snd->numactive--; it = ZL_AudioActive->erase(it); }

		while (it != ZL_AudioActive->end())
		{
			if (it->paused || !it->snd->audiofactor || !it->mix_into(stream, total_samples, true)) ++it;
			else { it->snd->numactive--; it = ZL_AudioActive->erase(it); }
		}
	}
	ZL_MutexUnlock(ZL_AudioActiveMutex);
	if (haveNothingToMix) goto nothingtomix;
	return true;
}

bool ZL_AudioPlayingHandle::mix_into(short* buf, unsigned int rem, bool add)
{
	short tmpbuf[128];
	float factor = audio_global_factor * snd->audiofactor, fdelta = 1.0f;
	const float vol = snd->audiovol;
	if (snd->fixshift) factor /= s(1<<snd->fixshift);
	if (factor < 1.0f && factor > 0.99f) factor = 1.0f; //round factor (avoid issues with direct_write)
	const bool rescale = (factor != 1.0f);
	const bool allow_direct_write = (!add && factor <= 1.0f && vol == 1.0f); //no need for tmpbuf
	const unsigned int fixshift = (rescale ? 0 : snd->fixshift);
	short stmp, *ssrc = buf;
	unsigned int readscaled = 0, read, write;
	do //until no bytes are remaining (or the audio stream is finished without loop)
	{
		if (rescale)
		{
			read = (unsigned int)(rem*factor);
			if (!read || (read & 1)) read += 2-(read & 1); //align to 2 channels and minimal one sample
			readscaled = read;
		}
		else read = rem;

		if (snd->decoder)
		{
			if (!allow_direct_write && read > (unsigned int)COUNT_OF(tmpbuf)) read = (unsigned int)COUNT_OF(tmpbuf);
			ssrc = (allow_direct_write ? buf : tmpbuf);
			int got = OGG_READ(snd->decoder, ssrc, read);
			if (got < 0) break;
			if ((unsigned int)got != read) { read = got; pos = snd->totalsamples; } //no more samples available, loop
		}
		else
		{
			if (read > (snd->totalsamples - pos)) read = (snd->totalsamples - pos);
			ssrc = (snd->audiodata + pos);
		}

		if (rescale)
		{
			write = (readscaled == read ? rem : (unsigned int)((read+.499f)/factor));
			if (!write || (write & 1)) write += 2-(write & 1); //align to 2 channels and minimal one sample
			if (write > rem) write = rem;
			fdelta = (read / (write + .5f));
		}
		else write = read;

		short *s = buf, *send = (buf + write);
		if (allow_direct_write)
		{
			if (rescale) for (float f = 0; send != s; send-=2, f+=fdelta)
			{
				int off = read - (((unsigned int)f)<<1);
				send[-2] = ssrc[off-2];
				send[-1] = ssrc[off-1];
			}
			else if (fixshift) for (unsigned int count = write / sizeof(short); count--;) s[count] = ssrc[count>>fixshift];
			else if (s != ssrc) memcpy(buf, ssrc, write<<1); //shorts
		}
		else if (vol != 1.0f)
		{
			if (vol == 0.0f)
			{
				if (!add) memset(s, 0, write<<1); //shorts
			}
			else if (add) for (float f = 0; s != send; s+=2, f+=fdelta)
			{
				unsigned int off = (((unsigned int)f)<<1);
				if ((stmp = ssrc[  off])) { int tmp = s[0] + (int)(stmp * vol); s[0] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp)); }
				if ((stmp = ssrc[1+off])) { int tmp = s[1] + (int)(stmp * vol); s[1] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp)); }
			}
			else for (float f = 0; s != send; s+=2, f+=fdelta)
			{
				unsigned int off = (((unsigned int)f)<<1);
				{ int tmp = (int)(ssrc[  off] * vol); s[0] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp)); }
				{ int tmp = (int)(ssrc[1+off] * vol); s[1] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp)); }
			}
		}
		else
		{
			if (!rescale) for (; s != send; s++, ssrc++)
			{
				if (*ssrc == 0) continue;
				int tmp = *s + *ssrc;
				*s = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp));
			}
			else if (fixshift) for (unsigned int count = 0; s != send; s++, count++)
			{
				if (!(stmp = ssrc[count>>fixshift])) continue;
				int tmp = *s + stmp;
				*s = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp));
			}
			else if (add) for (float f = 0; s != send; s+=2, f+=fdelta)
			{
				unsigned int off = (((unsigned int)f)<<1);
				if ((stmp = ssrc[  off])) { int tmp = s[0] + stmp; s[0] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp)); }
				if ((stmp = ssrc[1+off])) { int tmp = s[1] + stmp; s[1] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x8000 ? -0x8000 : (short)tmp)); }
			}
			else for (float f = 0; s != send; s+=2, f+=fdelta)
			{
				unsigned int off = (((unsigned int)f)<<1);
				s[0] = ssrc[  off];
				s[1] = ssrc[1+off];
			}
		}
		rem -= write;
		buf += write;
		pos += read;
		if (pos >= snd->totalsamples)
		{
			pos = 0;
			if (snd->decoder) OGG_SEEKRESET(snd->decoder);
			if (!loop && !rem) return true;
		}
		if (!loop && !pos && write) break;
	} while (rem);
	if (rem && !add) memset(buf, 0, rem<<1); //shorts
	return (rem > 0); //rem > 0 means the audio stopped during play and is not looped and needs to be removed from the list of playing samples
}

static ZL_Sound_Impl* ZL_Sound_Load(ZL_File_Impl* file_impl, bool stream)
{
	ZL_ASSERTMSG(ZL_AudioActive, "ZL_Audio::Init needs to be called before using audio functions");
	if (!file_impl || !ZL_AudioActive) return NULL;

	#ifdef __ANDROID__
	if (stream && file_impl->archive == ZL_ImplFromOwner<ZL_FileContainer_Impl>(ZL_File::DefaultReadFileContainer))
	{
		void* player = ZL_AudioAndroidLoad(file_impl);
		if (player)
		{
			ZL_Sound_Impl* ah = new ZL_Sound_Impl();
			ah->Android_AudioPlayer = player;
			return ah;
		}
	}
	#endif

	#ifdef __IPHONEOS__
	if (stream && !file_impl->archive && file_impl->filename.length() && file_impl->filename.c_str()[file_impl->filename.length()-1] == '4')
	{
		//stream MP4
		ZL_Sound_Impl* ah = new ZL_Sound_Impl();
		ah->IOS_AudioPlayer = ZL_AudioPlayerOpen(file_impl->filename);
		ah->stream_fh = file_impl;
		file_impl->AddRef();
		return ah;
	}
	#endif

	if (!file_impl->src) return NULL;
	#if defined(STB_VORBIS_INCLUDE_STB_VORBIS_H)
	stb_vorbis *v = stb_vorbis_open_zlrwops(file_impl->src);
	if (!v) return NULL;

	stb_vorbis_info *vi = (stb_vorbis_info*)v;
	if (vi->sample_rate > 44100) { ZL_LOG2("SOUND", "%s unsupported sample rate: %d (more than 44100)", file_impl->filename.c_str(), vi->sample_rate); }
	if (vi->channels > 2) { ZL_LOG2("SOUND", "%s unsupported number of channels: %d (more than 2)", file_impl->filename.c_str(), vi->channels); }
	if (vi->sample_rate > 44100 || vi->channels > 2) { stb_vorbis_close(v); return NULL; } //incompatible

	ZL_Sound_Impl* ah = new ZL_Sound_Impl();
	ah->totalsamples = stb_vorbis_stream_length_in_samples(v) * vi->channels;
	for (unsigned int r = (44100*2)/(vi->sample_rate*vi->channels); r >>= 1;) ah->fixshift++;

	if (stream)
	{
		ah->decoder = v;
		ah->stream_fh = file_impl;
		file_impl->AddRef();
	}
	else
	{
		ah->audiodata = (short*)malloc(ah->totalsamples<<1); //shorts
		if (ah->audiodata == NULL) { stb_vorbis_close(v); delete ah; return NULL; }
		stb_vorbis_get_samples_short_interleaved(v, vi->channels, (short*)ah->audiodata, (int)ah->totalsamples);
		stb_vorbis_close(v);
	}
	return ah;
	#elif defined(_OV_FILE_H_)
	ov_callbacks callbacks;
	callbacks.read_func = ogg_read_func;
	callbacks.seek_func = ogg_seek_func;
	callbacks.close_func = NULL; //we have our file op class that closes itself
	callbacks.tell_func = ogg_tell_func;

	OggVorbis_File ovf;
	if (ov_open_callbacks(file_impl->src, &ovf, NULL, 0, callbacks) < 0)
		return NULL; //invalid ogg sound stream

	vorbis_info *info = ov_info(&ovf, -1); //frequency = info->rate;
	if (info->rate > 44100 || info->channels > 2) { ov_clear(&ovf); return NULL; } //incompatible

	ZL_Sound_Impl* ah = new ZL_Sound_Impl();
	memset(ah, 0, sizeof(ZL_Sound_Impl));
	ah->audiofactor = 1;
	ah->totalsamples = ((unsigned int)ov_pcm_total(&ovf, -1)) * info->channels;
	for (unsigned int r = (44100*2)/(info->rate*info->channels); r >>= 1;) ah->fixshift++;

	if (stream)
	{
		ah->decoder = (void*) new OggVorbis_File();
		memcpy(ah->decoder, &ovf, sizeof(OggVorbis_File));
		ah->stream_fh = file_impl;
		file_impl->AddRef();
	}
	else
	{

		long bytes = (long)ah->totalsamples<<1, read; //shorts
		ah->audiodata = (short*)malloc(bytes);
		char* buffer = (char*)ah->audiodata;
		while (bytes && (read = ov_read(&ovf, buffer, bytes, NULL)))
		{
			if (read == OV_HOLE || read == OV_EBADLINK) break; // data error
			buffer += read;
			bytes -= read;
		}

		ov_clear(&ovf);
	}
	return ah;
	#endif
}

ZL_Sound ZL_SoundLoadFromBuffer(short* audiodata, unsigned int totalsamples, unsigned int fixshift)
{
	ZL_Sound_Impl* impl = new ZL_Sound_Impl();
	impl->audiodata = audiodata;
	impl->totalsamples = totalsamples;
	impl->fixshift = fixshift;
	return ZL_ImplMakeOwner<ZL_Sound>(impl, false);
}

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Sound)

ZL_Sound::ZL_Sound(const ZL_File& file, bool stream) : impl(ZL_Sound_Load(ZL_ImplFromOwner<ZL_File_Impl>(file), stream)) { }

const ZL_Sound& ZL_Sound::Play(bool looping, bool startPaused) const
{
	if (impl) impl->Play(looping, startPaused);
	return *this;
}

const ZL_Sound& ZL_Sound::Stop() const
{
	if (impl && impl->numactive) impl->Stop();
	return *this;
}

const ZL_Sound& ZL_Sound::Pause() const
{
	if (impl && impl->numactive) impl->Pause();
	return *this;
}

const ZL_Sound& ZL_Sound::Resume() const
{
	if (impl && impl->numactive) impl->Resume();
	return *this;
}

ZL_Sound& ZL_Sound::SetSpeedFactor(scalar factor)
{
	if (impl) impl->audiofactor = MAX(0.f, (float)factor);
	#ifdef __IPHONEOS__
	if (impl && impl->IOS_AudioPlayer) ZL_AudioPlayerRate(impl->IOS_AudioPlayer, (float)factor);
	#endif
	return *this;
}

ZL_Sound& ZL_Sound::SetVolume(scalar vol)
{
	if (impl) impl->audiovol = (float)vol;
	#ifdef __IPHONEOS__
	if (impl && impl->IOS_AudioPlayer) ZL_AudioPlayerVolume(impl->IOS_AudioPlayer, (float)vol);
	#endif
	return *this;
}

scalar ZL_Sound::GetSpeedFactor()
{
	return (impl ? s(impl->audiofactor) : 0);
}

scalar ZL_Sound::GetVolume()
{
	return (impl ? s(impl->audiovol) : 0);
}

ZL_Sound ZL_Sound::Clone() const
{
	return ZL_ImplMakeOwner<ZL_Sound>((impl ? (impl->stream_fh ? ZL_Sound_Load(impl->stream_fh, true) : new ZL_Sound_Impl(impl)) : NULL), false);
}

bool ZL_Sound::IsPlaying() const
{
	return (impl && impl->numactive && !impl->stream_fh);
}

#ifdef __IPHONEOS__
ZL_String ZL_SOUND_FILENAME_TO_NATIVE_FORMAT(const char* string_filename)
{
	ZL_String filename = string_filename;
	size_t len = filename.length();
	if (len > 3 && filename[len-3]=='O' && filename[len-2]=='G' && filename[len-1]=='G') { filename[len-3] = 'M'; filename[len-2] = 'P'; filename[len-1] = '4'; }
	if (len > 3 && filename[len-3]=='O' && filename[len-2]=='g' && filename[len-1]=='g') { filename[len-3] = 'M'; filename[len-2] = 'p'; filename[len-1] = '4'; }
	if (len > 3 && filename[len-3]=='o' && filename[len-2]=='g' && filename[len-1]=='g') { filename[len-3] = 'm'; filename[len-2] = 'p'; filename[len-1] = '4'; }
	return filename;
}
#endif
