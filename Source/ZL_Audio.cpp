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
#define OGG_READ(H, buffer, byte_len) stb_vorbis_get_samples_short_interleaved((stb_vorbis*)(H), ((stb_vorbis_info*)(H))->channels, (short*)(buffer), (int)(byte_len)>>1)<<1*((stb_vorbis_info*)(H))->channels
#define OGG_SEEKRESET(H) stb_vorbis_seek((stb_vorbis*)(H),0) //stb_vorbis_seek_start((stb_vorbis*)(H));
#elif defined(_OV_FILE_H_)
#define OGG_READ(H, buffer, byte_len) ov_read((OggVorbis_File*)(H), (char*)(buffer), (int)(byte_len), NULL)
#define OGG_SEEKRESET(H) ov_raw_seek((OggVorbis_File*)(H), 0);
static size_t ogg_read_func(void *ptr, size_t size, size_t num, void *src) { return ZL_RWread((ZL_RWops*)src, ptr, size, num); }
static int ogg_seek_func(void *src, ogg_int64_t offset, int mode) { return ZL_RWseek((ZL_RWops*)src, (int)offset, mode); }
static long ogg_tell_func(void *src) { return (long)ZL_RWtell((ZL_RWops*)src); }
#endif

struct ZL_AudioPlayingHandle
{
	struct ZL_Sound_Impl *snd;
	size_t pos;
	bool loop, paused;
	bool mix_into(char* buf, int rem, bool add);
};

static std::vector<ZL_AudioPlayingHandle> *ZL_AudioActive = NULL;
static ZL_MutexHandle ZL_AudioActiveMutex;
static float audio_global_factor = 1.0f;
bool (*funcAudioMix)(char*, int) = 0;

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

void ZL_Audio::HookMusicMix(bool (*pFuncAudioMix)(char*, int))
{
	funcAudioMix = pFuncAudioMix;
}

bool ZL_PlatformAudioMix(char *stream, int len)
{
	bool didFuncAudioMix;
	if (ZL_WINDOWFLAGS_HAS(ZL_WINDOW_MINIMIZED) || !((didFuncAudioMix = (funcAudioMix && funcAudioMix(stream, len))) || ZL_AudioActive->size()))
	{
		nothingtomix:
		memset(stream, 0, len);
		return false;
	}
	if (didFuncAudioMix && !ZL_AudioActive->size()) return true;

	bool haveNothingToMix = false;
	ZL_MutexLock(ZL_AudioActiveMutex);
	std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin();
	while (it != ZL_AudioActive->end() && it->paused) ++it;
	if (it == ZL_AudioActive->end()) haveNothingToMix = true;
	else
	{
		if (!it->mix_into(stream, len, didFuncAudioMix)) ++it;
		else it = ZL_AudioActive->erase(it);

		while (it != ZL_AudioActive->end())
		{
			if (it->paused || !it->mix_into(stream, len, true)) ++it;
			else it = ZL_AudioActive->erase(it);
		}
	}
	ZL_MutexUnlock(ZL_AudioActiveMutex);
	if (haveNothingToMix) goto nothingtomix;
	return true;
}

struct ZL_Sound_Impl : ZL_Impl
{
	ZL_File_Impl* stream_fh;
	void *decoder; //audio decoder handle
	char* audiodata;
	size_t audiolen;
	int audiofixshift;
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

	ZL_Sound_Impl() : stream_fh(NULL), decoder(NULL), audiodata(NULL), audiolen(0), audiofixshift(0), audiofactor(1), audiovol(1) ZL_SOUND_IMPL_PLATFORM_INIT {}

	ZL_Sound_Impl(ZL_Sound_Impl* b) : stream_fh(NULL), decoder(NULL), audiodata(NULL), audiolen(b->audiolen), audiofixshift(b->audiofixshift), audiofactor(b->audiofactor), audiovol(b->audiovol) ZL_SOUND_IMPL_PLATFORM_INIT
		{ if (audiolen) { audiodata = (char*)malloc(b->audiolen); memcpy(audiodata, b->audiodata, audiolen); } }

