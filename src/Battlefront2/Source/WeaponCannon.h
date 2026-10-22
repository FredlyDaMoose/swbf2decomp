#pragma once

#include <stddef.h>

#include "GameSound.h"
#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"

class OrdnanceClass;

// VTABLE: SWBF2 0x007b0674
class WeaponCannonClass : public WeaponClass
{
public:
    WeaponCannonClass(unsigned int uHash);

    WeaponCannonClass(WeaponCannonClass* pBase, unsigned int uHash);

    struct ShotPattern
    {
        float mPitch;
        float mYaw;
    };

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetHeatPerSalvo();
    virtual float GetShotsPerSalvo();
    virtual int GetBarrageMin();
    virtual int GetBarrageMax();

    static PblHash rttiHashWeaponCannonClass;
    static char rttiNameWeaponCannonClass[];

    float mMaxPressedTime;
    float mShotDelayMin;
    float mShotDelayMax;
    float mPitchSpread;
    float mYawSpread;
    float mStanceSpread[3][3];
    float mSpreadPerShot;
    float mSpreadRecoverRate;
    float mSpreadThreshold;
    float mSpreadLimit;
    float mKickStrength;
    float mKickBuildup;
    int mSalvoCount;
    int mShotsPerSalvo;
    float mSalvoDelayMin;
    float mSalvoDelayMax;
    float mInitialSalvoDelay;
    ShotPattern* mPatternArray;
    int mPatternCount;
    float mWarmUpTime;
    float mCoolDownTime;
    GameSound mWarmUpSound;
    float mWarmUpSoundStartPitch;
    float mWarmUpSoundFadeInTime;
    float mWarmUpSoundFadeOutTime;
    GameSound mBarrelSound;
    float mBarrelSoundFadeInTime;
    float mBarrelSoundFadeOutTime;
    GameSound mCoolDownSound;
    float mCoolDownSoundEndPitch;
    float mCoolDownSoundFadeInTime;
    float mCoolDownSoundFadeOutTime;
    OrdnanceClass* mSecondaryOrdnanceClass;
    int mSecondaryOrdnancePeriod;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
