#pragma once

#include "LuaCallbacks.h"

namespace Lua_Callbacks
{

int ScriptCB_GetLoginList(lua_State* L);
int ScriptCB_IsCurProfileDirty(lua_State* L);
int ScriptCB_IsProfileDirty(lua_State* L);
int ScriptCB_SetProfileNotDirty(lua_State* L);
int ScriptCB_LoadSavePopupResult(lua_State* L);
int ScriptCB_DoInitialMemcardCheck(lua_State* L);
int ScriptCB_StartInitialMemcardCheck(lua_State* L);
int ScriptCB_StartPreOp(lua_State* L);
int ScriptCB_IsPlayerLoggedIn(lua_State* L);
int ScriptCB_GetCurrentProfileName(lua_State* L);
int ScriptCB_GetProfileName(lua_State* L);
int ScriptCB_GetCurrentProfileNetName(lua_State* L);
int ScriptCB_GetCurrentPCTitleVersion(lua_State* L);
int ScriptCB_AddProfile(lua_State* L);
int ScriptCB_SwapActiveProfileSlots(lua_State* L);
int ScriptCB_StartLoadProfile(lua_State* L);
int ScriptCB_MakeFakeProfiles(lua_State* L);
int ScriptCB_SetProfileAsDefault(lua_State* L);
int ScriptCB_RestoreDefaultProfile(lua_State* L);
int ScriptCB_MarkCurrentProfile(lua_State* L);
int ScriptCB_ReloadMarkedProfile(lua_State* L);
int ScriptCB_StartDeleteProfile(lua_State* L);
int ScriptCB_StartSaveProfile(lua_State* L);
int ScriptCB_StartLoadMetagame(lua_State* L);
int ScriptCB_StartSaveMetagame(lua_State* L);
int ScriptCB_StartDeleteMetagame(lua_State* L);
int ScriptCB_IsCurrentMetagame(lua_State* L);
int ScriptCB_StartLoadCampaign(lua_State* L);
int ScriptCB_StartSaveCampaign(lua_State* L);
int ScriptCB_StartDeleteCampaign(lua_State* L);
int ScriptCB_IsCurrentCampaign(lua_State* L);
int ScriptCB_TrimLoginName(lua_State* L);
int ScriptCB_GetUniqueLoginName(lua_State* L);
int ScriptCB_IsUniqueLoginName(lua_State* L);
int ScriptCB_MarkMemoryCard(lua_State* L);
int ScriptCB_CheckMemoryCardMark(lua_State* L);
int ScriptCB_IsMemoryCardUnformatted(lua_State* L);
int ScriptCB_IsMemoryCardInsertted(lua_State* L);
int ScriptCB_DoesProfileExistOnCard(lua_State* L);
int ScriptCB_InMemoryProfileMode(lua_State* L);
int ScriptCB_SetMemoryProfileMode(lua_State* L);
int ScriptCB_CheckIfOkToExitMemoryMode(lua_State* L);
int ScriptCB_GetSavedMetagameList(lua_State* L);
int ScriptCB_StartNewMetagame(lua_State* L);
int ScriptCB_DoesMetagameExistOnCard(lua_State* L);
int ScriptCB_GetSavedCampaignList(lua_State* L);
int ScriptCB_StartNewCampaign(lua_State* L);
int ScriptCB_DoesCampaignExistOnCard(lua_State* L);
int ScriptCB_Logout(lua_State* L);
int ScriptCB_SaveMetagameState(lua_State* L);
int ScriptCB_LoadMetagameState(lua_State* L);
int ScriptCB_ClearMetagameState(lua_State* L);
int ScriptCB_IsMetagameStateSaved(lua_State* L);
int ScriptCB_SaveCampaignState(lua_State* L);
int ScriptCB_LoadCampaignState(lua_State* L);
int ScriptCB_ClearCampaignState(lua_State* L);
int ScriptCB_IsCampaignStateSaved(lua_State* L);
int ScriptCB_SetInTrainingMission(lua_State* L);
int ScriptCB_GetInTrainingMission(lua_State* L);
int ScriptCB_SaveMissionSetup(lua_State* L);
int ScriptCB_LoadMissionSetup(lua_State* L);
int ScriptCB_ClearMissionSetup(lua_State* L);
int ScriptCB_IsMissionSetupSaved(lua_State* L);

int ScriptCB_HasProfileChanged(lua_State* L);

void InitLoadSave();

void LoadMetagameDoneCallback();
void LoadCampaignDoneCallback();

}
