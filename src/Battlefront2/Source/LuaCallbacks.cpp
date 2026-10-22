#include "LuaCallbacks.h"
#include "LuaHelper.h"
#include "LuaScript.h"
#include "RedEngineFL/Application/RedTerrain.h"

#include "LuaCallbacks_IFace.h"
#include "LuaCallbacks_LoadSave.h"
#include "LuaCallbacks_Mission.h"
#include "LuaCallbacks_Net.h"
#include "LuaCallbacks_Options.h"
#include "LuaCallbacks_PC.h"
#include "LuaCallbacks_Sound.h"
#include "LuaCallbacks_Stats.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "LuaFL/include/lauxlib.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "CameraManager.h"
#include "DownloadableContent.h"
#include "FLGuiManager.h"
#include "FLInputManager.h"
#include "GameLoop.h"
#include "GameState.h"
#include "MissionPlayList.h"
#include "PauseMenu.h"
#include "PlayerProfile.h"
#include "ShellLoop.h"

#include <d3dx9math.h>
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "FLJoystick.h"
#include "Team.h"
#include "ScreenTransition.h"
#include "SoldierAnimatorLowRes.h"
#include "SoldierAnimationData.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRedTerrain.h"
#include "NetGame.h"

// FUNCTION: SWBF2 0x0042ced0
const int Max(int iA, int iB)
{
    return iA > iB ? iA : iB;
}

namespace Lua_Callbacks {
void CleanupOptions();
}
namespace Lua_Callbacks {
void CleanupSound();
}

// FUNCTION: SWBF2 0x0072b390
void __cdecl printf_wrapper(const char*, ...)
{
}

namespace PblUnicode { unsigned int ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount); }
unsigned int __cdecl ConvertUnicodeSkippingWide(char* pDest, const unsigned short* pSrc, unsigned int uiCount);

// FUNCTION: SWBF2 0x0052a9b0
int FLInputManager::CountControllers() const
{
    int iConnected = 0;
    for (unsigned char i = 0; i < RedInputManager::GetNumJoystickPorts(); ++i)
    {
        if (RedInputManager::GetJoystick(i)->m_connected)
        {
            ++iConnected;
        }
    }
    return iConnected ? iConnected : 1;
}

// FUNCTION: SWBF2 0x005c5080
void NetGame::Reboot()
{
}

// FUNCTION: SWBF2 0x005805e0
int __fastcall GetScreenTransitionType(const char* pcName, void*)
{
    if (!pcName)
    {
        return 2;
    }
    switch ((unsigned int)PblHash(pcName))
    {
    case 0x6bfbd198u: return 1;
    case 0x02ba7498u: return 3;
    case 0xec70b044u: return 4;
    case 0x027fdd70u: return 5;
    case 0xf497cc4cu: return 6;
    case 0xca020c79u: return 7;
    default: return 2;
    }
}
extern bool gDemoMode;
extern bool gAttractMode;

namespace GameLoop { extern bool sShowStats; }
extern int gActiveViewport;
extern bool gDrawFlyerShadows;
extern HWND hGlobalWnd;

class NetSetup
{
public:
    static char* GetGameVersion();
    static char const* GetAutoNetScript();
    static bool IsAutoNetLan();
    static char const* GetAutoNetName();
    static int GetAutoNetPlayerCount();
    static void SetupAutoNetMissions();

    static void RequestQuit(unsigned int uiReason);
};

class NetSetupP
{
public:
    static void UpdateScreen();
};

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00580380
int DrawParticleAt(lua_State*)
{
    const PblMatrix mWorld = *(const PblMatrix*)lua_touserdata(LuaHelper::mState, 1);
    const char* pcTexture = LuaHelper::GetString(2);
    const float fWidth = LuaHelper::GetFloat(3);
    RedColor color;
    color.r = (unsigned char)(int)LuaHelper::GetFloat(4);
    color.g = (unsigned char)(int)LuaHelper::GetFloat(5);
    color.b = (unsigned char)(int)LuaHelper::GetFloat(6);
    color.a = (unsigned char)(int)LuaHelper::GetFloat(7);
    const float fRotation = LuaHelper::GetFloat(8);
    sDrawAllParticleAt->Add(mWorld.GetTranslation(), pcTexture, fWidth, color, fRotation);
    return 0;
}

