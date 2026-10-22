#pragma once

#include <stddef.h>

struct CntBitFields
{
    unsigned int mBoolInputs : 30;
    unsigned int mMouseMode : 2;
};
 

enum ePROCESSEDINPUT_TYPE
{
    ePROCESSEDINPUT_NONE = -1,
    ePROCESSEDINPUT_primaryFireButtonDown = 0,
    ePROCESSEDINPUT_secondaryFireButtonDown = 1,
    ePROCESSEDINPUT_sprintButtonDown = 2,
    ePROCESSEDINPUT_jumpButtonDown = 3,
    ePROCESSEDINPUT_crouchButtonDown = 4,
    ePROCESSEDINPUT_zoomButtonDown = 5,
    ePROCESSEDINPUT_viewButtonDown = 6,
    ePROCESSEDINPUT_reloadButtonDown = 7,
    ePROCESSEDINPUT_useButtonPressed = 8,
    ePROCESSEDINPUT_squadCommandButtonPressed = 9,
    ePROCESSEDINPUT_acceptHeroPressed = 10,
    ePROCESSEDINPUT_declineHeroPressed = 11,
    ePROCESSEDINPUT_lockTargetButtonPressed = 12,
    ePROCESSEDINPUT_primaryNextButtonPressed = 13,
    ePROCESSEDINPUT_primaryPrevButtonPressed = 14,
    ePROCESSEDINPUT_secondaryNextButtonPressed = 15,
    ePROCESSEDINPUT_secondaryPrevButtonPressed = 16,
    ePROCESSEDINPUT_playerList = 17,
    ePROCESSEDINPUT_chat = 18,
    ePROCESSEDINPUT_friendChat = 19,
    ePROCESSEDINPUT_Comm_Chat_0 = 20,
    ePROCESSEDINPUT_Comm_Chat_1 = 21,
    ePROCESSEDINPUT_Comm_Chat_2 = 22,
    ePROCESSEDINPUT_Comm_Chat_3 = 23,
    ePROCESSEDINPUT_Comm_Chat_4 = 24,
    ePROCESSEDINPUT_Comm_Chat_5 = 25,
    ePROCESSEDINPUT_Comm_Chat_6 = 26,
    ePROCESSEDINPUT_Comm_Chat_7 = 27,
    ePROCESSEDINPUT_Map = 28,
    ePROCESSEDINPUT_TakeScreenshot = 29,
    ePROCESSEDINPUT_BOOLCOUNT = 30,
    ePROCESSEDINPUT_STRAFE_AXIS = 30,
    ePROCESSEDINPUT_MOVE_AXIS = 31,
    ePROCESSEDINPUT_TURN_AXIS = 32,
    ePROCESSEDINPUT_PITCH_AXIS = 33,
    ePROCESSEDINPUT_MAX = 34,
    ePROCESSEDINPUT_FLOATCOUNT = 4,
    ePROCESSEDINPUT_STRAFE_POS = 35,
    ePROCESSEDINPUT_STRAFE_NEG = 36,
    ePROCESSEDINPUT_MOVE_POS = 37,
    ePROCESSEDINPUT_MOVE_NEG = 38,
    ePROCESSEDINPUT_TURN_POS = 39,
    ePROCESSEDINPUT_TURN_NEG = 40,
    ePROCESSEDINPUT_PITCH_POS = 41,
    ePROCESSEDINPUT_PITCH_NEG = 42,
    ePROCESSEDINPUT_NUM_KEY_ACTIONS = 43,
    ePROCESSEDINPUT_walkKeyPressed = 43
};

class ProcessedInputs
{
public:
    ProcessedInputs();

    // FUNCTION: SWBF2 0x00529ad0
    ~ProcessedInputs() {}

    void Clear();

    bool GetDigital(ePROCESSEDINPUT_TYPE eType) const;

    void Set(ePROCESSEDINPUT_TYPE eType, bool& bLocked, float fValue, float fLastValue);

    float GetStrafeAxis() const { return mFloatInputs[0]; }
    float GetMoveAxis() const { return mFloatInputs[1]; }

