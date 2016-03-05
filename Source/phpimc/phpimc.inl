/*
  PHPIMC3 - PuavoHard Intro Music Composer 3
  Copyright (c) 2008 Juuso Luukkonen

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
  The API has been altered for usage in ZillaLib
*/

static unsigned short ImcSongNoteFreqTable[] = {
	//C      C#     D      D#     E      F      F#     G      G#     A      A#     B
	12441, 13181, 13965, 14795, 15675, 16607, 17594, 18641, 19749, 20923, 22168, 23486
};

static int ImcRand(int & seed) { return (seed = seed * 214013 + 2531011)>>8; }
static int ImcIntSin(int x) { x = (x&255)-128; return (x*(x>0?x:-x)>>5) - (x<<2); }
static int RenderOsc_Sine(unsigned int pos) { return ImcIntSin(pos>>8)*256; }
static int RenderOsc_Saw(unsigned int pos) { return pos-32768; }
static int RenderOsc_Square(unsigned int pos) { return (pos>32767)?32767:-32768; }
static int RenderOsc_Noise(unsigned int /*pos*/) { static int rndSeed; return (ImcRand(rndSeed)&65535)-32768; }
typedef int (*TRenderOscFn)(unsigned int);
static const TRenderOscFn OscRenderers[] = { RenderOsc_Sine, RenderOsc_Saw, RenderOsc_Square, RenderOsc_Noise };

int ZL_SynthImcTrack_Impl::RenderSample()
{
	if (!data) return 0;

	int channelSample[8] = {0, };
	//static int rndSeed;

	for(int i=0;i<data->IMCSONG_OSCLISTSIZE;i++) {
		const TImcSongOscillator * osc = &data->ImcSongOscillatorList[i];
		int oscVal = OscRenderers[osc->type](ImcSongOscPos[i]&65535);
		int vol = (osc->vol * data->ImcSongEnvCounterList[osc->volEnvCounterId].val) >> 8;
		oscVal = (oscVal * vol) >> 8;
		if(osc->fmTargetOscId!=-1) {
			ImcSongOscFmPos[(unsigned char)osc->fmTargetOscId] = oscVal;
		} else {
			if(ImcSongChannelNotePlaying[osc->channel])
				channelSample[osc->channel] += oscVal;
		}
	}

	int s = 0;
	for(int channel=0;channel<8;channel++) {

		int channelVol = (data->ImcSongChannelVol[channel] * data->ImcSongEnvCounterList[data->ImcSongChannelEnvCounter[channel]].val) >> 8;

		int inSample = (channelSample[channel] * channelVol) >> 8;
		int outSample = inSample;

		for(int i=0;i<data->IMCSONG_EFFECTLISTSIZE;i++) {
			if(data->ImcSongEffectList[i].channel != channel)
				continue;

			int varSample1 = ImcSongEffectData1[i];
			int varSample2 = ImcSongEffectData2[i];
			int fxSample1 = data->ImcSongEffectList[i].v1;
			int fxSample2 = data->ImcSongEffectList[i].v2;
			int fxEnvSample1 = (fxSample1 * data->ImcSongEnvCounterList[data->ImcSongEffectList[i].envCounterId1].val) >> 8;
			int fxEnvSample2 = (fxSample2 * data->ImcSongEnvCounterList[data->ImcSongEffectList[i].envCounterId2].val) >> 8;

			int * histPtr = ImcSongEffectHistPtr[i];
			int histSize = data->ImcSongEffectList[i].histSize;
			int histPos = curSampleNum % histSize;
			int histSample = histPtr[histPos];

			switch(data->ImcSongEffectList[i].type) {

				case IMCSONGEFFECTTYPE_DELAY:
					outSample =
						inSample +
						((histSample * fxSample1) >> 8);
					histSample = outSample;
					break;

				case IMCSONGEFFECTTYPE_FLANGE:
					{
						int histReadPos =
							(
								histPos -
								data->ImcSongEnvCounterList[data->ImcSongEffectList[i].envCounterId1].val +
								histSize
							) % histSize;
						outSample =
							inSample +
							histPtr[histReadPos];
						histSample = inSample;
					}
					break;

				case IMCSONGEFFECTTYPE_LOWPASS:
					outSample = (inSample*fxEnvSample1)/256 + (histSample*(255-fxEnvSample1))/256;
					histSample = outSample;
					break;

				case IMCSONGEFFECTTYPE_HIGHPASS:
					{
						int invFxEnvSample1 = 255 - fxEnvSample1;
						outSample =
							(
								inSample*((256+invFxEnvSample1)/2) +
								varSample1*(-(256+invFxEnvSample1)/2) +
								varSample2*invFxEnvSample1
							) >> 8;
						varSample1 = inSample;
						varSample2 = outSample;
					}
					break;

				case IMCSONGEFFECTTYPE_RESONANCE:
					{
						int ifc = fxEnvSample1;
						int ifr = 255 - fxEnvSample2;
						int iff = 255 - (ifc*ifr)/256;
						varSample1 = (iff*varSample1 - ifc*varSample2 + ifc*inSample) >> 8;
						varSample2 = (iff*varSample2 + ifc*varSample1) >> 8;
						outSample = varSample2;
					}
					break;

				case IMCSONGEFFECTTYPE_OVERDRIVE:
					if(inSample>fxSample1)
						inSample = fxSample1;
					if(inSample<-fxSample1)
						inSample = -fxSample1;
					outSample = (inSample*fxEnvSample2) >> 8;
					break;
			}

			ImcSongEffectData1[i] = varSample1;
			ImcSongEffectData2[i] = varSample2;
			histPtr[histPos] = histSample;
			inSample = outSample;

		}

		s += (outSample * data->IMCSONG_VOL) >> 8;

	}

	if(s<-32768) s=-32768;
	if(s>32767) s=32767;
	return s;
}

