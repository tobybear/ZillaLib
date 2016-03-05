//Midi parser for ZillaLib based on FluidSynth (fluid_midi.c)

//------------------------------------------------------------------------------
/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
//------------------------------------------------------------------------------

#ifndef __ZL_MIDI__
#define __ZL_MIDI__

#include <ZL_String.h>
#include <ZL_File.h>
#include <map>

enum ZL_Midi_Event_Type { NOTE_OFF = 0x80, NOTE_ON = 0x90, KEY_PRESSURE = 0xa0, CONTROL_CHANGE = 0xb0, PROGRAM_CHANGE = 0xc0, CHANNEL_PRESSURE = 0xd0, PITCH_BEND = 0xe0, MIDI_SYSEX = 0xf0, MIDI_TIME_CODE = 0xf1, MIDI_SONG_POSITION = 0xf2, MIDI_SONG_SELECT = 0xf3, MIDI_TUNE_REQUEST = 0xf6, MIDI_EOX = 0xf7, MIDI_SYNC = 0xf8, MIDI_TICK = 0xf9, MIDI_START = 0xfa, MIDI_CONTINUE = 0xfb, MIDI_STOP = 0xfc, MIDI_ACTIVE_SENSING = 0xfe, MIDI_SYSTEM_RESET = 0xff, MIDI_META_EVENT = 0xff};
enum ZL_Midi_Control_Change { BANK_SELECT_MSB = 0x00, MODULATION_MSB = 0x01, BREATH_MSB = 0x02, FOOT_MSB = 0x04, PORTAMENTO_TIME_MSB = 0x05, DATA_ENTRY_MSB = 0x06, VOLUME_MSB = 0x07, BALANCE_MSB = 0x08, PAN_MSB = 0x0A, EXPRESSION_MSB = 0x0B, EFFECTS1_MSB = 0x0C, EFFECTS2_MSB = 0x0D, GPC1_MSB = 0x10, GPC2_MSB = 0x11, GPC3_MSB = 0x12, GPC4_MSB = 0x13, BANK_SELECT_LSB = 0x20, MODULATION_WHEEL_LSB = 0x21, BREATH_LSB = 0x22, FOOT_LSB = 0x24, PORTAMENTO_TIME_LSB = 0x25, DATA_ENTRY_LSB = 0x26, VOLUME_LSB = 0x27, BALANCE_LSB = 0x28, PAN_LSB = 0x2A, EXPRESSION_LSB = 0x2B, EFFECTS1_LSB = 0x2C, EFFECTS2_LSB = 0x2D, GPC1_LSB = 0x30, GPC2_LSB = 0x31, GPC3_LSB = 0x32, GPC4_LSB = 0x33, SUSTAIN_SWITCH = 0x40, PORTAMENTO_SWITCH = 0x41, SOSTENUTO_SWITCH = 0x42, SOFT_PEDAL_SWITCH = 0x43, LEGATO_SWITCH = 0x45, HOLD2_SWITCH = 0x45, SOUND_CTRL1 = 0x46, SOUND_CTRL2 = 0x47, SOUND_CTRL3 = 0x48, SOUND_CTRL4 = 0x49, SOUND_CTRL5 = 0x4A, SOUND_CTRL6 = 0x4B, SOUND_CTRL7 = 0x4C, SOUND_CTRL8 = 0x4D, SOUND_CTRL9 = 0x4E, SOUND_CTRL10 = 0x4F, GPC5 = 0x50, GPC6 = 0x51, GPC7 = 0x52, GPC8 = 0x53, PORTAMENTO_CTRL = 0x54, EFFECTS_DEPTH1 = 0x5B, EFFECTS_DEPTH2 = 0x5C, EFFECTS_DEPTH3 = 0x5D, EFFECTS_DEPTH4 = 0x5E, EFFECTS_DEPTH5 = 0x5F, DATA_ENTRY_INCR = 0x60, DATA_ENTRY_DECR = 0x61, NRPN_LSB = 0x62, NRPN_MSB = 0x63, RPN_LSB = 0x64, RPN_MSB = 0x65, ALL_SOUND_OFF = 0x78, ALL_CTRL_OFF = 0x79, LOCAL_CONTROL = 0x7A, ALL_NOTES_OFF = 0x7B, OMNI_OFF = 0x7C, OMNI_ON = 0x7D, POLY_OFF = 0x7E, POLY_ON = 0x7F};
enum ZL_Midi_Meta_Event { MIDI_TEXT = 0x01, MIDI_COPYRIGHT = 0x02, MIDI_TRACK_NAME = 0x03, MIDI_INST_NAME = 0x04, MIDI_LYRIC = 0x05, MIDI_MARKER = 0x06, MIDI_CUE_POINT = 0x07, MIDI_EOT = 0x2f, MIDI_SET_TEMPO = 0x51, MIDI_SMPTE_OFFSET = 0x54, MIDI_TIME_SIGNATURE = 0x58, MIDI_KEY_SIGNATURE = 0x59, MIDI_SEQUENCER_EVENT = 0x7f};

