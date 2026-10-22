#include "pcRedInputManager.h"

#include "PebbleFL/Common/PblJournal.h"
#include "Battlefront2/Source/NetSetup.h"
#include "lglcd/lgLcd.h"

extern HINSTANCE hGlobalInstance;

// FUNCTION: SWBF2 0x006dbc00
ULONG FakeDirectInput::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006dbc10
ULONG FakeInputDevice::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006dc0e0
ULONG FakeDirectInput::Release()
{
    if (--refCount <= 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006dc100
ULONG FakeInputDevice::Release()
{
    if (--refCount <= 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006dbce0
HRESULT FakeDirectInput::CreateDevice(REFGUID, LPDIRECTINPUTDEVICE8A* lplpDirectInputDevice, LPUNKNOWN)
{
    *lplpDirectInputDevice = new FakeInputDevice;
    return DI_OK;
}

// FUNCTION: SWBF2 0x006dc0c0
HRESULT FakeDirectInput::QueryInterface(REFIID, LPVOID*) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbd60
HRESULT FakeDirectInput::EnumDevices(DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbe10
HRESULT FakeDirectInput::GetDeviceStatus(REFGUID) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc120
HRESULT FakeDirectInput::RunControlPanel(HWND, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc090
HRESULT FakeDirectInput::Initialize(HINSTANCE, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbdc0
HRESULT FakeDirectInput::FindDevice(REFGUID, LPCSTR, LPGUID) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbd70
HRESULT FakeDirectInput::EnumDevicesBySemantics(LPCSTR, LPDIACTIONFORMATA, LPDIENUMDEVICESBYSEMANTICSCBA, LPVOID, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbcd0
HRESULT FakeDirectInput::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSA, DWORD, LPVOID) { return DI_OK; }

// FUNCTION: SWBF2 0x006dc0d0
HRESULT FakeInputDevice::QueryInterface(REFIID, LPVOID*) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbdd0
HRESULT FakeInputDevice::GetCapabilities(LPDIDEVCAPS) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbda0
HRESULT FakeInputDevice::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbf30
HRESULT FakeInputDevice::GetProperty(REFGUID, LPDIPROPHEADER) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc1a0
HRESULT FakeInputDevice::SetProperty(REFGUID, LPCDIPROPHEADER) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbbf0
HRESULT FakeInputDevice::Acquire() { return DI_OK; }
// FUNCTION: SWBF2 0x006dc1b0
HRESULT FakeInputDevice::Unacquire() { return DI_OK; }
// FUNCTION: SWBF2 0x006dbe00
HRESULT FakeInputDevice::GetDeviceState(DWORD, LPVOID) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbde0
HRESULT FakeInputDevice::GetDeviceData(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc180
HRESULT FakeInputDevice::SetDataFormat(LPCDIDATAFORMAT) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc190
HRESULT FakeInputDevice::SetEventNotification(HANDLE) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc170
HRESULT FakeInputDevice::SetCooperativeLevel(HWND, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbf20
HRESULT FakeInputDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbdf0
HRESULT FakeInputDevice::GetDeviceInfo(LPDIDEVICEINSTANCEA) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc130
HRESULT FakeInputDevice::RunControlPanel(HWND, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc0a0
HRESULT FakeInputDevice::Initialize(HINSTANCE, DWORD, REFGUID) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbd20
HRESULT FakeInputDevice::CreateEffect(REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT*, LPUNKNOWN) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbd80
HRESULT FakeInputDevice::EnumEffects(LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbe20
HRESULT FakeInputDevice::GetEffectInfo(LPDIEFFECTINFOA, REFGUID) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbe30
HRESULT FakeInputDevice::GetForceFeedbackState(LPDWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc150
HRESULT FakeInputDevice::SendForceFeedbackCommand(DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbd50
HRESULT FakeInputDevice::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbdb0
HRESULT FakeInputDevice::Escape(LPDIEFFESCAPE) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc0b0
HRESULT FakeInputDevice::Poll() { return DI_OK; }
// FUNCTION: SWBF2 0x006dc140
HRESULT FakeInputDevice::SendDeviceData(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbd90
HRESULT FakeInputDevice::EnumEffectsInFile(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc240
HRESULT FakeInputDevice::WriteEffectToFile(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbc20
HRESULT FakeInputDevice::BuildActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dc160
HRESULT FakeInputDevice::SetActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return DI_OK; }
// FUNCTION: SWBF2 0x006dbe40
HRESULT FakeInputDevice::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA) { return DI_OK; }

// FUNCTION: SWBF2 0x006dbd30
static IDirectInput8A* CreateFakeInput()
{
    return new FakeDirectInput;
}

namespace RedInputManager
{

bool isReadingJournal;
IDirectInput8A* m_pDI;
unsigned int m_numJoysticks;
// GLOBAL: SWBF2 0x0099cd18
// SYNTHETIC: SWBF2 0x004085d0
// `dynamic initializer for 'RedInputManager::m_keyboards''
// SYNTHETIC: SWBF2 0x007699d0
// `dynamic atexit destructor for 'RedInputManager::m_keyboards''
RedKeyboard m_keyboards[1];
// GLOBAL: SWBF2 0x0099d138
// SYNTHETIC: SWBF2 0x00408630
// `dynamic initializer for 'RedInputManager::m_mouses''
// SYNTHETIC: SWBF2 0x00769a10
// `dynamic atexit destructor for 'RedInputManager::m_mouses''
RedMouse m_mouses[1];
// GLOBAL: SWBF2 0x0099d188
// SYNTHETIC: SWBF2 0x004085a0
// `dynamic initializer for 'RedInputManager::m_joysticks''
// SYNTHETIC: SWBF2 0x007699b0
// `dynamic atexit destructor for 'RedInputManager::m_joysticks''
RedJoystick m_joysticks[10];

// FUNCTION: SWBF2 0x006dbec0
RedMouse* GetMouse(unsigned char ucPort)
{
    return &m_mouses[ucPort];
}

// FUNCTION: SWBF2 0x006dbf50
void Init()
{

    isReadingJournal = (PblJournal::Get(PJT_FULL)->mMode == PJM_READING);

    if (g_bNoRender)
    {

        m_pDI = 0;
    }
    else if (!isReadingJournal)
    {

        DirectInput8Create(hGlobalInstance, DIRECTINPUT_VERSION, IID_IDirectInput8A,
                           (void**)&m_pDI, 0);
    }

    if (!m_pDI)
    {
        m_pDI = CreateFakeInput();
    }

    for (unsigned char ucPort = 0; ucPort < 10; ++ucPort)
    {
        m_joysticks[ucPort].Init(ucPort);
    }

    if (!gDisableLCD)
    {
        ConnectLCDPanel(&gLCDPanels[0], 0);
        OpenLCDPanel(&gLCDPanels[0]);
    }

    RedKeyboard::StaticInit();
    m_keyboards[0].Init(0);
    m_keyboards[0].Insert();
    m_mouses[0].Init(0);
    m_mouses[0].Insert();

    m_numJoysticks = 0;
    if (!isReadingJournal)
    {

        m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, pcEnumJoystickDevice, 0,
                           DIEDFL_ATTACHEDONLY);
    }
    else
    {

        bool bMoreJoysticks = false;
        while (bMoreJoysticks)
        {
            if (m_joysticks[m_numJoysticks].pcCreateDevice(0))
            {
                ++m_numJoysticks;
            }
        }
    }
}

// FUNCTION: SWBF2 0x006dc290
IDirectInput8A* pcGetDIInterface()
{
    return m_pDI;
}

// FUNCTION: SWBF2 0x006dc250
BOOL CALLBACK pcEnumJoystickDevice(DIDEVICEINSTANCEA const* pInstance, void*  )
{

    if (m_joysticks[m_numJoysticks].pcCreateDevice(pInstance))
    {
        ++m_numJoysticks;
    }

    return m_numJoysticks < 10 ? DIENUM_CONTINUE : DIENUM_STOP;
}

// FUNCTION: SWBF2 0x006dbee0
unsigned int GetNumJoystickPorts()
{
    return m_numJoysticks;
}

// FUNCTION: SWBF2 0x006dbe50
RedJoystick* GetJoystick(unsigned char ucPort)
{
    return &m_joysticks[ucPort];
}

// FUNCTION: SWBF2 0x006dbe70
RedKeyboard* GetKeyboard(unsigned char ucPort)
{
    return &m_keyboards[ucPort];
}

// FUNCTION: SWBF2 0x006dbe90
RedLogitechLCD* GetLCDPanel(unsigned char ucPort)
{
    if (gDisableLCD)
    {
        return 0;
    }

    return &gLCDPanels[ucPort];
}

// FUNCTION: SWBF2 0x006dbc30
void Cleanup()
{
    for (unsigned char ucPort = 0; ucPort < 10; ++ucPort)
    {
        m_joysticks[ucPort].Remove();
        m_joysticks[ucPort].Cleanup();
    }

    m_keyboards[0].Remove();
    m_keyboards[0].Cleanup();
    m_mouses[0].Remove();
    m_mouses[0].Cleanup();

    if (!gDisableLCD)
    {
        CloseLCDPanel(&gLCDPanels[0]);
        DisconnectLCDPanel(&gLCDPanels[0]);
    }

    m_numJoysticks = 0;
    if (m_pDI)
    {
        if (!isReadingJournal)
        {
            m_pDI->Release();
        }
        m_pDI = 0;
    }
}

// FUNCTION: SWBF2 0x006dbef0
unsigned int GetNumKeybaordPorts()
{
    return 1;
}

// FUNCTION: SWBF2 0x006dbf00
unsigned int GetNumLCDPorts()
{
    return 1;
}

// FUNCTION: SWBF2 0x006dbf10
unsigned int GetNumMousePorts()
{
    return 1;
}

}

// GLOBAL: SWBF2 0x0099cd00

bool gDisableLCD;

// FUNCTION: SWBF2 0x006dc1c0
void RedInputManager::Update(float fDeltaTime)
{
    for (int i = 0; i < 10; i++)
    {
        m_joysticks[i].Update(fDeltaTime);
    }
    m_keyboards[0].Update(fDeltaTime);
    m_mouses[0].Update(fDeltaTime);
    if (!gDisableLCD)
    {
        UpdateLCDPanel(&gLCDPanels[0], fDeltaTime);
    }
}

// LIBRARY: SWBF2 0x00754920
// DirectInput8Create

// FUNCTION: SWBF2 0x006f00d0
void DisconnectLCDPanel(RedLogitechLCD* pThis)
{
    if (pThis->mConnected)
    {
        lgLcdDisconnect(pThis->mConnectCtx.connection);
    }
    lgLcdDeInit();
    pThis->mConnected = false;
}

// FUNCTION: SWBF2 0x006f0620
void CloseLCDPanel(RedLogitechLCD* pThis)
{
    if (pThis->mpBitmap)
    {
        delete pThis->mpBitmap;
        pThis->mpBitmap = 0;
    }
    if (pThis->mhBitmap)
    {
        DeleteObject(pThis->mhBitmap);
        pThis->mhBitmap = 0;
        pThis->mpBits = 0;
    }
    if (pThis->mpBitmapInfo)
    {
        delete[] (BYTE*)pThis->mpBitmapInfo;
        pThis->mpBitmapInfo = 0;
    }
    if (pThis->mhDC)
    {
        DeleteDC(pThis->mhDC);
        pThis->mhDC = 0;
    }
    if (pThis->mhFont)
    {
        DeleteObject(pThis->mhFont);
        pThis->mhFont = 0;
    }
    pThis->mhOldBitmap = 0;
    if (pThis->mOpened && pThis->mConnected)
    {
        lgLcdClose(pThis->mOpenCtx.device);
        pThis->mOpened = false;
    }
}
