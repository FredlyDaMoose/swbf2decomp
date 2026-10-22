#pragma once

#include "RedEngineFL/input/RedJoystick.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/input/RedMouse.h"

struct RedLogitechLCD;

namespace RedInputManager
{

void Init();

IDirectInput8A* pcGetDIInterface();

BOOL CALLBACK pcEnumJoystickDevice(DIDEVICEINSTANCEA const* pInstance, void* pvRef);

RedMouse* GetMouse(unsigned char ucPort);

void Cleanup();
void Update(float fDeltaTime);
RedJoystick* GetJoystick(unsigned char ucPort);
RedKeyboard* GetKeyboard(unsigned char ucPort);
unsigned int GetNumJoystickPorts();
unsigned int GetNumKeybaordPorts();
unsigned int GetNumMousePorts();

unsigned int GetNumLCDPorts();

RedLogitechLCD* GetLCDPanel(unsigned char ucPort);

extern bool isReadingJournal;
extern IDirectInput8A* m_pDI;
extern unsigned int m_numJoysticks;
extern RedKeyboard m_keyboards[1];
extern RedMouse m_mouses[1];
extern RedJoystick m_joysticks[10];

}

// VTABLE: SWBF2 0x007833b0
class FakeInputDevice : public IDirectInputDevice8A
{
public:

    FakeInputDevice() : refCount(1) {}

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(GetCapabilities)(LPDIDEVCAPS);
    STDMETHOD(EnumObjects)(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD);
    STDMETHOD(GetProperty)(REFGUID, LPDIPROPHEADER);
    STDMETHOD(SetProperty)(REFGUID, LPCDIPROPHEADER);
    STDMETHOD(Acquire)();
    STDMETHOD(Unacquire)();
    STDMETHOD(GetDeviceState)(DWORD, LPVOID);
    STDMETHOD(GetDeviceData)(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
    STDMETHOD(SetDataFormat)(LPCDIDATAFORMAT);
    STDMETHOD(SetEventNotification)(HANDLE);
    STDMETHOD(SetCooperativeLevel)(HWND, DWORD);
    STDMETHOD(GetObjectInfo)(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD);
    STDMETHOD(GetDeviceInfo)(LPDIDEVICEINSTANCEA);
    STDMETHOD(RunControlPanel)(HWND, DWORD);
    STDMETHOD(Initialize)(HINSTANCE, DWORD, REFGUID);
    STDMETHOD(CreateEffect)(REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT*, LPUNKNOWN);
    STDMETHOD(EnumEffects)(LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD);
    STDMETHOD(GetEffectInfo)(LPDIEFFECTINFOA, REFGUID);
    STDMETHOD(GetForceFeedbackState)(LPDWORD);
    STDMETHOD(SendForceFeedbackCommand)(DWORD);
    STDMETHOD(EnumCreatedEffectObjects)(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD);
    STDMETHOD(Escape)(LPDIEFFESCAPE);
    STDMETHOD(Poll)();
    STDMETHOD(SendDeviceData)(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD);
    STDMETHOD(EnumEffectsInFile)(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD);
    STDMETHOD(WriteEffectToFile)(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD);
    STDMETHOD(BuildActionMap)(LPDIACTIONFORMATA, LPCSTR, DWORD);
    STDMETHOD(SetActionMap)(LPDIACTIONFORMATA, LPCSTR, DWORD);
    STDMETHOD(GetImageInfo)(LPDIDEVICEIMAGEINFOHEADERA);

    int refCount;
};
 

// VTABLE: SWBF2 0x00783434
class FakeDirectInput : public IDirectInput8A
{
public:

    FakeDirectInput() : refCount(1) {}

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(CreateDevice)(REFGUID, LPDIRECTINPUTDEVICE8A*, LPUNKNOWN);
    STDMETHOD(EnumDevices)(DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
    STDMETHOD(GetDeviceStatus)(REFGUID);
    STDMETHOD(RunControlPanel)(HWND, DWORD);
    STDMETHOD(Initialize)(HINSTANCE, DWORD);
    STDMETHOD(FindDevice)(REFGUID, LPCSTR, LPGUID);
    STDMETHOD(EnumDevicesBySemantics)(LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPVOID, DWORD);
    STDMETHOD(ConfigureDevices)(LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSA, DWORD, LPVOID);

    int refCount;
};
 

extern bool gDisableLCD;
