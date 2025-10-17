/*
  ZillaLib
  Copyright (C) 2010-2019 Bernhard Schelling

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

//Based on PuavoHard Intro Music Composer 3
//By Juuso Luukkonen and Petri Jarvisalo
//http://www.puavohard.net/php/prods/phpimc

#include "ZL_Platform.h"
#include "ZL_SynthImc.h"
#include "ZL_Audio.h"
#include "ZL_Impl.h"
#include <string.h>
#include <vector>

static ZL_MutexHandle ZL_ImcMutex = ZL_MutexNone;
static std::vector<ZL_SynthImcTrack_Impl*> *ActiveTracks = NULL;
static bool mix_music(short *stream, unsigned int samples, bool mix);

struct ZL_SynthImcTrack_Impl : ZL_Impl
{
	TImcSongData *data;

	int *ImcSongEnvCounterPos;
	int *ImcSongOscPosAdd;
	int *ImcSongOscPos;
	int *ImcSongOscFmPos;
	int *ImcSongEffectData1;
	int *ImcSongEffectData2;
	int **ImcSongEffectHistPtr;
	bool ImcSongChannelNotePlaying[8];
	bool ImcSongChannelNoteOff[8];
	bool ImcSongRepeat, ImcSongPaused;

	int curSampleNum;
	int curSampleNumInRow;
	unsigned char curSampleNumInTick;
	unsigned char curRowInPattern;
	unsigned char curOrderPos;

	int RenderSample();
	void RowHit();
	bool Advance();
	void DoNoteOn(unsigned char channel, unsigned char note);
	void TickHit();

	ZL_SynthImcTrack_Impl(TImcSongData *songdata, bool repeat) : data(NULL)
	{
		ImcSongRepeat = repeat;
		LoadSong(songdata);
		if (!songdata) return;

		if (ZL_MutexIsNone(ZL_ImcMutex)) ZL_MutexInit(ZL_ImcMutex);
		ZL_MutexLock(ZL_ImcMutex);
		ZL_Audio::HookAudioMix(&mix_music);
		if (!ActiveTracks) ActiveTracks = new std::vector<ZL_SynthImcTrack_Impl*>();
		ActiveTracks->push_back(this);
		ZL_MutexUnlock(ZL_ImcMutex);

		/*
		//dump to wav file
		#pragma pack(push,1)
		struct WAV_HEADER
		{
			char RIFF[4]; unsigned int ChunkSize; char WAVE[4], fmt[4]; unsigned int Subchunk1Size;
			unsigned short AudioFormat,NumOfChan; unsigned int SamplesPerSec, bytesPerSec;
			unsigned short blockAlign, bitsPerSample; char Subchunk2ID[4]; unsigned int Subchunk2Size;
		};
		#pragma pack(pop)
		unsigned int out_len = sizeof(short) * (data->IMCSONG_LEN*16*data->IMCSONG_ROWLENSAMPLES);
		unsigned int wav_len = out_len + sizeof(WAV_HEADER);
		void* wav = malloc(wav_len);
		short *out = (short*)(((WAV_HEADER*)wav)+1);
		for (short *ps = out, *psEnd = out+(out_len/sizeof(short)); ps != psEnd; ps++) { *ps = RenderSample(); Advance(); }
		WAV_HEADER* wav_hdr = (WAV_HEADER*)wav;
		memcpy(wav_hdr->Subchunk2ID, "data", 4);
		wav_hdr->Subchunk2Size = out_len * 1 * sizeof(short);
		memcpy(wav_hdr->RIFF, "RIFF", 4);
		wav_hdr->ChunkSize = sizeof(WAV_HEADER) - 4 - 4 + wav_hdr->Subchunk2Size;
		memcpy(wav_hdr->WAVE, "WAVE", 4);
		memcpy(wav_hdr->fmt, "fmt ", 4);
		wav_hdr->Subchunk1Size = 16;
		wav_hdr->AudioFormat = 1;
		wav_hdr->NumOfChan = 1;
		wav_hdr->SamplesPerSec = 44100;
		wav_hdr->bytesPerSec = 44100 * 1 * sizeof(short);
		wav_hdr->blockAlign = 1 * sizeof(short);
		wav_hdr->bitsPerSample = sizeof(short) * 8;
		FILE *f = fopen("imc.wav", "wb");
		fwrite(wav, 1, wav_len, f);
		fclose(f);
		*/
	}

	ZL_SynthImcTrack_Impl(TImcSongData *songdata) : data(NULL)
	{
		LoadSong(songdata);
		if (!songdata) return;
	}

	void LoadSong(TImcSongData *songdata)
	{
		ImcSongPaused = true;
		if (!songdata) return;
		data = songdata;

		ImcSongEnvCounterPos = new int[data->IMCSONG_ENVCOUNTERLISTSIZE];
		ImcSongOscPosAdd     = new int[data->IMCSONG_OSCLISTSIZE];
		ImcSongOscPos        = new int[data->IMCSONG_OSCLISTSIZE];
		ImcSongOscFmPos      = new int[data->IMCSONG_OSCLISTSIZE];
		ImcSongEffectData1   = new int[data->IMCSONG_EFFECTLISTSIZE];
		ImcSongEffectData2   = new int[data->IMCSONG_EFFECTLISTSIZE];
		ImcSongEffectHistPtr = new int*[data->IMCSONG_EFFECTLISTSIZE];
		for(int i=0;i<data->IMCSONG_EFFECTLISTSIZE;i++) ImcSongEffectHistPtr[i] = new int[data->ImcSongEffectList[i].histSize*4];

		SongReset();
	}

	void SongReset()
	{
		memset(ImcSongEnvCounterPos, 0, data->IMCSONG_ENVCOUNTERLISTSIZE*sizeof(int));
		memset(ImcSongOscPosAdd, 0, data->IMCSONG_OSCLISTSIZE*sizeof(int));
		memset(ImcSongOscPos, 0, data->IMCSONG_OSCLISTSIZE*sizeof(int));
		memset(ImcSongOscFmPos, 0, data->IMCSONG_OSCLISTSIZE*sizeof(int));
		memset(ImcSongEffectData1, 0, data->IMCSONG_EFFECTLISTSIZE*sizeof(int));
		memset(ImcSongEffectData2, 0, data->IMCSONG_EFFECTLISTSIZE*sizeof(int));
		for(int i=0;i<data->IMCSONG_EFFECTLISTSIZE;i++) memset(ImcSongEffectHistPtr[i], 0, sizeof(int)*data->ImcSongEffectList[i].histSize*4);
		memset(ImcSongChannelNotePlaying, 0, sizeof(ImcSongChannelNotePlaying));
		memset(ImcSongChannelNoteOff, 0, sizeof(ImcSongChannelNoteOff));

		curSampleNum = 0;
		curSampleNumInRow = 0;
		curSampleNumInTick = 0;
		curRowInPattern = 0;
		curOrderPos = 0;

		if (data->ImcSongOrderTable) RowHit();
	}

	~ZL_SynthImcTrack_Impl()
	{
		if (!data) return;

		ZL_MutexLock(ZL_ImcMutex);
		delete ImcSongEnvCounterPos;
		delete ImcSongOscPosAdd;
		delete ImcSongOscPos;
		delete ImcSongOscFmPos;
		delete ImcSongEffectData1;
		delete ImcSongEffectData2;
		for(int i=0;i<data->IMCSONG_EFFECTLISTSIZE;i++) delete ImcSongEffectHistPtr[i];
		delete ImcSongEffectHistPtr;

		if (ActiveTracks)
			for (std::vector<ZL_SynthImcTrack_Impl*>::iterator it = ActiveTracks->begin(); it != ActiveTracks->end(); ++it)
				if (*it == this) { ActiveTracks->erase(it); break; }
		ZL_MutexUnlock(ZL_ImcMutex);
	}

	ZL_Sound LoadAsSample()
	{
		if (!data) return ZL_Sound();
		size_t out_len = sizeof(short) * (data->IMCSONG_LEN*16*data->IMCSONG_ROWLENSAMPLES);
		short *out = (short*)malloc(out_len), *outlast = NULL;
		for (short *ps = out, *psEnd = out+(out_len/sizeof(short)); ps != psEnd; ps++) { *ps = RenderSample(); if (*ps) outlast = ps; Advance(); }
		extern ZL_Sound ZL_SoundLoadFromBuffer(short* audiodata, unsigned int totalsamples, unsigned int fixshift);
		return ZL_SoundLoadFromBuffer(out, (outlast ? (unsigned int)(1 + outlast - out) : 0), 1 /*stereo to mono*/);
	}
};