    float GetTurnAxis() const { return mFloatInputs[2]; }
    float GetPitchAxis() const { return mFloatInputs[3]; }

    // FUNCTION: SWBF2 0x005b4010
    bool IsPrimaryFireButtonDown() const { return (bits.mBoolInputs & 0x1) != 0; }
    bool IsSecondaryFireButtonDown() const { return (bits.mBoolInputs & 0x2) != 0; }
    bool IsSprintButtonDown() const { return (bits.mBoolInputs & 0x4) != 0; }

    // FUNCTION: SWBF2 0x005b4090
    bool IsJumpButtonDown() const { return (bits.mBoolInputs & 0x8) != 0; }
    bool IsCrouchButtonDown() const { return (bits.mBoolInputs & 0x10) != 0; }
    bool IsZoomButtonDown() const { return (bits.mBoolInputs & 0x20) != 0; }
    bool IsViewButtonDown() const { return (bits.mBoolInputs & 0x40) != 0; }

    // FUNCTION: SWBF2 0x005b4050
    bool IsReloadButtonDown() const { return (bits.mBoolInputs & 0x80) != 0; }
    bool IsUseButtonPressed() const { return (bits.mBoolInputs & 0x100) != 0; }
    bool IsSquadCommandButtonDown() const { return (bits.mBoolInputs & 0x200) != 0; }
    bool IsAcceptHeroPressed() const { return (bits.mBoolInputs & 0x400) != 0; }
    bool IsDeclineHeroPressed() const { return (bits.mBoolInputs & 0x800) != 0; }
    bool IsLockTargetButtonPressed() const { return (bits.mBoolInputs & 0x1000) != 0; }

    float IsPrimarySwitchButtonPressed() const;
    float IsSecondarySwitchButtonPressed() const;

    float mFloatInputs[4];
    CntBitFields bits;
    bool mJoyUsage;
};
 
 
 

enum eGUIINPUT_TYPE
{
    eGUIINPUT_NONE = -1,
    eGUIINPUT_Accept = 0,
    eGUIINPUT_Back = 1,
    eGUIINPUT_Start = 2,
    eGUIINPUT_Select = 3,
    eGUIINPUT_Misc1 = 4,
    eGUIINPUT_Misc2 = 5,
    eGUIINPUT_Up = 6,
    eGUIINPUT_Down = 7,
    eGUIINPUT_Left = 8,
    eGUIINPUT_Right = 9,
    eGUIINPUT_LeftTrigger = 10,
    eGUIINPUT_RightTrigger = 11,
    eGUIINPUT_LeftTrigger2 = 12,
    eGUIINPUT_RightTrigger2 = 13,
    eGUIINPUT_LeftTrigger3 = 14,
    eGUIINPUT_RightTrigger3 = 15,
    eGUIINPUT_yAxis = 16,
    eGUIINPUT_xAxis = 17,
    eGUIINPUT_FreeEyeUp = 18,
    eGUIINPUT_FreeEyeDown = 19,
    eGUIINPUT_FreeEyeReset = 20,
    eGUIINPUT_FindPlayer = 21,
    eGUIINPUT_FreeEyeMoveFaster = 22,
    eGUIINPUT_FreeEyeTurnFaster = 23,
    eGUIINPUT_FreeEyeMoveSlower = 24,
    eGUIINPUT_FreeEyeTurnSlower = 25,
    eGUIINPUT_MAX = 26
};

struct MouseCoords
{
    int mPosX;
    int mPosY;
};

class GUIInputs
{
public:

    GUIInputs();
    ~GUIInputs();
    void Process();

    void Clear();

    void Set(eGUIINPUT_TYPE eType, float fValue);

    bool IsLeftPressed() const;
    bool IsRightPressed() const;

    bool IsStartPressed() const
    {
        return mProcessedGUIInputs[eGUIINPUT_Start] >= 0.125f && mLastGUIInputs[eGUIINPUT_Start] < 0.125f;
    }
    bool IsSelectPressed() const { return mProcessedGUIInputs[eGUIINPUT_Select] >= 0.125f; }