#pragma pack(push, r, 1)
#pragma pack(1)
struct ZL_MidiEvent
{
	ZL_MidiEvent() : next(NULL), time(0), type(0), channel(0), param1(0), param2(0) { }
	ZL_MidiEvent* next;
	unsigned int time;
	unsigned char type, channel;
	union { struct { unsigned int param1, param2; }; void* ptr; };
};
#pragma pack(pop, r)

struct ZL_Midi
{
public:
	ZL_Midi();
	ZL_Midi(const ZL_File& File, std::map<int, ZL_String> *pLyrics = NULL);
	ZL_Midi(void* data, size_t size);
	~ZL_Midi();
	ZL_Midi(const ZL_Midi &source);
	ZL_Midi &operator=(const ZL_Midi &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Midi &b) const;
	bool operator!=(const ZL_Midi &b) const;

	ZL_MidiEvent *GetFirstEvent();

	private: struct ZL_Midi_Impl* impl;
};

#endif //__ZL_MIDI__

#ifdef ZL_OPT_DO_IMPLEMENTATION

#include "../Source/ZL_File_Impl.h"
#include "../Source/zlib/zlib.h"
#include <ZL_Math.h>

struct ZL_MidiTrack
{
	ZL_MidiTrack() : first(NULL), cur(NULL), last(NULL), ticks(0) { }
	void reset()
	{
		ticks = 0;
		cur = first;
	}
	void add_event(ZL_MidiEvent* evt)
	{
		evt->next = NULL;
		if (first == NULL) { first = cur = last = evt; return; }
		last->next = evt;
		last = evt;
	}
	ZL_String name;
	ZL_MidiEvent *first;
	ZL_MidiEvent *cur;
	ZL_MidiEvent *last;
	unsigned int ticks;
};

struct ZL_Midi_Impl : ZL_Impl
{
	int trackpos;
	int eot;
	int running_status;
	int iInvalidGetC;
	bool bIgnoreLyrics;
	ZL_RWops* src;
	ZL_MidiEvent *pFirstEvent;

	ZL_Midi_Impl()
	{
	}

	~ZL_Midi_Impl()
	{
		if (!pFirstEvent) return;
		while (pFirstEvent)
		{
			ZL_MidiEvent* pPrevEvent = pFirstEvent;
			pFirstEvent = pPrevEvent->next;
			delete pPrevEvent;
		}

	}

