#include <stdio.h>
#include <string.h>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>

#include "gfGalaxy.h"
#include "gfSteamAuth.h"

namespace
{
#include "steamworks.h"
}

void __fastcall FreeGalaxyGlueObject(void* pBlock);

extern bool gFriendListDirty;
extern bool gGalaxyInitialized;
extern bool gFriendsStarted;

// GLOBAL: SWBF2 0x007eb918
// SYNTHETIC: SWBF2 0x00403be0
// `dynamic initializer for 'gEncryptedAppTicket''
// SYNTHETIC: SWBF2 0x00768820
// `dynamic atexit destructor for 'gEncryptedAppTicket''
std::string gEncryptedAppTicket;

// GLOBAL: SWBF2 0x01ec0bf0
// SYNTHETIC: SWBF2 0x00403bf0
// `dynamic initializer for 'gFriendConnectionStrings''
// SYNTHETIC: SWBF2 0x00768880
// `dynamic atexit destructor for 'gFriendConnectionStrings''
std::unordered_map<unsigned __int64, std::string> gFriendConnectionStrings;

// TEMPLATE: SWBF2 0x0052fbb0
// std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >

// TEMPLATE: SWBF2 0x0052f740
// std::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >

// TEMPLATE: SWBF2 0x0052fc00
// std::list<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >::clear

// TEMPLATE: SWBF2 0x0052fe70
// std::_List_alloc<0,std::_List_base_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > >::_Buynode0

// TEMPLATE: SWBF2 0x0052fc90
// std::vector<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > >,std::_Wrap_alloc<std::allocator<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > > >::_Insert_n

// TEMPLATE: SWBF2 0x0052feb0
// std::vector<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > >,std::_Wrap_alloc<std::allocator<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > > >::_Grow_to

// TEMPLATE: SWBF2 0x00530460
// std::_Uninitialized_fill_n<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > *,unsigned int,std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > >,std::_Wrap_alloc<std::_Wrap_alloc<std::allocator<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > > > >

// TEMPLATE: SWBF2 0x005305e0
// std::_Uninitialized_move<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > *,std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > *,std::_Wrap_alloc<std::_Wrap_alloc<std::allocator<std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > > > >

// GLOBAL: SWBF2 0x01e55fac
gf::SteamAuthProvider* gSteamAuthProvider;

// GLOBAL: SWBF2 0x01e55fb0
galaxy::api::IGalaxyListener* gFriendListListener;

// GLOBAL: SWBF2 0x01e55fb8
galaxy::api::IGalaxyListener* gGameJoinRequestedListener;

// GLOBAL: SWBF2 0x01faa480
char gUnsignedStringBuffer[256];

// GLOBAL: SWBF2 0x01faa580
char gSignedStringBuffer[256];

// SYNTHETIC: SWBF2 0x0052e440
// galaxy::api::IGalaxyListener::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052e4d0
void InitializeGalaxyInstance()
{
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::CreateInstance();
    pGalaxy->Init("48393168095086872",
                            "2eb6e8e813464e74a4854c94648b12f07c4a2e2446dfcac4bd83d782c580b31a",
                            ".", false);
}

// FUNCTION: SWBF2 0x0052e4f0
void ShutdownGalaxyInstance()
{
    if (galaxy::api::GalaxyFactory::GetInstance())
    {
        galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
        pGalaxy->Shutdown();
        galaxy::api::GalaxyFactory::ResetInstance();
    }
}

// FUNCTION: SWBF2 0x0052e510
void* GetGalaxyUserInterface()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->User();
}

// FUNCTION: SWBF2 0x0052e530
void* GetGalaxyFriendsInterface()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Friends();
}

// FUNCTION: SWBF2 0x0052e550
void* GetGalaxyLastError()
{
    galaxy::api::IErrorManager* pErrors = galaxy::api::GalaxyFactory::GetErrorManager();
    return pErrors->GetLastError();
}

