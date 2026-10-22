#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

class Trackable;

class Character;

class Tracker
{
public:
    Tracker();
    ~Tracker();

    static Tracker* GetTracker(int iIndex);

    static void SetTrackable(Trackable* pTrackable, int iPlayer);

    static void SetDeathTrackable(Trackable* pTrackable, int iPlayer, Character* pKiller);

    bool IsFirstPersonView();

    void SetFirstPersonView(int iPlayer, bool bFirstPerson);

    static void EnterClient();
    static void LeaveClient();

    static void CreateClient();

    PblList<Tracker>::Node m_AllListNode;

    // FUNCTION: SWBF2 0x004294f0
    Trackable* GetTrackable() const { return mTrackable; }

    Trackable* mTrackable;
    bool mIsFirstPersonView;
    bool mIsSpectating;

    static PblList<Tracker> sAllInstances;
    static void Cleanup();

    static void RemoveTrackable(Trackable* pTrackable);
};
 
 
 
 
