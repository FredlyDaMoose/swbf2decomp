#pragma once

#include <stddef.h>

#include "GameSound.h"
#include "Thread.h"
#include "WeaponCannon.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

class Aimer;
class AmmoCounter;
class Controllable;
class EnergyBar;
class GameObject;
class MemoryPool;
class NetPktGroup;
class Ordnance;
class ParticleEmitterObject;
class RedColor;
class RedPose;
class Trigger;
struct OrdnanceDesc;
struct WeaponDesc;

class Weapon : public Thread
{
public:
    enum WeaponState
    {
        IDLE = 0,
        FIRE = 1,
        FIRE2 = 2,
        CHARGE = 3,
        RELOAD = 4,
        OVERHEAT = 5,
        EMPTY = 6
    };

    Weapon(const WeaponClass* pClass, const WeaponDesc& desc);
    virtual ~Weapon();
    virtual bool Update(float fDeltaTime);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Init();
    virtual float GetOrdnanceVelocity() const;
    virtual float GetOrdnanceGravity() const;
    virtual bool IsLocked() const;
    virtual const GameObject* GetLocked() const;
    virtual int GetLockedTargetBodyId() const;
    virtual void SetKickSpreadFactor(float fFactor);
    virtual void SetYawSpreadFactor(float fFactor);
    virtual float GetYawSpread() const;
    virtual float GetPitchSpread() const;
    virtual bool Deflect(const Ordnance* pOrdnance, const PblVector3& pos,
                         const PblVector3& dir);
    virtual void SignalFire();
    virtual bool ShouldShowReticule() const;
    virtual bool IsMelee() const;
    virtual bool IsMeleeThrow() const;
    virtual void NotifySoldierState(int iState, float fTime, bool b0, bool b1,
                                    const PblVector3& vec);
    virtual bool SoldierCanOperate() const;
    virtual bool OverrideSoldierVelocity(float& f0, float& f1, float& f2,
                                         float& f3) const;
    virtual bool OverrideSoldierControls();
    virtual bool OverrideSoldierEnergyRestore(float& fRestore) const;
    virtual bool OverrideAimer();
    virtual void Select(unsigned int uFlags, bool bOffhand);
    virtual void Deselect(unsigned int uFlags, bool bOffhand);
    virtual bool IsBusy() const;
    virtual bool WillBecomeBusy() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
    virtual void Render(const PblMatrix& mat, const RedPose* pPose,
                        const RedColor& color, unsigned int uFlags,
                        bool bArg) const;
    virtual unsigned int GetNameHash() const;

protected:
    virtual void EnterIdle();
    virtual bool UpdateIdle(float fDeltaTime);
    virtual void ExitIdle();
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual void EnterCharge();
    virtual bool UpdateCharge(float fDeltaTime);
    virtual void ExitCharge();
    virtual void EnterReload();
    virtual bool UpdateReload(float fDeltaTime);
    virtual void ExitReload();
    virtual void EnterEmpty();
    virtual bool UpdateEmpty(float fDeltaTime);
    virtual void ExitEmpty();
    virtual bool CheckCharge() const;
    virtual bool CheckFire() const;
    virtual bool FireDone() const;
    virtual bool CheckEmpty() const;
    virtual bool ForceReload() const;
    virtual bool CheckReload() const;
    virtual bool ReloadDone() const;

    void PlayFireSound(bool bArg);
    void StopFireSound();

public:
    bool CheckOverheat() const;

    void SetSkip(bool bSkip);

    void ResetSkip();

    void UpdateZoom(float fDeltaTime, float fZoomInput);
    bool CycleZoom(bool bCycle);
    bool ZoomFirstPerson() const;
    int GetSoldierAnimationWeapon() const;
    float GetSoundRadius() const;
    void PlayFoleyFX(unsigned int uIndex);
    void ActivatePhysics();
    void DeactivatePhysics();

    Controllable* GetOwner() const;

    float GetHeat() const;
    float GetStateTimer() const { return mStateTimer; }
    const Trigger* GetTrigger() const { return mTrigger; }

    bool IsSelected() const { return mSelectedFlag; }