namespace
{

inline bool SteamUser_BLoggedOn()
{
    ISteamUser* pUser = SteamUser();
    return pUser->BLoggedOn();
}

class EncryptedAppTicketDownloader
{
public:
    EncryptedAppTicketDownloader(std::string& ticket, int& eResult);
    void OnEncryptedAppTicketResponse(EncryptedAppTicketResponse_t* pResponse, bool bIOFailure);

    CCallResult<EncryptedAppTicketDownloader, EncryptedAppTicketResponse_t> m_callResult;
    std::string* m_pTicket;
    int* m_peResult;
    bool m_bDone;
};

 
 

// FUNCTION: SWBF2 0x0052e830
EncryptedAppTicketDownloader::EncryptedAppTicketDownloader(std::string& ticket, int& eResult)
    : m_pTicket(&ticket), m_peResult(&eResult), m_bDone(false)
{
    if (SteamUser_BLoggedOn())
    {

        ISteamUser* pUser = SteamUser();
        const SteamAPICall_t hCall = pUser->RequestEncryptedAppTicket(0, 0);
        m_callResult.Set(hCall, this, &EncryptedAppTicketDownloader::OnEncryptedAppTicketResponse);
    }
    else
    {
        *m_peResult = 2;
        m_bDone = true;
    }
}

// FUNCTION: SWBF2 0x0052e930
void EncryptedAppTicketDownloader::OnEncryptedAppTicketResponse(EncryptedAppTicketResponse_t* pResponse, bool bIOFailure)
{
    m_bDone = true;
    if (bIOFailure)
    {
        return;
    }
    if (pResponse->m_eResult == 1)
    {

        unsigned char acTicket[1024];

        unsigned int uSize = 0;
        ISteamUser* pUser = SteamUser();
        pUser->GetEncryptedAppTicket(acTicket, sizeof(acTicket), &uSize);
        m_pTicket->reserve(uSize + 1);
        m_pTicket->assign((const char*)acTicket, uSize);
    }
    else
    {
        *m_peResult = pResponse->m_eResult;
    }
}

// TEMPLATE: SWBF2 0x0052ff30
// `anonymous namespace'::CCallResult<`anonymous namespace'::EncryptedAppTicketDownloader,`anonymous namespace'::EncryptedAppTicketResponse_t>::Run

// TEMPLATE: SWBF2 0x0052ff70
// `anonymous namespace'::CCallResult<`anonymous namespace'::EncryptedAppTicketDownloader,`anonymous namespace'::EncryptedAppTicketResponse_t>::Run

// TEMPLATE: SWBF2 0x0052f440
// `anonymous namespace'::CCallResult<`anonymous namespace'::EncryptedAppTicketDownloader,`anonymous namespace'::EncryptedAppTicketResponse_t>::GetCallbackSizeBytes

class GalaxyFriendListListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::IFriendListListener>
{
public:
    // FUNCTION: SWBF2 0x0052ea00
    virtual void OnFriendListRetrieveSuccess()
    {
        gFriendListDirty = true;
        gFriendsStarted = true;
    }

    // FUNCTION: SWBF2 0x0052ea10
    virtual void OnFriendListRetrieveFailure(int failureReason)
    {
        gFriendsStarted = false;
    }
};

class GalaxyGameJoinRequestedListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::IGameJoinRequestedListener>
{
public:
    // FUNCTION: SWBF2 0x0052ea20
    virtual void OnGameJoinRequested(galaxy::api::GalaxyID userID, const char* connectionString)
    {
        if (connectionString)
        {
            gFriendConnectionStrings[userID.value] = connectionString;
        }
    }
};

// SYNTHETIC: SWBF2 0x005301b0
// `anonymous namespace'::GalaxyFriendListListener::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00530230
// `anonymous namespace'::GalaxyGameJoinRequestedListener::`scalar deleting destructor'

}

