/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

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

#ifndef SDL_XAUDIO2_H_
#define SDL_XAUDIO2_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmreg.h>
#include <objbase.h>

/* XAudio2 packs its structure members together as tightly as possible.
   This pragma is needed to ensure compatibility with XAudio2 on 64-bit
   platforms.
*/
#pragma pack(push, 1)

#define X2DEFAULT(x)
#undef OPAQUE
#define OPAQUE struct

#define DEFINE_GUID_X(n, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) static const GUID n = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#define DEFINE_CLSID_X(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) DEFINE_GUID_X(CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define DEFINE_IID_X(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) DEFINE_GUID_X(IID_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)

#ifndef INTERFACE
#define INTERFACE void
#endif

DEFINE_CLSID_X(XAudio2, 5a508685, a254, 4fba, 9b, 82, 9a, 24, b0, 03, 06, af); /* 2.7 */
DEFINE_IID_X(IXAudio2, 8bcf1f58, 9fe7, 4583, 8a, c6, e2, ad, c4, 65, c8, bb); /* 2.7 */

/* 2.7 : #define XAUDIO2_DEFAULT_FREQ_RATIO 4.0f */
/* 2.9 : #define XAUDIO2_DEFAULT_FREQ_RATIO 2.0f */

#define XAUDIO2_COMMIT_NOW          0
#define XAUDIO2_DEFAULT_CHANNELS    0
#define XAUDIO2_DEFAULT_SAMPLERATE  0

#define XAUDIO2_DEBUG_ENGINE  0x0001
#define XAUDIO2_VOICE_NOPITCH 0x0002
#define XAUDIO2_VOICE_NOSRC   0x0004

typedef enum _AUDIO_STREAM_CATEGORY
{
    AudioCategory_Other = 0,
    AudioCategory_ForegroundOnlyMedia,
    AudioCategory_BackgroundCapableMedia,
    AudioCategory_Communications,
    AudioCategory_Alerts,
    AudioCategory_SoundEffects,
    AudioCategory_GameEffects,
    AudioCategory_GameMedia,
} AUDIO_STREAM_CATEGORY;

typedef enum XAUDIO2_DEVICE_ROLE
{
    NotDefaultDevice = 0x0,
    DefaultConsoleDevice = 0x1,
    DefaultMultimediaDevice = 0x2,
    DefaultCommunicationsDevice = 0x4,
    DefaultGameDevice = 0x8,
    GlobalDefaultDevice = 0xf,
    InvalidDeviceRole = ~GlobalDefaultDevice
} XAUDIO2_DEVICE_ROLE;

typedef enum XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER
{
#if defined(__STDC_C89__)
    XAUDIO2_ANY_PROCESSOR = 0xffff,
#else
    XAUDIO2_ANY_PROCESSOR = 0xffffffff,
#endif
    XAUDIO2_DEFAULT_PROCESSOR = XAUDIO2_ANY_PROCESSOR
} XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER, XAUDIO2_PROCESSOR;

typedef struct XAUDIO2_DEVICE_DETAILS
{
    WCHAR DeviceID[256];
    WCHAR DisplayName[256];
    XAUDIO2_DEVICE_ROLE Role;
    WAVEFORMATEXTENSIBLE OutputFormat;
} XAUDIO2_DEVICE_DETAILS;

/* Forward declarations. */
typedef OPAQUE XAUDIO2_VOICE_DETAILS XAUDIO2_VOICE_DETAILS;
typedef OPAQUE XAUDIO2_VOICE_SENDS XAUDIO2_VOICE_SENDS;
typedef OPAQUE XAUDIO2_EFFECT_DESCRIPTOR XAUDIO2_EFFECT_DESCRIPTOR;
typedef OPAQUE XAUDIO2_EFFECT_CHAIN XAUDIO2_EFFECT_CHAIN;
typedef OPAQUE XAUDIO2_FILTER_PARAMETERS XAUDIO2_FILTER_PARAMETERS;
typedef OPAQUE XAUDIO2_BUFFER_WMA XAUDIO2_BUFFER_WMA;
typedef OPAQUE XAUDIO2_VOICE_STATE XAUDIO2_VOICE_STATE;
typedef OPAQUE XAUDIO2_PERFORMANCE_DATA XAUDIO2_PERFORMANCE_DATA;
typedef OPAQUE XAUDIO2_DEBUG_CONFIGURATION XAUDIO2_DEBUG_CONFIGURATION;
typedef OPAQUE IXAudio2EngineCallback IXAudio2EngineCallback;
typedef OPAQUE IXAudio2SubmixVoice IXAudio2SubmixVoice;

