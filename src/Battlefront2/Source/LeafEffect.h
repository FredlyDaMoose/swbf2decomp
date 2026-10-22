#pragma once

#include <stddef.h>

#include "RedEngineFL/Util/RedSceneObject.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "GameSound.h"
#include "Thread.h"

class pcRedPrimitive;
class pcRedShader;

enum LeafState
{
    LEAF_PERIODIC_FLUTTER = 0,
    LEAF_TUMBLE = 1,
    LEAF_STATIC = 2,
    LEAF_GROUNDED = 3,
    LEAF_NONE = 4
};

class LeafSpawner
{
public:
    virtual void DeactivateLeaf() = 0;
};

// VTABLE: SWBF2 0x007a3950
class FallingLeafParticle : public RedSceneObject
{
public:
    FallingLeafParticle();
    virtual ~FallingLeafParticle();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    bool IsValid() const { return mIsValid; }

    void RespawnLeaf(const PblVector3& vPos, LeafSpawner* pSpawner, LeafState eState);
    void SetConstantWindVelocity(const PblVector3& vVelocity);
    void AddWindVelocity(const PblVector3& vVelocity);
    bool Update(float fDeltaTime);

    void PlatformInit();
    void PlatformCleanup();

protected:
    void DoFlutterUpdate(float fDeltaTime);
    void DoTumbleUpdate(float fDeltaTime);
    void DoGroundedUpdate(float fDeltaTime);

    pcRedPrimitive* CreateGeometry(float fSize, RedColor color);
    pcRedPrimitive* CreateCompressedGeometry(float fSize, RedColor color);

public:
    LeafState mState;
    PblMatrix mMatrix;
    PblVector3 mVelocity;
    PblVector3 mOmega;
    PblMatrix mInitMatrix;
    float mSize;
    float mPeriod;
    float mFallTime;
    bool mIsValid;
    float mGroundHeight;
    unsigned int mTextureHash;
    float mRandomness;
    LeafSpawner* mSpawner;
    PblVector3 mWindVelocity;
    PblVector3 mConstantWindVelocity;
    float mRenderTimer;
    pcRedShader* m_pShader;
};

 
 
 
 
 

// VTABLE: SWBF2 0x007a39b8
class LeafParticleManager : public Thread
{
public:
    LeafParticleManager();
    virtual ~LeafParticleManager();

    virtual bool Update(float fDeltaTime);

    static bool Create();
    static bool CreateClient();
    static bool Destroy();
    static FallingLeafParticle* GetFreeLeaf();

    GameSound mLeafFallSound;
    float mGroundLeavesTimer;
    FallingLeafParticle* mNextFreeLeaf;
    FallingLeafParticle mLeafParticles[100];

    static LeafParticleManager* sInstance;

    static LeafParticleManager* Instance() { return sInstance; }
};

 
 
 
 
 
