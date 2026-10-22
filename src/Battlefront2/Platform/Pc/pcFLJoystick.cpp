#include <math.h>
#include <string.h>

#include "pcFLJoystick.h"
#include "Battlefront2/Source/ChatMessageDisplay.h"
#include "Battlefront2/Source/FLInputManager.h"
#include "Battlefront2/Source/GameMovie.h"
#include "Battlefront2/Source/GameState.h"
#include "Battlefront2/Source/NetGame.h"
#include "Battlefront2/Source/PauseMenu.h"
#include "Battlefront2/Source/PlayerProfile.h"
#include "RedEngineFL/input/RedJoystick.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/input/RedMouse.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "PebbleFL/Common/PblUnicode.h"

// FUNCTION: SWBF2 0x00415c60
void RawControllerInputs::CloseChatMode()
{
    m_ChatMode = 0;
    m_ChatLength = 0;
    memset(m_ChatWords, 0, 0x80);
}

#define DEFAULT_UNBOUND_8 { -1 }, { -1 }, { -1 }, { -1 }, { -1 }, { -1 }, { -1 }, { -1 }
#define DEFAULT_UNBOUND_64 DEFAULT_UNBOUND_8, DEFAULT_UNBOUND_8, DEFAULT_UNBOUND_8, \
    DEFAULT_UNBOUND_8, DEFAULT_UNBOUND_8, DEFAULT_UNBOUND_8, DEFAULT_UNBOUND_8, DEFAULT_UNBOUND_8
// GLOBAL: SWBF2 0x007e5888
BindDestination RawControllerInputs::sDefaultBindings[5][76] =
{

    {
        DEFAULT_UNBOUND_64,
        { 32 }, { 33 }, { 0 }, { 1 }, { 5 }, { 6 }, { 5 }, { 7 }, { 3 }, { 1 }, { 13 }, { 14 }
    },

    {
        DEFAULT_UNBOUND_64,
        { 32 }, { 33 }, { 0 }, { 1 }, { 5 }, { 6 }, { 5 }, { 7 }, { 3 }, { 1 }, { 13 }, { 14 }
    },

    {
        DEFAULT_UNBOUND_64,
        { 32 }, { 33 }, { 0 }, { 1 }, { 5 }, { -1 }, { 5 }, { 7 }, { 3 }, { 1 }, { 13 }, { 14 }
    },

    {
        DEFAULT_UNBOUND_64,
        { 32 }, { 33 }, { 0 }, { 1 }, { 5 }, { 6 }, { 5 }, { 7 }, { 3 }, { 1 }, { 13 }, { 14 }
    },

    {
        DEFAULT_UNBOUND_64,
        { 32 }, { 33 }, { 0 }, { 1 }, { 5 }, { 6 }, { 5 }, { 7 }, { 3 }, { 1 }, { 13 }, { 14 }
    },
};
#undef DEFAULT_UNBOUND_64
#undef DEFAULT_UNBOUND_8

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

// FUNCTION: SWBF2 0x004149a0
unsigned short __cdecl RawInputToActionKey(unsigned short usRawInput)
{
    return (unsigned short)((usRawInput + 1) << 8);
}

// GLOBAL: SWBF2 0x01e2fd14

int mask_chat;

unsigned short __cdecl RawInputToActionKey(unsigned short usRawInput);

