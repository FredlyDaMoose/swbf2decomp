#ifndef SWBF2_EFFECTS_LIGHTNINGBOLTEFFECTCLASS_H
#define SWBF2_EFFECTS_LIGHTNINGBOLTEFFECTCLASS_H

#include <stddef.h>
#include "FLEffect.h"
#include "Thread.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class LightningBoltEffectObject;
class PblConfig;

// VTABLE: SWBF2 0x007a3c38
// VTABLE: SWBF2 0x007a3c20
class LightningBoltEffectClass : public FLEffectClass, public Thread
{
public:
    LightningBoltEffectClass(PblConfig& config);
    virtual ~LightningBoltEffectClass();
    virtual FLEffectObject* BuildEffect();
    virtual bool Update(float fDeltaTime);

    PblList<LightningBoltEffectClass>::Node mNode;
    PblList<LightningBoltEffectObject> mObjects;
    RedColor mColor;
    RedColor mChildColor;
    unsigned int mTexture;
    float mWidth;
    float mFadeTime;
    float mSpreadFactor;
    float mInterpolationSpeed;
    float mBranchFactor;
    float mBranchSpreadFactor;
    float mBranchLength;
    int mMaxBranches;
    float mBreakDistance;
    float mTextureSize;
    float mChildBreakDistance;
    float mChildTextureSize;
    float mChildWidth;
    float mChildSpreadFactor;
    int mNumChildren;
};
 
 
 
 
 
 
 
  // VTABLE: SWBF2 0x007a3bec
class LightningBoltEffect : public FLEffect
{
public:

    LightningBoltEffect()
        : FLEffect(0x1F678786u, 6)
    {
    }

    virtual ~LightningBoltEffect();
    virtual void Render(int value);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static void CreateClient();

    static LightningBoltEffect* sInstance;

    PblList<LightningBoltEffectClass> mClasses;
};
 
 
 // VTABLE: SWBF2 0x007a3c44
// VTABLE: SWBF2 0x007a3c74
class LightningBoltEffectObject : public FLEffectObject, public RedSceneObject
{
public:
    LightningBoltEffectObject(const LightningBoltEffectClass* pClass);
    virtual ~LightningBoltEffectObject();

    static LightningBoltEffectObject* Create(const LightningBoltEffectClass* pClass);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsEffectActive() const;
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    bool UpdateBolt(float fDeltaTime);

    bool SetOrdnanceOwner(GameObject* pObject);

    void SetStrength(float fStrength);

    void SetTargetPoint(const PblVector3& vPos) { mTargetPos = vPos; }

    static MemoryPool sMemoryPool;

private:
    void Render(const PblVector3& vStart, const PblVector3& vEnd, float fWidth, float fSpread,
                unsigned int uiSeed, unsigned int uiSeed2, float fInterpolation,
                const RedColor& color, float fBreakDistance, float fTextureSize,
                float fBranchFactor, int iMaxBranches, float fBranchSpread,
                float fBranchLength) const;

public:
    PblVector3 mTargetPos;
    float mLifeTime;
    LightningBoltEffectClass* mClass;
    PblList<LightningBoltEffectObject>::Node mNode;
    unsigned int mRandSeed;
    unsigned int mRandSeed2;
    float mInterpolation;
    float mStrength;
    PblHandle<GameObject> mGameObject;
};
 
 
 
 
 
 
 
 
 
 

#endif
