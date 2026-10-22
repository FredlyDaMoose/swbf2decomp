#ifndef GF_GALAXY_H
#define GF_GALAXY_H

#include <new>

#include "Battlefront2/Source/GameMemory.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "galaxy_api.h"

namespace gf
{

bool initialize();
void deinitialize();
bool isInitialized();
bool checkError();

namespace utils
{
const char* ToString(bool bValue);
const char* ToString(int iValue);
const char* ToString(unsigned int uValue);
bool FromString(const char* pcValue, bool bDefault);
int FromString(const char* pcValue, int iDefault);
unsigned int FromString(const char* pcValue, unsigned int uDefault);
}

namespace friends
{

bool start();
void stop();
void reset();
unsigned int friendCount();
galaxy::api::GalaxyID friendByIndex(unsigned int uIndex);
const char* friendNameByIndex(unsigned int uIndex);
bool getAndResetDirty();
bool isFriend(const galaxy::api::GalaxyID& id);
bool isJoinableByIndex(unsigned int index);
const char* connectionStringByIndex(unsigned int index);
}

template <typename T>
T* Allocate()
{
    void* pMem = RedAllocFromHeap(GameMemory::GamespyHeap, sizeof(T), 0);
    if (!pMem)
    {
        return 0;
    }
    return new (pMem) T();
}

template <typename T, typename A1>
T* Allocate(A1&& a1)
{
    void* pMem = RedAllocFromHeap(GameMemory::GamespyHeap, sizeof(T), 0);
    if (!pMem)
    {
        return 0;
    }
    return new (pMem) T(a1);
}

class GalaxyAuthListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::IAuthListener>
{
public:
    GalaxyAuthListener(int iViewport) : m_iViewport(iViewport) {}

    virtual void OnAuthSuccess();
    virtual void OnAuthFailure(int failureReason);
    virtual void OnAuthLost();

private:
    int m_iViewport;
};

class GalaxyLobbyCreatedListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyCreatedListener>
{
public:
    virtual void OnLobbyCreated(const galaxy::api::GalaxyID& lobbyID, int result);
};

class GalaxyLobbyLeftListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyLeftListener>
{
public:
    virtual void OnLobbyLeft(const galaxy::api::GalaxyID& lobbyID, bool ioFailure);
};

class GalaxyLobbyMemberStateListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyMemberStateListener>
{
public:
    virtual void OnLobbyMemberStateChanged(const galaxy::api::GalaxyID& lobbyID,
                                           const galaxy::api::GalaxyID& memberID,
                                           int memberStateChange);
};

class GalaxyLobbyEnteredListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyEnteredListener>
{
public:
    virtual void OnLobbyEntered(const galaxy::api::GalaxyID& lobbyID, int result);
};

class GalaxyLobbyListListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyListListener>
{
public:
    virtual void OnLobbyList(unsigned int lobbyCount, bool ioFailure);
};

class GalaxyLobbyDataListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyDataListener>
{
public:
    virtual void OnLobbyDataUpdated(const galaxy::api::GalaxyID& lobbyID,
                                    const galaxy::api::GalaxyID& memberID);
};

class GalaxyLobbyOwnerChangeListener
    : public galaxy::api::SelfRegisteringListener<galaxy::api::ILobbyOwnerChangeListener>
{
public:
    virtual void OnLobbyOwnerChanged(const galaxy::api::GalaxyID& lobbyID,
                                     const galaxy::api::GalaxyID& newOwnerID);
};

}

#endif
