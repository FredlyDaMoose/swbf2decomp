#include "FLGuiManager.h"

#include <stdio.h>
#include <string.h>

#include "FLInputManager.h"
#include "FLJoystick.h"
#include "GameLoop.h"
#include "PlayerProfile.h"
#include "GameMemory.h"
#include "GameState.h"
#include "LuaHelper.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedIFScreen.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Memory/RedMemory.h"

namespace FLRenderer
{
void EnableCursor(bool bEnable);
}

namespace GameLoop { extern bool sShowStats; }

static const float kINITIAL_REPEAT_DELAY = 0.65f;
static const float kEXTRA_REPEAT_DELAY = 0.15f;

static char* ERROR_POPUP_NAME = "Popup_Error";

// GLOBAL: SWBF2 0x007e651e
bool g_InputRepeatEnable = true;

// GLOBAL: SWBF2 0x01ebd9b0
// SYNTHETIC: SWBF2 0x00403950
// `dynamic initializer for 'GuiManager::Event::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768760
// `dynamic atexit destructor for 'GuiManager::Event::sMemoryPool''
MemoryPool GuiManager::Event::sMemoryPool("GuiManager::Event", 0x4c);

// GLOBAL: SWBF2 0x01ebda00
// SYNTHETIC: SWBF2 0x00403970
// `dynamic initializer for 'sInstances''
// SYNTHETIC: SWBF2 0x00768770
// `dynamic atexit destructor for 'sInstances''
GuiManager sInstances[1];

// GLOBAL: SWBF2 0x007e6664
GuiManager* GuiManager::mInstances = sInstances;

// GLOBAL: SWBF2 0x007e6690
GuiManager::InputEventFunction GuiManager::EventToLuaFunction[eEventType_Num] = {
    {eEventType_Input, "Input_"},
    {eEventType_LifeCycle_Init, "Init"},
    {eEventType_LifeCycle_Exit, "Exit"},
    {eEventType_LifeCycle_Enter, "Enter"},
    {eEventType_LifeCycle_Cleanup, "Cleanup"},
    {eEventType_Update, "Update"},
};

// GLOBAL: SWBF2 0x007e66c0
GuiManager::InputCodeFunctionExt GuiManager::InputCodeToLuaFunctionExt[eInputCode_Num] = {
    {eInputCode_Accept, "Accept"},
    {eInputCode_Back, "Back"},
    {eInputCode_Misc, "Misc"},
    {eInputCode_Misc2, "Misc2"},
    {eInputCode_Select, "Select"},
    {eInputCode_Start, "Start"},
    {eInputCode_Action1, "Action1"},
    {eInputCode_Action2, "Action2"},
    {eInputCode_Action3, "Action3"},
    {eInputCode_Action4, "Action4"},
    {eInputCode_LTrigger, "LTrigger"},
    {eInputCode_RTrigger, "RTrigger"},
    {eInputCode_LTrigger2, "LTrigger2"},
    {eInputCode_RTrigger2, "RTrigger2"},
    {eInputCode_LTrigger3, "LTrigger3"},
    {eInputCode_RTrigger3, "RTrigger3"},
    {eInputCode_LAJoyUp, "LAJoyUp"},
    {eInputCode_LAJoyRight, "LAJoyRight"},
    {eInputCode_LAJoyDown, "LAJoyDown"},
    {eInputCode_LAJoyLeft, "LAJoyLeft"},
    {eInputCode_RAJoyUp, "RAJoyUp"},
    {eInputCode_RAJoyRight, "RAJoyRight"},
    {eInputCode_RAJoyDown, "RAJoyDown"},
    {eInputCode_RAJoyLeft, "RAJoyLeft"},
    {eInputCode_DPadUp, "DPadUp"},
    {eInputCode_DPadRight, "DPadRight"},
    {eInputCode_DPadDown, "DPadDown"},
    {eInputCode_DPadLeft, "DPadLeft"},
    {eInputCode_GeneralUp, "GeneralUp"},
    {eInputCode_GeneralRight, "GeneralRight"},
    {eInputCode_GeneralDown, "GeneralDown"},
    {eInputCode_GeneralLeft, "GeneralLeft"},
    {eInputCode_Key, "KeyDown"},
};

// GLOBAL: SWBF2 0x01e55f48
int GuiManager::mMouseX;
// GLOBAL: SWBF2 0x01e55f70
int GuiManager::mMouseY;
// GLOBAL: SWBF2 0x01e55f4c
unsigned int GuiManager::mCurViewport;
// GLOBAL: SWBF2 0x01e55f6c
int GuiManager::sStartupDelay;
// GLOBAL: SWBF2 0x01e55f50
bool GuiManager::m_InputTriggered[1][26];

