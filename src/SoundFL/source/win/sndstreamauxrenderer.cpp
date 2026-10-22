#include <new>
#include <string.h>

#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "snddsbufferrenderer.h"
#include "sndstreamauxrenderer.h"

#include "snddsdeviceenumerator.h"
#include "sndengine.h"
#include "sndsoftoutput.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073f310
StreamAuxRenderer::StreamAuxRenderer()
    : mIsOpen(false),
      mDevice(0)
{

}

// FUNCTION: SWBF2 0x0073f340
StreamAuxRenderer::~StreamAuxRenderer()
{

    if (mIsOpen)
    {
        StreamAuxRenderer__Close(this);
    }
}

// FUNCTION: SWBF2 0x0073f3b0
void* __fastcall sub_73F3B0(void* pAuxRenderer, unsigned int uiFlags)
{
    char* const pThis = (char*)pAuxRenderer;

    ((StreamAuxRenderer*)pThis)->StreamAuxRenderer::~StreamAuxRenderer();

    if ((uiFlags & 1) != 0)
    {
        switch (((DSBufferRenderer*)pThis)->GetAllocType())
        {
        case Allocator<DSBufferRenderer>::allocType_heap:
            Free(pThis);
            break;
        case Allocator<DSBufferRenderer>::allocType_tempHeap:
            FreeTemp(pThis);
            break;
        default:
            break;
        }
    }

    return pThis;
}

// FUNCTION: SWBF2 0x0073f440
void __fastcall StreamAuxRenderer__Close(void* pAuxRenderer)
{
    StreamAuxRenderer* const pThis = (StreamAuxRenderer*)pAuxRenderer;

    Engine::GetThreadUpdate()->Pause(false);

    pThis->SetFormat(SampleInfo::format_Unknown, 0, 0,
                     false, 200.0f, false, 0, 0, false);

    IDirectSound* const pDevice = pThis->mDevice;
    pThis->mIsOpen = false;

    if (pDevice != 0)
    {
        pDevice->Release();
        pThis->mDevice = 0;
    }

    Engine::GetThreadUpdate()->Resume();
}

// FUNCTION: SWBF2 0x0073f4a0
bool StreamAuxRenderer::Open(unsigned char ucDeviceIndex,
                             StreamFormat const& cFormat, float fBufferMs)
{

    const unsigned int uiIndex = ucDeviceIndex;

    GUID cGuidDevice;

    if (uiIndex == 0xFEu)
    {

        mDevice = Engine::smDirectSound;
        Engine::smDirectSound->AddRef();
    }
    else
    {
        if (uiIndex == 0xFFu)
        {

            cGuidDevice = DSDEVID_DefaultVoicePlayback;
        }
        else
        {

            DSDeviceEnumerator cContext;

            if (!cContext.Enumerate(DSDeviceEnumerator::type_Render, false))
            {
                return false;
            }

            cGuidDevice = cContext.GetDeviceIDByIndex(uiIndex);

            GUID cNull;
            memset(&cNull, 0, sizeof(cNull));

            if (memcmp(&cGuidDevice, &DSDEVID_DefaultPlayback, sizeof(GUID)) == 0
                || memcmp(&cGuidDevice, &cNull, sizeof(GUID)) == 0)
            {

                if (Engine::smDirectSound != 0)
                {
                    return Open(0xFE, cFormat, fBufferMs);
                }
            }

        }

        IDirectSound** const ppDS = &mDevice;

        const HRESULT hr = DirectSoundCreate(&cGuidDevice, ppDS, 0);
        if (!Engine::Succeeded(hr, "source\\win\\sndstreamauxrenderer.cpp", 64))
        {
            return mIsOpen;
        }

        Engine::UpdateCooperativeLevel(*ppDS, true);
    }

    mIsOpen = SetFormat(cFormat.GetFormat(),
                        cFormat.GetNumChannels(),
                        cFormat.GetFrequency(),
                        false,
                        fBufferMs,
                        false,
                        mDevice,
                        0,
                        true);

    if (!mIsOpen)
    {

        mDevice->Release();
        mDevice = 0;
    }

    return mIsOpen;
}

// FUNCTION: SWBF2 0x0073f6c0
void StreamAuxRenderer::UpdateObject()
{
    IDirectSound* const pDevice = mDevice;
    if (pDevice == 0)
    {
        return;
    }

    Engine::UpdateCooperativeLevel(pDevice, false);

    DSBufferRenderer::UpdateObject();
}

}

// LIBRARY: SWBF2 0x00754908
// DirectSoundCreate
