#include <string.h>

#include "pcRedInputManager.h"
#include "RedEngineFL/input/RedInputEventHandler.h"
#include "PebbleFL/Common/PblJournal.h"

void __cdecl PblTraceP(const char* Format, ...);

extern HWND hGlobalWnd;

char sAsciiCodeMap[256][2];
unsigned char sRedKeyMap[256];
float s_keyRepeatDelay;
float s_keyRepeatSpeed;

// FUNCTION: SWBF2 0x006c6230
void RedKeyboard::StaticInit()
{
    memset(sAsciiCodeMap, 0, sizeof(sAsciiCodeMap));
    memset(sRedKeyMap, 0, sizeof(sRedKeyMap));

    sAsciiCodeMap[DIK_1][0] = '1'; sAsciiCodeMap[DIK_1][1] = '!';
    sAsciiCodeMap[DIK_2][0] = '2'; sAsciiCodeMap[DIK_2][1] = '@';
    sAsciiCodeMap[DIK_3][0] = '3'; sAsciiCodeMap[DIK_3][1] = '#';
    sAsciiCodeMap[DIK_4][0] = '4'; sAsciiCodeMap[DIK_4][1] = '$';
    sAsciiCodeMap[DIK_5][0] = '5'; sAsciiCodeMap[DIK_5][1] = '%';
    sAsciiCodeMap[DIK_6][0] = '6'; sAsciiCodeMap[DIK_6][1] = '^';
    sAsciiCodeMap[DIK_7][0] = '7'; sAsciiCodeMap[DIK_7][1] = '&';
    sAsciiCodeMap[DIK_8][0] = '8'; sAsciiCodeMap[DIK_8][1] = '*';
    sAsciiCodeMap[DIK_9][0] = '9'; sAsciiCodeMap[DIK_9][1] = '(';
    sAsciiCodeMap[DIK_0][0] = '0'; sAsciiCodeMap[DIK_0][1] = ')';
    sAsciiCodeMap[DIK_MINUS][0] = '-'; sAsciiCodeMap[DIK_MINUS][1] = '_';
    sAsciiCodeMap[DIK_EQUALS][0] = '='; sAsciiCodeMap[DIK_EQUALS][1] = '+';
    sAsciiCodeMap[DIK_BACK][0] = '\b'; sAsciiCodeMap[DIK_BACK][1] = '\b';
    sAsciiCodeMap[DIK_TAB][0] = '\t'; sAsciiCodeMap[DIK_TAB][1] = '\t';
    sAsciiCodeMap[DIK_Q][0] = 'q'; sAsciiCodeMap[DIK_Q][1] = 'Q';
    sAsciiCodeMap[DIK_W][0] = 'w'; sAsciiCodeMap[DIK_W][1] = 'W';
    sAsciiCodeMap[DIK_E][0] = 'e'; sAsciiCodeMap[DIK_E][1] = 'E';
    sAsciiCodeMap[DIK_R][0] = 'r'; sAsciiCodeMap[DIK_R][1] = 'R';
    sAsciiCodeMap[DIK_T][0] = 't'; sAsciiCodeMap[DIK_T][1] = 'T';
    sAsciiCodeMap[DIK_Y][0] = 'y'; sAsciiCodeMap[DIK_Y][1] = 'Y';
    sAsciiCodeMap[DIK_U][0] = 'u'; sAsciiCodeMap[DIK_U][1] = 'U';
    sAsciiCodeMap[DIK_I][0] = 'i'; sAsciiCodeMap[DIK_I][1] = 'I';
    sAsciiCodeMap[DIK_O][0] = 'o'; sAsciiCodeMap[DIK_O][1] = 'O';
    sAsciiCodeMap[DIK_P][0] = 'p'; sAsciiCodeMap[DIK_P][1] = 'P';
    sAsciiCodeMap[DIK_LBRACKET][0] = '['; sAsciiCodeMap[DIK_LBRACKET][1] = '{';
    sAsciiCodeMap[DIK_RBRACKET][0] = ']'; sAsciiCodeMap[DIK_RBRACKET][1] = '}';

    sAsciiCodeMap[DIK_RETURN][0] = '\n'; sAsciiCodeMap[DIK_RETURN][1] = '\n';
    sAsciiCodeMap[DIK_A][0] = 'a'; sAsciiCodeMap[DIK_A][1] = 'A';
    sAsciiCodeMap[DIK_S][0] = 's'; sAsciiCodeMap[DIK_S][1] = 'S';
    sAsciiCodeMap[DIK_D][0] = 'd'; sAsciiCodeMap[DIK_D][1] = 'D';
    sAsciiCodeMap[DIK_F][0] = 'f'; sAsciiCodeMap[DIK_F][1] = 'F';
    sAsciiCodeMap[DIK_G][0] = 'g'; sAsciiCodeMap[DIK_G][1] = 'G';
    sAsciiCodeMap[DIK_H][0] = 'h'; sAsciiCodeMap[DIK_H][1] = 'H';
    sAsciiCodeMap[DIK_J][0] = 'j'; sAsciiCodeMap[DIK_J][1] = 'J';
    sAsciiCodeMap[DIK_K][0] = 'k'; sAsciiCodeMap[DIK_K][1] = 'K';
    sAsciiCodeMap[DIK_L][0] = 'l'; sAsciiCodeMap[DIK_L][1] = 'L';
    sAsciiCodeMap[DIK_SEMICOLON][0] = ';'; sAsciiCodeMap[DIK_SEMICOLON][1] = ':';
    sAsciiCodeMap[DIK_APOSTROPHE][0] = '\''; sAsciiCodeMap[DIK_APOSTROPHE][1] = '"';
    sAsciiCodeMap[DIK_GRAVE][0] = '`'; sAsciiCodeMap[DIK_GRAVE][1] = '~';
    sAsciiCodeMap[DIK_BACKSLASH][0] = '\\'; sAsciiCodeMap[DIK_BACKSLASH][1] = '|';
    sAsciiCodeMap[DIK_Z][0] = 'z'; sAsciiCodeMap[DIK_Z][1] = 'Z';
    sAsciiCodeMap[DIK_X][0] = 'x'; sAsciiCodeMap[DIK_X][1] = 'X';
    sAsciiCodeMap[DIK_C][0] = 'c'; sAsciiCodeMap[DIK_C][1] = 'C';
    sAsciiCodeMap[DIK_V][0] = 'v'; sAsciiCodeMap[DIK_V][1] = 'V';
    sAsciiCodeMap[DIK_B][0] = 'b'; sAsciiCodeMap[DIK_B][1] = 'B';
    sAsciiCodeMap[DIK_N][0] = 'n'; sAsciiCodeMap[DIK_N][1] = 'N';
    sAsciiCodeMap[DIK_M][0] = 'm'; sAsciiCodeMap[DIK_M][1] = 'M';
    sAsciiCodeMap[DIK_COMMA][0] = ','; sAsciiCodeMap[DIK_COMMA][1] = '<';
    sAsciiCodeMap[DIK_PERIOD][0] = '.'; sAsciiCodeMap[DIK_PERIOD][1] = '>';
    sAsciiCodeMap[DIK_SLASH][0] = '/'; sAsciiCodeMap[DIK_SLASH][1] = '?';
    sAsciiCodeMap[DIK_MULTIPLY][0] = '*'; sAsciiCodeMap[DIK_MULTIPLY][1] = '*';
    sAsciiCodeMap[DIK_SPACE][0] = ' '; sAsciiCodeMap[DIK_SPACE][1] = ' ';

    sAsciiCodeMap[DIK_NUMPAD7][0] = '7'; sAsciiCodeMap[DIK_NUMPAD7][1] = 0;
    sAsciiCodeMap[DIK_NUMPAD8][0] = '8'; sAsciiCodeMap[DIK_NUMPAD8][1] = 0;
    sAsciiCodeMap[DIK_NUMPAD9][0] = '9'; sAsciiCodeMap[DIK_NUMPAD9][1] = 0;
    sAsciiCodeMap[DIK_SUBTRACT][0] = '-'; sAsciiCodeMap[DIK_SUBTRACT][1] = '-';
    sAsciiCodeMap[DIK_NUMPAD4][0] = '4'; sAsciiCodeMap[DIK_NUMPAD4][1] = '4';
    sAsciiCodeMap[DIK_NUMPAD5][0] = '5'; sAsciiCodeMap[DIK_NUMPAD5][1] = '5';
    sAsciiCodeMap[DIK_NUMPAD6][0] = '6'; sAsciiCodeMap[DIK_NUMPAD6][1] = '6';
    sAsciiCodeMap[DIK_ADD][0] = '+'; sAsciiCodeMap[DIK_ADD][1] = '+';
    sAsciiCodeMap[DIK_NUMPAD1][0] = '1'; sAsciiCodeMap[DIK_NUMPAD1][1] = 0;
    sAsciiCodeMap[DIK_NUMPAD2][0] = '2'; sAsciiCodeMap[DIK_NUMPAD2][1] = 0;
    sAsciiCodeMap[DIK_NUMPAD3][0] = '3'; sAsciiCodeMap[DIK_NUMPAD3][1] = 0;
    sAsciiCodeMap[DIK_NUMPAD0][0] = '0'; sAsciiCodeMap[DIK_NUMPAD0][1] = '0';
    sAsciiCodeMap[DIK_DECIMAL][0] = '.'; sAsciiCodeMap[DIK_DECIMAL][1] = '.';
    sAsciiCodeMap[DIK_DIVIDE][0] = '/'; sAsciiCodeMap[DIK_DIVIDE][1] = '/';

    sRedKeyMap[DIK_ESCAPE] = DIK_ESCAPE; sRedKeyMap[DIK_1] = DIK_1; sRedKeyMap[DIK_2] = DIK_2;
    sRedKeyMap[DIK_3] = DIK_3; sRedKeyMap[DIK_4] = DIK_4; sRedKeyMap[DIK_5] = DIK_5;
    sRedKeyMap[DIK_6] = DIK_6; sRedKeyMap[DIK_7] = DIK_7; sRedKeyMap[DIK_8] = DIK_8;
    sRedKeyMap[DIK_9] = DIK_9; sRedKeyMap[DIK_0] = DIK_0; sRedKeyMap[DIK_MINUS] = DIK_MINUS;
    sRedKeyMap[DIK_EQUALS] = DIK_EQUALS; sRedKeyMap[DIK_BACK] = DIK_BACK;
    sRedKeyMap[DIK_TAB] = DIK_TAB; sRedKeyMap[DIK_Q] = DIK_Q; sRedKeyMap[DIK_W] = DIK_W;
    sRedKeyMap[DIK_E] = DIK_E; sRedKeyMap[DIK_R] = DIK_R; sRedKeyMap[DIK_T] = DIK_T;
    sRedKeyMap[DIK_Y] = DIK_Y; sRedKeyMap[DIK_U] = DIK_U; sRedKeyMap[DIK_I] = DIK_I;
    sRedKeyMap[DIK_O] = DIK_O; sRedKeyMap[DIK_P] = DIK_P;
    sRedKeyMap[DIK_LBRACKET] = DIK_LBRACKET; sRedKeyMap[DIK_RBRACKET] = DIK_RBRACKET;
    sRedKeyMap[DIK_RETURN] = DIK_RETURN; sRedKeyMap[DIK_LCONTROL] = DIK_LCONTROL;
    sRedKeyMap[DIK_A] = DIK_A; sRedKeyMap[DIK_S] = DIK_S; sRedKeyMap[DIK_D] = DIK_D;
    sRedKeyMap[DIK_F] = DIK_F; sRedKeyMap[DIK_G] = DIK_G; sRedKeyMap[DIK_H] = DIK_H;
    sRedKeyMap[DIK_J] = DIK_J; sRedKeyMap[DIK_K] = DIK_K; sRedKeyMap[DIK_L] = DIK_L;
    sRedKeyMap[DIK_SEMICOLON] = DIK_SEMICOLON; sRedKeyMap[DIK_APOSTROPHE] = DIK_APOSTROPHE;
    sRedKeyMap[DIK_GRAVE] = DIK_GRAVE; sRedKeyMap[DIK_LSHIFT] = DIK_LSHIFT;
    sRedKeyMap[DIK_BACKSLASH] = DIK_BACKSLASH; sRedKeyMap[DIK_Z] = DIK_Z;
    sRedKeyMap[DIK_X] = DIK_X; sRedKeyMap[DIK_C] = DIK_C; sRedKeyMap[DIK_V] = DIK_V;
    sRedKeyMap[DIK_B] = DIK_B; sRedKeyMap[DIK_N] = DIK_N; sRedKeyMap[DIK_M] = DIK_M;
    sRedKeyMap[DIK_COMMA] = DIK_COMMA; sRedKeyMap[DIK_PERIOD] = DIK_PERIOD;
    sRedKeyMap[DIK_SLASH] = DIK_SLASH; sRedKeyMap[DIK_RSHIFT] = DIK_RSHIFT;
    sRedKeyMap[DIK_MULTIPLY] = DIK_MULTIPLY; sRedKeyMap[DIK_LMENU] = DIK_LMENU;
    sRedKeyMap[DIK_SPACE] = DIK_SPACE; sRedKeyMap[DIK_CAPITAL] = DIK_CAPITAL;
    sRedKeyMap[DIK_F1] = DIK_F1; sRedKeyMap[DIK_F2] = DIK_F2; sRedKeyMap[DIK_F3] = DIK_F3;
    sRedKeyMap[DIK_F4] = DIK_F4; sRedKeyMap[DIK_F5] = DIK_F5; sRedKeyMap[DIK_F6] = DIK_F6;
    sRedKeyMap[DIK_F7] = DIK_F7; sRedKeyMap[DIK_F8] = DIK_F8; sRedKeyMap[DIK_F9] = DIK_F9;
    sRedKeyMap[DIK_F10] = DIK_F10; sRedKeyMap[DIK_NUMLOCK] = DIK_NUMLOCK;
    sRedKeyMap[DIK_SCROLL] = DIK_SCROLL; sRedKeyMap[DIK_NUMPAD7] = DIK_NUMPAD7;
    sRedKeyMap[DIK_NUMPAD8] = DIK_NUMPAD8; sRedKeyMap[DIK_NUMPAD9] = DIK_NUMPAD9;
    sRedKeyMap[DIK_SUBTRACT] = DIK_SUBTRACT; sRedKeyMap[DIK_NUMPAD4] = DIK_NUMPAD4;
    sRedKeyMap[DIK_NUMPAD5] = DIK_NUMPAD5; sRedKeyMap[DIK_NUMPAD6] = DIK_NUMPAD6;
    sRedKeyMap[DIK_ADD] = DIK_ADD; sRedKeyMap[DIK_NUMPAD1] = DIK_NUMPAD1;
    sRedKeyMap[DIK_NUMPAD2] = DIK_NUMPAD2; sRedKeyMap[DIK_NUMPAD3] = DIK_NUMPAD3;
    sRedKeyMap[DIK_NUMPAD0] = DIK_NUMPAD0; sRedKeyMap[DIK_DECIMAL] = DIK_DECIMAL;
    sRedKeyMap[DIK_OEM_102] = DIK_OEM_102; sRedKeyMap[DIK_F11] = DIK_F11;
    sRedKeyMap[DIK_F12] = DIK_F12; sRedKeyMap[DIK_F13] = DIK_F13; sRedKeyMap[DIK_F14] = DIK_F14;
    sRedKeyMap[DIK_F15] = DIK_F15; sRedKeyMap[DIK_KANA] = DIK_KANA;
    sRedKeyMap[DIK_ABNT_C1] = DIK_ABNT_C1; sRedKeyMap[DIK_CONVERT] = DIK_CONVERT;
    sRedKeyMap[DIK_NOCONVERT] = DIK_NOCONVERT; sRedKeyMap[DIK_YEN] = DIK_YEN;
    sRedKeyMap[DIK_ABNT_C2] = DIK_ABNT_C2; sRedKeyMap[DIK_NUMPADEQUALS] = DIK_NUMPADEQUALS;
    sRedKeyMap[DIK_PREVTRACK] = DIK_PREVTRACK; sRedKeyMap[DIK_AT] = DIK_AT;
    sRedKeyMap[DIK_COLON] = DIK_COLON; sRedKeyMap[DIK_UNDERLINE] = DIK_UNDERLINE;
    sRedKeyMap[DIK_KANJI] = DIK_KANJI; sRedKeyMap[DIK_STOP] = DIK_STOP;
    sRedKeyMap[DIK_AX] = DIK_AX; sRedKeyMap[DIK_UNLABELED] = DIK_UNLABELED;
    sRedKeyMap[DIK_NEXTTRACK] = DIK_NEXTTRACK; sRedKeyMap[DIK_NUMPADENTER] = DIK_NUMPADENTER;
    sRedKeyMap[DIK_RCONTROL] = DIK_RCONTROL; sRedKeyMap[DIK_MUTE] = DIK_MUTE;
    sRedKeyMap[DIK_CALCULATOR] = DIK_CALCULATOR; sRedKeyMap[DIK_PLAYPAUSE] = DIK_PLAYPAUSE;
    sRedKeyMap[DIK_MEDIASTOP] = DIK_MEDIASTOP; sRedKeyMap[DIK_VOLUMEDOWN] = DIK_VOLUMEDOWN;
    sRedKeyMap[DIK_VOLUMEUP] = DIK_VOLUMEUP; sRedKeyMap[DIK_WEBHOME] = DIK_WEBHOME;
    sRedKeyMap[DIK_NUMPADCOMMA] = DIK_NUMPADCOMMA; sRedKeyMap[DIK_DIVIDE] = DIK_DIVIDE;
    sRedKeyMap[DIK_SYSRQ] = DIK_SYSRQ; sRedKeyMap[DIK_RMENU] = DIK_RMENU;
    sRedKeyMap[DIK_PAUSE] = DIK_PAUSE; sRedKeyMap[DIK_HOME] = DIK_HOME;
    sRedKeyMap[DIK_UP] = DIK_UP; sRedKeyMap[DIK_PRIOR] = DIK_PRIOR;
    sRedKeyMap[DIK_LEFT] = DIK_LEFT; sRedKeyMap[DIK_RIGHT] = DIK_RIGHT;
    sRedKeyMap[DIK_END] = DIK_END; sRedKeyMap[DIK_DOWN] = DIK_DOWN;
    sRedKeyMap[DIK_NEXT] = DIK_NEXT; sRedKeyMap[DIK_INSERT] = DIK_INSERT;
    sRedKeyMap[DIK_DELETE] = DIK_DELETE; sRedKeyMap[DIK_LWIN] = DIK_LWIN;
    sRedKeyMap[DIK_RWIN] = DIK_RWIN; sRedKeyMap[DIK_APPS] = DIK_APPS;
    sRedKeyMap[DIK_POWER] = DIK_POWER; sRedKeyMap[DIK_SLEEP] = DIK_SLEEP;
    sRedKeyMap[DIK_WAKE] = DIK_WAKE; sRedKeyMap[DIK_WEBSEARCH] = DIK_WEBSEARCH;
    sRedKeyMap[DIK_WEBFAVORITES] = DIK_WEBFAVORITES;
    sRedKeyMap[DIK_WEBREFRESH] = DIK_WEBREFRESH; sRedKeyMap[DIK_WEBSTOP] = DIK_WEBSTOP;
    sRedKeyMap[DIK_WEBFORWARD] = DIK_WEBFORWARD; sRedKeyMap[DIK_WEBBACK] = DIK_WEBBACK;
    sRedKeyMap[DIK_MYCOMPUTER] = DIK_MYCOMPUTER; sRedKeyMap[DIK_MAIL] = DIK_MAIL;
    sRedKeyMap[DIK_MEDIASELECT] = DIK_MEDIASELECT;

    int iRepeatDelay = 0;
    SystemParametersInfoA(SPI_GETKEYBOARDDELAY, 0, &iRepeatDelay, 0);
    int iRepeatSpeed = 0;
    SystemParametersInfoA(SPI_GETKEYBOARDSPEED, 0, &iRepeatSpeed, 0);

    s_keyRepeatDelay = ((iRepeatDelay + 1) * 250) / 1000.0f;

    s_keyRepeatSpeed = (1000 / (iRepeatSpeed + 1)) / 1000.0f;
}