#include "phpimc/phpimc.inl"

ZL_Sound ZL_SynthImcTrack::LoadAsSample(TImcSongData *songdata)
{
	return ZL_SynthImcTrack_Impl(songdata).LoadAsSample();
}

static bool mix_music(short *buffer, unsigned int samples, bool mix)
{
	if (ActiveTracks->size() == 0)
	{
		return false;
	}

	ZL_MutexLock(ZL_ImcMutex);
	std::vector<ZL_SynthImcTrack_Impl*>::iterator it = ActiveTracks->begin();
	while (it != ActiveTracks->end() && ((*it)->ImcSongPaused || (!(*it)->ImcSongRepeat && (*it)->curOrderPos >= (*it)->data->IMCSONG_LEN))) ++it;
	ZL_SynthImcTrack_Impl* t; short *ps = NULL, *psEnd = buffer + (samples << 1);
	if (it != ActiveTracks->end())
	{
		if (!mix)
		{
			for (t = *it, ps = buffer; ps != psEnd; ps+=2)
			{
				ps[1] = ps[0] = t->RenderSample();
				t->Advance();
			}
			++it;
		}
		while (it != ActiveTracks->end())
		{
			if ((*it)->ImcSongPaused || (!(*it)->ImcSongRepeat && (*it)->curOrderPos >= (*it)->data->IMCSONG_LEN)) { ++it; continue; }
			for (t = *it, ps = buffer; ps != psEnd; ps+=2)
			{
				int tmp = t->RenderSample(), tmp0 = ps[0] + tmp, tmp1 = ps[1] + tmp;
				ps[0] = (tmp0 > 0x7FFF ? 0x7FFF : (tmp0 < -0x8000 ? -0x8000 : (short)tmp0));
				ps[1] = (tmp1 > 0x7FFF ? 0x7FFF : (tmp1 < -0x8000 ? -0x8000 : (short)tmp1));
				t->Advance();
			}
			++it;
		}
	}
	ZL_MutexUnlock(ZL_ImcMutex);

	return (ps != NULL);
}