// FUNCTION: SWBF2 0x00580490
int ScriptCB_EnableScene(lua_State*)
{
    const bool bEnable = lua_toboolean(LuaHelper::mState, 1) != 0;
    CameraManager::Instance()->SetNumCameras(GameLoop::mNumCameras);
    CameraManager::Instance()->EnableScene(bEnable);
    if (bEnable)
    {
        RedTerrain::Activate();
        RedRenderer::Flush();
    }
    else
    {
        RedTerrain::Deactivate();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005804f0
int ScriptCB_PostLoadHack(lua_State*)
{
    SoldierAnimatorLowResClass::PostLoad();
    SoldierAnimationData::DiscardLoadTimeData();
    sDrawAllParticleAt = new DrawAllParticleAt;
    sDrawAllBeamBetween = new DrawAllBeamBetween;
    return 0;
}

// FUNCTION: SWBF2 0x00580670
int BeginScreenTransition(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 6)
    {
        const unsigned int uScreen = (unsigned int)(int)LuaHelper::GetFloat(1);
        const float fInTime = LuaHelper::GetFloat(2);
        const float fHoldTime = LuaHelper::GetFloat(3);
        const float fOutTime = LuaHelper::GetFloat(4);
        const int iInType = GetScreenTransitionType(LuaHelper::GetString(5), 0);
        const int iOutType = GetScreenTransitionType(LuaHelper::GetString(6), 0);
        ScreenTransition::sInstance->BeginTransition(
            uScreen, fInTime, fHoldTime, fOutTime,
            (ScreenTransition::TransitionType)iInType, (ScreenTransition::TransitionType)iOutType, 0, 0);
    }
    return 0;
}

// GLOBAL: SWBF2 0x007e69e0
static const CallbackEntryT sCallbacks[91] =
{
    { "ScriptCB_DoFile", ScriptCB_DoFile },
    { "ScriptCB_GetPlatform", ScriptCB_GetPlatform },
    { "ScriptCB_GetLanguage", ScriptCB_GetLanguage },
    { "ScriptCB_FormatTimeString", ScriptCB_FormatTimeString },
    { "ScriptCB_GetVKeyboardCharacter", ScriptCB_GetVKeyboardCharacter },
    { "ScriptCB_IsLegalGamespyString", ScriptCB_IsLegalGamespyString },
    { "ScriptCB_SetTeamNames", ScriptCB_SetTeamNames },
    { "ScriptCB_SetMissionNames", ScriptCB_SetMissionNames },
    { "ScriptCB_GetMaxMissionQueue", ScriptCB_GetMaxMissionQueue },
    { "ScriptCB_tounicode", ScriptCB_tounicode },
    { "ScriptCB_ununicode", ScriptCB_ununicode },
    { "ScriptCB_UnicodeStrCat", ScriptCB_UnicodeStrCat },
    { "ScriptCB_getlocalizestr", ScriptCB_getlocalizestr },
    { "ScriptCB_usprintf", ScriptCB_usprintf },
    { "ScriptCB_ustrupper", ScriptCB_ustrupper },
    { "ScriptCB_srand", ScriptCB_srand },
    { "ScriptCB_random", ScriptCB_random },
    { "ScriptCB_GetDateStr", ScriptCB_GetDateStr },
    { "ScriptCB_GetShellActive", ScriptCB_GetShellActive },
    { "ScriptCB_SetAutoAcquireControllers", ScriptCB_SetAutoAcquireControllers },
    { "ScriptCB_ReadAllControllers", ScriptCB_ReadAllControllers },
    { "ScriptCB_UnbindController", ScriptCB_UnbindController },
    { "ScriptCB_GetPrimaryController", ScriptCB_GetPrimaryController },
    { "ScriptCB_GetSecondaryController", ScriptCB_GetSecondaryController },
    { "ScriptCB_IsControllerConnected", ScriptCB_IsControllerConnected },
    { "ScriptCB_IsControllerBound", ScriptCB_IsControllerBound },
    { "ScriptCB_SwapPrimaryController", ScriptCB_SwapPrimaryController },
    { "ScriptCB_SetHotController", ScriptCB_SetHotController },
    { "ScriptCB_RestorePrimaryController", ScriptCB_RestorePrimaryController },
    { "ScriptCB_SetIgnoreControllerRemoval", ScriptCB_SetIgnoreControllerRemoval },
    { "ScriptCB_GetNumControllers", ScriptCB_GetNumControllers },
    { "ScriptCB_GetMaxControllers", ScriptCB_GetMaxControllers },
    { "ScriptCB_ReadLeftstick", ScriptCB_ReadLeftstick },
    { "ScriptCB_GetAutoNetIsDedicated", ScriptCB_GetAutoNetIsDedicated },
    { "ScriptCB_GetAutoNetGameName", ScriptCB_GetAutoNetGameName },
    { "ScriptCB_GetAutoGetPlayerCount", ScriptCB_GetAutoGetPlayerCount },
    { "ScriptCB_SetupAutoNetMissions", ScriptCB_SetupAutoNetMissions },
    { "ScriptCB_SetupAutoNetIsLan", ScriptCB_SetupAutoNetIsLan },
    { "ScriptCB_GetAutoNetScript", ScriptCB_GetAutoNetScript },
    { "ScriptCB_SetQuitPlayer", ScriptCB_SetQuitPlayer },
    { "ScriptCB_GetQuitPlayer", ScriptCB_GetQuitPlayer },
    { "ScriptCB_ShouldShowLegal", ScriptCB_ShouldShowLegal },
    { "ScriptCB_PauseDemoTimer", ScriptCB_PauseDemoTimer },
    { "ScriptCB_ShouldShowDemoPostscreen", ScriptCB_ShouldShowDemoPostscreen },
    { "ScriptCB_ShouldCheckDNAS", ScriptCB_ShouldCheckDNAS },
    { "ScriptCB_DoneCheckDNAS", ScriptCB_DoneCheckDNAS },
    { "ScriptCB_HasSeenDNASEULA", ScriptCB_HasSeenDNASEULA },
    { "ScriptCB_SetHasSeenDNASEULA", ScriptCB_SetHasSeenDNASEULA },
    { "ScriptCB_ShouldCheckPatch", ScriptCB_ShouldCheckPatch },
    { "ScriptCB_DoneCheckPatch", ScriptCB_DoneCheckPatch },
    { "ScriptCB_IndexMultipageText", ScriptCB_IndexMultipageText },
    { "ScriptCB_ShowMultipageText", ScriptCB_ShowMultipageText },
    { "ScriptCB_SetDCMap", ScriptCB_SetDCMap },
    { "ScriptCB_HideDownloadableContent", ScriptCB_HideDownloadableContent },
    { "ScriptCB_EnterMission", ScriptCB_EnterMission },
    { "ScriptCB_LaunchDemo", ScriptCB_LaunchDemo },
    { "ScriptCB_RestartMission", ScriptCB_RestartMission },
    { "ScriptCB_GetPausingViewport", ScriptCB_GetPausingViewport },
    { "ScriptCB_GetPausingJoystick", ScriptCB_GetPausingJoystick },
    { "ScriptCB_PausingIsPrimary", ScriptCB_PausingIsPrimary },
    { "ScriptCB_Reboot", ScriptCB_Reboot },
    { "ScriptCB_IsGameOver", ScriptCB_IsGameOver },
    { "ScriptCB_QuitToShell", ScriptCB_QuitToShell },
    { "ScriptCB_QuitToWindows", ScriptCB_QuitToWindows },
    { "ScriptCB_QuitFromStats", ScriptCB_QuitFromStats },
    { "ScriptCB_MinimizeWindow", ScriptCB_MinimizeWindow },
    { "ScriptCB_Unpause", ScriptCB_Unpause },
    { "ScriptCB_SkipToPlayerList", ScriptCB_SkipToPlayerList },
    { "ScriptCB_ResetSkipToPlayerList", ScriptCB_ResetSkipToPlayerList },
    { "ScriptCB_CheckPlayerListDone", ScriptCB_CheckPlayerListDone },
    { "ScriptCB_GetMissionTime", ScriptCB_GetMissionTime },
    { "ScriptCB_Freecamera", ScriptCB_Freecamera },
    { "ScriptCB_GetTeamName", ScriptCB_GetTeamName },
    { "ScriptCB_GetBuildStr", ScriptCB_GetBuildStr },
    { "ScriptCB_UpdateScreen", ScriptCB_UpdateScreen },
    { "ScriptCB_GetSPProgress", ScriptCB_GetSPProgress },
    { "ScriptCB_SetSPProgress", ScriptCB_SetSPProgress },
    { "ScriptCB_UnlockableState", ScriptCB_UnlockableState },
    { "ScriptCB_UnlockUnlockable", ScriptCB_UnlockUnlockable },
    { "ScriptCB_GetLastBattleVictory", ScriptCB_GetLastBattleVictory },
    { "ScriptCB_SetLastBattleVictoryValid", ScriptCB_SetLastBattleVictoryValid },
    { "ScriptCB_MetagameSetBonus", ScriptCB_MetagameSetBonus },
    { "ScriptCB_MetagameSetSide", ScriptCB_MetagameSetSide },
    { "SetVictoryMovie", Lua_Callbacks::ScriptCB_SetVictoryMovie },
    { "SetDefeatMovie", Lua_Callbacks::ScriptCB_SetDefeatMovie },
    { "ScriptCB_DisableFlyerShadows", ScriptCB_DisableFlyerShadows },
    { "DrawBeamBetween", DrawBeamBetween },
    { "DrawParticleAt", DrawParticleAt },
    { "ScriptCB_EnableScene", ScriptCB_EnableScene },
    { "ScriptCB_PostLoadHack", ScriptCB_PostLoadHack },
    { "BeginScreenTransition", BeginScreenTransition },
};

static const unsigned int SHELL_STATE = 0x11e1fc01;
static const unsigned int MISSION_STATE = 0x8ff60339;

// GLOBAL: SWBF2 0x01e57610
char sTempReturnString[256];

// GLOBAL: SWBF2 0x01e57410
unsigned short sTempReturnString16[256];

}

// GLOBAL: SWBF2 0x00789d60
extern const char sVKeyboardKeys[104] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '@', '.', '-',
    ' ', '#', '$', '%', '&', '(', ')', '[', ']', '*', '+', '\'', '"',
    '!', '=', '^', '{', '}', '_', '<', '>', ':', ';', '?', '`', '|',
    '/', ',', '~', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
};

