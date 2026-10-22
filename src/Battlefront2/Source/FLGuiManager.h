#pragma once

#include <stddef.h>

#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/input/RedInputEventHandler.h"
#include "FLList.h"

class GUIInputs;
class IFScreen;

struct KeyStroke
{
    unsigned char key;
    unsigned char ascii;
    unsigned short flags;
};

class KeyGrabber : public RedInputEventHandler
{
public:

    KeyGrabber() : mInited(false) {}

    virtual ~KeyGrabber() {}

    // SYNTHETIC: SWBF2 0x00527ff0
    // KeyGrabber::`scalar deleting destructor'

    void Init()
    {
        RedInputEventHandler::AddEventHandler(this);
        mInited = true;
        mStroke.key = 0;
    }

    bool IsInited() { return mInited; }

    KeyStroke GetStroke()
    {
        KeyStroke stroke = mStroke;
        *(unsigned int*)&mStroke = 0;
        return stroke;
    }

private:

    virtual void KeyboardEvent(RedInputEventType eType, RedKeyboard* pKeyboard,
                               unsigned char ucKey, unsigned char ucAscii,
                               unsigned short usFlags);

public:
    KeyStroke mStroke;
    bool mInited;
};
 
 
 

class GuiManager
{
     

public:

    enum eEventType
    {
        eEventType_Input = 0,
        eEventType_LifeCycle = 1,
        eEventType_LifeCycle_Init = 1,
        eEventType_LifeCycle_Exit = 2,
        eEventType_LifeCycle_Enter = 3,
        eEventType_LifeCycle_Cleanup = 4,
        eEventType_Update = 5,
        eEventType_Num = 6
    };

    enum eLifeCycle
    {
        eLifeCycle_Void = 0,
        eLifeCycle_Initialized = 1,
        eLifeCycle_Entered = 2,
        eLifeCycle_Changing = 3,
        eLifeCycle_Exited = 4,
        eLifeCycle_Cleaned = 5
    };

    enum eErrorSeverity
    {
        ERROR_NONE = 0,
        OLD_ERROR_PAD_MISSING = 1,
        ERROR_WARNING = 2,
        ERROR_DELAY = 3,
        ERROR_SUBSCREEN = 4,
        ERROR_IGNORE_OR_REBOOT = 5,
        ERROR_LEAVE_SESSION = 6,
        ERROR_LOGOFF_SECONDARY = 7,
        ERROR_LOGOFF = 8,
        ERROR_LOGOFF_CABLE = 9,
        ERROR_LOGOFF_OR_REBOOT = 10,
        ERROR_REBOOT = 11,
        ERROR_MISSING_DC = 12,
        ERROR_PAD_MISSING = 13,
        ERROR_MAX = 14
    };

    enum eInputCode
    {
        eInputCode_Accept = 0,
        eInputCode_Back = 1,
        eInputCode_Misc = 2,
        eInputCode_Misc2 = 3,
        eInputCode_Select = 4,
        eInputCode_Start = 5,
        eInputCode_Action1 = 6,
        eInputCode_Action2 = 7,
        eInputCode_Action3 = 8,
        eInputCode_Action4 = 9,
        eInputCode_LTrigger = 10,
        eInputCode_RTrigger = 11,
        eInputCode_LTrigger2 = 12,
        eInputCode_RTrigger2 = 13,
        eInputCode_LTrigger3 = 14,
        eInputCode_RTrigger3 = 15,
        eInputCode_LAJoyUp = 16,
        eInputCode_LAJoyRight = 17,
        eInputCode_LAJoyDown = 18,
        eInputCode_LAJoyLeft = 19,
        eInputCode_RAJoyUp = 20,
        eInputCode_RAJoyRight = 21,
        eInputCode_RAJoyDown = 22,
        eInputCode_RAJoyLeft = 23,
        eInputCode_DPadUp = 24,
        eInputCode_DPadRight = 25,
        eInputCode_DPadDown = 26,
        eInputCode_DPadLeft = 27,
        eInputCode_GeneralUp = 28,
        eInputCode_GeneralRight = 29,
        eInputCode_GeneralDown = 30,
        eInputCode_GeneralLeft = 31,
        eInputCode_Key = 32,
        eInputCode_Num = 33,
        eInputCode_Coords = 34
    };