// GLOBAL: SWBF2 0x01ebd944
// SYNTHETIC: SWBF2 0x004039a0
// `dynamic initializer for 'sKeygrabber''
// SYNTHETIC: SWBF2 0x00768790
// `dynamic atexit destructor for 'sKeygrabber''
KeyGrabber sKeygrabber;

// FUNCTION: SWBF2 0x00527fd0
void KeyGrabber::KeyboardEvent(RedInputEventType, RedKeyboard*, unsigned char ucKey,
                               unsigned char ucAscii, unsigned short usFlags)
{
    mStroke.key = ucKey;
    mStroke.ascii = ucAscii;
    mStroke.flags = usFlags;
}

GuiManager::Event::Event(eEventType eType, unsigned int uiVal, const char* pcName, int iController)
{
    type = eType;
    val.i32 = uiVal;
    if (pcName)
    {
        strlcpy(pName, pcName, sizeof(pName));
    }
    else
    {
        pName[0] = 0;
    }
    mController = iController;
}

// FUNCTION: SWBF2 0x00528020
GuiManager::GuiManager() : mEventQueue(64)
{
    Reset();
}

// FUNCTION: SWBF2 0x00528040
GuiManager::~GuiManager()
{
}

// FUNCTION: SWBF2 0x00528050
void GuiManager::InitAll()
{
    Event::sMemoryPool.Setup(0x40, Event::sMemoryPool.mSize,
                             Event::sMemoryPool.mGrow);

    GuiManager* pGui = mInstances;
    pGui->mbInitalized = true;
    pGui->mViewport = 0;
    pGui->mJoystickIndex = 0;
}

// FUNCTION: SWBF2 0x00528090
void GuiManager::ResetAll()
{
    mInstances->Reset();
}

// FUNCTION: SWBF2 0x005280a0
void GuiManager::Reset()
{
    mCurrentGuiLifeCycle = eLifeCycle_Void;
    mJoystickIndex = 0;
    memset(mCurrentScope, 0, sizeof(mCurrentScope));
    memset(mCurrentPopup, 0, sizeof(mCurrentPopup));
    memset(mCurrentError, 0, sizeof(mCurrentError));
    memset(mErrorLevelInfo, 0, sizeof(mErrorLevelInfo));
    ClearScreenStack();
    sStartupDelay = 5;

    mInstances->m_fNextUpTrigger = kINITIAL_REPEAT_DELAY;
    mInstances->m_fNextDownTrigger = kINITIAL_REPEAT_DELAY;
    mInstances->m_fNextLeftTrigger = kINITIAL_REPEAT_DELAY;
    mInstances->m_fNextRightTrigger = kINITIAL_REPEAT_DELAY;

    while (mEventQueue.GetList().GetCount())
    {
        mEventQueue.RemoveFirst();
    }

    mErrorLevel = ERROR_NONE;
    mfnIsErrorResolved = 0;
    mfnOnErrorBoxOk = 0;
    mbInitalized = false;
}

// FUNCTION: SWBF2 0x005281c0
void GuiManager::UpdateAll(float fElapsed)
{
    if (sStartupDelay > 0)
    {
        --sStartupDelay;
    }
    else
    {
        unsigned int uiState = GameState::GetState();
        bool bSplitScreen = uiState == 0x8ff60339 && GameLoop::sShowStats && GameLoop::mNumCameras > 1;
        bool bShell = uiState == 0x11e1fc01;
        FLInputManager* pInput = FLInputManager::Instance();
        bool bReadAll = pInput->GetReadAll();
        bool bBindMultiple = pInput->GetAllowBindMultiple();
        int iPrimary = pInput->GetPrimaryPort();

        if (!FLInputManager::s_instance.AnyMissing())
        {
            GuiManager* pGui = mInstances;
            if (pGui->mCurrentError[0] && pGui->mfnIsErrorResolved && pGui->mfnIsErrorResolved())
            {
                mInstances->CloseError(false, false);
            }

            int iController = mInstances->mJoystickIndex;
            GUIInputs* pInputs = 0;
            if (bSplitScreen)
            {
                if (playerJoystickNumbers[0] == iPrimary)
                {
                    iController = 0;
                }
                pInputs = pInput->GetPlayerGuiInputs(iController);
            }
            else if (!bShell)
            {
                iController = 0;
                pInputs = pInput->GetPlayerGuiInputs(iController);
            }
            else if (bReadAll)
            {
                bool bBound;
                if (bBindMultiple)
                {
                    iController = pInput->GetHWPort(0);
                    bBound = iController >= 0;
                }
                else
                {
                    bBound = pInput->IsControllerBound(iController);
                }
                if (bBound)
                {
                    pInputs = pInput->GetPlayerGuiInputs(iController);
                }
            }
            else if (iPrimary <= 0)
            {
                pInputs = pInput->GetPlayerGuiInputs(iController);
            }

            if (bReadAll || bShell || mInstances->mCurrentScope[0] || mInstances->mCurrentError[0])
            {
                mInstances->CollectInputs(0, pInputs, iController, fElapsed);
            }
        }
    }
    mInstances->Update(fElapsed);
}