// SYNTHETIC: SWBF2 0x0052f450
// galaxy::api::SelfRegisteringListener<galaxy::api::IFriendListListener>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0052f4e0
// galaxy::api::SelfRegisteringListener<galaxy::api::IGameJoinRequestedListener>::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x0052ffa0
// gf::Allocate<`anonymous namespace'::GalaxyFriendListListener>

// TEMPLATE: SWBF2 0x00530040
// gf::Allocate<`anonymous namespace'::GalaxyGameJoinRequestedListener>

// TEMPLATE: SWBF2 0x005302b0
// std::_To_xtime<__int64,std::ratio<1,1000> >

// FUNCTION: SWBF2 0x0052eab0
gf::SteamAuthProvider* gf::_sap()
{
    if (!gSteamAuthProvider)
    {
        gSteamAuthProvider = new SteamAuthProvider;
    }
    return gSteamAuthProvider;
}

// FUNCTION: SWBF2 0x0052eae0
void __cdecl OnSteamWarning(int, const char*) {}

// FUNCTION: SWBF2 0x0052eaf0
bool gf::SteamAuthProvider::Init(unsigned int uiAppId, bool& bRestarting)
{
    m_bInitialized = false;
    bRestarting = false;
    if (SteamAPI_RestartAppIfNecessary(uiAppId))
    {
        bRestarting = true;
        return false;
    }
    if (!SteamAPI_Init())
    {
        return false;
    }

    ISteamUtils* pUtils = SteamUtils();
    pUtils->SetOverlayNotificationPosition(2);
    pUtils = SteamUtils();
    pUtils->SetWarningMessageHook(OnSteamWarning);

    m_bInitialized = true;
    return true;
}

// FUNCTION: SWBF2 0x0052eb70
void gf::SteamAuthProvider::Shutdown()
{
    SteamAPI_Shutdown();
}

// FUNCTION: SWBF2 0x0052eb80
int gf::SteamAuthProvider::GetEncryptedAppTicket(char* pTicket, int iSize)
{
    if (!SteamUser_BLoggedOn())
    {
        return 0;
    }

    gEncryptedAppTicket.clear();
    EncryptedAppTicketDownloader downloader(gEncryptedAppTicket, m_eResult);
    while (!downloader.m_bDone)
    {
        SteamAPI_RunCallbacks();
        if (downloader.m_bDone)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!gEncryptedAppTicket.size() || (unsigned int)iSize < gEncryptedAppTicket.size())
    {
        return 0;
    }
    return (int)gEncryptedAppTicket.copy(pTicket, gEncryptedAppTicket.size());
}

// FUNCTION: SWBF2 0x0052ed50
const char* gf::SteamAuthProvider::GetPersonaName()
{
    if (!SteamUser_BLoggedOn() || !SteamFriends())
    {
        return 0;
    }
    ISteamFriends* pFriends = SteamFriends();
    return pFriends->GetPersonaName();
}

// FUNCTION: SWBF2 0x0052e9e0
void __fastcall FreeGalaxyGlueObject(void* pBlock)
{
    RedFreeToHeap(GameMemory::GamespyHeap, pBlock, 0);
}

// GLOBAL: SWBF2 0x01e55fb5
bool gGalaxyInitialized;

// FUNCTION: SWBF2 0x0052eda0
bool gf::isInitialized()
{
    return gGalaxyInitialized;
}

// FUNCTION: SWBF2 0x0052edb0
bool gf::checkError()

{
    galaxy::api::IError* pError = (galaxy::api::IError*)GetGalaxyLastError();
    if (!pError)
    {
        return false;
    }
    pError->GetMsg();
    return true;
}

// FUNCTION: SWBF2 0x0052edd0
bool gf::initialize()
{
    gGalaxyInitialized = false;
    if (!gSteamAuthProvider || !gSteamAuthProvider->IsInitialized())
    {
        return false;
    }

    InitializeGalaxyInstance();
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        gGalaxyInitialized = false;
        return false;
    }
    if (checkError())
    {
        gGalaxyInitialized = false;
        return false;
    }

    gGalaxyInitialized = true;
    gFriendListListener = Allocate<GalaxyFriendListListener>();
    gGameJoinRequestedListener = Allocate<GalaxyGameJoinRequestedListener>();
    return gGalaxyInitialized;
}

