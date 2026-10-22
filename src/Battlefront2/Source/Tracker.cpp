#include "Tracker.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "Trackable.h"
#include "CameraManager.h"
#include "Controllable.h"

extern int netCurMaxPlayers;

// FUNCTION: SWBF2 0x0065aa20
Tracker::Tracker()
{
    mTrackable = 0;
    mIsFirstPersonView = false;
    mIsSpectating = false;
    sAllInstances.AddLast(&m_AllListNode, this);
}

// FUNCTION: SWBF2 0x0065aa80
Tracker::~Tracker()
{
    sAllInstances.Remove(&m_AllListNode);
}

extern Tracker (*sInstances)[64];

// FUNCTION: SWBF2 0x0065acb0
Tracker* Tracker::GetTracker(int iIndex)
{
    return &(*sInstances)[iIndex];
}

// FUNCTION: SWBF2 0x0065acc0
void Tracker::RemoveTrackable(Trackable* pTrackable)
{
    int iCount = NetGame::IsEnabled() ? netCurMaxPlayers : (int)GameLoop::GetNumCameras();
    for (int i = 0; i < iCount; ++i)
    {
        if ((*sInstances)[i].mTrackable == pTrackable)
        {
            (*sInstances)[i].mTrackable = 0;
        }
    }
    pTrackable->SetTracker(0);
}

// GLOBAL: SWBF2 0x007ebc9c
PblList<Tracker> Tracker::sAllInstances;

// GLOBAL: SWBF2 0x01f99a68
// SYNTHETIC: SWBF2 0x00406ae0
// `dynamic initializer for 'sHostInstances''
// SYNTHETIC: SWBF2 0x00769600
// `dynamic atexit destructor for 'sHostInstances''
Tracker sHostInstances[64];
// GLOBAL: SWBF2 0x01f99468
// SYNTHETIC: SWBF2 0x00406b10
// `dynamic initializer for 'sClientInstances''
// SYNTHETIC: SWBF2 0x00769620
// `dynamic atexit destructor for 'sClientInstances''
Tracker sClientInstances[64];

// GLOBAL: SWBF2 0x007e9ee0
Tracker (*sInstances)[64] = &sHostInstances;

extern bool netIsOneWorld;

// FUNCTION: SWBF2 0x0065aab0
void Tracker::EnterClient()
{
    if (!netIsOneWorld)
    {
        sInstances = &sClientInstances;
    }
}

// FUNCTION: SWBF2 0x0065aad0
void Tracker::LeaveClient()
{
    if (!netIsOneWorld)
    {
        sInstances = &sHostInstances;
    }
}

// FUNCTION: SWBF2 0x0065ad20
void Tracker::CreateClient()
{
}

// FUNCTION: SWBF2 0x0065ac10
void Tracker::SetDeathTrackable(Trackable* pTrackable, int iPlayer, Character* pKiller)
{
    if (iPlayer < 0)
    {
        return;
    }
    Tracker& rTracker = (*sInstances)[iPlayer];
    if (rTracker.mTrackable && rTracker.mTrackable->mTracker == &rTracker)
    {
        rTracker.mTrackable->SetTracker(0);
    }
    rTracker.mTrackable = pTrackable;
    if (pTrackable)
    {
        pTrackable->SetTracker(&rTracker);
    }
    int iJoystick = NetGame::GetJoystickIndex(iPlayer);
    if (iJoystick >= 0 &&
        (!NetGame::IsEnabled() || NetGame::IsClient() ||
         (NetGame::IsOneWorld() && NetGame::IsHost())))
    {
        CameraManager::sInstance->TrackDeath(pTrackable, iJoystick, pKiller);
    }
}

// FUNCTION: SWBF2 0x0065aaf0
void Tracker::SetTrackable(Trackable* pTrackable, int iPlayer)
{
    if (iPlayer < 0)
    {
        return;
    }
    Tracker& rTracker = (*sInstances)[iPlayer];
    if (rTracker.mTrackable)
    {
        Tracker* pOld = rTracker.mTrackable->mTracker;
        if ((unsigned int)pOld != 0xDDDDDDDDu && pOld && rTracker.IsFirstPersonView())
        {
            rTracker.mTrackable->SetFirstPersonView(iPlayer, false);
        }
        if (pOld == &rTracker)
        {
            rTracker.mTrackable->SetTracker(0);
        }
    }
    rTracker.mTrackable = pTrackable;
    if (pTrackable)
    {
        pTrackable->SetTracker(&rTracker);
    }
    int iJoystick = NetGame::GetJoystickIndex(iPlayer);
    if (iJoystick >= 0 &&
        (!NetGame::IsEnabled() || NetGame::IsClient() || (NetGame::IsOneWorld() && NetGame::IsHost())))
    {
        CameraManager::sInstance->Chase(pTrackable, iJoystick);
    }
    if (!pTrackable)
    {
        rTracker.SetFirstPersonView(iPlayer, false);
    }
    if (rTracker.IsFirstPersonView())
    {
        rTracker.mTrackable->SetFirstPersonView(iPlayer, true);
    }
}

// FUNCTION: SWBF2 0x0065ad60
void Tracker::SetFirstPersonView(int iPlayer, bool bFirstPerson)
{
    if (mTrackable)
    {
        Controllable* pCtrl = mTrackable->GetControllable();
        unsigned int uForce = pCtrl->GetControllableClass()->mForceMode;
        if (uForce != 0 && uForce <= 2)
        {
            return;
        }
    }
    mIsFirstPersonView = bFirstPerson;
    if (mTrackable)
    {
        mTrackable->SetFirstPersonView(iPlayer, bFirstPerson);
    }
}

// FUNCTION: SWBF2 0x0065ad30
void Tracker::Cleanup()
{
    for (PblList<Tracker>::Itor it(sAllInstances); it != sAllInstances.End(); ++it)
    {
        it->mTrackable = 0;
    }
}