// GLOBAL: SWBF2 0x01f9a928
// SYNTHETIC: SWBF2 0x004014d0
// `dynamic initializer for 'RawControllerInputs::sDefaultKeyboardBindings''
ActionKey RawControllerInputs::sDefaultKeyboardBindings[5][43] =
{

    {
        { { RawInputToActionKey(66), 0, 0 } },
        { { RawInputToActionKey(67), 0, 0 } },
        { { DIK_LSHIFT, DIK_RSHIFT, 0 } },
        { { DIK_SPACE, DIK_NUMPAD0, 0 } },
        { { DIK_C, DIK_RCONTROL, 0 } },
        { { DIK_Z, RawInputToActionKey(68), 0 } },
        { { DIK_V, DIK_END, 0 } },
        { { DIK_R, DIK_DELETE, 0 } },
        { { DIK_E, DIK_RETURN, 0 } },
        { { DIK_F4, DIK_NUMPAD4, 0 } },
        { { DIK_PRIOR, DIK_F1, 0 } },
        { { DIK_NEXT, DIK_F2, 0 } },
        { { DIK_Q, RawInputToActionKey(69), 0 } },
        { { DIK_1, RawInputToActionKey(75), 0 } },
        { { 0, 0, 0 } },
        { { DIK_2, RawInputToActionKey(74), 0 } },
        { { 0, 0, 0 } },
        { { DIK_TAB, DIK_BACKSLASH, 0 } },
        { { DIK_T, 0, 0 } },
        { { DIK_Y, 0, 0 } },
        { { DIK_F5, 0, 0 } },
        { { DIK_F6, 0, 0 } },
        { { DIK_F7, 0, 0 } },
        { { DIK_F8, 0, 0 } },
        { { DIK_F9, 0, 0 } },
        { { DIK_F10, 0, 0 } },
        { { DIK_F11, 0, 0 } },
        { { DIK_F12, 0, 0 } },
        { { DIK_M, 0, 0 } },
        { { DIK_LMENU, DIK_NUMPAD1, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { RawInputToActionKey(64), 0, 0 } },
        { { RawInputToActionKey(65), 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_D, DIK_RIGHT, 0 } },
        { { DIK_A, DIK_LEFT, 0 } },
        { { DIK_W, DIK_UP, 0 } },
        { { DIK_S, DIK_DOWN, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
    },

    {
        { { RawInputToActionKey(66), 0, 0 } },
        { { RawInputToActionKey(67), 0, 0 } },
        { { DIK_LSHIFT, DIK_RSHIFT, 0 } },
        { { DIK_SPACE, DIK_NUMPAD0, 0 } },
        { { DIK_C, DIK_RCONTROL, 0 } },
        { { DIK_Z, RawInputToActionKey(68), 0 } },
        { { DIK_V, DIK_END, 0 } },
        { { DIK_R, DIK_DELETE, 0 } },
        { { DIK_E, DIK_RETURN, 0 } },
        { { DIK_F4, DIK_NUMPAD4, 0 } },
        { { DIK_PRIOR, DIK_F1, 0 } },
        { { DIK_NEXT, DIK_F2, 0 } },
        { { DIK_Q, RawInputToActionKey(69), 0 } },
        { { DIK_1, RawInputToActionKey(74), 0 } },
        { { DIK_2, RawInputToActionKey(75), 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_TAB, DIK_BACKSLASH, 0 } },
        { { DIK_T, 0, 0 } },
        { { DIK_Y, 0, 0 } },
        { { DIK_F5, 0, 0 } },
        { { DIK_F6, 0, 0 } },
        { { DIK_F7, 0, 0 } },
        { { DIK_F8, 0, 0 } },
        { { DIK_F9, 0, 0 } },
        { { DIK_F10, 0, 0 } },
        { { DIK_F11, 0, 0 } },
        { { DIK_F12, 0, 0 } },
        { { DIK_M, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { RawInputToActionKey(64), 0, 0 } },
        { { RawInputToActionKey(65), 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_D, DIK_RIGHT, 0 } },
        { { DIK_A, DIK_LEFT, 0 } },
        { { DIK_W, DIK_UP, 0 } },
        { { DIK_S, DIK_DOWN, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
    },

    {
        { { RawInputToActionKey(66), 0, 0 } },
        { { RawInputToActionKey(67), 0, 0 } },
        { { DIK_LSHIFT, DIK_RSHIFT, 0 } },
        { { DIK_SPACE, DIK_NUMPAD0, 0 } },
        { { DIK_LMENU, DIK_RCONTROL, 0 } },
        { { DIK_LCONTROL, RawInputToActionKey(68), 0 } },
        { { DIK_V, DIK_END, 0 } },
        { { DIK_R, DIK_DELETE, 0 } },
        { { DIK_E, DIK_RETURN, 0 } },
        { { DIK_F4, DIK_NUMPAD4, 0 } },
        { { DIK_PRIOR, DIK_F1, 0 } },
        { { DIK_NEXT, DIK_F2, 0 } },
        { { DIK_Q, RawInputToActionKey(69), 0 } },
        { { DIK_1, RawInputToActionKey(74), 0 } },
        { { DIK_2, RawInputToActionKey(75), 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_TAB, DIK_BACKSLASH, 0 } },
        { { DIK_T, 0, 0 } },
        { { DIK_Y, 0, 0 } },
        { { DIK_F5, 0, 0 } },
        { { DIK_F6, 0, 0 } },
        { { DIK_F7, 0, 0 } },
        { { DIK_F8, 0, 0 } },
        { { DIK_F9, 0, 0 } },
        { { DIK_F10, 0, 0 } },
        { { DIK_F11, 0, 0 } },
        { { DIK_F12, 0, 0 } },
        { { DIK_M, 0, 0 } },
        { { DIK_C, DIK_NUMPAD1, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { RawInputToActionKey(64), 0, 0 } },
        { { RawInputToActionKey(65), 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_D, DIK_RIGHT, 0 } },
        { { DIK_A, DIK_LEFT, 0 } },
        { { DIK_W, DIK_UP, 0 } },
        { { DIK_S, DIK_DOWN, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
    },

    {
        { { RawInputToActionKey(66), 0, 0 } },
        { { DIK_F, RawInputToActionKey(68), 0 } },
        { { DIK_LSHIFT, DIK_RSHIFT, 0 } },
        { { DIK_SPACE, DIK_NUMPAD0, 0 } },
        { { DIK_C, DIK_RCONTROL, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { RawInputToActionKey(67), 0, 0 } },
        { { DIK_E, DIK_RETURN, 0 } },
        { { DIK_F4, DIK_NUMPAD4, 0 } },
        { { DIK_PRIOR, DIK_F1, 0 } },
        { { DIK_NEXT, DIK_F2, 0 } },
        { { DIK_Q, RawInputToActionKey(69), 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_1, RawInputToActionKey(74), 0 } },
        { { 0, 0, 0 } },
        { { DIK_TAB, DIK_BACKSLASH, 0 } },
        { { DIK_T, 0, 0 } },
        { { DIK_Y, 0, 0 } },
        { { DIK_F5, 0, 0 } },
        { { DIK_F6, 0, 0 } },
        { { DIK_F7, 0, 0 } },
        { { DIK_F8, 0, 0 } },
        { { DIK_F9, 0, 0 } },
        { { DIK_F10, 0, 0 } },
        { { DIK_F11, 0, 0 } },
        { { DIK_F12, 0, 0 } },
        { { DIK_M, 0, 0 } },
        { { DIK_LMENU, DIK_NUMPAD1, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { RawInputToActionKey(64), 0, 0 } },
        { { RawInputToActionKey(65), 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_D, DIK_RIGHT, 0 } },
        { { DIK_A, DIK_LEFT, 0 } },
        { { DIK_W, DIK_UP, 0 } },
        { { DIK_S, DIK_DOWN, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
    },

    {
        { { RawInputToActionKey(66), 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_Z, RawInputToActionKey(68), 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_E, DIK_RETURN, 0 } },
        { { DIK_F4, DIK_NUMPAD4, 0 } },
        { { DIK_PRIOR, DIK_F1, 0 } },
        { { DIK_NEXT, DIK_F2, 0 } },
        { { DIK_Q, RawInputToActionKey(69), 0 } },
        { { DIK_1, RawInputToActionKey(74), 0 } },
        { { DIK_2, RawInputToActionKey(75), 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_TAB, DIK_BACKSLASH, 0 } },
        { { DIK_T, 0, 0 } },
        { { DIK_Y, 0, 0 } },
        { { DIK_F5, 0, 0 } },
        { { DIK_F6, 0, 0 } },
        { { DIK_F7, 0, 0 } },
        { { DIK_F8, 0, 0 } },
        { { DIK_F9, 0, 0 } },
        { { DIK_F10, 0, 0 } },
        { { DIK_F11, 0, 0 } },
        { { DIK_F12, 0, 0 } },
        { { DIK_M, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { RawInputToActionKey(64), 0, 0 } },
        { { RawInputToActionKey(65), 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
        { { DIK_W, DIK_UP, 0 } },
        { { DIK_S, DIK_DOWN, 0 } },
        { { DIK_D, DIK_RIGHT, 0 } },
        { { DIK_A, DIK_LEFT, 0 } },
        { { 0, 0, 0 } },
        { { 0, 0, 0 } },
    },
};

// FUNCTION: SWBF2 0x004149c0
RawControllerInputs::RawControllerInputs()
{

    mProcessAfterReading = 0;
    mProcessDestination = 0;
    memcpy(mCurBindings, sDefaultBindings, sizeof(sDefaultBindings));
    memcpy(mCurKeyboardBindings, sDefaultKeyboardBindings, sizeof(sDefaultKeyboardBindings));
    memset(mLockedRawInputBool, 0, 0x4c);
    _redJoystick[0] = 0;
    _redJoystick[1] = 0;
    _redJoystick[2] = 0;
    _redJoystick[3] = 0;
    _redJoystick[4] = 0;
    _redJoystick[5] = 0;
    _redJoystick[6] = 0;
    _redJoystick[7] = 0;
    _redJoystick[8] = 0;
    _redJoystick[9] = 0;
    mLCDPanel = 0;
    _redMouse = 0;
    _redKeyboard = 0;
    for (int i = 0; i < 5; i++)
    {
        *(double*)mMouseSensitivity[i] = 1.0;
        mJoystickSensitivityX[i] = 4;
        mJoystickSensitivityY[i] = 4;
        mMouseAssist[i] = 1;
        mFlip[i] = (i == 2);
    }
    mScreenshotButtonState[0] = false;
    mScreenshotButtonState[1] = false;
    m_ChatMode = 0;
    m_ChatLength = 0;
    memset(m_CurrKeyState, 0, 0x90);
    memset(m_LastKeyState, 0, 0x90);
    memset(m_ChatWords, 0, 0x80);
    memset(m_KeyDownTime, 0, 0x90);
    mMouseOldXTimer = 0;
    mMouseOldYTimer = 0;
    memset(mMouseOldX, 0, sizeof(mMouseOldX));
    memset(mMouseOldY, 0, sizeof(mMouseOldY));
    mLastMouseDeltaX = 0;
    mLastMouseDeltaY = 0;
}

// GLOBAL: SWBF2 0x00787f00
extern const int RedKeyTo_BindID[144] =
{
    DIK_ESCAPE, DIK_1, DIK_2, DIK_3,
    DIK_4, DIK_5, DIK_6, DIK_7,
    DIK_8, DIK_9, DIK_0, DIK_MINUS,
    DIK_EQUALS, DIK_BACK, DIK_TAB, DIK_Q,
    DIK_W, DIK_E, DIK_R, DIK_T,
    DIK_Y, DIK_U, DIK_I, DIK_O,
    DIK_P, DIK_LBRACKET, DIK_RBRACKET, DIK_RETURN,
    DIK_LCONTROL, DIK_A, DIK_S, DIK_D,
    DIK_F, DIK_G, DIK_H, DIK_J,
    DIK_K, DIK_L, DIK_SEMICOLON, DIK_APOSTROPHE,
    DIK_GRAVE, DIK_LSHIFT, DIK_BACKSLASH, DIK_Z,
    DIK_X, DIK_C, DIK_V, DIK_B,
    DIK_N, DIK_M, DIK_COMMA, DIK_PERIOD,
    DIK_SLASH, DIK_RSHIFT, DIK_MULTIPLY, DIK_LMENU,
    DIK_SPACE, DIK_CAPITAL, DIK_F1, DIK_F2,
    DIK_F3, DIK_F4, DIK_F5, DIK_F6,
    DIK_F7, DIK_F8, DIK_F9, DIK_F10,
    DIK_NUMLOCK, DIK_SCROLL, DIK_NUMPAD7, DIK_NUMPAD8,
    DIK_NUMPAD9, DIK_SUBTRACT, DIK_NUMPAD4, DIK_NUMPAD5,
    DIK_NUMPAD6, DIK_ADD, DIK_NUMPAD1, DIK_NUMPAD2,
    DIK_NUMPAD3, DIK_NUMPAD0, DIK_DECIMAL, DIK_OEM_102,
    DIK_F11, DIK_F12, DIK_F13, DIK_F14,
    DIK_F15, DIK_KANA, DIK_ABNT_C1, DIK_CONVERT,
    DIK_NOCONVERT, DIK_YEN, DIK_ABNT_C2, DIK_NUMPADEQUALS,
    DIK_PREVTRACK, DIK_AT, DIK_COLON, DIK_UNDERLINE,
    DIK_KANJI, DIK_STOP, DIK_AX, DIK_UNLABELED,
    DIK_NEXTTRACK, DIK_NUMPADENTER, DIK_RCONTROL, DIK_MUTE,
    DIK_CALCULATOR, DIK_PLAYPAUSE, DIK_MEDIASTOP, DIK_VOLUMEDOWN,
    DIK_VOLUMEUP, DIK_WEBHOME, DIK_NUMPADCOMMA, DIK_DIVIDE,
    DIK_SYSRQ, DIK_RMENU, DIK_PAUSE, DIK_HOME,
    DIK_UP, DIK_PRIOR, DIK_LEFT, DIK_RIGHT,
    DIK_END, DIK_DOWN, DIK_NEXT, DIK_INSERT,
    DIK_DELETE, DIK_LWIN, DIK_RWIN, DIK_APPS,
    DIK_POWER, DIK_SLEEP, DIK_WAKE, DIK_WEBSEARCH,
    DIK_WEBFAVORITES, DIK_WEBREFRESH, DIK_WEBSTOP, DIK_WEBFORWARD,
    DIK_WEBBACK, DIK_MYCOMPUTER, DIK_MAIL, DIK_MEDIASELECT
};

// GLOBAL: SWBF2 0x007eb000
UiBindKeys RawControllerInputs::s_defUIBindings[76] =
{
    { eGUIINPUT_Accept, eGUIINPUT_NONE },
    { eGUIINPUT_Accept, eGUIINPUT_NONE },
    { eGUIINPUT_Accept, eGUIINPUT_NONE },
    { eGUIINPUT_Accept, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Accept, eGUIINPUT_NONE },
    { eGUIINPUT_Misc1, eGUIINPUT_NONE },
    { eGUIINPUT_LeftTrigger, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Up, eGUIINPUT_NONE },
    { eGUIINPUT_Down, eGUIINPUT_NONE },
};

// GLOBAL: SWBF2 0x007eb260
UiBindKeys RawControllerInputs::sKeyboardGuiBindings[144] =
{
    { eGUIINPUT_Back, eGUIINPUT_Start },
    { eGUIINPUT_Misc1, eGUIINPUT_NONE },
    { eGUIINPUT_Misc2, eGUIINPUT_NONE },
    { eGUIINPUT_LeftTrigger, eGUIINPUT_NONE },
    { eGUIINPUT_RightTrigger, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeMoveSlower, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeMoveFaster, eGUIINPUT_NONE },
    { eGUIINPUT_Select, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Up, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeTurnSlower, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeTurnFaster, eGUIINPUT_NONE },
    { eGUIINPUT_Accept, eGUIINPUT_FindPlayer },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Left, eGUIINPUT_NONE },
    { eGUIINPUT_Down, eGUIINPUT_NONE },
    { eGUIINPUT_Right, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeReset, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Select, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Misc1, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_Accept, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeUp, eGUIINPUT_NONE },
    { eGUIINPUT_Up, eGUIINPUT_NONE },
    { eGUIINPUT_LeftTrigger, eGUIINPUT_NONE },
    { eGUIINPUT_Left, eGUIINPUT_NONE },
    { eGUIINPUT_Right, eGUIINPUT_NONE },
    { eGUIINPUT_FreeEyeDown, eGUIINPUT_NONE },
    { eGUIINPUT_Down, eGUIINPUT_NONE },
    { eGUIINPUT_RightTrigger, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
    { eGUIINPUT_NONE, eGUIINPUT_NONE },
};

// GLOBAL: SWBF2 0x01e2fd10
bool gInvertMouse;

// GLOBAL: SWBF2 0x01e2fd11
bool chatCursorOn;

// GLOBAL: SWBF2 0x007e5880
unsigned short chatCursor[2] = { 'I', 0 };

// GLOBAL: SWBF2 0x007e5884
char chatCursorCurTime = 10;

// FUNCTION: SWBF2 0x004149b0
unsigned char __cdecl ActionKeyToRawInput(unsigned short usActionKey)
{
    return (unsigned char)((usActionKey >> 8) - 1);
}

// FUNCTION: SWBF2 0x00414b80
void RawControllerInputs::ReadLatest(float fDeltaTime)
{
    double& mMouseOldX = *(double*)this->mMouseOldX;
    double& mMouseOldY = *(double*)this->mMouseOldY;
    int& iLastDX = mLastMouseDeltaX;
    int& iLastDY = mLastMouseDeltaY;

    bool bAnyInput = false;

    memcpy(mLastRawInputFloat, mRawInputFloat, 10 * 76 * sizeof(float));
    memset(mRawInputFloat, 0, 10 * 76 * sizeof(float));
    m_pGuiInputs->Clear();

    for (int iPort = 0; iPort < 10; iPort++)
    {
        RedJoystick* pJoystick = _redJoystick[iPort];
        if (!pJoystick)
        {
            continue;
        }

        const unsigned __int64 uiButtons = pJoystick->m_uiDownButtons;
        for (unsigned int i = 0; i < 48; i++)
        {
            if (uiButtons & ((unsigned __int64)1 << i))
            {
                bAnyInput = true;
                mRawInputFloat[iPort][i] = 1.0f;
            }
        }

        for (int iAxis = 0; iAxis < 8; iAxis++)
        {
            float fAxis = pJoystick->m_fAxis[iAxis];
            if (fabs(fAxis) < 1.0f)
            {
                if (fAxis < -1.0f)
                {
                    fAxis = -1.0f;
                }
                else if (fAxis > 1.0f)
                {
                    fAxis = 1.0f;
                }
            }
            mRawInputFloat[iPort][48 + 2 * iAxis] = fAxis;
            mRawInputFloat[iPort][49 + 2 * iAxis] = -fAxis;
        }

        if (!mJoystickSensitivityX[mMode])
        {
            mJoystickSensitivityX[mMode] = 4;
        }
        if (!mJoystickSensitivityY[mMode])
        {
            mJoystickSensitivityY[mMode] = 4;
        }
    }

    if (_redMouse)
    {
        const unsigned int uiMouseButtons = _redMouse->m_uiDownButtons;
        for (int i = 0; i < 8; i++)
        {
            if (uiMouseButtons & (1 << i))
            {
                mRawInputFloat[0][66 + i] = 1.0f;
                bAnyInput = true;
            }
        }

        int iDX = _redMouse->m_dX;
        int iDY = _redMouse->m_dY;
        if (iDX == 0 && iDY == 0 && (iLastDX != 0 || iLastDY != 0))
        {
            iDX = iLastDX;
            iDY = iLastDY;
            iLastDX = 0;
            iLastDY = 0;
        }
        else
        {
            iLastDX = iDX;
            iLastDY = iDY;
        }

        double dY = -iDY;
        if (gInvertMouse)
        {
            dY = -dY;
        }
        const double dSensitivity = *(double*)mMouseSensitivity[mMode] * (1.0 / 2048.0);
        double dX = iDX / (double)fDeltaTime;
        dY = dY / (double)fDeltaTime;

        if (mMouseAssist[mMode] && (mMode == 1 || mMode == 2 || mMode == 4) &&
            GameState::GetState() == 0x8FF60339 && !PauseMenu::GetPaused(0))
        {
            const double dThreshold = (mMode == 2) ? 1.0 : 1000.0;

            const double dOldX = mMouseOldX;
            if (fabs(dOldX) > dThreshold)
            {
                if (dOldX * dX >= 0.0)
                {
                    mMouseOldXTimer += fDeltaTime;
                    if (fabs(dOldX) > fabs(dX) && mMouseOldXTimer > 0.1f)
                    {
                        dX = dOldX;
                    }
                }
                else
                {
                    dX = dX * 2.0 + dOldX;
                    if (mMouseOldX * dX > 0.0)
                    {
                        mMouseOldXTimer += fDeltaTime;
                    }
                    else
                    {
                        dX = 0.0;
                        mMouseOldXTimer = 0.0f;
                    }
                }
            }
            else
            {
                mMouseOldXTimer = 0.0f;
            }

            const double dOldY = mMouseOldY;
            if (fabs(dOldY) > dThreshold)
            {
                if (dOldY * dY >= 0.0)
                {
                    mMouseOldYTimer += fDeltaTime;
                    if (fabs(dOldY) > fabs(dY) && mMouseOldYTimer > 0.1f)
                    {
                        dY = dOldY;
                    }
                }
                else
                {
                    dY = dY * 2.0 + dOldY;
                    if (mMouseOldY * dY > 0.0)
                    {
                        mMouseOldYTimer += fDeltaTime;
                    }
                    else
                    {
                        dY = 0.0;
                        mMouseOldYTimer = 0.0f;
                    }
                }
            }
            else
            {
                mMouseOldYTimer = 0.0f;
            }

            mMouseOldX = dX;
            mMouseOldY = dY;
            if (fabs(dX) * dSensitivity > 1.1)
            {
                mMouseOldX = (dX > 0.0 ? 1.1 : -1.1) / dSensitivity;
            }
            if (fabs(dY) * dSensitivity > 1.1)
            {
                mMouseOldY = (dY > 0.0 ? 1.1 : -1.1) / dSensitivity;
            }
        }
        else
        {
            mMouseOldXTimer = 0.0f;
            mMouseOldX = 0.0;
            mMouseOldY = 0.0;
            mMouseOldYTimer = 0.0f;
        }

        mRawInputFloat[0][64] = (float)(dSensitivity * dX);
        mRawInputFloat[0][65] = (float)(dSensitivity * dY);

        if (_redMouse->m_dZ > 0)
        {
            mRawInputFloat[0][74] = 1.0f;
        }
        else if (_redMouse->m_dZ < 0)
        {
            mRawInputFloat[0][75] = 1.0f;
        }
    }
    else
    {
        mProcessDestination->mJoyUsage = true;
    }

    if (_redKeyboard && _redKeyboard->IsKeyDown(DIK_ESCAPE))
    {
        bAnyInput = true;
    }

    if (mProcessAfterReading && mProcessDestination)
    {
        mProcessDestination->Clear();
    }

    if (_redKeyboard && _redKeyboard->m_connected)
    {
        if (NetGame::IsEnabled())
        {
            ChatInputProcess();
        }
        else
        {
            StandardInputProcess();
        }
    }

    for (int i = 64; i < 76; i++)
    {
        m_pGuiInputs->Set(s_defUIBindings[i].mKeyA, mRawInputFloat[0][i]);
        m_pGuiInputs->Set(s_defUIBindings[i].mKeyB, mRawInputFloat[0][i]);
    }

    for (int iPort = 0; iPort < 10; iPort++)
    {
        if (_redJoystick[iPort])
        {
            for (int i = 0; i < 64; i++)
            {
                m_pGuiInputs->Set(s_defUIBindings[i].mKeyA, mRawInputFloat[iPort][i]);
                m_pGuiInputs->Set(s_defUIBindings[i].mKeyB, mRawInputFloat[iPort][i]);
            }
        }
    }

    m_pGuiInputs->Process();

    if (_redMouse)
    {
        m_pGuiInputs->mMouseCoords.mPosX = _redMouse->m_X;
        m_pGuiInputs->mMouseCoords.mPosY = _redMouse->m_Y;
    }

    if (bAnyInput)
    {
        StopInGameMoviePlay();
    }
}

// FUNCTION: SWBF2 0x004153c0
void RawControllerInputs::StandardInputProcess()
{

    bool bLocked = false;

    for (int i = 0; i < 144; i++)
    {
        if (_redKeyboard->IsKeyDown((unsigned char)RedKeyTo_BindID[i]))
        {
            m_pGuiInputs->Set(sKeyboardGuiBindings[i].mKeyA, 1.0f);
            m_pGuiInputs->Set(sKeyboardGuiBindings[i].mKeyB, 1.0f);
        }
    }

    bool bScreenshot = true;
    for (int iAction = 0; iAction < 43; iAction++)
    {
        const ActionKey& key = mCurKeyboardBindings[mMode][iAction];
        const unsigned char* pucPort = (const unsigned char*)&key.mKey[2];
        for (int j = 0; j < 2; j++)
        {
            if (key.mKey[j] == DIK_SYSRQ && pucPort[j] == 0)
            {
                bScreenshot = false;
                break;
            }
        }
    }
    if (bScreenshot)
    {
        bScreenshot = _redKeyboard->IsKeyDown(DIK_SYSRQ);
    }
    const bool bScreenshotPressed = !mScreenshotButtonState[0] && bScreenshot;
    mScreenshotButtonState[0] = bScreenshot;
    mScreenshotButtonState[1] = bScreenshotPressed;

    for (int iAction = 0; iAction < 43; iAction++)
    {
        for (int j = 0; j < 2; j++)
        {
            const ActionKey& key = mCurKeyboardBindings[mMode][iAction];
            const unsigned short usKey = key.mKey[j];
            if (usKey > 0xff)
            {
                const unsigned char ucRaw = ActionKeyToRawInput(usKey);
                const int iRaw = ucRaw;
                unsigned char ucPort;
                if (iRaw > -1 && ucRaw < 64)
                {
                    ucPort = ((const unsigned char*)&key.mKey[2])[j];
                }
                else
                {
                    ucPort = 0;
                }

                bool bCancelAssist = false;
                float fValue;
                float fLastValue;
                if (ucRaw < 64 && (iAction == ePROCESSEDINPUT_TURN_AXIS ||
                                   iAction == ePROCESSEDINPUT_TURN_POS ||
                                   iAction == ePROCESSEDINPUT_TURN_NEG))
                {
                    fValue = AdjustValueForSensitivity(mRawInputFloat[ucPort][iRaw], mJoystickSensitivityX[mMode]);
                    fLastValue = AdjustValueForSensitivity(mLastRawInputFloat[ucPort][iRaw], mJoystickSensitivityX[mMode]);
                    if (fValue < -0.4 || fValue > 0.4)
                    {
                        bCancelAssist = true;
                    }
                }
                else if (ucRaw < 64 && (iAction == ePROCESSEDINPUT_PITCH_AXIS ||
                                        iAction == ePROCESSEDINPUT_PITCH_POS ||
                                        iAction == ePROCESSEDINPUT_PITCH_NEG))
                {
                    fValue = AdjustValueForSensitivity(mRawInputFloat[ucPort][iRaw], mJoystickSensitivityY[mMode]);
                    fLastValue = AdjustValueForSensitivity(mLastRawInputFloat[ucPort][iRaw], mJoystickSensitivityY[mMode]);
                    if (fValue < -0.4 || fValue > 0.4)
                    {
                        bCancelAssist = true;
                    }
                }
                else
                {
                    fValue = mRawInputFloat[ucPort][iRaw];
                    fLastValue = mLastRawInputFloat[ucPort][iRaw];
                }

                if (mFlip[mMode] && iAction == ePROCESSEDINPUT_PITCH_AXIS)
                {
                    fValue *= -1.0f;
                    fLastValue *= -1.0f;
                }

                if (bCancelAssist)
                {
                    mMouseOldXTimer = 0.0f;
                    *(double*)mMouseOldX = 0.0;
                    *(double*)mMouseOldY = 0.0;
                    mMouseOldYTimer = 0.0f;
                }

                if (fValue != fLastValue)
                {
                    if (ucRaw < 64)
                    {
                        mProcessDestination->bits.mMouseMode = 0;
                    }
                    else if (ucRaw < 76)
                    {
                        mProcessDestination->bits.mMouseMode = 2;
                    }
                }

                mProcessDestination->Set((ePROCESSEDINPUT_TYPE)iAction,
                                         mLockedRawInputBool[ucPort][iRaw], fValue, fLastValue);
            }
            else if (_redKeyboard->IsKeyDown((unsigned char)usKey))
            {
                if (iAction == ePROCESSEDINPUT_TURN_AXIS || iAction == ePROCESSEDINPUT_TURN_POS ||
                    iAction == ePROCESSEDINPUT_TURN_NEG || iAction == ePROCESSEDINPUT_PITCH_AXIS ||
                    iAction == ePROCESSEDINPUT_PITCH_POS || iAction == ePROCESSEDINPUT_PITCH_NEG)
                {
                    mMouseOldXTimer = 0.0f;
                    *(double*)mMouseOldX = 0.0;
                    *(double*)mMouseOldY = 0.0;
                    mMouseOldYTimer = 0.0f;
                }
                mProcessDestination->Set((ePROCESSEDINPUT_TYPE)iAction, bLocked, 1.0f, 0.0f);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00415750
void RawControllerInputs::ChatInputProcess()
{

    if (m_ChatMode)
    {
        RedColor color = RedColor::YELLOW;
        if (m_ChatMode == 2)
        {
            color = RedColor::GREEN;
        }

        memcpy(m_LastKeyState, m_CurrKeyState, 144);
        for (unsigned char i = 0; i < 144; i++)
        {
            if (_redKeyboard->IsKeyDown((unsigned char)RedKeyTo_BindID[i]))
            {
                m_CurrKeyState[i] = true;
                if (m_LastKeyState[i] == true && ++m_KeyDownTime[i] > 10)
                {
                    m_LastKeyState[i] = false;
                    m_KeyDownTime[i] = 9;
                }
            }
            else
            {
                m_CurrKeyState[i] = false;
                m_KeyDownTime[i] = 0;
            }
            ChatKeyProcess(i);
        }

        if (--chatCursorCurTime < 0)
        {
            chatCursorCurTime = 10;
            chatCursorOn = !chatCursorOn;
        }

        if (chatCursorOn)
        {
            unsigned short ausLine[128] = { 0 };
            PblUnicode::ustrlcpy(ausLine, m_ChatWords, 128);
            PblUnicode::ustrlcat(ausLine, chatCursor, 128);
            ChatMessageDisplay::sInstance->AddMessage(ausLine, color);
        }
        else
        {
            ChatMessageDisplay::sInstance->AddMessage(m_ChatWords, color);
        }
    }
    else
    {
        StandardInputProcess();
        TeamOrderInputProcess();

        memcpy(m_LastKeyState, m_CurrKeyState, 144);
        for (int i = 0; i < 144; i++)
        {
            m_CurrKeyState[i] = _redKeyboard->IsKeyDown((unsigned char)RedKeyTo_BindID[i]);
        }

        if (PauseMenu::GetPaused(0))
        {
            return;
        }

        const bool bChat = mProcessDestination->GetDigital(ePROCESSEDINPUT_chat);
        if (!bChat && !mProcessDestination->GetDigital(ePROCESSEDINPUT_friendChat))
        {
            return;
        }

        if (!netInShell)
        {
            m_ChatLength = 0;
            m_ChatMode = bChat ? 1 : 2;
            memset(m_ChatWords, 0, 128);
            ChatMessageDisplay::sInstance->Show(true, m_ChatMode);
        }
    }
}

// FUNCTION: SWBF2 0x00415b90
void RawControllerInputs::TeamOrderInputProcess()
{

    const unsigned int auiOrders[8] = {
        0x0e6cfcde, 0x0f6cfe71, 0x0c6cf9b8, 0x0d6cfb4b,
        0x126d032a, 0x136d04bd, 0x106d0004, 0x116d0197
    };

    for (int i = 0; i < 8; i++)
    {
        const unsigned char ucKey = (unsigned char)mCurKeyboardBindings[mMode][ePROCESSEDINPUT_Comm_Chat_0 + i].mKey[0];
        unsigned char ucIndex;
        for (ucIndex = 0; ucIndex < 144; ucIndex++)
        {
            if (RedKeyTo_BindID[ucIndex] == ucKey)
            {
                break;
            }
        }

        if (!m_LastKeyState[ucIndex] && m_CurrKeyState[ucIndex] == true)
        {
            NetGame::AddChatMessageToQue(false, NetGame::GetLocalIndex(0), 0, i, RedLocalizeDB::Find(auiOrders[i], false), 1);
            return;
        }
    }
}

// FUNCTION: SWBF2 0x004159e0
void RawControllerInputs::ChatKeyProcess(unsigned char ucIndex)
{
    const unsigned char ucKey = (unsigned char)RedKeyTo_BindID[ucIndex];
    const bool bDown = m_CurrKeyState[ucIndex] != 0;
    const bool bWasDown = m_LastKeyState[ucIndex] != 0;
    unsigned short& usChatMode = m_ChatMode;
    unsigned short* pusChat = m_ChatWords;
    unsigned char& ucLength = m_ChatLength;

    if (bDown && !bWasDown)
    {
        if (ucKey == 1)
        {
            usChatMode = 0;
            ucLength = 0;
            memset(pusChat, 0, 0x80);
            ChatMessageDisplay::sInstance->Show(false);
            PauseMenu::sEnablePause = false;
            return;
        }
        if (ucKey == 28 || ucKey == 0x9c || ucKey == 15 || ucKey == 29 ||
            ucKey == 42 || ucKey == 0x9d || ucKey == 54 || ucKey == 58)
        {
            return;
        }
        if (ucKey == 14)
        {
            if (ucLength)
            {
                --ucLength;
            }
            pusChat[ucLength] = 0;
        }
        else if (ucLength < 62)
        {
            pusChat[ucLength] = GetKeyboard()->pcGetAsciiValue(ucKey);
            if (pusChat[ucLength])
            {
                ++ucLength;
            }
        }
    }
    else if (!bDown && bWasDown && (ucKey == 28 || ucKey == 0x9c))
    {
        int iType = usChatMode == 1 ? mask_chat : 1;
        if (ucLength)
        {
            NetGame::AddChatMessageToQue(false, NetGame::GetLocalIndex(0), 0, -1, pusChat, iType);
        }
        usChatMode = 0;
        ucLength = 0;
        memset(pusChat, 0, 0x80);
        ChatMessageDisplay::sInstance->Show(false);
    }
}
