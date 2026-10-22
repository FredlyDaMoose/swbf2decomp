#include <string.h>

#include "Battlefront2/Source/NetSetup.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "gfGalaxy.h"
#include "gfSteamAuth.h"
#include "Battlefront2/Source/NetMatch.h"

extern bool netLoginPending;
extern bool netLobbyRequestPending;
extern bool netLeavingLobby;
extern int netConnectState;
extern int netLoginFailureReason;
extern int netLobbyState;
extern int updatePercent;
extern unsigned __int64 netEnteredLobbyId;
extern unsigned __int64 netCreatedLobbyId;
extern char gLoginName[256];

namespace StatsManager { void Init(); }
void RemoveFoundGameByAddress(const NetAddr* address);
int __cdecl DecodeBase64(unsigned char* pOutput, unsigned int uiOutputCapacity, unsigned int* pOutputSize,
                       const unsigned char* pSource, unsigned int uiSourceSize);

// GLOBAL: SWBF2 0x01e65044
unsigned int gListedLobbyCount;

// FUNCTION: SWBF2 0x005d4610
void* GetGalaxyUser()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->User();
}

// FUNCTION: SWBF2 0x005d4630
void* GetGalaxyFriends()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Friends();
}

// FUNCTION: SWBF2 0x005d4650
void* GetGalaxyMatchmaking()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Matchmaking();
}

// FUNCTION: SWBF2 0x005d4670
void* GetGalaxyStats()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Stats();
}

// FUNCTION: SWBF2 0x005dafe0
void* GetGalaxyNetworkingInterface()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Networking();
}

// GLOBAL: SWBF2 0x01e64f0c
gf::GalaxyLobbyDataListener* gGalaxyLobbyDataListener;

// GLOBAL: SWBF2 0x01e64f10
gf::GalaxyLobbyCreatedListener* gGalaxyLobbyCreatedListener;

// GLOBAL: SWBF2 0x01e64f14
gf::GalaxyAuthListener* gGalaxyAuthListener;

// GLOBAL: SWBF2 0x01e64f18
gf::GalaxyLobbyListListener* gGalaxyLobbyListListener;

// GLOBAL: SWBF2 0x01e64f1c
gf::GalaxyLobbyMemberStateListener* gGalaxyLobbyMemberStateListener;

// GLOBAL: SWBF2 0x01e64f20
gf::GalaxyLobbyLeftListener* gGalaxyLobbyLeftListener;

// GLOBAL: SWBF2 0x01e64f24
gf::GalaxyLobbyEnteredListener* gGalaxyLobbyEnteredListener;

// GLOBAL: SWBF2 0x01e64f28
gf::GalaxyLobbyOwnerChangeListener* gGalaxyLobbyOwnerChangeListener;

// FUNCTION: SWBF2 0x005d4b80
void gf::GalaxyAuthListener::OnAuthSuccess()
{
    netLoginPending = false;
    netLoginFailureReason = 0;
    strlcpy(&gLoginName[m_iViewport << 8], NetMatch::GetLoginOverrideName(m_iViewport), 256);

    void* pUser = GetGalaxyUser();

    galaxy::api::GalaxyID result = ((galaxy::api::IUser*)pUser)->GetGalaxyID();
    const galaxy::api::GalaxyID* id = &result;
    NetMatch::SetLocalGSID((unsigned int)id->value, (unsigned int)(id->value >> 32));

    StatsManager::Init();
    gf::friends::start();

    void* pFriends = GetGalaxyFriends();
    ((galaxy::api::IFriends*)pFriends)->DeleteRichPresence("connect");

    void* pStats = GetGalaxyStats();
    galaxy::api::GalaxyID self;
    self.value = 0;
    ((galaxy::api::IStats*)pStats)->RequestUserStatsAndAchievements(self);
}

// FUNCTION: SWBF2 0x005d4c20
void gf::GalaxyAuthListener::OnAuthFailure(int failureReason)
{
    netLoginPending = false;
    netLoginFailureReason = failureReason;
    gf::friends::stop();
}

// FUNCTION: SWBF2 0x005d4c40
void gf::GalaxyAuthListener::OnAuthLost()
{
    netLoginPending = false;
    netLoginFailureReason = 0;
    gf::friends::stop();
}

// FUNCTION: SWBF2 0x005d4ce0
void gf::GalaxyLobbyCreatedListener::OnLobbyCreated(const galaxy::api::GalaxyID& lobbyID, int result)
{
    netLobbyRequestPending = false;
    if (!result)
    {
        netCreatedLobbyId = lobbyID.value;
    }
}

