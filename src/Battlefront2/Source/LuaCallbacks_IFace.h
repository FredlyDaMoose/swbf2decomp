#pragma once

#include <stddef.h>

#include "LuaCallbacks.h"

class RedInterfaceElement;
class RedScreenGroupElement;
class RedColorValue;
class PblVector3;

namespace StatsManager { int GetState(); }
void StatsManagerSetStoppedState();
namespace StatsManager { bool __fastcall ReadStatsGetResults(int iUnused, void* pData); }
namespace StatsManager { bool FindServicePlayerID(); }
namespace StatsManager { int GetLeaderBoardId(); }
namespace StatsManager { bool ResetPlayer(); }
namespace Lua_Callbacks { void __fastcall TableReadValWithDefault(const char* pcKey, float* pfValue); }
namespace Lua_Callbacks { void __fastcall TableReadColor(const char* pcKey, float* pfColor); }
namespace Lua_Callbacks { void __fastcall TableReadDirection(int iUnused, float* pfDirection); }
bool ToggleInvincibilityCheat();

namespace Lua_Callbacks
{
int GetLobbyPlayerCount();
char const* GetLobbyPlayer(int iPlayer);
}

struct LeaderBoardStatsT
{
    __int64 rating;
    int shotsFired;
    int shotsHit;
    int criticalshots;
    int livingStreak;
    int narrowEscapes;
    int kills;
    int deaths;
    int captures;
    int vehiclesDestroyed;
    int lastPlayed;
    int totalPlayingTime;
    int wins;
    int losses;
    int tankBuster;
    int killingSpree;
    int camper;
    int survivalist;
    int traitor;
    int publicEnemy;
    int meat;
    int deadEye;
    int pointman;
    int points;
    int hero_points;
    int game_start;
    int game_finish;
    int game_rank;
    unsigned int favoriteClassHash;
    int favoriteClassTime;
    unsigned int favoriteVehicleHash;
    int favoriteVehicleTime;
    unsigned int favoriteWeaponHash;
    int favoriteWeaponTime;
    char nemesis[32];
    char bait[32];
    int nemesisDeaths;
    int baitKills;
};
 
 
 

struct LeaderBoardPlayerStatsT
{
    char username[32];
    int level;
    unsigned int titleHash;
    LeaderBoardStatsT factionStats[4];
    LeaderBoardStatsT overallStats;
};
 
 
 

namespace Lua_Callbacks { bool GetServicePlayerID(const char* pcName, unsigned __int64* puId); }

struct CheatToggleT
{
    bool (*pfnToggle)();
    unsigned int uNameHash;
};
extern CheatToggleT gCheatToggles[6];
bool EvaluateCheatCode(const char* pcName, char* pcResult);