// FUNCTION: SWBF2 0x006c6140
void RedKeyboard::Init(unsigned char ucPortNum)
{
    m_portNum = ucPortNum;
    m_connected = false;
}

// FUNCTION: SWBF2 0x006c6160
void RedKeyboard::Insert()
{
    IDirectInput8A* pDI = RedInputManager::pcGetDIInterface();

    if (SUCCEEDED(pDI->CreateDevice(GUID_SysKeyboard, &pc_pKeyboardDevice, 0)))
    {

        pc_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);

        pc_pKeyboardDevice->SetCooperativeLevel(
            hGlobalWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY);
        pc_pKeyboardDevice->Acquire();
        m_connected = true;

        pc_curRepeatCode = -1;
    }
}

// FUNCTION: SWBF2 0x006c6100
unsigned short AnsiToUnicode(char cAnsi)
{
    MultiByteToWideChar(CP_ACP, 0, &cAnsi, 1, 0, 1);
    return 0;
}

// FUNCTION: SWBF2 0x006c67c0
unsigned char RedKeyboard::pcGetAsciiValue(unsigned int uiScanCode)
{

    unsigned short achChar[10];
    memset(achChar, 0, sizeof(achChar));

    UINT uVirtKey = MapVirtualKeyA(uiScanCode, MAPVK_VSC_TO_VK);

    BYTE abKeyState[256];
    GetKeyboardState(abKeyState);

    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    unsigned int uiChar = 0;
    int iCount;

    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        iCount = ToAscii(uVirtKey, uiScanCode, abKeyState, achChar, 0);
        if (iCount != 0)
        {
            uiChar = AnsiToUnicode((char)achChar[0]);
        }
    }
    else
    {
        iCount = ToUnicode(uVirtKey, uiScanCode, abKeyState, (LPWSTR)achChar, 10, 0);
        if (iCount > 0)
        {
            uiChar = achChar[iCount - 1];
        }
    }

    if (iCount != 0)
    {

        const unsigned char cChar = (unsigned char)uiChar;

        if (cChar >= 0xAC && cChar <= 0xB8)
        {
            if (cChar == 0xAC || cChar == 0xB1 || cChar == 0xB2
                || cChar == 0xB3 || cChar == 0xB4 || cChar == 0xB8)
            {
                return 0;
            }
        }

        else if (cChar == 0xFE)
        {
            return 0;
        }

        return cChar;
    }

    if (pc_mCapslockMode
        || (pc_DownKeys[DIK_LSHIFT] & 0x80)
        || (pc_DownKeys[DIK_RSHIFT] & 0x80))
    {
        return sAsciiCodeMap[uiScanCode][1];
    }

    return sAsciiCodeMap[uiScanCode][0];
}

