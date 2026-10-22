#pragma once

#include "PebbleFL/Common/PblTimeBase.h"

class LuaScript;

class ShellLoop
{
public:

    enum eLEGALSCREENS_SHOWTYPE
    {
        eLEGALSCREENS_SHOW = 0,
        eLEGALSCREENS_SHOWFAST = 1,
        eLEGALSCREENS_SKIP = 2,
    };

    static void SetLegalShowtype(eLEGALSCREENS_SHOWTYPE eType) { sLegalShowtype = eType; }
    static eLEGALSCREENS_SHOWTYPE GetLegalShowtype() { return sLegalShowtype; }

    static int Cleanup();

    static eLEGALSCREENS_SHOWTYPE sLegalShowtype;

    static LuaScript* ifScript;
    static int mLastGameNumCameras;
    static float sDeltaTime;
    static PblTimeBase::Stamp sMissionTime;

    static float GetMissionTime();
    static int Init();

    static int PostLoad();

    static int Update(float);

    static bool mbNetCableErrorOnEnter;

    static bool GetNetCableErrorOnEnter();

    static void SetNetCableErrorOnEnter(int iSeverity, const char* pcError);
    static unsigned int mbNetCableScreenHash;
    static int mbNetCableErrorTimer;
    static unsigned int mpNoCableMessageHash;
    static int sIngameErrorSeverity;

    static bool mDemoIsOver;
    static void SetDemoOver(bool bOver) { mDemoIsOver = bOver; }
public:
    static float clientInputAcc;
    static float clientRenderAcc;
    static unsigned int sSystemDeltaTime;
    static unsigned int clientTimePrev;
};