typedef struct XAUDIO2_BUFFER
{
    UINT32 Flags;
    UINT32 AudioBytes;
    const BYTE* pAudioData;
    UINT32 PlayBegin;
    UINT32 PlayLength;
    UINT32 LoopBegin;
    UINT32 LoopLength;
    UINT32 LoopCount;
    void *pContext;
} XAUDIO2_BUFFER;

typedef struct XAUDIO2_VOICE_STATE {
    void *pCurrentBufferContext;
    UINT32 BuffersQueued;
    UINT64 SamplesPlayed;
} XAUDIO2_VOICE_STATE;

#undef INTERFACE
#define INTERFACE IXAudio2VoiceCallback
DECLARE_INTERFACE(IXAudio2VoiceCallback)
{
    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) PURE;
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) PURE;
    STDMETHOD_(void, OnStreamEnd) (THIS) PURE;
    STDMETHOD_(void, OnBufferStart) (THIS_ void *pBufferContext) PURE;
    STDMETHOD_(void, OnBufferEnd) (THIS_ void *pBufferContext) PURE;
    STDMETHOD_(void, OnLoopEnd) (THIS_ void *pBufferContext) PURE;
    STDMETHOD_(void, OnVoiceError) (THIS_ void *pBufferContext, HRESULT Error) PURE;
};