	ZL_Midi_Impl(const ZL_File& File, std::map<int, ZL_String> *pLyrics) : iInvalidGetC(-1), pFirstEvent(NULL)
	{
		char mthd[15];

		unsigned int division; //If uses_SMPTE == 0 then division is ticks per beat (quarter-note)

		bIgnoreLyrics = (!pLyrics);
		src = (*(ZL_File_Impl**)&File)->src;
		if (src && ZL_RWtell(src)) ZL_RWseektell(src, 0, RW_SEEK_SET);
		if (!src || !ZL_RWread(src, mthd, 14, 1)) return;
		if ((strncmp(mthd, "MThd", 4) != 0) || (mthd[7] != 6) || (mthd[9] > 2))
		{
			//ERROR: Doesn't look like a MIDI file: invalid MThd header
			return;
		}

		//int type = mthd[9];
		unsigned int ntracks = (unsigned char) mthd[11];
		ntracks += (unsigned int) (mthd[10]) << 16;

		if((mthd[12]) < 0)
		{
			//ERROR: File uses SMPTE timing -- Not implemented yet
			return;
		}
		else
		{
			//uses_smpte = 0;
			division = (mthd[12] << 8) | (mthd[13] & 0xff);
		}

		running_status = -1;

		unsigned int i;
		ZL_MidiTrack** Tracks = new ZL_MidiTrack* [ntracks];
		for (i = 0; i < ntracks; i++) Tracks[i] = NULL;

		midi_file_read_tracks(Tracks, ntracks);

		unsigned int start_ticks = 0; //the number of tempo ticks passed at the last tempo change
		unsigned int cur_ticks = 0;   //the number of tempo ticks passed
		//int begin_msec = 0;         //the time (msec) of the beginning of the file
		int start_msec = 0;           //the start time of the last tempo change
		int cur_msec = 0;             //the current time
		int miditempo = 480000;       //as indicated by MIDI SetTempo: n 24th of a usec per midi-clock. bravo!
		double deltatime = miditempo / division / 1000.0; //milliseconds per midi tick. depends on set-tempo

		for (i = 0; i < ntracks; i++)
			if (Tracks[i] != NULL)
				Tracks[i]->reset();

		ZL_MidiEvent *pPrevEvent = NULL;
		while (1)
		{
			bool bFoundEventOnTrack = false;
			int smallesttdiff = 0x7FFFFFFF;
			for (i = 0; i < ntracks; i++)
			{
				cur_msec = start_msec + (int)((cur_ticks - start_ticks) * deltatime);
				ZL_MidiTrack *t = Tracks[i];
				while (t != NULL && t->cur && t->ticks + t->cur->time <= cur_ticks)
				{
					bFoundEventOnTrack = true;
					t->ticks += t->cur->time;

					if (t->cur->type == MIDI_LYRIC)
 					{
 						if (pLyrics) (*pLyrics)[cur_msec] = *(ZL_String*)t->cur->ptr;
 						delete (ZL_String*)t->cur->ptr;
 						t->cur->type = 0;
 					}
					else if (t->cur->type == MIDI_SET_TEMPO)
					{
						miditempo = t->cur->param1;
						deltatime = (double) miditempo / division / 1000.0;
						start_msec = cur_msec;
						start_ticks = cur_ticks;
					}
					else if (t->cur->type == PROGRAM_CHANGE && t->cur->param1 >= 128) //fix out of bound program changes
					{
 						t->cur->param1 = t->cur->param1%128;
					}

					if (!t->cur->type || cur_msec > 18000000) //cut off insane long probably malformatted midis
					{
						ZL_MidiEvent *next = t->cur->next;
						delete t->cur;
						t->cur = next;
					}
					else
					{
						t->cur->time = cur_msec;
						if (!pFirstEvent) pFirstEvent = t->cur;
						else pPrevEvent->next = t->cur;
						pPrevEvent = t->cur;
						t->cur = t->cur->next;
					}
				}
				if (t != NULL && t->cur && t->ticks + t->cur->time > cur_ticks) { int diff = (int)(t->ticks + t->cur->time - cur_ticks); if (diff < smallesttdiff) { smallesttdiff = diff; } continue; }
			}
			if (!bFoundEventOnTrack) break;
			cur_ticks += smallesttdiff;
		}
		if (pPrevEvent) pPrevEvent->next = NULL;

		for (i = 0; i < ntracks; i++) if (Tracks[i] != NULL) delete Tracks[i];
		delete Tracks;
	}

	ZL_Midi_Impl(void* data, size_t size) : iInvalidGetC(-1), pFirstEvent(NULL)
	{
		unsigned int iTotalSize;
		unsigned char *pcBuf;
		memcpy(&iTotalSize, data, 4);
		pcBuf = new unsigned char[iTotalSize];
		uLongf ActualSize = iTotalSize;
		uncompress(pcBuf, &ActualSize, (unsigned char*)data+4, (uLongf)(size-4));
		unsigned int iEventSize = sizeof(ZL_MidiEvent)-sizeof(ZL_MidiEvent*);
		ZL_MidiEvent *pe = NULL;
		for (unsigned int i=0; i+iEventSize <= ActualSize; i+=iEventSize)
		{
			pe = (pe ? pe->next : pFirstEvent) = new ZL_MidiEvent();
			memcpy(&pe->time, pcBuf+i, iEventSize);
		}
	}