void ZL_SynthImcTrack_Impl::DoNoteOn(unsigned char channel, unsigned char note)
{
	if(note==0)
		return;

	if(note==255) {
		ImcSongChannelNoteOff[channel] = true;
		if (!data) return;
		if(data->ImcSongChannelStopNote[channel])
			ImcSongChannelNotePlaying[channel] = false;
		return;
	}

	ImcSongChannelNoteOff[channel] = false;
	ImcSongChannelNotePlaying[channel] = true;
	if (!data) return;
	int i;
	for(i=0;i<data->IMCSONG_ENVCOUNTERLISTSIZE;i++) {
		if(data->ImcSongEnvCounterList[i].channel != channel)
			continue;
		int envId = data->ImcSongEnvCounterList[i].envId;
		if(envId==-1)
			continue;
		if(data->ImcSongEnvList[envId].resetOnNewNote)
			ImcSongEnvCounterPos[i] = 0;
	}

	for(i=0;i<data->IMCSONG_OSCLISTSIZE;i++) {
		if(data->ImcSongOscillatorList[i].channel != channel)
			continue;
		ImcSongOscPos[i] = 0;
		ImcSongOscFmPos[i] = 0;
		int octave = note>>4;
		int noteIndex = note&15;
		int posAdd = ((unsigned int)ImcSongNoteFreqTable[noteIndex] * (1<<octave)) / 512;
		posAdd = (posAdd << data->ImcSongOscillatorList[i].transOctave) >> 8;
		posAdd = (posAdd * (data->ImcSongOscillatorList[i].transFine + 256)) >> 8;
		ImcSongOscPosAdd[i] = posAdd;
	}
}

void ZL_SynthImcTrack_Impl::RowHit() {
	if(curOrderPos>=data->IMCSONG_LEN)
	{
		if (ImcSongRepeat) curOrderPos = 0;
		else return;
	}

	for(int channel=0;channel<8;channel++) {
		int pattern = (data->ImcSongOrderTable[curOrderPos] >> (channel*4)) & 15;
		if(pattern==0)
			continue;

		int patternIndex = ((int)data->ImcSongPatternLookupTable[channel] + pattern-1) * 16;
		const unsigned char * pptr = data->ImcSongPatternData+patternIndex;
		DoNoteOn(channel, pptr[curRowInPattern]);
	}

}

void ZL_SynthImcTrack_Impl::TickHit() {
	for(int i=0;i<data->IMCSONG_ENVCOUNTERLISTSIZE;i++) {
		int channel = data->ImcSongEnvCounterList[i].channel;
		int envId = data->ImcSongEnvCounterList[i].envId;
		if(envId==-1)
			continue;
		int pos = ImcSongEnvCounterPos[i] + data->ImcSongEnvList[envId].speed;

		if(
			!ImcSongChannelNoteOff[channel] &&
			data->ImcSongEnvList[envId].sustain<255 &&
			pos > data->ImcSongEnvList[envId].sustain*2048
		)
			pos = data->ImcSongEnvList[envId].sustain*2048;

		if(data->ImcSongEnvList[envId].keep<255 && pos > data->ImcSongEnvList[envId].keep*2048)
			pos = data->ImcSongEnvList[envId].keep*2048;

		data->ImcSongEnvCounterList[i].val =
			(ImcIntSin((pos+data->ImcSongEnvList[envId].phase*2048)>>8) + 128) *
			(data->ImcSongEnvList[envId].maxVal-data->ImcSongEnvList[envId].minVal) / 256 +
			data->ImcSongEnvList[envId].minVal;

		ImcSongEnvCounterPos[i] = pos;
	}
}

bool ZL_SynthImcTrack_Impl::Advance()
{
	if (!data) return true;
	//if (curOrderPos>=data->IMCSONG_LEN) return false;

	curSampleNum++;
	curSampleNumInRow++;
	if(data->ImcSongOrderTable && curSampleNumInRow>=data->IMCSONG_ROWLENSAMPLES) {
		curSampleNumInRow=0;
		curRowInPattern++;
		if(curRowInPattern>15) {
			curRowInPattern=0;
			curOrderPos++;
			//-- Pattern hit
			for(int i=0;i<data->IMCSONG_ENVCOUNTERLISTSIZE;i++) {
				int envId = data->ImcSongEnvCounterList[i].envId;
				if(envId==-1)
					continue;
				if((curOrderPos&data->ImcSongEnvList[envId].resetOnPatternMask)==0)
					ImcSongEnvCounterPos[i] = 0;
			}
			//--
		}
		RowHit();
	}
	curSampleNumInTick++;
	if(curSampleNumInTick>=44) {
		curSampleNumInTick=0;
		TickHit();
	}
	for(int i=0;i<data->IMCSONG_OSCLISTSIZE;i++) {
		int posAdd =
			((ImcSongOscPosAdd[i] *
			data->ImcSongEnvCounterList[data->ImcSongOscillatorList[i].transEnvCounterId].val) >> 8) +
			(ImcSongOscFmPos[i] >> 3);
		ImcSongOscPos[i] += posAdd;
	}
	return true;
}
