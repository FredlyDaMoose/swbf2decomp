#include <math.h>
#include <string.h>

#include "pcRedInputManager.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

extern HWND hGlobalWnd;

BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCEA const* pObject, void* pvRef);

// FUNCTION: SWBF2 0x006c54a0
void RedJoystick::Insert()
{
    if (m_connected)
    {
        return;
    }
    memset(&pc_state, 0, sizeof(pc_state));
    pc_device->Unacquire();
    pc_device->SetCooperativeLevel(hGlobalWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    HRESULT result;
    do
    {
        result = pc_device->Acquire();
    } while (result == DIERR_INPUTLOST);
    m_connected = result == DI_OK;
}

// FUNCTION: SWBF2 0x006c5540
void RedJoystick::SetHeavyEngine(float f)
{
    (void)f;
}

// FUNCTION: SWBF2 0x006c5550
void RedJoystick::SetLightEngine(float f)
{
    (void)f;
}

// FUNCTION: SWBF2 0x006c5470
void RedJoystick::Init(unsigned char ucPortNum)
{

    pc_device = 0;
    m_portNum = ucPortNum;
    m_connected = false;
    mPreviousAxisX = 1000.0f;
    mPreviousAxisY = 1000.0f;
}

// FUNCTION: SWBF2 0x006c5a60
bool RedJoystick::pcCreateDevice(DIDEVICEINSTANCEA const* pInstance)
{

    mPreviousAxisX = 1000.0f;
    mPreviousAxisY = 1000.0f;

    IDirectInput8A* pDI = RedInputManager::pcGetDIInterface();

    if (pc_device)
    {
        return false;
    }

    if (pDI->CreateDevice(pInstance->guidInstance, &pc_device, 0) != DI_OK)
    {
        return false;
    }

    if (SUCCEEDED(pc_device->SetDataFormat(&c_dfDIJoystick2))
        && SUCCEEDED(pc_device->SetCooperativeLevel(hGlobalWnd,
                                                    DISCL_EXCLUSIVE | DISCL_FOREGROUND))
        && SUCCEEDED(pc_device->EnumObjects(EnumObjectsCallback, this, DIDFT_ALL)))
    {
        return true;
    }

    pc_device->Release();
    pc_device = 0;
    return false;
}

// FUNCTION: SWBF2 0x006c53a0
BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCEA const* pObject, void* pvRef)
{
    RedJoystick* pJoystick = (RedJoystick*)pvRef;

    if (pObject->dwType & DIDFT_AXIS)
    {

        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwObj = pObject->dwType;
        diprg.diph.dwHow = DIPH_BYID;
        diprg.lMin = -16384;
        diprg.lMax = 16384;
        pJoystick->pc_device->SetProperty(DIPROP_RANGE, &diprg.diph);

        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = pObject->dwType;
        dipdw.diph.dwHow = DIPH_BYID;
        dipdw.dwData = (DWORD)(pJoystick->mDeadZoneScale * 16384.0f);
        pJoystick->pc_device->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

        pJoystick->pc_device->SetProperty(DIPROP_RANGE, &diprg.diph);
    }

    return DIENUM_CONTINUE;
}

// FUNCTION: SWBF2 0x006c5310
RedJoystick::RedJoystick()
{
    mDeadZoneScale = 0.2f;
}

// FUNCTION: SWBF2 0x006c5330
RedJoystick::~RedJoystick()
{
}

namespace FLRenderer
{
void SetCursorPos(float fX, float fY);
}

