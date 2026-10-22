#pragma once

struct NetAddr;
struct NetKey;
struct NetSessionId;
class NetPacket;

class NetMatch
{
public:

    static char* GetIPAddr();

    static char SetNoCableError();

    static void ClearGameError();

    static int GetPercentDone();

    static char __fastcall EndFindGames(bool bCancel);

    static void StartRegisterGame(bool bMatchService);

    static void StartLogin(int iViewport, bool bArg2, char const* pcArg3, char const* pcArg4,
                           char const* pcArg5, bool bArg6);

    static char __fastcall StartJoinGame(const NetSessionId* pId);

    static char EndJoinGame();

    static bool StartConnect();

    static bool UpdateConnect(const NetAddr* pAddr, const NetKey* pKey, int iDest, int* pReason);

    static void EndConnect();

    static bool IsDoneFindGames();

    static void UpdateFindGames();

    static bool PingServerByAddress();

    static bool GetPlayerDataByAddress();

    static unsigned int HostGetAddr(const NetAddr* pAddr, void* pOut, int iDest);

    static unsigned int __fastcall GetSessionId(NetSessionId* pId);

    static void SetLocalGSID(unsigned int lo, unsigned int hi);

    static unsigned __int64 GetLocalGSID();

    static void __fastcall ReceiveLanRegisterPacket(NetPacket* pPacket, const void* pSource);

public:

    static void EndRegisterGame();
    static void SetGameError(const char* message, int severity);
    static void SetRegisterArchFilter();

    static char const* GetLoginOverrideName(int iViewport);
    static bool UpdateLogin(int iViewport);
    static void UpdateRegisterGame();
    static bool IsLoginSuccessful(int iViewport, bool bArg);
    static bool UpdatePresence(bool bArg);
    static void GetLoginError(char const*& pcError, int& iSeverity);
    static void CancelLogin(int iViewport, bool bArg);
    static void GetGameError(char const*& pcError, int& iSeverity);

    static void ChangeRegisterGame();
    static void __fastcall ReceiveRegisterPacket(const char* pData, int iLen, const void* pSource);
    static char StartFindGames(int, const NetSessionId* id);
    static void PauseFindGames();
    static void UpdateJoinGame();
    static void HandleLoginError(bool);
    static void StartFindAddress();
    static void EndFindAddress();
    static bool UpdateFindAddress();
    static void ClientGetAddr();
    static int __fastcall SetAddr(unsigned int* pOut, const unsigned int* pIn);
};

bool __fastcall IsPrivateIP(unsigned long netAddr);
unsigned long GetPrivateIP();
void __fastcall HandleFindPacket(NetPacket* pPacket);
void __fastcall HandleFoundPacket(NetPacket* pPacket, const void* pSource);
void UpdateFindLanGames();

bool __fastcall IsSameNet(const unsigned int* pLeft, const unsigned int* pRight);

class NetPktGroup;
int __fastcall ReadSessionId(NetPktGroup* pGroup, unsigned int* pWords);