    struct InputEventFunction
    {
        eEventType type;
        char* luaCallbackFunction;
    };
    struct InputCodeFunctionExt
    {
        eInputCode code;
        char* luaCallbackFunctionExt;
    };

    struct ErrorLevelInfo
    {
        unsigned short* pMessage;
        bool (*pfnIsResolved)();
        void (*pfnErrorBoxOk)(bool);
    };

    struct Event
    {

        Event() {}

        Event(eEventType eType, unsigned int uiVal, const char* pcName, int iController);

        ~Event() {}

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
        static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

        eEventType type;
        union
        {
            float f32;
            unsigned int i32;
        } val;
        int mController;
        char pName[64];

        static MemoryPool sMemoryPool;
    };

    GuiManager();
    ~GuiManager();

    static void InitAll();
    static void ResetAll();
    void Reset();
    static void UpdateAll(float fElapsed);
    void Update(float fElapsed);

    // FUNCTION: SWBF2 0x00527f50
    static GuiManager* Instance() { return Instance(mCurViewport); }

    static GuiManager* Instance(unsigned int uViewport)
    {
        return uViewport < 1 ? &mInstances[uViewport] : 0;
    }

    void ChangeToScreen(const char* pScreen, bool bArg2, bool bExitArg);
    void PushScreen(const char* pScreen, int iJoystick, int iArg3);
    void PopScreen(const char* pScreen);
    void ClearScreenStack();
    bool IsScreenInStack(const char* pScreen);
    void OpenPopup(const char* pPopup);
    void ClosePopup(bool bClearLua);
    bool OpenError(eErrorSeverity eSeverity, const unsigned short* pMessage,
                   bool (*pfnIsResolved)(), void (*pfnErrorBoxOk)(bool));
    void CloseError(bool bOk, bool bArg2);
    const char* GetCurrentFocus() const;

    // FUNCTION: SWBF2 0x004294e0
    const char* GetCurrentScreen() const { return mCurrentScope; }
    // FUNCTION: SWBF2 0x00527f70
    bool IsPopupOpen() const { return mCurrentPopup[0] != 0; }

    bool IsErrorOpen() const { return mCurrentError[0] != 0; }
    bool CallLuaFunctionOfScope(const char* pcFunc, bool bPopupOnly);
    bool CallLuaFunctionOfScope(const char* pcFunc, float fArg, bool bPopupOnly);
    bool CallLuaFunctionOfScope(const char* pcFunc, const unsigned short* pusArg,
                                bool bPopupOnly);
    static void UpdateCursor();
    void CreateKeyInputEvent(int iController, int iKey);
    static void DebounceJoystick(int iJoystick);
    static int GetJoystickFromViewport(unsigned int uViewport);
    static void SetInputRepeat(bool bRepeat);

private:
    void _ExitScreen(const char* pScreen, bool bArg);
    void _EnterScreen(const char* pScreen, bool bArg);
    void SetCurrentScreen(const char* pScreen, bool bExitArg, bool bEnterArg);
    void HandleEvents();
    void CreateInputEvent(int iCode, int iController);
    void CollectInputs(int iJoystick, GUIInputs* pInputs, int iController,
                       float fElapsed);
    static void DebounceOneJoystick(int iJoystick);

public:

    static GuiManager* mInstances;

    static InputEventFunction EventToLuaFunction[eEventType_Num];
    static InputCodeFunctionExt InputCodeToLuaFunctionExt[eInputCode_Num];
    static int mMouseX;
    static int mMouseY;
    static unsigned int mCurViewport;
    static int sStartupDelay;
    static bool m_InputTriggered[1][26];

    FLList<Event*> mEventQueue;
    char mCurrentScope[64];
    char mCurrentPopup[64];
    char mCurrentError[64];
    eLifeCycle mCurrentGuiLifeCycle;
    IFScreen* mScreenStack[32];
    int mScreenStackPos;
    float m_fNextUpTrigger;
    float m_fNextDownTrigger;
    float m_fNextLeftTrigger;
    float m_fNextRightTrigger;
    int mJoystickIndex;
    int mViewport;
    eErrorSeverity mErrorLevel;
    bool (*mfnIsErrorResolved)();
    void (*mfnOnErrorBoxOk)(bool);
    ErrorLevelInfo mErrorLevelInfo[ERROR_MAX];
    bool mbInitalized;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
