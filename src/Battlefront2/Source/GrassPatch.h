#pragma once

#include <stddef.h>

#include "Entity.h"
#include "Thread.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class GrassPatch;
class GrassPatchRenderable;

// VTABLE: SWBF2 0x007a01b0
// VTABLE: SWBF2 0x007a0264
class GrassPatchClass : public Thread, public EntityClass
{
public:

    void Render(const PblMatrix& matrix, const class GrassPatch*, unsigned int uiFlags) const;

    struct GrassParticle;

    GrassPatchClass(unsigned int uHash);

    virtual ~GrassPatchClass();

    virtual bool Update(float fDeltaTime);
    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    void Init();
    void RenderMaxDist(const class PblMatrix& matrix, float fMaxDist,
                       const class RedColorValue& color, unsigned int uiFlags) const;

    void PlatformRender(const class PblMatrix& matrix, const class RedColorValue& color,
                        unsigned int uiFlags) const;

    void CreateRenderData();
    void DestroyRenderData();
    static void CreateClient();

    float mBoundingRadius;
    PblVector3 mBoundingOffset;
    PblVector3 mBoundingBoxRadius;
    GrassParticle* mParticle;
    int mNumParticles;
    float mSizeMin;
    float mSizeMax;
    float mYOffset;
    float mAlpha;
    float mMaxDistance;
    unsigned int mTextureHash;
    float mRadiusFadeMin;
    float mRadiusFadeMax;
    float mDarknessMin;
    float mDarknessMax;
    int mNumParts;
    float mBoxX;
    float mBoxZ;
    float mFlatHeightMin;
    float mFlatHeightMax;
    float mFlatSizeMultiplier;
    float mFlatFaceFactor;
    float mFlatShadowHeight;
    float mMaxSkew;
    int mFlatCount;
    float mSkinnyFactor;
    bool mFlatGrassSwingFlag;
    GrassPatchRenderable* mRenderData;
    PblList<GrassPatchClass>::Node mClassNode;
    bool mTransparentFlag;

    static PblList<GrassPatchClass> sClasses;
    static PblList<GrassPatch> sObjects;
    static void Reset();
};
   
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

struct GrassPatchClass::GrassParticle
{
    GrassParticle() {}

    PblVector3 mPos;
    PblVector3 mRight;
    PblVector3 mUp;
    float mSize;
    float mTime;
    float mOffset;
    unsigned char mTexture;
    RedColorValue mColor;
    float mHeight;
    float mUpFactor;
    float mLODPriority;
    float mSkew;
    bool mFlatType;
};
 
 
 

// VTABLE: SWBF2 0x007a01c8
// VTABLE: SWBF2 0x007a0230
class GrassPatch : public RedSceneObject, public Entity
{
public:

    GrassPatch(GrassPatchClass* pClass, const PblMatrix& matrix);
    virtual ~GrassPatch();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    static bool IsEnabled();

    PblMatrix mMatrix;
    GrassPatchClass* mClass;
    PblList<GrassPatch>::Node mObjectNode;
};
 
 
 