// FUNCTION: SWBF2 0x00528340
void GuiManager::_ExitScreen(const char*, bool bArg)
{
    IFScreen* pScreen = IFScreen::Find(mCurrentScope);
    if (pScreen)
    {
        strlcpy(mCurrentScope, pScreen->GetNameStr(), sizeof(mCurrentScope));
        if (bArg)
        {
            CallLuaFunctionOfScope(EventToLuaFunction[eEventType_LifeCycle_Exit].luaCallbackFunction, 1.0f, false);
        }
        else
        {
            CallLuaFunctionOfScope(EventToLuaFunction[eEventType_LifeCycle_Exit].luaCallbackFunction, false);
        }
        pScreen->RemoveFromRedScreen();
    }
    mCurrentGuiLifeCycle = eLifeCycle_Exited;
    FLRenderer::EnableCursor(false);
}

// FUNCTION: SWBF2 0x005283d0
void GuiManager::_EnterScreen(const char* pScreenName, bool bArg)
{
    IFScreen* pScreen = IFScreen::Find(pScreenName);
    if (pScreen)
    {
        strlcpy(mCurrentScope, pScreen->GetNameStr(), sizeof(mCurrentScope));
        mScreenStack[mScreenStackPos] = pScreen;
        if (bArg)
        {
            CallLuaFunctionOfScope(EventToLuaFunction[eEventType_LifeCycle_Enter].luaCallbackFunction, 1.0f, false);
        }
        else
        {
            CallLuaFunctionOfScope(EventToLuaFunction[eEventType_LifeCycle_Enter].luaCallbackFunction, false);
        }
        pScreen->AddToRedScreen();
        IFScreen::SetCurScreen(mViewport, pScreen);
        mCurrentGuiLifeCycle = eLifeCycle_Entered;
        FLRenderer::EnableCursor(true);
    }
    else
    {
        memset(mCurrentScope, 0, sizeof(mCurrentScope));
        mCurrentGuiLifeCycle = eLifeCycle_Void;
    }
    mMouseX = -999;
    mMouseY = -999;
    FLInputManager::Instance()->StopAllRumble();
}

// FUNCTION: SWBF2 0x005284b0
void GuiManager::SetCurrentScreen(const char* pScreen, bool bExitArg, bool bEnterArg)
{
    if (Event::sMemoryPool.mUsed + 2 <= Event::sMemoryPool.GetCount())
    {
        mEventQueue.AddLast(new Event(eEventType_LifeCycle_Exit, bExitArg, 0, mJoystickIndex));
        mEventQueue.AddLast(new Event(eEventType_LifeCycle_Enter, bEnterArg, pScreen, mJoystickIndex));
    }
}

// FUNCTION: SWBF2 0x00528620
void GuiManager::ChangeToScreen(const char* pScreen, bool, bool bExitArg)
{
    if (mCurrentPopup[0])
    {
        ClosePopup(true);
    }
    SetCurrentScreen(pScreen, bExitArg, true);
}

// FUNCTION: SWBF2 0x00528660
void GuiManager::PushScreen(const char* pScreen, int iJoystick, int)
{
    if (iJoystick >= 0)
    {
        mJoystickIndex = iJoystick;
    }
    if (mScreenStackPos < 32)
    {
        if (mCurrentPopup[0])
        {
            ClosePopup(true);
        }
        ++mScreenStackPos;
        IFScreen* pIFScreen = IFScreen::Find(pScreen);
        if (pIFScreen)
        {
            mScreenStack[mScreenStackPos] = pIFScreen;
        }
        SetCurrentScreen(pScreen, true, true);
    }
}