// FUNCTION: SWBF2 0x005d4d10
void gf::GalaxyLobbyLeftListener::OnLobbyLeft(const galaxy::api::GalaxyID& lobbyID, bool ioFailure)
{
    if (!netEnteredLobbyId)
    {
        netLobbyRequestPending = false;
        return;
    }
    if (netEnteredLobbyId != lobbyID.value)
    {
        return;
    }

    netLobbyRequestPending = false;
    netLobbyState = 0;
    netEnteredLobbyId = 0;
    netCreatedLobbyId = 0;
    if (!netLeavingLobby)
    {
        NetSetup::SetErrorMode(true);
        NetMatch::SetGameError("ifs.mp.joinerrors.connectlost", 6);
    }
    netLeavingLobby = false;
}

// FUNCTION: SWBF2 0x005d4da0
void gf::GalaxyLobbyMemberStateListener::OnLobbyMemberStateChanged(const galaxy::api::GalaxyID& lobbyID,
                                                                    const galaxy::api::GalaxyID& memberID,
                                                                    int memberStateChange)
{
}

// FUNCTION: SWBF2 0x005d4db0
void gf::GalaxyLobbyEnteredListener::OnLobbyEntered(const galaxy::api::GalaxyID& lobbyID, int result)
{
    if (netLobbyState != 3)
    {
        return;
    }
    netLobbyState = 0;
    netLobbyRequestPending = false;
    netConnectState = result;
    if (!result)
    {
        netEnteredLobbyId = lobbyID.value;
    }
}

// FUNCTION: SWBF2 0x005d4df0
void gf::GalaxyLobbyListListener::OnLobbyList(unsigned int lobbyCount, bool ioFailure)
{
    if (netLobbyState != 1)
    {
        return;
    }
    updatePercent = 0;
    netLobbyState = 2;
    gListedLobbyCount = lobbyCount;
    if (ioFailure)
    {
        return;
    }

    for (unsigned int i = 0; i < lobbyCount; ++i)
    {
        void* pMatchmaking = GetGalaxyMatchmaking();
        galaxy::api::GalaxyID lobby = ((galaxy::api::IMatchmaking*)pMatchmaking)->GetLobbyByIndex(i);
        NetAddr address;
        address.pubAddr = (int)lobby.value;
        address.pubPort = (int)(lobby.value >> 32);
        NetSetupP::AddGame(&address);
        pMatchmaking = GetGalaxyMatchmaking();
        ((galaxy::api::IMatchmaking*)pMatchmaking)->RequestLobbyData(lobby);
    }
}

