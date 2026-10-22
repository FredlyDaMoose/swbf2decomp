#ifndef GF_GALAXY_API_H
#define GF_GALAXY_API_H

namespace galaxy { namespace api {

class IGalaxy;
class IErrorManager;
class IListenerRegistrar;

class GalaxyFactory
{
public:
    static IGalaxy* GetInstance();
    static IErrorManager* GetErrorManager();
    static void ResetInstance();
    static IGalaxy* CreateInstance();
};

class GalaxyID
{
public:
    GalaxyID() {}
    explicit GalaxyID(unsigned __int64 id) : value(id) {}

    unsigned __int64 ToUint64() const { return value; }
    unsigned __int64 value;
};

class IUser
{
public:
    virtual ~IUser() {}
    virtual bool SignedIn() = 0;
    virtual GalaxyID GetGalaxyID() = 0;
    virtual void SignInWithServerKey(const char*) = 0;
    virtual void SignInWithXboxOne(unsigned int) = 0;
    virtual void SignInWithPS4(const char*, const char*, const char*, unsigned int) = 0;
    virtual void SignInWithGalaxyCheckConnectivity(bool) = 0;
    virtual void SignInWithSteam(const char*, int, const char*) = 0;
};

class IFriends
{
public:
    virtual ~IFriends() {}
    virtual void RequestUserInformation(GalaxyID, bool) = 0;
    virtual const char* GetPersonaName() = 0;
    virtual void GetPersonaNameCopy(char*, unsigned int) = 0;
    virtual int GetPersonaState() = 0;
    virtual const char* GetFriendPersonaName(GalaxyID) = 0;
    virtual void GetFriendPersonaNameCopy(GalaxyID, char*, unsigned int) = 0;
    virtual int GetFriendPersonaState(GalaxyID) = 0;
    virtual const char* GetFriendAvatarUrl(GalaxyID, int) = 0;
    virtual void GetFriendAvatarUrlCopy(GalaxyID, int, char*, unsigned int) = 0;
    virtual unsigned int GetFriendAvatarImageID(GalaxyID, int) = 0;
    virtual void GetFriendAvatarImageRGBA(GalaxyID, int, void*, unsigned int) = 0;
    virtual bool IsFriendAvatarImageRGBAAvailable(GalaxyID, int) = 0;
    virtual void RequestFriendList() = 0;
    virtual bool IsFriend(GalaxyID) = 0;
    virtual unsigned int GetFriendCount() = 0;
    virtual GalaxyID GetFriendByIndex(unsigned int) = 0;
    virtual void SendFriendInvitation(GalaxyID) = 0;
    virtual void RequestFriendInvitationList() = 0;
    virtual unsigned int GetFriendInvitationCount() = 0;
    virtual void GetFriendInvitationByIndex(unsigned int, GalaxyID&, unsigned int&) = 0;
    virtual void RespondToFriendInvitation(GalaxyID, bool) = 0;
    virtual void DeleteFriend(GalaxyID) = 0;
    virtual void SetRichPresence(const char*, const char*) = 0;
    virtual void DeleteRichPresence(const char*) = 0;
};

class IStats
{
public:
    virtual ~IStats() {}
    virtual void RequestUserStatsAndAchievements(GalaxyID) = 0;
};

class IMatchmaking
{
public:
    virtual ~IMatchmaking() {}
    virtual void CreateLobby(int, unsigned int, bool, int) = 0;
    virtual bool RequestLobbyList(bool) = 0;
    virtual void AddRequestLobbyListStringFilter(const char*, const char*, int) = 0;
    virtual void AddRequestLobbyListNumericalFilter(const char*, int, int) = 0;
    virtual void AddRequestLobbyListNearValueFilter(const char*, int) = 0;
    virtual GalaxyID GetLobbyByIndex(unsigned int) = 0;
    virtual void JoinLobby(GalaxyID) = 0;
    virtual void LeaveLobby(GalaxyID) = 0;
    virtual void SetMaxNumLobbyMembers(GalaxyID, unsigned int) = 0;
    virtual unsigned int GetMaxNumLobbyMembers(GalaxyID) = 0;
    virtual unsigned int GetNumLobbyMembers(GalaxyID) = 0;
    virtual GalaxyID GetLobbyMemberByIndex(GalaxyID, unsigned int) = 0;
    virtual void SetLobbyType(GalaxyID, int) = 0;
    virtual int GetLobbyType(GalaxyID) = 0;
    virtual bool SetLobbyJoinable(GalaxyID, bool) = 0;
    virtual bool IsLobbyJoinable(GalaxyID) = 0;
    virtual void RequestLobbyData(GalaxyID) = 0;
    virtual const char* GetLobbyData(GalaxyID, const char*) = 0;
    virtual void GetLobbyDataCopy(GalaxyID, const char*, char*, unsigned int) = 0;
    virtual bool SetLobbyData(GalaxyID, const char*, const char*) = 0;
    virtual unsigned int GetLobbyDataCount(GalaxyID) = 0;
    virtual bool GetLobbyDataByIndex(GalaxyID, unsigned int, char*, unsigned int,
                                     char*, unsigned int) = 0;
    virtual void DeleteLobbyData(GalaxyID, const char*) = 0;
    virtual const char* GetLobbyMemberData(GalaxyID, GalaxyID, const char*) = 0;
    virtual void GetLobbyMemberDataCopy(GalaxyID, GalaxyID, const char*, char*, unsigned int) = 0;
    virtual void SetLobbyMemberData(GalaxyID, const char*, const char*) = 0;
    virtual unsigned int GetLobbyMemberDataCount(GalaxyID, GalaxyID) = 0;
    virtual bool GetLobbyMemberDataByIndex(GalaxyID, GalaxyID, unsigned int, char*,
                                           unsigned int, char*, unsigned int) = 0;
    virtual void DeleteLobbyMemberData(GalaxyID, const char*) = 0;
    virtual GalaxyID GetLobbyOwner(GalaxyID) = 0;
};

class INetworking
{
public:
    virtual ~INetworking() {}
    virtual bool SendP2PPacket(GalaxyID, const void*, unsigned int, int, int) = 0;
    virtual bool PeekP2PPacket(void*, unsigned int, unsigned int*, GalaxyID&, int) = 0;
    virtual bool IsP2PPacketAvailable(unsigned int*, int) = 0;
    virtual bool ReadP2PPacket(void*, unsigned int, unsigned int*, GalaxyID&, int) = 0;
    virtual void PopP2PPacket(int) = 0;
    virtual int GetPingWith(GalaxyID) = 0;
};

class IError
{
public:
    virtual ~IError() {}
    virtual const char* GetName() const = 0;
    virtual const char* GetMsg() const = 0;
};

class IErrorManager
{
public:
    virtual ~IErrorManager() {}
    virtual IError* GetLastError() = 0;
};

struct InitOptions;

class IGalaxy
{
public:
    virtual ~IGalaxy() {}