// FUNCTION: SWBF2 0x0052ee40
void gf::deinitialize()
{
    if (gFriendListListener)
    {
        FreeGalaxyGlueObject(gFriendListListener);
        gFriendListListener = 0;
    }
    if (gGameJoinRequestedListener)
    {
        FreeGalaxyGlueObject(gGameJoinRequestedListener);
        gGameJoinRequestedListener = 0;
    }
    if (gGalaxyInitialized)
    {
        if (gSteamAuthProvider)
        {
            gSteamAuthProvider->Shutdown();
            if (gSteamAuthProvider)
            {
                FreeGalaxyGlueObject(gSteamAuthProvider);
                gSteamAuthProvider = 0;
            }
        }
        ShutdownGalaxyInstance();
        gGalaxyInitialized = false;
    }
}

// FUNCTION: SWBF2 0x0052eeb0
const char* gf::utils::ToString(bool bValue)
{
    return bValue ? "1" : "0";
}

// FUNCTION: SWBF2 0x0052eed0
const char* gf::utils::ToString(int iValue)
{
    sprintf(gSignedStringBuffer, "%d", iValue);
    return gSignedStringBuffer;
}

// FUNCTION: SWBF2 0x0052eef0
const char* gf::utils::ToString(unsigned int uValue)
{
    sprintf(gUnsignedStringBuffer, "%u", uValue);
    return gUnsignedStringBuffer;
}

// FUNCTION: SWBF2 0x0052ef10
bool gf::utils::FromString(const char* pcValue, bool bDefault)
{
    if (!strcmp(pcValue, "1"))
    {
        return true;
    }
    if (!strcmp(pcValue, "0"))
    {
        return false;
    }
    return bDefault;
}

// FUNCTION: SWBF2 0x0052efa0
int gf::utils::FromString(const char* pcValue, int iDefault)
{
    sscanf(pcValue, "%d", &iDefault);
    return iDefault;
}

// FUNCTION: SWBF2 0x0052efd0
unsigned int gf::utils::FromString(const char* pcValue, unsigned int uDefault)
{
    sscanf(pcValue, "%u", &uDefault);
    return uDefault;
}

// GLOBAL: SWBF2 0x01e55fb6
bool gFriendsStarted;

// FUNCTION: SWBF2 0x0052f000
bool gf::friends::start()
{
    gFriendsStarted = false;
    if (!gf::isInitialized())
    {
        return false;
    }

    typedef bool(__thiscall * BoolGetter)(void* pThis);
    typedef void(__thiscall * VoidCall)(void* pThis);

    void* pFirst = GetGalaxyUserInterface();
    if (!(*(BoolGetter*)(*(char**)pFirst + 0x4))(pFirst))
    {
        return false;
    }

    void* pSecond = GetGalaxyFriendsInterface();
    (*(VoidCall*)(*(char**)pSecond + 0x34))(pSecond);
    return !gf::checkError();
}

// FUNCTION: SWBF2 0x0052f040
void gf::friends::stop()
{
    gFriendsStarted = false;
}

// FUNCTION: SWBF2 0x0052f050
void gf::friends::reset()
{
    stop();
    start();
}

// FUNCTION: SWBF2 0x0052f060
unsigned int gf::friends::friendCount()
{
    if (!gf::isInitialized() || !gFriendsStarted)
    {
        return 0;
    }
    void* pFriends = GetGalaxyFriendsInterface();
    return ((galaxy::api::IFriends*)pFriends)->GetFriendCount();
}