#undef INTERFACE
#define INTERFACE IXAudio2Voice
DECLARE_INTERFACE(IXAudio2Voice)
{
#define Declare_IXAudio2Voice_Methods() \
    STDMETHOD_(void, GetVoiceDetails) (THIS_ XAUDIO2_VOICE_DETAILS* pVoiceDetails) PURE; \
    STDMETHOD(SetOutputVoices) (THIS_ const XAUDIO2_VOICE_SENDS* pSendList) PURE; \
    STDMETHOD(SetEffectChain) (THIS_ const XAUDIO2_EFFECT_CHAIN* pEffectChain) PURE; \
    STDMETHOD(EnableEffect) (THIS_ UINT32 EffectIndex, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD(DisableEffect) (THIS_ UINT32 EffectIndex, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD_(void, GetEffectState) (THIS_ UINT32 EffectIndex, BOOL* pEnabled) PURE; \
    STDMETHOD(SetEffectParameters) (THIS_ UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD(GetEffectParameters) (THIS_ UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize) PURE; \
    STDMETHOD(SetFilterParameters) (THIS_ const XAUDIO2_FILTER_PARAMETERS* pParameters, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD_(void, GetFilterParameters) (THIS_ XAUDIO2_FILTER_PARAMETERS* pParameters) PURE; \
    /* ------------------- */ \
    /* 2.7 only */ \
    /* STDMETHOD_(void, SetOutputFilterParameters) (THIS_ IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS* pParameters, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; */ \
    /* 2.9 only */ \
    /* STDMETHOD(SetOutputFilterParameters) (THIS_ IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS* pParameters, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; */ \
    /* generic unused padding method */ \
    STDMETHOD(_paddingmethod1)() PURE; \
    /* ------------------- */ \
    STDMETHOD_(void, GetOutputFilterParameters) (THIS_ IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS* pParameters) PURE; \
    STDMETHOD(SetVolume) (THIS_ float Volume, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD_(void, GetVolume) (THIS_ float* pVolume) PURE; \
    STDMETHOD(SetChannelVolumes) (THIS_ UINT32 Channels, const float* pVolumes, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD_(void, GetChannelVolumes) (THIS_ UINT32 Channels, float* pVolumes) PURE; \
    STDMETHOD(SetOutputMatrix) (THIS_ IXAudio2Voice* pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float* pLevelMatrix, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    STDMETHOD_(void, GetOutputMatrix) (THIS_ IXAudio2Voice* pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float* pLevelMatrix) PURE; \
    STDMETHOD_(void, DestroyVoice) (THIS) PURE

    Declare_IXAudio2Voice_Methods();
};

#undef INTERFACE
#define INTERFACE IXAudio2MasteringVoice
DECLARE_INTERFACE_(IXAudio2MasteringVoice, IXAudio2Voice)
{
    Declare_IXAudio2Voice_Methods();
};

#undef INTERFACE
#define INTERFACE IXAudio2SourceVoice
DECLARE_INTERFACE_(IXAudio2SourceVoice, IXAudio2Voice)
{
    Declare_IXAudio2Voice_Methods();
    STDMETHOD(Start) (THIS_ UINT32 Flags, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD(Stop) (THIS_ UINT32 Flags, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD(SubmitSourceBuffer) (THIS_ const XAUDIO2_BUFFER* pBuffer, const XAUDIO2_BUFFER_WMA* pBufferWMA X2DEFAULT(NULL)) PURE;
    STDMETHOD(FlushSourceBuffers) (THIS) PURE;
    STDMETHOD(Discontinuity) (THIS) PURE;
    STDMETHOD(ExitLoop) (THIS_ UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;
    /* 2.7 : STDMETHOD_(void, GetState) (THIS_ XAUDIO2_VOICE_STATE* pVoiceState) PURE; */
    /* 2.9 : STDMETHOD_(void, GetState) (THIS_ XAUDIO2_VOICE_STATE* pVoiceState, UINT32 Flags X2DEFAULT(0)) PURE; */
    STDMETHOD_(void, _PlaceHolder_GetState) () PURE;
    STDMETHOD(SetFrequencyRatio) (THIS_ float Ratio, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;
    STDMETHOD_(void, GetFrequencyRatio) (THIS_ float* pRatio) PURE;
    /* 2.9 : STDMETHOD(SetSourceSampleRate) (THIS_ UINT32 NewSourceSampleRate) PURE; */
};
typedef  void (__stdcall * IXAudio2SourceVoice_27_GetStateFunc)(IXAudio2SourceVoice*, XAUDIO2_VOICE_STATE*);
typedef  void (__stdcall * IXAudio2SourceVoice_29_GetStateFunc)(IXAudio2SourceVoice*, XAUDIO2_VOICE_STATE*, UINT32);

#undef INTERFACE
#define INTERFACE IXAudio2_27
DECLARE_INTERFACE_(IXAudio2_27, IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void** ppvInterface) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;
    STDMETHOD(GetDeviceCount) (THIS_ UINT32* pCount) PURE;
    STDMETHOD(GetDeviceDetails) (THIS_ UINT32 Index, XAUDIO2_DEVICE_DETAILS* pDeviceDetails) PURE;
    STDMETHOD(Initialize) (THIS_ UINT32 Flags X2DEFAULT(0), XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR)) PURE;
    STDMETHOD(RegisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;
    STDMETHOD_(void, UnregisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;
    STDMETHOD(CreateSourceVoice) (THIS_ IXAudio2SourceVoice** ppSourceVoice, const WAVEFORMATEX* pSourceFormat, UINT32 Flags X2DEFAULT(0), float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO), IXAudio2VoiceCallback* pCallback X2DEFAULT(NULL), const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL), const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;
    STDMETHOD(CreateSubmixVoice) (THIS_ IXAudio2SubmixVoice** ppSubmixVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0), const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL), const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;
    STDMETHOD(CreateMasteringVoice) (THIS_ IXAudio2MasteringVoice** ppMasteringVoice, UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS), UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE), UINT32 Flags X2DEFAULT(0), UINT32 DeviceIndex X2DEFAULT(0), const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;
    STDMETHOD(StartEngine) (THIS) PURE;
    STDMETHOD_(void, StopEngine) (THIS) PURE;
    STDMETHOD(CommitChanges) (THIS_ UINT32 OperationSet) PURE;
    STDMETHOD_(void, GetPerformanceData) (THIS_ XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;
    STDMETHOD_(void, SetDebugConfiguration) (THIS_ const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration, void *pReserved X2DEFAULT(NULL)) PURE;
};

#undef INTERFACE
#define INTERFACE IXAudio2_29
DECLARE_INTERFACE_(IXAudio2_29, IUnknown)
{
   STDMETHOD(QueryInterface) (THIS_ REFIID riid, void** ppvInterface) PURE;
   STDMETHOD_(ULONG, AddRef) (THIS) PURE;
   STDMETHOD_(ULONG, Release) (THIS) PURE;
   STDMETHOD(RegisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;
   STDMETHOD_(void, UnregisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;
   STDMETHOD(CreateSourceVoice) (THIS_ IXAudio2SourceVoice** ppSourceVoice, const WAVEFORMATEX* pSourceFormat, UINT32 Flags X2DEFAULT(0), float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO), IXAudio2VoiceCallback* pCallback X2DEFAULT(NULL), const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL), const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;
   STDMETHOD(CreateSubmixVoice) (THIS_ IXAudio2SubmixVoice** ppSubmixVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0), const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL), const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;
   STDMETHOD(CreateMasteringVoice) (THIS_ IXAudio2MasteringVoice** ppMasteringVoice, UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS), UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE), UINT32 Flags X2DEFAULT(0), LPCWSTR szDeviceId X2DEFAULT(NULL), const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL), AUDIO_STREAM_CATEGORY StreamCategory X2DEFAULT(AudioCategory_GameEffects)) PURE;
   STDMETHOD(StartEngine) (THIS) PURE;
   STDMETHOD_(void, StopEngine) (THIS) PURE;
   STDMETHOD(CommitChanges) (THIS_ UINT32 OperationSet) PURE;
   STDMETHOD_(void, GetPerformanceData) (THIS_ XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;
   STDMETHOD_(void, SetDebugConfiguration) (THIS_ const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration, void *pReserved X2DEFAULT(NULL)) PURE;
};

#define IXAudio2_27_Initialize(handle,a,b) ((IXAudio2_27*)(handle))->lpVtbl->Initialize((IXAudio2_27*)(handle), a, b)
#define IXAudio2_27_Release(handle) (((IXAudio2_27*)(handle)))->lpVtbl->Release((IXAudio2_27*)(handle))
#define IXAudio2_27_CreateSourceVoice(handle,ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain) (((IXAudio2_27*)(handle)))->lpVtbl->CreateSourceVoice((IXAudio2_27*)(handle), ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain)
#define IXAudio2_27_CreateMasteringVoice(handle,ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceIndex,pEffectChain) (((IXAudio2_27*)(handle)))->lpVtbl->CreateMasteringVoice((IXAudio2_27*)(handle), ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceIndex,pEffectChain)
#define IXAudio2_27_StartEngine(handle) (((IXAudio2_27*)(handle)))->lpVtbl->StartEngine((IXAudio2_27*)(handle))
#define IXAudio2_27_StopEngine(handle) (((IXAudio2_27*)(handle)))->lpVtbl->StopEngine((IXAudio2_27*)(handle))
#define IXAudio2_29_Release(handle) (((IXAudio2_29*)(handle)))->lpVtbl->Release((IXAudio2_29*)(handle))
#define IXAudio2_29_CreateSourceVoice(handle,ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain) (((IXAudio2_29*)(handle)))->lpVtbl->CreateSourceVoice((IXAudio2_29*)(handle), ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain)
#define IXAudio2_29_CreateMasteringVoice(handle,ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceId,pEffectChain,StreamCategory) (((IXAudio2_29*)(handle)))->lpVtbl->CreateMasteringVoice((IXAudio2_29*)(handle), ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceId,pEffectChain,StreamCategory)
#define IXAudio2_29_StartEngine(handle) (((IXAudio2_29*)(handle)))->lpVtbl->StartEngine((IXAudio2_29*)(handle))
#define IXAudio2_29_StopEngine(handle) (((IXAudio2_29*)(handle)))->lpVtbl->StopEngine((IXAudio2_29*)(handle))
#define IXAudio2MasteringVoice_DestroyVoice(handle) (handle)->lpVtbl->DestroyVoice(handle)
#define IXAudio2SourceVoice_Start(handle, Flags, OperationSet) (handle)->lpVtbl->Start(handle, Flags, OperationSet)
#define IXAudio2SourceVoice_Stop(handle, Flags, OperationSet) (handle)->lpVtbl->Stop(handle, Flags, OperationSet)
#define IXAudio2SourceVoice_SubmitSourceBuffer(handle, pBuffer, pBufferWMA) (handle)->lpVtbl->SubmitSourceBuffer(handle, pBuffer, pBufferWMA)
#define IXAudio2SourceVoice_DestroyVoice(handle) (handle)->lpVtbl->DestroyVoice(handle)
#define IXAudio2SourceVoice_FlushSourceBuffers(handle) (handle)->lpVtbl->FlushSourceBuffers(handle)
#define IXAudio2SourceVoice_Discontinuity(handle) (handle)->lpVtbl->Discontinuity(handle)
#define IXAudio2SourceVoice_GetState(is29, handle, a) (is29 ? ((IXAudio2SourceVoice_29_GetStateFunc)((handle)->lpVtbl->_PlaceHolder_GetState))(handle, a, 0) : ((IXAudio2SourceVoice_27_GetStateFunc)((handle)->lpVtbl->_PlaceHolder_GetState))(handle, a))

typedef struct IXAudio2_Generic IXAudio2_Generic;

static HRESULT SDL_XAudio2Create(IXAudio2_Generic **ppXAudio2, UINT32 flags, XAUDIO2_PROCESSOR proc, SDL_bool* is29)
{
    static HMODULE xa29instance, xa27reference;
    HRESULT hr;
    SDL_bool had29 = (xa29instance != NULL);
    if (had29 || (!xa27reference && (xa29instance = LoadLibraryA("xaudio2_9.dll")) != NULL))
    {
        /* When compiled for RS5 or later, try to invoke XAudio2CreateWithVersionInfo.
         * Need to use LoadLibrary in case the app is running on an older OS. */
        typedef HRESULT(__stdcall *XAudio2CreateWithVersionInfoFunc)(IXAudio2_29**, UINT32, XAUDIO2_PROCESSOR, DWORD);
        typedef HRESULT(__stdcall *XAudio2CreateInfoFunc)(IXAudio2_29**, UINT32, XAUDIO2_PROCESSOR);
        static XAudio2CreateWithVersionInfoFunc pfnAudio2CreateWithVersion;
        static XAudio2CreateInfoFunc pfnAudio2Create;

        *is29 = SDL_TRUE;

        if (!pfnAudio2CreateWithVersion) pfnAudio2CreateWithVersion = (XAudio2CreateWithVersionInfoFunc)(void*)GetProcAddress(xa29instance, "XAudio2CreateWithVersionInfo");
        hr = (*pfnAudio2CreateWithVersion)((IXAudio2_29**)ppXAudio2, flags, proc, NTDDI_VERSION);
        if (SUCCEEDED(hr)) return hr;

        if (!pfnAudio2Create) pfnAudio2Create = (XAudio2CreateInfoFunc)(void*)GetProcAddress(xa29instance, "XAudio2Create");
        hr = (*pfnAudio2Create)((IXAudio2_29**)ppXAudio2, flags, proc);
        if (SUCCEEDED(hr)) return hr;
    }

    if (!xa27reference) { hr = WIN_CoInitialize(); if (FAILED(hr)) return hr; }
    hr = CoCreateInstance(&CLSID_XAudio2, NULL, CLSCTX_INPROC_SERVER, &IID_IXAudio2, (void**)ppXAudio2);
    if (SUCCEEDED(hr))
    {
        hr = IXAudio2_27_Initialize(*ppXAudio2, flags, proc);
        if (SUCCEEDED(hr))
        {
            /* Create an explicit reference to the DLL and hold on to it. */
            /* See: https://walbourn.github.io/known-issues-xaudio-2-7/ */
            if (!xa27reference) xa27reference = LoadLibraryA("XAudio2_7.dll");
            *is29 = SDL_FALSE;
            return hr;
        }
        IXAudio2_27_Release(*ppXAudio2);
    }
    WIN_CoUninitialize();
    return hr;
}

#pragma pack(pop) /* Undo pragma push */

#endif  /* SDL_XAUDIO2_H_ */

/* vi: set ts=4 sw=4 expandtab: */
