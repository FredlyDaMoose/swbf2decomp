typedef int HSteamPipe;
typedef int HSteamUser;
typedef unsigned __int64 SteamAPICall_t;
typedef void(__cdecl* SteamAPIWarningMessageHook_t)(int, const char*);

class CCallbackBase;
class ISteamUser;
class ISteamFriends;
class ISteamUtils;

class ISteamClient
{
public:
    virtual HSteamPipe CreateSteamPipe() = 0;
    virtual bool BReleaseSteamPipe(HSteamPipe) = 0;
    virtual HSteamUser ConnectToGlobalUser(HSteamPipe) = 0;
    virtual HSteamUser CreateLocalUser(HSteamPipe*, int) = 0;
    virtual void ReleaseUser(HSteamPipe, HSteamUser) = 0;
    virtual ISteamUser* GetISteamUser(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamGameServer(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void SetLocalIPBinding(unsigned int, unsigned short) = 0;
    virtual ISteamFriends* GetISteamFriends(HSteamUser, HSteamPipe, const char*) = 0;
    virtual ISteamUtils* GetISteamUtils(HSteamPipe, const char*) = 0;
    virtual void* GetISteamMatchmaking(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamMatchmakingServers(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamGenericInterface(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamUserStats(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamGameServerStats(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamApps(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamNetworking(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamRemoteStorage(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamScreenshots(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void RunFrame() = 0;
    virtual unsigned int GetIPCCallCount() = 0;
    virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t) = 0;
    virtual bool BShutdownIfAllPipesClosed() = 0;
    virtual void* GetISteamHTTP(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamUnifiedMessages(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamController(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamUGC(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamAppList(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamMusic(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamMusicRemote(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamHTMLSurface(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess(void (*)()) = 0;
    virtual void DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess(void (*)()) = 0;
    virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(unsigned int (*)(int)) = 0;
    virtual void* GetISteamInventory(HSteamUser, HSteamPipe, const char*) = 0;
    virtual void* GetISteamVideo(HSteamUser, HSteamPipe, const char*) = 0;
};

class CSteamID;
class CGameID;
class ISteamUser
{
public:
    virtual HSteamUser GetHSteamUser() = 0;
    virtual bool BLoggedOn() = 0;
    virtual CSteamID GetSteamID() = 0;
    virtual int InitiateGameConnection(void*, int, CSteamID, unsigned int, unsigned short, bool) = 0;
    virtual void TerminateGameConnection(unsigned int, unsigned short) = 0;
    virtual void TrackAppUsageEvent(CGameID, int, const char*) = 0;
    virtual bool GetUserDataFolder(char*, int) = 0;
    virtual void StartVoiceRecording() = 0;
    virtual void StopVoiceRecording() = 0;
    virtual int GetAvailableVoice(unsigned int*, unsigned int*, unsigned int) = 0;
    virtual int GetVoice(bool, void*, unsigned int, unsigned int*, bool, void*, unsigned int, unsigned int*, unsigned int) = 0;
    virtual int DecompressVoice(const void*, unsigned int, void*, unsigned int, unsigned int*, unsigned int) = 0;
    virtual unsigned int GetVoiceOptimalSampleRate() = 0;
    virtual unsigned int GetAuthSessionTicket(void*, int, unsigned int*) = 0;
    virtual int BeginAuthSession(const void*, int, CSteamID) = 0;
    virtual void EndAuthSession(CSteamID) = 0;
    virtual void CancelAuthTicket(unsigned int) = 0;
    virtual int UserHasLicenseForApp(CSteamID, unsigned int) = 0;
    virtual bool BIsBehindNAT() = 0;
    virtual void AdvertiseGame(CSteamID, unsigned int, unsigned short) = 0;
    virtual SteamAPICall_t RequestEncryptedAppTicket(void*, int) = 0;
    virtual bool GetEncryptedAppTicket(void*, int, unsigned int*) = 0;
};

class ISteamFriends
{
public:
    virtual const char* GetPersonaName() = 0;
};

class ISteamUtils
{
public:
    virtual unsigned int GetSecondsSinceAppActive() = 0;
    virtual unsigned int GetSecondsSinceComputerActive() = 0;
    virtual int GetConnectedUniverse() = 0;
    virtual unsigned int GetServerRealTime() = 0;
    virtual const char* GetIPCountry() = 0;
    virtual bool GetImageSize(int, unsigned int*, unsigned int*) = 0;
    virtual bool GetImageRGBA(int, unsigned char*, int) = 0;
    virtual bool GetCSERIPPort(unsigned int*, unsigned short*) = 0;
    virtual unsigned char GetCurrentBatteryPower() = 0;
    virtual unsigned int GetAppID() = 0;
    virtual void SetOverlayNotificationPosition(int) = 0;
    virtual bool IsAPICallCompleted(SteamAPICall_t, bool*) = 0;
    virtual int GetAPICallFailureReason(SteamAPICall_t) = 0;
    virtual bool GetAPICallResult(SteamAPICall_t, void*, int, int, bool*) = 0;
    virtual void RunFrame() = 0;
    virtual unsigned int GetIPCCallCount() = 0;
    virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t) = 0;
};

extern "C" __declspec(dllimport) bool __cdecl SteamAPI_Init();
extern "C" __declspec(dllimport) bool __cdecl SteamAPI_RestartAppIfNecessary(unsigned int unOwnAppID);
extern "C" __declspec(dllimport) void __cdecl SteamAPI_Shutdown();
extern "C" __declspec(dllimport) void __cdecl SteamAPI_RunCallbacks();
extern "C" __declspec(dllimport) void __cdecl SteamAPI_RegisterCallResult(CCallbackBase* pCallback, SteamAPICall_t hAPICall);
extern "C" __declspec(dllimport) void __cdecl SteamAPI_UnregisterCallResult(CCallbackBase* pCallback, SteamAPICall_t hAPICall);
extern "C" __declspec(dllimport) HSteamPipe __cdecl SteamAPI_GetHSteamPipe();
extern "C" __declspec(dllimport) HSteamUser __cdecl SteamAPI_GetHSteamUser();
extern "C" __declspec(dllimport) void* __cdecl SteamInternal_ContextInit(void* pContextInitData);
extern "C" __declspec(dllimport) void* __cdecl SteamInternal_CreateInterface(const char* ver);

class CSteamAPIContext
{
public:
    void Clear()
    {
        m_pSteamClient = 0;
        m_pSteamUser = 0;
        m_pSteamFriends = 0;
        m_pSteamUtils = 0;
        m_pSteamMatchmaking = 0;
        m_pSteamUserStats = 0;
        m_pSteamApps = 0;
        m_pSteamMatchmakingServers = 0;
        m_pSteamNetworking = 0;
        m_pSteamRemoteStorage = 0;
        m_pSteamHTTP = 0;
        m_pSteamScreenshots = 0;
        m_pSteamUnifiedMessages = 0;
        m_pController = 0;
        m_pSteamUGC = 0;
        m_pSteamAppList = 0;
        m_pSteamMusic = 0;
        m_pSteamMusicRemote = 0;
        m_pSteamHTMLSurface = 0;
        m_pSteamInventory = 0;
    }

    bool Init()
    {
        HSteamUser hSteamUser = SteamAPI_GetHSteamUser();
        HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe();
        if (!hSteamPipe)
        {
            return false;
        }

        m_pSteamClient = (ISteamClient*)SteamInternal_CreateInterface("SteamClient017");
        if (!m_pSteamClient)
        {
            return false;
        }

        if (!(m_pSteamUser = m_pSteamClient->GetISteamUser(hSteamUser, hSteamPipe, "SteamUser019")))
        {
            return false;
        }
        if (!(m_pSteamFriends = m_pSteamClient->GetISteamFriends(hSteamUser, hSteamPipe, "SteamFriends015")))
        {
            return false;
        }
        if (!(m_pSteamUtils = m_pSteamClient->GetISteamUtils(hSteamPipe, "SteamUtils008")))
        {
            return false;
        }
        if (!(m_pSteamMatchmaking = m_pSteamClient->GetISteamMatchmaking(hSteamUser, hSteamPipe, "SteamMatchMaking009")))
        {
            return false;
        }
        if (!(m_pSteamMatchmakingServers = m_pSteamClient->GetISteamMatchmakingServers(hSteamUser, hSteamPipe, "SteamMatchMakingServers002")))
        {
            return false;
        }
        if (!(m_pSteamUserStats = m_pSteamClient->GetISteamUserStats(hSteamUser, hSteamPipe, "STEAMUSERSTATS_INTERFACE_VERSION011")))
        {
            return false;
        }
        if (!(m_pSteamApps = m_pSteamClient->GetISteamApps(hSteamUser, hSteamPipe, "STEAMAPPS_INTERFACE_VERSION008")))
        {
            return false;
        }
        if (!(m_pSteamNetworking = m_pSteamClient->GetISteamNetworking(hSteamUser, hSteamPipe, "SteamNetworking005")))
        {
            return false;
        }
        if (!(m_pSteamRemoteStorage = m_pSteamClient->GetISteamRemoteStorage(hSteamUser, hSteamPipe, "STEAMREMOTESTORAGE_INTERFACE_VERSION014")))
        {
            return false;
        }
        if (!(m_pSteamScreenshots = m_pSteamClient->GetISteamScreenshots(hSteamUser, hSteamPipe, "STEAMSCREENSHOTS_INTERFACE_VERSION003")))
        {
            return false;
        }
        if (!(m_pSteamHTTP = m_pSteamClient->GetISteamHTTP(hSteamUser, hSteamPipe, "STEAMHTTP_INTERFACE_VERSION002")))
        {
            return false;
        }
        if (!(m_pSteamUnifiedMessages = m_pSteamClient->GetISteamUnifiedMessages(hSteamUser, hSteamPipe, "STEAMUNIFIEDMESSAGES_INTERFACE_VERSION001")))
        {
            return false;
        }
        if (!(m_pController = m_pSteamClient->GetISteamController(hSteamUser, hSteamPipe, "SteamController005")))
        {
            return false;
        }
        if (!(m_pSteamUGC = m_pSteamClient->GetISteamUGC(hSteamUser, hSteamPipe, "STEAMUGC_INTERFACE_VERSION009")))
        {
            return false;
        }
        if (!(m_pSteamAppList = m_pSteamClient->GetISteamAppList(hSteamUser, hSteamPipe, "STEAMAPPLIST_INTERFACE_VERSION001")))
        {
            return false;
        }
        if (!(m_pSteamMusic = m_pSteamClient->GetISteamMusic(hSteamUser, hSteamPipe, "STEAMMUSIC_INTERFACE_VERSION001")))
        {
            return false;
        }
        if (!(m_pSteamMusicRemote = m_pSteamClient->GetISteamMusicRemote(hSteamUser, hSteamPipe, "STEAMMUSICREMOTE_INTERFACE_VERSION001")))
        {
            return false;
        }
        if (!(m_pSteamHTMLSurface = m_pSteamClient->GetISteamHTMLSurface(hSteamUser, hSteamPipe, "STEAMHTMLSURFACE_INTERFACE_VERSION_003")))
        {
            return false;
        }
        if (!(m_pSteamInventory = m_pSteamClient->GetISteamInventory(hSteamUser, hSteamPipe, "STEAMINVENTORY_INTERFACE_V001")))
        {
            return false;
        }
        m_pSteamVideo = m_pSteamClient->GetISteamVideo(hSteamUser, hSteamPipe, "STEAMVIDEO_INTERFACE_V001");
        return m_pSteamVideo != 0;
    }

    ISteamUser* SteamUser() const { return m_pSteamUser; }
    ISteamFriends* SteamFriends() const { return m_pSteamFriends; }
    ISteamUtils* SteamUtils() const { return m_pSteamUtils; }

private:
    ISteamClient* m_pSteamClient;
    ISteamUser* m_pSteamUser;
    ISteamFriends* m_pSteamFriends;
    ISteamUtils* m_pSteamUtils;
    void* m_pSteamMatchmaking;
    void* m_pSteamUserStats;
    void* m_pSteamApps;
    void* m_pSteamMatchmakingServers;
    void* m_pSteamNetworking;
    void* m_pSteamRemoteStorage;
    void* m_pSteamScreenshots;
    void* m_pSteamHTTP;
    void* m_pSteamUnifiedMessages;
    void* m_pController;
    void* m_pSteamUGC;
    void* m_pSteamAppList;
    void* m_pSteamMusic;
    void* m_pSteamMusicRemote;
    void* m_pSteamHTMLSurface;
    void* m_pSteamInventory;
    void* m_pSteamVideo;
};

inline void __cdecl SteamInternal_OnContextInit(void* p)
{
    ((CSteamAPIContext*)p)->Clear();
    if (SteamAPI_GetHSteamPipe())
    {
        ((CSteamAPIContext*)p)->Init();
    }
}

inline CSteamAPIContext& SteamInternal_ModuleContext()
{

    static void* volatile s_CallbackCounterAndContext[2 + sizeof(CSteamAPIContext) / sizeof(void*)] =
        { (void*)&SteamInternal_OnContextInit, 0 };
    return *(CSteamAPIContext*)SteamInternal_ContextInit((void*)s_CallbackCounterAndContext);
}

inline ISteamUser* SteamUser() { return SteamInternal_ModuleContext().SteamUser(); }
inline ISteamFriends* SteamFriends() { return SteamInternal_ModuleContext().SteamFriends(); }
inline ISteamUtils* SteamUtils() { return SteamInternal_ModuleContext().SteamUtils(); }

class CCallbackBase
{
public:
    CCallbackBase() { m_nCallbackFlags = 0; m_iCallback = 0; }
    virtual void Run(void* pvParam) = 0;
    virtual void Run(void* pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall) = 0;
    int GetICallback() { return m_iCallback; }
    virtual int GetCallbackSizeBytes() = 0;

protected:
    unsigned char m_nCallbackFlags;
    int m_iCallback;
};

template <class T, class P>
class CCallResult : private CCallbackBase
{
public:
    typedef void (T::*func_t)(P*, bool);

    CCallResult()
    {
        m_hAPICall = 0;
        m_pObj = 0;
        m_Func = 0;
        m_iCallback = P::k_iCallback;
    }

    void Set(SteamAPICall_t hAPICall, T* p, func_t func)
    {
        if (m_hAPICall)
        {
            SteamAPI_UnregisterCallResult(this, m_hAPICall);
        }
        m_hAPICall = hAPICall;
        m_pObj = p;
        m_Func = func;
        if (hAPICall)
        {
            SteamAPI_RegisterCallResult(this, hAPICall);
        }
    }

    bool IsActive() const { return m_hAPICall != 0; }

    void Cancel()
    {
        if (m_hAPICall != 0)
        {
            SteamAPI_UnregisterCallResult(this, m_hAPICall);
            m_hAPICall = 0;
        }
    }

    ~CCallResult() { Cancel(); }

private:
    virtual void Run(void* pvParam)
    {
        m_hAPICall = 0;
        (m_pObj->*m_Func)((P*)pvParam, false);
    }

    virtual void Run(void* pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall)
    {
        if (hSteamAPICall == m_hAPICall)
        {
            m_hAPICall = 0;
            (m_pObj->*m_Func)((P*)pvParam, bIOFailure);
        }
    }

    virtual int GetCallbackSizeBytes() { return sizeof(P); }

    SteamAPICall_t m_hAPICall;
    T* m_pObj;
    func_t m_Func;
};

struct EncryptedAppTicketResponse_t
{
    enum { k_iCallback = 154 };
    int m_eResult;
};