// GLOBAL: SWBF2 0x007e6cbc
int gQuitPlayer = 1;

// GLOBAL: SWBF2 0x007e6cb8
int s_doDNAS = 1;

// GLOBAL: SWBF2 0x007e69dc
int s_doPatch = 1;

// GLOBAL: SWBF2 0x01f9c2f0
unsigned short* sPageStart[300];

// GLOBAL: SWBF2 0x01f9c7a0
char sAttractMap[32];

// GLOBAL: SWBF2 0x01e57710
int gSkipToPlayerListState;

// GLOBAL: SWBF2 0x01e573fc
float gSkipToPlayerListTime;

// GLOBAL: SWBF2 0x01f9c2ec
DrawAllBeamBetween* sDrawAllBeamBetween;

// GLOBAL: SWBF2 0x01f9c2e8
DrawAllParticleAt* sDrawAllParticleAt;

namespace GameLoop
{
// GLOBAL: SWBF2 0x007e6688
int mLastGameWinner = -1;

// GLOBAL: SWBF2 0x01e56014
int mMetagameSide[4];
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x0057dc80
void DoPeriodicLuaGC(bool bForce)
{
    static unsigned int uCount;

    if (bForce || (uCount & 0x3f) == 0)
    {
        lua_setgcthreshold(LuaHelper::mState, lua_getgccount(LuaHelper::mState));
    }
    ++uCount;
}

// FUNCTION: SWBF2 0x0057dcc0
int ScriptCB_DoFile(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        LuaScript::LoadScript(luaL_checkstring(LuaHelper::mState, 1));
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057dd10
int ScriptCB_GetPlatform(lua_State*)
{
    lua_pushstring(LuaHelper::mState, "PC");
    return 1;
}

// FUNCTION: SWBF2 0x0057dd30
int ScriptCB_GetLanguage(lua_State*)
{
    lua_pushstring(LuaHelper::mState, RedLocalizeDB::GetLanguageStr());
    lua_pushnumber(LuaHelper::mState, (float)RedLocalizeDB::GetLanguageID());
    return 2;
}

// FUNCTION: SWBF2 0x0057dd70
int ScriptCB_FormatTimeString(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }

    char acBuffer[64];
    int iTime = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iMinutes = iTime / 60;
    int iSeconds = iTime % 60;
    int iHours = iMinutes / 60;
    iMinutes %= 60;
    if (iHours)
    {
        _snprintf(acBuffer, sizeof(acBuffer), "%02d:%02d:%02d", iHours, iMinutes, iSeconds);
    }
    else
    {
        _snprintf(acBuffer, sizeof(acBuffer), "%02d:%02d", iMinutes, iSeconds);
    }
    PblUnicode::ustrncnv(sTempReturnString16, acBuffer, 256);
    LuaHelper::PushString(sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x0057de60
int ScriptCB_GetVKeyboardCharacter(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }

    unsigned int uKey = (int)luaL_checknumber(LuaHelper::mState, 1);
    sTempReturnString[1] = 0;
    if (uKey > 104)
    {
        uKey = 0;
    }
    sTempReturnString[0] = sVKeyboardKeys[uKey];
    lua_pushstring(LuaHelper::mState, sTempReturnString);
    return 1;
}

// FUNCTION: SWBF2 0x0057dee0
int ScriptCB_IsLegalGamespyString(lua_State*)
{
    static const char sBadFirstChars[] = "@+:#";

    bool bLegal = true;
    bool bResult = false;
    const char* pcError = "ifs.gsprofile.badchars";

    if (lua_gettop(LuaHelper::mState) != 3 || !lua_isstring(LuaHelper::mState, 1) ||
        !lua_isstring(LuaHelper::mState, 2) || !lua_isstring(LuaHelper::mState, 3))
    {
        return 0;
    }

    const char* apcStrings[3];
    apcStrings[0] = luaL_checklstring(LuaHelper::mState, 1, 0);
    apcStrings[1] = luaL_checklstring(LuaHelper::mState, 2, 0);
    apcStrings[2] = luaL_checklstring(LuaHelper::mState, 3, 0);

    for (int i = 0; i < 3; ++i)
    {
        unsigned int uLen = strlen(apcStrings[i]);
        if (uLen < 1 || (i == 0 && uLen < 3))
        {
            goto done;
        }

        bool bFound = false;
        for (unsigned int j = 0; j < uLen; ++j)
        {
            for (int k = 0; k < 104; ++k)
            {
                if (apcStrings[i][j] == sVKeyboardKeys[k])
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                bLegal = false;
                break;
            }
        }
    }
    if (!bLegal)
    {
        goto done;
    }

    if (apcStrings[0][0] >= '0' && apcStrings[0][0] <= '9')
    {
        pcError = "gamespy.errors.nick_nonumber";
        goto done;
    }
    for (unsigned int j = 0; j < strlen(apcStrings[0]); ++j)
    {
        if (apcStrings[0][j] == ' ')
        {
            pcError = "gamespy.errors.nick_nospaces";
            goto done;
        }
    }
    for (unsigned int j = 0; j < strlen(apcStrings[0]); ++j)
    {
        if (apcStrings[0][j] == '!')
        {
            pcError = "gamespy.errors.nick_validchars";
            goto done;
        }
    }
    for (unsigned int j = 0; j < strlen(apcStrings[0]); ++j)
    {
        if (apcStrings[0][j] == '\\')
        {
            pcError = "gamespy.errors.nick_validchars";
            goto done;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        for (unsigned int c = 0; c < sizeof(sBadFirstChars) - 1; ++c)
        {
            if (apcStrings[i][0] == sBadFirstChars[c])
            {
                bLegal = false;
                break;
            }
        }
    }
    if (!bLegal)
    {
        goto done;
    }

    {
        const char* pcAt = strchr(apcStrings[1] + 1, '@');
        if (pcAt && !strchr(pcAt + 1, '@') && pcAt[1] && pcAt[2] && strchr(pcAt + 2, '.'))
        {
            pcError = "";
            bResult = bLegal;
        }
        else
        {
            pcError = "ifs.gsprofile.bademail";
        }
    }

done:
    lua_pushboolean(LuaHelper::mState, bResult);
    lua_pushstring(LuaHelper::mState, pcError);
    return 2;
}

// FUNCTION: SWBF2 0x0057e140
int ScriptCB_tounicode(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    const char* pcString = luaL_checklstring(LuaHelper::mState, 1, 0);
    if (strlen(pcString) < 256)
    {
        PblUnicode::ustrncnv(sTempReturnString16, (const unsigned char*)pcString, 256);
    }
    else
    {
        PblUnicode::ustrncnv(sTempReturnString16, "BUFFER TOO SMALL", 256);
    }
    LuaHelper::PushString(sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x0057e1d0
int ScriptCB_UnicodeStrCat(lua_State*)
{
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop < 1)
    {
        return 0;
    }

    PblUnicode::ustrncpy(sTempReturnString16,
                         (const unsigned short*)luaL_checklstring(LuaHelper::mState, 1, 0), 256);
    for (int i = 2; i <= iTop; ++i)
    {
        PblUnicode::ustrlcat(sTempReturnString16,
                             (const unsigned short*)luaL_checklstring(LuaHelper::mState, i, 0), 256);
    }
    LuaHelper::PushString(sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x0057e260
int ScriptCB_ununicode(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1)
    {
        return 0;
    }

    if (!lua_isstring(LuaHelper::mState, 1))
    {
        lua_pushstring(LuaHelper::mState, "");
        return 1;
    }

    const unsigned short* pusString = (const unsigned short*)luaL_checklstring(LuaHelper::mState, 1, 0);
    if (PblUnicode::ustrlen(pusString) < 256)
    {
        ConvertUnicodeSkippingWide(sTempReturnString, pusString, 256);
    }
    else
    {
        _snprintf(sTempReturnString, 256, "BUFFER TOO SMALL");
    }
    lua_pushstring(LuaHelper::mState, sTempReturnString);
    return 1;
}

// FUNCTION: SWBF2 0x0057e310
int ScriptCB_getlocalizestr(lua_State*)
{
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop != 1 && iTop != 2)
    {
        return 0;
    }

    bool bOptional = iTop == 2;
    if (!lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    const char* pcKey = luaL_checklstring(LuaHelper::mState, 1, 0);
    const unsigned short* pusString = RedLocalizeDB::Find(PblHash(pcKey), bOptional);
    if (pusString)
    {
        LuaHelper::PushString(pusString);
        return 1;
    }
    if (bOptional)
    {
        return 0;
    }

    PblUnicode::ustrncnv(sTempReturnString16, pcKey, 255);
    LuaHelper::PushString(sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x0057e3d0
int ScriptCB_usprintf(lua_State*)
{
    int iTop = lua_gettop(LuaHelper::mState);
    if ((unsigned int)(iTop - 1) > 6)
    {
        return 0;
    }

    for (int i = 1; i <= iTop; ++i)
    {
        if (!lua_isstring(LuaHelper::mState, i))
        {
            return 0;
        }
    }

    const char* pcKey = luaL_checklstring(LuaHelper::mState, 1, 0);
    const unsigned short* apusArgs[7];
    for (int i = 2; i <= iTop; ++i)
    {
        if (!lua_isstring(LuaHelper::mState, i))
        {
            return 0;
        }
        apusArgs[i - 2] = (const unsigned short*)luaL_checklstring(LuaHelper::mState, i, 0);
    }

    const unsigned short* pusFormat = RedLocalizeDB::Find(pcKey ? (unsigned int)PblHash(pcKey) : 0, false);
    int iLen;
    if (!pusFormat)
    {
        iLen = PblUnicode::ustrncnv(sTempReturnString16, pcKey, 255);
    }
    else
    {
        switch (iTop)
        {
        case 2:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0]);
            break;
        case 3:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0], apusArgs[1]);
            break;
        case 4:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0], apusArgs[1],
                                         apusArgs[2]);
            break;
        case 5:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0], apusArgs[1],
                                         apusArgs[2], apusArgs[3]);
            break;
        case 6:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0], apusArgs[1],
                                         apusArgs[2], apusArgs[3], apusArgs[4]);
            break;
        case 7:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0], apusArgs[1],
                                         apusArgs[2], apusArgs[3], apusArgs[4], apusArgs[5]);
            break;
        case 8:
            iLen = PblUnicode::usnprintf(sTempReturnString16, 255, pusFormat, apusArgs[0], apusArgs[1],
                                         apusArgs[2], apusArgs[3], apusArgs[4], apusArgs[5], apusArgs[6]);
            break;
        default:
            PblUnicode::ustrncpy(sTempReturnString16, pusFormat, 255);
            iLen = PblUnicode::ustrlen(sTempReturnString16);
            break;
        }
    }
    lua_pushlstring(LuaHelper::mState, (const char*)sTempReturnString16, iLen * 2 + 2);
    return 1;
}

