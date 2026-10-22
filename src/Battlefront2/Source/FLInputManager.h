#pragma once

#include <string.h>

#include "FLJoystick.h"
#include "ActionKey.h"
#include "RedEngineFL/input/RedInputEventHandler.h"

struct BindDestination
{
    int mDest;
};

struct ActionKey;

struct UiBindKeys;

class RedKeyboard;
class RedLogitechLCD;

// VTABLE: SWBF2 0x00796504
class RawControllerInputs
{
public:

    void ApplyDeadZone(int iLevel);

    RawControllerInputs();

    // SYNTHETIC: SWBF2 0x00414960
    // RawControllerInputs::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x00414950
    virtual ~RawControllerInputs() {}

    void CloseChatMode();

    void ReadLatest(float fDeltaTime);
    void StandardInputProcess();
    void ChatInputProcess();

    void TeamOrderInputProcess();
    void ChatKeyProcess(unsigned char ucIndex);

    static float AdjustValueForSensitivity(float fValue, unsigned char ucSensitivity);

    static int __fastcall IndexToRedKey(int iIndex);

    RedKeyboard* GetKeyboard() { return _redKeyboard; }

    void ClearInputsToOff();

    int GetFunctionForAnalog(int iAnalog, int iMode);

    bool IsDefaultBindings(int iMode);

    unsigned int GetInputMode() const { return mMode; }
    void SetInputMode(unsigned int uiMode) { mMode = uiMode; }

    bool GetFlip() { return mFlip[GetInputMode()] != 0; }
    bool GetMouseAssist() const { return mMouseAssist[GetInputMode()] != 0; }

    void SetFlip(bool bFlip) { mFlip[GetInputMode()] = bFlip; }
    bool GetFlipForMode(int iMode) const { return mFlip[iMode]; }

    void SetKeyBindingsFromMode(ActionKey* pKeys, unsigned int uiMode)
    {
        memcpy(mCurKeyboardBindings[uiMode], pKeys, 0x102);
    }

    float GetMouseX() const { return mRawInputFloat[0][64]; }
    float GetMouseY() const { return mRawInputFloat[0][65]; }

    static BindDestination sDefaultBindings[5][76];

    static ActionKey sDefaultKeyboardBindings[5][43];

    static UiBindKeys sKeyboardGuiBindings[144];
    static UiBindKeys s_defUIBindings[76];

    union {

    unsigned char mRawInputStorage[0x28a0 - 4];
    struct {

    unsigned char mReservedBeforeMode[4];
    unsigned int mMode;
    bool mFlip[5];
    unsigned char mReservedAfterFlip[3];
    float mRawInputFloat[10][76];
    float mLastRawInputFloat[10][76];
    bool mLockedRawInputBool[10][76];
    BindDestination mCurBindings[5][76];
    ActionKey mCurKeyboardBindings[5][43];
    unsigned char mReservedBeforeGuiInputs[2];
    GUIInputs* m_pGuiInputs;
    bool mProcessAfterReading;
    unsigned char mReservedBeforeProcessDestination[3];
    ProcessedInputs* mProcessDestination;
    union {
        RedJoystick* _redJoystick[11];
        struct {
            RedJoystick* mAttachedJoysticks[10];
    RedLogitechLCD* mLCDPanel;
        };
    };

    bool mScreenshotButtonState[2];
    unsigned char mReservedBeforeMouse[2];
    RedMouse* _redMouse;
    RedKeyboard* _redKeyboard;
    unsigned char mReservedBeforeSensitivity[4];
    unsigned char mMouseSensitivity[5][8];
    unsigned char mJoystickSensitivityX[5];
    unsigned char mJoystickSensitivityY[5];
    bool mMouseAssist[5];
    bool m_CurrKeyState[144];
    bool m_LastKeyState[144];
    unsigned short m_ChatMode;
    unsigned short m_ChatWords[64];
    unsigned char m_ChatLength;
    unsigned char m_KeyDownTime[144];
    unsigned char mReservedBeforeMousePosition[5];
    unsigned char mMouseOldX[8];
    unsigned char mMouseOldY[8];
    float mMouseOldXTimer;
    float mMouseOldYTimer;