	/*
	void DumpMidiDataCompressed()
	{
		ZL_MidiEvent* e;
		unsigned char *pcBufComp, *pcBuf;
		int iEventSize = sizeof(ZL_MidiEvent)-sizeof(ZL_MidiEvent*);
		int iNum = 0; for (e = pFirstEvent; e; e = e->next, iNum++);
		unsigned int iTotalSize = iNum*iEventSize;
		unsigned int iCompSize = ((int)(iTotalSize*1.001))+12;
		pcBuf = new unsigned char[iTotalSize];
		pcBufComp = new unsigned char[iCompSize];
		int i; for (e = pFirstEvent, i = 0; e; e = e->next, i++)
			memcpy(pcBuf+iEventSize*i, &e->time, iEventSize);
		if (compress(pcBufComp, &iCompSize, pcBuf, iTotalSize) == Z_OK)
		{
			printf("unsigned char cMidiData[%d] = {\n", (int)(4+iCompSize));
			int iHexLine = 4;
			printf("0x%02X, 0x%02X, 0x%02X, 0x%02X, ", *(((char*)&iTotalSize)+0), *(((char*)&iTotalSize)+1), *(((char*)&iTotalSize)+2), *(((char*)&iTotalSize)+3));
			for (unsigned int i=0; i<iCompSize; i++) { printf("0x%02X, ", pcBufComp[i]); if (!(++iHexLine%16)) printf("\n"); }
			printf("};\n");
		}
	}
	*/

	bool midi_file_read_tracks(ZL_MidiTrack* Tracks[], int ntracks)
	{
		for (int i = 0; i < ntracks; i++)
		{
			unsigned char id[4], length[4];
			int found_track = 0;
			if (!ZL_RWread(src, id, 4, 1)) return false;
			while (!found_track)
			{
				if (strncmp((char*) id, "MTrk", 4) == 0)
				{
					found_track = 1;
					if (!ZL_RWread(src, length, 4, 1)) return false;
					int tracklen = length[3] | (length[2]<<8) | (length[1]<<16) | (length[0]<<24);;
					trackpos = 0;
					eot = 0;

					Tracks[i] = new ZL_MidiTrack();
					while (!eot && trackpos < tracklen)
					{
						if (!midi_file_read_event(Tracks[i])) return false;
					}
				}
				else
				{
					found_track = 0;
					if (!ZL_RWread(src, length, 4, 1)) return false;
					int skip = length[3] | (length[2]<<8) | (length[1]<<16) | (length[0]<<24);;
					if (ZL_RWseek(src, skip, RW_SEEK_CUR)) return false;
				}
			}
		}
		return true;
	}

	int midi_file_getc()
	{
		unsigned char c;
		if (iInvalidGetC >= 0)
		{
			c = iInvalidGetC;
			iInvalidGetC = -1;
		}
		else
		{
			if (!ZL_RWread(src, &c, 1, 1)) return -1;
			trackpos++;
		}
		return (int) c;
	}


	int midi_file_read_varlen()
	{
		int c;
		int varlen = 0;
		for (int i = 0;;i++)
		{
			if (i == 4) return -1; //ERROR: Invalid variable length number

			c = midi_file_getc();
			if (c < 0) return -1; //ERROR: Unexpected end of file
			if (c & 0x80)
			{
				varlen |= ((int)c & 0x7F);
				varlen <<= 7;
			}
			else
			{
				varlen += (int)c;
				break;
			}
		}
		return varlen;
	}

	int midi_file_read(void* buf, int len)
	{
		int n = (int)ZL_RWread(src, buf, 1, len);
		if (n != len) return 0;
		trackpos+=n;
		return n;
	}