    bool IsAcceptPressed() const { return mProcessedGUIInputs[eGUIINPUT_Accept] >= 0.125f; }
    bool IsBackPressed() const { return mProcessedGUIInputs[eGUIINPUT_Back] >= 0.125f; }
    void ClearStart()
    {
        mGUIInputs[eGUIINPUT_Start] = 1.0f;
        mLastGUIInputs[eGUIINPUT_Start] = 1.0f;
    }

    bool IsDPadUpPressed() const { return mProcessedGUIInputs[eGUIINPUT_Up] >= 0.125f; }
    bool IsDPadDownPressed() const { return mProcessedGUIInputs[eGUIINPUT_Down] >= 0.125f; }
    bool IsMisc1Pressed() const { return mProcessedGUIInputs[eGUIINPUT_Misc1] >= 0.125f; }
    bool IsMisc2Pressed() const { return mProcessedGUIInputs[eGUIINPUT_Misc2] >= 0.125f; }
    bool IsLeftTriggerPressed() const { return mProcessedGUIInputs[eGUIINPUT_LeftTrigger] >= 0.125f; }
    bool IsRightTriggerPressed() const { return mProcessedGUIInputs[eGUIINPUT_RightTrigger] >= 0.125f; }
    bool IsLeftTrigger2Pressed() const { return mProcessedGUIInputs[eGUIINPUT_LeftTrigger2] >= 0.125f; }
    bool IsRightTrigger2Pressed() const { return mProcessedGUIInputs[eGUIINPUT_RightTrigger2] >= 0.125f; }
    bool IsLeftTrigger3Pressed() const { return mProcessedGUIInputs[eGUIINPUT_LeftTrigger3] >= 0.125f; }
    bool IsRightTrigger3Pressed() const { return mProcessedGUIInputs[eGUIINPUT_RightTrigger3] >= 0.125f; }

    bool IsFreeEyeUpButtonDown() const { return mProcessedGUIInputs[eGUIINPUT_FreeEyeUp] >= 0.125f; }
    bool IsFreeEyeDownButtonDown() const { return mProcessedGUIInputs[eGUIINPUT_FreeEyeDown] >= 0.125f; }
    bool IsFreeEyeMoveFasterButtonDown() const { return mProcessedGUIInputs[eGUIINPUT_FreeEyeMoveFaster] >= 0.125f; }
    bool IsFreeEyeTurnFasterButtonDown() const { return mProcessedGUIInputs[eGUIINPUT_FreeEyeTurnFaster] >= 0.125f; }
    bool IsFreeEyeMoveSlowerButtonDown() const { return mProcessedGUIInputs[eGUIINPUT_FreeEyeMoveSlower] >= 0.125f; }
    bool IsFreeEyeTurnSlowerButtonDown() const { return mProcessedGUIInputs[eGUIINPUT_FreeEyeTurnSlower] >= 0.125f; }

    float mGUIInputs[eGUIINPUT_MAX];
    float mProcessedGUIInputs[eGUIINPUT_MAX];
    float mLastGUIInputs[eGUIINPUT_MAX];
    MouseCoords mMouseCoords;
};
 
 

class RedJoystick;

class Rumble
{
public:

    Rumble() : mRumbleMode(eNone), mJoystick(0), mHeavyDecay(0), mLightDecay(0),
        mLight(0), mHeavy(0), mInterval(0), mEnabled(true) {}

    enum RumbleMode
    {
        eNone = 0,
        eRumble = 1,
        eIntermittent = 2
    };

    void StopAllRumble();

    void StopIntermittent();

    void Update(float fDeltaTime);

    RumbleMode mRumbleMode;
    RedJoystick* mJoystick;
    float mHeavyDecay;
    float mLightDecay;
    float mLight;
    float mHeavy;
    float mDelayLight;
    float mDelayHeavy;
    float mInterval;
    float mTime;
    float mTimeLeftHeavy;
    float mTimeLeftLight;
    bool mEnabled;
};
 
 
 
