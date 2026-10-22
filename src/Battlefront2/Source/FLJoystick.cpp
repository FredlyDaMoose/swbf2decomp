#include "FLJoystick.h"

#include <math.h>
#include <string.h>

#include "FLInputManager.h"
#include "LuaHelper.h"
#include "RedEngineFL/input/RedJoystick.h"
#include "GameLoop.h"

extern bool gAttractMode;

// FUNCTION: SWBF2 0x0052b490
void RawControllerInputs::ClearInputsToOff()
{
    const float* pRaw = (const float*)&mRawInputStorage[0x14 - 4];
    bool* pLocked = (bool*)&mRawInputStorage[0x17d4 - 4];
    for (int i = 0; i < 10 * 76; i += 76)
    {
        for (int j = 0; j < 76; j++)
        {
            if ((j >= 66 && j <= 75) || j <= 4)
            {
                pLocked[i + j] = (float)fabs(pRaw[i + j]) >= 0.125f;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0052b620
ProcessedInputs::ProcessedInputs()
{
    for (int i = 0; i < 4; i++)
    {
        mFloatInputs[i] = 0.0f;
    }
    bits.mBoolInputs = 0;
    bits.mMouseMode = 2;
}

// FUNCTION: SWBF2 0x0052b880
bool ProcessedInputs::GetDigital(ePROCESSEDINPUT_TYPE eType) const
{
    return (bits.mBoolInputs & (1 << eType)) != 0;
}

// FUNCTION: SWBF2 0x0052b8b0
void ProcessedInputs::Clear()
{
    bits.mBoolInputs = 0;
    for (int i = 0; i < 4; i++)
    {
        mFloatInputs[i] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0052b920
void GUIInputs::Clear()
{
    memcpy(mLastGUIInputs, mGUIInputs, sizeof(mGUIInputs));
    memset(mGUIInputs, 0, sizeof(mGUIInputs));
    memset(mProcessedGUIInputs, 0, sizeof(mProcessedGUIInputs));
}

// FUNCTION: SWBF2 0x0052b9a0
void GUIInputs::Process()
{
    for (int i = 0; i < eGUIINPUT_MAX; i++)
    {
        float fAbs = fabsf(mGUIInputs[i]);
        bool bRepeat = false;
        switch (i)
        {
        case eGUIINPUT_Up:
        case eGUIINPUT_Down:
        case eGUIINPUT_Left:
        case eGUIINPUT_Right:
        case eGUIINPUT_yAxis:
        case eGUIINPUT_xAxis:
            bRepeat = true;
            break;
        default:
            break;
        }

        lua_pushstring(LuaHelper::mState, "gbWantMousedownEvents");
        lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
        bool bSend = bRepeat;
        if (!lua_isnil(LuaHelper::mState, -1))
        {
            bSend = true;
        }
        lua_pop(LuaHelper::mState, 1);

        if (i != eGUIINPUT_FreeEyeUp && i != eGUIINPUT_FreeEyeDown)
        {
            if (mGUIInputs[i] == 0.0f)
            {
                continue;
            }
            if (!bSend && mGUIInputs[i] == mLastGUIInputs[i])
            {
                continue;
            }
            if (i == eGUIINPUT_xAxis || i == eGUIINPUT_yAxis)
            {
                mProcessedGUIInputs[i] = mGUIInputs[i];
                continue;
            }
        }
        mProcessedGUIInputs[i] = fAbs > mGUIInputs[i] ? fAbs : mGUIInputs[i];
    }
}

// FUNCTION: SWBF2 0x0052b8c0
float ProcessedInputs::IsPrimarySwitchButtonPressed() const
{
    if (bits.mBoolInputs & 0x2000)
    {
        return 1.0f;
    }
    if (bits.mBoolInputs & 0x4000)
    {
        return -1.0f;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x0052b8f0
float ProcessedInputs::IsSecondarySwitchButtonPressed() const
{
    if (bits.mBoolInputs & 0x8000)
    {
        return 1.0f;
    }
    if (bits.mBoolInputs & 0x10000)
    {
        return -1.0f;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x0052b5d0
bool RawControllerInputs::IsDefaultBindings(int iMode)
{
    return memcmp(&mRawInputStorage[0x1acc - 4 + iMode * sizeof(sDefaultBindings[0])],
                  sDefaultBindings[iMode], sizeof(sDefaultBindings[0])) == 0;
}

// FUNCTION: SWBF2 0x0052b510
float RawControllerInputs::AdjustValueForSensitivity(float fValue, unsigned char ucSensitivity)
{
    float fScale = (float)(ucSensitivity + 1) * 0.2f;
    float fDeadZone = 0.85f;
    if (fScale > 1.0f && fScale * fDeadZone > 1.0f)
    {
        fDeadZone = 1.0f / fScale;
    }

    float fMagnitude = (float)fabs(fValue);
    if (fMagnitude >= fDeadZone)
    {
        float fBlend = (fMagnitude - fDeadZone) / (1.0f - fDeadZone);
        if (fBlend < 0.0f)
        {
            fBlend = 0.0f;
        }
        else if (fBlend > 1.0f)
        {
            fBlend = 1.0f;
        }
        fScale = fScale * (1.0f - fBlend) + fBlend;
    }

    float fResult = fScale * fValue;
    if (fResult < -1.0f)
    {
        return -1.0f;
    }
    if (fResult > 1.0f)
    {
        return 1.0f;
    }
    return fResult;
}

// FUNCTION: SWBF2 0x0052bab0
void Rumble::StopIntermittent()
{
    if (mRumbleMode == eIntermittent)
    {
        mRumbleMode = eNone;
    }
}

// FUNCTION: SWBF2 0x0052b640
void ProcessedInputs::Set(ePROCESSEDINPUT_TYPE eType, bool& bLocked, float fValue, float fLastValue)
{
    if (eType == ePROCESSEDINPUT_NONE)
    {
        return;
    }

    if (bLocked == true)
    {
        if (fabs(fValue) < 0.125f)
        {
            bLocked = false;
        }
        else
        {
            fValue = 0.0f;
        }
    }

    if ((unsigned int)(eType - ePROCESSEDINPUT_BOOLCOUNT) <= 3)
    {
        float& fAxis = mFloatInputs[eType - ePROCESSEDINPUT_BOOLCOUNT];
        if (fabsf(fAxis) <= fabsf(fValue))
        {
            fAxis = fValue;
        }
        return;
    }

    switch (eType)
    {
    case ePROCESSEDINPUT_STRAFE_POS:
        if (fValue > fabsf(mFloatInputs[0]))
        {
            mFloatInputs[0] = fValue;
        }
        break;
    case ePROCESSEDINPUT_STRAFE_NEG:
        if (fValue > fabsf(mFloatInputs[0]))
        {
            mFloatInputs[0] = -fValue;
        }
        break;
    case ePROCESSEDINPUT_MOVE_POS:
        if (fValue > fabsf(mFloatInputs[1]))
        {
            mFloatInputs[1] = fValue;
        }
        break;
    case ePROCESSEDINPUT_MOVE_NEG:
        if (fValue > fabsf(mFloatInputs[1]))
        {
            mFloatInputs[1] = -fValue;
        }
        break;
    case ePROCESSEDINPUT_TURN_POS:
        if (fValue > fabsf(mFloatInputs[2]))
        {
            mFloatInputs[2] = fValue;
        }
        break;
    case ePROCESSEDINPUT_TURN_NEG:
        if (fValue > fabsf(mFloatInputs[2]))
        {
            mFloatInputs[2] = -fValue;
        }
        break;
    case ePROCESSEDINPUT_PITCH_POS:
        if (fValue > fabsf(mFloatInputs[3]))
        {
            mFloatInputs[3] = fValue;
        }
        break;
    case ePROCESSEDINPUT_PITCH_NEG:
        if (fValue > fabsf(mFloatInputs[3]))
        {
            mFloatInputs[3] = -fValue;
        }
        break;

    case ePROCESSEDINPUT_useButtonPressed:
    case ePROCESSEDINPUT_primaryNextButtonPressed:
    case ePROCESSEDINPUT_primaryPrevButtonPressed:
    case ePROCESSEDINPUT_secondaryNextButtonPressed:
    case ePROCESSEDINPUT_secondaryPrevButtonPressed:
    case ePROCESSEDINPUT_playerList:
    case ePROCESSEDINPUT_chat:
    case ePROCESSEDINPUT_friendChat:
    case ePROCESSEDINPUT_Comm_Chat_0:
    case ePROCESSEDINPUT_Comm_Chat_1:
    case ePROCESSEDINPUT_Comm_Chat_2:
    case ePROCESSEDINPUT_Comm_Chat_3:
    case ePROCESSEDINPUT_Comm_Chat_4:
    case ePROCESSEDINPUT_Comm_Chat_5:
    case ePROCESSEDINPUT_Comm_Chat_6:
    case ePROCESSEDINPUT_Comm_Chat_7:
    case ePROCESSEDINPUT_Map:
        if (fValue >= 0.125f && fLastValue < 0.125f)
        {
            bits.mBoolInputs |= 1 << eType;
        }
        break;

    default:
        if (fValue >= 0.125f)
        {
            bits.mBoolInputs |= 1 << eType;
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0052b960
void GUIInputs::Set(eGUIINPUT_TYPE eType, float fValue)
{
    if (eType >= 0 && eType < eGUIINPUT_MAX)
    {
        if (eType == eGUIINPUT_xAxis || eType == eGUIINPUT_yAxis ||
            fabsf(fValue) > fabsf(mGUIInputs[eType]))
        {
            mGUIInputs[eType] = fValue;
        }
    }
}

// FUNCTION: SWBF2 0x0052bac0
void Rumble::StopAllRumble()
{
    mRumbleMode = eNone;
    mDelayLight = 0.0f;
    mDelayHeavy = 0.0f;
    mLight = 0.0f;
    mHeavy = 0.0f;
    if (mJoystick)
    {
        mJoystick->SetLightEngine(0.0f);
        mJoystick->SetHeavyEngine(0.0f);
        mJoystick->Update(0.0f);
    }
}

// FUNCTION: SWBF2 0x0052bb20
void Rumble::Update(float fDeltaTime)
{
    if (GameLoop::IsGameOver())
    {
        mDelayLight = 0.0f;
        mDelayHeavy = 0.0f;
        mLight = 0.0f;
        mHeavy = 0.0f;
    }

    if (!mJoystick || !mJoystick->m_connected || !mEnabled || gAttractMode)
    {
        return;
    }

    switch (mRumbleMode)
    {
    case eRumble:
        mDelayLight -= fDeltaTime;
        mDelayHeavy -= fDeltaTime;
        if (mDelayLight <= 0.0f)
        {
            mLight -= mLightDecay * fDeltaTime;
            mTimeLeftLight -= fDeltaTime;
            if (mLight > 0.0f && mTimeLeftLight > 0.0f)
            {
                mJoystick->SetLightEngine(mLight);
            }
        }
        if (mDelayHeavy <= 0.0f)
        {
            mHeavy -= mHeavyDecay * fDeltaTime;
            mTimeLeftHeavy -= fDeltaTime;
            if (mHeavy > 0.0f && mTimeLeftHeavy > 0.0f)
            {
                mJoystick->SetHeavyEngine(mHeavy);
            }
        }
        if ((mLight <= 0.0f && mHeavy <= 0.0f) || (mTimeLeftHeavy <= 0.0f && mTimeLeftLight <= 0.0f))
        {
            mRumbleMode = eNone;
        }
        break;

    case eIntermittent:
        mTime += fDeltaTime;
        if (mTime > mInterval)
        {
            mDelayHeavy -= fDeltaTime;
            mDelayLight -= fDeltaTime;
            if (mDelayLight <= 0.0f)
            {
                mJoystick->SetLightEngine(mLight);
                mDelayLight = mLightDecay;
            }
            if (mDelayHeavy <= 0.0f)
            {
                mJoystick->SetHeavyEngine(mHeavy);
                mDelayHeavy = mHeavyDecay;
            }
            mTime -= mInterval;
        }
        break;

    default:
        mDelayLight = 0.0f;
        mDelayHeavy = 0.0f;
        mLight = 0.0f;
        mHeavy = 0.0f;
        break;
    }
}
