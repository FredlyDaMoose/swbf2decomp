#pragma once

#include <stddef.h>

#include "RedEngineFL/Util/RedSceneObject.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "Thread.h"

class pcRedPrimitive;
class pcRedShader;

enum FishState
{
    FISH_SWIM = 0,
    FISH_FLOAT = 1,
    FISH_JUMP = 2
};

struct TexCoord
{
    float u;
    float v;
};

struct FishParticleType
{
    unsigned int mTexture;
    float mSize;
    pcRedShader* m_pShader;
};

// VTABLE: SWBF2 0x0079eba0
class FishParticle : public RedSceneObject
{
public:
    FishParticle();
    virtual ~FishParticle();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    bool IsValid() const { return mIsValid; }
    void RespawnFish(const PblVector3& vPos, const FishParticleType& type, FishState eState);
    bool Update(float fDeltaTime);

    bool UpdateSwimmingFish(float fDeltaTime);
    bool UpdateFloatingFish(float fDeltaTime);
    bool UpdateJumpingFish(float fDeltaTime);

protected:

    pcRedPrimitive* CreateCompressedGeometry(PblVector3* pVertices, TexCoord* pTexCoords);
    pcRedPrimitive* CreateGeometry(PblVector3* pVertices, TexCoord* pTexCoords);

public:
    FishState mState;
    float mTimeOffCamera;
    PblMatrix mMatrix;
    PblMatrix mBaseMatrix;
    PblVector3 mVelocity;
    PblVector3 mOmega;
    float mPhase;
    float mNextTurnAngle;
    float mTotalStraightTime;
    bool mIsValid;
    bool mUnderwater;
    FishParticleType mType;
};

 
 
 
 
 
 

// VTABLE: SWBF2 0x0079ec08
class FishParticleManager : public Thread
{
public:
    FishParticleManager();
    virtual ~FishParticleManager();

    virtual bool Update(float fDeltaTime);

    static bool Create();
    static bool CreateClient();
    static bool Destroy();
    static FishParticle* GetFreeFish();
    static void SetNumFishTypes(int iNumTypes);
    static void SetFishType(int iType, float fSize, const char* pcTexture);
    static void SpawnFloatingFish(const PblVector3& vPos, float fRadius, int iNumFish);

    static void SpawnSwimmingFish(const PblVector3& vPos, float fRadius, int);

    static void PlatformInit();
    static void PlatformCleanup();

    FishParticle* mNextFreeFish;
    FishParticle mFishParticles[20];

    static FishParticleType sFishTypes[3];
    static int sNumFishTypes;
    static FishParticleManager* sInstance;
};

 
 
 