	bool midi_file_read_event(ZL_MidiTrack *pMidiTrack)
	{
		int dtime;
		int status;
		unsigned char* metadata = NULL;
		unsigned char* dyn_buf = NULL;
		unsigned char static_buf[256];
		ZL_MidiEvent* evt;

		//read the delta-time of the event
		dtime = midi_file_read_varlen();
		if (dtime > 1000*60*10) dtime = 0; //throw away delays that are insanely high for malformatted midis

		//read the status byte
		status = midi_file_getc();
		if (status < 0) return false; //ERROR: Unexpected end of file

		//not a valid status byte: use the running status instead
		if ((status & 0x80) == 0)
		{
			if ((running_status & 0x80) == 0) return false; //ERROR: Undefined status and invalid running status
			iInvalidGetC = status;
			status = running_status;
		}

		//check what message we have
		if (status & 0x80)
		{
			int type;
			running_status = status;

			if ((status == MIDI_SYSEX) || (status == MIDI_EOX)) //system exclusif
			{
				//Sysex messages are not handled yet
				//read the length of the message
				int buflen = midi_file_read_varlen();
				if (buflen > 0)
				{
					if (buflen < 255) { metadata = static_buf; }
					else
					{
						dyn_buf = (unsigned char*)malloc(buflen + 1);
						if (dyn_buf == NULL) return false; //ERROR: Out of memory
						metadata = dyn_buf;
					}

					//read the data of the message
					if (!midi_file_read(metadata, buflen))
					{
						if (dyn_buf) free(dyn_buf);
						return false;
					}

					if (dyn_buf) free(dyn_buf);
				}
				return true;
			}
			else if (status == MIDI_META_EVENT) //meta events
			{
				bool result = true;
				int tempo, sf, mi;

				//get the type of the meta message
				type = midi_file_getc();
				if (type < 0) return false; //ERROR: Unexpected end of file

				//get the length of the data part
				int buflen = midi_file_read_varlen();
				if (buflen > 0)
				{
					if (buflen < 255) metadata = static_buf;
					else
					{
						dyn_buf = (unsigned char*)malloc(buflen + 1);
						if (dyn_buf == NULL) return false; //ERROR: Out of memory
						metadata = dyn_buf;
					}

					//read the data
					if (!midi_file_read(metadata, buflen))
					{
						if (dyn_buf) free(dyn_buf);
						return false;
					}
				}

				//handle meta data
				bool bStoredMetaEvent = false;
				switch (type)
				{
					case MIDI_COPYRIGHT:
						if (!metadata || buflen<=0) break;
						metadata[buflen] = 0;
						break;

					case MIDI_TRACK_NAME:
						if (!metadata || buflen<=0) break;
						metadata[buflen] = 0;
						pMidiTrack->name = (char*) metadata;
						break;

					case MIDI_INST_NAME:
						if (!metadata || buflen<=0) break;
						metadata[buflen] = 0;
						break;

					case MIDI_LYRIC:
					case MIDI_TEXT:
 						if (!metadata || buflen<=0 || bIgnoreLyrics) break;
 						metadata[buflen] = 0;
 						evt = new ZL_MidiEvent();
 						evt->time = dtime;
 						evt->type = MIDI_LYRIC;
						//remove karaoke formatting information characters from string in metadata
						{ int o = 0; char *p; for (p = (char*)metadata; *p != 0; p++) { if (o) *(p-o) = *p; if (*p==';'||*p=='/'||*p=='\\'||*p=='\n'||*p=='\r') o++; } if (o) *(p-o) = *p; }
 						evt->ptr = new ZL_String((char*)metadata);
 						pMidiTrack->add_event(evt);
 						bStoredMetaEvent = true;
						break;

					case MIDI_MARKER:
						break;

					case MIDI_CUE_POINT:
						break;

					case MIDI_EOT:
						if (buflen != 0) { result = false; break; } //ERROR: Invalid length for EndOfTrack event
						eot = 1;
						break;

					case MIDI_SET_TEMPO:
						if (buflen != 3) { result = false; break; } //ERROR: Invalid length for SetTempo meta event
						tempo = (metadata[0] << 16) + (metadata[1] << 8) + metadata[2];
						evt = new ZL_MidiEvent();
						evt->time = dtime;
						evt->type = MIDI_SET_TEMPO;
						evt->channel = 0;
						evt->param1 = tempo;
						evt->param2 = 0;
						pMidiTrack->add_event(evt);
						bStoredMetaEvent = true;
						break;

					case MIDI_SMPTE_OFFSET:
						if (buflen != 5) { result = false; break; } //ERROR: Invalid length for SMPTE Offset meta event
						break; //we don't use smtp

					case MIDI_TIME_SIGNATURE:
						if (buflen != 4) { result = false; break; } //ERROR: Invalid length for TimeSignature meta event
						//int nominator = metadata[0];
						//int denominator = (int)(pow(2.0, (double) metadata[1]));
						//int clocks = metadata[2];
						//int notes = metadata[3];

						evt = new ZL_MidiEvent();
						evt->time = dtime;
						evt->type = MIDI_TIME_SIGNATURE;
						evt->channel = 0;
						memcpy(&evt->param1, metadata, sizeof(evt->param1));
						evt->param2 = 0;
						pMidiTrack->add_event(evt);
						bStoredMetaEvent = true;
						break;

					case MIDI_KEY_SIGNATURE:
						if (buflen != 2) { result = false; break; } //ERROR: Invalid length for KeySignature meta event
						sf = metadata[0];
						mi = metadata[1];

						evt = new ZL_MidiEvent();
						evt->time = dtime;
						evt->type = MIDI_KEY_SIGNATURE;
						evt->channel = 0;
						evt->param1 = sf;
						evt->param2 = mi;
						pMidiTrack->add_event(evt);
						bStoredMetaEvent = true;
						break;

					case MIDI_SEQUENCER_EVENT:
						break;

					default:
						break;
				}

				if (!bStoredMetaEvent && dtime)
				{
						evt = new ZL_MidiEvent();
						evt->time = dtime;
						pMidiTrack->add_event(evt);
				}

				if (dyn_buf) free(dyn_buf);

				return result;

			}
			else
			{
				//channel messages
				int param1 = 0;
				int param2 = 0;

				type = status & 0xf0;
				int channel = status & 0x0f;

				//all channel message have at least 1 byte of associated data
				if ((param1 = midi_file_getc()) < 0) return false; //ERROR: Unexpected end of file

				switch (type)
				{
					case NOTE_ON:
						if ((param2 = midi_file_getc()) < 0) return false; //ERROR: Unexpected end of file
						break;

					case NOTE_OFF:
						if ((param2 = midi_file_getc()) < 0) return false; //ERROR: Unexpected end of file
						break;

					case KEY_PRESSURE:
						if ((param2 = midi_file_getc()) < 0) return false; //ERROR: Unexpected end of file
						break;

					case CONTROL_CHANGE:
						if ((param2 = midi_file_getc()) < 0) return false; //ERROR: Unexpected end of file
						break;

					case PROGRAM_CHANGE:
						break;

					case CHANNEL_PRESSURE:
						break;

					case PITCH_BEND:
						if ((param2 = midi_file_getc()) < 0) return false; //ERROR: Unexpected end of file
						param1 = ((param2 & 0x7f) << 7) | (param1 & 0x7f);
						param2 = 0;
						break;

					default:
						//Should not happen, but ignore anyway
						//return false; //ERROR: Unrecognized MIDI event
						break;
				}

				evt = new ZL_MidiEvent();
				evt->time = dtime;
				evt->type = type;
				evt->channel = channel;
				evt->param1 = param1;
				evt->param2 = param2;
				pMidiTrack->add_event(evt);
			}
		}
		return true;
	}
};


ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Midi)

ZL_Midi::ZL_Midi(const ZL_File& File, std::map<int, ZL_String> *pLyrics) : impl(new ZL_Midi_Impl(File, pLyrics))
{
}

ZL_Midi::ZL_Midi(void* data, size_t size) : impl(new ZL_Midi_Impl(data, size))
{
}

ZL_MidiEvent *ZL_Midi::GetFirstEvent()
{
	return (impl ? impl->pFirstEvent : NULL);
}

#endif //ZL_OPT_DO_IMPLEMENTATION