// FUNCTION: SWBF2 0x006c6020
RedKeyboard::RedKeyboard()
{
    pc_pKeyboardDevice = 0;
    memset(pc_DownKeys, 0, sizeof(pc_DownKeys));
    memset(pc_PressedKeys, 0, sizeof(pc_PressedKeys));
    memset(pc_ReleasedKeys, 0, sizeof(pc_ReleasedKeys));
    pc_mCapslockMode = (GetKeyState(VK_CAPITAL) & 1) != 0;
}

// FUNCTION: SWBF2 0x006c6090
RedKeyboard::~RedKeyboard()
{
    if (pc_pKeyboardDevice)
    {
        pc_pKeyboardDevice->Unacquire();
        pc_pKeyboardDevice->Release();
        pc_pKeyboardDevice = 0;
    }
}

// FUNCTION: SWBF2 0x006c6590
void RedKeyboard::Update(float fDeltaTime)
{
    if (!m_connected)
    {
        return;
    }

    if (FAILED(pc_pKeyboardDevice->Poll()))
    {
        while (pc_pKeyboardDevice->Acquire() == DIERR_INPUTLOST)
            ;
        return;
    }

    unsigned char aucOldKeys[256];
    memcpy(aucOldKeys, pc_DownKeys, sizeof(aucOldKeys));
    memset(pc_DownKeys, 0, sizeof(pc_DownKeys));
    pc_pKeyboardDevice->GetDeviceState(sizeof(pc_DownKeys), pc_DownKeys);

    PblJournal::Get(PJT_FULL);

    const unsigned short usFlags = pcGetFlags();

    if (!(aucOldKeys[DIK_CAPITAL] & 0x80) && (pc_DownKeys[DIK_CAPITAL] & 0x80))
    {
        pc_mCapslockMode = !pc_mCapslockMode;
        PblTraceP("Flipping capslock mode to %d\n", pc_mCapslockMode);
    }

    for (int i = 0; i < 256; ++i)
    {
        if (!(aucOldKeys[i] & 0x80) && (pc_DownKeys[i] & 0x80))
        {
            pc_PressedKeys[i] = 0x80;
            const unsigned char ucAscii = pcGetAsciiValue(i);
            RedInputEventHandler::PostKeyboardEvent(KEYCHAR, this, sRedKeyMap[i],
                                                    ucAscii, usFlags);
            pc_curRepeatCode = (char)i;
            pc_keyRepeatElapsedTime = 0.0f;
            pc_nextKeyRepeatPress = s_keyRepeatDelay;
        }
        else if ((aucOldKeys[i] & 0x80) && !(pc_DownKeys[i] & 0x80))
        {
            pc_ReleasedKeys[i] = 0x80;
            if (pc_curRepeatCode == i)
            {
                pc_curRepeatCode = -1;
            }
        }
    }

    if (pc_curRepeatCode >= 0)
    {
        if (pc_keyRepeatElapsedTime > pc_nextKeyRepeatPress)
        {
            RedInputEventHandler::PostKeyboardEvent(
                KEYCHAR, this, sRedKeyMap[pc_curRepeatCode],
                pcGetAsciiValue(pc_curRepeatCode), usFlags);
            pc_nextKeyRepeatPress += s_keyRepeatSpeed;
        }
        pc_keyRepeatElapsedTime += fDeltaTime;
    }
}