    virtual void Init(const char*, const char*, const char*, bool) = 0;
    virtual void Init(const char*, const char*, bool) = 0;
    virtual void Init(const InitOptions&) = 0;
    virtual void Shutdown() = 0;
    virtual IUser* User() = 0;
    virtual IFriends* Friends() = 0;
    virtual void* Chat() = 0;
    virtual IMatchmaking* Matchmaking() = 0;
    virtual INetworking* Networking() = 0;
    virtual INetworking* ServerNetworking() = 0;
    virtual IStats* Stats() = 0;
    virtual void* Utils() = 0;
    virtual void* Apps() = 0;
    virtual void* Storage() = 0;
    virtual void* CustomNetworking() = 0;
    virtual IListenerRegistrar* GetListenerRegistrar() = 0;
    virtual void* GetLogger() = 0;
    virtual void ProcessData() = 0;
};

enum ListenerType
{
    LOBBY_LIST = 0,
    LOBBY_CREATED = 1,
    LOBBY_ENTERED = 2,
    LOBBY_LEFT = 3,
    LOBBY_DATA = 4,
    LOBBY_MEMBER_STATE = 5,
    LOBBY_OWNER_CHANGE = 6,
    AUTH = 7,
    GAME_JOIN_REQUESTED = 20,
    FRIEND_LIST_RETRIEVE = 23
};

class IGalaxyListener
{
public:
    virtual ~IGalaxyListener() {}
};

template <ListenerType type>
class GalaxyTypeAwareListener : public IGalaxyListener
{
public:
    static ListenerType GetListenerType() { return type; }
};

class IListenerRegistrar
{
public:
    virtual ~IListenerRegistrar() {}
    virtual void Register(ListenerType listenerType, IGalaxyListener* listener) = 0;
    virtual void Unregister(ListenerType listenerType, IGalaxyListener* listener) = 0;
};

inline IListenerRegistrar* ListenerRegistrar()
{
    IGalaxy* pGalaxy = GalaxyFactory::GetInstance();
    return pGalaxy->GetListenerRegistrar();
}

class IAuthListener : public GalaxyTypeAwareListener<AUTH>
{
public:
    virtual void OnAuthSuccess() = 0;
    virtual void OnAuthFailure(int failureReason) = 0;
    virtual void OnAuthLost() = 0;
};

class IFriendListListener : public GalaxyTypeAwareListener<FRIEND_LIST_RETRIEVE>
{
public:
    virtual void OnFriendListRetrieveSuccess() = 0;
    virtual void OnFriendListRetrieveFailure(int failureReason) = 0;
};

class IGameJoinRequestedListener : public GalaxyTypeAwareListener<GAME_JOIN_REQUESTED>
{
public:
    virtual void OnGameJoinRequested(GalaxyID userID, const char* connectionString) = 0;
};

class ILobbyListListener : public GalaxyTypeAwareListener<LOBBY_LIST>
{
public:
    virtual void OnLobbyList(unsigned int lobbyCount, bool ioFailure) = 0;
};

class ILobbyCreatedListener : public GalaxyTypeAwareListener<LOBBY_CREATED>
{
public:
    virtual void OnLobbyCreated(const GalaxyID& lobbyID, int result) = 0;
};

class ILobbyEnteredListener : public GalaxyTypeAwareListener<LOBBY_ENTERED>
{
public:
    virtual void OnLobbyEntered(const GalaxyID& lobbyID, int result) = 0;
};

class ILobbyLeftListener : public GalaxyTypeAwareListener<LOBBY_LEFT>
{
public:
    virtual void OnLobbyLeft(const GalaxyID& lobbyID, bool ioFailure) = 0;
};

class ILobbyDataListener : public GalaxyTypeAwareListener<LOBBY_DATA>
{
public:
    virtual void OnLobbyDataUpdated(const GalaxyID& lobbyID, const GalaxyID& memberID) = 0;
};

class ILobbyMemberStateListener : public GalaxyTypeAwareListener<LOBBY_MEMBER_STATE>
{
public:
    virtual void OnLobbyMemberStateChanged(const GalaxyID& lobbyID, const GalaxyID& memberID,
                                           int memberStateChange) = 0;
};

class ILobbyOwnerChangeListener : public GalaxyTypeAwareListener<LOBBY_OWNER_CHANGE>
{
public:
    virtual void OnLobbyOwnerChanged(const GalaxyID& lobbyID, const GalaxyID& newOwnerID) = 0;
};

template <typename _TypeAwareListener>
class SelfRegisteringListener : public _TypeAwareListener
{
public:
    explicit SelfRegisteringListener(IListenerRegistrar* _registrar = 0)
        : registrar(_registrar)
    {
        IListenerRegistrar* pRegistrar = registrar ? registrar : ListenerRegistrar();
        if (pRegistrar)
        {
            pRegistrar->Register(_TypeAwareListener::GetListenerType(), this);
        }
    }

    ~SelfRegisteringListener()
    {
        IListenerRegistrar* pRegistrar = registrar ? registrar : ListenerRegistrar();
        if (pRegistrar)
        {
            pRegistrar->Unregister(_TypeAwareListener::GetListenerType(), this);
        }
    }

private:
    IListenerRegistrar* registrar;
};

} }

#endif