// FUNCTION: SWBF2 0x005d4ea0
void gf::GalaxyLobbyDataListener::OnLobbyDataUpdated(const galaxy::api::GalaxyID& lobbyID,
                                                     const galaxy::api::GalaxyID& memberID)
{
    if (netLobbyState != 2 || !gListedLobbyCount)
    {
        return;
    }
    updatePercent += 100u / gListedLobbyCount;

    void* pMatchmaking = GetGalaxyMatchmaking();
    const unsigned int uCount = ((galaxy::api::IMatchmaking*)pMatchmaking)->GetLobbyDataCount(lobbyID);

    NetAddr address;
    address.pubAddr = (int)lobbyID.value;
    address.pubPort = (int)(lobbyID.value >> 32);
    FindGameData data;
    memset(&data, 0, sizeof(data));
    FindGameRules rules;
    memset(&rules, 0, sizeof(rules));
    NetSessionId session;
    NetSetup::GetPrevSessionId(&session);
    NetKey key;

    bool bDrop = false;
    for (unsigned int i = 0; i < uCount; ++i)
    {
        char acKey[512];
        char acValue[512];
        memset(acKey, 0, sizeof(acKey));
        memset(acValue, 0, sizeof(acValue));
        pMatchmaking = GetGalaxyMatchmaking();
        if (!((galaxy::api::IMatchmaking*)pMatchmaking)->GetLobbyDataByIndex(
                lobbyID, i, acKey, sizeof(acKey), acValue, sizeof(acValue)))
        {
            continue;
        }

        if (!strcmp(acKey, "fgd_str_host_name"))
        {
            strncpy(data.mName, acValue, sizeof(data.mName));
        }
        else if (!strcmp(acKey, "fgd_str_game_type"))
        {
            strncpy(data.mMapType, acValue, sizeof(data.mMapType));
        }
        else if (!strcmp(acKey, "fgd_str_map_name"))
        {
            strncpy(data.mMapName, acValue, sizeof(data.mMapName));
        }
        else if (!strcmp(acKey, "fgr_bool_netAutoAim"))
        {
            rules.mAimAssist = gf::utils::FromString(acValue, rules.mAimAssist);
        }
        else if (!strcmp(acKey, "fgr_bool_netAutoAssignTeams"))
        {
            rules.mAutoTeams = gf::utils::FromString(acValue, rules.mAutoTeams);
        }
        else if (!strcmp(acKey, "fgr_bool_netTeamDamage"))
        {
            rules.mTeamDamage = gf::utils::FromString(acValue, rules.mTeamDamage);
        }
        else if (!strcmp(acKey, "fgd_bool_has_password"))
        {
            data.mLocked = gf::utils::FromString(acValue, data.mLocked);
        }
        else if (!strcmp(acKey, "fgr_int_netNumBots"))
        {
            rules.mNumAIUnits = gf::utils::FromString(acValue, rules.mNumAIUnits);
        }
        else if (!strcmp(acKey, "fgr_int_r0"))
        {
            rules.mNumReinforcements1 = gf::utils::FromString(acValue, rules.mNumReinforcements1);
        }
        else if (!strcmp(acKey, "fgr_int_r1"))
        {
            rules.mNumReinforcements2 = gf::utils::FromString(acValue, rules.mNumReinforcements2);
        }
        else if (!strcmp(acKey, "fgd_int_numPlayers"))
        {
            data.mNumPlayers = gf::utils::FromString(acValue, data.mNumPlayers);
        }
        else if (!strcmp(acKey, "fgd_int_netPlayerLimit"))
        {
            data.mMaxPlayers = gf::utils::FromString(acValue, data.mMaxPlayers);
        }
        else if (!strcmp(acKey, "fgd_int_serverType"))
        {
            data.mServerType = gf::utils::FromString(acValue, data.mServerType);
        }
        else if (!strcmp(acKey, "fgd_int_turns_per_second"))
        {
            gf::utils::FromString(acValue, 0);
        }
        else if (!strcmp(acKey, "fgr_int_netMinPlayersToAutoLaunchDedicated"))
        {
            rules.mMinPlayers = gf::utils::FromString(acValue, rules.mMinPlayers);
        }
        else if (!strcmp(acKey, "fgr_int_mDifficulty"))
        {
            rules.mAIDifficulty = gf::utils::FromString(acValue, rules.mAIDifficulty);
        }
        else if (!strcmp(acKey, "fgr_int_netShowNames"))
        {
            rules.mShowPlayerNames = gf::utils::FromString(acValue, rules.mShowPlayerNames);
        }
        else if (!strcmp(acKey, "fgr_int_gSpawnInvicibility"))
        {
            rules.mSpawnInvincibilityTime = gf::utils::FromString(acValue, rules.mSpawnInvincibilityTime);
        }
        else if (!strcmp(acKey, "fgr_int_netHeroesEnabled"))
        {
            rules.mHeroes = gf::utils::FromString(acValue, rules.mHeroes);
        }
        else if (!strcmp(acKey, "fgr_uint_netHeroRuleUnlock"))
        {
            rules.mHeroUnlockRule = gf::utils::FromString(acValue, rules.mHeroUnlockRule);
        }
        else if (!strcmp(acKey, "fgr_int_netHeroRuleUnlockVal"))
        {
            rules.mHeroUnlockRuleVal = gf::utils::FromString(acValue, rules.mHeroUnlockRuleVal);
        }
        else if (!strcmp(acKey, "fgr_uint_netHeroRuleTeam"))
        {
            rules.mHeroTeamRule = gf::utils::FromString(acValue, rules.mHeroTeamRule);
        }
        else if (!strcmp(acKey, "fgr_uint_netHeroRulePlayer"))
        {
            rules.mHeroPlayerRule = gf::utils::FromString(acValue, rules.mHeroPlayerRule);
        }
        else if (!strcmp(acKey, "fgr_uint_netHeroRuleRespawn"))
        {
            rules.mHeroRespawnRule = gf::utils::FromString(acValue, rules.mHeroRespawnRule);
        }
        else if (!strcmp(acKey, "fgr_uint_netHeroRuleRespawnVal"))
        {
            rules.mHeroRespawnRuleVal = gf::utils::FromString(acValue, rules.mHeroRespawnRuleVal);
        }
        else if (!strcmp(acKey, "galaxy_revision"))
        {
            data.mbVerMatch = gf::utils::FromString(acValue, 0) == 1;
        }
        else if (!strcmp(acKey, "galaxy_lobby_cancelled"))
        {
            bDrop = gf::utils::FromString(acValue, false);
            if (bDrop)
            {
                break;
            }
        }
        else if (!strcmp(acKey, "galaxy_lobby_password"))
        {
            unsigned int uHashSize = 0;
            DecodeBase64((unsigned char*)data.mGalaxyLobbyPasswordData, sizeof(data.mGalaxyLobbyPasswordData), &uHashSize,
                       (const unsigned char*)acValue, strlen(acValue));
            if (uHashSize != 64)
            {
                bDrop = true;
                break;
            }
        }
    }

    pMatchmaking = GetGalaxyMatchmaking();
    const unsigned int uLobbyQuery = ((galaxy::api::IMatchmaking*)pMatchmaking)->GetNumLobbyMembers(lobbyID);
    if (bDrop || !uLobbyQuery || data.mNumPlayers <= 0)
    {
        RemoveFoundGameByAddress(&address);
        return;
    }
    data.mPingTime = 0.0f;
    NetSetupP::UpdateGame(&address, &key, &session, NetSetup::GetGameVersion(), &data, &rules);
}

