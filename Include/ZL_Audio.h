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

#ifndef __ZL_Audio__
#define __ZL_Audio__

#include "ZL_Application.h"
#include "ZL_File.h"
#include "ZL_Math.h"

//Global audio system functions (needs at least a call to ZL_Audio::Init to use any other audio system)
struct ZL_Audio
{
	static bool Init();
	static void SetGlobalSpeedFactor(scalar factor);
	static void HookMusicMix(bool (*pFuncAudioMix)(char*, int)); //For custom music mixing, sound generation, etc.
};

#ifdef __IPHONEOS__
//On iOS this function returns a filename string with mp4 instead of ogg extension
//Using mp4 files is optional but can give a performance boost for streaming music on older devices
ZL_String ZL_SOUND_FILENAME_TO_NATIVE_FORMAT(const char* string_filename);
#else
#define ZL_SOUND_FILENAME_TO_NATIVE_FORMAT(string_filename) (string_filename)
#endif

//A playable sound waveform, can be fully loaded into memory on construction or streamed from the source file
struct ZL_Sound
{
	ZL_Sound();
	ZL_Sound(const ZL_File& file, bool stream = false);
	~ZL_Sound();
	ZL_Sound(const ZL_Sound &source);
	ZL_Sound &operator =(const ZL_Sound &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Sound &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Sound &b) const { return (impl!=b.impl); }

	//Function for controlling playback
	const ZL_Sound& Play(bool looping = false, bool startPaused = false) const;
	const ZL_Sound& Stop() const;
	const ZL_Sound& Pause() const;
	const ZL_Sound& Resume() const;

	//Get and set settings of this sound
	scalar GetSpeedFactor();
	scalar GetVolume();
	ZL_Sound& SetSpeedFactor(scalar factor);
	ZL_Sound& SetVolume(scalar vol);

	ZL_Sound Clone() const; //get a copy of this sound to have separate above settings
	bool IsPlaying() const; //only works for non-streams

	private: struct ZL_Sound_Impl* impl;
};

#endif //__ZL_Audio__
