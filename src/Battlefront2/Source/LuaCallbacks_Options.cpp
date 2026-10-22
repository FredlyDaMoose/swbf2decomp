#include "LuaCallbacks_Options.h"
#include "LuaCallbacks_Options.h"
#include "LuaHelper.h"
#include <string.h>
#include "LuaFL/include/lauxlib.h"
#include "FLInputManager.h"
#include "GameLoop.h"
#include "ShellLoop.h"
#include "GameState.h"
#include "GameVoiceChat.h"
#include "HUDElement.h"
#include "NetGame.h"
#include "NetComm.h"
#include "NetSetup.h"
#include "VoiceChatTransport.h"
#include "PauseMenu.h"
#include "PlayerProfile.h"
#include "ToolTips.h"
#include "PebbleFL/Common/PblHashTable.h"

static unsigned __int64 ShaRotate(unsigned __int64 value, unsigned int bits)
{
    return (value >> bits) | (value << (64 - bits));
}

namespace Lua_Callbacks
{

// GLOBAL: SWBF2 0x007e8260
int gNumBots[4] = { -1, -1, 16, 16 };

}

namespace GameLoop { extern bool mbHistoricalRules; }

extern bool netTeamDamage;
extern bool netAutoAssignTeams;
extern int netNumBots;
extern bool netHeroesEnabled;
extern int netHeroRuleUnlockVal;
extern int netHeroRuleRespawn;
extern int netHeroRuleRespawnVal;
namespace PblBuildType { extern bool gDemoBuild; }
extern int hostVoteThreshold;
extern int netHostedPlayerLimit;
extern int netDedicatedPlayerLimit;
extern int netPlayerLimit;
extern int gGlobalNumBots;
extern bool netAutoAim;
extern bool netShowNames;
namespace PblBuildType { extern bool gDemoAutoAssign; }
extern bool netAutoAssignTeamsOriginalSetting;
extern int netMinPlayersToAutoLaunchDedicated;
extern int netThrottle;
extern int netHeroRuleUnlock;
extern int netHeroRulePlayer;
extern int netHeroRuleTeam;
namespace Lua_Callbacks { extern int gTeamSetReinforcementMult; }
extern int netRequestedCameraCount;
extern bool g_SpectatorEnabled[1];

bool GetNetAwardsEnabled();
void SetNetAwardsEnabled(int);

namespace Lua_Callbacks
{

// GLOBAL: SWBF2 0x007e7e98
int gUberScoreLimit = 400;
// GLOBAL: SWBF2 0x007e7e9c
eGAME_RULES gCurRules = eGAMERULES_MAX;
// GLOBAL: SWBF2 0x007e7fd0
int gDMReinforcementMult[5] = { 100, 100, 100, 100, 100 };
// GLOBAL: SWBF2 0x007e7ff0
int gCONReinforcementMult[5] = { 100, 100, 100, 100, 100 };
// GLOBAL: SWBF2 0x007e7f50
int gCONNumBots[4] = { -1, -1, -1, -1 };
// GLOBAL: SWBF2 0x007e7f20
int gCTFNumBots[4] = { -1, -1, -1, -1 };
// GLOBAL: SWBF2 0x007e7fb0
int gELINumBots[4] = { -1, -1, -1, -1 };
// GLOBAL: SWBF2 0x007e7f40
int gASSNumBots[4] = { -1, -1, -1, -1 };
// GLOBAL: SWBF2 0x007e7f80
int gSpaceAssaultScoreLimit[5] = { 180, 180, 180, 180, 180 };
// GLOBAL: SWBF2 0x007e8030
int gCTFScore[5] = { 5, 5, 5, 5, 5 };
// GLOBAL: SWBF2 0x007e8010
int gHuntScoreLimit[5] = { 50, 50, 50, 50, 50 };
// GLOBAL: SWBF2 0x007e8240
int gHuntMaxTimeLimit[5] = { 5, 5, 5, 5, 5 };
// GLOBAL: SWBF2 0x01e57970
int gCTFMaxTimeLimit[5];
// GLOBAL: SWBF2 0x01e57950
int gCONMaxTimeLimit[5];
// GLOBAL: SWBF2 0x01e57937
bool gRandomizePlaylist;
// GLOBAL: SWBF2 0x01e57948
bool gIsDedicated;

// GLOBAL: SWBF2 0x0078a294
static const int netPlayersSupported[9] = { 3, 6, 9, 12, 19, 25, 38, 76, 153 };

}

// GLOBAL: SWBF2 0x0078a2d0
const char* const gGameRulesNames[Lua_Callbacks::eGAMERULES_MAX] =
{
    "instantaction", "mp", "campaign", "metagame"
};

// GLOBAL: SWBF2 0x01f9cdd0
unsigned int gGameRulesNameHashes[Lua_Callbacks::eGAMERULES_MAX];

// GLOBAL: SWBF2 0x0078a2f0
static const int sLuaFunctionToEnum[5][21] = {
    { -1, 2, 4, 3, 0, 1, 13, 15, 8, 5, 7, 9, 10, 11, 12, -1, 6, -1, -1, -1, -1 },
    { -1, 2, -1, 3, 0, 1, 13, -1, 8, 5, 7, 9, 10, 11, 12, -1, 6, -1, -1, -1, -1 },
    { -1, 2, 4, 3, 0, 1, 13, -1, 8, 5, 7, 9, 10, 11, 12, -1, 6, -1, -1, -1, -1 },
    { -1, 2, 4, 3, 0, 1, 13, 15, 8, 5, 7, 9, 10, 11, 12, -1, 6, -1, -1, -1, -1 },
    { -1, -1, -1, 3, 0, 1, 13, -1, 8, 5, 7, 9, 10, 11, 12, -1, 6, -1, -1, -1, -1 }
};

// FUNCTION: SWBF2 0x0052b500
int RawControllerInputs::GetFunctionForAnalog(int, int) { return 0; }

namespace GameLoop {

// FUNCTION: SWBF2 0x00533630
void SetFunctionIdForButtonId(int iButton, int iFunction)
{
    RawControllerInputs* pInputs = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    if (pInputs)
    {
        pInputs->mCurBindings[pInputs->GetInputMode()][iButton].mDest = iFunction;
    }
}
}

namespace GameLoop {

// FUNCTION: SWBF2 0x00533660
int GetFunctionIdForButtonId(int iButton)
{
    RawControllerInputs* pInputs = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    if (!pInputs)
    {
        return -1;
    }
    return pInputs->mCurBindings[pInputs->GetInputMode()][iButton].mDest;
}
}

namespace GameLoop {

// FUNCTION: SWBF2 0x00533690
int GetFunctionIdForAnalog()
{
    if (RawControllerInputs* pInputs = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport()))
    {
        return pInputs->GetFunctionForAnalog(0, 0);
    }
    return 0;
}
}

// FUNCTION: SWBF2 0x00597250
int LuaButtonIndexToEnum(int iButton) { return iButton; }

// FUNCTION: SWBF2 0x00597260
int LuaFunctionIndexToEnum(unsigned int iFunction, unsigned int iMode)
{
    if (iFunction > 20 || iMode > 4)
    {
        return -1;
    }
    return sLuaFunctionToEnum[iMode][iFunction];
}

// FUNCTION: SWBF2 0x00597280
int EnumToLuaFunctionIndex(int iEnum, int iMode)
{
    for (int i = 0; i < 21; ++i)
    {
        if (sLuaFunctionToEnum[iMode][i] == iEnum)
        {
            return i;
        }
    }
    return 0;
}

namespace Lua_Callbacks
{

void _SetGameRules(eGAME_RULES eRules);

// FUNCTION: SWBF2 0x00596b70
int ScriptCB_SetSplitscreen(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        int iNumCameras = (int)luaL_checknumber(LuaHelper::mState, 1);
        if (iNumCameras < 1)
        {
            iNumCameras = 1;
        }
        if (iNumCameras > 1)
        {
            iNumCameras = 1;
        }
        GameLoop::SetNumCameras(iNumCameras);
    }
    else
    {
        GameLoop::SetNumCameras(1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596be0
int ScriptCB_GetNumCameras(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)(int)GameLoop::GetNumCameras());
    return 1;
}

// FUNCTION: SWBF2 0x00596c10
int ScriptCB_IsSplitscreen(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, GameLoop::GetNumCameras() > 1);
    return 1;
}

