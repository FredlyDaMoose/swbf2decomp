#pragma once

#include <stddef.h>

#include "CollisionObject.h"
#include "Damageable.h"
#include "Factory.h"
#include "FoleyFX.h"
#include "GameSound.h"
#include "Thread.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class ExplosionClass;
class FLEffectClass;
class FLEffectObject;
class GameObject;
class PblFileChunk;
class Weapon;

struct OrdnanceDesc
{
    // FUNCTION: SWBF2 0x005b40e0
    OrdnanceDesc()
        : mPosition(0.0f, 0.0f, 0.0f),
          mDirection(0.0f, 0.0f, 0.0f),
          mStrength(1.0f),
          mOwner(0),
          mDamageOwner(0, 0, 0, 0, 0, 1.0f),
          mTarget(0),
          mBodyID(-1),
          mFireWeapon(0)
    {
    }

    PblVector3 mPosition;
    PblVector3 mDirection;
    float mStrength;
    GameObject* mOwner;
    DamageOwner mDamageOwner;
    GameObject* mTarget;
    int mBodyID;
    Weapon* mFireWeapon;
};
 
 
 

// VTABLE: SWBF2 0x007ac44c
class OrdnanceClass : public Factory<Ordnance, OrdnanceClass, OrdnanceDesc>,
                      public FoleyFXColliderClass,
                      public FoleyFXCollideeClass
{
public:
    OrdnanceClass(unsigned int uHash);

    OrdnanceClass(OrdnanceClass* pBase, unsigned int uHash);

    static void Read(PblFileChunk& chunk);

    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetVelocity();
    virtual float GetGravity();
    virtual float GetRange();

    virtual DamageDesc GetDamage(const DamageOwner& owner, float fUnused, float fRatio,
                                 float fElapsed) const;

    DamageDesc GetDamage() const;

    float mStrengthLimit;
    OrdnanceClass* mNextCharge;
    float mLifeSpan;
    float mDamageTransitionDelay;
    float mDamageTransitionPeriod;
    float mDamageFinalDamage;
    float mBonusAmplification;
    RedColor mBonusColor;
    ExplosionClass* mExplosionExpire;
    ExplosionClass* mExplosionImpact;
    FLEffectClass* mImpactExplosionEffect;
    DamageDesc mDamage;
    FLEffectClass* mTrailEffect;
    GameSound mSoundOrdnance;
    GameSound mSoundCollision;
    GameSound mSoundCollisionWater;
    GameSound mSoundCollisionShield;

    static PblHash rttiHashOrdnanceClass;
    static char rttiNameOrdnanceClass[];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007ac40c
// VTABLE: SWBF2 0x007ac48c
class Ordnance : public Thread, public PblHandled, public FoleyFXCollider, public FoleyFXCollidee
{
protected:
    Ordnance(OrdnanceClass* pClass, const OrdnanceDesc& desc);

public:
    virtual ~Ordnance();

    virtual bool Update(float fDeltaTime) = 0;

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual const PblVector3& GetPosition() const;
    virtual const PblVector3& GetOldPosition() const;
    virtual const PblVector3& GetVelocity() const;
    virtual void KillOrdnance();
    virtual bool CheckCollision();
    virtual bool Collide(CollisionObject* pObject, const PblVector3& vPos,
                         const PblVector3& vNormal, int iBodyId);
    virtual void CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                                GameObject* pObject) const;

    GameObject* GetOwner() const;
    DamageDesc GetDamage();
    void AddOrdnanceToCollisionManager();

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }

    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow)
    {
        sMemoryPool.Setup(uCount, uSize, uGrow);
    }
    static void UpdateMemoryPoolSize(unsigned int uSize)
    {
        if (sMemoryPool.GetSize() < uSize)
        {
            sMemoryPool.SetSize(uSize);
        }
    }

    OrdnanceClass* mClass;
    float mRatio;
    bool mCheckStaticCollision;
    bool mDecrementLifeTime;
    float mLifeTime;
    float mLifeElapsed;
    RedColor mRenderColor;
    PblVector3 mPosition;
    PblHandle<GameObject> mOwner;
    DamageOwner mDamageOwner;
    PblList<Ordnance>::Node mCollisionManagerNode;
    PblHandle<FLEffectObject> mTrailEmitterObj;
    GameSoundControllable mSoundOrdnance;
    int mInterpIndex;

    static PblHash rttiHashOrdnance;
    static char rttiNameOrdnance[];
    static MemoryPool sMemoryPool;
};
 
 
 
 
 
 
 
 
 
 
 