// FUNCTION: SWBF2 0x006c6930
unsigned short RedKeyboard::pcGetFlags()
{
    unsigned short usFlags = 0;
    if ((pc_DownKeys[DIK_LSHIFT] & 0x80) || (pc_DownKeys[DIK_RSHIFT] & 0x80))
    {
        usFlags = 2;
    }
    if ((pc_DownKeys[DIK_LCONTROL] & 0x80) || (pc_DownKeys[DIK_RCONTROL] & 0x80))
    {
        usFlags |= 1;
    }
    if ((pc_DownKeys[DIK_LMENU] & 0x80) || (pc_DownKeys[DIK_RMENU] & 0x80))
    {
        usFlags |= 4;
    }
    if (pc_DownKeys[DIK_CAPITAL] & 0x80)
    {
        usFlags |= 8;
    }
    if (pc_DownKeys[DIK_NUMLOCK] & 0x80)
    {
        usFlags |= 0x10;
    }
    if (pc_DownKeys[DIK_SCROLL] & 0x80)
    {
        usFlags |= 0x20;
    }
    return usFlags;
}

// FUNCTION: SWBF2 0x006c6130
unsigned short RedKeyboard::GetFlags()
{
    return pcGetFlags();
}

// FUNCTION: SWBF2 0x006c69a0
bool RedKeyboard::pcGetKeyName(unsigned int uiKey, unsigned short* pusName, unsigned int uiSize)
{
    (void)uiSize;
    DIPROPSTRING dips;
    dips.diph.dwSize = sizeof(DIPROPSTRING);
    dips.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dips.diph.dwObj = uiKey;
    dips.diph.dwHow = DIPH_BYOFFSET;
    memset(dips.wsz, 0, sizeof(dips.wsz));

    HRESULT hr = pc_pKeyboardDevice->GetProperty(DIPROP_KEYNAME, &dips.diph);
    dips.wsz[MAX_PATH - 1] = 0;
    memcpy(pusName, dips.wsz, MAX_PATH - 1);
    if (hr != DI_OK)
    {
        return false;
    }

    PblJournal::Get(PJT_FULL);
    return true;
}

// FUNCTION: SWBF2 0x006c6120
void RedKeyboard::Cleanup()
{
    m_connected = false;
}

// FUNCTION: SWBF2 0x006c61c0
bool RedKeyboard::IsKeyDown(unsigned char ucKey)
{
    if (!ucKey)
    {
        return false;
    }
    return pc_DownKeys[ucKey] != 0;
}

// FUNCTION: SWBF2 0x006c61f0
void RedKeyboard::Remove()
{
    m_connected = false;
    if (pc_pKeyboardDevice)
    {
        pc_pKeyboardDevice->Unacquire();
        pc_pKeyboardDevice->Release();
        pc_pKeyboardDevice = 0;
    }
}
