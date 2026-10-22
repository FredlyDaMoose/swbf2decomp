#pragma once

#include <stddef.h>

#include "Entity.h"
#include "Thread.h"
#include "CollisionObject.h"
#include "LeafEffect.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class LeafPatch;
class LeafPatchRenderable;
class PblRandom;

// VTABLE: SWBF2 0x007a3ac4
// VTABLE: SWBF2 0x007a3aa0
class LeafPatchClass : public Thread, public EntityClass
{
public:

    struct LeafParticle;

    LeafPatchClass(unsigned int uHash);
    virtual ~LeafPatchClass();

    virtual bool Update(float fDeltaTime);
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    void Init();
    bool RenderMaxDist(const class PblMatrix& matrix, float& fDist, unsigned int uiFlags,
                       float fMaxDist, const class RedColorValue& color) const;

    void Render(float fAlpha, const class PblMatrix& matrix, unsigned int uiFlags,
                const class RedColorValue& color) const;

    void CreateVineBranch(PblVector3 vPos, PblVector3 vDir, int iCount, PblRandom& rand);
    void CreateRenderData();
    void DestroyRenderData();

    float mBoundingRadius;
    PblVector3 mBoundingBoxRadius;
    PblVector3 mBoundingOffset;
    unsigned char mMaxFallingLeaves;
    unsigned char mMaxScatterBirds;
    float mRadius;
    float mHeightScale;
    float mHeight;
    int mSeed;
    int mType;
    LeafParticle* mParticle;
    int mNumParticles;
    PblVector3 mOffset;
    float mSizeMin;
    float mSizeMax;
    float mAlpha;
    float mMaxDistance;
    float mConeHeight;
    PblVector3 mBoxSize;
    unsigned int mTextureHash;
    float mDarknessMin;
    float mDarknessMax;
    float mNearFadeDist;
    int mNumParts;
    float mVineXSpread;
    float mVineZSpread;
    int mVineLengthMin;
    int mVineLengthMax;
    float mVineSpread;
    float mWiggleSpeed;
    float mWiggleAmount;
    int mNumVisible;
    float mFadeInStep;
    LeafPatchRenderable* mRenderData;
    PblList<LeafPatchClass>::Node mClassNode;

    static PblList<LeafPatchClass> sClasses;

    static PblList<LeafPatch> sObjects;
    static void CreateClient();
    static void Reset();
};
   
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

struct LeafPatchClass::LeafParticle
{
    PblVector3 mPos;
    float mRotation;
    float mSize;
    float mTime;
    unsigned char mTexture;
    RedColorValue mColor;
};
 
 
 
 
 

class LeafPatch : public RedSceneObject, public Entity, public LeafSpawner, public CollisionObject
{
public:
    LeafPatch(LeafPatchClass* pClass, const PblMatrix& matrix);
    virtual ~LeafPatch();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void DeactivateLeaf();
    virtual const PblVector3& GetPosition() const;
    virtual const PblMatrix& GetMatrix() const;
    virtual bool RayTest(const PblVector3& vStart, const PblVector3& vDir, float fMaxT);
    virtual float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fMaxT,
                         PblVector3* pNormal, int* pMaterial, bool bUnused);

    PblMatrix mMatrix;
    LeafPatchClass* mClass;
    unsigned char mNumFallingLeaves;
    RedColorValue mColor;
    float mLod;
    float mMaxDistance;
    PblList<LeafPatch>::Node mObjectNode;
};
 
 
 
 
 