namespace Lua_Callbacks
{
void TableWriteColor(const char* pcName, const RedColorValue& color);
void TableWriteDirection(const char* pcName, const PblVector3& direction);

void LeaderBoard_FilloutTableData(LeaderBoardStatsT& stats);

extern RedInterfaceElement* pCurCreateObject;
extern RedScreenGroupElement* pCurCreateScreenGroup;

bool Util_AddToHierarchy(RedInterfaceElement* pElement, const char* pcName);
bool Util_CreateGroupInHierarchy(const char* pcName);

bool Util_GetRedInterfaceElement(int iNumArgs, RedInterfaceElement*& pElement,
                                 int& iArgStart, bool bExactArgs);
bool Util_GetRedScreenGroupElement(int iNumArgs, RedScreenGroupElement*& pElement,
                                   int& iArgStart, bool bExactArgs);

int ScriptCB_IFObj_SetPos(lua_State* L);
int ScriptCB_IFObj_GetPos(lua_State* L);
int ScriptCB_IFObj_SetBasicRotation(lua_State* L);
int ScriptCB_IFObj_SetRelativeRotation(lua_State* L);
int ScriptCB_IFObj_SetRotation(lua_State* L);
int ScriptCB_IFObj_SetAlpha(lua_State* L);
int ScriptCB_IFObj_GetAlpha(lua_State* L);
int ScriptCB_IFObj_SetColor(lua_State* L);
int ScriptCB_IFObj_GetColor(lua_State* L);
int ScriptCB_IFObj_SetZOrder(lua_State* L);
int ScriptCB_IFObj_GetZOrder(lua_State* L);
int ScriptCB_IFObj_BringToFront(lua_State* L);
int ScriptCB_IFObj_SendToBack(lua_State* L);
int ScriptCB_IFObj_Enable(lua_State* L);
int ScriptCB_IFObj_IsEnabled(lua_State* L);
int ScriptCB_IFObj_IsGroup(lua_State* L);
int ScriptCB_IFObj_SetVis(lua_State* L);
int ScriptCB_IFObj_SetEnabled(lua_State* L);
int ScriptCB_IFObj_GetVis(lua_State* L);
int ScriptCB_IFObj_SetWidthHeight(lua_State* L);
int ScriptCB_IFObj_GetWidthHeight(lua_State* L);
int ScriptCB_IFObj_SetPosition(lua_State* L);
int ScriptCB_IFObj_GetPosition(lua_State* L);
int ScriptCB_IFObj_MouseOver(lua_State* L);
int ScriptCB_IFObj_TestHotSpot(lua_State* L);
int ScriptCB_IFObj_CreateHotSpot(lua_State* L);
int ScriptCB_IFObj_RemoveHotSpot(lua_State* L);
int ScriptCB_GetFontHeight(lua_State* L);
int ScriptCB_IFText_SetString(lua_State* L);
int ScriptCB_IFText_SetUString(lua_State* L);
int ScriptCB_IFText_SetTextPosition(lua_State* L);
int ScriptCB_IFText_SetFont(lua_State* L);
int ScriptCB_IFText_SetJustify(lua_State* L);
int ScriptCB_IFText_SetTextBreak(lua_State* L);
int ScriptCB_IFText_SetTextScale(lua_State* L);
int ScriptCB_IFText_GetTextScaleX(lua_State* L);
int ScriptCB_IFText_GetTextScaleY(lua_State* L);
int ScriptCB_IFText_GetTextExtent(lua_State* L);
int ScriptCB_IFText_GetDisplayRect(lua_State* L);
int ScriptCB_IFText_SetIntercharacterSpacing(lua_State* L);
int ScriptCB_IFText_GetIntercharacterSpacing(lua_State* L);
int ScriptCB_IFText_SetTextBox(lua_State* L);
int ScriptCB_IFText_GetTextBox(lua_State* L);
int ScriptCB_IFText_SetTextStyle(lua_State* L);
int ScriptCB_IFText_SetLeading(lua_State* L);
int ScriptCB_IFText_GetLeading(lua_State* L);
int ScriptCB_IFFlashyText_Setup(lua_State* L);
int ScriptCB_IFFlashyText_SetTextColor(lua_State* L);
int ScriptCB_IFFlashyText_SetBackground(lua_State* L);
int ScriptCB_IFFlashyText_SetBackgroundSize(lua_State* L);
int ScriptCB_IFFlashyText_SetBackgroundRightJustify(lua_State* L);
int ScriptCB_IFFlashyText_StartFlashiness(lua_State* L);
int ScriptCB_IFImage_SetTexture(lua_State* L);
int ScriptCB_IFImage_GetRect(lua_State* L);
int ScriptCB_IFImage_SetRect(lua_State* L);
int ScriptCB_IFImage_GetTexCoords(lua_State* L);
int ScriptCB_IFImage_SetTexCoords(lua_State* L);
int ScriptCB_IFMaskImage_SetMaskTexture(lua_State* L);
int ScriptCB_IFMaskImage_SetMaskTexCoords(lua_State* L);
int ScriptCB_IFMaskImage_GetMaskTexCoords(lua_State* L);
int ScriptCB_IFModel_SetModel(lua_State* L);
int ScriptCB_IFModel_SetAnimation(lua_State* L);
int ScriptCB_IFModel_QueueAnimation(lua_State* L);
int ScriptCB_IFModel_SetAnimationBanks(lua_State* L);
int ScriptCB_IFModel_SetAnimationTime(lua_State* L);
int ScriptCB_IFModel_AttachModel(lua_State* L);
int ScriptCB_IFModel_SetScale(lua_State* L);
int ScriptCB_IFModel_GetScale(lua_State* L);
int ScriptCB_IFModel_SetRotation(lua_State* L);
int ScriptCB_IFModel_GetRotation(lua_State* L);
int ScriptCB_IFModel_SetTranslation(lua_State* L);
int ScriptCB_IFModel_GetTranslation(lua_State* L);
int ScriptCB_IFModel_SetDepth(lua_State* L);
int ScriptCB_IFModel_GetDepth(lua_State* L);
int ScriptCB_IFModel_SetOmegaY(lua_State* L);
int ScriptCB_IFModel_SetLighting(lua_State* L);
int ScriptCB_IFModel_UpdateAnimation(lua_State* L);
int ScriptCB_IFModel_SetAnimationLooping(lua_State* L);
int ScriptCB_AddIFScreen(lua_State* L);
int ScriptCB_SetIFScreenViewport(lua_State* L);
int ScriptCB_EndIFScreen(lua_State* L);
int ScriptCB_IsPAL(lua_State* L);
int ScriptCB_IsFileExist(lua_State* L);
int ScriptCB_IFObj_GetCPointer(lua_State* L);
int ScriptCB_IFObj_SetScreenPosition(lua_State* L);
int ScriptCB_IFObj_SetUseSafezone(lua_State* L);
int ScriptCB_EndIFObj(lua_State* L);
int ScriptCB_AddIFText(lua_State* L);
int ScriptCB_AddIFFlashyText(lua_State* L);
int ScriptCB_AddIFImage(lua_State* L);
int ScriptCB_AddIFMaskImage(lua_State* L);
int ScriptCB_AddIFModel(lua_State* L);
int ScriptCB_AddIFContainer(lua_State* L);
int ScriptCB_SetIFScreen(lua_State* L);
int ScriptCB_PushScreen(lua_State* L);
int ScriptCB_PopScreen(lua_State* L);
int ScriptCB_IsScreenInStack(lua_State* L);
int ScriptCB_OpenPopup(lua_State* L);
int ScriptCB_ClosePopup(lua_State* L);
int ScriptCB_IsPopupOpen(lua_State* L);
int ScriptCB_SetIgnoreInputs(lua_State* L);
int ScriptCB_OpenMovie(lua_State* L);
int ScriptCB_CloseMovie(lua_State* L);
int ScriptCB_PlayMovie(lua_State* L);
int ScriptCB_IsMoviePlaying(lua_State* L);
int ScriptCB_AreMoviePropertiesPlaying(lua_State* L);
int ScriptCB_StopMovie(lua_State* L);
int ScriptCB_SetMovieAudioBus(lua_State* L);
int ScriptCB_IsPropertyPlayingNow(lua_State* L);
int ScriptCB_PlayInGameMovie(lua_State* L);
int ScriptCB_StopInGameMovie(lua_State* L);
int ScriptCB_GetScreenInfo(lua_State* L);
int ScriptCB_GetSafeScreenInfo(lua_State* L);
int ScriptCB_IsErrorBoxOpen(lua_State* L);
int ScriptCB_OpenErrorBox(lua_State* L);
int ScriptCB_CloseErrorBox(lua_State* L);
int ScriptCB_GetErrorLevel(lua_State* L);
int ScriptCB_GetConsoleCmds(lua_State* L);
int ScriptCB_DoConsoleCmd(lua_State* L);
int ScriptCB_GetLeaderBoardPlayerData(lua_State* L);
int ScriptCB_DownloadLeaderBoardData(lua_State* L);
int ScriptCB_LeaderBoardDownloadDone(lua_State* L);
int ScriptCB_CancelLeaderBoardPlayerData(lua_State* L);
int ScriptCB_SetLeaderBoardID(lua_State* L);
int ScriptCB_ResetLeaderBoardPlayerData(lua_State* L);
int ScriptCB_EnumerateLeaderBoard(lua_State* L);
int ScriptCB_EnumerateLeaderBoardDone(lua_State* L);
int ScriptCB_GetEnumerateLeaderBoardData(lua_State* L);
int ScriptCB_IsEnableLeaderBoard(lua_State* L);
int ScriptCB_SetSunlight(lua_State* L);
int ScriptCB_ResetControl(lua_State* L);
int ScriptCB_SetBinding(lua_State* L);
int ScriptCB_GetKeyBoardCmds(lua_State* L);

unsigned int GetButtonName(int iButton);
unsigned int GetActionName(int iAction, int iMode);
int ScriptCB_GetEntityMesh(lua_State* L);
int ScriptCB_GetEntityWeaponMesh(lua_State* L);
int ScriptCB_GetEntityLabel(lua_State* L);
int ScriptCB_GetIFaceLighting(lua_State* L);
int ScriptCB_SetIFaceLighting(lua_State* L);
int ScriptCB_RemoveTexture(lua_State* L);
int ScriptCB_ShowLoadDisplay(lua_State* L);
int ScriptCB_SetInputRepeat(lua_State* L);

int ScriptCB_IFBorder_SetTexture(lua_State* L);
int ScriptCB_IFBorder_GetRect(lua_State* L);
int ScriptCB_IFBorder_SetRect(lua_State* L);
int ScriptCB_IFBorder_GetTexCoords(lua_State* L);
int ScriptCB_IFBorder_SetTexCoords(lua_State* L);
int ScriptCB_AddIFBorder(lua_State* L);
int ScriptCB_GetActionFromIdx(lua_State* L);
int ScriptCB_EnableCursor(lua_State* L);
int ScriptCB_MrMrsEval(lua_State* L);

void InitIFace();

}