    int mLastMouseDeltaX;
    int mLastMouseDeltaY;
    };
    };
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079f1e0
class FLInputManager : public RedInputEventHandler
{
public:

    FLInputManager();

    // SYNTHETIC: SWBF2 0x00529bd0
    // FLInputManager::`scalar deleting destructor'

    virtual ~FLInputManager();

    RawControllerInputs* GetPlayerJoystick(int iPlayer);

    void StopAllRumble();

    int GetPlayerFromHWPort(int iHWPort) const;
    void EnableRumble(int iPort, bool bEnable);

    void StopIntermittentRumble(int iPlayer);

    // FUNCTION: SWBF2 0x004294d0
    static FLInputManager* Instance() { return &s_instance; }

    // FUNCTION: SWBF2 0x00527f30
    int GetPrimaryPort() const { return mPrimaryPort; }
    bool GetReadAll() const { return mReadAllControllers != 0; }
    bool GetAllowBindMultiple() const { return mAllowBindMultiple != 0; }

    int GetSecondaryPort() const { return mSecondaryPort; }
    void SetAllowBindMultiple(bool b) { mAllowBindMultiple = b; }
    void SetBindAll(bool b) { mBindAll = b; }
    bool GetBindAll() const { return mBindAll != 0; }
    void SetAutoAcquire(bool b) { mAutoAcquire = b; }
    void SetIgnoreControllerRemoval(bool b) { mIgnoreMissing = b; }

    void SetReadAll(bool bReadAll);
    void SwapPrimarySecondary();
    void RestorePrimarySecondary();

    void BindPlayerInput();

    bool AnyMissing() const;

    GUIInputs* GetPlayerGuiInputs(int iPlayer);
    ProcessedInputs* GetPlayerInputs(int iPlayer);

    int GetHWPort(int iViewport);
    void SetHotController(int iJoystick);

    static void Reset();
    int GetBindCount() const;
    int CountControllers() const;
    static void Init();
    static void InitCustomFontData();
    void RescanForMissing();

    bool Update(float fDeltaTime);
    bool IsControllerBound(int iPlayer) const;
    void UnbindController(int iPlayer);

    void ProcessChatMessageInput(const unsigned short* pwText);

    bool mLCDPageToggle[1];
    unsigned char mReservedAfterPageToggle;

    unsigned short mChatText[0x200];
    unsigned char mReservedBeforeTextTimer[2];
    float mTextTimer;
    float mPageTimer;
    int mPageIndex;
    unsigned char mReservedBeforeJoysticks[4];

    RawControllerInputs mJoysticks[1];

    ProcessedInputs m_inputs[1];
    Rumble m_rumble[1];
    GUIInputs m_guiInputs[1];

    int m_HWPort[1];
    int m_HWPort_Full[1];
    int mPrimaryPort;
    int mSecondaryPort;
    int mShadowPrimary;
    int mShadowSecondary;
    bool mActiveControllers[1];
    bool mReadAllControllers;
    bool mAllowBindMultiple;
    bool mBindAll;
    bool m_AnyMissing;
    bool m_bPausedByInput;
    bool m_bHudGroupsEnabled[3];
    unsigned char mReservedBeforeMissingPort[3];
    int m_MissingPort;
    bool m_bIsMissing[1];
    bool m_bGameWasPaused;
    bool mAutoAcquire;
    unsigned char mReservedBeforeAskController;
    unsigned short mAskControllerMsg[128];
    bool mIgnoreMissing;

    static FLInputManager s_instance;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

void InitMagicCharMap(void);

unsigned short GetRawFromProcessed_(unsigned int uiAction);

bool CanProcessChatMessage(void);
