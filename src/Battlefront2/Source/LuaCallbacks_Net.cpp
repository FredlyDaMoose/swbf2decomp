#include "LuaCallbacks_Net.h"
#include "LuaHelper.h"
#include "LuaCallbacks_IFace.h"

#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <string>
#include <unordered_map>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "CommandLine.h"
#include "FLGuiManager.h"
#include "GameState.h"
#include "GameLoop.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatTransport.h"
#include "GameVoiceChatNode.h"
#include "NetComm.h"
#include "NetGame.h"
#include "NetSetup.h"
#include "PauseMenu.h"
#include "PlayerProfile.h"
#include "Team.h"
#include "VoiceChatNode.h"
#include "LuaCallbacks_Options.h"
#include "SoundFL/source/win/sndengine.h"
#include "NetMatch.h"
#include "Steam/gfGalaxy.h"
#include "BootDisplay.h"

void JRR_to_Errorbox(JoinRejectReason eReason);
extern int hostVoteThreshold;
namespace FriendBuddyList { bool __fastcall IsFriendBuddy(int ignored, int iPlayer); }

// FUNCTION: SWBF2 0x00429300
RedColor Team::GetColor(int iTeam) const
{
    return mColor[iTeam];
}

// GLOBAL: SWBF2 0x01e57728
namespace Lua_Callbacks { unsigned int HighestError; }

namespace Lua_Callbacks
{
extern unsigned short sTempReturnString16[256];
}

// FUNCTION: SWBF2 0x00437550
bool __fastcall LuaHelper::IsString(int iArg)
{
    return lua_isstring(LuaHelper::mState, iArg) != 0;
}

// FUNCTION: SWBF2 0x0052e3c0
unsigned int __fastcall HashGalaxyID(const unsigned char* pID)
{
    unsigned int hash = 0x811c9dc5;
    for (int i = 0; i < 8; ++i)
    {
        hash = (hash ^ pID[i]) * 0x01000193;
    }
    return hash;
}

// FUNCTION: SWBF2 0x0053dc00
unsigned char* GameVoiceChat::Node::GetSendMask(unsigned int* pWord, unsigned char* pFirst,
                                               unsigned char* pSecond) const
{
    *pWord = (unsigned int)mSendMask.mMask;
    *pFirst = mSendMask.mEnable;
    *pSecond = mSendMask.mMaskExisting;
    return pSecond;
}

// FUNCTION: SWBF2 0x0053dc40
unsigned char* GameVoiceChat::Node::GetReceiveMask(unsigned int* pWord, unsigned char* pFirst,
                                                  unsigned char* pSecond) const
{
    *pWord = (unsigned int)mReceiveMask.mMask;
    *pFirst = mReceiveMask.mEnable;
    *pSecond = mReceiveMask.mMaskExisting;
    return pSecond;
}

extern int hostPlayer;
extern int netCurMaxPlayers;

// GLOBAL: SWBF2 0x01e5772c
int lastSessionCount;

// GLOBAL: SWBF2 0x01e57938
int lastChangeCount;

// GLOBAL: SWBF2 0x01e5793c
int patchCheckStatus;

// GLOBAL: SWBF2 0x01e57723
bool gSkipToNTGUI;

// FUNCTION: SWBF2 0x00595f40
static unsigned long PatchCheckMain(void* pArg)
{
    patchCheckStatus = 3;
    return 0;
}

// FUNCTION: SWBF2 0x00595f50
static void InitPatchCheck()
{
    if (patchCheckStatus == 0)
    {
        DWORD dwThreadId;
        CreateThread(0, 0x4000, (LPTHREAD_START_ROUTINE)PatchCheckMain, 0, 0, &dwThreadId);
    }
}

// FUNCTION: SWBF2 0x00595f80
static int UpdatePatchCheck()
{
    return patchCheckStatus;
}

// GLOBAL: SWBF2 0x01e57934

bool gWasHost;

// GLOBAL: SWBF2 0x01e57935
bool gVoiceLocalEchoEnabled;

// GLOBAL: SWBF2 0x01faa698

int gFriendListIcon;

namespace GameVoiceChat { void* GetNode(); }
namespace GameVoiceChat { void* FindNodeByDeviceID(); }
namespace GameVoiceChat { unsigned int GetNumNodes(); }

// FUNCTION: SWBF2 0x00596020
static void VoiceLocalEchoEnable(const bool bEnable)
{
    if (!GameVoiceChat::IsOpen())
    {
        return;
    }

    GameVoiceChat::Transport* pTransport = GameVoiceChat::GetTransport();
    const unsigned int uNumNodes = GameVoiceChat::GetNumNodes();

    if (!NetGame::IsEnabled() && bEnable)
    {
        for (unsigned int i = 0; i < uNumNodes; ++i)
        {
            void* pNode = GameVoiceChat::GetNode();
            if (pNode && ((VoiceChat::Node*)pNode)->IsOpen())
            {
                ((GameVoiceChat::Node*)pNode)->SetID(i);
            }
        }
    }

    for (unsigned int i = 0; i < uNumNodes; ++i)
    {
        const unsigned char ucID = ((VoiceChat::Node*)GameVoiceChat::GetNode())->GetID();
        if (ucID != 0xFF)
        {
            pTransport->SetSendEnable(ucID, ucID, bEnable, false);
            pTransport->SetReceiveEnable(ucID, ucID, bEnable, false);
        }
    }

    if (!NetGame::IsEnabled() && !bEnable)
    {
        for (unsigned int i = 0; i < uNumNodes; ++i)
        {
            void* pNode = GameVoiceChat::GetNode();
            if (pNode && ((VoiceChat::Node*)pNode)->IsOpen())
            {
                ((GameVoiceChat::Node*)pNode)->SetID(0xFF);
            }
        }
    }

    gVoiceLocalEchoEnabled = bEnable;
}

extern char gLoginName[];

extern int gActiveViewport;
extern int netNumBots;
extern int netRequestedCameraCount;

static const unsigned int kMissionState = 0x8FF60339u;

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00591b70
static GuiManager::eErrorSeverity GetHighestError()
{
    const char* pcError;
    int iSeverity = 0;
    NetSetup::GetLoginError(pcError, iSeverity);

    const char* pcGameError;
    int iGameSeverity = 0;
    NetSetup::GetGameError(pcGameError, iGameSeverity);

    if (iGameSeverity > iSeverity)
    {
        iSeverity = iGameSeverity;
        pcError = pcGameError;
        NetSetup::SetErrorMode(false);
    }

    if (iSeverity <= 0)
    {
        return GuiManager::ERROR_NONE;
    }

    const unsigned short* pusText = RedLocalizeDB::Find(PblHash(pcError), false);
    if (pusText)
    {
        PblUnicode::ustrncpy(sTempReturnString16, pusText, 255);
        sTempReturnString16[255] = 0;
    }
    else
    {
        sTempReturnString16[0] = 0;
    }
    return (GuiManager::eErrorSeverity)iSeverity;
}

// FUNCTION: SWBF2 0x00591c20
int ScriptCB_GetLatestError(lua_State* L)
{
    int iSeverity = 0;
    const unsigned short* pusText = (const unsigned short*)L"";
    const int iHighest = GetHighestError();
    if (iHighest > (int)Lua_Callbacks::HighestError)
    {
        Lua_Callbacks::HighestError = iHighest;
        iSeverity = iHighest;
        pusText = sTempReturnString16;
    }
    lua_pushnumber(LuaHelper::mState, (float)iSeverity);
    LuaHelper::PushString(pusText);
    return 2;
}

// FUNCTION: SWBF2 0x00591c70
int ScriptCB_GetError(lua_State* L)
{
    int iSeverity = GetHighestError();
    if (iSeverity < (int)Lua_Callbacks::HighestError)
    {
        iSeverity = Lua_Callbacks::HighestError;
    }

    const char* pcGameError;
    int iGameSeverity;
    NetSetup::GetGameError(pcGameError, iGameSeverity);
    if (pcGameError && (strcmp(pcGameError, "ifs.mp.joinerrors.full") == 0 ||
                        strcmp(pcGameError, "ifs.mp.joinerrors.password") == 0))
    {
        ScriptCB_ClearError(0);
        iSeverity = 0;
    }

    lua_pushnumber(LuaHelper::mState, (float)iSeverity);
    LuaHelper::PushString(sTempReturnString16);
    return 2;
}

// FUNCTION: SWBF2 0x00591d50
int ScriptCB_ClearError(lua_State* L)
{
    if ((int)Lua_Callbacks::HighestError >= GuiManager::ERROR_LOGOFF)
    {
        NetSetup::CancelLogin(0, false);
    }
    Lua_Callbacks::HighestError = GuiManager::ERROR_NONE;
    return 0;
}

// FUNCTION: SWBF2 0x00591d70
int ScriptCB_IsLoginDone(lua_State* L)
{
    float fResult;
    if (NetSetup::IsLoginDone(0))
    {
        fResult = NetSetup::IsLoginSuccessful(0, false) ? 1.0f : -1.0f;
    }
    else
    {
        fResult = 0.0f;
    }
    lua_pushnumber(LuaHelper::mState, fResult);
    return 1;
}

// FUNCTION: SWBF2 0x00591dc0
int ScriptCB_IsLoggedIn(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetSetup::IsLoginSuccessful(0, false));
    return 1;
}