// FUNCTION: SWBF2 0x00596d10
int ScriptCB_SetDifficulty(lua_State* L)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if ((iArgs == 1 || iArgs == 2) && lua_isnumber(LuaHelper::mState, 1))
    {
        int iDifficulty = (int)luaL_checknumber(LuaHelper::mState, 1);
        if (iDifficulty < eDIFFICULTY_EASY || iDifficulty > eDIFFICULTY_HARD)
        {
            iDifficulty = eDIFFICULTY_MEDIUM;
        }

        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            pProfile->mGameOptions.mCurDiffLevel = (eGAME_DIFFICULTY)iDifficulty;
            if (iArgs == 2 && lua_type(LuaHelper::mState, 2) == LUA_TBOOLEAN &&
                lua_toboolean(LuaHelper::mState, 2))
            {
                bool bAssist = iDifficulty <= eDIFFICULTY_MEDIUM;
                pProfile->mGameOptions.mbAimAssist = bAssist;
                pProfile->mGameOptions.mbStickyReticule = bAssist;
                pProfile->mGameOptions.mbTargetAutoTracking = bAssist;
            }
            pProfile->UpdateProfCRC();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596de0
int ScriptCB_GetDifficulty(lua_State* L)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    int iDifficulty = pProfile ? pProfile->mGameOptions.mCurDiffLevel : PlayerPrefs::mAllUserPrefs[0].mDifficulty;
    if (iDifficulty < eDIFFICULTY_EASY || iDifficulty > eDIFFICULTY_HARD)
    {
        iDifficulty = eDIFFICULTY_MEDIUM;
    }
    lua_pushnumber(LuaHelper::mState, (float)iDifficulty);
    return 1;
}

// FUNCTION: SWBF2 0x00596ea0
int ScriptCB_GetHuntScoreLimit(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)gHuntScoreLimit[gCurRules]);
    return 1;
}

extern int gHuntShowScoreLimit;

// FUNCTION: SWBF2 0x00596ed0
int ScriptCB_ShowHuntScoreLimit(lua_State* L)
{
    gHuntShowScoreLimit = (int)luaL_checknumber(LuaHelper::mState, 1);
    return 0;
}

// FUNCTION: SWBF2 0x00596f70
int ScriptCB_GetUberScoreLimit(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)gUberScoreLimit);
    return 1;
}

// FUNCTION: SWBF2 0x00596fa0
int ScriptCB_SetUberScoreLimit(lua_State* L)
{
    gUberScoreLimit = (int)luaL_checknumber(LuaHelper::mState, 1);
    return 0;
}

// FUNCTION: SWBF2 0x00596fd0
int ScriptCB_GetCONMaxTimeLimit(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)(gCONMaxTimeLimit[gCurRules] * 60));
    return 1;
}

// FUNCTION: SWBF2 0x00597010
int ScriptCB_GetCONNumBots(lua_State* L)
{
    int iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gCONNumBots[gCurRules] : netNumBots;
    lua_pushnumber(LuaHelper::mState, (float)iBots);
    return 1;
}

// FUNCTION: SWBF2 0x00597780
int ScriptCB_GetGameRules(lua_State* L)
{
    if ((unsigned int)gCurRules > eGAMERULES_METAGAME)
    {
        lua_pushstring(LuaHelper::mState, "");
    }
    else
    {
        lua_pushstring(LuaHelper::mState, gGameRulesNames[gCurRules]);
    }
    return 1;
}

// FUNCTION: SWBF2 0x005976d0
int ScriptCB_SetGameRules(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        if (!gGameRulesNameHashes[0])
        {
            for (int i = 0; i < eGAMERULES_MAX; ++i)
            {
                gGameRulesNameHashes[i] = PblHash(gGameRulesNames[i]);
            }
        }

        unsigned int uHash = PblHash(luaL_checkstring(LuaHelper::mState, 1));
        int i;
        for (i = 0; i < eGAMERULES_MAX; ++i)
        {
            if (uHash == gGameRulesNameHashes[i])
            {
                break;
            }
        }
        if (i != eGAMERULES_MAX)
        {
            _SetGameRules((eGAME_RULES)i);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597860
int ScriptCB_GetYAxisFlip(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)GameLoop::GetJoystickYFlip());
    return 1;
}

