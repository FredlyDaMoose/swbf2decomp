#pragma once

#include <stddef.h>

#include "Damageable.h"
#include "EntityGeometry.h"
#include "Factory.h"
#include "GameSound.h"
#include "RedEngineFL/Graphics/RedColor.h"

class FLEffectClass;
class PblFileChunk;

// VTABLE: SWBF2 0x0079e8bc
class ExplosionClass : public Factory<Ordnance, ExplosionClass, OrdnanceDesc>
{
public:
    ExplosionClass(unsigned int uHash);

    ExplosionClass(ExplosionClass* pParent, unsigned int uHash);
    virtual ~ExplosionClass();
    static void Read(PblFileChunk& chunk);

    virtual ExplosionClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    void CreateExplosion(const PblVector3& aPos, const DamageOwner& aOwner, bool aFlying,
                         CollisionObjectType aCollType);

    DamageDesc mDamage;
    float mDamageRadius[2];
    float mRumbleLight;
    float mRumbleHeavy;
    float mRumbleLength;
    float mShake;
    float mShakeRadius[2];
    float mShakeLength;
    float mPersonScale;
    float mVehicleScale;
    float mShieldScale;
    float mPush;
    float mPushRadius[2];
    bool mPushDeadOnly : 1;
    bool mDoDamageToOwner : 1;
    FLEffectClass* mEffects[5];
    int mNumEffects;
    unsigned int mDecalHash;
    unsigned int mWaterEffectHash;
    unsigned int mShieldEffectHash;
    GameSound mSound;
    unsigned char mNumChunks;
    ChunkDescriptor mChunkDescriptor;
    float mInitialChunkSpeedMin;
    float mInitialChunkSpeedMax;
    float mChunkStartDistance;
    RedColor mLightColor;
    float mLightRadius;
    float mLightDuration;
    float mVisibleRadius;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