// FUNCTION: SWBF2 0x00591df0
int ScriptCB_GetLoginName(lua_State* L)
{
    if (NetSetup::IsLoginSuccessful(0, false))
    {
        lua_pushstring(LuaHelper::mState, gLoginName);
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00591e30
int ScriptCB_StartLoginDedicatedServer(lua_State* L)
{
    const int iViewport = gActiveViewport;
    NetSetup::StartLogin(iViewport, false, 0, 0, 0, false);
    PlayerProfile::ResetAllProfiles();

    unsigned short ausName[30];
    PblUnicode::ustrncnv(ausName, "AUTONET", 30);
    PlayerProfile::GetProfile(0)->SetTrueProfileName(ausName);
    NetSetup::SetLoginName(iViewport, "AUTONET");

    PlayerProfile::SetCurProfile(0);
    PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetCurProfile();
    PlayerProfile::SetCurProfile(0);
    return 0;
}

// FUNCTION: SWBF2 0x00592640
int ScriptCB_UpdateLobby(lua_State* L)
{
    const bool bForce = lua_toboolean(LuaHelper::mState, 1) != 0;
    GuiManager* pGui = GuiManager::Instance();

    if (!NetGame::InShell())
    {
        if (netEnabledNext)
        {
            ScriptCB_GetLobbyPlayerlist(0);
            if (!_stricmp(pGui->mCurrentScope, "ifs_mp_lobby"))
            {
                LuaHelper::CallTableProcWThis(pGui->mCurrentScope, "RepaintListbox");
            }
        }
        return 0;
    }

    bool bRefresh = false;
    int iChangeCount = 0;
    if (NetComm::IsHost())
    {
        if (!hostGame)
        {
            return 0;
        }
        hostGame->Update();
        if (NetGame::IsDedicated())
        {
            if (hostGame->IsLaunchReady())
            {
                delete hostGame;
                hostGame = 0;
                pGui->ChangeToScreen("", false, true);
                GameState::SetState(kMissionState);
            }
        }
        else
        {
            iChangeCount = hostGame->GetChangeCount();
            bRefresh = true;
        }
    }
    else
    {
        if (!joinGame)
        {
            return 0;
        }
        joinGame->Update();
        if (joinGame->IsLaunchReady())
        {
            if (pGui->mCurrentPopup[0])
            {
                pGui->ClosePopup(true);
                RedInterfaceElement* pLaunching =
                    RedInterfaceElement::Find("ifs_mp_lobby_quick.launching");
                if (pLaunching)
                {
                    pLaunching->Enable(true);
                    pLaunching->Visible(true);
                }
            }
            else
            {
                delete joinGame;
                joinGame = 0;
                pGui->ChangeToScreen("", false, true);
                GameState::SetState(kMissionState);
            }
        }
        else
        {
            iChangeCount = joinGame->GetChangeCount();
            bRefresh = true;
        }
    }

    if (bRefresh && (bForce || iChangeCount != lastChangeCount))
    {
        lastChangeCount = iChangeCount;
        ScriptCB_GetLobbyPlayerlist(0);
        if (!_stricmp(pGui->mCurrentScope, "ifs_mp_lobby"))
        {
            LuaHelper::CallTableProcWThis(pGui->mCurrentScope, "RepaintListbox");
        }
    }

    if (NetSetup::IsLoginError() || NetSetup::IsGameError())
    {
        pGui->PopScreen(0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00592830
int ScriptCB_HasServerLaunched(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, joinGame->IsHostPlaying());
    return 1;
}

// FUNCTION: SWBF2 0x00592860
int ScriptCB_LaunchLobby(lua_State* L)
{
    if (NetQOS::IsDone())
    {
        if (!NetComm::IsHost())
        {
            if (joinGame)
            {
                joinGame->Launch();
            }
        }
        else if (hostGame && hostGame->IsLaunchReady())
        {
            delete hostGame;
            hostGame = 0;
            GuiManager::Instance()->ChangeToScreen("", false, true);
            GameState::SetState(kMissionState);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005928f0
int ScriptCB_CancelLobby(lua_State* L)
{
    if (!NetGame::InShell())
    {
        return 0;
    }

    if (NetComm::IsHost())
    {
        if (hostGame)
        {
            hostGame->Cancel();
            delete hostGame;
            hostGame = 0;
        }
    }
    else if (joinGame)
    {
        joinGame->Cancel();
        delete joinGame;
        joinGame = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00592960
int ScriptCB_LobbyAction(lua_State* L)
{
    const int iTop = lua_gettop(LuaHelper::mState);
    if (iTop == 3)
    {
        if (!lua_isnumber(LuaHelper::mState, 1) || !lua_isstring(LuaHelper::mState, 2) ||
            !lua_isstring(LuaHelper::mState, iTop))
        {
            return 0;
        }

        const int iIndex = (int)luaL_checknumber(LuaHelper::mState, 1);
        const int iCount = GetLobbyPlayerCount();
        luaL_checkstring(LuaHelper::mState, 2);
        const unsigned int uiAction = PblHash(luaL_checkstring(LuaHelper::mState, iTop));

        if (uiAction == 0x789A24E4u)
        {
            if (hostGame)
            {
                hostGame->BootPlayer(iIndex);
            }
            else if (postLoadHostGame && iIndex >= 0 && iIndex < iCount && iIndex != 64)
            {
                NetGame::ForceBootPlayer(iIndex);
            }
        }
        else if (uiAction == 0xA2FD7D0Cu)
        {
            if (iIndex >= 0 && iIndex < iCount)
            {
                NetGame::SetStartTeam(iIndex, 3 - NetGame::GetStartTeam(iIndex));
            }
            ScriptCB_GetLobbyPlayerlist(0);
        }
        else if (uiAction == 0xD6069F54u)
        {
            if (iIndex >= 0 && iIndex < iCount)
            {
                GameVoiceChat::Transport* pTransport = GameVoiceChat::GetTransport();
                const unsigned char ucLocal = (unsigned char)NetGame::GetLocalIndex(PauseMenu::GetPausingViewport());
                const bool bMuted = pTransport->GetReceiveEnable(ucLocal, (unsigned char)iIndex);
                pTransport->SetReceiveEnable(ucLocal, (unsigned char)iIndex, !bMuted, false);
            }
        }
        return 0;
    }

    if (iTop != 4 && iTop != 5)
    {
        return 0;
    }
    if (!lua_isnumber(LuaHelper::mState, 1) || !lua_isstring(LuaHelper::mState, 2) ||
        !lua_isnumber(LuaHelper::mState, 3) || !lua_isstring(LuaHelper::mState, 4))
    {
        return 0;
    }

    const int iIndex = (int)luaL_checknumber(LuaHelper::mState, 1);
    const int iCount = GetLobbyPlayerCount();
    const float fReason = (float)luaL_checknumber(LuaHelper::mState, 3);
    const char* pcAction = luaL_checkstring(LuaHelper::mState, 4);
    int iJoystick = PauseMenu::GetPausingJoystick();
    if (iTop == 5)
    {
        iJoystick = LuaHelper::GetInt(iTop);
    }

    if (strcmp(pcAction, "boot") != 0)
    {
        return 0;
    }

    if (hostGame)
    {
        hostGame->BootPlayer(iIndex);
    }
    else if (!NetGame::InShell())
    {
        if (NetGame::GetBootPlayer() == -1)
        {
            if (iIndex >= 0 && iIndex < iCount && iIndex != 64)
            {
                NetGame::NominateBootPlayer(iIndex, (int)fReason, iJoystick);
            }
        }
        else if (!NetGame::IsBootVoted(iJoystick))
        {
            NetGame::ConfirmBootPlayer(iJoystick);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00592c60
int ScriptCB_VoteKick(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    const char* pcName = luaL_checkstring(LuaHelper::mState, 1);
    for (int i = 0; i < GetLobbyPlayerCount(); ++i)
    {
        const int iPlayer = GetLobbyPlayerIndex(i);
        if (strcmp(pcName, GetLobbyPlayer(iPlayer)) == 0)
        {
            if (NetGame::GetBootPlayer() < 0)
            {
                NetGame::NominateBootPlayer(iPlayer, 0, 0);
            }
            else if (!NetGame::IsBootVoted(0))
            {
                NetGame::ConfirmBootPlayer(0);
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00592d40
int ScriptCB_VoteOrNominate(lua_State* L)
{
    return 0;
}

// FUNCTION: SWBF2 0x00592d50
int ScriptCB_BeginSessionList(lua_State* L)
{
    lastSessionCount = 0;
    findGame = NetSetup::FindGame(FT_NORMAL, 0, 0);
    lua_pushboolean(LuaHelper::mState, 0);
    return 1;
}

// FUNCTION: SWBF2 0x00592d80
int ScriptCB_ClearPrevSessionId(lua_State* L)
{
    netPrevSessionId.SetNULL();
    return 0;
}

// FUNCTION: SWBF2 0x00592d90
int ScriptCB_GetSessionListPercent(lua_State* L)
{
    const int iPercent = findGame ? findGame->GetPercentDone() : 0;
    lua_pushnumber(LuaHelper::mState, (float)iPercent);
    return 1;
}

// FUNCTION: SWBF2 0x00592dd0
int ScriptCB_UpdateSessionList(lua_State* L)
{
    if (!findGame)
    {
        lua_pushnumber(LuaHelper::mState, 0.0f);
        return 1;
    }

    lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_layout");
    lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
    int iSelected;
    LuaHelper::TableReadVal("SelectedIdx", iSelected, true);
    lua_settop(LuaHelper::mState, -2);

    if (iSelected > 0)
    {
        lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_contents");
        lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
        int iSession = iSelected;
        if (iSelected <= luaL_getn(LuaHelper::mState, -1))
        {
            lua_rawgeti(LuaHelper::mState, -1, iSelected);
            LuaHelper::TableReadVal("sessionIdx", iSession, false);
            lua_settop(LuaHelper::mState, -2);
        }
        lua_settop(LuaHelper::mState, -2);
        findGame->SetSelectedSession(iSession - 1);
    }
    else
    {
        findGame->SetSelectedSession(0);
    }

    const int iBefore = findGame->GetGameCount();
    const bool bChanged = findGame->Update();
    const int iCount = findGame->GetGameCount();
    if (bChanged || iBefore != iCount)
    {
        lastSessionCount = findGame->GetGameCount();
        ScriptCB_GetSessionList(0);
        LuaHelper::CallTableProcWThis("ifs_mp_sessionlist", "RepaintListbox");
    }

    if (NetSetup::InNetGame())
    {
        if (findGame->IsReady())
        {
            LuaHelper::CallTableProcWThis(GuiManager::Instance()->mCurrentScope, "fnFindReady");
        }
        else if (findGame->IsFinished())
        {
            NetSessionId id;
            id.SetNULL();
            NetSetup::GetPrevSessionId(&id);
            findGame->Cancel();
            delete findGame;
            NetSetup::SetInNetGame(true);
            findGame = NetSetup::FindGame(FT_PREVIOUS, 0, &id);
        }
    }

    lua_pushnumber(LuaHelper::mState, (float)(iCount > 0));
    return 1;
}

// FUNCTION: SWBF2 0x00593020
int ScriptCB_LaunchSessionList(lua_State* L)
{
    lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_layout");
    lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
    int iSelected;
    LuaHelper::TableReadVal("SelectedIdx", iSelected, false);
    lua_settop(LuaHelper::mState, -2);

    lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_contents");
    lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
    int iSession = iSelected;
    if (iSelected <= luaL_getn(LuaHelper::mState, -1))
    {
        lua_rawgeti(LuaHelper::mState, -1, iSelected);
        LuaHelper::TableReadVal("sessionIdx", iSession, false);
        lua_settop(LuaHelper::mState, -2);
    }
    lua_settop(LuaHelper::mState, -2);

    findGame->SelectFirst();
    if (iSession > 1)
    {
        findGame->Select(iSession - 1);
    }
    joinGame = NetSetup::JoinGame(findGame);
    return 0;
}

// FUNCTION: SWBF2 0x00593120
int ScriptCB_CancelSessionList(lua_State* L)
{
    lua_newtable(LuaHelper::mState);
    lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_contents");
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(true);

    if (joinGame)
    {
        joinGame->Cancel();
        do
        {
            joinGame->Update();
        } while (!joinGame->IsCancelled());
        delete joinGame;
        joinGame = 0;
    }

    if (findGame)
    {
        findGame->Cancel();
        delete findGame;
    }
    findGame = 0;
    return 0;
}

// FUNCTION: SWBF2 0x005931e0
int ScriptCB_PauseSessionList(lua_State* L)
{
    if (findGame)
    {
        findGame->Pause();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00593200
int ScriptCB_IsSessionReady(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, findGame ? findGame->IsReady() : false);
    return 1;
}

// FUNCTION: SWBF2 0x00591450
int ScriptCB_AutoNetJoin(lua_State* L)
{
    if (joinGame)
    {
        joinGame->Update();
        lua_pushboolean(LuaHelper::mState, 1);
        return 1;
    }

    if (findGame && findGame->IsReady())
    {
        findGame->Update();
        if (findGame->GetGameCount() > 0 &&
            !_stricmp(findGame->GetName(), NetSetup::GetAutoNetName()))
        {
            if (findGame->IsReady())
            {
                if (netRequestedCameraCount > 0)
                {
                    GameLoop::mNumCameras = netRequestedCameraCount;
                }
                joinGame = NetSetup::JoinGame(findGame);
            }
        }
        else
        {
            findGame->SelectFirst();
            for (int i = 0; i < findGame->GetGameCount(); ++i)
            {
                findGame->Select(1);
                if (!_stricmp(findGame->GetName(), NetSetup::GetAutoNetName()))
                {
                    break;
                }
            }
        }
    }
    else
    {
        findGame = NetSetup::FindGame(FT_NORMAL, 0, 0);
        findGame->SelectFirst();
    }
    lua_pushboolean(LuaHelper::mState, 0);
    return 1;
}

// FUNCTION: SWBF2 0x005915a0
int ScriptCB_GetOnlineService(lua_State* L)
{
    lua_pushstring(LuaHelper::mState, NetSetup::GetOnlineService());
    return 1;
}

// FUNCTION: SWBF2 0x005917b0
int ScriptCB_IsInShell(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetGame::InShell());
    return 1;
}

// FUNCTION: SWBF2 0x00591600
int ScriptCB_GetAmHost(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetComm::IsHost());
    return 1;
}

// FUNCTION: SWBF2 0x00591760
int ScriptCB_IsDedicated(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetGame::IsDedicated());
    return 1;
}

// FUNCTION: SWBF2 0x00591f10
int ScriptCB_IsGuest(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, 0);
    return 1;
}

// FUNCTION: SWBF2 0x00595e70
int ScriptCB_GetFriendListIcon(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)gFriendListIcon);
    return 1;
}

// FUNCTION: SWBF2 0x005917d0
int ScriptCB_IsAutoNet(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, autoNetEnabled);
    return 1;
}

// FUNCTION: SWBF2 0x00591820
int ScriptCB_OpenNetShell(lua_State* L)
{
    int iStage = 1;
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        iStage = (int)luaL_checknumber(LuaHelper::mState, 1);
    }
    NetGame::BeginShell(iStage);
    return 0;
}

// FUNCTION: SWBF2 0x00591950
int ScriptCB_SetNumBots(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        netNumBots = (int)luaL_checknumber(LuaHelper::mState, 1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594d90
int ScriptCB_GetCmdlineLogin(lua_State* L)
{
    lua_pushstring(LuaHelper::mState, gGamespyAutoName);
    lua_pushstring(LuaHelper::mState, gGamespyAutoEmail);
    lua_pushstring(LuaHelper::mState, gGamespyAutoPPass);
    return 3;
}

// FUNCTION: SWBF2 0x00595ec0
int ScriptCB_CheckForPatch(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1)
    {
        return 0;
    }

    int iResult = 0;
    switch ((int)luaL_checknumber(LuaHelper::mState, 1))
    {
    case 1:
        InitPatchCheck();
        break;
    case 2:
        iResult = UpdatePatchCheck();
        break;
    }
    lua_pushnumber(LuaHelper::mState, (float)iResult);
    return 1;
}

// FUNCTION: SWBF2 0x00595f90
int ScriptCB_SkipToNTGUI(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, gSkipToNTGUI);
    return 1;
}

// FUNCTION: SWBF2 0x00595fe0
int ScriptCB_ResetSkipToNTGUI(lua_State* L)
{
    gSkipToNTGUI = false;
    return 0;
}

// FUNCTION: SWBF2 0x00591660
int ScriptCB_SetInNetGame(lua_State* L)
{
    NetSetup::SetInNetGame(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 1;
}

// FUNCTION: SWBF2 0x00591690
int ScriptCB_InMultiplayer(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetSetup::InMultiplayer());
    return 1;
}

// FUNCTION: SWBF2 0x005918b0
int ScriptCB_CloseNetShell(lua_State* L)
{
    int iStage = 1;
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        iStage = (int)luaL_checknumber(LuaHelper::mState, 1);
    }
    NetGame::EndShell(iStage);
    return 0;
}

// FUNCTION: SWBF2 0x00591a10
int ScriptCB_SetConnectType(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    if (strcmp(luaL_checkstring(LuaHelper::mState, 1), "lan") == 0)
    {
        NetComm::SetConnectType(CT_LAN);
        NetSetup::EnableOnlineService(false);
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
        const unsigned short* pusName = pProfile ? pProfile->mName : 0;
        if (pusName && pusName[0])
        {
            char acName[32];
            PblUnicode::ustrncnv(acName, pusName, 32);
            NetSetup::SetLoginName(0, acName);
        }
    }
    else
    {
        NetComm::SetConnectType(CT_WAN);
        NetSetup::EnableOnlineService(true);
        if (gLoginName[0])
        {
            NetSetup::SetLoginName(0, gLoginName);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00591ee0
int ScriptCB_CancelLogin(lua_State* L)
{
    NetSetup::CancelLogin(0, false);
    return 0;
}

// FUNCTION: SWBF2 0x00594cf0
int ScriptCB_IsSpecialJoinPending(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetSetup::GetSpecialJoinValid());
    return 1;
}

// FUNCTION: SWBF2 0x00594d10
int ScriptCB_IsCmdlineJoinPending(lua_State* L)
{
    if (gGamespyAutoConnectAddr[0])
    {
        lua_pushstring(LuaHelper::mState, gGamespyAutoConnectAddr);
    }
    else
    {
        lua_pushboolean(LuaHelper::mState, false);
    }
    return 1;
}

// FUNCTION: SWBF2 0x005915c0
int ScriptCB_SetAmHost(lua_State* L)
{
    if (lua_toboolean(LuaHelper::mState, 1))
    {
        NetSetup::SetAsHost();
        gWasHost = true;
    }
    else
    {
        NetSetup::SetAsClient();
        gWasHost = false;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00591640
int ScriptCB_InNetGame(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetSetup::InNetGame());
    return 1;
}

// FUNCTION: SWBF2 0x00591780
int ScriptCB_SetDedicated(lua_State* L)
{
    NetSetup::SetAsDedicated(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x00591880
int ScriptCB_IsNetworkOn(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, NetGame::IsEnabled());
    return 1;
}

// FUNCTION: SWBF2 0x00591b10
int ScriptCB_GetConnectType(lua_State* L)
{
    switch (NetComm::GetConnectType())
    {
    case CT_LINK:
    case CT_LAN:
        lua_pushstring(LuaHelper::mState, "LAN");
        return 1;
    case CT_WAN:
        lua_pushstring(LuaHelper::mState, NetSetup::GetOnlineService());
        return 1;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00591eb0
int ScriptCB_StartLogin(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    NetSetup::StartLogin(gActiveViewport, false, 0, 0, 0, false);
    return 0;
}

// FUNCTION: SWBF2 0x00595840
int ScriptCB_GetGSProfileInfo(lua_State* L)
{
    char acSaveInfo[4] = "000";
    const char* pcNickname = "";
    const char* pcEmail = "";
    const char* pcPassword = "";
    int iAutoLogin = 0;

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        PlayerProfile::tOnlineOpt& online = pProfile->mOnlineOptions;
        pcNickname = NetMatch::GetLoginOverrideName(0);
        if (!pcNickname)
        {
            pcNickname = online.mGamespyNickname;
        }
        pcPassword = online.mGamespyPassword;
        pcEmail = online.mGamespyEmail;
        if (online.mu8_SaveInfo & 0x10)
        {
            if (online.mu8_SaveInfo & 1)
            {
                acSaveInfo[0] = '1';
            }
            if (online.mu8_SaveInfo & 2)
            {
                acSaveInfo[1] = '1';
            }
            if (online.mu8_SaveInfo & 4)
            {
                acSaveInfo[2] = '1';
            }
        }
        iAutoLogin = online.mu8GSID_Autologin;
    }

    lua_pushstring(LuaHelper::mState, pcNickname);
    lua_pushstring(LuaHelper::mState, pcEmail);
    lua_pushstring(LuaHelper::mState, pcPassword);
    lua_pushstring(LuaHelper::mState, acSaveInfo);
    lua_pushnumber(LuaHelper::mState, (float)iAutoLogin);
    return 5;
}

// FUNCTION: SWBF2 0x00596280
int ScriptCB_GetVoiceEnable(lua_State* L)
{
    int iJoystick;
    if (lua_gettop(LuaHelper::mState) >= 1)
    {
        iJoystick = (int)luaL_checknumber(LuaHelper::mState, 1);
    }
    else
    {
        iJoystick = PauseMenu::GetPausingJoystick();
    }
    if (GameState::GetState() != 0x11e1fc01)
    {
        iJoystick = NetGame::GetLocalIndex(PauseMenu::GetPausingViewport());
    }

    bool bEnabled = false;
    if (iJoystick >= 0)
    {
        VoiceChat::Node* pNode = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        bEnabled = pNode && pNode->GetID() != 0xFF;
    }
    lua_pushboolean(LuaHelper::mState, bEnabled);
    return 1;
}

// FUNCTION: SWBF2 0x00596310
int ScriptCB_SetVoiceLocalEchoEnable(lua_State* L)
{
    VoiceLocalEchoEnable((int)luaL_checknumber(LuaHelper::mState, 1) > 0);
    return 0;
}

// FUNCTION: SWBF2 0x00596880
int ScriptCB_GetVoiceCaptureConnected(lua_State* L)
{
    if (!GameVoiceChat::IsOpen())
    {
        return 0;
    }
    VoiceChat::Node* pNode = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
    if (!pNode || pNode->GetID() == 0xFF)
    {
        return 0;
    }
    if (!GameVoiceChat::GetTransport()->IsConnectedSend(pNode->GetID()))
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x005968e0
int ScriptCB_GetVoiceTVGain(lua_State* L)
{
    int iGain = 0;
    if (GameVoiceChat::IsOpen())
    {
        VoiceChat::Node* pNode = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        if (pNode && pNode->GetID() != 0xFF && pNode->GetDecoder().GetNumOutputs() > 1)
        {
            iGain = (unsigned char)(int)(pNode->GetDecoder().GetGain(1) * 10.0f);
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)iGain);
    return 1;
}

// FUNCTION: SWBF2 0x00596950
int ScriptCB_CanClientLeaveStats(lua_State* L)
{
    bool bCanLeave = true;
    bool bNetworkEnabled = netInShell ? netEnabled : netEnabledNext;
    if (bNetworkEnabled && !netIsHost)
    {
        bCanLeave = NetGameP::HasHostCycled();
    }
    lua_pushboolean(LuaHelper::mState, bCanLeave);
    return 1;
}
}

namespace Lua_Callbacks
{

// GLOBAL: SWBF2 0x007e7a60
static const CallbackEntryT sNetCallbacks[135] =
{
    { "ScriptCB_AutoNetJoin", ScriptCB_AutoNetJoin },
    { "ScriptCB_GetOnlineService", ScriptCB_GetOnlineService },
    { "ScriptCB_SetNetLoginName", ScriptCB_SetNetLoginName },
    { "ScriptCB_SetAmHost", ScriptCB_SetAmHost },
    { "ScriptCB_GetAmHost", ScriptCB_GetAmHost },
    { "ScriptCB_GetWasHost", ScriptCB_GetWasHost },
    { "ScriptCB_SetGameName", ScriptCB_SetGameName },
    { "ScriptCB_SetNumBots", ScriptCB_SetNumBots },
    { "ScriptCB_GetGameName", ScriptCB_GetGameName },
    { "ScriptCB_GetLobbyPlayerlist", ScriptCB_GetLobbyPlayerlist },
    { "ScriptCB_GetLobbyPlayerFlags", ScriptCB_GetLobbyPlayerFlags },
    { "ScriptCB_InNetGame", ScriptCB_InNetGame },
    { "ScriptCB_SetInNetGame", ScriptCB_SetInNetGame },
    { "ScriptCB_InMultiplayer", ScriptCB_InMultiplayer },
    { "ScriptCB_InNetSession", ScriptCB_InNetSession },
    { "ScriptCB_NetWasHost", ScriptCB_NetWasHost },
    { "ScriptCB_NetWasDedicated", ScriptCB_NetWasDedicated },
    { "ScriptCB_NetWasDedicatedQuit", ScriptCB_NetWasDedicatedQuit },
    { "ScriptCB_NetWasClient", ScriptCB_NetWasClient },
    { "ScriptCB_SetDedicated", ScriptCB_SetDedicated },
    { "ScriptCB_IsDedicated", ScriptCB_IsDedicated },
    { "ScriptCB_IsInShell", ScriptCB_IsInShell },
    { "ScriptCB_IsAutoNet", ScriptCB_IsAutoNet },
    { "ScriptCB_EndAutoNet", ScriptCB_EndAutoNet },
    { "ScriptCB_GetAutoNetMode", ScriptCB_GetAutoNetMode },
    { "ScriptCB_OpenNetShell", ScriptCB_OpenNetShell },
    { "ScriptCB_IsNetworkOn", ScriptCB_IsNetworkOn },
    { "ScriptCB_CloseNetShell", ScriptCB_CloseNetShell },
    { "ScriptCB_IsBootInvitePending", ScriptCB_IsBootInvitePending },
    { "ScriptCB_GetAutoAssignTeams", ScriptCB_GetAutoAssignTeams },
    { "ScriptCB_SetConnectType", ScriptCB_SetConnectType },
    { "ScriptCB_GetConnectType", ScriptCB_GetConnectType },
    { "ScriptCB_EnableJournal", ScriptCB_EnableJournal },
    { "ScriptCB_EnablePlayback", ScriptCB_EnablePlayback },
    { "ScriptCB_GetLatestError", ScriptCB_GetLatestError },
    { "ScriptCB_GetError", ScriptCB_GetError },
    { "ScriptCB_ClearError", ScriptCB_ClearError },
    { "ScriptCB_GetGSProfileInfo", ScriptCB_GetGSProfileInfo },
    { "ScriptCB_SetGSProfileInfo", ScriptCB_SetGSProfileInfo },
    { "ScriptCB_IsLoginDone", ScriptCB_IsLoginDone },
    { "ScriptCB_IsLoggedIn", ScriptCB_IsLoggedIn },
    { "ScriptCB_GetLoginName", ScriptCB_GetLoginName },
    { "ScriptCB_StartLogin", ScriptCB_StartLogin },
    { "ScriptCB_StartLoginDedicatedServer", ScriptCB_StartLoginDedicatedServer },
    { "ScriptCB_CancelLogin", ScriptCB_CancelLogin },
    { "ScriptCB_TrackLoginErrors", ScriptCB_TrackLoginErrors },
    { "ScriptCB_IsGuest", ScriptCB_IsGuest },
    { "ScriptCB_BeginLobby", ScriptCB_BeginLobby },
    { "ScriptCB_UpdateLobby", ScriptCB_UpdateLobby },
    { "ScriptCB_HasServerLaunched", ScriptCB_HasServerLaunched },
    { "ScriptCB_LaunchLobby", ScriptCB_LaunchLobby },
    { "ScriptCB_CancelLobby", ScriptCB_CancelLobby },
    { "ScriptCB_LobbyAction", ScriptCB_LobbyAction },
    { "ScriptCB_VoteKick", ScriptCB_VoteKick },
    { "ScriptCB_VoteOrNominate", ScriptCB_VoteOrNominate },
    { "ScriptCB_ClearPrevSessionId", ScriptCB_ClearPrevSessionId },
    { "ScriptCB_BeginSessionList", ScriptCB_BeginSessionList },
    { "ScriptCB_UpdateSessionList", ScriptCB_UpdateSessionList },
    { "ScriptCB_LaunchSessionList", ScriptCB_LaunchSessionList },
    { "ScriptCB_CancelSessionList", ScriptCB_CancelSessionList },
    { "ScriptCB_PauseSessionList", ScriptCB_PauseSessionList },
    { "ScriptCB_GetSessionList", ScriptCB_GetSessionList },
    { "ScriptCB_IsSessionReady", ScriptCB_IsSessionReady },
    { "ScriptCB_GetSessionListPercent", ScriptCB_GetSessionListPercent },
    { "ScriptCB_GetExtraSessionInfo", ScriptCB_GetExtraSessionInfo },
    { "ScriptCB_GetFilters", ScriptCB_GetFilters },
    { "ScriptCB_SetFilters", ScriptCB_SetFilters },
    { "ScriptCB_ApplyFilters", ScriptCB_ApplyFilters },
    { "ScriptCB_GetSessionSortMode", ScriptCB_GetSessionSortMode },
    { "ScriptCB_SetSessionSortMode", ScriptCB_SetSessionSortMode },
    { "ScriptCB_SetAsFavorite", ScriptCB_SetAsFavorite },
    { "ScriptCB_NextHost", ScriptCB_NextHost },
    { "ScriptCB_PreviousHost", ScriptCB_PreviousHost },
    { "ScriptCB_IsSpecialJoinPending", ScriptCB_IsSpecialJoinPending },
    { "ScriptCB_IsCmdlineJoinPending", ScriptCB_IsCmdlineJoinPending },
    { "ScriptCB_GetCmdlineLogin", ScriptCB_GetCmdlineLogin },
    { "ScriptCB_IsGamespyArcadePasswordReady", ScriptCB_IsGamespyArcadePasswordReady },
    { "ScriptCB_BeginQuickmatch", ScriptCB_BeginQuickmatch },
    { "ScriptCB_BeginJoinIP", ScriptCB_BeginJoinIP },
    { "ScriptCB_BeginJoinSpecial", ScriptCB_BeginJoinSpecial },
    { "ScriptCB_UpdateQuickmatch", ScriptCB_UpdateQuickmatch },
    { "ScriptCB_LaunchQuickmatch", ScriptCB_LaunchQuickmatch },
    { "ScriptCB_CancelQuickmatch", ScriptCB_CancelQuickmatch },
    { "ScriptCB_IsQuickmatchDone", ScriptCB_IsQuickmatchDone },
    { "ScriptCB_IsQuickmatchPassworded", ScriptCB_IsQuickmatchPassworded },
    { "ScriptCB_IsBadNetworkConnection", ScriptCB_IsBadNetworkConnection },
    { "ScriptCB_CanSupportMaxPlayers", ScriptCB_CanSupportMaxPlayers },
    { "ScriptCB_BeginJoin", ScriptCB_BeginJoin },
    { "ScriptCB_UpdateJoin", ScriptCB_UpdateJoin },
    { "ScriptCB_LaunchJoin", ScriptCB_LaunchJoin },
    { "ScriptCB_CancelJoin", ScriptCB_CancelJoin },
    { "ScriptCB_IsJoinDone", ScriptCB_IsJoinDone },
    { "ScriptCB_BeginLeave", ScriptCB_BeginLeave },
    { "ScriptCB_UpdateLeave", ScriptCB_UpdateLeave },
    { "ScriptCB_LaunchLeave", ScriptCB_LaunchLeave },
    { "ScriptCB_CancelLeave", ScriptCB_CancelLeave },
    { "ScriptCB_IsLeaveDone", ScriptCB_IsLeaveDone },
    { "ScriptCB_IsNetHWPresent", ScriptCB_IsNetHWPresent },
    { "ScriptCB_IsNetCableIn", ScriptCB_IsNetCableIn },
    { "ScriptCB_SetNoticeNoCable", ScriptCB_SetNoticeNoCable },
    { "ScriptCB_GetIPAddr", ScriptCB_GetIPAddr },
    { "ScriptCB_AllowSkipDNAS", ScriptCB_AllowSkipDNAS },
    { "ScriptCB_CheckForPatch", ScriptCB_CheckForPatch },
    { "ScriptCB_BeginFriends", ScriptCB_BeginFriends },
    { "ScriptCB_CancelFriends", ScriptCB_CancelFriends },
    { "ScriptCB_UpdateFriends", ScriptCB_UpdateFriends },
    { "ScriptCB_GetFriendStateStr", ScriptCB_GetFriendStateStr },
    { "ScriptCB_GetFriendActions", ScriptCB_GetFriendActions },
    { "ScriptCB_DoFriendAction", ScriptCB_DoFriendAction },
    { "ScriptCB_GetFriendListIcon", ScriptCB_GetFriendListIcon },
    { "ScriptCB_SkipToNTGUI", ScriptCB_SkipToNTGUI },
    { "ScriptCB_GetSkipToNTGUIProfileName", ScriptCB_GetSkipToNTGUIProfileName },
    { "ScriptCB_ResetSkipToNTGUI", ScriptCB_ResetSkipToNTGUI },
    { "ScriptCB_GetLobbyPlayerCount", ScriptCB_GetLobbyPlayerCount },
    { "ScriptCB_VoiceEnable", ScriptCB_VoiceEnable },
    { "ScriptCB_GetVoiceEnable", ScriptCB_GetVoiceEnable },
    { "ScriptCB_SetVoiceLocalEchoEnable", ScriptCB_SetVoiceLocalEchoEnable },
    { "ScriptCB_SetVoiceSendPreset", ScriptCB_SetVoiceSendPreset },
    { "ScriptCB_GetVoiceSendPreset", ScriptCB_GetVoiceSendPreset },
    { "ScriptCB_SetVoiceReceivePreset", ScriptCB_SetVoiceReceivePreset },
    { "ScriptCB_GetVoiceReceivePreset", ScriptCB_GetVoiceReceivePreset },
    { "ScriptCB_SetVoiceSendEnable", ScriptCB_SetVoiceSendEnable },
    { "ScriptCB_GetVoiceSendStatus", ScriptCB_GetVoiceSendStatus },
    { "ScriptCB_SetVoiceReceiveEnable", ScriptCB_SetVoiceReceiveEnable },
    { "ScriptCB_GetVoiceReceiveStatus", ScriptCB_GetVoiceReceiveStatus },
    { "ScriptCB_GetVoiceRenderConnected", ScriptCB_GetVoiceRenderConnected },
    { "ScriptCB_GetVoiceCaptureConnected", ScriptCB_GetVoiceCaptureConnected },
    { "ScriptCB_GetVoiceTVGain", ScriptCB_GetVoiceTVGain },
    { "ScriptCB_CanClientLeaveStats", ScriptCB_CanClientLeaveStats },
    { "ScriptCB_SearchStatsRank", ScriptCB_SearchStatsRank },
    { "ScriptCB_GetBandwidth", ScriptCB_GetBandwidth },
    { "ScriptCB_SetBandwidth", ScriptCB_SetBandwidth },
    { "ScriptCB_GetPlayerAwardsEnabled", ScriptCB_GetPlayerAwardsEnabled },
    { "ScriptCB_SetPlayerAwardsEnabled", ScriptCB_SetPlayerAwardsEnabled },
    { "ScriptCB_LastSignInError", ScriptCB_LastSignInError },
};

// FUNCTION: SWBF2 0x005969a0
void InitNet()
{
    for (int i = 0; i < 135; ++i)
    {
        LuaHelper::Register(sNetCallbacks[i].pcName, sNetCallbacks[i].pfnCallback, 0);
    }

    Lua_Callbacks::HighestError = 0;
}

}

// GLOBAL: SWBF2 0x007e7378
int gSortMode = 4;

struct FriendAction
{
    const char* name;
    bool enabled;
    unsigned char padding[3];
};
 

// GLOBAL: SWBF2 0x007e7ea0
FriendAction sFriendActions[14] = {
    { "INVITE", false }, { "INVITEVOICE", false }, { "UNINVITE", false },
    { "ADD", false }, { "ADDVOICE", false }, { "JOIN", false },
    { "ACCEPT", false }, { "DECLINE", false }, { "NEVER", false },
    { "YESINVITE", false }, { "NOINVITE", false }, { "FEEDBACK", false },
    { "REMOVE", false }, { "BACK", false }
};

// GLOBAL: SWBF2 0x007e69ca
bool gSortForward = true;

// GLOBAL: SWBF2 0x01e57730
char Lua_Callbacks::pFilters[7][64];

// GLOBAL: SWBF2 0x01e578f0
char gSkipToNTGUIProfile[64];

struct SessionSortKey
{
    bool present;
    unsigned char padding[3];
    int first;
    int second;
    char* name;
    float ping;
    int sessionIndex;
    int serverRank;
    char* map;
};
 

struct FindGamePlayer
{
    char mPlayerName[64];
    int mNumKills;
    int mNumDeaths;
    int mNumPoints;
    int mNumHeroPoints;
};
 

// FUNCTION: SWBF2 0x00593b50
int __cdecl SessionCompare(const void* pLeft, const void* pRight)
{
    const SessionSortKey& left = *(const SessionSortKey*)pLeft;
    const SessionSortKey& right = *(const SessionSortKey*)pRight;
    if (left.present != right.present)
    {
        return left.present ? -1 : 1;
    }

    switch (gSortMode)
    {
    case 2:
        return _stricmp(left.name, right.name);
    case 3:
        if (left.serverRank < right.serverRank)
        {
            return 1;
        }
        if (left.serverRank > right.serverRank)
        {
            return -1;
        }
        break;
    case 4:
        break;
    case 5:
    {
        const int result = _stricmp(left.map, right.map);
        if (result)
        {
            return result;
        }
        break;
    }
    case 6:
    {
        int a = toupper(left.map[4]);
        int b = toupper(right.map[4]);
        if (a == 'I')
        {
            a = 'A';
        }
        else if (a == 'R')
        {
            a = 'C';
        }
        if (b == 'I')
        {
            b = 'A';
        }
        else if (b == 'R')
        {
            b = 'C';
        }
        if (a != b)
        {
            return a - b;
        }
        break;
    }
    case 7:
    {
        const int result = _stricmp(strchr(left.map, '_') + 1,
                                    strchr(right.map, '_') + 1);
        if (result)
        {
            return result;
        }
        break;
    }
    default:
        if (left.first < right.first)
        {
            return 1;
        }
        if (left.first > right.first)
        {
            return -1;
        }
        if (left.second < right.second)
        {
            return 1;
        }
        if (left.second > right.second)
        {
            return -1;
        }
        break;
    }
    if (right.ping > left.ping)
    {
        return -1;
    }
    if (left.ping > right.ping)
    {
        return 1;
    }
    return _stricmp(left.name, right.name);
}

int GetLastSignInError();
extern bool GetNetAwardsEnabled();
extern void SetNetAwardsEnabled(int);

extern bool netAutoAssignTeams;

namespace Lua_Callbacks { static const unsigned int netPlayersSupported[9] = {3, 6, 9, 12, 19, 25, 38, 76, 153}; }

// GLOBAL: SWBF2 0x007e8edb
bool NetSetup::mLoginErrorsAreFatal = true;

extern int TEMP_bandwith, gAutoNetPlayers;

// FUNCTION: SWBF2 0x00595b00
int Lua_Callbacks::GetSelectedFriendIdx()
{
    int selected;
    lua_pushstring(LuaHelper::mState, "gsfriends_listbox_layout");
    lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
    LuaHelper::TableReadVal("SelectedIdx", selected, false);
    lua_settop(LuaHelper::mState, -2);
    const unsigned int count = gf::friends::friendCount();
    return selected >= 1 && (unsigned int)selected <= count ? selected - 1 : 0xdddddddd;
}

// FUNCTION: SWBF2 0x00595b70
void RefreshLuaFriends(bool)
{
    const unsigned int count = gf::friends::friendCount();
    Lua_Callbacks::DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);
    for (unsigned int i = 0; i < count; ++i)
    {
        lua_newtable(LuaHelper::mState);
        LuaHelper::TableWriteVal("name", gf::friends::friendNameByIndex(i));
        LuaHelper::TableWriteVal("VoiceIcon", 0);
        LuaHelper::TableWriteVal("StateIcon", 0);
        lua_rawseti(LuaHelper::mState, -2, i + 1);
    }
    lua_pushstring(LuaHelper::mState, "friends_listbox_contents");
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    Lua_Callbacks::DoPeriodicLuaGC(false);
    GuiManager* pGui = GuiManager::Instance();
    LuaHelper::CallTableProcWThis(pGui ? pGui->mCurrentScope : (const char*)52,
                                  "RepaintListbox");
}

namespace Lua_Callbacks
{

void ResortSessions(bool bForward);

// FUNCTION: SWBF2 0x00592020
int GetLobbyPlayerCount()
{
    if (hostGame)
    {
        return hostGame->GetPlayerCount();
    }
    if (joinGame)
    {
        return joinGame->GetPlayerCount();
    }
    int iCount = 0;
    for (int i = 0; i < netCurMaxPlayers; ++i)
    {
        if (NetGame::IsPlaying(i, false))
        {
            ++iCount;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x00592070
int GetLobbyPlayerIndex(int iIndex)
{
    if (hostGame)
    {
        return hostGame->GetPlayerIndex(iIndex);
    }
    if (joinGame)
    {
        return joinGame->GetPlayerIndex(iIndex);
    }
    for (int iPlayer = 0; iPlayer < netCurMaxPlayers; ++iPlayer)
    {
        if (NetGame::IsPlaying(iPlayer, false) && iIndex-- == 0)
        {
            return iPlayer;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x005920d0
char const* GetLobbyPlayer(int iPlayer)
{
    if (iPlayer < 0)
    {
        return 0;
    }
    if (iPlayer == 64)
    {
        return netIsDedicated ? 0 : NetSetup::GetHostName();
    }
    if (hostGame)
    {
        return hostGame->GetPlayerName(iPlayer);
    }
    if (joinGame)
    {
        return joinGame->GetPlayerName(iPlayer);
    }
    return NetGame::GetPlayerName(iPlayer);
}

}

// GLOBAL: SWBF2 0x01e62f92
bool netIsHostDedicated;
// GLOBAL: SWBF2 0x01e57930
int sortTopTeam;

extern NetDest netDests[MAX_DESTS];
extern int netPlayerLimit;

// FUNCTION: SWBF2 0x00592120
bool IsLocalToHost(int iPlayer)
{
    return !netIsHostDedicated && iPlayer == 0;
}

// FUNCTION: SWBF2 0x00592140
int GetLobbyPing(int iPlayer)
{
    return NetComm::GetPing(iPlayer);
}

// FUNCTION: SWBF2 0x00592150
int GetLobbyKills(int iPlayer)
{
    return NetComm::GetPLKills(iPlayer);
}

// FUNCTION: SWBF2 0x00592160
int GetLobbyTeam(int iPlayer)
{
    if (iPlayer == 64)
    {
        return 0;
    }
    if (hostGame)
    {
        return hostGame->GetStartTeam(iPlayer) - 1;
    }
    if (joinGame)
    {
        return joinGame->GetStartTeam(iPlayer) - 1;
    }
    return NetGame::GetStartTeam(iPlayer) - 1;
}

// FUNCTION: SWBF2 0x005921a0
int GetLobbyQOS(int iPlayer)
{
    int iUp, iDown;
    if (IsLocalToHost(iPlayer) || iPlayer == 64)
    {
        NetComm::GetBandwidth(64, iUp, iDown);
        int iLimit = netPlayerLimit - (NetGame::IsDedicated() ? 0 : 1);
        float fRatio = (float)(iUp / 0x8000) / (float)iLimit;
        if (fRatio >= 1.0f)
        {
            return 5;
        }
        if (fRatio > 0.9f)
        {
            return 4;
        }
        if (fRatio > 0.8f)
        {
            return 3;
        }
        return fRatio > 0.7f ? 2 : 1;
    }
    NetComm::GetBandwidth(iPlayer, iUp, iDown);
    int iQuality = (iUp >= 10240 && iDown >= 0x8000) ? 5 : 1;
    int iPing = NetComm::GetPing(iPlayer);
    if (iPing >= 270 && iQuality > 1)
    {
        iQuality = 1;
    }
    else if (iPing >= 210 && iQuality > 2)
    {
        iQuality = 2;
    }
    else if (iPing >= 150 && iQuality > 3)
    {
        iQuality = 3;
    }
    else if (iPing >= 90 && iQuality > 4)
    {
        iQuality = 4;
    }
    return iQuality;
}

// FUNCTION: SWBF2 0x00592310
int GetLobbyBandwidth(int iPlayer)
{
    int iUp, iDown;
    if (IsLocalToHost(iPlayer) || iPlayer == 64)
    {
        NetComm::GetBandwidth(64, iUp, iDown);
        return iUp;
    }
    NetComm::GetBandwidth(iPlayer, iUp, iDown);
    return iDown;
}

// FUNCTION: SWBF2 0x00592360
int Lua_Callbacks::ScriptCB_GetLobbyPlayerlist(lua_State*)
{
    const char* pcScreen = GuiManager::Instance()->GetCurrentScreen();
    if (!pcScreen[0])
    {
        return 0;
    }

    char acBuffer[256];
    int aiPlayers[64];
    lua_pushnil(LuaHelper::mState);
    _snprintf(acBuffer, 0x100, "%s_listbox_contents", pcScreen);
    lua_pushstring(LuaHelper::mState, acBuffer);
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);

    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);
    int iCount = Lua_Callbacks::GetLobbyPlayerCount();
    NetGame::GetLocalIndex(PauseMenu::GetPausingViewport());
    for (int i = 0; i < iCount; ++i)
    {
        aiPlayers[i] = GetLobbyPlayerIndex(i);
    }

    if (GameState::GetState() == 0x8FF60339u)
    {
        sortTopTeam = Team::GetPtr(1)->GetCurReinforcements() >
                         Team::GetPtr(2)->GetCurReinforcements();
    }

    int iListIndex = 1;
    for (int i = 0; i < iCount; ++i)
    {
        int iPlayer = aiPlayers[i];
        const char* pcName = Lua_Callbacks::GetLobbyPlayer(iPlayer);
        if (!pcName || !pcName[0])
        {
            continue;
        }

        lua_newtable(LuaHelper::mState);
        LuaHelper::TableWriteVal("indexstr", iPlayer);
        LuaHelper::TableWriteVal("namestr", Lua_Callbacks::GetLobbyPlayer(iPlayer));
        _snprintf(acBuffer, 0x100, "%d", GetLobbyPing(iPlayer));
        LuaHelper::TableWriteVal("pingstr", acBuffer);
        _snprintf(acBuffer, 0x100, "%d", GetLobbyKills(iPlayer));
        LuaHelper::TableWriteVal("killsstr", acBuffer);
        _snprintf(acBuffer, 0x100, "%dK", GetLobbyBandwidth(iPlayer) / 1024);
        LuaHelper::TableWriteVal("bwstr", acBuffer);
        LuaHelper::TableWriteVal("iQOS", GetLobbyQOS(iPlayer));
        LuaHelper::TableWriteVal("iTeam", GetLobbyTeam(iPlayer));
        int iViewport = NetGame::GetJoystickIndex(iPlayer);
        LuaHelper::TableWriteVal("bIsLocal", iViewport >= 0);
        if (iViewport >= 0)
        {
            LuaHelper::TableWriteVal("iViewport", iViewport);
        }
        lua_rawseti(LuaHelper::mState, -2, iListIndex++);
    }
    _snprintf(acBuffer, 0x100, "%s_listbox_contents", pcScreen);
    lua_pushstring(LuaHelper::mState, acBuffer);
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    if (!netInShell)
    {
        DoPeriodicLuaGC(true);
    }
    return 0;
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00593d40
int ScriptCB_GetFilters(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
    if (pProfile)
    {
        const PlayerProfile::tOnlineOpt& options = pProfile->mOnlineOptions;
        lua_pushstring(LuaHelper::mState, options.mGameNameFilter);
        lua_pushnumber(LuaHelper::mState, (float)options.mMinPlayers);
        lua_pushnumber(LuaHelper::mState, (float)options.mMapIdx);
        lua_pushnumber(LuaHelper::mState, (float)options.mEraIdx);
        lua_pushnumber(LuaHelper::mState, (float)options.mMaxPing);
        lua_pushnumber(LuaHelper::mState, (float)options.mServerIdx);
    }
    else
    {
        lua_pushstring(LuaHelper::mState, "");
        for (int i = 0; i < 5; ++i)
        {
            lua_pushnumber(LuaHelper::mState, 0.0f);
        }
    }
    return 6;
}

// FUNCTION: SWBF2 0x00594d50
int ScriptCB_IsGamespyArcadePasswordReady(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, gGamespyAutoPassStr[0] != 0);
    return 1;
}

// FUNCTION: SWBF2 0x00595fb0
int ScriptCB_GetSkipToNTGUIProfileName(lua_State*)
{
    unsigned short name[64];
    PblUnicode::ustrncnv(name, gSkipToNTGUIProfile, 64);
    LuaHelper::PushString(name);
    return 1;
}

// FUNCTION: SWBF2 0x00596820
int ScriptCB_GetVoiceRenderConnected(lua_State*)
{
    if (!GameVoiceChat::IsOpen())
    {
        return 0;
    }
    VoiceChat::Node* pNode = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
    if (!pNode || pNode->GetID() == 0xFF)
    {
        return 0;
    }
    if (!GameVoiceChat::GetTransport()->IsConnectedReceive(pNode->GetID()))
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x00594170
int ScriptCB_GetSessionSortMode(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)gSortMode);
    return 1;
}

// FUNCTION: SWBF2 0x00594df0
int ScriptCB_BeginJoinIP(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x00594e50
int ScriptCB_UpdateQuickmatch(lua_State*)
{
    if (findGame)
    {
        findGame->Update();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594ef0
int ScriptCB_CancelQuickmatch(lua_State*)
{
    if (findGame)
    {
        findGame->Cancel();
        delete findGame;
        findGame = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594f20
int ScriptCB_IsQuickmatchDone(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, findGame && findGame->IsReady() ? 1.0f : 0.0f);
    return 1;
}

// FUNCTION: SWBF2 0x00594f60
int ScriptCB_IsQuickmatchPassworded(lua_State*)
{
    bool passworded = false;
    if (findGame)
    {
        char* name = 0;
        char* mission = 0;
        int numPlayers = 0;
        int maxPlayers = 0;
        bool flag = false;
        float ping;
        const unsigned int initialPing = 1092615143u;
        memcpy(&ping, &initialPing, sizeof(ping));
        int arg7 = 0;
        int arg8 = 0;
        bool arg10 = false;
        char* arg11 = 0;
        bool arg12 = false;
        findGame->GetInfo(name, mission, numPlayers, maxPlayers, flag,
                          ping, arg7, arg8, passworded, arg10, arg11, arg12);
    }
    lua_pushboolean(LuaHelper::mState, passworded);
    return 1;
}

// FUNCTION: SWBF2 0x00595010
int ScriptCB_IsBadNetworkConnection(lua_State*)
{
    int selected;
    lua_gettop(LuaHelper::mState);
    lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_layout");
    lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
    LuaHelper::TableReadVal("SelectedIdx", selected, false);
    lua_settop(LuaHelper::mState, -2);
    lua_pushstring(LuaHelper::mState, "mpsessionlist_listbox_contents");
    lua_gettable(LuaHelper::mState, LUA_GLOBALSINDEX);
    int session = selected;
    if (selected <= luaL_getn(LuaHelper::mState, -1))
    {
        lua_rawgeti(LuaHelper::mState, -1, selected);
        LuaHelper::TableReadVal("sessionIdx", session, false);
        lua_settop(LuaHelper::mState, -2);
    }
    lua_settop(LuaHelper::mState, -2);
    findGame->SelectFirst();
    if (session > 1)
    {
        findGame->Select(session - 1);
    }
    lua_gettop(LuaHelper::mState);

    char* name = 0;
    char* mission = 0;
    int numPlayers = 0;
    int maxPlayers = 0;
    bool flag = false;
    float ping = 0.0f;
    int arg7 = 0;
    int arg8 = 0;
    bool arg9 = false;
    bool arg10 = false;
    char* arg11 = 0;
    bool arg12 = false;
    findGame->GetInfo(name, mission, numPlayers, maxPlayers, flag,
                      ping, arg7, arg8, arg9, arg10, arg11, arg12);
    lua_pushboolean(LuaHelper::mState, ping * 1000.0f > 300.0f);
    return 1;
}

// FUNCTION: SWBF2 0x00594ff0
int ScriptCB_CanSupportMaxPlayers(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, 1);
    return 1;
}

// FUNCTION: SWBF2 0x005953a0
int ScriptCB_UpdateJoin(lua_State*)
{
    if (joinGame)
    {
        joinGame->Update();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005953c0
int ScriptCB_LaunchJoin(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x005953d0
int ScriptCB_CancelJoin(lua_State*)
{
    if (joinGame)
    {
        joinGame->Cancel();
        do
            joinGame->Update();
        while (!joinGame->IsCancelled());
        delete joinGame;
        joinGame = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00595420
int ScriptCB_IsJoinDone(lua_State*)
{
    float result = -1.0f;
    if (joinGame && !joinGame->IsFailed())
    {
        result = joinGame->IsConnected() ? 1.0f : 0.0f;
    }
    lua_pushnumber(LuaHelper::mState, result);
    return 1;
}

// FUNCTION: SWBF2 0x00595480
int ScriptCB_BeginLeave(lua_State*)
{
    if (netIsHost || netHostIndex >= 0)
    {
        hostGame->Cancel();
    }
    else
    {
        joinGame->Cancel();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005954b0
int ScriptCB_UpdateLeave(lua_State*)
{
    if (netIsHost || netHostIndex >= 0)
    {
        hostGame->Update();
    }
    else
    {
        joinGame->Update();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005954e0
int ScriptCB_LaunchLeave(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x005954f0
int ScriptCB_CancelLeave(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x00595500
int ScriptCB_IsLeaveDone(lua_State*)
{
    bool done = netIsHost || netHostIndex >= 0 ? hostGame->IsCancelled() : joinGame->IsCancelled();
    lua_pushnumber(LuaHelper::mState, done ? 1.0f : 0.0f);
    return 1;
}

// FUNCTION: SWBF2 0x00595560
int ScriptCB_IsNetHWPresent(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, 1);
    return 1;
}

// FUNCTION: SWBF2 0x00595580
int ScriptCB_IsNetCableIn(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, NetSetup::IsNetCableConnected());
    return 1;
}

// FUNCTION: SWBF2 0x005955a0
int ScriptCB_SetNoticeNoCable(lua_State*)
{
    NetSetup::mNoticeNoCable = lua_toboolean(LuaHelper::mState, 1) != 0;
    return 0;
}

// FUNCTION: SWBF2 0x005955c0
int ScriptCB_GetIPAddr(lua_State*)
{
    lua_pushstring(LuaHelper::mState, NetSetup::GetIPAddr());
    return 1;
}

// FUNCTION: SWBF2 0x005955e0
int ScriptCB_SetNetLoginName(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        unsigned short name[34];
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        PblUnicode::ustrncnv((char*)name, (const unsigned short*)pcName, 66);
        NetSetup::SetLoginName(gActiveViewport, (const char*)name);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00595c90
int ScriptCB_CancelFriends(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x00595ce0
int ScriptCB_GetFriendStateStr(lua_State*)
{
    return 0;
}

// FUNCTION: SWBF2 0x00595ea0
int ScriptCB_AllowSkipDNAS(lua_State*)
{
    lua_pushboolean(LuaHelper::mState, 0);
    return 1;
}

// FUNCTION: SWBF2 0x00595ff0
int ScriptCB_GetLobbyPlayerCount(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, (float)GetLobbyPlayerCount());
    return 1;
}

}

// FUNCTION: SWBF2 0x00593cf0
int __cdecl ReverseSessionCompare(const void* pLeft, const void* pRight)
{
    const unsigned char leftPresent = *(const unsigned char*)pLeft;
    const unsigned char rightPresent = *(const unsigned char*)pRight;
    if (leftPresent && !rightPresent)
    {
        return -1;
    }
    if (!leftPresent && rightPresent)
    {
        return 1;
    }
    return SessionCompare(pLeft, pRight) <= 0 ? 1 : -1;
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00594cb0
int ScriptCB_NextHost(lua_State*)
{
    if (netInShell)
    {
        NetSetupP::NextHost(1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594cd0
int ScriptCB_PreviousHost(lua_State*)
{
    if (netInShell)
    {
        NetSetupP::NextHost(-1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594dd0
int ScriptCB_BeginQuickmatch(lua_State*)
{
    findGame = NetSetup::FindGame(FT_QUICK, 0, 0);
    return 0;
}

// FUNCTION: SWBF2 0x00594e00
int ScriptCB_BeginJoinSpecial(lua_State*)
{
    if (NetSetup::GetSpecialJoinValid())
    {
        NetSessionId id;
        NetSetup::GetSpecialJoinAddress(&id);
        findGame = NetSetup::FindGame(FT_FIXED, 0, &id);
        NetSetup::SetSpecialJoinValid(false);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594e70
int ScriptCB_LaunchQuickmatch(lua_State*)
{
    const char* password = 0;
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        password = luaL_checkstring(LuaHelper::mState, 1);
        if (password && !password[0])
        {
            password = 0;
        }
    }
    NetSetup::SetPassword(password);
    joinGame = NetSetup::JoinGame(findGame);
    return 0;
}

// FUNCTION: SWBF2 0x00593e70
int ScriptCB_SetFilters(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    if (lua_gettop(L) != 6 || !lua_isstring(L, 1) ||
        !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4) || !lua_isnumber(L, 5) ||
        !lua_isnumber(L, 6))
    {
        return 0;
    }

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
    if (pProfile)
    {
        PlayerProfile::tOnlineOpt& options = pProfile->mOnlineOptions;
        strlcpy(options.mGameNameFilter, luaL_checkstring(L, 1), sizeof(options.mGameNameFilter));
        options.mMinPlayers = (unsigned char)(int)(float)luaL_checknumber(L, 2);
        options.mMapIdx = (unsigned char)(int)(float)luaL_checknumber(L, 3);
        options.mEraIdx = (unsigned char)(int)(float)luaL_checknumber(L, 4);
        int maxPing = (int)(float)luaL_checknumber(L, 5);
        if (maxPing < 0)
        {
            maxPing = 0;
        }
        if (maxPing > 65520)
        {
            maxPing = 65520;
        }
        options.mMaxPing = (unsigned short)maxPing;
        options.mServerIdx = (unsigned char)(int)(float)luaL_checknumber(L, 6);
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00593fc0
int ScriptCB_ApplyFilters(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    if (lua_gettop(L) != 7)
    {
        return 0;
    }
    pFilters[6][0] = 0;
    for (int i = 0; i < 7; ++i)
    {
        if (!lua_isstring(L, i + 1))
        {
            return 0;
        }
        strlcpy(pFilters[i], luaL_checkstring(L, i + 1), sizeof(pFilters[i]));
    }
    const char* all = "ifs.gsprofile.all";
    const int allFilters[] = {2, 3, 5, 6};
    for (int i = 0; i < 4; ++i)
    {
        if (strcmp(pFilters[allFilters[i]], all) == 0)
        {
            pFilters[allFilters[i]][0] = 0;
        }
    }
    if (findGame)
    {
        ResortSessions(true);
        ScriptCB_GetSessionList(0);
        LuaHelper::CallTableProcWThis("ifs_mp_sessionlist", "RepaintListbox");
    }
    return 0;
}

// FUNCTION: SWBF2 0x005941a0
int ScriptCB_SetSessionSortMode(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int mode = (int)(float)luaL_checknumber(L, 1);
        if (mode < 1)
        {
            mode = 1;
        }
        if (mode > 8)
        {
            mode = 8;
        }
        if (gSortMode == mode)
        {
            gSortForward = !gSortForward;
        }
        else
        {
            gSortMode = mode;
        }
        ResortSessions(true);
        ScriptCB_GetSessionList(0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00594230
int ScriptCB_SetAsFavorite(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    if (lua_gettop(L) != 2 || !lua_isnumber(L, 1) || lua_type(L, 2) != LUA_TBOOLEAN)
    {
        return 0;
    }
    const int index = (int)(float)luaL_checknumber(L, 1) - 1;
    const bool favorite = lua_toboolean(L, 2) != 0;
    if (findGame)
    {
        findGame->SelectFirst();
        if (index >= 0)
        {
            findGame->SelectSortPos(index);
        }
        findGame->SetFavorite(favorite);
        ScriptCB_GetSessionList(0);
        LuaHelper::CallTableProcWThis("ifs_mp_sessionlist", "RepaintListbox");
    }
    return 0;
}

// FUNCTION: SWBF2 0x00593230
int ScriptCB_GetExtraSessionInfo(lua_State*)
{
    if (!findGame)
    {
        return 0;
    }

    char* pcName = 0;
    char* pcMission = 0;
    int iNumPlayers = 0;
    int iMaxPlayers;
    bool bPassword;
    float fPing = 9.999f;
    int iServerType = 0;
    int iSortPos;
    bool bLocked = false;
    bool bRightVersion;
    char* pcVersion = 0;
    bool bFavorite;

    findGame->SelectFirst();
    int iSession;
    for (iSession = 0; iSession < lastSessionCount; ++iSession)
    {
        if (iSession == findGame->GetSelectedSession())
        {
            break;
        }
        findGame->Select(1);
    }

    if (iSession >= lastSessionCount)
    {
        lua_newtable(LuaHelper::mState);
        lua_pushstring(LuaHelper::mState, "mpsessionlist_serverinfo_contents");
        lua_insert(LuaHelper::mState, -2);
        lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
        lua_newtable(LuaHelper::mState);
        lua_pushstring(LuaHelper::mState, "mpsessionlist_playerlist_contents");
        lua_insert(LuaHelper::mState, -2);
        lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    }
    else
    {
        findGame->GetInfo(pcName, pcMission, iNumPlayers, iMaxPlayers, bPassword, fPing,
                          iServerType, iSortPos, bLocked, bRightVersion, pcVersion, bFavorite);

        const unsigned short* pusOn = RedLocalizeDB::Find(0xBEC02787, false);
        const unsigned short* pusOff = RedLocalizeDB::Find(0x3969C5AB, false);
        const unsigned short* pusEasy = RedLocalizeDB::Find(0x6A8A91A2, false);
        const unsigned short* pusMedium = RedLocalizeDB::Find(0x6B6CFA1B, false);
        const unsigned short* pusHard = RedLocalizeDB::Find(0xB544ABF9, false);

        unsigned short ausText[256];
        unsigned short ausArg[256];
        char acNumber[256];
        FindGameRules rules;
        FindGamePlayer player;

        lua_newtable(LuaHelper::mState);
        bool bHaveRules = false;
        if (findGame->GetRules(rules))
        {
            bHaveRules = true;

            lua_newtable(LuaHelper::mState);
            PblUnicode::ustrncnv(ausText, pcName, 255);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 1);

            lua_newtable(LuaHelper::mState);
            PblUnicode::ustrlcpy(ausText, RedLocalizeDB::Find(0x145D8471, false), 256);
            PblUnicode::ustrncnv(ausArg, " ", 256);
            PblUnicode::ustrlcat(ausText, ausArg, 256);
            const char* pcModeName = "modename.name.con";
            if (pcMission)
            {
                const char* pcMode = strchr(pcMission, '_');
                if (pcMode)
                {
                    if (!strncmp(pcMode, "_1flag", 6))
                    {
                        pcModeName = "modename.name.1flag";
                    }
                    else if (!strncmp(pcMode, "_ctf", 4))
                    {
                        pcModeName = "modename.name.ctf";
                    }
                    else if (!strncmp(pcMode, "_obj", 4))
                    {
                        pcModeName = "modename.name.obj";
                    }
                    else if (!strncmp(pcMode, "_ass", 4))
                    {
                        pcModeName = "modename.name.ass";
                    }
                    else if (!strncmp(pcMode, "_hunt", 5))
                    {
                        pcModeName = "modename.name.hunt";
                    }
                    else if (!strncmp(pcMode, "_eli", 4))
                    {
                        pcModeName = "modename.name.elimination";
                    }
                }
            }
            PblUnicode::ustrlcat(ausText, RedLocalizeDB::Find(PblHash(pcModeName), false), 256);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 2);

            lua_newtable(LuaHelper::mState);
            const unsigned short* pusFormat = RedLocalizeDB::Find(0x61804B54, false);
            _snprintf(acNumber, sizeof(acNumber), "%d", rules.mNumAIUnits);
            PblUnicode::ustrncnv(ausArg, acNumber, 256);
            PblUnicode::usnprintf(ausText, 256, pusFormat, ausArg);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 3);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0xCBB2619B, false);
            _snprintf(acNumber, sizeof(acNumber), "%d", rules.mMinPlayers);
            PblUnicode::ustrncnv(ausArg, acNumber, 256);
            PblUnicode::usnprintf(ausText, 256, pusFormat, ausArg);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 4);

            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("text", RedLocalizeDB::Find(rules.mAutoTeams ? 0x71A886D8 : 0x76A9F05A, false));
            lua_rawseti(LuaHelper::mState, -2, 5);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0x99C3E5C6, false);
            PblUnicode::usnprintf(ausText, 256, pusFormat, rules.mTeamDamage ? pusOn : pusOff);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 6);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0x61828675, false);
            if (rules.mAIDifficulty == 1)
            {
                PblUnicode::usnprintf(ausText, 256, pusFormat, pusEasy);
            }
            else if (rules.mAIDifficulty == 3)
            {
                PblUnicode::usnprintf(ausText, 256, pusFormat, pusHard);
            }
            else
            {
                PblUnicode::usnprintf(ausText, 256, pusFormat, pusMedium);
            }
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 7);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0x9F493F0F, false);
            PblUnicode::usnprintf(ausText, 256, pusFormat, rules.mShowPlayerNames ? pusOn : pusOff);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 8);

            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("text", RedLocalizeDB::Find(rules.mHeroes ? 0x46AEBA83 : 0x8777BA60, false));
            lua_rawseti(LuaHelper::mState, -2, 9);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0xCD2D0322, false);
            _snprintf(acNumber, sizeof(acNumber), "%d", rules.mSpawnInvincibilityTime);
            PblUnicode::ustrncnv(ausArg, acNumber, 256);
            PblUnicode::usnprintf(ausText, 256, pusFormat, ausArg);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 10);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0xE4C8D8B5, false);
            PblUnicode::usnprintf(ausText, 256, pusFormat, bLocked ? pusOn : pusOff);
            LuaHelper::TableWriteVal("text", ausText);
            lua_rawseti(LuaHelper::mState, -2, 11);

            lua_newtable(LuaHelper::mState);
            pusFormat = RedLocalizeDB::Find(0xF388D488, false);
            if (pcVersion)
            {
                PblUnicode::ustrncnv(ausArg, pcVersion, 256);
                PblUnicode::usnprintf(ausText, 256, pusFormat, ausArg);
                LuaHelper::TableWriteVal("text", ausText);
            }
            lua_rawseti(LuaHelper::mState, -2, 12);
        }
        lua_pushstring(LuaHelper::mState, "mpsessionlist_serverinfo_contents");
        lua_insert(LuaHelper::mState, -2);
        lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);

        lua_newtable(LuaHelper::mState);
        int iRow = 1;
        for (int iPlayer = 0; iPlayer < iNumPlayers; ++iPlayer)
        {
            if (bHaveRules && findGame->GetPlayerData(iPlayer, player))
            {
                lua_newtable(LuaHelper::mState);
                PblUnicode::ustrncnv(ausText, player.mPlayerName, 255);
                LuaHelper::TableWriteVal("text", ausText);
                LuaHelper::TableWriteVal("kills", player.mNumKills);
                LuaHelper::TableWriteVal("deaths", player.mNumDeaths);
                LuaHelper::TableWriteVal("points", player.mNumPoints);
                LuaHelper::TableWriteVal("heropoints", player.mNumHeroPoints);
                lua_rawseti(LuaHelper::mState, -2, iRow++);
            }
        }
        lua_pushstring(LuaHelper::mState, "mpsessionlist_playerlist_contents");
        lua_insert(LuaHelper::mState, -2);
        lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    }
    LuaHelper::CallTableProcWThis("ifs_mp_sessionlist", "RepaintExtraInfo");
    return 0;
}

// FUNCTION: SWBF2 0x00594300
void ResortSessions(bool bForward)
{
    if (!findGame)
    {
        return;
    }

    SessionSortKey sessions[2000];
    int iCount = lastSessionCount;

    const size_t nameLen = strlen(pFilters[0]);
    char cEra = 0;
    if (pFilters[3][0])
    {
        if (strcmp(pFilters[3], "common.era.cw") == 0)
        {
            cEra = 'c';
        }
        else if (strcmp(pFilters[3], "common.era.gcw") == 0)
        {
            cEra = 'g';
        }
    }

    char acMap[64];
    size_t mapLen = 0;
    if (pFilters[2][0])
    {
        strlcpy(acMap, pFilters[2], sizeof(acMap));
        char* pcCut = strchr(acMap, '%');
        if (pcCut)
        {
            *pcCut = 0;
        }
        pcCut = strchr(acMap, '_');
        if (pcCut && pcCut > acMap)
        {
            pcCut[-1] = 0;
        }
        mapLen = strlen(acMap);
    }
    else
    {
        acMap[0] = 0;
    }

    int iServerType = 0;
    if (pFilters[5][0])
    {
        if (!_stricmp(pFilters[5], "ifs.mp.sessionlist.servertypes.pc"))
        {
            iServerType = 1;
        }
        else if (!_stricmp(pFilters[5], "ifs.mp.sessionlist.servertypes.pcdedicated"))
        {
            iServerType = 2;
        }
        else if (!_stricmp(pFilters[5], "ifs.mp.sessionlist.servertypes.ps2"))
        {
            iServerType = 3;
        }
        else if (!_stricmp(pFilters[5], "ifs.mp.sessionlist.servertypes.ps2dedicated"))
        {
            iServerType = 4;
        }
        else if (!_stricmp(pFilters[5], "ifs.mp.sessionlist.servertypes.xbox"))
        {
            iServerType = 5;
        }
        else if (!_stricmp(pFilters[5], "ifs.mp.sessionlist.servertypes.xboxdedicated"))
        {
            iServerType = 6;
        }
    }

    int iMaxPing = 0;
    if (pFilters[4][0])
    {
        iMaxPing = atoi(pFilters[4]);
        if (iMaxPing < 0)
        {
            iMaxPing = 0;
        }
    }
    const float fMaxPing = (float)iMaxPing / 1000.0f;

    int iMinPlayers = 0;
    if (pFilters[1][0])
    {
        iMinPlayers = atoi(pFilters[1]);
        if (iMinPlayers < 0)
        {
            iMinPlayers = 0;
        }
    }

    if (iCount >= 2000)
    {
        iCount = 1999;
    }

    findGame->SelectFirst();
    int iVisible = 0;
    SessionSortKey* pKey = sessions;
    for (int i = 0; i < iCount; ++i)
    {
        bool bPassword;
        int iType;
        int iSortPos;
        bool bLocked;
        bool bRightVersion;
        char* pcVersion;
        findGame->GetInfo(pKey->name, pKey->map, pKey->first, pKey->second, bPassword,
                          pKey->ping, iType, iSortPos, bLocked, bRightVersion, pcVersion,
                          pKey->present);
        pKey->sessionIndex = i;
        switch (iType)
        {
        case 2: pKey->serverRank = 6; break;
        case 3: pKey->serverRank = 1; break;
        case 4: pKey->serverRank = 4; break;
        case 5: pKey->serverRank = 2; break;
        case 6: pKey->serverRank = 5; break;
        default: pKey->serverRank = 3; break;
        }

        bool bFiltered = false;
        if (nameLen)
        {
            const size_t len = strlen(pKey->name);
            if (nameLen <= len)
            {
                size_t pos = 0;
                while (_strnicmp(pKey->name + pos, pFilters[0], nameLen))
                {
                    if (++pos > len - nameLen)
                    {
                        bFiltered = true;
                        break;
                    }
                }
            }
            else
            {
                bFiltered = true;
            }
        }
        if (iMinPlayers && pKey->first < iMinPlayers)
        {
            bFiltered = true;
        }
        if (mapLen && _strnicmp(acMap, pKey->map, mapLen))
        {
            bFiltered = true;
        }
        if (pFilters[6][0])
        {
            char acMode[64];
            strlcpy(acMode, pKey->map, sizeof(acMode));
            const char* pcMode = strchr(acMode, '_');
            if (pcMode)
            {
                ++pcMode;
            }
            if (!_stricmp(pFilters[6], "ass"))
            {
                if (_stricmp("eli", pcMode) && _stricmp(pFilters[6], pcMode))
                {
                    bFiltered = true;
                }
            }
            else if (_stricmp(pFilters[6], pcMode))
            {
                bFiltered = true;
            }
        }
        if (cEra)
        {
            const char* pcSep = strchr(pKey->map, '_');
            if (pcSep && pcSep > pKey->map)
            {
                const char cMapEra = (char)tolower(pcSep[-1]);
                if (cMapEra && cMapEra != cEra)
                {
                    bFiltered = true;
                }
            }
        }
        if (iMaxPing > 0 && iMaxPing < 9999 && pKey->ping > fMaxPing)
        {
            bFiltered = true;
        }
        if (iServerType && iServerType != iType)
        {
            bFiltered = true;
        }

        findGame->SetFiltered(bFiltered);
        if (!bFiltered)
        {
            ++iVisible;
            ++pKey;
        }
        findGame->Select(1);
    }

    qsort(sessions, iVisible, sizeof(SessionSortKey), gSortForward ? SessionCompare : ReverseSessionCompare);

    findGame->SelectFirst();
    for (int i = 0; i < iCount; ++i)
    {
        if (!findGame->GetFiltered())
        {
            for (int pos = 0; pos < iCount; ++pos)
            {
                if (sessions[pos].sessionIndex == i)
                {
                    findGame->SetSortPosition((unsigned short)pos);
                    if (bForward && pos == 0)
                    {
                        findGame->SetSelectedSession(i);
                    }
                    break;
                }
            }
        }
        findGame->Select(1);
    }
    findGame->SetNumVisible(iVisible);
    findGame->SetDirty(false);
}

// FUNCTION: SWBF2 0x00594900
int ScriptCB_GetSessionList(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    lua_gettop(L);
    if (!findGame)
    {
        return 0;
    }

    LuaHelper::CallTableProcWThis("ifs_mp_sessionlist", "ClearListbox");
    const char* contentNames[3] = {
        "mpsessionlist_listbox_contents", "mpsessionlist_serverinfo_contents",
        "mpsessionlist_playerlist_contents"
    };
    for (int i = 0; i < 3; ++i)
    {
        lua_newtable(L);
        lua_pushstring(L, contentNames[i]);
        lua_insert(L, -2);
        lua_settable(L, LUA_GLOBALSINDEX);
    }
    DoPeriodicLuaGC(true);
    lua_newtable(L);
    const int count = lastSessionCount;
    if (findGame->GetDirty())
    {
        ResortSessions(false);
    }
    findGame->SelectFirst();
    int selected = -1;
    int visible = 0;
    int players = 0;
    int maxPlayers = 0;
    for (int i = 0; i < count; ++i)
    {
        if (!findGame->GetFiltered())
        {
            char* name = 0;
            char* map = 0;
            int numPlayers = 0;
            int maxSessionPlayers = 0;
            bool passworded = false;
            float ping = 0.0f;
            int serverType = 0;
            int sortPosition = 0;
            bool locked = false;
            bool rightVersion = false;
            char* unusedName = 0;
            bool favorite = false;
            if (!findGame->GetInfo(name, map, numPlayers, maxSessionPlayers, passworded,
                                   ping, serverType, sortPosition, locked, rightVersion,
                                   unusedName, favorite))
            {
                continue;
            }
            if (sortPosition < 1000)
            {
                lua_newtable(L);
                ++visible;
                players += numPlayers;
                maxPlayers += maxSessionPlayers;
                LuaHelper::TableWriteVal("namestr", name);
                char countText[32];
                _snprintf(countText, sizeof(countText), "%d/%d", numPlayers, maxSessionPlayers);
                LuaHelper::TableWriteVal("numplayers", countText);
                LuaHelper::TableWriteVal("mapname", map);
                LuaHelper::TableWriteVal("bFavorite", favorite);
                LuaHelper::TableWriteVal("bLocked", locked);
                LuaHelper::TableWriteVal("bWrongVer", !rightVersion);
                LuaHelper::TableWriteVal("servertype", serverType);
                LuaHelper::TableWriteVal("pingint", (int)(float)(ping * 1000.0f));
                LuaHelper::TableWriteVal("sessionIdx", i + 1);
                lua_rawseti(L, -2, sortPosition + 1);
                if (i == findGame->GetSelectedSession())
                {
                    selected = sortPosition + 1;
                }
            }
        }
        findGame->Select(1);
    }
    lua_pushstring(L, "mpsessionlist_listbox_contents");
    lua_insert(L, -2);
    lua_settable(L, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(true);
    lua_pushstring(L, "ifs_mp_sessionlist");
    lua_gettable(L, LUA_GLOBALSINDEX);
    LuaHelper::TableWriteVal("iNumSessions", visible);
    LuaHelper::TableWriteVal("iNumPlayers", players);
    LuaHelper::TableWriteVal("iMaxPlayers", maxPlayers);
    lua_pushstring(L, "ifs_mp_sessionlist");
    lua_insert(L, -2);
    lua_settable(L, LUA_GLOBALSINDEX);
    if (selected != -1)
    {
        lua_pushstring(L, "mpsessionlist_listbox_layout");
        lua_gettable(L, LUA_GLOBALSINDEX);
        LuaHelper::TableWriteVal("SelectedIdx", selected);
        LuaHelper::TableWriteVal("CursorIdx", selected);
        lua_settop(L, -2);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00595180
int ScriptCB_BeginJoin(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    lua_gettop(L);
    int session;
    lua_pushstring(L, "mpsessionlist_listbox_layout");
    lua_gettable(L, LUA_GLOBALSINDEX);
    LuaHelper::TableReadVal("SelectedIdx", session, false);
    lua_settop(L, -2);
    lua_pushstring(L, "mpsessionlist_listbox_contents");
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (session <= luaL_getn(L, -1))
    {
        lua_rawgeti(L, -1, session);
        LuaHelper::TableReadVal("sessionIdx", session, false);
        lua_settop(L, -2);
    }
    lua_settop(L, -2);

    findGame->SelectFirst();
    if (session > 1)
    {
        findGame->Select(session - 1);
    }
    char* name = 0;
    char* map = 0;
    int players = 0, maxPlayers = 0;
    bool passworded = false;
    float ping = 0.0f;
    int serverType = 0, sortPosition = 0;
    bool locked = false, rightVersion = false;
    char* extra = 0;
    bool favorite = false;
    findGame->GetInfo(name, map, players, maxPlayers, passworded, ping,
                      serverType, sortPosition, locked, rightVersion, extra, favorite);
    if (!rightVersion)
    {
        return 0;
    }

    const char* password = 0;
    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        password = luaL_checkstring(L, 1);
    }
    NetSetup::SetPassword(password);
    if (!NetComm::IsLAN() && password)
    {
        const unsigned char* remoteHash = (const unsigned char*)static_cast<CNetFindGame*>(findGame)->GetDescriptorExtension();
        unsigned char localHash[64] = {0};
        Sha512Hash((const unsigned char*)password, strlen(password), localHash, 0);
        if (memcmp(remoteHash, localHash, sizeof(localHash)) != 0)
        {
            JRR_to_Errorbox(JRR_PASSWORD);
            return 0;
        }
    }
    joinGame = NetSetup::JoinGame(findGame);
    lua_gettop(L);
    return 0;
}

// FUNCTION: SWBF2 0x00595640
int ScriptCB_GetLobbyPlayerFlags(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    bool isFriend = false;
    bool canKick = false;
    bool friendUnavailable = true;
    if (lua_gettop(L) == 2 && lua_isstring(L, 1))
    {
        const char* name = luaL_checkstring(L, 1);
        const int wantedPlayer = (int)(float)luaL_checknumber(L, 2);
        for (int i = 0; i < GetLobbyPlayerCount(); ++i)
        {
            const int player = GetLobbyPlayerIndex(i);
            if (strcmp(name, GetLobbyPlayer(player)) != 0 || player != wantedPlayer)
            {
                continue;
            }

            if (!NetComm::IsLAN())
            {
                const unsigned __int64 id = NetGame::GetPlayerGSID(player);
                isFriend = gf::friends::isFriend(*(const galaxy::api::GalaxyID*)&id);
                friendUnavailable = false;
            }
            const int owner = NetGame::GetGuestOf(player);
            const int voteTarget = NetGame::GetBootPlayer();
            if (voteTarget < 0 || voteTarget == player)
            {
                if (NetGetHostPlayer() == -1)
                {
                    canKick = player != 64 && owner != 64;
                }
                else if (player)
                {
                    canKick = owner != 0;
                }
            }
            if (NetGame::IsSpectator(NetGame::GetLocalIndex(0)))
            {
                canKick = false;
            }
            const bool networkEnabled = netInShell ? netEnabled : netEnabledNext;
            if (networkEnabled && !netIsHost && netHostIndex < 0 && !hostVoteThreshold)
            {
                canKick = false;
            }
            break;
        }
    }
    lua_pushboolean(L, 0);
    lua_pushboolean(L, isFriend);
    lua_pushboolean(L, canKick);
    lua_pushboolean(L, 0);
    lua_pushboolean(L, friendUnavailable);
    return 5;
}

}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00595c70
int ScriptCB_BeginFriends(lua_State*)
{
    lua_toboolean(LuaHelper::mState, 1);
    RefreshLuaFriends(false);
    return 0;
}

// FUNCTION: SWBF2 0x00595ca0
int ScriptCB_UpdateFriends(lua_State*)
{
    lua_toboolean(LuaHelper::mState, 1);
    if (gf::friends::getAndResetDirty())
    {
        RefreshLuaFriends(false);
    }
    lua_pushboolean(LuaHelper::mState, 0);
    return 1;
}

// FUNCTION: SWBF2 0x00595cf0
int ScriptCB_GetFriendActions(lua_State*)
{
    lua_newtable(LuaHelper::mState);
    const unsigned int selected = (unsigned int)GetSelectedFriendIdx();
    for (int i = 0; i < 14; ++i)
    {
        sFriendActions[i].enabled = false;
    }
    sFriendActions[5].enabled = gf::friends::isJoinableByIndex(selected);
    for (int i = 0; i < 14; ++i)
    {
        if (sFriendActions[i].enabled)
        {
            LuaHelper::TableWriteVal(sFriendActions[i].name, 1);
        }
    }
    return 1;
}

// FUNCTION: SWBF2 0x00595d70
int ScriptCB_DoFriendAction(lua_State*)
{
    lua_State* L = LuaHelper::mState;
    if (lua_gettop(L) != 2 || !lua_isstring(L, 1))
    {
        return 0;
    }
    const char* action = luaL_checkstring(L, 1);
    int index = 14;
    for (int i = 0; i < 14; ++i)
    {
        if (_stricmp(sFriendActions[i].name, action) == 0)
        {
            index = i;
        }
    }
    if (index == 5)
    {
        const unsigned int friendIndex = (unsigned int)GetSelectedFriendIdx();
        if (gf::friends::isJoinableByIndex(friendIndex) &&
            GameState::GetState() == 0x11e1fc01)
        {
            NetSessionId id;
            id.id = 0;
            const char* connection = gf::friends::connectionStringByIndex(friendIndex);
            sscanf(connection, "%ull", &id.id);
            findGame = NetSetup::FindGame(FT_FIXED, 0, &id);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00595930
int ScriptCB_SetGSProfileInfo(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    lua_State* L = LuaHelper::mState;
    if (!pProfile || lua_gettop(L) != 5 ||
        !lua_isstring(L, 1) || !lua_isstring(L, 2) ||
        !lua_isstring(L, 3) || !lua_isstring(L, 4) ||
        !lua_isnumber(L, 5))
    {
        return 0;
    }

    luaL_checkstring(L, 1);
    const char* email = luaL_checkstring(L, 2);
    const char* password = luaL_checkstring(L, 3);
    const char* saveInfo = luaL_checkstring(L, 4);
    int autoLogin = (int)(float)luaL_checknumber(L, 5);
    if (autoLogin < 0)
    {
        autoLogin = 0;
    }
    if (autoLogin > 2)
    {
        autoLogin = 2;
    }
    PlayerProfile::tOnlineOpt& options = pProfile->mOnlineOptions;
    options.mu8GSID_Autologin = (unsigned char)autoLogin;
    memset(options.mGamespyNickname, 0, sizeof(options.mGamespyNickname));
    memset(options.mGamespyEmail, 0, sizeof(options.mGamespyEmail));
    memset(options.mGamespyPassword, 0, sizeof(options.mGamespyPassword));
    options.mu8_SaveInfo = 0x10;
    if (saveInfo && strlen(saveInfo) == 3)
    {
        if (saveInfo[0] != '0')
        {
            options.mu8_SaveInfo |= 1;
        }
        if (saveInfo[1] != '0')
        {
            options.mu8_SaveInfo |= 2;
            strlcpy(options.mGamespyEmail, email, sizeof(options.mGamespyEmail));
        }
        if (saveInfo[2] != '0')
        {
            options.mu8_SaveInfo |= 4;
            strlcpy(options.mGamespyPassword, password, sizeof(options.mGamespyPassword));
        }
    }
    pProfile->UpdateProfCRC();
    return 0;
}

// FUNCTION: SWBF2 0x00596340
int ScriptCB_SetVoiceSendPreset(lua_State*)
{
    if (GameVoiceChat::IsOpen())
    {
        const int preset = (int)(float)luaL_checknumber(LuaHelper::mState, 1);
        void* node = GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            ((GameVoiceChat::Node*)node)->SetSendMask(preset);
            static_cast<GameVoiceChat::Node*>(node)->Update();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596390
int ScriptCB_GetVoiceSendPreset(lua_State*)
{
    unsigned int preset = 5;
    if (GameVoiceChat::IsOpen())
    {
        void* node = GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            unsigned char enabled, keepCurrent;
            ((GameVoiceChat::Node*)node)->GetSendMask(&preset, &enabled, &keepCurrent);
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)(int)preset);
    return 1;
}

// FUNCTION: SWBF2 0x005963f0
int ScriptCB_SetVoiceReceivePreset(lua_State*)
{
    if (GameVoiceChat::IsOpen())
    {
        const int preset = (int)(float)luaL_checknumber(LuaHelper::mState, 1);
        void* node = GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            ((GameVoiceChat::Node*)node)->SetReceiveMask(preset);
            static_cast<GameVoiceChat::Node*>(node)->Update();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596440
int ScriptCB_GetVoiceReceivePreset(lua_State*)
{
    unsigned int preset = 5;
    if (GameVoiceChat::IsOpen())
    {
        void* node = GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            unsigned char enabled, keepCurrent;
            ((GameVoiceChat::Node*)node)->GetReceiveMask(&preset, &enabled, &keepCurrent);
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)(int)preset);
    return 1;
}

// FUNCTION: SWBF2 0x005964a0
int ScriptCB_SetVoiceSendEnable(lua_State*)
{
    if (GameVoiceChat::IsOpen())
    {
        const unsigned int selected = (unsigned int)luaL_checknumber(LuaHelper::mState, 1);
        const unsigned char player = (unsigned char)GetLobbyPlayerIndex(selected);
        VoiceChat::Node* node = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            const bool enabled = lua_toboolean(LuaHelper::mState, 2) != 0;
            GameVoiceChat::GetTransport()->SetSendEnable(node->GetID(), player, enabled, false);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596640
int ScriptCB_SetVoiceReceiveEnable(lua_State*)
{
    if (GameVoiceChat::IsOpen())
    {
        const unsigned int selected = (unsigned int)luaL_checknumber(LuaHelper::mState, 1);
        const unsigned char player = (unsigned char)GetLobbyPlayerIndex(selected);
        VoiceChat::Node* node = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            const bool enabled = lua_toboolean(LuaHelper::mState, 2) != 0;
            GameVoiceChat::GetTransport()->SetReceiveEnable(node->GetID(), player, enabled, false);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00596520
int ScriptCB_GetVoiceSendStatus(lua_State*)
{
    int status = 0;
    if (GameVoiceChat::IsOpen())
    {
        const int player = lua_gettop(LuaHelper::mState) >= 1
            ? GetLobbyPlayerIndex((unsigned int)luaL_checknumber(LuaHelper::mState, 1))
            : PauseMenu::GetPausingJoystick();
        const unsigned char selected = (unsigned char)player;
        VoiceChat::Node* node = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        GameVoiceChat::Transport* transport = GameVoiceChat::GetTransport();
        if (!node || node->GetID() == 0xff)
        {
            status = 1;
        }
        else
        {
            const unsigned char local = node->GetID();
            if (selected == local)
            {
                if (transport->IsConnectedSend(local))
                {
                    status = 4 + node->IsCapturing();
                }
            }
            else if (transport->IsConnectedSend(selected))
            {
                if (transport->GetReceivePossible(local, selected))
                {
                    status = 4 + transport->GetSending(selected);
                }
                else if (transport->GetReceiveEnable(local, selected))
                {
                    status = 3;
                }
                else
                {
                    status = 1 + transport->GetSendEnable(selected, local);
                }
            }
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)status);
    return 1;
}

// FUNCTION: SWBF2 0x005966c0
int ScriptCB_GetVoiceReceiveStatus(lua_State*)
{
    int status = 0;
    if (GameVoiceChat::IsOpen())
    {
        const int player = lua_gettop(LuaHelper::mState) >= 1
            ? GetLobbyPlayerIndex((unsigned int)luaL_checknumber(LuaHelper::mState, 1))
            : PauseMenu::GetPausingJoystick();
        const unsigned char selected = (unsigned char)player;
        VoiceChat::Node* node = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        GameVoiceChat::Transport* transport = GameVoiceChat::GetTransport();
        if (!node || node->GetID() == 0xff)
        {
            status = 1;
        }
        else
        {
            const unsigned char local = node->GetID();
            if (selected == local)
            {
                if (transport->IsConnectedSend(local))
                {
                    status = 4 + node->IsReceiving();
                }
                else
                {
                    status = transport->IsConnectedReceive(selected) ? 6 : 0;
                }
            }
            else if (transport->IsConnectedReceive(selected))
            {
                if (transport->GetSendPossible(local, selected))
                {
                    if (transport->IsConnectedSend(selected))
                    {
                        status = 4 + transport->GetSending(local);
                    }
                    else
                    {
                        status = transport->IsConnectedReceive(selected) ? 6 : 0;
                    }
                }
                else if (transport->GetSendEnable(local, selected))
                {
                    status = 3;
                }
                else
                {
                    status = 1 + transport->GetReceiveEnable(selected, local);
                }
            }
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)status);
    return 1;
}

// FUNCTION: SWBF2 0x00596130
int ScriptCB_VoiceEnable(lua_State*)
{
    if (Snd::Engine::GetMixConfig() == Snd::Engine::mixConfig_Disabled)
    {
        return 0;
    }
    lua_State* L = LuaHelper::mState;
    const int joystick = lua_gettop(L) >= 2
        ? (int)(float)luaL_checknumber(L, 2) : PauseMenu::GetPausingJoystick();
    const int local = GameState::GetState() == 0x11e1fc01
        ? joystick : NetGame::GetLocalIndex(PauseMenu::GetPausingViewport());
    bool enabled = false;
    VoiceChat::Decoder* decoder = 0;
    if (local >= 0)
    {
        VoiceChat::Node* node = (VoiceChat::Node*)GameVoiceChat::FindNodeByDeviceID();
        if (node)
        {
            enabled = lua_toboolean(L, 1) != 0;
            ((GameVoiceChat::Node*)node)->SetID(enabled ? local : 0xff);
            if (enabled)
            {
                decoder = &node->GetDecoder();
                VoiceChat::Encoder& encoder = node->GetEncoder();
                if (!((VoiceChat::Encoder*)(&encoder))->IsEnabled() && ((VoiceChat::Decoder*)decoder)->GetNumOutputs() > 1)
                {
                    ((VoiceChat::Decoder*)decoder)->SetGain(1.0f, 1);
                }
            }
        }
    }
    PlayerProfile::tProfile* profile = PlayerProfile::GetProfile(joystick);
    if (profile)
    {
        if (decoder && ((VoiceChat::Decoder*)decoder)->GetNumOutputs() > 1)
        {
            profile->mOnlineOptions.mu8VoiceToTVVol =
                (unsigned char)(int)(((VoiceChat::Decoder*)decoder)->GetGain(1) * 10.0f);
        }
        profile->mOnlineOptions.mVoiceEnable = enabled;
        profile->UpdateProfCRC();
    }
    if (enabled && gVoiceLocalEchoEnabled)
    {
        VoiceLocalEchoEnable(true);
    }
    return 0;
}

}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005916b0
int ScriptCB_InNetSession(lua_State*) { lua_pushboolean(LuaHelper::mState, NetSetup::IsInSession()); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005916d0
int ScriptCB_NetWasHost(lua_State*) { lua_pushboolean(LuaHelper::mState, NetSetup::WasHost()); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005916f0
int ScriptCB_NetWasDedicated(lua_State*) { lua_pushboolean(LuaHelper::mState, NetSetup::WasDedicated()); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591710
int ScriptCB_NetWasDedicatedQuit(lua_State*) { lua_pushnumber(LuaHelper::mState, (float)(unsigned char)PauseMenu::sPauseDedicated); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591740
int ScriptCB_NetWasClient(lua_State*) { lua_pushboolean(LuaHelper::mState, NetSetup::WasClient()); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005917f0
int ScriptCB_EndAutoNet(lua_State*) { autoNetEnabled = false; return 0; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591800
int ScriptCB_GetAutoNetMode(lua_State*) { lua_pushstring(LuaHelper::mState, autoNetScript); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591910
int ScriptCB_IsBootInvitePending(lua_State*) { lua_pushboolean(LuaHelper::mState, 0); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591930
int ScriptCB_GetAutoAssignTeams(lua_State*) { lua_pushboolean(LuaHelper::mState, netAutoAssignTeams); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005919a0
int ScriptCB_GetGameName(lua_State*) { lua_pushstring(LuaHelper::mState, NetSetup::GetHostName()); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005919c0
int ScriptCB_SetGameName(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        NetSetup::SetGameName(luaL_checklstring(LuaHelper::mState, 1, 0));
    }
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591b50
int ScriptCB_EnableJournal(lua_State*) { NetGame::EnableJournal(); return 0; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591b60
int ScriptCB_EnablePlayback(lua_State*) { NetGame::EnablePlayback(); return 0; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591ef0
int ScriptCB_TrackLoginErrors(lua_State*) { NetSetup::mLoginErrorsAreFatal = lua_toboolean(LuaHelper::mState, 1) != 0; return 0; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00591f30
int ScriptCB_BeginLobby(lua_State*)
{
    if (!netInShell)
    {
        if (BootDisplay::sInstance[0])
        {
            BootDisplay::sInstance[0]->SetLobbyMode(true);
            BootDisplay::EnableViewport(0, true);
        }
        ScriptCB_GetLobbyPlayerlist(0);
        return 0;
    }
    if (!netIsHost || hostGame)
    {
        return 0;
    }
    PlayerProfile::tProfile* profile = PlayerProfile::GetCurProfile();
    unsigned int index = profile ? profile->mOnlineOptions.mu8PlayersSupported : 0;
    if (index >= 9)
    {
        index = 8;
    }
    unsigned int players = Lua_Callbacks::netPlayersSupported[index];
    if (NetComm::IsLAN())
    {
        players = 32;
    }
    gAutoNetPlayers = players;
    hostGame = NetSetup::HostGame();
    lastChangeCount = 0;
    if (netIsDedicated)
    {
        return 0;
    }
    GuiManager* gui = GuiManager::mCurViewport < 1 ?
        &GuiManager::mInstances[GuiManager::mCurViewport] : 0;
    LuaHelper::GetGlobal(gui->mCurrentScope);
    LuaHelper::TableWriteVal("launchflag", 1);
    lua_settop(LuaHelper::mState, -2);
    return 0;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005969e0
int ScriptCB_SearchStatsRank(lua_State*) { return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x005969f0
int ScriptCB_GetBandwidth(lua_State*) { lua_pushnumber(LuaHelper::mState, (float)TEMP_bandwith); return 1; }
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00596a20
int ScriptCB_SetBandwidth(lua_State*)
{
    if (lua_isnumber(LuaHelper::mState, 1))
    {
        float bandwidth = (float)luaL_checknumber(LuaHelper::mState, 1);
        TEMP_bandwith = (int)bandwidth;
        NetComm::SetBandwidthPercent();
    }
    return 0;
}
}

// FUNCTION: SWBF2 0x00596ab0
int Lua_Callbacks::ScriptCB_LastSignInError(lua_State* L)
{
    switch (GetLastSignInError())
    {
    case 1:
        lua_pushstring(LuaHelper::mState, "ifs.mp.galaxy.SIGNIN_SERVICE_NOT_AVAILABLE");
        return 1;
    case 3:
        lua_pushstring(LuaHelper::mState, "ifs.mp.galaxy.SIGNIN_CONNECTION_FAILURE");
        return 1;
    case 4:
        lua_pushstring(LuaHelper::mState, "ifs.mp.galaxy.SIGNIN_NO_LICENSE");
        return 1;
    case 5:
        lua_pushstring(LuaHelper::mState, "ifs.mp.galaxy.SIGNIN_INVALID_CREDENTIALS");
        return 1;
    default:
        lua_pushstring(LuaHelper::mState, "ifs.mp.galaxy.SIGNIN_SERVICE_NOT_SIGNED_IN");
        return 1;
    }
}

// FUNCTION: SWBF2 0x00591620
int Lua_Callbacks::ScriptCB_GetWasHost(lua_State*) { lua_pushboolean(LuaHelper::mState, gWasHost); return 1; }

// FUNCTION: SWBF2 0x00596a60
int Lua_Callbacks::ScriptCB_GetPlayerAwardsEnabled(lua_State*) { lua_pushboolean(LuaHelper::mState, GetNetAwardsEnabled()); return 1; }

// FUNCTION: SWBF2 0x00596a80
int Lua_Callbacks::ScriptCB_SetPlayerAwardsEnabled(lua_State*) { SetNetAwardsEnabled(lua_toboolean(LuaHelper::mState, 1) != 0); return 0; }

// GLOBAL: SWBF2 0x007e7374
int TEMP_bandwith = 100;

// GLOBAL: SWBF2 0x01e65060

char gLoginName[256];

// TEMPLATE: SWBF2 0x0052f300
// ?assign@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QAEAAV12@PBD@Z

// TEMPLATE: SWBF2 0x0052f340
// ?assign@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QAEAAV12@PBDI@Z

// TEMPLATE: SWBF2 0x0052f570
// ?assign@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QAEAAV12@ABV12@II@Z

// TEMPLATE: SWBF2 0x0052f9f0
// ?erase@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QAEAAV12@II@Z

// TEMPLATE: SWBF2 0x0052f690
// std::basic_string<char,std::char_traits<char>,std::allocator<char> >::_Grow

// TEMPLATE: SWBF2 0x0052fa70
// std::basic_string<char,std::char_traits<char>,std::allocator<char> >::_Copy

// TEMPLATE: SWBF2 0x0052fbd0
// std::_Wrap_alloc<std::allocator<char> >::allocate

// FUNCTION: SWBF2 0x0052e430
void sub_52E430() {}

// FUNCTION: SWBF2 0x00530150
void __stdcall sub_530150(unsigned int) {}

// FUNCTION: SWBF2 0x00530160
void __stdcall sub_530160(unsigned int) {}

// FUNCTION: SWBF2 0x00530170
int __fastcall FillWordRange(unsigned int first, unsigned int last, const int* value)
{
    unsigned int count = first > last ? 0 : (last - first + 3) >> 2;
    int result = 0;
    for (unsigned int i = 0; i < count; ++i)
    {
        result = *value;
        ((int*)first)[i] = result;
    }
    return result;
}

// GLOBAL: SWBF2 0x01e64a24

int netRequestedCameraCount = 0;
