#include <stdio.h>

#include "VehicleMusic.h"
#include "Team.h"

// FUNCTION: SWBF2 0x0066e660
VehicleMusicClass::VehicleMusicClass()
{
    mMusicSpeed = 0.0f;
    mMusicDelay = 0.0f;
}

// FUNCTION: SWBF2 0x0066e510
VehicleMusic::VehicleMusic(const VehicleMusicClass* pClass)
{
    mTimeWithPilot = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mMusic[i] = pClass->mMusic[i];
    }
}

// FUNCTION: SWBF2 0x0066e570
VehicleMusic::~VehicleMusic()
{
    for (int i = 0; i < 8; ++i)
    {
        mMusic[i].Stop();
    }
}

// FUNCTION: SWBF2 0x0066e5a0
void VehicleMusic::Update(const VehicleMusicClass* pClass, float fDeltaTime, bool bPiloted, float fSpeed, int iSide)
{
    GameMusic& music = mMusic[iSide];
    if (!bPiloted)
    {
        if (music.GetPlayState() == GameMusic::playState_playing)
        {
            music.Stop();
        }
        mTimeWithPilot = 0.0f;
        return;
    }

    if (fSpeed < pClass->mMusicSpeed)
    {
        if (music.GetPlayState() == GameMusic::playState_playing &&
            music.GetCurrentPlayState() == GameMusic::playState_stopped)
        {
            music.Stop();
        }
    }
    else if (mTimeWithPilot >= pClass->mMusicDelay &&
             music.GetPlayState() == GameMusic::playState_stopped)
    {
        music.Play();
    }
    mTimeWithPilot += fDeltaTime;
}

// FUNCTION: SWBF2 0x0066e710
bool VehicleMusicClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x9f9c4fd4:
        for (int i = 0; i < 8; ++i)
        {
            mMusic[i].SetID(pszValue);
        }
        return true;
    case 0x4232e9bf:
        mMusic[Team::side_Alliance].SetID(pszValue);
        return true;
    case 0xcedc7e88:
        mMusic[Team::side_Empire].SetID(pszValue);
        return true;
    case 0x9347d123:
        mMusic[Team::side_Republic].SetID(pszValue);
        return true;
    case 0xaf8e60ed:
        mMusic[Team::side_CIS].SetID(pszValue);
        return true;
    case 0x4defb65f:
        sscanf(pszValue, "%f", &mMusicSpeed);
        return true;
    case 0x1784e0a3:
        sscanf(pszValue, "%f", &mMusicDelay);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0066e6a0
VehicleMusicClass::VehicleMusicClass(const VehicleMusicClass* pBase)
{
    for (int i = 0; i < 8; ++i)
    {
        mMusic[i] = pBase->mMusic[i];
    }
    mMusicSpeed = pBase->mMusicSpeed;
    mMusicDelay = pBase->mMusicDelay;
}