// FUNCTION: SWBF2 0x0052f090
galaxy::api::GalaxyID gf::friends::friendByIndex(unsigned int uIndex)
{
    galaxy::api::GalaxyID id;
    if (gf::isInitialized() && gFriendsStarted)
    {
        void* pFriends = GetGalaxyFriendsInterface();

        return ((galaxy::api::IFriends*)pFriends)->GetFriendByIndex(uIndex);
    }
    id.value = 0;
    return id;
}

// FUNCTION: SWBF2 0x0052f0e0
const char* gf::friends::friendNameByIndex(unsigned int uIndex)
{
    galaxy::api::GalaxyID id = friendByIndex(uIndex);
    if (!id.value)
    {
        return "";
    }
    void* pFriends = GetGalaxyFriendsInterface();
    return ((galaxy::api::IFriends*)pFriends)->GetFriendPersonaName(id);
}

// GLOBAL: SWBF2 0x01e55fb4
bool gFriendListDirty;

// FUNCTION: SWBF2 0x0052f130
bool gf::friends::getAndResetDirty()
{
    const bool bDirty = gFriendListDirty;
    gFriendListDirty = false;
    return bDirty;
}

// FUNCTION: SWBF2 0x0052f140
bool gf::friends::isFriend(const galaxy::api::GalaxyID& id)
{
    for (unsigned int i = 0; i < friendCount(); ++i)
    {
        if (friendByIndex(i).value == id.value)
        {
            return true;
        }
    }
    return false;
}

// TEMPLATE: SWBF2 0x0052f8b0
// std::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >::equal_range

// TEMPLATE: SWBF2 0x0052f800
// std::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >::find

// TEMPLATE: SWBF2 0x005300e0
// std::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >::emplace

// TEMPLATE: SWBF2 0x00530350
// std::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >::_Insert

// TEMPLATE: SWBF2 0x005304e0
// std::_Hash<std::_Umap_traits<unsigned __int64,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::_Uhash_compare<unsigned __int64,std::hash<unsigned __int64>,std::equal_to<unsigned __int64> >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,0> >::_Check_size

// TEMPLATE: SWBF2 0x00530490
// std::list<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >::erase

// TEMPLATE: SWBF2 0x005305a0
// std::_List_buy<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<unsigned __int64 const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >::_Buynode<std::piecewise_construct_t const &,std::tuple<unsigned __int64 const &>,std::tuple<> >

// FUNCTION: SWBF2 0x0052f1a0
bool gf::friends::isJoinableByIndex(unsigned int index)
{
    const galaxy::api::GalaxyID id = friendByIndex(index);
    if (!id.value)
    {
        return false;
    }
    const std::pair<std::unordered_map<unsigned __int64, std::string>::iterator,
                    std::unordered_map<unsigned __int64, std::string>::iterator> range =
        gFriendConnectionStrings.equal_range(id.value);
    return range.first != range.second;
}

// FUNCTION: SWBF2 0x0052f210
const char* gf::friends::connectionStringByIndex(unsigned int index)
{
    const galaxy::api::GalaxyID id = friendByIndex(index);
    if (!id.value)
    {
        return 0;
    }
    const std::pair<std::unordered_map<unsigned __int64, std::string>::iterator,
                    std::unordered_map<unsigned __int64, std::string>::iterator> range =
        gFriendConnectionStrings.equal_range(id.value);
    if (range.first == range.second)
    {
        return 0;
    }
    std::unordered_map<unsigned __int64, std::string>::iterator entry =
        gFriendConnectionStrings.find(id.value);
    if (entry == gFriendConnectionStrings.end())
    {
        return gFriendConnectionStrings[id.value].c_str();
    }
    return entry->second.c_str();
}

// LIBRARY: SWBF2 0x0052e560
// `anonymous namespace'::SteamInternal_OnContextInit

// LIBRARY: SWBF2 0x0052e580
// `anonymous namespace'::CSteamAPIContext::Clear

// LIBRARY: SWBF2 0x0052e610
// `anonymous namespace'::CSteamAPIContext::Init
