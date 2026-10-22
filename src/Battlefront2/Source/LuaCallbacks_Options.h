#pragma once

#include "LuaCallbacks.h"

namespace GameLoop { void SetFunctionIdForButtonId(int iButton, int iFunction); }
namespace GameLoop { int GetFunctionIdForButtonId(int iButton); }
namespace GameLoop { int GetFunctionIdForAnalog(); }
int LuaButtonIndexToEnum(int iButton);
int LuaFunctionIndexToEnum(unsigned int iFunction, unsigned int iMode);
int EnumToLuaFunctionIndex(int iEnum, int iMode);

namespace Lua_Callbacks
{

int ScriptCB_SetSplitscreen(lua_State* L);
int ScriptCB_GetNumCameras(lua_State* L);
int ScriptCB_IsSplitscreen(lua_State* L);
int ScriptCB_WasSplitscreen(lua_State* L);
int ScriptCB_SetPlayerSide(lua_State* L);
int ScriptCB_SetDifficulty(lua_State* L);
int ScriptCB_GetDifficulty(lua_State* L);
int ScriptCB_GetCTFCaptureLimit(lua_State* L);
int ScriptCB_GetHuntMaxTimeLimit(lua_State* L);
int ScriptCB_GetHuntScoreLimit(lua_State* L);
int ScriptCB_ShowHuntScoreLimit(lua_State* L);
int ScriptCB_GetCTFMaxTimeLimit(lua_State* L);
int ScriptCB_GetAssaultScoreLimit(lua_State* L);
int ScriptCB_GetCONMaxTimeLimit(lua_State* L);
int ScriptCB_GetCONNumBots(lua_State* L);
int ScriptCB_GetCTFNumBots(lua_State* L);
int ScriptCB_GetASSNumBots(lua_State* L);
int ScriptCB_GetFunctionIdForAnalogId(lua_State* L);
int ScriptCB_SetFunctionIdForAnalogId(lua_State* L);
int ScriptCB_GetFunctionIdForButtonId(lua_State* L);
int ScriptCB_SetFunctionIdForButtonId(lua_State* L);
int ScriptCB_SetSpectatorMode(lua_State* L);
int ScriptCB_GetSpectatorMode(lua_State* L);
int ScriptCB_SetCanSwitchSides(lua_State* L);
int ScriptCB_SetGameRules(lua_State* L);
int ScriptCB_GetGameRules(lua_State* L);
int ScriptCB_SetAIDifficulty(lua_State* L);
int ScriptCB_IsMetaAllMapsOn(lua_State* L);
int ScriptCB_SetMetaAllMapsOn(lua_State* L);
int ScriptCB_GetYAxisFlip(lua_State* L);
int ScriptCB_SetYAxisFlip(lua_State* L);
int ScriptCB_GetMouseTurnAssist(lua_State* L);
int ScriptCB_SetMouseTurnAssist(lua_State* L);
int ScriptCB_GetControlScale(lua_State* L);
int ScriptCB_SetControlScale(lua_State* L);
int ScriptCB_ResetGameOptionsToDefault(lua_State* L);
int ScriptCB_ResetSoundToDefault(lua_State* L);
int ScriptCB_ResetControlsToDefault(lua_State* L);
int ScriptCB_GetGeneralOptions(lua_State* L);
int ScriptCB_SetGeneralOptions(lua_State* L);
int ScriptCB_IsHorizontalSplitScreen(lua_State* L);
int ScriptCB_SetHorizontalSplitScreen(lua_State* L);
int ScriptCB_SetHUDScale(lua_State* L);
int ScriptCB_GetHUDScale(lua_State* L);
int ScriptCB_NextToolTipState(lua_State* L);
int ScriptCB_PrevToolTipState(lua_State* L);
int ScriptCB_ResetToolTips(lua_State* L);
int ScriptCB_SetControlMode(lua_State* L);
int ScriptCB_GetControlMode(lua_State* L);
int ScriptCB_GetSideSelectButtonSetting(lua_State* L);
int ScriptCB_SetOnlineOpts(lua_State* L);
int ScriptCB_GetOnlineOpts(lua_State* L);
int ScriptCB_GetNetGameDefaults(lua_State* L);
int ScriptCB_SetNetGameDefaults(lua_State* L);
int ScriptCB_GetNetHeroDefaults(lua_State* L);
int ScriptCB_SetNetHeroDefaults(lua_State* L);

int ScriptCB_GetUberScoreLimit(lua_State* L);
int ScriptCB_SetUberScoreLimit(lua_State* L);
int ScriptCB_ResetOnlineOptionsToDefault(lua_State* L);
int ScriptCB_SetToolTipState(lua_State* L);

enum eGAME_RULES
{
    eGAMERULES_INSTANTACTION = 0,
    eGAMERULES_MULTIPLAYER = 1,
    eGAMERULES_CAMPAIGN = 2,
    eGAMERULES_METAGAME = 3,
    eGAMERULES_MAX = 4
};

extern eGAME_RULES gCurRules;
extern int gDMReinforcementMult[5];
extern int gCONReinforcementMult[5];
extern int gCONNumBots[4];
extern int gCTFNumBots[4];
extern int gELINumBots[4];
extern int gASSNumBots[4];
extern int gSpaceAssaultScoreLimit[5];
extern int gCTFScore[5];
extern int gHuntScoreLimit[5];
extern int gHuntMaxTimeLimit[5];
extern int gCTFMaxTimeLimit[5];
extern int gCONMaxTimeLimit[5];

extern bool gHeroesEnabled[4];
extern int gHeroRuleUnlock[4];
extern int gHeroRuleUnlockVal[4];
extern int gHeroRuleTeam[4];
extern int gHeroRulePlayer[4];
extern int gHeroRuleRespawn[4];
extern int gHeroRuleRespawnVal[4];
extern bool gRandomizePlaylist;
extern bool gIsDedicated;

void InitOptions();

}

#include <stddef.h>

struct Sha512Context
{
    unsigned __int64 total[2];
    unsigned __int64 state[8];
    unsigned char buffer[128];
    int mode;
};
 
 
 
 

int Base64Encode(unsigned char* dst, unsigned int capacity, unsigned int* length,
               const unsigned char* src, unsigned int size);
void Sha512WipeContext(Sha512Context* ctx);
void Sha512ClearContext(Sha512Context* ctx);
void Sha512WipeContextIfPresent(Sha512Context* ctx);
void Sha512InitContext(Sha512Context* ctx, int mode);
void Sha512TransformBlock(Sha512Context* ctx, const unsigned char* block);
void Sha512Update(Sha512Context* ctx, const unsigned char* input, unsigned int size);
void Sha512Finish(Sha512Context* ctx, unsigned char* output);
void Sha512Hash(const unsigned char* input, unsigned int size, unsigned char* output, int mode);
