#pragma once

#include <stddef.h>

#include "GameMusic.h"

class VehicleMusicClass : public GameMusic
{
public:
    VehicleMusicClass();

    VehicleMusicClass(const VehicleMusicClass* pBase);
    bool SetProperty(unsigned int uProperty, const char* pszValue);

    GameMusic mMusic[8];
    float mMusicSpeed;
    float mMusicDelay;
};
 
 
 
 

class VehicleMusic
{
public:
    VehicleMusic(const VehicleMusicClass* pClass);
    ~VehicleMusic();

    void Update(const VehicleMusicClass* pClass, float fDeltaTime, bool bPiloted, float fSpeed, int iSide);

    float mTimeWithPilot;
    GameMusic mMusic[8];
};
 
 