// FUNCTION: SWBF2 0x006c5560
void RedJoystick::Update(float fDeltaTime)
{
    if (!pc_device)
    {
        return;
    }

    HRESULT hr = pc_device->Poll();
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
    {
        Remove();
        Insert();
        if (FAILED(pc_device->Poll()))
        {
            return;
        }
    }

    if (FAILED(pc_device->GetDeviceState(sizeof(DIJOYSTATE2), &pc_state)))
    {
        return;
    }

    unsigned __int64 uiOldButtons = m_uiDownButtons;
    m_uiDownButtons = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (pc_state.rgbButtons[i] & 0x80)
        {
            m_uiDownButtons |= 1 << i;
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        int iBit = 32 + i * 4;
        switch (pc_state.rgdwPOV[i])
        {
        case 0:     m_uiDownButtons |= (unsigned __int64)1 << iBit; break;
        case 4500:  m_uiDownButtons |= ((unsigned __int64)1 << iBit) | ((unsigned __int64)1 << (iBit + 1)); break;
        case 9000:  m_uiDownButtons |= (unsigned __int64)1 << (iBit + 1); break;
        case 13500: m_uiDownButtons |= ((unsigned __int64)1 << (iBit + 1)) | ((unsigned __int64)1 << (iBit + 2)); break;
        case 18000: m_uiDownButtons |= (unsigned __int64)1 << (iBit + 2); break;
        case 22500: m_uiDownButtons |= ((unsigned __int64)1 << (iBit + 2)) | ((unsigned __int64)1 << (iBit + 3)); break;
        case 27000: m_uiDownButtons |= (unsigned __int64)1 << (iBit + 3); break;
        case 31500: m_uiDownButtons |= ((unsigned __int64)1 << iBit) | ((unsigned __int64)1 << (iBit + 3)); break;
        }
    }

    unsigned __int64 uiChanged = uiOldButtons ^ m_uiDownButtons;
    m_uiPressedButtons = m_uiDownButtons & uiChanged;
    m_uiReleasedButtons = uiOldButtons & uiChanged;

    const float fScale = 1.0f / 16384.0f;
    m_fAxis[0] = pc_state.lX * fScale;
    m_fAxis[1] = pc_state.lY * fScale;
    m_fAxis[2] = pc_state.lZ * fScale;
    m_fAxis[3] = pc_state.lRx * fScale;
    m_fAxis[4] = pc_state.lRy * fScale;
    m_fAxis[5] = pc_state.lRz * fScale;
    m_fAxis[6] = pc_state.rglSlider[0] * fScale;
    m_fAxis[7] = pc_state.rglSlider[1] * fScale;

    RedMouse* pMouse = RedInputManager::GetMouse(0);
    if (!pMouse)
    {
        return;
    }

    float fX = pc_state.lX * fScale;
    float fY = pc_state.lY * fScale;
    if (mPreviousAxisX > 10.0f)
    {
        mPreviousAxisX = fX;
    }
    if (mPreviousAxisY > 10.0f)
    {
        mPreviousAxisY = fY;
    }

    if ((fabsf(fX) > 0.4f && mPreviousAxisX != 0.0f && fabsf(fX - mPreviousAxisX) > 0.03f)
        || (fabsf(fY) > 0.4f && mPreviousAxisY != 0.0f && fabsf(fY - mPreviousAxisY) > 0.03f))
    {
        pMouse->pCursorImage = this;
    }

    if (pMouse->pCursorImage == this)
    {
        int iDX = (int)(fX * fDeltaTime * 512.0f);
        pMouse->m_X += iDX;
        pMouse->m_dX = iDX;
        int iDY = (int)(fY * fDeltaTime * 512.0f);
        pMouse->m_Y += iDY;
        pMouse->m_dY = iDY;

        int iX = pMouse->m_X;
        if (iX < 0)
        {
            iX = 0;
        }
        else if (iX > (int)RedInterfaceScreen::GetScreenWidth())
        {
            iX = RedInterfaceScreen::GetScreenWidth();
        }
        pMouse->m_X = iX;

        int iY = pMouse->m_Y;
        if (iY < 0)
        {
            iY = 0;
        }
        else if (iY > (int)RedInterfaceScreen::GetScreenHeight())
        {
            iY = RedInterfaceScreen::GetScreenHeight();
        }
        pMouse->m_Y = iY;

        FLRenderer::SetCursorPos((float)iX, (float)iY);
    }

    mPreviousAxisX = fX;
    mPreviousAxisY = fY;
}

// FUNCTION: SWBF2 0x006c5370
void RedJoystick::Cleanup()
{
    if (pc_device)
    {
        pc_device->Unacquire();
        pc_device->Release();
        pc_device = 0;
    }
    m_connected = false;
}

// FUNCTION: SWBF2 0x006c5510
void RedJoystick::Remove()
{
    if (pc_device)
    {
        pc_device->Unacquire();
        pc_device->SetCooperativeLevel(hGlobalWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    }
    m_connected = false;
}