    const WeaponClass* GetClass() const { return mClass; }
    bool GetFiredFlag() const { return mFiredFlag != 0; }
    void ClearFiredFlag() { mFiredFlag = 0; }
    WeaponState GetState() const { return mState; }

    static PblHash rttiHashWeapon;
    static char rttiNameWeapon[];

    static void operator delete(void* p);

    static void* operator new(size_t uSize);
    static MemoryPool sMemoryPool;

    PblMatrix mFirePointMatrix;
    WeaponClass* mStart;
    WeaponClass* mClass;
    WeaponClass* mRenderClass;
    Controllable* mOwner;
    Aimer* mAimer;
    Trigger* mTrigger;
    Trigger* mReload;
    PblVector3 mFirePos;
    AmmoCounter* m_pAmmoCounter;
    EnergyBar* m_pEnergyBar;
    float mCurChargeStrengthHeavy;
    float mCurChargeStrengthLight;
    float mCurChargeDelayHeavy;
    float mCurChargeDelayLight;
    float mCurChargeRateLight;
    float mCurChargeRateHeavy;
    float mCurTimeAtMaxCharge;
    unsigned int mHideWeapon : 1;
    unsigned int mFiredFlag : 1;
    unsigned int mSelectedFlag : 1;
    int m_iSoldierState : 6;
    WeaponState mState;
    float mStateTimer;
    float mStateLimit;
    float mZoom;
    float mZoomTurnScale;
    float mMuzzleFlashStartTime;
    int mSoldierAnimationMap;
    GameSoundControllable mSoundControl;
    GameSoundControllable mSoundControlFire;
    GameSoundControllable mFoleyFXControl;
    GameSound mSoundProps;
    GameSound mSoundPropsFire;
    GameSound mFoleyFXProps;
    PblHandle<ParticleEmitterObject> mChargeUpEmitter;
    float mLastFireTime;
    float mSkipTime;
    bool mSkip : 1;
    bool mCharged : 1;
    unsigned char mDeactivateScheduled;
    PblHandle<const GameObject> mTarget;
    int mTargetBodyID;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007b057c
class WeaponCannon : public Weapon
{
public:
    WeaponCannon(const WeaponCannonClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponCannon();
    virtual bool Update(float fDeltaTime);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetKickSpreadFactor(float fFactor);
    virtual void SetYawSpreadFactor(float fFactor);
    virtual float GetYawSpread() const;
    virtual float GetPitchSpread() const;
    virtual void NotifySoldierState(int iState, float fTime, bool b0, bool b1,
                                    const PblVector3& vec);
    virtual bool IsBusy() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual void EnterCharge();
    virtual bool UpdateCharge(float fDeltaTime);
    virtual void ExitCharge();
    virtual void EnterEmpty();
    virtual bool CheckCharge() const;
    virtual bool CheckFire() const;
    virtual bool FireDone() const;
    virtual bool CheckEmpty() const;
    virtual bool ReloadDone() const;
    virtual void SetOrdnance(Ordnance* pOrdnance);
    virtual Ordnance* CreateOrdnance(OrdnanceDesc& desc);

    bool Fire();
    void PlayWarmUpSound();
    void StopWarmUpSound();
    void UpdateWarmUpSound(float fDeltaTime);
    void PlayBarrelSound();
    void StopBarrelSound();
    void UpdateBarrelSound(float fDeltaTime);
    void PlayCoolDownSound();
    void StopCoolDownSound();
    void UpdateCoolDownSound(float fDeltaTime);

    static PblHash rttiHashWeaponCannon;
    static char rttiNameWeaponCannon[];

    float mStrength;
    int mSalvoCount;
    float mSalvoDelay;
    int mPatternIndex;
    float mKickSpread;
    float mKickSpreadFactor;
    float mYawSpreadFactor;
    float mStanceSpread;
    float mWarmCoolTimer;
    bool mWarmUp : 1;
    GameSoundControllable mSoundControlWarmUp;
    GameSound mSoundPropsWarmUp;
    GameSoundControllable mSoundControlBarrel;
    GameSound mSoundPropsBarrel;
    GameSoundControllable mSoundControlCoolDown;
    GameSound mSoundPropsCoolDown;
    int mOrdCounter;
};
  
 
 
 
 
 
 
 
 
