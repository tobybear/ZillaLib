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

#ifndef __ZL_SYNTHIMC__
#define __ZL_SYNTHIMC__

#include "ZL_Math.h"
#include "ZL_Audio.h"

#ifdef ZL_USE_SYNTHIMC

struct TImcSongData;

struct ZL_SynthImcTrack
{
	ZL_SynthImcTrack();
	ZL_SynthImcTrack(TImcSongData *songdata, bool repeat = true);
	~ZL_SynthImcTrack();
	ZL_SynthImcTrack(const ZL_SynthImcTrack &source);
	ZL_SynthImcTrack &operator=(const ZL_SynthImcTrack &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_SynthImcTrack &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_SynthImcTrack &b) const { return (impl!=b.impl); }

	ZL_SynthImcTrack& NoteOn(unsigned char channel, unsigned char note);
	ZL_SynthImcTrack& NoteOff(unsigned char channel);
	ZL_SynthImcTrack& StopAllNotes();
	ZL_SynthImcTrack& SetChannelVolume(unsigned char channel, int volume);
	ZL_SynthImcTrack& SetSongRepeatMode(bool repeat); //defaults to true
	ZL_SynthImcTrack& SetSongBPM(scalar bpm);
	ZL_SynthImcTrack& SetSongVolume(int volume);
	ZL_SynthImcTrack& Play(bool restart = false);
	ZL_SynthImcTrack& Pause();
	ZL_SynthImcTrack& Stop();
	bool IsPlaying();

	static ZL_Sound LoadAsSample(TImcSongData *songdata);

	private: struct ZL_SynthImcTrack_Impl* impl;
};

struct TImcSongEnvelope
{
	int minVal;
	int maxVal;
	int speed;
	unsigned char phase;
	unsigned char keep;
	unsigned char sustain;
	bool resetOnNewNote;
	unsigned char resetOnPatternMask;
};

struct TImcSongEnvelopeCounter
{
	signed char envId;
	signed char channel;
	int val;
};

#define IMCSONGOSCTYPE_SINE   ((unsigned char)0)
#define IMCSONGOSCTYPE_SAW    ((unsigned char)1)
#define IMCSONGOSCTYPE_SQUARE ((unsigned char)2)
#define IMCSONGOSCTYPE_NOISE  ((unsigned char)3)

struct TImcSongOscillator
{
	unsigned char transOctave;
	unsigned char transFine;
	unsigned char type;
	unsigned char channel;
	signed char fmTargetOscId;
	unsigned char vol;
	unsigned char volEnvCounterId;
	unsigned char transEnvCounterId;
};

#define IMCSONGEFFECTTYPE_DELAY     ((unsigned char)0)
#define IMCSONGEFFECTTYPE_FLANGE    ((unsigned char)1)
#define IMCSONGEFFECTTYPE_LOWPASS   ((unsigned char)2)
#define IMCSONGEFFECTTYPE_HIGHPASS  ((unsigned char)3)
#define IMCSONGEFFECTTYPE_RESONANCE ((unsigned char)4)
#define IMCSONGEFFECTTYPE_OVERDRIVE ((unsigned char)5)

struct TImcSongEffect
{
	int v1;
	int v2;
	int histSize;
	unsigned char channel;
	unsigned char type;
	unsigned char envCounterId1;
	unsigned char envCounterId2;
};

// holds all song data, construct with:
//
//TImcSongData imcIMCPRFXData = {
//	IMCPRFX_LEN, IMCPRFX_ROWLENSAMPLES, IMCPRFX_ENVLISTSIZE, IMCPRFX_ENVCOUNTERLISTSIZE, IMCPRFX_OSCLISTSIZE, IMCPRFX_EFFECTLISTSIZE, IMCPRFX_VOL,
//	IMCPRFX_OrderTable, IMCPRFX_PatternData, IMCPRFX_PatternLookupTable, IMCPRFX_EnvList, IMCPRFX_EnvCounterList, IMCPRFX_OscillatorList, IMCPRFX_EffectList,
//	IMCPRFX_ChannelVol, IMCPRFX_ChannelEnvCounter, IMCPRFX_ChannelStopNote };
//
// for instrument data only, construct with:
//
//TImcSongData imcIMCPRFXData = { 0, 0, IMCPRFX_ENVLISTSIZE, IMCPRFX_ENVCOUNTERLISTSIZE, IMCPRFX_OSCLISTSIZE, IMCPRFX_EFFECTLISTSIZE, IMCPRFX_VOL,
//	NULL, NULL, NULL, IMCPRFX_EnvList, IMCPRFX_EnvCounterList, IMCPRFX_OscillatorList, IMCPRFX_EffectList, IMCPRFX_ChannelVol, IMCPRFX_ChannelEnvCounter, IMCPRFX_ChannelStopNote };
//
struct TImcSongData
{
	int IMCSONG_LEN;
	int IMCSONG_ROWLENSAMPLES;
	int IMCSONG_ENVLISTSIZE;
	int IMCSONG_ENVCOUNTERLISTSIZE;
	int IMCSONG_OSCLISTSIZE;
	int IMCSONG_EFFECTLISTSIZE;
	int IMCSONG_VOL;

	const unsigned int       *ImcSongOrderTable;
	const unsigned char      *ImcSongPatternData;
	const unsigned char      *ImcSongPatternLookupTable;
	const TImcSongEnvelope   *ImcSongEnvList;
	TImcSongEnvelopeCounter  *ImcSongEnvCounterList;
	const TImcSongOscillator *ImcSongOscillatorList;
	const TImcSongEffect     *ImcSongEffectList;
	unsigned char            *ImcSongChannelVol; //[8];
	const unsigned char      *ImcSongChannelEnvCounter; //[8];
	const bool               *ImcSongChannelStopNote; //[8];
};
#endif

#endif //__ZL_SYNTHIMC__
