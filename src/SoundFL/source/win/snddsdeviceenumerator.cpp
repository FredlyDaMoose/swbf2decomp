#include <string.h>

#include "snddsdeviceenumerator.h"

#include "SoundFL/source/sndglobals.h"
#include "sndengine.h"

namespace Snd
{

static const char kFile[] = "source\\win\\snddsdeviceenumerator.cpp";

// FUNCTION: SWBF2 0x0074be00
DSDeviceEnumerator::DSDeviceEnumerator()
{
    memset(mDeviceNames, 0, sizeof(mDeviceNames));
    mDeviceCount = 0;
    mGetNames = 0;
}

void DSDeviceEnumerator::FreeNames()
{
    for (unsigned int i = 0; i < mDeviceCount; ++i)
    {
        if (mDeviceNames[i] != 0)
        {
            Free(mDeviceNames[i]);
        }
    }
}

// FUNCTION: SWBF2 0x0074be30
DSDeviceEnumerator::~DSDeviceEnumerator()
{
    FreeNames();
}

// FUNCTION: SWBF2 0x0074be60
bool DSDeviceEnumerator::AddDevice(const GUID* const pGuid, const char* const pcName)
{
    if (mDeviceCount >= kMaxDevices)
    {
        return false;
    }

    if (pcName != 0 && mGetNames)
    {
        mDeviceNames[mDeviceCount] = (char*)Alloc((unsigned int)strlen(pcName) + 1);
        if (mDeviceNames[mDeviceCount] != 0)
        {
            strcpy(mDeviceNames[mDeviceCount], pcName);
        }
    }

    if (pGuid != 0)
    {
        mDeviceIDs[mDeviceCount] = *pGuid;
    }
    else
    {
        memset(&mDeviceIDs[mDeviceCount], 0, sizeof(GUID));
    }

    ++mDeviceCount;
    return true;
}

// FUNCTION: SWBF2 0x0074bf20
BOOL CALLBACK DSDeviceEnumerator::EnumCallback(LPGUID lpGuid,
                                               LPCSTR lpcstrDescription,
                                               LPCSTR  ,
                                               LPVOID lpContext)
{
    return ((DSDeviceEnumerator*)lpContext)->AddDevice(lpGuid, lpcstrDescription);
}

// FUNCTION: SWBF2 0x0074bf40
bool DSDeviceEnumerator::Enumerate(Type eType, bool bGetNames)
{
    FreeNames();
    mDeviceCount = 0;
    mGetNames = bGetNames ? 1 : 0;

    HRESULT hr;
    switch (eType)
    {
    case type_Render:
        hr = DirectSoundEnumerateA(EnumCallback, this);
        break;
    case type_Capture:
        hr = DirectSoundCaptureEnumerateA(EnumCallback, this);
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return Engine::Succeeded(hr, kFile, 55);
}

// FUNCTION: SWBF2 0x0074bfc0
const GUID& DSDeviceEnumerator::GetDeviceIDByIndex(unsigned int uiIndex) const
{
    return mDeviceIDs[uiIndex];
}

}