// FUNCTION: SWBF2 0x0057e650
int ScriptCB_ustrupper(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    PblUnicode::ustrncpy(sTempReturnString16,
                         (const unsigned short*)luaL_checklstring(LuaHelper::mState, 1, 0), 256);
    int iLen = PblUnicode::ustrlen(sTempReturnString16);
    for (int i = 0; i < iLen; ++i)
    {
        if (sTempReturnString16[i] >= 'a' && sTempReturnString16[i] <= 'z')
        {
            sTempReturnString16[i] -= 'a' - 'A';
        }
    }
    LuaHelper::PushString(sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x0057e6f0
int ScriptCB_srand(lua_State*)
{
    unsigned int uSeed = 0xdeadbeef;
    if (!lua_gettop(LuaHelper::mState))
    {
        srand((unsigned int)time(0) + GetTickCount());
        return 0;
    }
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        uSeed = (int)luaL_checknumber(LuaHelper::mState, 1);
    }
    srand(uSeed);
    return 0;
}

// FUNCTION: SWBF2 0x0057e780
int ScriptCB_random(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)rand() / 32767.0f);
    return 1;
}

// FUNCTION: SWBF2 0x0057e7b0
int ScriptCB_GetDateStr(lua_State*)
{
    memset(sTempReturnString, 0, sizeof(sTempReturnString));
    time_t tNow = time(0);
    strftime(sTempReturnString, 254, " %Y-%m-%d %H%M", localtime(&tNow));
    PblUnicode::ustrncnv(sTempReturnString16, sTempReturnString, 256);
    LuaHelper::PushString(sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x0057e820
int ScriptCB_GetShellActive(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    lua_pushboolean(LuaHelper::mState, GameState::GetState() == SHELL_STATE);
    return 1;
}

// FUNCTION: SWBF2 0x0057e850
int ScriptCB_SetAutoAcquireControllers(lua_State*)
{
    FLInputManager::Instance()->SetAutoAcquire(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x0057e870
int ScriptCB_ReadAllControllers(lua_State*)
{
    FLInputManager* pInput = FLInputManager::Instance();
    int iTop = lua_gettop(LuaHelper::mState);
    bool bReadAll = lua_toboolean(LuaHelper::mState, 1) != 0;
    bool bWasReadAll = pInput->GetReadAll();
    if (bReadAll)
    {
        pInput->SetAllowBindMultiple(iTop != 2);
    }
    pInput->SetReadAll(bReadAll);
    if (iTop == 3)
    {
        pInput->SetBindAll(true);
    }
    lua_pushboolean(LuaHelper::mState, bWasReadAll);
    return 1;
}

// FUNCTION: SWBF2 0x0057e8f0
static void UnbindOneController(FLInputManager* pInput, int iPlayer)
{
    if (iPlayer == 0)
    {
        pInput->UnbindController(iPlayer + 1);
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(iPlayer);
        if (pProfile)
        {
            pProfile->InitProfile(0);
        }
    }
}

// FUNCTION: SWBF2 0x0057e920
int ScriptCB_UnbindController(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        int iController = (int)luaL_checknumber(LuaHelper::mState, 1);
        FLInputManager* pInput = FLInputManager::Instance();
        if (iController >= 0)
        {
            UnbindOneController(pInput, iController - 1);
        }
        else if (iController == -1)
        {
            UnbindOneController(pInput, 0);
        }
        else if (iController == -2)
        {
            if (pInput->GetPrimaryPort() != 0)
            {
                UnbindOneController(pInput, 0);
            }
            if (!pInput->GetReadAll())
            {
                pInput->SetReadAll(true);
                pInput->SetReadAll(false);
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057e9e0
int ScriptCB_GetPrimaryController(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)FLInputManager::Instance()->GetPrimaryPort());
    return 1;
}

// FUNCTION: SWBF2 0x0057ea10
int ScriptCB_GetSecondaryController(lua_State*)
{
    int iPort = FLInputManager::Instance()->GetSecondaryPort();
    if (iPort < 0)
    {
        lua_pushnil(LuaHelper::mState);
    }
    else
    {
        lua_pushnumber(LuaHelper::mState, (float)iPort);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0057ea50
int ScriptCB_IsControllerConnected(lua_State*)
{
    unsigned char ucPort = (unsigned char)((int)luaL_optnumber(LuaHelper::mState, 1, 1.0f) - 1);
    bool bConnected = false;
    if (ucPort < RedInputManager::GetNumJoystickPorts())
    {
        bConnected = RedInputManager::GetJoystick(ucPort)->m_connected;
    }
    lua_pushboolean(LuaHelper::mState, bConnected);
    return 1;
}

// FUNCTION: SWBF2 0x0057eac0
int ScriptCB_IsControllerBound(lua_State*)
{
    int iPlayer = (int)luaL_optnumber(LuaHelper::mState, 1, 1.0f) - 1;
    lua_pushboolean(LuaHelper::mState, FLInputManager::Instance()->IsControllerBound(iPlayer));
    return 1;
}

// FUNCTION: SWBF2 0x0057eb10
int ScriptCB_SwapPrimaryController(lua_State*)
{
    FLInputManager::Instance()->SwapPrimarySecondary();
    return 0;
}

// FUNCTION: SWBF2 0x0057eb20
int ScriptCB_SetHotController(lua_State*)
{
    int iOldPrimary = FLInputManager::Instance()->GetPrimaryPort();
    int iController = -1;
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        iController = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    }
    FLInputManager::Instance()->SetHotController(iController);
    lua_pushnumber(LuaHelper::mState, (float)(iOldPrimary + 1));
    return 1;
}

// FUNCTION: SWBF2 0x0057eb90
int ScriptCB_RestorePrimaryController(lua_State*)
{
    FLInputManager::Instance()->RestorePrimarySecondary();
    return 0;
}

// FUNCTION: SWBF2 0x0057eba0
int ScriptCB_SetIgnoreControllerRemoval(lua_State*)
{
    FLInputManager::Instance()->SetIgnoreControllerRemoval(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x0057ebc0
int ScriptCB_GetNumControllers(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)FLInputManager::Instance()->CountControllers());
    return 1;
}

// FUNCTION: SWBF2 0x0057ebf0
int ScriptCB_GetMaxControllers(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x0057ec10
int ScriptCB_GetAutoNetIsDedicated(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, true);
    return 1;
}

// FUNCTION: SWBF2 0x0057ec30
int ScriptCB_GetAutoNetGameName(lua_State*)
{
    lua_pushstring(LuaHelper::mState, NetSetup::GetAutoNetName());
    return 1;
}

// FUNCTION: SWBF2 0x0057ec50
int ScriptCB_GetAutoGetPlayerCount(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)NetSetup::GetAutoNetPlayerCount());
    return 1;
}

// FUNCTION: SWBF2 0x0057ec80
int ScriptCB_SetupAutoNetMissions(lua_State*)
{
    NetSetup::SetupAutoNetMissions();
    return 0;
}

// FUNCTION: SWBF2 0x0057ec90
int ScriptCB_SetupAutoNetIsLan(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, NetSetup::IsAutoNetLan());
    return 1;
}

// FUNCTION: SWBF2 0x0057ecb0
int ScriptCB_GetAutoNetScript(lua_State*)
{
    lua_pushstring(LuaHelper::mState, NetSetup::GetAutoNetScript());
    return 1;
}

// FUNCTION: SWBF2 0x0057ecd0
int ScriptCB_ReadLeftstick(lua_State*)
{
    unsigned int uViewport;
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop == 0)
    {
        uViewport = GuiManager::mCurViewport;
    }
    else
    {
        if (iTop != 1 || !lua_isnumber(LuaHelper::mState, 1))
        {
            return 0;
        }
        uViewport = (int)luaL_checknumber(LuaHelper::mState, 1);
    }

    int iJoystick = GuiManager::GetJoystickFromViewport(uViewport);
    if (iJoystick < 0)
    {
        return 0;
    }

    RawControllerInputs* pJoystick = FLInputManager::Instance()->GetPlayerJoystick(iJoystick);
    const float* pfRaw = (const float*)&pJoystick->mRawInputStorage[0x10 - 4] + iJoystick * 76;
    lua_pushnumber(LuaHelper::mState, pfRaw[49]);
    lua_pushnumber(LuaHelper::mState, pfRaw[51]);
    return 2;
}

// FUNCTION: SWBF2 0x0057ed90
int ScriptCB_SetQuitPlayer(lua_State*)
{
    gQuitPlayer = (int)luaL_checknumber(LuaHelper::mState, 1);
    return 0;
}

// FUNCTION: SWBF2 0x0057edc0
int ScriptCB_GetQuitPlayer(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)gQuitPlayer);
    return 1;
}