	~ZL_Sound_Impl()
	{
		if (ZL_AudioActive) Stop();
		#if defined(__IPHONEOS__)
		if (IOS_AudioPlayer) { ZL_AudioPlayerRelease(IOS_AudioPlayer); IOS_AudioPlayer = NULL; }
		#elif defined(__ANDROID__)
		if (Android_AudioPlayer) { ZL_AudioAndroidRelease(Android_AudioPlayer); Android_AudioPlayer = NULL; return; }
		#endif
		if (audiodata) free(audiodata);
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
		if (decoder) Stop(); //stop streamed file before playing it again
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
		bool was_playing = false;
		ZL_MutexLock(ZL_AudioActiveMutex);
		for (std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin(); it != ZL_AudioActive->end(); )
			if (it->snd == this) { it = ZL_AudioActive->erase(it); was_playing = true; }
			else ++it;
		ZL_MutexUnlock(ZL_AudioActiveMutex);
		if (was_playing && decoder) OGG_SEEKRESET(decoder);
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

bool ZL_AudioPlayingHandle::mix_into(char* buf, int rem, bool add)
{
	char tmpbuf[512];
	float factor = audio_global_factor * snd->audiofactor;
	float vol = snd->audiovol;
	int fixshift = (factor == 1.0f ? snd->audiofixshift : 0), read, write;
	if (snd->audiofixshift) factor /= s(1<<snd->audiofixshift);
	if (factor < 1.0f && factor > 0.98f) factor = 1.0f; //round factor (avoid issues with direct_write)
	bool rescale = (factor != 1.0f);
	bool allow_direct_write = (!add && factor <= 1.0f && vol == 1.0f); //no need for tmpbuf
	short stmp, *ssrc = (short*)buf;
	do //until no bytes are remaining (or the audio stream is finished without loop)
	{
		if (rescale)
		{
			read = (int)(rem*factor);
			if (!read || read & 3) read += 4-(read & 3); //align to shorts per 2 channels
		}
		else read = rem;

		if (snd->decoder)
		{
			if (!allow_direct_write && read > (int)sizeof(tmpbuf)) read = (int)sizeof(tmpbuf);
			ssrc = (short*)(allow_direct_write ? buf : tmpbuf);
			int got = OGG_READ(snd->decoder, ssrc, read);
			if (got < 0) break;
			if (got != read) { read = got; pos = snd->audiolen; } //no more samples available, loop
		}
		else
		{
			if (read > (int)(snd->audiolen - pos)) read = (int)(snd->audiolen - pos);
			ssrc = (short*)(snd->audiodata+pos);
		}

		if (rescale)
		{
			write = (int)(read/factor);
			if (!write || write & 3) write += 4-(write & 3); //align to shorts per 2 channels
			if (write > rem) write = rem;
		}
		else write = read;

		short *s = (short*)buf, *send = (short*)(buf + write);
		if (allow_direct_write)
		{
			if (!snd->decoder && !rescale) memcpy(buf, ssrc, write);
			else if (rescale)
			{
				if (fixshift) for (unsigned int count = write / sizeof(short); count--;) s[count] = ssrc[count>>fixshift];
				//else          while (total--) s[total] = ssrc[(int)(total*factor)];
				else for (float f = 0; send != s; send-=2, f+=factor)
				{
					int off = (read>>1)-(((int)f)<<1);
					send[-2] = ssrc[off-2];
					send[-1] = ssrc[off-1];
				}
			}
		}
		else if (vol != 1.0f)
		{
			if (add) for (float f = 0; s != send; s+=2, f+=factor)
			{
				int off = ((int)f)<<1;
				if ((stmp = ssrc[  off])) { int tmp = s[0] + (int)(stmp * vol); s[0] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp)); }
				if ((stmp = ssrc[1+off])) { int tmp = s[1] + (int)(stmp * vol); s[1] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp)); }
			}
			else for (float f = 0; s != send; s+=2, f+=factor)
			{
				int off = ((int)f)<<1, tmp;
				tmp = (int)(ssrc[  off] * vol); s[0] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp));
				tmp = (int)(ssrc[1+off] * vol); s[1] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp));
			}
		}
		else
		{
			if (!rescale) for (; s != send; s++, ssrc++)
			{
				if (*ssrc == 0) continue;
				int tmp = *s + *ssrc;
				*s = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp));
			}
			else if (fixshift) for (unsigned int count = 0; s != send; s++, count++)
			{
				if (!(stmp = ssrc[count>>fixshift])) continue;
				int tmp = *s + stmp;
				*s = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp));
			}
			else if (add) for (float f = 0; s != send; s+=2, f+=factor)
			{
				int off = ((int)f)<<1;
				if ((stmp = ssrc[  off])) { int tmp = s[0] + stmp; s[0] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp)); }
				if ((stmp = ssrc[1+off])) { int tmp = s[1] + stmp; s[1] = (tmp > 0x7FFF ? 0x7FFF : (tmp < -0x7FFF ? -0x7FFF : (short)tmp)); }
			}
			else for (float f = 0; s != send; s+=2, f+=factor)
			{
				int off = ((int)f)<<1;
				s[0] = ssrc[  off];
				s[1] = ssrc[1+off];
			}
		}
		rem -= write;
		buf += write;
		pos += read;
		if (pos >= snd->audiolen)
		{
			pos = 0;
			if (snd->decoder) OGG_SEEKRESET(snd->decoder);
			if (!loop && !rem) return true;
		}
		if (!loop && !pos && write) break;
	} while (rem);
	if (rem && !add) memset(buf, 0, rem);
	return (rem > 0); //rem > 0 means the audio stopped during play and is not looped and needs to be removed from the list of playing samples
}