// FUNCTION: SWBF2 0x005d5a00
void gf::GalaxyLobbyOwnerChangeListener::OnLobbyOwnerChanged(const galaxy::api::GalaxyID& lobbyID,
                                                             const galaxy::api::GalaxyID& newOwnerID)
{
    if (lobbyID.value != netEnteredLobbyId)
    {
        return;
    }
    void* pMatchmaking = GetGalaxyMatchmaking();
    ((galaxy::api::IMatchmaking*)pMatchmaking)->LeaveLobby(lobbyID);
}

// FUNCTION: SWBF2 0x005d6160
void NetMatch::StartLogin(int iViewport, bool bArg2, char const* pcArg3, char const* pcArg4,
                char const* pcArg5, bool bArg6)
{
    if (!gf::isInitialized() && gf::initialize())
    {
        gGalaxyAuthListener = gf::Allocate<gf::GalaxyAuthListener>(iViewport);
        gGalaxyLobbyCreatedListener = gf::Allocate<gf::GalaxyLobbyCreatedListener>();
        gGalaxyLobbyLeftListener = gf::Allocate<gf::GalaxyLobbyLeftListener>();
        gGalaxyLobbyMemberStateListener = gf::Allocate<gf::GalaxyLobbyMemberStateListener>();
        gGalaxyLobbyEnteredListener = gf::Allocate<gf::GalaxyLobbyEnteredListener>();
        gGalaxyLobbyListListener = gf::Allocate<gf::GalaxyLobbyListListener>();
        gGalaxyLobbyDataListener = gf::Allocate<gf::GalaxyLobbyDataListener>();
        gGalaxyLobbyOwnerChangeListener = gf::Allocate<gf::GalaxyLobbyOwnerChangeListener>();
    }
    netLoginFailureReason = 0;
    if (!gf::isInitialized())
    {
        return;
    }

    void* pUser = GetGalaxyUser();
    if (((galaxy::api::IUser*)pUser)->SignedIn() || netLoginPending)
    {
        return;
    }

    netLoginPending = true;
    NetMatch::SetLocalGSID(0, 0);
    if (gf::_sap())
    {
        const char* pcPersonaName = gf::_sap()->GetPersonaName();
        char acTicket[256] = { 0 };
        const int iTicketSize = gf::_sap()->GetEncryptedAppTicket(acTicket, sizeof(acTicket));
        if (pcPersonaName && iTicketSize > 0)
        {
            pUser = GetGalaxyUser();
            ((galaxy::api::IUser*)pUser)->SignInWithSteam(acTicket, iTicketSize, pcPersonaName);
            if (!gf::checkError())
            {
                return;
            }
        }
    }
    netLoginPending = false;
    netLoginFailureReason = 0;
}

// SYNTHETIC: SWBF2 0x005d4c60
// gf::GalaxyAuthListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8690
// gf::GalaxyLobbyCreatedListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8710
// gf::GalaxyLobbyLeftListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8790
// gf::GalaxyLobbyMemberStateListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8810
// gf::GalaxyLobbyEnteredListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8890
// gf::GalaxyLobbyListListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8910
// gf::GalaxyLobbyDataListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d8990
// gf::GalaxyLobbyOwnerChangeListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d78b0
// galaxy::api::SelfRegisteringListener<galaxy::api::IAuthListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d7940
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyCreatedListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d79d0
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyLeftListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d7a60
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyMemberStateListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d7af0
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyEnteredListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d7b80
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyListListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d7c10
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyDataListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005d7ca0
// galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyOwnerChangeListener>::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x005d7de0
// gf::Allocate<gf::GalaxyAuthListener,int &>

// TEMPLATE: SWBF2 0x005d7e80
// gf::Allocate<gf::GalaxyLobbyCreatedListener>

// TEMPLATE: SWBF2 0x005d7f20
// gf::Allocate<gf::GalaxyLobbyLeftListener>

// TEMPLATE: SWBF2 0x005d7fc0
// gf::Allocate<gf::GalaxyLobbyMemberStateListener>

// TEMPLATE: SWBF2 0x005d8060
// gf::Allocate<gf::GalaxyLobbyEnteredListener>

// TEMPLATE: SWBF2 0x005d8100
// gf::Allocate<gf::GalaxyLobbyListListener>

// TEMPLATE: SWBF2 0x005d81a0
// gf::Allocate<gf::GalaxyLobbyDataListener>

// TEMPLATE: SWBF2 0x005d8240
// gf::Allocate<gf::GalaxyLobbyOwnerChangeListener>