// FUNCTION: SWBF2 0x0057edf0
int ScriptCB_GetMaxMissionQueue(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)MissionPlayList::GetMaxSize());
    return 1;
}

// FUNCTION: SWBF2 0x0057ee20
int ScriptCB_SetMissionNames(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2)
    {
        return 0;
    }

    if (lua_isstring(LuaHelper::mState, 1))
    {
        MissionPlayList::ClearEntries();
        MissionPlayList::AddEntry(luaL_checklstring(LuaHelper::mState, 1, 0),
                                  (unsigned char)PlayerPrefs::mAllUserPrefs[0].mTeamNum, 0, 0, 0, 0);
        MissionPlayList::SetLoopForever(false);
    }
    else if (lua_type(LuaHelper::mState, 1) == LUA_TTABLE)
    {
        lua_gettop(LuaHelper::mState);
        MissionPlayList::ClearEntries();
        int iCount = luaL_getn(LuaHelper::mState, 1);
        for (int i = 1; i <= iCount; ++i)
        {
            char acMap[64];
            char acTeam1[64];
            char acTeam2[64];
            int iSide;
            lua_rawgeti(LuaHelper::mState, 1, i);
            LuaHelper::TableReadVal("Map", acMap, sizeof(acMap), false);
            LuaHelper::TableReadVal("Side", iSide, false);
            LuaHelper::TableReadVal("Team1", acTeam1, sizeof(acTeam1), true);
            LuaHelper::TableReadVal("Team2", acTeam2, sizeof(acTeam2), true);
            lua_settop(LuaHelper::mState, -2);
            if (!MissionPlayList::AddEntry(acMap, (unsigned char)iSide, acTeam1, acTeam2, 0, 0))
            {
                break;
            }
        }
        MissionPlayList::SetLoopForever(true);
        lua_gettop(LuaHelper::mState);
    }

    if (lua_toboolean(LuaHelper::mState, 2) || gRandomizePlaylist)
    {
        MissionPlayList::RandomizeOrder();
    }
    MissionPlayList::SelectNextEntry();
    return 0;
}