// FUNCTION: SWBF2 0x00597890
int ScriptCB_SetYAxisFlip(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        GameLoop::SetJoystickYFlip((int)luaL_checknumber(LuaHelper::mState, 1));
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            pProfile->NoteBindingChange();
            pProfile->UpdateProfCRC();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597910
int ScriptCB_GetMouseTurnAssist(lua_State* L)
{
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    lua_pushnumber(LuaHelper::mState, (float)pJoystick->GetMouseAssist());
    return 1;
}

// FUNCTION: SWBF2 0x00597a70
int ScriptCB_SetControlScale(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 2 && lua_isnumber(LuaHelper::mState, 1) &&
        lua_isnumber(LuaHelper::mState, 2))
    {
        int iAxis = (int)luaL_checknumber(LuaHelper::mState, 1);
        int iScale = (int)luaL_checknumber(LuaHelper::mState, 2);
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            int iMode = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport())->GetInputMode();
            pProfile->mGameOptions.mControllerScale[iAxis][iMode] = (unsigned char)iScale;
            pProfile->ApplySettings();
            pProfile->UpdateProfCRC();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597b40
int ScriptCB_ResetGameOptionsToDefault(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->mGameOptions.ResetToDefault();
        pProfile->mToolTipOptions.ResetToDefault();
        pProfile->ApplySettings();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597b90
int ScriptCB_ResetSoundToDefault(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->mSoundOptions.ResetToDefault();
        pProfile->ApplySettings();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597bd0
int ScriptCB_ResetOnlineOptionsToDefault(lua_State* L)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->mOnlineOptions.ResetToDefault();
        pProfile->ApplySettings();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597c00
int ScriptCB_ResetControlsToDefault(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->mGameOptions.ResetBindings(FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport())->GetInputMode());
        pProfile->ApplySettings();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597c50
int ScriptCB_GetGeneralOptions(lua_State* L)
{
    int iNumArgs = lua_gettop(LuaHelper::mState);
    lua_newtable(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (iNumArgs == 0 && pProfile)
    {
        PlayerProfile::tGameOpt& rGame = pProfile->mGameOptions;
        PlayerProfile::tToolTipOpt& rTips = pProfile->mToolTipOptions;
        LuaHelper::TableWriteVal("bFirstPerson", rGame.mCockpitCam);
        LuaHelper::TableWriteVal("bFirstPersonFlyer", rGame.mCockpitCamFlyer);
        LuaHelper::TableWriteVal("bRumble", rGame.mbRumble);
        LuaHelper::TableWriteVal("bAutoAim", rGame.mbTargetAutoTracking);
        LuaHelper::TableWriteVal("iFriendlyFire", (int)rGame.mFriendlyFire);
        LuaHelper::TableWriteVal("bHeroes", rGame.mbHerosEnabled);
        LuaHelper::TableWriteVal("iDifficulty", (int)rGame.mCurDiffLevel);
        LuaHelper::TableWriteVal("bToolTips", rTips.mOn);
        LuaHelper::TableWriteVal("bToolTipAuto", !rTips.mOn && !rTips.mOff);
        LuaHelper::TableWriteVal("bAimAssist", rGame.mbAimAssist);
        LuaHelper::TableWriteVal("bStickyReticule", rGame.mbStickyReticule);
        LuaHelper::TableWriteVal("bFlyerRolling", rGame.mbFlyerRolling);
        LuaHelper::TableWriteVal("bObjectiveDetails", rGame.mbObjectiveDetails);
        LuaHelper::TableWriteVal("bMovieSubtitles", rGame.mu8MovieSubtitles != 0);
        LuaHelper::TableWriteVal("bSoldierHints", rGame.mbSoldierHints);
        LuaHelper::TableWriteVal("iReticuleAlpha", rGame.mReticuleAlpha);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00597da0
int ScriptCB_SetGeneralOptions(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_istable(LuaHelper::mState, 1))
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tGameOpt& rGame = pProfile->mGameOptions;
            bool bVal;
            LuaHelper::TableReadVal("bFirstPerson", bVal);
            rGame.mCockpitCam = bVal;
            LuaHelper::TableReadVal("bFirstPersonFlyer", bVal);
            rGame.mCockpitCamFlyer = bVal;
            LuaHelper::TableReadVal("bRumble", bVal);
            rGame.mbRumble = bVal;
            LuaHelper::TableReadVal("bAutoAim", bVal);
            rGame.mbTargetAutoTracking = bVal;

            int iFriendlyFire = 100;
            LuaHelper::TableReadVal("iFriendlyFire", iFriendlyFire, false);
            if (iFriendlyFire != 0 && iFriendlyFire != 100)
            {
                iFriendlyFire = 0;
            }
            rGame.mFriendlyFire = (unsigned char)iFriendlyFire;

            LuaHelper::TableReadVal("bHeroes", bVal);
            rGame.mbHerosEnabled = bVal;
            LuaHelper::TableReadVal("bAimAssist", bVal);
            rGame.mbAimAssist = bVal;
            LuaHelper::TableReadVal("bStickyReticule", bVal);
            rGame.mbStickyReticule = bVal;
            LuaHelper::TableReadVal("bFlyerRolling", bVal);
            rGame.mbFlyerRolling = bVal;
            LuaHelper::TableReadVal("bObjectiveDetails", bVal);
            rGame.mbObjectiveDetails = bVal;
            LuaHelper::TableReadVal("bMovieSubtitles", bVal);
            rGame.mu8MovieSubtitles = bVal != 0;
            LuaHelper::TableReadVal("bSoldierHints", bVal);
            rGame.mbSoldierHints = bVal != 0;

            float fReticuleAlpha = 1.0f;
            LuaHelper::TableReadVal("iReticuleAlpha", fReticuleAlpha, false);
            rGame.mReticuleAlpha = fReticuleAlpha;

            int iDifficulty = eDIFFICULTY_MEDIUM;
            LuaHelper::TableReadVal("iDifficulty", iDifficulty, false);
            if (iDifficulty < eDIFFICULTY_EASY || iDifficulty >= eDIFFICULTY_LAST)
            {
                iDifficulty = eDIFFICULTY_MEDIUM;
            }
            if (rGame.mCurDiffLevel != iDifficulty || PlayerPrefs::mAllUserPrefs[0].mDifficulty != iDifficulty)
            {
                PlayerPrefs::mAllUserPrefs[0].mDifficulty = (eGAME_DIFFICULTY)iDifficulty;
                rGame.mCurDiffLevel = (eGAME_DIFFICULTY)iDifficulty;
            }

            pProfile->ApplySettings();
            pProfile->UpdateProfCRC();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597f80
int ScriptCB_IsHorizontalSplitScreen(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, false);
    return 1;
}

// FUNCTION: SWBF2 0x00597fa0
int ScriptCB_SetHorizontalSplitScreen(lua_State* L)
{
    if (GameState::GetState() != 0x8ff60339 && lua_gettop(LuaHelper::mState) == 1)
    {
        lua_toboolean(LuaHelper::mState, 1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597fe0
int ScriptCB_SetHUDScale(lua_State* L)
{
    HUD::Element::SetRuntimeScaleAll((float)luaL_checknumber(LuaHelper::mState, 1));
    return 0;
}

// FUNCTION: SWBF2 0x00598010
int ScriptCB_GetHUDScale(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, HUD::Element::GetRuntimeScaleAll());
    return 1;
}

// FUNCTION: SWBF2 0x00598030
int ScriptCB_NextToolTipState(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tToolTipOpt& rTips = pProfile->mToolTipOptions;
            if (rTips.mOn)
            {
                rTips.mOn = false;
                rTips.mOff = false;
                if (ToolTipsDisplay::sInstance)
                {
                    ToolTipsDisplay::sInstance->SetAuto();
                }
            }
            else if (rTips.mOff)
            {
                rTips.mOn = true;
                rTips.mOff = false;
                if (ToolTipsDisplay::sInstance)
                {
                    ToolTipsDisplay::sInstance->SetAlwaysOn();
                }
            }
            else
            {
                rTips.mOn = false;
                rTips.mOff = true;
                if (ToolTipsDisplay::sInstance)
                {
                    ToolTipsDisplay::sInstance->SetAlwaysOff();
                }
            }
            pProfile->UpdateProfCRC();
        }
    }
    return 1;
}

// FUNCTION: SWBF2 0x005980f0
int ScriptCB_PrevToolTipState(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            PlayerProfile::tToolTipOpt& rTips = pProfile->mToolTipOptions;
            if (rTips.mOn)
            {
                rTips.mOn = false;
                rTips.mOff = true;
                if (ToolTipsDisplay::sInstance)
                {
                    ToolTipsDisplay::sInstance->SetAlwaysOff();
                }
            }
            else if (rTips.mOff)
            {
                rTips.mOn = false;
                rTips.mOff = false;
                if (ToolTipsDisplay::sInstance)
                {
                    ToolTipsDisplay::sInstance->SetAuto();
                }
            }
            else
            {
                rTips.mOn = true;
                rTips.mOff = false;
                if (ToolTipsDisplay::sInstance)
                {
                    ToolTipsDisplay::sInstance->SetAlwaysOn();
                }
            }
            pProfile->UpdateProfCRC();
        }
    }
    return 1;
}

// FUNCTION: SWBF2 0x005981b0
int ScriptCB_SetToolTipState(lua_State* L)
{
    PlayerProfile::tProfile* pProfile = 0;
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    }

    int iState = 0;
    if (pProfile)
    {
        PlayerProfile::tToolTipOpt& rTips = pProfile->mToolTipOptions;
        int iRequested = (int)luaL_checknumber(LuaHelper::mState, 1);
        if (iRequested == 1)
        {
            rTips.mOn = false;
            rTips.mOff = true;
            if (ToolTipsDisplay::sInstance)
            {
                ToolTipsDisplay::sInstance->SetAlwaysOff();
            }
            iState = 1;
        }
        else if (iRequested == 2)
        {
            rTips.mOn = false;
            rTips.mOff = false;
            if (ToolTipsDisplay::sInstance)
            {
                ToolTipsDisplay::sInstance->SetAuto();
            }
            iState = 2;
        }
        else
        {
            rTips.mOn = true;
            rTips.mOff = false;
            if (ToolTipsDisplay::sInstance)
            {
                ToolTipsDisplay::sInstance->SetAlwaysOn();
            }
            iState = 3;
        }
        pProfile->UpdateProfCRC();
    }
    lua_pushnumber(LuaHelper::mState, (float)iState);
    return 1;
}

// FUNCTION: SWBF2 0x00598290
int ScriptCB_ResetToolTips(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {
            memset(pProfile->mToolTipOptions.mToolTipCount, 0,
                   sizeof(pProfile->mToolTipOptions.mToolTipCount));
            pProfile->UpdateProfCRC();
            pProfile->ApplySettings();
        }
    }
    return 1;
}

// FUNCTION: SWBF2 0x005982f0
int ScriptCB_SetControlMode(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        int iMode = (int)luaL_checknumber(LuaHelper::mState, 1);
        FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport())->SetInputMode(iMode);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00598350
int ScriptCB_GetControlMode(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState,
                   (float)(int)FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport())->GetInputMode());
    return 1;
}

// FUNCTION: SWBF2 0x00598390
int ScriptCB_GetSideSelectButtonSetting(lua_State* L)
{
    int iFirst = 2;
    int iSecond = 2;
    int iFourth = 2;
    if (NetGame::IsEnabled() && netAutoAssignTeams)
    {
        iFirst = 1;
        iSecond = 1;
    }
    if (!NetGame::IsEnabled())
    {
        iFourth = 0;
    }

    lua_pushnumber(LuaHelper::mState, (float)iFirst);
    lua_pushnumber(LuaHelper::mState, (float)iSecond);
    lua_pushnumber(LuaHelper::mState, 2.0f);
    lua_pushnumber(LuaHelper::mState, (float)iFourth);
    return 4;
}

