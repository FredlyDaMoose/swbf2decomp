#pragma once

#if !defined(DIRECTSOUND_VERSION) || DIRECTSOUND_VERSION < 0x0800
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800
#endif

#include <windows.h>
#include <dsound.h>

namespace Snd
{

class DSDeviceEnumerator
{
public:

    enum Type
    {
        type_Invalid = 0,
        type_Render = 1,
        type_Capture = 2,
        type_Max = 3
    };

    enum { kMaxDevices = 256 };

    DSDeviceEnumerator();
    ~DSDeviceEnumerator();

    bool Enumerate(Type eType, bool bGetNames);
    const GUID& GetDeviceIDByIndex(unsigned int uiIndex) const;

private:
    bool AddDevice(const GUID* const pGuid,
                   const char* const pcName);
    void FreeNames();

    static BOOL CALLBACK EnumCallback(LPGUID lpGuid,
                                      LPCSTR lpcstrDescription,
                                      LPCSTR lpcstrModule,
                                      LPVOID lpContext);

    char* mDeviceNames[kMaxDevices];
    GUID mDeviceIDs[kMaxDevices];
    unsigned int mDeviceCount;
    unsigned char mGetNames;
};

 

}
