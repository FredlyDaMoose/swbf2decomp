#pragma once

#include <stddef.h>

#include "Thread.h"
#include "SoundFL/source/sndi3dl2reverb.h"

// VTABLE: SWBF2 0x007ad710
class PauseMenu : public Thread
{
public:
    PauseMenu();
    virtual ~PauseMenu();

    // SYNTHETIC: SWBF2 0x006163b0
    // PauseMenu::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    static void Init();
    static void Reset();
    static bool GetPaused(int iViewport);
    static void SetPaused(int iViewport, bool bPaused);
    static void SetFreeCamera(int iViewport, bool bFreeCamera);
    static int GetPausingJoystick();

    static int GetPausingViewport();
    static bool IsPauseAllowed();
    static bool AllStatsDone();
    static void SetStatVisible();
    static void SetStatsDone();
    static bool DoUpdate(float fDeltaTime);
    static void UpdateAllInstances(float fDeltaTime);

protected:
    void _Init(int iViewport);
    void _SetPauseAudio(bool bPause);
    void _SetPaused(bool bPaused);
    void _SetFreeCamera(bool bFreeCamera);
    void _SetStatsVisible();
    void _Reset();

public:
    bool mVisible;
    bool mFreecamView;
    bool mEntryStatusVisible;
    int mViewport;
    bool mFreecamSetting;
    float mDebounceTime;
    bool mShowStats;
    bool mStatsDone;
    Snd::I3DL2Reverb* mInGameReverb;
    Snd::I3DL2Reverb mPauseReverb;

    static PauseMenu* sPauseMenuInstance;
public:
    static bool sEnablePause;
    static int sPausingViewport;
    static bool sSkipToPlayerList;
    static bool sPauseDedicated;
};
 
 
 
 