// FUNCTION: SWBF2 0x00598450
int ScriptCB_SetOnlineOpts(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_istable(LuaHelper::mState, 1))
    {
        bool bAppearOffline = false;
        bool bVoiceMask = false;
        int iTVVoiceVol = 10;
        int iVoiceRecordVol = 10;
        int iVoicePlayVol = 10;
        int iPlayersSupported = 3;
        int iTurnsPerSecond = 20;
        int iOnlineIcon = 0;
        bool bAllRegions = false;
        LuaHelper::TableReadVal("bAppearOffline", bAppearOffline);
        LuaHelper::TableReadVal("bVoiceMask", bVoiceMask);
        LuaHelper::TableReadVal("iTVVoiceVol", iTVVoiceVol, false);
        LuaHelper::TableReadVal("iVoiceRecordVol", iVoiceRecordVol, false);
        LuaHelper::TableReadVal("iVoicePlayVol", iVoicePlayVol, false);
        LuaHelper::TableReadVal("iPlayersSupported", iPlayersSupported, false);
        LuaHelper::TableReadVal("iTurnsPerSecond", iTurnsPerSecond, false);
        LuaHelper::TableReadVal("iOnlineIcon", iOnlineIcon, false);
        LuaHelper::TableReadVal("bAllRegions", bAllRegions);

        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        if (pProfile)
        {
            PlayerProfile::tOnlineOpt& rOnline = pProfile->mOnlineOptions;
            rOnline.mu8AppearOffline = bAppearOffline;
            rOnline.mu8VoiceMask = bVoiceMask;
            rOnline.mu8VoiceToTVVol = (unsigned char)iTVVoiceVol;
            rOnline.mu8VoiceRecordVol = (unsigned char)iVoiceRecordVol;
            rOnline.mu8VoicePlayVol = (unsigned char)iVoicePlayVol;
            rOnline.mu8PlayersSupported = (unsigned char)iPlayersSupported;
            rOnline.mu8TurnsPerSecond = (unsigned char)iTurnsPerSecond;
            rOnline.mOnlineIcon = (unsigned char)iOnlineIcon;
            rOnline.mShowAllRegions = bAllRegions;
            pProfile->ApplySettings();
            pProfile->UpdateProfCRC();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005985c0
int ScriptCB_GetOnlineOpts(lua_State* L)
{
    bool bAppearOffline = false;
    bool bVoiceMask = false;
    int iTVVoiceVol = 10;
    int iVoiceRecordVol = 10;
    int iVoicePlayVol = 10;
    int iPlayersSupported = 0;
    int iTurnsPerSecond = 0;
    int iOnlineIcon = 0;
    bool bAllRegions = false;

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        PlayerProfile::tOnlineOpt& rOnline = pProfile->mOnlineOptions;
        bAppearOffline = rOnline.mu8AppearOffline != 0;
        bVoiceMask = rOnline.mu8VoiceMask != 0;
        iTVVoiceVol = rOnline.mu8VoiceToTVVol;
        iVoiceRecordVol = rOnline.mu8VoiceRecordVol;
        iVoicePlayVol = rOnline.mu8VoicePlayVol;
        iPlayersSupported = rOnline.mu8PlayersSupported;
        if (iPlayersSupported >= 9)
        {
            iPlayersSupported = 8;
        }
        iTurnsPerSecond = rOnline.mu8TurnsPerSecond;
        iOnlineIcon = rOnline.mOnlineIcon;
        bAllRegions = rOnline.mShowAllRegions != 0;
    }
    if (iTurnsPerSecond == 0)
    {
        iTurnsPerSecond = 20;
    }

    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("bAppearOffline", bAppearOffline);
    LuaHelper::TableWriteVal("bVoiceMask", bVoiceMask);
    LuaHelper::TableWriteVal("iTVVoiceVol", iTVVoiceVol);
    LuaHelper::TableWriteVal("iVoiceRecordVol", iVoiceRecordVol);
    LuaHelper::TableWriteVal("iVoicePlayVol", iVoicePlayVol);
    LuaHelper::TableWriteVal("iPlayersSupported", iPlayersSupported);
    LuaHelper::TableWriteVal("iTurnsPerSecond", iTurnsPerSecond);
    LuaHelper::TableWriteVal("iOnlineIcon", iOnlineIcon);
    LuaHelper::TableWriteVal("bAllRegions", bAllRegions);
    return 1;
}

// FUNCTION: SWBF2 0x005986f0
int ScriptCB_GetNetGameDefaults(lua_State* L)
{
    int iWarmUp = NetGame::GetMaxPreGameTime();
    int iVote = hostVoteThreshold;
    int iMaxPlayers = netHostedPlayerLimit;
    int iMaxDedicatedPlayers = netDedicatedPlayerLimit;
    int iNumPlayers = netPlayerLimit;
    int iNumBots = gGlobalNumBots;
    int iTeamDmg = netTeamDamage ? 100 : 0;
    int iAutoAim = netAutoAim ? 100 : 0;
    bool bShowNames = netShowNames;
    bool bIsPrivate = NetSetup::mPrivateGame;

    bool bAutoAssignTeams;
    if (PblBuildType::gDemoBuild && PblBuildType::gDemoAutoAssign)
    {
        bAutoAssignTeams = true;
    }
    else
    {
        bAutoAssignTeams = netAutoAssignTeams;
        netAutoAssignTeamsOriginalSetting = netAutoAssignTeams;
    }

    int iStartCnt = netMinPlayersToAutoLaunchDedicated;
    if (*NetSetup::GetAutoNetScript())
    {
        iStartCnt = NetSetup::GetAutoNetPlayerCount();
    }
    bool bHeroesEnabled = netHeroesEnabled;

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    int iDifficulty = eDIFFICULTY_HARD;
    if (pProfile)
    {
        iDifficulty = pProfile->mGameOptions.mCurDiffLevel;
    }
    if (netDifficulty)
    {
        iDifficulty = netDifficulty;
    }
    if (iDifficulty < eDIFFICULTY_EASY)
    {
        iDifficulty = eDIFFICULTY_EASY;
    }
    else if (iDifficulty > eDIFFICULTY_HARD)
    {
        iDifficulty = eDIFFICULTY_HARD;
    }

    if (NetGame::IsHost())
    {
        PlayerProfile::tProfile* pCur = PlayerProfile::GetCurProfile();
        if (pCur && !NetComm::IsLAN())
        {
            int iSupported = pCur->mOnlineOptions.mu8PlayersSupported;
            if (iSupported >= 9)
            {
                iSupported = 8;
            }
            int iLimit = Lua_Callbacks::netPlayersSupported[iSupported];
            if (netThrottle)
            {
                iLimit = 64;
            }
            if (iLimit < iMaxDedicatedPlayers)
            {
                iMaxDedicatedPlayers = iLimit;
            }
            if (iLimit + 1 < iMaxPlayers)
            {
                iMaxPlayers = iLimit + 1;
            }
        }
        if (iNumPlayers > iMaxPlayers)
        {
            iNumPlayers = iMaxPlayers;
        }
    }
    else if (iMaxDedicatedPlayers > iMaxPlayers)
    {
        iMaxPlayers = iMaxDedicatedPlayers;
    }

    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("iNumPlayers", iNumPlayers);
    LuaHelper::TableWriteVal("iMaxPlayers", iMaxPlayers);
    LuaHelper::TableWriteVal("iWarmUp", iWarmUp);
    LuaHelper::TableWriteVal("iVote", iVote);
    LuaHelper::TableWriteVal("iNumBots", iNumBots);
    LuaHelper::TableWriteVal("iMaxBots", 32);
    LuaHelper::TableWriteVal("iTeamDmg", iTeamDmg);
    LuaHelper::TableWriteVal("iAutoAim", iAutoAim);
    LuaHelper::TableWriteVal("bShowNames", bShowNames);
    LuaHelper::TableWriteVal("bIsPrivate", bIsPrivate);
    LuaHelper::TableWriteVal("bAutoAssignTeams", bAutoAssignTeams);
    LuaHelper::TableWriteVal("iStartCnt", iStartCnt);
    LuaHelper::TableWriteVal("bHeroesEnabled", bHeroesEnabled);
    LuaHelper::TableWriteVal("iDifficulty", iDifficulty);
    LuaHelper::TableWriteVal("iMaxDedicatedPlayers", iMaxDedicatedPlayers);
    LuaHelper::TableWriteVal("iMaxDedicatedBots", 32);
    LuaHelper::TableWriteVal("iVoiceMode", ((VoiceChat::Transport*)GameVoiceChat::GetTransport())->GetMethod().GetType());
    LuaHelper::TableWriteVal("iVoiceModeMin", 1);
    LuaHelper::TableWriteVal("iVoiceModeMax", 4);
    LuaHelper::TableWriteVal("iVoiceModeDedicatedMax", 4);
    LuaHelper::TableWriteVal("iDMMult", gDMReinforcementMult[gCurRules]);
    LuaHelper::TableWriteVal("iCONMult", gCONReinforcementMult[gCurRules]);

    int iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gCONNumBots[gCurRules] : netNumBots;
    LuaHelper::TableWriteVal("iCONNumBots", iBots < 32 ? iBots : 32);
    iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gCTFNumBots[gCurRules] : netNumBots;
    LuaHelper::TableWriteVal("iCTFNumBots", iBots < 32 ? iBots : 32);
    iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gELINumBots[gCurRules] : netNumBots;
    LuaHelper::TableWriteVal("iELINumBots", iBots < 32 ? iBots : 32);
    iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gASSNumBots[gCurRules] : netNumBots;
    LuaHelper::TableWriteVal("iASSNumBots", iBots < 32 ? iBots : 32);

    LuaHelper::TableWriteVal("iASSScoreLimit", gSpaceAssaultScoreLimit[gCurRules]);
    LuaHelper::TableWriteVal("iCTFScore", gCTFScore[gCurRules]);
    LuaHelper::TableWriteVal("iHUNTScoreLimit", gHuntScoreLimit[gCurRules]);
    LuaHelper::TableWriteVal("iHUNTimer", gHuntMaxTimeLimit[gCurRules]);
    LuaHelper::TableWriteVal("iCTFTimer", gCTFMaxTimeLimit[gCurRules]);
    LuaHelper::TableWriteVal("iCONTimer", gCONMaxTimeLimit[gCurRules]);
    LuaHelper::TableWriteVal("bRandomizePlaylist", gRandomizePlaylist);
    LuaHelper::TableWriteVal("bIsDedicated", gIsDedicated);
    LuaHelper::TableWriteVal("bIsPlayerAwardsEnabled", GetNetAwardsEnabled());
    return 1;
}

// FUNCTION: SWBF2 0x00598b00
int ScriptCB_SetNetGameDefaults(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_istable(LuaHelper::mState, 1))
    {
        int iNumPlayers, iWarmUp, iVote, iNumBots, iTeamDmg, iAutoAim;
        LuaHelper::TableReadVal("iNumPlayers", iNumPlayers, false);
        LuaHelper::TableReadVal("iWarmUp", iWarmUp, false);
        LuaHelper::TableReadVal("iVote", iVote, false);
        LuaHelper::TableReadVal("iNumBots", iNumBots, false);
        LuaHelper::TableReadVal("iTeamDmg", iTeamDmg, false);
        LuaHelper::TableReadVal("iAutoAim", iAutoAim, false);

        bool bIsPrivate, bShowNames, bAutoAssignTeams;
        LuaHelper::TableReadVal("bIsPrivate", bIsPrivate);
        LuaHelper::TableReadVal("bShowNames", bShowNames);
        LuaHelper::TableReadVal("bAutoAssignTeams", bAutoAssignTeams);
        GameLoop::SetCanSwitchSides(NetGame::IsEnabled() ? !bAutoAssignTeams : true);

        int iStartCnt;
        LuaHelper::TableReadVal("iStartCnt", iStartCnt, false);
        bool bHeroesEnabled;
        LuaHelper::TableReadVal("bHeroesEnabled", bHeroesEnabled);
        int iDifficulty;
        LuaHelper::TableReadVal("iDifficulty", iDifficulty, false);

        char acPassword[128];
        LuaHelper::TableReadVal("PasswordStr", acPassword, sizeof(acPassword), true);
        NetSetup::SetPassword(acPassword);

        int iValue;
        LuaHelper::TableReadVal("iVoiceMode", iValue, false);
        void* pTransport = GameVoiceChat::GetTransport();
        VoiceChat::Transport::Method cMethod((VoiceChat::Transport::Method::Type)iValue,
            ((VoiceChat::Transport*)pTransport)->GetMethod().GetNumRelayNodes());
        ((VoiceChat::Transport*)pTransport)->SetMethod(&cMethod, 1);

        NetGame::SetMaxPreGameTime(iWarmUp);
        hostVoteThreshold = iVote;
        netPlayerLimit = iNumPlayers;
        if (!g_bNoRender)
        {
            netNumBots = iNumBots;
        }
        netAutoAssignTeams = bAutoAssignTeams;
        netHeroesEnabled = bHeroesEnabled;
        netShowNames = bShowNames;
        netMinPlayersToAutoLaunchDedicated = iStartCnt;
        netAutoAssignTeamsOriginalSetting = bAutoAssignTeams;
        if (iDifficulty < eDIFFICULTY_EASY || iDifficulty >= eDIFFICULTY_LAST)
        {
            iDifficulty = eDIFFICULTY_MEDIUM;
        }
        PlayerPrefs::mAllUserPrefs[0].mDifficulty = (eGAME_DIFFICULTY)iDifficulty;
        netTeamDamage = iTeamDmg > 0;
        netDifficulty = iDifficulty;
        netAutoAim = iAutoAim > 0;
        NetSetup::mPrivateGame = bIsPrivate;
        gGlobalNumBots = iNumBots;
        if (netRequestedCameraCount > 0)
        {
            GameLoop::mNumCameras = netRequestedCameraCount;
        }

        LuaHelper::TableReadVal("iDMMult", gDMReinforcementMult[gCurRules], false);
        LuaHelper::TableReadVal("iCONMult", gCONReinforcementMult[gCurRules], false);
        LuaHelper::TableReadVal("iCONNumBots", iValue, false);
        if ((unsigned int)gCurRules <= eGAMERULES_METAGAME)
        {
            gCONNumBots[gCurRules] = iValue;
        }
        LuaHelper::TableReadVal("iCTFNumBots", iValue, false);
        if ((unsigned int)gCurRules <= eGAMERULES_METAGAME)
        {
            gCTFNumBots[gCurRules] = iValue;
        }
        LuaHelper::TableReadVal("iELINumBots", iValue, false);
        if ((unsigned int)gCurRules <= eGAMERULES_METAGAME)
        {
            gELINumBots[gCurRules] = iValue;
        }
        LuaHelper::TableReadVal("iASSNumBots", iValue, false);
        if ((unsigned int)gCurRules <= eGAMERULES_METAGAME)
        {
            gASSNumBots[gCurRules] = iValue;
        }
        LuaHelper::TableReadVal("iASSScoreLimit", gSpaceAssaultScoreLimit[gCurRules], false);
        LuaHelper::TableReadVal("iCTFScore", gCTFScore[gCurRules], false);
        LuaHelper::TableReadVal("iHUNTScoreLimit", gHuntScoreLimit[gCurRules], false);
        LuaHelper::TableReadVal("iHUNTimer", gHuntMaxTimeLimit[gCurRules], false);
        LuaHelper::TableReadVal("iCTFTimer", gCTFMaxTimeLimit[gCurRules], false);
        LuaHelper::TableReadVal("iCONTimer", gCONMaxTimeLimit[gCurRules], false);
        LuaHelper::TableReadVal("bRandomizePlaylist", gRandomizePlaylist);
        LuaHelper::TableReadVal("bIsDedicated", gIsDedicated);
        Lua_Callbacks::gTeamSetReinforcementMult = 0;

        bool bAwards = true;
        LuaHelper::TableReadVal("bIsPlayerAwardsEnabled", bAwards);
        SetNetAwardsEnabled(bAwards);
        if (gIsDedicated)
        {
            NetSetup::SetAsDedicated(true);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00598ee0
int ScriptCB_GetNetHeroDefaults(lua_State* L)
{
    int iHeroUnlock = (netHeroRuleUnlock == 3) ? 3 : 5;
    int iHeroUnlockVal = netHeroRuleUnlockVal;
    int iHeroPlayer;
    if (netHeroRulePlayer < 4)
    {
        iHeroPlayer = 1;
    }
    else
    {
        iHeroPlayer = (netHeroRulePlayer < 7) ? 4 : 7;
    }
    int iHeroTeam = netHeroRuleTeam;
    int iHeroRespawn = netHeroRuleRespawn;
    int iHeroRespawnVal = netHeroRuleRespawnVal;

    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("iHeroUnlock", iHeroUnlock);
    LuaHelper::TableWriteVal("iHeroUnlockVal", iHeroUnlockVal);
    LuaHelper::TableWriteVal("iHeroPlayer", iHeroPlayer);
    LuaHelper::TableWriteVal("iHeroRespawn", iHeroRespawn);
    LuaHelper::TableWriteVal("iHeroRespawnVal", iHeroRespawnVal);
    LuaHelper::TableWriteVal("iHeroTeam", iHeroTeam);
    return 1;
}

// FUNCTION: SWBF2 0x00598fb0
int ScriptCB_SetNetHeroDefaults(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_istable(LuaHelper::mState, 1))
    {
        int iValue;
        LuaHelper::TableReadVal("iHeroUnlock", iValue, false);
        netHeroRuleUnlock = iValue;
        LuaHelper::TableReadVal("iHeroUnlockVal", iValue, false);
        netHeroRuleUnlockVal = iValue;
        LuaHelper::TableReadVal("iHeroPlayer", iValue, false);
        netHeroRulePlayer = iValue;
        LuaHelper::TableReadVal("iHeroRespawn", iValue, false);
        netHeroRuleRespawn = iValue;
        LuaHelper::TableReadVal("iHeroRespawnVal", iValue, false);
        netHeroRuleRespawnVal = iValue;
        LuaHelper::TableReadVal("iHeroTeam", iValue, false);
        netHeroRuleTeam = iValue;
        netHeroRuleRespawn = (netHeroRuleRespawnVal >= 0) ? 2 : 1;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596c40
int ScriptCB_WasSplitscreen(lua_State*)
{
    if (ShellLoop::mLastGameNumCameras >= 2)
    {
        lua_pushnumber(LuaHelper::mState, (float)ShellLoop::mLastGameNumCameras);
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00596e30
int ScriptCB_GetCTFCaptureLimit(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)gCTFScore[gCurRules]);
    return 1;
}

// FUNCTION: SWBF2 0x00596e60
int ScriptCB_GetHuntMaxTimeLimit(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)(60 * gHuntMaxTimeLimit[gCurRules]));
    return 1;
}

// FUNCTION: SWBF2 0x00596f00
int ScriptCB_GetCTFMaxTimeLimit(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)(60 * gCTFMaxTimeLimit[gCurRules]));
    return 1;
}

