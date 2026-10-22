#pragma once

class PblFileChunk;

#include <stddef.h>

#include "AmmoCounter.h"
#include "EnergyBar.h"
#include "Factory.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class FLEffectClass;
class OrdnanceClass;
class RedModel;

struct WeaponAmmoDesc
{
    unsigned int uiAmmo : 24;
    unsigned int uiHeat : 5;
    unsigned int bFlagAmmoInfinite : 1;
    unsigned int bFlagShareHeat : 1;
    unsigned int bFlagShareAmmo : 1;
};
 

class Aimer;
class AmmoCounter;
class Controllable;
class EnergyBar;
class Trigger;

struct WeaponDesc
{
    Controllable* mOwner;
    Aimer* mAimer;
    Trigger* mTrigger;
    Trigger* mReload;
    int mNumClips;
    AmmoCounter* m_pAmmoCounter;
    EnergyBar* m_pEnergyBar;
};
 
 

// VTABLE: SWBF2 0x007b0170
class WeaponClass : public Factory<Weapon, WeaponClass, WeaponDesc>
{
public:

    enum Label
    {
        BLASTER = 0,
        LASER = 1,
        THROWN = 2,
        LAUNCHER = 3,
        MELEE = 4,
        SPECIAL = 5,
        NONE = 6
    };

    static void Read(PblFileChunk& chunk);

    enum FoleyFX
    {
        foleyFX_leftWalk = 0,
        foleyFX_rightWalk = 1,
        foleyFX_leftRun = 2,
        foleyFX_rightRun = 3,
        foleyFX_jump = 4,
        foleyFX_land = 5,
        foleyFX_roll = 6,
        foleyFX_prone = 7,
        foleyFX_squat = 8,
        foleyFX_stand = 9,
        foleyFX_max = 10
    };

    WeaponClass(unsigned int uHash);

    WeaponClass(WeaponClass* pBase, unsigned int uHash);
    virtual ~WeaponClass();
    virtual WeaponClass* Derive(unsigned int uHash);

    virtual bool IsRtti(unsigned int uRtti);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void PostReadInit();
    virtual bool PostLoadInit();

    void Render(const class PblMatrix& mMatrix, const class RedPose* pPose, const RedColor& color,
                unsigned int uiFlags, bool bArg) const;

    void RenderFlash(const PblVector3& vPos, const PblVector3& vDir, float fScale) const;
    virtual float GetHeatPerSalvo();
    virtual float GetShotsPerSalvo();
    virtual int GetBarrageMin();
    virtual int GetBarrageMax();

    void CreateMuzzleFlash(const PblMatrix& mFire) const;

    float GetBarrageDelay() const;

    static PblHash rttiHashWeaponClass;
    static char rttiNameWeaponClass[];

    static GameSound sWeaponChangeUnableSound;

    int mSoldierAnimationWeapon;
    PblVector3 mFirePointOffset;
    char mFilename[32];
    unsigned short* mLabel;
    unsigned int mNameHash;
    unsigned int mHUDNameHash;
    float mStrengthLimit;
    WeaponClass* mNextCharge;
    RedModel* mModel;
    RedModel* mHighResModel;
    unsigned int mIconTexture;
    unsigned int mReticuleTexture;
    unsigned int mScopeTexture;
    OrdnanceClass* mOrdnanceClass;
    float mRoundsPerShot;
    float mHeatPerShot;
    float mHeatRecoverRate;
    AmmoCounterClass mAmmoCounterClass;
    EnergyBarClass mEnergyBarClass;
    float mReloadTime;
    float mZoomMin;
    float mZoomMax;
    float mZoomRate;
    float mZoomTurnDivisorMin;
    float mZoomTurnDivisorMax;
    float mRecoilStrengthHeavy;
    float mRecoilStrengthLight;
    float mRecoilLengthLight;
    float mRecoilLengthHeavy;
    float mRecoilDelayLight;
    float mRecoilDelayHeavy;
    float mRecoilDecayLight;
    float mRecoilDecayHeavy;
    float mChargeRateLight;
    float mChargeRateHeavy;
    float mMaxChargeStrengthHeavy;
    float mMaxChargeStrengthLight;
    float mChargeDelayLight;
    float mChargeDelayHeavy;
    float mTimeAtMaxCharge;
    float mExtremeRange;
    float mLockOnRange;
    float mLockOnAngle;
    float mLockOffAngle;
    float mMinRange;
    float mOptimalRange;
    float mMaxRange;
    int mTargetSides;
    int mTargetTypes;
    int mAITargetTypes;
    GameSound mFireSound;
    GameSound mFireLoopSound;
    GameSound mFireEmptySound;
    GameSound mReloadSound;
    GameSound mChargeSound;
    float mChargeSoundPitch;
    GameSound mChargedSound;
    GameSound mWeaponChangeSound;
    GameSound mOverheatSound;
    float mOverheatSoundPitch;
    unsigned int mFireSoundStop;
    GameSound mFoleyFX[10];
    FLEffectClass* mChargeUpEffect;
    float mFlashLength;
    float mEnergyDrain;
    float mAutoPitchScreenDist;
    float mAutoTurnScreenDist;
    float mTargetLockMaxDistance;
    float mTargetLockMaxDistanceLose;
    short mScoreForMedalsType;
    short mMedalsTypeToUnlock;
    short mMedalsTypeToLock;
    unsigned int mFireAnim : 2;
    unsigned int mTriggerSingle : 1;
    unsigned int mZoomFirstPerson : 1;
    unsigned int mSniperScope : 1;
    unsigned int mMaxRangeDefault : 1;
    unsigned int mInstantPlayFireAnim : 1;
    unsigned int mIsOffhand : 1;

    bool mHashWeaponName : 1;
    bool mDisplayRefire : 1;
    bool mPostLoadInit : 1;
    bool mReticuleInAimingOnly : 1;
    bool mNoFirstPersonFireAnim : 1;
    bool mStopChargeSound : 1;
    bool mAIUseBubbleCircle : 1;
    bool mWarnedAboutLocalization : 1;
    float mAIBubbleSizeMultiplier;
    float mAIBubbleScaleDistDivider;
    float mAIBubbleScaleClamp;
    RedModel* mMuzzleFlashModel;
    FLEffectClass* mMuzzleFlashEffect;
    float mFlashRadius;
    float mFlashWidth;
    RedColor mFlashColor;
    RedColor mFlashLightColor;
    float mFlashLightRadius;
    float mFlashLightDuration;
    int mBarrageMin;
    int mBarrageMax;
    float mBarrageDelay;
    char mName[20];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
