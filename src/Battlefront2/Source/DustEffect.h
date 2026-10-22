#pragma once

#include <stddef.h>

#include "Entity.h"
#include "GameSound.h"
#include "Thread.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class DustEffect;
class DustEffectRenderData;

// VTABLE: SWBF2 0x00799110
// VTABLE: SWBF2 0x0079923c
// VTABLE: SWBF2 0x0079914c
class DustEffectClass : public Thread, public EntityClass, public Renderable
{
public:

    struct DustParticle;

    DustEffectClass(unsigned int uHash);

    virtual ~DustEffectClass();
    virtual bool Update(float fDeltaTime);
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void Render(int value);

    bool Init();
    bool UpdateParticle(DustParticle& particle, float fDeltaTime,
                        const PblVector3& vWind);
    void SpawnParticle(DustParticle& particle);
    void CalcBoundingRadius();
    static void CreateClient();
    static void Reset();

    void Render(const PblMatrix& matrix, float fAlpha, unsigned int uiFlags) const;
    void CreateRenderData();
    void DestroyRenderData();

    DustParticle* mParticle;
    int mNumParticles;
    float mBoundingRadius;
    PblVector3 mPosMin;
    PblVector3 mPosMax;
    PblVector3 mVelMin;
    PblVector3 mVelMax;
    float mSizeMin;
    float mSizeMax;
    float mLifeTimeMin;
    float mLifeTimeMax;
    RedColorValue mColor;
    float mMinDistance;
    float mMaxDistance;
    unsigned int mTextureHash;
    float mRadiusFadeMin;
    float mRadiusFadeMax;
    float mHeightScale;
    float mTimer;
    float mCameraDistance;
    PblVector3 mOldCamPos;
    void* mRenderHeap;
    GameSound mDustSound;
    DustEffectRenderData* mRenderData;
    PblList<DustEffectClass>::Node mClassNode;

    static PblList<DustEffectClass> sDustEffectClasses;
    static PblList<DustEffect> sDustEffects;
};
    
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

struct DustEffectClass::DustParticle
{
    PblVector3 mPos;
    PblVector3 mVel;
    float mSize;
    RedColorValue mColor;
    float mLifeTime;
    float mTime;
};
 
 
 
 
 

// VTABLE: SWBF2 0x0079916c
// VTABLE: SWBF2 0x00799208
class DustEffect : public RedSceneObject, public Entity
{
public:
    DustEffect(DustEffectClass* pClass, const PblMatrix& matrix);
    virtual ~DustEffect();
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    PblMatrix mMatrix;
    DustEffectClass* mClass;
    PblList<DustEffect>::Node mEffectNode;
};
 
 
 
 

// VTABLE: SWBF2 0x00799128
class CloudClusterClass : public EntityClass
{
public:
    CloudClusterClass(unsigned int uHash);
    virtual ~CloudClusterClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    void RenderDustEffects(const PblVector3& vPos, unsigned int uSeed, unsigned int uiFlags);

    int mNumDustEffectsInCluster;
    DustEffectClass* mDustEffects[5];
    int mNumDustObjectsInEffects[5];
    float mHorizontalSpread;
    float mVerticalSpread;
    float mBoundingRadius;
};
  
 
 
 
 
 
 

class CloudClusterObject : public RedSceneObject, public Entity
{
public:
    CloudClusterObject(CloudClusterClass* pClass, const PblVector3& vPosition);
    virtual ~CloudClusterObject();
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    PblVector3 mPosition;
    unsigned int mSeed;
    CloudClusterClass* mClass;
};
 
 