// FUNCTION: SWBF2 0x00596f40
int ScriptCB_GetAssaultScoreLimit(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)gSpaceAssaultScoreLimit[gCurRules]);
    return 1;
}

// FUNCTION: SWBF2 0x00597050
int ScriptCB_GetCTFNumBots(lua_State*)
{
    int iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gCTFNumBots[gCurRules] : netNumBots;
    lua_pushnumber(LuaHelper::mState, (float)iBots);
    return 1;
}

// FUNCTION: SWBF2 0x00597090
int ScriptCB_GetASSNumBots(lua_State*)
{
    int iBots = (unsigned int)gCurRules <= eGAMERULES_METAGAME ? gASSNumBots[gCurRules] : netNumBots;
    lua_pushnumber(LuaHelper::mState, (float)iBots);
    return 1;
}

// FUNCTION: SWBF2 0x005970d0
static int ScriptCB_SetNumBots(lua_State*)
{
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        netNumBots = (int)luaL_checknumber(LuaHelper::mState, 1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005977c0
int ScriptCB_SetAIDifficulty(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x00597460
int ScriptCB_SetSpectatorMode(lua_State*)
{
    int iLocal = (int)luaL_checknumber(LuaHelper::mState, 1);
    bool bEnabled = lua_toboolean(LuaHelper::mState, 2) != 0;
    g_SpectatorEnabled[iLocal] = bEnabled;
    NetGame::SetSpectator(NetGame::GetLocalIndex(iLocal), bEnabled ? 0 : -1);
    return 0;
}

// FUNCTION: SWBF2 0x005974d0
int ScriptCB_GetSpectatorMode(lua_State*)
{
    int iLocal = (int)luaL_checknumber(LuaHelper::mState, 1);
    lua_pushnumber(LuaHelper::mState, (float)g_SpectatorEnabled[iLocal]);
    return 1;
}

// FUNCTION: SWBF2 0x00597520
int ScriptCB_SetCanSwitchSides(lua_State*)
{
    GameLoop::SetCanSwitchSides(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x005977d0
int ScriptCB_IsMetaAllMapsOn(lua_State*)
{
    bool bEnabled = false;
    if (lua_gettop(LuaHelper::mState) == 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        if (pProfile)
        {
            bEnabled = pProfile->mGameOptions.mbMetaAllMaps;
        }
    }
    lua_pushboolean(LuaHelper::mState, bEnabled);
    return 1;
}

// FUNCTION: SWBF2 0x00597810
int ScriptCB_SetMetaAllMapsOn(lua_State*)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    bool bEnabled = lua_toboolean(LuaHelper::mState, 1) != 0;
    if (iArgs == 1)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        if (pProfile)
        {
            if (pProfile->mGameOptions.mbMetaAllMaps != bEnabled)
            {
                pProfile->mGameOptions.mbMetaAllMaps = bEnabled;
                pProfile->UpdateProfCRC();
            }
            pProfile->ApplySettings();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597110
int ScriptCB_GetFunctionIdForAnalogId(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 ||
        !lua_isnumber(LuaHelper::mState, 1) || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    luaL_checknumber(LuaHelper::mState, 1);
    luaL_checknumber(LuaHelper::mState, 2);
    lua_pushnumber(LuaHelper::mState, (float)GameLoop::GetFunctionIdForAnalog());
    return 1;
}

// FUNCTION: SWBF2 0x005971a0
int ScriptCB_SetFunctionIdForAnalogId(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 3 ||
        !lua_isnumber(LuaHelper::mState, 1) ||
        !lua_isnumber(LuaHelper::mState, 2) ||
        !lua_isnumber(LuaHelper::mState, 3))
    {
        return 0;
    }
    luaL_checknumber(LuaHelper::mState, 1);
    luaL_checknumber(LuaHelper::mState, 2);
    luaL_checknumber(LuaHelper::mState, 3);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->NoteBindingChange();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005972b0
int ScriptCB_GetFunctionIdForButtonId(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 ||
        !lua_isnumber(LuaHelper::mState, 1) || !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    int iButton = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    int iMode = (int)luaL_checknumber(LuaHelper::mState, 2);
    int iEnum = GameLoop::GetFunctionIdForButtonId(LuaButtonIndexToEnum(iButton));
    lua_pushnumber(LuaHelper::mState, (float)EnumToLuaFunctionIndex(iEnum, iMode));
    return 1;
}

// FUNCTION: SWBF2 0x00597370
int ScriptCB_SetFunctionIdForButtonId(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 3 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iButton = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    int iFunction = -1;
    if (lua_isnumber(LuaHelper::mState, 2) && lua_isnumber(LuaHelper::mState, 3))
    {
        int iFunctionIndex = (int)luaL_checknumber(LuaHelper::mState, 2);
        int iMode = (int)luaL_checknumber(LuaHelper::mState, 3);
        iFunction = LuaFunctionIndexToEnum(iFunctionIndex, iMode);
    }
    GameLoop::SetFunctionIdForButtonId(LuaButtonIndexToEnum(iButton), iFunction);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->NoteBindingChange();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596c80
int ScriptCB_SetPlayerSide(lua_State*)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if ((unsigned)(iArgs - 1) <= 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        int iSide = (int)luaL_checknumber(LuaHelper::mState, 1);
        if ((unsigned)(iSide - 1) <= 1)
        {
            int iPlayer = 0;
            if (iArgs != 2 || (LuaHelper::IsNumber(iArgs) && (iPlayer = LuaHelper::GetInt(iArgs)) == 0))
            {
                playerTeamNumbers[11 * iPlayer] = iSide;
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00597950
int ScriptCB_SetMouseTurnAssist(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iAssist = (int)luaL_checknumber(LuaHelper::mState, 1);
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
    ((char*)pJoystick)[0x2642 + pJoystick->GetInputMode()] = iAssist == 1;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        pProfile->NoteBindingChange();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005979e0
int ScriptCB_GetControlScale(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    int iScale = 0;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
    if (pProfile)
    {
        int iAxis = (int)luaL_checknumber(LuaHelper::mState, 1);
        RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());
        iScale = pProfile->mGameOptions.mControllerScale[iAxis][pJoystick->GetInputMode()];
    }
    lua_pushnumber(LuaHelper::mState, (float)iScale);
    return 1;
}

// GLOBAL: SWBF2 0x007e8050
static const CallbackEntryT sOptionsCallbacks[61] =
{
    { "ScriptCB_SetSplitscreen", ScriptCB_SetSplitscreen },
    { "ScriptCB_GetNumCameras", ScriptCB_GetNumCameras },
    { "ScriptCB_IsSplitscreen", ScriptCB_IsSplitscreen },
    { "ScriptCB_WasSplitscreen", ScriptCB_WasSplitscreen },
    { "ScriptCB_SetPlayerSide", ScriptCB_SetPlayerSide },
    { "ScriptCB_SetDifficulty", ScriptCB_SetDifficulty },
    { "ScriptCB_GetDifficulty", ScriptCB_GetDifficulty },
    { "ScriptCB_GetCTFCaptureLimit", ScriptCB_GetCTFCaptureLimit },
    { "ScriptCB_GetHuntMaxTimeLimit", ScriptCB_GetHuntMaxTimeLimit },
    { "ScriptCB_GetHuntScoreLimit", ScriptCB_GetHuntScoreLimit },
    { "ScriptCB_ShowHuntScoreLimit", ScriptCB_ShowHuntScoreLimit },
    { "ScriptCB_GetAssaultScoreLimit", ScriptCB_GetAssaultScoreLimit },
    { "ScriptCB_GetUberScoreLimit", ScriptCB_GetUberScoreLimit },
    { "ScriptCB_SetUberScoreLimit", ScriptCB_SetUberScoreLimit },
    { "ScriptCB_GetCTFMaxTimeLimit", ScriptCB_GetCTFMaxTimeLimit },
    { "ScriptCB_GetCONMaxTimeLimit", ScriptCB_GetCONMaxTimeLimit },
    { "ScriptCB_GetCONNumBots", ScriptCB_GetCONNumBots },
    { "ScriptCB_GetCTFNumBots", ScriptCB_GetCTFNumBots },
    { "ScriptCB_GetASSNumBots", ScriptCB_GetASSNumBots },
    { "ScriptCB_SetNumBots", ScriptCB_SetNumBots },
    { "ScriptCB_GetFunctionIdForButtonId", ScriptCB_GetFunctionIdForButtonId },
    { "ScriptCB_SetFunctionIdForButtonId", ScriptCB_SetFunctionIdForButtonId },
    { "ScriptCB_GetFunctionIdForAnalogId", ScriptCB_GetFunctionIdForAnalogId },
    { "ScriptCB_SetFunctionIdForAnalogId", ScriptCB_SetFunctionIdForAnalogId },
    { "ScriptCB_GetYAxisFlip", ScriptCB_GetYAxisFlip },
    { "ScriptCB_SetYAxisFlip", ScriptCB_SetYAxisFlip },
    { "ScriptCB_GetMouseTurnAssist", ScriptCB_GetMouseTurnAssist },
    { "ScriptCB_SetMouseTurnAssist", ScriptCB_SetMouseTurnAssist },
    { "ScriptCB_GetControlScale", ScriptCB_GetControlScale },
    { "ScriptCB_SetControlScale", ScriptCB_SetControlScale },
    { "ScriptCB_ResetControlsToDefault", ScriptCB_ResetControlsToDefault },
    { "ScriptCB_ResetGameOptionsToDefault", ScriptCB_ResetGameOptionsToDefault },
    { "ScriptCB_ResetSoundToDefault", ScriptCB_ResetSoundToDefault },
    { "ScriptCB_ResetOnlineOptionsToDefault", ScriptCB_ResetOnlineOptionsToDefault },
    { "ScriptCB_GetGeneralOptions", ScriptCB_GetGeneralOptions },
    { "ScriptCB_SetGeneralOptions", ScriptCB_SetGeneralOptions },
    { "ScriptCB_IsHorizontalSplitScreen", ScriptCB_IsHorizontalSplitScreen },
    { "ScriptCB_SetHorizontalSplitScreen", ScriptCB_SetHorizontalSplitScreen },
    { "ScriptCB_SetHUDScale", ScriptCB_SetHUDScale },
    { "ScriptCB_GetHUDScale", ScriptCB_GetHUDScale },
    { "ScriptCB_NextToolTipState", ScriptCB_NextToolTipState },
    { "ScriptCB_PrevToolTipState", ScriptCB_PrevToolTipState },
    { "ScriptCB_SetToolTipState", ScriptCB_SetToolTipState },
    { "ScriptCB_ResetToolTips", ScriptCB_ResetToolTips },
    { "ScriptCB_SetControlMode", ScriptCB_SetControlMode },
    { "ScriptCB_GetControlMode", ScriptCB_GetControlMode },
    { "ScriptCB_IsMetaAllMapsOn", ScriptCB_IsMetaAllMapsOn },
    { "ScriptCB_SetMetaAllMapsOn", ScriptCB_SetMetaAllMapsOn },
    { "ScriptCB_SetCanSwitchSides", ScriptCB_SetCanSwitchSides },
    { "ScriptCB_SetGameRules", ScriptCB_SetGameRules },
    { "ScriptCB_GetGameRules", ScriptCB_GetGameRules },
    { "ScriptCB_SetAIDifficulty", ScriptCB_SetAIDifficulty },
    { "ScriptCB_SetSpectatorMode", ScriptCB_SetSpectatorMode },
    { "ScriptCB_GetSpectatorMode", ScriptCB_GetSpectatorMode },
    { "ScriptCB_GetSideSelectButtonSetting", ScriptCB_GetSideSelectButtonSetting },
    { "ScriptCB_SetOnlineOpts", ScriptCB_SetOnlineOpts },
    { "ScriptCB_GetOnlineOpts", ScriptCB_GetOnlineOpts },
    { "ScriptCB_GetNetGameDefaults", ScriptCB_GetNetGameDefaults },
    { "ScriptCB_SetNetGameDefaults", ScriptCB_SetNetGameDefaults },
    { "ScriptCB_GetNetHeroDefaults", ScriptCB_GetNetHeroDefaults },
    { "ScriptCB_SetNetHeroDefaults", ScriptCB_SetNetHeroDefaults },
};

// FUNCTION: SWBF2 0x00599090
void InitOptions()
{
    for (int i = 0; i < 61; ++i)
    {
        LuaHelper::Register(sOptionsCallbacks[i].pcName, sOptionsCallbacks[i].pfnCallback, 0);
    }
}

}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00597540
void _SetGameRules(eGAME_RULES eRules)
{
    for (int i = 0; i < eGAMERULES_MAX; ++i)
    {
        if (gNumBots[i] < 0)
        {
            gNumBots[i] = netNumBots;
        }
        if (gCONNumBots[i] < 0)
        {
            gCONNumBots[i] = netNumBots;
        }
        if (gCTFNumBots[i] < 0)
        {
            gCTFNumBots[i] = netNumBots;
        }
        if (gELINumBots[i] < 0)
        {
            gELINumBots[i] = netNumBots;
        }
        if (gASSNumBots[i] < 0)
        {
            gASSNumBots[i] = netNumBots;
        }
    }

    if (gCurRules < eGAMERULES_CAMPAIGN)
    {
        gHeroesEnabled[gCurRules] = netHeroesEnabled;
        gHeroRuleUnlock[gCurRules] = netHeroRuleUnlock;
        gHeroRuleUnlockVal[gCurRules] = netHeroRuleUnlockVal;
        gHeroRuleTeam[gCurRules] = netHeroRuleTeam;
        gHeroRulePlayer[gCurRules] = netHeroRulePlayer;
        gHeroRuleRespawn[gCurRules] = netHeroRuleRespawn;
        gNumBots[gCurRules] = netNumBots;
        gHeroRuleRespawnVal[gCurRules] = netHeroRuleRespawnVal;
    }

    netNumBots = gNumBots[eRules];
    netHeroesEnabled = gHeroesEnabled[eRules];
    netHeroRuleUnlock = gHeroRuleUnlock[eRules];
    netHeroRuleUnlockVal = gHeroRuleUnlockVal[eRules];
    netHeroRuleTeam = gHeroRuleTeam[eRules];
    netHeroRulePlayer = gHeroRulePlayer[eRules];
    netHeroRuleRespawn = gHeroRuleRespawn[eRules];
    netHeroRuleRespawnVal = gHeroRuleRespawnVal[eRules];
    gCurRules = eRules;

    switch (eRules)
    {
    case eGAMERULES_INSTANTACTION:
        GameLoop::SetCanSwitchSides(true);
        GameLoop::mbHistoricalRules = false;
        GameLoop::mbMetagameRules = false;
        break;
    case eGAMERULES_MULTIPLAYER:
        GameLoop::mbHistoricalRules = false;
        GameLoop::mbMetagameRules = false;
        break;
    case eGAMERULES_CAMPAIGN:
        GameLoop::mbHistoricalRules = true;
        GameLoop::mbMetagameRules = false;
        GameLoop::SetCanSwitchSides(false);
        break;
    case eGAMERULES_METAGAME:
        GameLoop::mbHistoricalRules = false;
        GameLoop::mbMetagameRules = true;
        GameLoop::SetCanSwitchSides(false);
        break;
    }
}
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e737c
bool gHeroesEnabled[4] = { true, true, true, false };
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e7fc0
int gHeroRuleUnlock[4] = { 3, 3, 7, 1 };
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e7f60
int gHeroRuleUnlockVal[4] = { 10, 10, 1, 1 };
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e7f70
int gHeroRuleTeam[4] = { 3, 3, 3, 3 };
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e7fa0
int gHeroRulePlayer[4] = { 1, 1, 1, 1 };
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e7f10
int gHeroRuleRespawn[4] = { 2, 2, 1, 1 };
}

namespace Lua_Callbacks {
// GLOBAL: SWBF2 0x007e7f30
int gHeroRuleRespawnVal[4] = { 90, 90, -1, -1 };
}

namespace Lua_Callbacks {

// GLOBAL: SWBF2 0x01e57944
int gHuntShowScoreLimit;
}

// GLOBAL: SWBF2 0x007e8e7a
bool netAwardsEnabled = true;

// FUNCTION: SWBF2 0x005d3690
bool GetNetAwardsEnabled()
{
    return netAwardsEnabled;
}

// GLOBAL: SWBF2 0x00788890
static const unsigned char kBase64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// FUNCTION: SWBF2 0x00448310
int Base64Encode(unsigned char* dst, unsigned int capacity, unsigned int* length,
               const unsigned char* src, unsigned int size)
{
    if (!size)
    {
        *length = 0;
        return 0;
    }
    unsigned int groups = size / 3 + (size % 3 != 0);
    if (groups > 0x3fffffff)
    {
        *length = ~0u;
        return -42;
    }
    if (capacity < groups * 4 + 1)
    {
        *length = groups * 4 + 1;
        return -42;
    }
    unsigned char* out = dst;
    unsigned int i = 0;
    for (; i < size / 3 * 3; i += 3, src += 3)
    {
        *out++ = kBase64Alphabet[src[0] >> 2];
        *out++ = kBase64Alphabet[((src[0] << 4) | (src[1] >> 4)) & 63];
        *out++ = kBase64Alphabet[((src[1] << 2) | (src[2] >> 6)) & 63];
        *out++ = kBase64Alphabet[src[2] & 63];
    }
    if (i < size)
    {
        unsigned int second = i + 1 < size ? src[1] : 0;
        *out++ = kBase64Alphabet[src[0] >> 2];
        *out++ = kBase64Alphabet[((src[0] << 4) | (second >> 4)) & 63];
        *out++ = i + 1 < size ? kBase64Alphabet[(second & 15) << 2] : '=';
        *out++ = '=';
    }
    *length = out - dst;
    *out = 0;
    return 0;
}

// FUNCTION: SWBF2 0x006347b0
void Sha512WipeContext(Sha512Context* ctx)
{
    volatile unsigned char* p = reinterpret_cast<volatile unsigned char*>(ctx);
    for (unsigned int i = 0; i < sizeof(*ctx); ++i)
    {
        *p++ = 0;
    }
}

// FUNCTION: SWBF2 0x006347d0
void Sha512ClearContext(Sha512Context* ctx)
{
    memset(ctx, 0, sizeof(*ctx));
}

// FUNCTION: SWBF2 0x006347f0
void Sha512WipeContextIfPresent(Sha512Context* ctx)
{
    if (ctx)
    {
        Sha512WipeContext(ctx);
    }
}

// FUNCTION: SWBF2 0x00634800
void Sha512InitContext(Sha512Context* ctx, int mode)
{
    ctx->total[0] = ctx->total[1] = 0;
    ctx->mode = mode;
    if (!mode)
    {
        ctx->state[0] = 0x6a09e667f3bcc908ui64;
        ctx->state[1] = 0xbb67ae8584caa73bui64;
        ctx->state[2] = 0x3c6ef372fe94f82bui64;
        ctx->state[3] = 0xa54ff53a5f1d36f1ui64;
        ctx->state[4] = 0x510e527fade682d1ui64;
        ctx->state[5] = 0x9b05688c2b3e6c1fui64;
        ctx->state[6] = 0x1f83d9abfb41bd6bui64;
        ctx->state[7] = 0x5be0cd19137e2179ui64;
    }
    else
    {
        ctx->state[0] = 0xcbbb9d5dc1059ed8ui64;
        ctx->state[1] = 0x629a292a367cd507ui64;
        ctx->state[2] = 0x9159015a3070dd17ui64;
        ctx->state[3] = 0x152fecd8f70e5939ui64;
        ctx->state[4] = 0x67332667ffc00b31ui64;
        ctx->state[5] = 0x8eb44a8768581511ui64;
        ctx->state[6] = 0xdb0c2e0d64f98fa7ui64;
        ctx->state[7] = 0x47b5481dbefa4fa4ui64;
    }
}

// GLOBAL: SWBF2 0x0078b1a0
static const unsigned __int64 kSha512RoundConstants[80] = {
    0x428a2f98d728ae22ui64, 0x7137449123ef65cdui64, 0xb5c0fbcfec4d3b2fui64, 0xe9b5dba58189dbbcui64,
    0x3956c25bf348b538ui64, 0x59f111f1b605d019ui64, 0x923f82a4af194f9bui64, 0xab1c5ed5da6d8118ui64,
    0xd807aa98a3030242ui64, 0x12835b0145706fbeui64, 0x243185be4ee4b28cui64, 0x550c7dc3d5ffb4e2ui64,
    0x72be5d74f27b896fui64, 0x80deb1fe3b1696b1ui64, 0x9bdc06a725c71235ui64, 0xc19bf174cf692694ui64,
    0xe49b69c19ef14ad2ui64, 0xefbe4786384f25e3ui64, 0x0fc19dc68b8cd5b5ui64, 0x240ca1cc77ac9c65ui64,
    0x2de92c6f592b0275ui64, 0x4a7484aa6ea6e483ui64, 0x5cb0a9dcbd41fbd4ui64, 0x76f988da831153b5ui64,
    0x983e5152ee66dfabui64, 0xa831c66d2db43210ui64, 0xb00327c898fb213fui64, 0xbf597fc7beef0ee4ui64,
    0xc6e00bf33da88fc2ui64, 0xd5a79147930aa725ui64, 0x06ca6351e003826fui64, 0x142929670a0e6e70ui64,
    0x27b70a8546d22ffcui64, 0x2e1b21385c26c926ui64, 0x4d2c6dfc5ac42aedui64, 0x53380d139d95b3dfui64,
    0x650a73548baf63deui64, 0x766a0abb3c77b2a8ui64, 0x81c2c92e47edaee6ui64, 0x92722c851482353bui64,
    0xa2bfe8a14cf10364ui64, 0xa81a664bbc423001ui64, 0xc24b8b70d0f89791ui64, 0xc76c51a30654be30ui64,
    0xd192e819d6ef5218ui64, 0xd69906245565a910ui64, 0xf40e35855771202aui64, 0x106aa07032bbd1b8ui64,
    0x19a4c116b8d2d0c8ui64, 0x1e376c085141ab53ui64, 0x2748774cdf8eeb99ui64, 0x34b0bcb5e19b48a8ui64,
    0x391c0cb3c5c95a63ui64, 0x4ed8aa4ae3418acbui64, 0x5b9cca4f7763e373ui64, 0x682e6ff3d6b2b8a3ui64,
    0x748f82ee5defb2fcui64, 0x78a5636f43172f60ui64, 0x84c87814a1f0ab72ui64, 0x8cc702081a6439ecui64,
    0x90befffa23631e28ui64, 0xa4506cebde82bde9ui64, 0xbef9a3f7b2c67915ui64, 0xc67178f2e372532bui64,
    0xca273eceea26619cui64, 0xd186b8c721c0c207ui64, 0xeada7dd6cde0eb1eui64, 0xf57d4f7fee6ed178ui64,
    0x06f067aa72176fbaui64, 0x0a637dc5a2c898a6ui64, 0x113f9804bef90daeui64, 0x1b710b35131c471bui64,
    0x28db77f523047d84ui64, 0x32caab7b40c72493ui64, 0x3c9ebe0a15c9bebcui64, 0x431d67c49c100d4cui64,
    0x4cc5d4becb3e42b6ui64, 0x597f299cfc657e2aui64, 0x5fcb6fab3ad6faecui64, 0x6c44198c4a475817ui64
};

// FUNCTION: SWBF2 0x00634910
void Sha512TransformBlock(Sha512Context* ctx, const unsigned char* block)
{
    unsigned __int64 w[80];
    for (int i = 0; i < 16; ++i)
    {
        w[i] = 0;
        for (int j = 0; j < 8; ++j)
        {
            w[i] = (w[i] << 8) | block[i * 8 + j];
        }
    }
    for (int i = 16; i < 80; ++i)
    {
        unsigned __int64 x = w[i - 15], y = w[i - 2];
        w[i] = w[i - 16] + (ShaRotate(x, 1) ^ ShaRotate(x, 8) ^ (x >> 7))
             + w[i - 7] + (ShaRotate(y, 19) ^ ShaRotate(y, 61) ^ (y >> 6));
    }
    unsigned __int64 a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3];
    unsigned __int64 e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];
    for (int i = 0; i < 80; ++i)
    {
        unsigned __int64 t1 = h + (ShaRotate(e, 14) ^ ShaRotate(e, 18) ^ ShaRotate(e, 41))
            + (g ^ (e & (f ^ g))) + kSha512RoundConstants[i] + w[i];
        unsigned __int64 t2 = (ShaRotate(a, 28) ^ ShaRotate(a, 34) ^ ShaRotate(a, 39))
            + ((a & b) | (c & (a | b)));
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

// FUNCTION: SWBF2 0x00635500
void Sha512Update(Sha512Context* ctx, const unsigned char* input, unsigned int size)
{
    if (!size)
    {
        return;
    }
    unsigned int left = static_cast<unsigned int>(ctx->total[0]) & 127;
    unsigned int fill = 128 - left;
    ctx->total[0] += size;
    if (ctx->total[0] < size)
    {
        ++ctx->total[1];
    }
    if (left && size >= fill)
    {
        memcpy(ctx->buffer + left, input, fill);
        Sha512TransformBlock(ctx, ctx->buffer);
        input += fill;
        size -= fill;
        left = 0;
    }
    while (size >= 128)
    {
        Sha512TransformBlock(ctx, input);
        input += 128;
        size -= 128;
    }
    if (size)
    {
        memcpy(ctx->buffer + left, input, size);
    }
}

// GLOBAL: SWBF2 0x0078b120
static const unsigned char kSha512Padding[128] = { 0x80 };

// FUNCTION: SWBF2 0x006355c0
void Sha512Finish(Sha512Context* ctx, unsigned char* output)
{
    unsigned __int64 high = (ctx->total[1] << 3) | (ctx->total[0] >> 61);
    unsigned __int64 low = ctx->total[0] << 3;
    unsigned char length[16];
    for (int i = 0; i < 8; ++i)
    {
        length[i] = static_cast<unsigned char>(high >> (56 - i * 8));
        length[i + 8] = static_cast<unsigned char>(low >> (56 - i * 8));
    }
    unsigned int left = static_cast<unsigned int>(ctx->total[0]) & 127;
    Sha512Update(ctx, kSha512Padding, (left < 112 ? 112 : 240) - left);
    Sha512Update(ctx, length, 16);
    for (int i = 0; i < (ctx->mode ? 6 : 8); ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            output[i * 8 + j] = static_cast<unsigned char>(ctx->state[i] >> (56 - j * 8));
        }
    }
}

// FUNCTION: SWBF2 0x00635a00
void Sha512Hash(const unsigned char* input, unsigned int size, unsigned char* output, int mode)
{
    Sha512Context ctx;
    Sha512ClearContext(&ctx);
    Sha512InitContext(&ctx, mode);
    Sha512Update(&ctx, input, size);
    Sha512Finish(&ctx, output);
    Sha512WipeContextIfPresent(&ctx);
}

namespace Lua_Callbacks
{

void CleanupOptions()
{
    _SetGameRules(gCurRules);
    Lua_Callbacks::gTeamSetReinforcementMult = 0;
    gHuntShowScoreLimit = 0;
}
}

// GLOBAL: SWBF2 0x01ec636c
// SYNTHETIC: SWBF2 0x004041f0
// `dynamic initializer for 'gGlobalNumBots''
int gGlobalNumBots = netNumBots;