// FUNCTION: SWBF2 0x0057efb0
int ScriptCB_SetTeamNames(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 2 && lua_isstring(LuaHelper::mState, 1) &&
        lua_isstring(LuaHelper::mState, 2))
    {
        const char* pcTeam1 = luaL_checklstring(LuaHelper::mState, 1, 0);
        const char* pcTeam2 = luaL_checklstring(LuaHelper::mState, 2, 0);
        MissionPlayList::SetCurrentTeamNames(pcTeam1 ? (unsigned int)PblHash(pcTeam1) : 0,
                                             pcTeam2 ? (unsigned int)PblHash(pcTeam2) : 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f060
int ScriptCB_EnterMission(lua_State*)
{
    if (GameState::GetState() == SHELL_STATE && MissionPlayList::GetCount() >= 1 &&
        lua_gettop(LuaHelper::mState) == 0)
    {
        GameState::SetState(MISSION_STATE);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f0a0
int ScriptCB_LaunchDemo(lua_State*)
{
    if (GameState::GetState() != SHELL_STATE)
    {
        return 0;
    }

    const char* pcMap;
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop == 0)
    {
        pcMap = "end1a";
    }
    else
    {
        if (iTop != 1 || !lua_isstring(LuaHelper::mState, 1))
        {
            return 0;
        }
        pcMap = luaL_checklstring(LuaHelper::mState, 1, 0);
        if (!pcMap)
        {
            return 0;
        }
    }

    GameLoop::SetNumCameras(1);
    if (sAttractMap[0])
    {
        printf_wrapper("Attract mode launching %s\n", sAttractMap);
        pcMap = sAttractMap;
    }
    else
    {
        printf_wrapper("Attract mode launching %s\n", pcMap);
    }
    GameLoop::SetNextMission(pcMap);
    gDemoMode = true;
    gAttractMode = true;
    GuiManager::Instance()->ChangeToScreen("", false, false);
    GameState::SetState(MISSION_STATE);
    return 0;
}

// FUNCTION: SWBF2 0x0057f190
int ScriptCB_Reboot(lua_State*)
{
    NetGame::Reboot();
    return 0;
}

// FUNCTION: SWBF2 0x0057f1a0
int ScriptCB_GetPausingViewport(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)PauseMenu::GetPausingViewport());
    return 1;
}

// FUNCTION: SWBF2 0x0057f1d0
int ScriptCB_GetPausingJoystick(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)PauseMenu::GetPausingJoystick());
    return 1;
}

// FUNCTION: SWBF2 0x0057f200
int ScriptCB_PausingIsPrimary(lua_State*)
{
    lua_pushboolean(LuaHelper::mState,
                    PauseMenu::GetPausingJoystick() == FLInputManager::Instance()->GetPrimaryPort());
    return 1;
}

// FUNCTION: SWBF2 0x0057f230
int ScriptCB_GetBuildStr(lua_State*)
{
    lua_pushstring(LuaHelper::mState, NetSetup::GetGameVersion());
    return 1;
}

// FUNCTION: SWBF2 0x0057f250
int ScriptCB_UpdateScreen(lua_State*)
{
    NetSetupP::UpdateScreen();
    return 0;
}

// FUNCTION: SWBF2 0x0057f260
int ScriptCB_UnlockableState(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    int iIndex = atoi(luaL_checklstring(LuaHelper::mState, 1, 0)) - 1;
    bool bUnlocked = false;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        bUnlocked = pProfile->GetUnlockableState(iIndex);
    }
    lua_pushboolean(LuaHelper::mState, bUnlocked);
    return 1;
}