ZL_SynthImcTrack::ZL_SynthImcTrack(TImcSongData *songdata, bool repeat) : impl(new ZL_SynthImcTrack_Impl(songdata, repeat))
{ if (!impl->data) { delete impl; impl = NULL; } }

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_SynthImcTrack)

ZL_SynthImcTrack& ZL_SynthImcTrack::SetSongRepeatMode(bool repeat)
{
	if (impl) impl->ImcSongRepeat = repeat;
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::SetSongBPM(scalar bpm)
{
	if (bpm < 1) bpm = 1;
	if (impl) impl->data->IMCSONG_ROWLENSAMPLES = (int)(s(661500) / bpm);
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::SetSongVolume(int volume)
{
	if (impl) impl->data->IMCSONG_VOL = volume;
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::NoteOff(unsigned char channel)
{
	if (impl) impl->DoNoteOn(channel, 255);
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::NoteOn(unsigned char channel, unsigned char note)
{
	if (!impl || note < 12) return *this;
	note -= 12;
	impl->DoNoteOn(channel, note+(note/12)*4);
	impl->ImcSongPaused = false;
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::SetChannelVolume( unsigned char channel, int volume )
{
	impl->data->ImcSongChannelVol[channel] = volume;
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::Play(bool restart)
{
	if (impl && ((restart && (!impl->ImcSongPaused || impl->curOrderPos > 0)) || (!impl->ImcSongRepeat && impl->curOrderPos >= impl->data->IMCSONG_LEN))) impl->SongReset();
	if (impl) impl->ImcSongPaused = false;
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::Pause()
{
	if (impl) impl->ImcSongPaused = true;
	return *this;
}

ZL_SynthImcTrack& ZL_SynthImcTrack::Stop()
{
	if (impl) { impl->ImcSongPaused = true; impl->SongReset(); }
	return *this;
}

bool ZL_SynthImcTrack::IsPlaying()
{
	return (impl && !impl->ImcSongPaused && impl->curOrderPos < impl->data->IMCSONG_LEN);
}


/*
static void RenderToBuffer(short** psBuffer, unsigned int* piPCMLength)
{
	*piPCMLength = (data->IMCSONG_LEN*16*data->IMCSONG_ROWLENSAMPLES);
	*psBuffer = new short[*piPCMLength];
	for (short *ps = *psBuffer, *psEnd = (*psBuffer)+*piPCMLength; ps != psEnd; ps++)
	{
		*ps = RenderSample();
		Advance();
	}
}
*/