// FUNCTION: SWBF2 0x005286e0
void GuiManager::PopScreen(const char* pScreen)
{
    if (mScreenStackPos < 0)
    {
        return;
    }
    if (mCurrentPopup[0])
    {
        ClosePopup(true);
    }

    if (pScreen)
    {
        for (int i = mScreenStackPos; i >= 0; --i)
        {
            if (!_stricmp(mScreenStack[i]->GetNameStr(), pScreen))
            {
                mScreenStackPos = i;
                SetCurrentScreen(mScreenStack[i]->GetNameStr(), false, false);
                return;
            }
        }
    }
    else
    {
        --mScreenStackPos;
        if (mScreenStackPos >= 0 && mScreenStack[mScreenStackPos])
        {
            SetCurrentScreen(mScreenStack[mScreenStackPos]->GetNameStr(), false, false);
        }
        else
        {
            SetCurrentScreen("", false, false);
        }
    }
}

// FUNCTION: SWBF2 0x005287b0
void GuiManager::ClearScreenStack()
{
    memset(mScreenStack, 0, sizeof(mScreenStack));
    mScreenStackPos = -1;
}

// FUNCTION: SWBF2 0x005287e0
bool GuiManager::IsScreenInStack(const char* pScreen)
{
    for (int i = mScreenStackPos; i >= 0; --i)
    {
        if (!_stricmp(mScreenStack[i]->GetNameStr(), pScreen))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00528830
void GuiManager::OpenPopup(const char* pPopup)
{
    if (!mCurrentPopup[0])
    {
        strncpy(mCurrentPopup, pPopup, sizeof(mCurrentPopup));
        RedScreenGroupElement* pGroup = (RedScreenGroupElement*)RedInterfaceElement::Find(mCurrentPopup);
        if (pGroup->IsScreenGroup())
        {
            pGroup->Visible(false);
            pGroup->Visible(true);
            pGroup->AddToRedScreen();
        }
    }
    mMouseX = -999;
    mMouseY = -999;
}

// FUNCTION: SWBF2 0x005288a0
void GuiManager::ClosePopup(bool bClearLua)
{
    if (mCurrentPopup[0])
    {
        if (bClearLua)
        {
            lua_getglobal(LuaHelper::mState, mCurrentPopup);
            if (lua_istable(LuaHelper::mState, -1))
            {
                LuaHelper::TableWriteNil("Open");
            }
            lua_pop(LuaHelper::mState, 1);
            lua_pushnil(LuaHelper::mState);
            lua_setglobal(LuaHelper::mState, "gCurPopup");
        }
        RedScreenGroupElement* pGroup = (RedScreenGroupElement*)RedInterfaceElement::Find(mCurrentPopup);
        if (pGroup->IsScreenGroup())
        {
            pGroup->RemoveFromRedScreen();
            pGroup->Visible(true);
            pGroup->Visible(false);
        }
        memset(mCurrentPopup, 0, sizeof(mCurrentPopup));
    }
    mMouseX = -999;
    mMouseY = -999;
    CallLuaFunctionOfScope("Reactivate", true);
}

// FUNCTION: SWBF2 0x005289a0
bool GuiManager::OpenError(eErrorSeverity eSeverity, const unsigned short* pMessage,
                           bool (*pfnIsResolved)(), void (*pfnErrorBoxOk)(bool))
{
    if (eSeverity >= ERROR_LEAVE_SESSION && eSeverity < mErrorLevel)
    {
        unsigned int uiSize = 2 * PblUnicode::ustrlen(pMessage) + 2;
        unsigned short* pCopy = (unsigned short*)RedAllocFromHeap(GameMemory::RunTimeHeap, uiSize, 0);
        if (pCopy)
        {
            memcpy(pCopy, pMessage, uiSize);
            mErrorLevelInfo[eSeverity].pfnIsResolved = pfnIsResolved;
            mErrorLevelInfo[eSeverity].pMessage = pCopy;
            mErrorLevelInfo[eSeverity].pfnErrorBoxOk = pfnErrorBoxOk;
        }
        return true;
    }

    if (eSeverity == ERROR_NONE || eSeverity < mErrorLevel)
    {
        return false;
    }

    RedScreenGroupElement* pGroup = (RedScreenGroupElement*)RedInterfaceElement::Find(ERROR_POPUP_NAME);
    if (!pGroup || !pGroup->IsScreenGroup())
    {
        return false;
    }

    pGroup->Visible(false);
    pGroup->Visible(true);
    pGroup->AddToRedScreen();
    mMouseX = -999;
    mMouseY = -999;
    strncpy(mCurrentError, ERROR_POPUP_NAME, sizeof(mCurrentError));
    mfnIsErrorResolved = pfnIsResolved;
    mfnOnErrorBoxOk = pfnErrorBoxOk;
    mErrorLevel = eSeverity;

    if (mErrorLevelInfo[eSeverity].pMessage != pMessage)
    {
        if (mErrorLevelInfo[mErrorLevel].pMessage)
        {
            RedFreeToHeap(GameMemory::RunTimeHeap, mErrorLevelInfo[mErrorLevel].pMessage, 0);
            mErrorLevelInfo[mErrorLevel].pMessage = 0;
        }
        unsigned int uiSize = 2 * PblUnicode::ustrlen(pMessage) + 2;
        unsigned short* pCopy = (unsigned short*)RedAllocFromHeap(GameMemory::RunTimeHeap, uiSize, 0);
        if (pCopy)
        {
            memcpy(pCopy, pMessage, uiSize);
            mErrorLevelInfo[eSeverity].pMessage = pCopy;
        }
    }
    mErrorLevelInfo[mErrorLevel].pfnIsResolved = pfnIsResolved;
    mErrorLevelInfo[mErrorLevel].pfnErrorBoxOk = pfnErrorBoxOk;

    CallLuaFunctionOfScope("fnActivate", (float)eSeverity, true);
    CallLuaFunctionOfScope("fnSetTitle", mErrorLevelInfo[eSeverity].pMessage, true);
    FLRenderer::EnableCursor(true);
    return true;
}

// FUNCTION: SWBF2 0x00528ba0
void GuiManager::CloseError(bool bOk, bool)
{
    if (!mCurrentError[0])
    {
        return;
    }

    mMouseX = -999;
    mMouseY = -999;
    RedScreenGroupElement* pGroup = (RedScreenGroupElement*)RedInterfaceElement::Find(mCurrentError);
    if (pGroup->IsScreenGroup())
    {
        pGroup->RemoveFromRedScreen();
        pGroup->Visible(true);
        pGroup->Visible(false);
    }
    memset(mCurrentError, 0, sizeof(mCurrentError));

    if (mErrorLevelInfo[mErrorLevel].pMessage)
    {
        RedFreeToHeap(GameMemory::RunTimeHeap, mErrorLevelInfo[mErrorLevel].pMessage, 0);
        mErrorLevelInfo[mErrorLevel].pMessage = 0;
    }
    mErrorLevelInfo[mErrorLevel].pMessage = 0;
    mErrorLevelInfo[mErrorLevel].pfnIsResolved = 0;
    mErrorLevelInfo[mErrorLevel].pfnErrorBoxOk = 0;

    void (*pfnErrorBoxOk)(bool) = mfnOnErrorBoxOk;
    int iLevel = mErrorLevel;
    mErrorLevel = ERROR_NONE;
    mfnIsErrorResolved = 0;
    if (pfnErrorBoxOk)
    {
        mfnOnErrorBoxOk = 0;
        pfnErrorBoxOk(bOk);
    }

    for (; iLevel > 0; --iLevel)
    {
        if (mErrorLevelInfo[iLevel].pMessage)
        {
            OpenError((eErrorSeverity)iLevel, mErrorLevelInfo[iLevel].pMessage,
                      mErrorLevelInfo[iLevel].pfnIsResolved, mErrorLevelInfo[iLevel].pfnErrorBoxOk);
            return;
        }
    }
    CallLuaFunctionOfScope("Reactivate", true);
}

// FUNCTION: SWBF2 0x00528d00
void GuiManager::Update(float fElapsed)
{
    unsigned int uiViewport = mCurViewport;
    mCurViewport = mViewport;
    Event* pEvent = new Event;
    if (pEvent)
    {
        pEvent->type = eEventType_Update;
        pEvent->val.f32 = fElapsed;
        mEventQueue.AddLast(pEvent);
    }
    HandleEvents();
    mCurViewport = uiViewport;
}

// FUNCTION: SWBF2 0x00528da0
const char* GuiManager::GetCurrentFocus() const
{
    if (mCurrentError[0])
    {
        return mCurrentError;
    }
    if (mCurrentPopup[0])
    {
        return mCurrentPopup;
    }
    return mCurrentScope;
}

// FUNCTION: SWBF2 0x00528dc0
bool GuiManager::CallLuaFunctionOfScope(const char* pcFunc, bool bPopupOnly)
{
    if (mCurrentError[0])
    {
        return LuaHelper::CallTableProcWThis(mCurrentError, pcFunc);
    }
    bool bPopup = mCurrentPopup[0] != 0;
    if (bPopup)
    {
        if (bPopupOnly)
        {
            return LuaHelper::CallTableProcWThis(mCurrentPopup, pcFunc);
        }
        LuaHelper::CallTableProcWThis(mCurrentPopup, pcFunc);
    }
    return LuaHelper::CallTableProcWThis(mCurrentScope, pcFunc);
}

// FUNCTION: SWBF2 0x00528e10
bool GuiManager::CallLuaFunctionOfScope(const char* pcFunc, float fArg, bool bPopupOnly)
{
    if (mCurrentError[0])
    {
        if (strcmp(pcFunc, "Enter"))
        {
            return LuaHelper::CallTableProcWThis(mCurrentError, pcFunc, fArg);
        }
        LuaHelper::CallTableProcWThis(mCurrentError, pcFunc, fArg);
    }
    if (!mCurrentScope[0])
    {
        return false;
    }
    bool bPopup = mCurrentPopup[0] != 0;
    if (bPopup)
    {
        if (bPopupOnly)
        {
            return LuaHelper::CallTableProcWThis(mCurrentPopup, pcFunc, fArg);
        }
        LuaHelper::CallTableProcWThis(mCurrentPopup, pcFunc, fArg);
    }
    return LuaHelper::CallTableProcWThis(mCurrentScope, pcFunc, fArg);
}

// FUNCTION: SWBF2 0x00528ec0
bool GuiManager::CallLuaFunctionOfScope(const char* pcFunc, const unsigned short* pusArg, bool bPopupOnly)
{
    if (mCurrentError[0])
    {
        if (strcmp(pcFunc, "Enter"))
        {
            return LuaHelper::CallTableProcWThis(mCurrentError, pcFunc, pusArg);
        }
        LuaHelper::CallTableProcWThis(mCurrentError, pcFunc, pusArg);
    }
    if (!mCurrentScope[0])
    {
        return false;
    }
    bool bPopup = mCurrentPopup[0] != 0;
    if (bPopup)
    {
        if (bPopupOnly)
        {
            return LuaHelper::CallTableProcWThis(mCurrentPopup, pcFunc, pusArg);
        }
        LuaHelper::CallTableProcWThis(mCurrentPopup, pcFunc, pusArg);
    }
    return LuaHelper::CallTableProcWThis(mCurrentScope, pcFunc, pusArg);
}

// FUNCTION: SWBF2 0x00528f60
void GuiManager::UpdateCursor()
{
    LuaHelper::CallTableProcWThis(Instance()->GetCurrentFocus(), "HandleMouse",
                                  (float)mMouseX, (float)mMouseY);
}

// FUNCTION: SWBF2 0x00528fb0
void GuiManager::HandleEvents()
{
    while (mEventQueue.GetList().GetCount())
    {
        PblList<Event*>::Itor it(mEventQueue.GetList().GetFirst());
        Event* pEvent = *it;
        mEventQueue.RemoveFirst();

        switch (pEvent->type)
        {
        case eEventType_Input:
            if (pEvent->val.i32 == eInputCode_Coords)
            {
                const char* pcScope = Instance()->mCurrentScope;
                if (pcScope && pcScope[0])
                {
                    UpdateCursor();
                }
            }
            else
            {
                char acFunc[128];
                _snprintf(acFunc, sizeof(acFunc) - 1, "%s%s",
                          EventToLuaFunction[eEventType_Input].luaCallbackFunction,
                          InputCodeToLuaFunctionExt[pEvent->val.i32].luaCallbackFunctionExt);
                acFunc[sizeof(acFunc) - 1] = 0;
                if (pEvent->val.i32 == eInputCode_Key || pEvent->mController >= 0)
                {
                    CallLuaFunctionOfScope(acFunc, (float)pEvent->mController, true);
                }
                else
                {
                    CallLuaFunctionOfScope(acFunc, true);
                }
            }
            break;
        case eEventType_LifeCycle_Exit:
            _ExitScreen(pEvent->pName, pEvent->val.i32 != 0);
            break;
        case eEventType_LifeCycle_Enter:
            _EnterScreen(pEvent->pName, pEvent->val.i32 != 0);
            break;
        case eEventType_Update:
            if (mCurrentGuiLifeCycle == eLifeCycle_Entered)
            {
                CallLuaFunctionOfScope(EventToLuaFunction[eEventType_Update].luaCallbackFunction,
                                       pEvent->val.f32, false);
            }
            break;
        }
        delete pEvent;
    }
}

// FUNCTION: SWBF2 0x00529140
void GuiManager::CreateInputEvent(int iCode, int iController)
{
    Event* pEvent = new Event;
    if (pEvent)
    {
        pEvent->type = eEventType_Input;
        pEvent->val.i32 = iCode;
        pEvent->mController = iController;
        mEventQueue.AddLast(pEvent);
    }
}

// FUNCTION: SWBF2 0x005291c0
void GuiManager::CreateKeyInputEvent(int, int iKey)
{
    Event* pEvent = new Event;
    if (pEvent)
    {
        pEvent->type = eEventType_Input;
        pEvent->val.i32 = eInputCode_Key;
        pEvent->mController = iKey;
        mEventQueue.AddLast(pEvent);
    }
}

// FUNCTION: SWBF2 0x00529240
void GuiManager::CollectInputs(int iJoystick, GUIInputs* pInputs, int iController, float fElapsed)
{
    bool bEventCreated = false;
    bool bTriggered[eGUIINPUT_MAX];
    memset(bTriggered, 0, sizeof(bTriggered));

    if (!pInputs || iController < 0)
    {
        memcpy(m_InputTriggered[iJoystick], bTriggered, sizeof(bTriggered));
        return;
    }

    if (pInputs->mProcessedGUIInputs[eGUIINPUT_Start] >= 0.125f &&
        pInputs->mLastGUIInputs[eGUIINPUT_Start] < 0.125f)
    {
        bTriggered[eGUIINPUT_Start] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Start])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_Start, iController);
        }
    }
    if (!bEventCreated && pInputs->mProcessedGUIInputs[eGUIINPUT_Accept] >= 0.125f)
    {
        bTriggered[eGUIINPUT_Accept] = true;
        LuaHelper::GetGlobal("gbWantMousedownEvents");
        if (lua_type(LuaHelper::mState, -1) != LUA_TNIL)
        {
            m_InputTriggered[iJoystick][eGUIINPUT_Accept] = false;
        }
        lua_pop(LuaHelper::mState, 1);
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Accept])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_Accept, iController);
        }
    }
    if (!bEventCreated && pInputs->mProcessedGUIInputs[eGUIINPUT_Select] >= 0.125f)
    {
        bTriggered[eGUIINPUT_Select] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Select])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_Select, iController);
        }
    }
    if (!bEventCreated && pInputs->mProcessedGUIInputs[eGUIINPUT_Back] >= 0.125f)
    {
        bTriggered[eGUIINPUT_Back] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Back])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_Back, iController);
        }
    }

    if (mInstances->mJoystickIndex == iController &&
        (mMouseX != pInputs->mMouseCoords.mPosX || mMouseY != pInputs->mMouseCoords.mPosY))
    {
        mMouseX = pInputs->mMouseCoords.mPosX;
        mMouseY = pInputs->mMouseCoords.mPosY;
        CreateInputEvent(eInputCode_Coords, iController);
    }

    if (!bEventCreated && pInputs->IsLeftTriggerPressed())
    {
        bTriggered[eGUIINPUT_LeftTrigger] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_LeftTrigger])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_LTrigger, iController);
        }
    }
    if (!bEventCreated && pInputs->IsLeftTrigger2Pressed())
    {
        bTriggered[eGUIINPUT_LeftTrigger2] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_LeftTrigger2])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_LTrigger2, iController);
        }
    }
    if (!bEventCreated && pInputs->IsLeftTrigger3Pressed())
    {
        bTriggered[eGUIINPUT_LeftTrigger3] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_LeftTrigger3])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_LTrigger3, iController);
        }
    }
    if (!bEventCreated && pInputs->IsRightTriggerPressed())
    {
        bTriggered[eGUIINPUT_RightTrigger] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_RightTrigger])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_RTrigger, iController);
        }
    }
    if (!bEventCreated && pInputs->IsRightTrigger2Pressed())
    {
        bTriggered[eGUIINPUT_RightTrigger2] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_RightTrigger2])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_RTrigger2, iController);
        }
    }
    if (!bEventCreated && pInputs->IsRightTrigger3Pressed())
    {
        bTriggered[eGUIINPUT_RightTrigger3] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_RightTrigger3])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_RTrigger3, iController);
        }
    }
    if (!bEventCreated && pInputs->IsMisc1Pressed())
    {
        bTriggered[eGUIINPUT_Misc1] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Misc1])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_Misc, iController);
        }
    }
    if (!bEventCreated && pInputs->IsMisc2Pressed())
    {
        bTriggered[eGUIINPUT_Misc2] = true;
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Misc2])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_Misc2, iController);
        }
    }

    if (!bEventCreated && (pInputs->IsDPadUpPressed() || pInputs->mProcessedGUIInputs[eGUIINPUT_yAxis] >= 0.5f))
    {
        bTriggered[eGUIINPUT_Up] = true;
        m_fNextUpTrigger -= fElapsed;
        if (m_fNextUpTrigger < 0.0f)
        {
            m_fNextUpTrigger = kEXTRA_REPEAT_DELAY;
            m_InputTriggered[iJoystick][eGUIINPUT_Up] = false;
        }
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Up])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_GeneralUp, iController);
            if (pInputs->IsDPadUpPressed())
            {
                CreateInputEvent(eInputCode_DPadUp, iController);
            }
        }
    }
    else
    {
        m_fNextUpTrigger = kINITIAL_REPEAT_DELAY;
    }

    if (!bEventCreated && (pInputs->IsDPadDownPressed() || pInputs->mProcessedGUIInputs[eGUIINPUT_yAxis] <= -0.5f))
    {
        bTriggered[eGUIINPUT_Down] = true;
        m_fNextDownTrigger -= fElapsed;
        if (m_fNextDownTrigger < 0.0f)
        {
            m_fNextDownTrigger = kEXTRA_REPEAT_DELAY;
            m_InputTriggered[iJoystick][eGUIINPUT_Down] = false;
        }
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Down])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_GeneralDown, iController);
            if (pInputs->IsDPadDownPressed())
            {
                CreateInputEvent(eInputCode_DPadDown, iController);
            }
        }
    }
    else
    {
        m_fNextDownTrigger = kINITIAL_REPEAT_DELAY;
    }

    if (!bEventCreated && (pInputs->mProcessedGUIInputs[eGUIINPUT_Left] >= 0.125f ||
                           pInputs->mProcessedGUIInputs[eGUIINPUT_xAxis] <= -0.5f))
    {
        bTriggered[eGUIINPUT_Left] = true;
        if (!g_InputRepeatEnable)
        {
            m_fNextLeftTrigger = kINITIAL_REPEAT_DELAY;
        }
        m_fNextLeftTrigger -= fElapsed;
        if (m_fNextLeftTrigger < 0.0f)
        {
            m_fNextLeftTrigger = kEXTRA_REPEAT_DELAY;
            m_InputTriggered[iJoystick][eGUIINPUT_Left] = false;
        }
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Left])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_GeneralLeft, iController);
            if (pInputs->mProcessedGUIInputs[eGUIINPUT_Left] >= 0.125f)
            {
                CreateInputEvent(eInputCode_DPadLeft, iController);
            }
        }
    }
    else
    {
        m_fNextLeftTrigger = kINITIAL_REPEAT_DELAY;
    }

    if (!bEventCreated && (pInputs->mProcessedGUIInputs[eGUIINPUT_Right] >= 0.125f ||
                           pInputs->mProcessedGUIInputs[eGUIINPUT_xAxis] >= 0.5f))
    {
        bTriggered[eGUIINPUT_Right] = true;
        if (!g_InputRepeatEnable)
        {
            m_fNextRightTrigger = kINITIAL_REPEAT_DELAY;
        }
        m_fNextRightTrigger -= fElapsed;
        if (m_fNextRightTrigger < 0.0f)
        {
            m_fNextRightTrigger = kEXTRA_REPEAT_DELAY;
            m_InputTriggered[iJoystick][eGUIINPUT_Right] = false;
        }
        if (!m_InputTriggered[iJoystick][eGUIINPUT_Right])
        {
            bEventCreated = true;
            CreateInputEvent(eInputCode_GeneralRight, iController);
            if (pInputs->mProcessedGUIInputs[eGUIINPUT_Right] >= 0.125f)
            {
                CreateInputEvent(eInputCode_DPadRight, iController);
            }
        }
    }
    else
    {
        m_fNextRightTrigger = kINITIAL_REPEAT_DELAY;
    }

    memcpy(m_InputTriggered[iJoystick], bTriggered, sizeof(bTriggered));

    if (!sKeygrabber.IsInited())
    {
        sKeygrabber.Init();
    }
    KeyStroke stroke = sKeygrabber.GetStroke();
    if (stroke.key)
    {
        if (stroke.ascii)
        {
            CreateKeyInputEvent(iController, stroke.ascii);
        }
        else
        {
            CreateKeyInputEvent(iController, -(int)stroke.key);
        }
    }
}

// FUNCTION: SWBF2 0x005298a0
void GuiManager::DebounceOneJoystick(int iJoystick)
{
    memset(m_InputTriggered[iJoystick], true, sizeof(m_InputTriggered[iJoystick]));
}

// FUNCTION: SWBF2 0x005298c0
void GuiManager::DebounceJoystick(int)
{
    DebounceOneJoystick(0);
}

// FUNCTION: SWBF2 0x005298d0
int GuiManager::GetJoystickFromViewport(unsigned int uViewport)
{
    return uViewport < 1 ? mInstances[uViewport].mJoystickIndex : -1;
}

// FUNCTION: SWBF2 0x005298f0
void GuiManager::SetInputRepeat(bool bRepeat)
{
    g_InputRepeatEnable = bRepeat;
}