// FUNCTION: SWBF2 0x0057f2c0
int ScriptCB_UnlockUnlockable(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    int iIndex = atoi(luaL_checklstring(LuaHelper::mState, 1, 0)) - 1;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        pProfile->SetUnlockableState(iIndex, true);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f300
int ScriptCB_SetVictoryMovie(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x0057f310
int ScriptCB_SetDefeatMovie(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x0057f320
int ScriptCB_ShouldShowLegal(lua_State*)
{
    bool bShow = ShellLoop::GetLegalShowtype() != ShellLoop::eLEGALSCREENS_SKIP;
    lua_pushboolean(LuaHelper::mState, bShow);
    if (bShow)
    {
        ShellLoop::SetLegalShowtype(ShellLoop::eLEGALSCREENS_SKIP);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0057f360
int ScriptCB_PauseDemoTimer(lua_State*)
{
    if (!GameState::GetKioskMode())
    {
        GameState::SetIgnoreNextUpdate(true);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f380
int ScriptCB_ShouldShowDemoPostscreen(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, false);
    return 1;
}

// FUNCTION: SWBF2 0x0057f3a0
int ScriptCB_ShouldCheckDNAS(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, s_doDNAS != 0);
    return 1;
}

// FUNCTION: SWBF2 0x0057f3c0
int ScriptCB_DoneCheckDNAS(lua_State*)
{
    s_doDNAS = 0;
    return 1;
}

// FUNCTION: SWBF2 0x0057f3d0
int ScriptCB_HasSeenDNASEULA(lua_State*)
{
    bool bSeen = false;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        bSeen = pProfile->mOnlineOptions.mHasSeenEULA;
    }
    lua_pushboolean(LuaHelper::mState, bSeen);
    return 1;
}

// FUNCTION: SWBF2 0x0057f400
int ScriptCB_SetHasSeenDNASEULA(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        pProfile->mOnlineOptions.mHasSeenEULA = true;
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f420
int ScriptCB_ShouldCheckPatch(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, s_doPatch != 0);
    return 1;
}

// FUNCTION: SWBF2 0x0057f440
int ScriptCB_DoneCheckPatch(lua_State*)
{
    s_doPatch = 0;
    return 1;
}

// FUNCTION: SWBF2 0x0057f450
int ScriptCB_IndexMultipageText(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    if (!lua_isstring(LuaHelper::mState, 1) || !lua_isstring(LuaHelper::mState, 2))
    {
        return 0;
    }

    const char* pcKey = luaL_checklstring(LuaHelper::mState, 1, 0);
    const char* pcElement = luaL_checklstring(LuaHelper::mState, 2, 0);
    unsigned short* pusText = (unsigned short*)RedLocalizeDB::Find(PblHash(pcKey), false);
    RedTextElement* pTextElement = (RedTextElement*)RedInterfaceElement::Find(pcElement);
    memset(sPageStart, 0, sizeof(sPageStart));

    int i;
    for (i = 0; i < 300; ++i)
    {
        sPageStart[i] = pusText;
        int iChars = pTextElement->SetText(pusText, true);
        if (!iChars)
        {
            break;
        }
        pusText += iChars + 1;
        if (!*pusText)
        {
            break;
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)(i + 1));
    return 1;
}

// FUNCTION: SWBF2 0x0057f550
int ScriptCB_ShowMultipageText(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    if (lua_isstring(LuaHelper::mState, 1) && lua_isnumber(LuaHelper::mState, 2))
    {
        const char* pcElement = luaL_checklstring(LuaHelper::mState, 1, 0);
        int iPage = (int)luaL_checknumber(LuaHelper::mState, 2);
        RedTextElement* pTextElement = (RedTextElement*)RedInterfaceElement::Find(pcElement);
        const unsigned short* pusText = sPageStart[iPage - 1];
        unsigned short ausEmpty[5];
        if (!pusText)
        {
            memset(ausEmpty, 0, sizeof(ausEmpty));
            pusText = ausEmpty;
        }
        pTextElement->SetText(pusText, true);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f5f0
int ScriptCB_SetDCMap(lua_State*)
{
    const char* pcMap = 0;
    if (lua_type(LuaHelper::mState, 1) != LUA_TNIL)
    {
        pcMap = luaL_checklstring(LuaHelper::mState, 1, 0);
    }
    DownloadableContent::SetCurrentMap(PblHash(pcMap));
    return 0;
}

// FUNCTION: SWBF2 0x0057f640
int ScriptCB_HideDownloadableContent(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, false);
    return 1;
}

// FUNCTION: SWBF2 0x0057f660
int ScriptCB_RestartMission(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    unsigned int uNumCameras = CameraManager::Instance()->m_iNumCam;
    for (unsigned int i = 0; i < uNumCameras; ++i)
    {
        PauseMenu::SetPaused(i, false);
    }
    if (GameState::GetState() != SHELL_STATE)
    {
        GameState::SetState(MISSION_STATE);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f6b0
int ScriptCB_Unpause(lua_State*)
{
    int iViewport = 0;
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        if (!lua_isnumber(LuaHelper::mState, 1))
        {
            return 0;
        }
        iViewport = LuaHelper::GetInt(1);
    }

    unsigned int uiState = GameState::GetState();
    if (uiState == SHELL_STATE)
    {
        return 0;
    }
    if (uiState == MISSION_STATE && GameLoop::sShowStats && GameLoop::mNumCameras > 1 &&
        playerJoystickNumbers[0] == gActiveViewport)
    {
        iViewport = 0;
    }
    PauseMenu::SetPaused(iViewport, false);
    return 0;
}

// FUNCTION: SWBF2 0x0057f740
int ScriptCB_SkipToPlayerList(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, PauseMenu::sSkipToPlayerList);
    return 1;
}

// FUNCTION: SWBF2 0x0057f760
int ScriptCB_ResetSkipToPlayerList(lua_State*)
{
    PauseMenu::sSkipToPlayerList = false;
    return 0;
}

// FUNCTION: SWBF2 0x0057f770
int ScriptCB_CheckPlayerListDone(lua_State*)
{
    int iJoystick = PauseMenu::GetPausingJoystick();
    bool bPressed = ((FLInputManager::s_instance.GetPlayerInputs(iJoystick)->bits.mBoolInputs >> 17) & 1) != 0;
    bool bDone = false;
    if (gSkipToPlayerListTime + 0.5f > GameLoop::GetMissionTime())
    {
        if (!bPressed)
        {
            gSkipToPlayerListState = 1;
        }
    }
    else if (gSkipToPlayerListState == 0)
    {
        bDone = !bPressed;
    }
    else if (gSkipToPlayerListState == 1)
    {
        if (bPressed)
        {
            gSkipToPlayerListState = 0;
        }
    }
    lua_pushboolean(LuaHelper::mState, bDone);
    return 1;
}

// FUNCTION: SWBF2 0x0057f810
int ScriptCB_GetMissionTime(lua_State*)
{
    float fTime;
    if (GameState::GetState() == SHELL_STATE)
    {
        fTime = ShellLoop::GetMissionTime();
    }
    else
    {
        fTime = GameLoop::GetMissionTime();
    }
    lua_pushnumber(LuaHelper::mState, fTime);
    return 1;
}

// FUNCTION: SWBF2 0x0057f850
int ScriptCB_Freecamera(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    if (GameState::GetState() != SHELL_STATE)
    {
        PauseMenu::SetFreeCamera(0, true);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f880
int ScriptCB_IsGameOver(lua_State*)
{
    bool bGameOver = false;
    if (GameState::GetState() == MISSION_STATE)
    {
        bGameOver = GameLoop::IsGameOver();
    }
    lua_pushboolean(LuaHelper::mState, bGameOver);
    return 1;
}

// FUNCTION: SWBF2 0x0057f8b0
int ScriptCB_QuitToShell(lua_State*)
{
    if (GameState::GetState() != SHELL_STATE)
    {
        NetSetup::RequestQuit(0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057f8d0
int ScriptCB_QuitToWindows(lua_State*)
{
    PostMessageA(hGlobalWnd, WM_CLOSE, 0, 0);
    return 0;
}

// FUNCTION: SWBF2 0x0057f8f0
int ScriptCB_MinimizeWindow(lua_State*)
{
    ShowWindow(hGlobalWnd, SW_MINIMIZE);
    return 0;
}

// FUNCTION: SWBF2 0x0057f910
int ScriptCB_QuitFromStats(lua_State*)
{
    PauseMenu::SetStatsDone();
    return 0;
}

// FUNCTION: SWBF2 0x0057f920
int ScriptCB_GetTeamName(lua_State*)
{
    int iTeam = 0;
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        if (!lua_isnumber(LuaHelper::mState, 1))
        {
            return 0;
        }
        iTeam = LuaHelper::GetInt(1);
    }

    Team* pTeam = Team::sTeams[iTeam];
    if (pTeam && pTeam->mName[0])
    {
        LuaHelper::PushString(pTeam->mName);
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x0057f9b0
int ScriptCB_GetSPProgress(lua_State*)
{
    int iMission = LuaHelper::GetInt(1);
    if (iMission < 1 || iMission > 16)
    {
        iMission = 1;
    }

    int iProgress = 0;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        iProgress = pProfile->mGameOptions.mCampaignProgress[iMission - 1];
    }
    lua_pushnumber(LuaHelper::mState, (float)iProgress);
    return 1;
}

// FUNCTION: SWBF2 0x0057fa20
int ScriptCB_SetSPProgress(lua_State*)
{
    int iMission = LuaHelper::GetInt(1);
    if (iMission < 1 || iMission > 16)
    {
        iMission = 1;
    }
    int iProgress = LuaHelper::GetInt(2);
    if (iProgress < 0)
    {
        iProgress = 0;
    }
    else if (iProgress > 255)
    {
        iProgress = 255;
    }
    unsigned char ucProgress = (unsigned char)iProgress;

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        unsigned char& ucSaved = pProfile->mGameOptions.mCampaignProgress[iMission - 1];
        ucSaved = ucProgress > ucSaved ? ucProgress : ucSaved;
        pProfile->UpdateProfCRC();
    }
    pProfile = PlayerProfile::GetProfile(1);
    if (pProfile)
    {
        unsigned char& ucSaved = pProfile->mGameOptions.mCampaignProgress[iMission - 1];
        ucSaved = ucProgress > ucSaved ? ucProgress : ucSaved;
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057faf0
int ScriptCB_GetLastBattleVictory(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)GameLoop::GetLastGameWinner());
    return 1;
}

// FUNCTION: SWBF2 0x0057fb20
int ScriptCB_SetLastBattleVictoryValid(lua_State*)
{
    if (!lua_toboolean(LuaHelper::mState, 1))
    {
        GameLoop::SetLastGameWinner(-1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057fb50
int ScriptCB_MetagameSetBonus(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x0057fb60
int ScriptCB_MetagameSetSide(lua_State*)
{
    const char* pcSide = LuaHelper::GetString(2);
    unsigned int uiIndex = (unsigned int)luaL_checknumber(LuaHelper::mState, 1);
    GameLoop::SetMetagameSide(uiIndex, Team::TranslateTeamName(pcSide));
    return 0;
}

// FUNCTION: SWBF2 0x0057fba0
int ScriptCB_DisableFlyerShadows(lua_State*)
{
    gDrawFlyerShadows = false;
    return 0;
}

}

namespace
{

RedParticleRenderer::TexCoords WholeTexture()
{
    RedParticleRenderer::TexCoords tc;
    tc.u0 = 0;
    tc.v0 = 0;
    tc.u1 = 255;
    tc.v1 = 255;
    return tc;
}
}

// FUNCTION: SWBF2 0x0057fbb0
void DrawAllBeamBetween::Render(int)
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        const Desc& desc = mArray[i];
        const PblVector3& vPos1 = desc.mPos1;
        const PblVector3& vPos2 = desc.mPos2;
        float fRadius = desc.mWidth * 2.0f;

        PblVector3 vMin, vMax;
        vMin.x = (vPos2.x > vPos1.x ? vPos1.x : vPos2.x) - fRadius;
        vMin.y = (vPos2.y > vPos1.y ? vPos1.y : vPos2.y) - fRadius;
        vMin.z = (vPos2.z > vPos1.z ? vPos1.z : vPos2.z) - fRadius;
        vMax.x = fRadius + (vPos1.x > vPos2.x ? vPos1.x : vPos2.x);
        vMax.y = fRadius + (vPos1.y > vPos2.y ? vPos1.y : vPos2.y);
        vMax.z = fRadius + (vPos1.z > vPos2.z ? vPos1.z : vPos2.z);
        RedParticleRenderer::SubmitTexture(desc.mTexture, BLEND_NORMAL, 0);
        RedParticleRenderer::SubmitBounds(vMin, vMax);

        if (desc.mRepetitions > 1.0f)
        {
            RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vPos1, desc.mColor, desc.mWidth, desc.mShift,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
            PblVector3 vMid;
            vMid.x = (vPos2.x + vPos1.x) * 0.5f;
            vMid.y = (vPos1.y + vPos2.y) * 0.5f;
            vMid.z = (vPos1.z + vPos2.z) * 0.5f;
            RedParticleRenderer::SubmitParticle(PIT_STREAK, vMid, desc.mColor, desc.mWidth,
                                                desc.mRepetitions * 0.5f + desc.mShift,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
            RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vPos2, desc.mColor, desc.mWidth,
                                                desc.mRepetitions + desc.mShift,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
        }
        else
        {
            PblVector3 vDelta, vDir;
            vDelta.x = vPos2.x - vPos1.x;
            vDelta.y = vPos2.y - vPos1.y;
            vDelta.z = vPos2.z - vPos1.z;
            D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vDelta);

            float fWidth = desc.mWidth;
            PblVector3 vStart;
            vStart.x = vPos1.x - vDir.x * fWidth;
            vStart.y = vPos1.y - vDir.y * fWidth;
            vStart.z = vPos1.z - vDir.z * fWidth;
            RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vStart, desc.mColor, fWidth, 0.0f,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
            RedParticleRenderer::SubmitParticle(PIT_STREAK, vPos1, desc.mColor, desc.mWidth, 0.25f,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
            RedParticleRenderer::SubmitParticle(PIT_STREAK, vPos2, desc.mColor, desc.mWidth, 0.75f,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
            fWidth = desc.mWidth;
            PblVector3 vEnd;
            vEnd.x = fWidth * vDir.x + vPos2.x;
            vEnd.y = vDir.y * fWidth + vPos2.y;
            vEnd.z = vDir.z * fWidth + vPos2.z;
            RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vEnd, desc.mColor, fWidth, 1.0f,
                                                WholeTexture(), gVectorUnitX, gVectorUnitY);
        }
    }
    mCount = 0;
}

// FUNCTION: SWBF2 0x0057ff60
bool DrawAllBeamBetween::Add(const PblVector3& vPos1, const PblVector3& vPos2, const char* pcTexture,
                             float fWidth, RedColor color, float fShift, float fRepetitions)
{
    if (mCount >= 64)
    {
        return false;
    }

    Desc& desc = mArray[mCount++];
    desc.mPos1 = vPos1;
    desc.mPos2 = vPos2;
    desc.mTexture = PblHash(pcTexture);
    desc.mWidth = fWidth;
    desc.mColor = color;
    desc.mShift = fShift;
    desc.mRepetitions = fRepetitions;
    return true;
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00580020
int DrawBeamBetween(lua_State*)
{
    PblMatrix mStart = *(PblMatrix*)lua_touserdata(LuaHelper::mState, 1);
    PblMatrix mEnd = *(PblMatrix*)lua_touserdata(LuaHelper::mState, 2);
    const char* pcTexture = LuaHelper::GetString(3);
    float fWidth = LuaHelper::GetFloat(4);
    RedColor color;
    color.a = (unsigned char)(int)LuaHelper::GetFloat(8);
    color.b = (unsigned char)(int)LuaHelper::GetFloat(7);
    color.g = (unsigned char)(int)LuaHelper::GetFloat(6);
    color.r = (unsigned char)(int)LuaHelper::GetFloat(5);
    float fShift = LuaHelper::GetFloat(9, 0.0f);
    float fRepetitions = LuaHelper::GetFloat(10, 1.0f);
    sDrawAllBeamBetween->Add(mStart.GetTranslation(), mEnd.GetTranslation(), pcTexture, fWidth, color, fShift,
                             fRepetitions);
    return 0;
}

}

// FUNCTION: SWBF2 0x005801b0
void DrawAllParticleAt::Render(int)
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        const Desc& desc = mArray[i];
        float fRadius = desc.mWidth * 2.0f;

        PblVector3 vMin, vMax;
        vMin.x = desc.mPos.x - fRadius;
        vMin.y = desc.mPos.y - fRadius;
        vMin.z = desc.mPos.z - fRadius;
        vMax.x = desc.mPos.x + fRadius;
        vMax.y = desc.mPos.y + fRadius;
        vMax.z = desc.mPos.z + fRadius;
        RedParticleRenderer::SubmitTexture(desc.mTexture, BLEND_NORMAL, 0);
        RedParticleRenderer::SubmitBounds(vMin, vMax);
        RedParticleRenderer::SubmitParticle(PIT_PARTICLE, desc.mPos, desc.mColor, desc.mWidth, desc.mRotation,
                                            WholeTexture(), gVectorUnitX, gVectorUnitY);
    }
    mCount = 0;
}

DrawAllBeamBetween::DrawAllBeamBetween() : Renderable(6), mCount(0)
{
    Activate(-1);
}

DrawAllParticleAt::DrawAllParticleAt() : Renderable(6), mCount(0)
{
    Activate(0);
}

// FUNCTION: SWBF2 0x005802c0
bool DrawAllParticleAt::Add(const PblVector3& vPos, const char* pcTexture, float fWidth, RedColor color,
                            float fRotation)
{
    if (mCount >= 128)
    {
        return false;
    }

    Desc& desc = mArray[mCount++];
    desc.mPos = vPos;
    desc.mTexture = PblHash(pcTexture);
    desc.mWidth = fWidth;
    desc.mColor = color;
    desc.mRotation = fRotation;
    return true;
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00580740
void Init()
{
    for (int i = 0; i < 91; ++i)
    {
        LuaHelper::Register(sCallbacks[i].pcName, sCallbacks[i].pfnCallback, 0);
    }

    InitLoadSave();
    InitOptions();
    InitMission();
    InitIFace();
    InitSound();
    InitNet();
    InitStats();
    InitPC();
}

}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005807a0
void Cleanup()
{
    CleanupSound();
    CleanupOptions();
}
}
