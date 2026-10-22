#pragma once

#include <stddef.h>

#include "RedEngineFL/Util/RedSceneObject.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "GameSound.h"
#include "Thread.h"

class pcRedPrimitive;
class pcRedShader;

enum BirdState
{
    BIRD_SCATTER = 0,
    BIRD_FLOCK = 1,
    BIRD_HOP_CIRCLE = 2,
    BIRD_WALK_CIRCLE = 3,
    BIRD_TAKEOFF = 4,
    BIRD_WALK = 5,
    BIRD_RUN = 6
};

struct BirdParticleType
{
    unsigned int mTexture;
    float mSize;
    pcRedShader* m_pShader;
};

// VTABLE: SWBF2 0x00796f30
class BirdParticle : public RedSceneObject
{
public:
    BirdParticle();
    virtual ~BirdParticle();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    void StartBirdSound();
    void RespawnBird(const PblVector3& vPos);
    void RespawnGroundBird(const PblVector3& vPos, const BirdParticleType& type);
    void RespawnFlockBird(const PblVector3& vPos, const PblVector3& vVelocity,
                          const BirdParticleType& type);
    void RespawnScatterBird(const PblVector3& vPos, const BirdParticleType& type,
                            const PblVector3& vAway);
    void UpdateFlying(float fDeltaTime);
    bool UpdateFlockBird(float fDeltaTime);
    bool UpdateWalkingCircleBird(float fDeltaTime);

    bool UpdateHoppingCircleBird();
    bool UpdateStandingBird(float fDeltaTime);

    bool UpdateScatterBird(float fDeltaTime);
    bool Update(float fDeltaTime);

protected:

    pcRedPrimitive* CreateCompressedGeometry();
    pcRedPrimitive* CreateGeometry();

public:
    BirdState mState;
    float mRandomness;
    float mTotalLifetime;
    float mFlapTime;
    float mTimeOffCamera;
    float mCircleRadius;
    float mCircleTime;
    float mTotalCircleTime;
    PblVector3 mCircleCenter;
    PblMatrix mInitMatrix;
    PblMatrix mMatrix;
    PblVector3 mVelocity;
    PblMatrix mWing1Matrix;
    PblMatrix mWing2Matrix;
    bool mIsValid;
    GameSoundControllable mBirdSound;
    BirdParticleType mType;
};

 
 
 
 
 

class BirdParticleManager : public Thread
{
public:
    BirdParticleManager();
    virtual ~BirdParticleManager();

    virtual bool Update(float fDeltaTime);

    static bool Create();
    static bool CreateClient();
    static bool Destroy();
    static void SetNumBirdTypes(int iNum);
    static void SetBirdType(int iIndex, float fSize, const char* pTexture);

    static void __fastcall SetPotentialScatterPoint(const PblVector3* pPosition, unsigned char ucNumBirds);
    static BirdParticle* GetFreeBird();
    static void ScatterRandomBirds(int iNum, const PblVector3& vPos);
    static void FlockRandomBirds(const PblVector3& vPos, const PblVector3& vVelocity);
    static void SpawnGroundBirds();
    static void PlatformInit();
    static void PlatformCleanup();

    GameSound mScatterSound;
    GameSound mBirdSound;
    PblVector3 mPotentialScatterPoints[5];
    unsigned char mNumScatterBirds[5];
    BirdParticle* mNextFreeBird;
    BirdParticle mBirdParticles[15];
    float mScatterTimer;
    float mFlockTimer;
    float mStandTimer;

    static BirdParticleType sBirdTypes[3];
    static int sNumBirdTypes;
    static float sFlockMinHeight;
    static BirdParticleManager* sInstance;

    static BirdParticleManager* Instance() { return sInstance; }
};

 
 
 
 
 
