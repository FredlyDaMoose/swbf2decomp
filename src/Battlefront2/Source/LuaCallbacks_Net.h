#pragma once

#include "LuaCallbacks.h"

extern int gSortMode;

namespace Lua_Callbacks
{

extern char pFilters[7][64];

int ScriptCB_AutoNetJoin(lua_State* L);
int ScriptCB_GetOnlineService(lua_State* L);
int ScriptCB_SetAmHost(lua_State* L);
int ScriptCB_GetAmHost(lua_State* L);
int ScriptCB_InNetGame(lua_State* L);
int ScriptCB_SetInNetGame(lua_State* L);
int ScriptCB_InMultiplayer(lua_State* L);
int ScriptCB_InNetSession(lua_State* L);
int ScriptCB_NetWasHost(lua_State* L);
int ScriptCB_NetWasDedicated(lua_State* L);
int ScriptCB_NetWasDedicatedQuit(lua_State* L);
int ScriptCB_NetWasClient(lua_State* L);
int ScriptCB_IsDedicated(lua_State* L);
int ScriptCB_SetDedicated(lua_State* L);
int ScriptCB_IsInShell(lua_State* L);
int ScriptCB_IsAutoNet(lua_State* L);
int ScriptCB_EndAutoNet(lua_State* L);
int ScriptCB_GetAutoNetMode(lua_State* L);
int ScriptCB_OpenNetShell(lua_State* L);
int ScriptCB_IsNetworkOn(lua_State* L);
int ScriptCB_CloseNetShell(lua_State* L);
int ScriptCB_IsBootInvitePending(lua_State* L);
int ScriptCB_GetAutoAssignTeams(lua_State* L);
int ScriptCB_GetGameName(lua_State* L);
int ScriptCB_SetGameName(lua_State* L);
int ScriptCB_SetConnectType(lua_State* L);
int ScriptCB_GetConnectType(lua_State* L);
int ScriptCB_EnableJournal(lua_State* L);
int ScriptCB_EnablePlayback(lua_State* L);
int ScriptCB_GetLatestError(lua_State* L);
int ScriptCB_GetError(lua_State* L);
int ScriptCB_ClearError(lua_State* L);
int ScriptCB_IsLoginDone(lua_State* L);
int ScriptCB_IsLoggedIn(lua_State* L);
int ScriptCB_StartLoginDedicatedServer(lua_State* L);
int ScriptCB_StartLogin(lua_State* L);
int ScriptCB_CancelLogin(lua_State* L);
int ScriptCB_TrackLoginErrors(lua_State* L);
int ScriptCB_IsGuest(lua_State* L);
int ScriptCB_BeginLobby(lua_State* L);
int ScriptCB_GetLobbyPlayerlist(lua_State* L);
int ScriptCB_UpdateLobby(lua_State* L);
int ScriptCB_HasServerLaunched(lua_State* L);
int ScriptCB_LaunchLobby(lua_State* L);
int ScriptCB_CancelLobby(lua_State* L);
int ScriptCB_LobbyAction(lua_State* L);
int ScriptCB_VoteKick(lua_State* L);
int ScriptCB_VoteOrNominate(lua_State* L);
int ScriptCB_BeginSessionList(lua_State* L);
int ScriptCB_ClearPrevSessionId(lua_State* L);
int ScriptCB_GetSessionListPercent(lua_State* L);
int ScriptCB_UpdateSessionList(lua_State* L);
int ScriptCB_LaunchSessionList(lua_State* L);
int ScriptCB_CancelSessionList(lua_State* L);
int ScriptCB_PauseSessionList(lua_State* L);
int ScriptCB_IsSessionReady(lua_State* L);
int ScriptCB_GetExtraSessionInfo(lua_State* L);
void ResortSessions(bool bForward);
int ScriptCB_GetFilters(lua_State* L);
int ScriptCB_SetFilters(lua_State* L);
int ScriptCB_ApplyFilters(lua_State* L);
int ScriptCB_GetSessionSortMode(lua_State* L);
int ScriptCB_SetSessionSortMode(lua_State* L);
int ScriptCB_SetAsFavorite(lua_State* L);
int ScriptCB_GetSessionList(lua_State* L);
int ScriptCB_NextHost(lua_State* L);
int ScriptCB_PreviousHost(lua_State* L);
int ScriptCB_IsSpecialJoinPending(lua_State* L);
int ScriptCB_IsCmdlineJoinPending(lua_State* L);
int ScriptCB_GetCmdlineLogin(lua_State* L);
int ScriptCB_BeginQuickmatch(lua_State* L);
int ScriptCB_BeginJoinIP(lua_State* L);
int ScriptCB_BeginJoinSpecial(lua_State* L);
int ScriptCB_UpdateQuickmatch(lua_State* L);
int ScriptCB_LaunchQuickmatch(lua_State* L);
int ScriptCB_CancelQuickmatch(lua_State* L);
int ScriptCB_IsQuickmatchDone(lua_State* L);
int ScriptCB_IsQuickmatchPassworded(lua_State* L);
int ScriptCB_CanSupportMaxPlayers(lua_State* L);
int ScriptCB_IsBadNetworkConnection(lua_State* L);
int ScriptCB_BeginJoin(lua_State* L);
int ScriptCB_UpdateJoin(lua_State* L);
int ScriptCB_LaunchJoin(lua_State* L);
int ScriptCB_CancelJoin(lua_State* L);
int ScriptCB_IsJoinDone(lua_State* L);
int ScriptCB_BeginLeave(lua_State* L);
int ScriptCB_UpdateLeave(lua_State* L);
int ScriptCB_LaunchLeave(lua_State* L);
int ScriptCB_CancelLeave(lua_State* L);
int ScriptCB_IsLeaveDone(lua_State* L);
int ScriptCB_IsNetHWPresent(lua_State* L);
int ScriptCB_IsNetCableIn(lua_State* L);
int ScriptCB_SetNoticeNoCable(lua_State* L);
int ScriptCB_GetIPAddr(lua_State* L);
int ScriptCB_SetNetLoginName(lua_State* L);
int ScriptCB_GetLobbyPlayerFlags(lua_State* L);
int ScriptCB_GetGSProfileInfo(lua_State* L);
int ScriptCB_SetGSProfileInfo(lua_State* L);
int ScriptCB_BeginFriends(lua_State* L);
int ScriptCB_CancelFriends(lua_State* L);
int ScriptCB_UpdateFriends(lua_State* L);
int ScriptCB_GetFriendStateStr(lua_State* L);
int ScriptCB_GetFriendActions(lua_State* L);
int ScriptCB_DoFriendAction(lua_State* L);
int ScriptCB_GetFriendListIcon(lua_State* L);
int ScriptCB_AllowSkipDNAS(lua_State* L);
int ScriptCB_CheckForPatch(lua_State* L);
int ScriptCB_SkipToNTGUI(lua_State* L);
int ScriptCB_GetSkipToNTGUIProfileName(lua_State* L);
int ScriptCB_ResetSkipToNTGUI(lua_State* L);
int ScriptCB_GetLobbyPlayerCount(lua_State* L);
int ScriptCB_VoiceEnable(lua_State* L);
int ScriptCB_GetVoiceEnable(lua_State* L);
int ScriptCB_SetVoiceLocalEchoEnable(lua_State* L);
int ScriptCB_SetVoiceSendPreset(lua_State* L);
int ScriptCB_GetVoiceSendPreset(lua_State* L);
int ScriptCB_SetVoiceReceivePreset(lua_State* L);
int ScriptCB_GetVoiceReceivePreset(lua_State* L);
int ScriptCB_SetVoiceSendEnable(lua_State* L);
int ScriptCB_GetVoiceSendStatus(lua_State* L);
int ScriptCB_SetVoiceReceiveEnable(lua_State* L);
int ScriptCB_GetVoiceReceiveStatus(lua_State* L);
int ScriptCB_GetVoiceRenderConnected(lua_State* L);
int ScriptCB_GetVoiceCaptureConnected(lua_State* L);
int ScriptCB_GetVoiceTVGain(lua_State* L);
int ScriptCB_CanClientLeaveStats(lua_State* L);
int ScriptCB_SearchStatsRank(lua_State* L);
int ScriptCB_GetBandwidth(lua_State* L);
int ScriptCB_SetBandwidth(lua_State* L);

int ScriptCB_SetNumBots(lua_State* L);

int ScriptCB_GetWasHost(lua_State* L);
int ScriptCB_GetLoginName(lua_State* L);
int ScriptCB_IsGamespyArcadePasswordReady(lua_State* L);
int ScriptCB_GetPlayerAwardsEnabled(lua_State* L);
int ScriptCB_SetPlayerAwardsEnabled(lua_State* L);
int ScriptCB_LastSignInError(lua_State* L);

void InitNet();

int GetLobbyPlayerCount();
int GetLobbyPlayerIndex(int iIndex);
char const* GetLobbyPlayer(int iPlayer);

int GetSelectedFriendIdx();

}
