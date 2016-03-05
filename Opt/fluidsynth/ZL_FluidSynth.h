#ifndef _ZL_FLUIDSYNTH_
#define _ZL_FLUIDSYNTH_

#include <ZL_File.h>

class ZL_FluidSynth
{
public:
	static void InitSynth(const ZL_File& SoundFontFile);
	static void SetSynthGain(float gain);
	static void NoteOn(unsigned char chan, int key, int vel);
	static void NoteOff(unsigned char chan, int key);
	static void StopAllNotes();
	static void SynthEvent(unsigned char chan, unsigned char type, unsigned int param1, unsigned int param2 = 0);
};

#endif //_ZL_FLUIDSYNTH_