ZL_Sound_Impl* ZL_Sound_Load(ZL_File_Impl* file_impl, bool stream)
{
	if (!file_impl) return NULL;

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
	ah->audiolen = ((size_t)stb_vorbis_stream_length_in_samples(v)) * vi->channels * sizeof(short);
	for (unsigned int r = (44100*2)/(vi->sample_rate*vi->channels); r >>= 1;) ah->audiofixshift++;

	if (stream)
	{
		ah->decoder = v;
		ah->stream_fh = file_impl;
		file_impl->AddRef();
	}
	else
	{
		ah->audiodata = (char*)malloc(ah->audiolen);
		if (ah->audiodata == NULL) { stb_vorbis_close(v); delete ah; return NULL; }
		stb_vorbis_get_samples_short_interleaved(v, vi->channels, (short*)ah->audiodata, (int)(ah->audiolen / sizeof(short)));
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
	ah->audiolen = ((size_t)ov_pcm_total(&ovf, -1)) * info->channels * sizeof(short);
	for (unsigned int r = (44100*2)/(info->rate*info->channels); r >>= 1;) ah->audiofixshift++;

	if (stream)
	{
		ah->decoder = (void*) new OggVorbis_File();
		memcpy(ah->decoder, &ovf, sizeof(OggVorbis_File));
		ah->stream_fh = file_impl;
		file_impl->AddRef();
	}
	else
	{
		ah->audiodata = (char*)malloc(ah->audiolen);

		char* buffer = ah->audiodata;
		long remaining = (long)ah->audiolen, read;
		while (remaining && (read = ov_read(&ovf, buffer, remaining, NULL)))
		{
			if (read == OV_HOLE || read == OV_EBADLINK) break; // data error
			buffer += read;
			remaining -= read;
		}

		ov_clear(&ovf);
	}
	return ah;
	#endif
}

ZL_Sound ZL_SoundLoadFromBuffer(char* audiodata, size_t audiolen, int audiofixshift)
{
	ZL_Sound_Impl* impl = new ZL_Sound_Impl();
	impl->audiodata = audiodata;
	impl->audiolen = audiolen;
	impl->audiofixshift = audiofixshift;
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
	if (impl) impl->Stop();
	return *this;
}

const ZL_Sound& ZL_Sound::Pause() const
{
	if (impl) impl->Pause();
	return *this;
}

const ZL_Sound& ZL_Sound::Resume() const
{
	if (impl) impl->Resume();
	return *this;
}

ZL_Sound& ZL_Sound::SetSpeedFactor(scalar factor)
{
	if (impl) impl->audiofactor = MAX(0.001f, (float)factor);
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
	ZL_Sound ret;
	if (impl) ret.impl = (impl->stream_fh ? ZL_Sound_Load(impl->stream_fh, true) : new ZL_Sound_Impl(impl));
	return ret;
}

bool ZL_Sound::IsPlaying() const
{
	if (!impl || impl->stream_fh) return false;
	for (std::vector<ZL_AudioPlayingHandle>::iterator it = ZL_AudioActive->begin(); it != ZL_AudioActive->end(); ++it)
		if (it->snd == impl) return !it->paused;
	return false;
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
