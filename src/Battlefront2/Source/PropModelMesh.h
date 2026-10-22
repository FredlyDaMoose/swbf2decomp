#pragma once

#include <stddef.h>

#include "PropModel.h"
#include "CollisionObject.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/RedColor.h"

class CollisionModel;
class GrassPatchClass;
class LeafPatchClass;
class PblFileChunk;
class RedModel;

// VTABLE: SWBF2 0x007aded4
class PropModelMesh : public PropModel
{
public:

    enum Type
    {
        PMM_INVALID = 0,
        PMM_MESH = 1,
        PMM_GRASS = 2,
        PMM_LEAF = 3
    };

    union RenderObject
    {
        RedModel* mGeometry;
        GrassPatchClass* mGrassClass;
        LeafPatchClass* mLeafClass;
    };

    PropModelMesh();
    virtual ~PropModelMesh();

    virtual PropObject* CreateObject(float x, float z, float rotation);
    virtual void CreateCollisionObject(float x, float z, float rotation);

    void Read(PblFileChunk& chunk);
    void SetupMatrix(PblMatrix& matrix, float rotation);

    static float GetDensity();
    static void SetDensity(float fDensity);
    static bool IsEnabled();
    static void SetEnable(bool bEnable);
    static float GetFadeAdj();
    static void SetFadeAdj(float fFadeAdj);

    Type mType;
    RenderObject mRenderObj;
    float mGeometryMaxDist;
    float mGeometryMinDist;
    float mScale;
    float mYOffset;
    CollisionModel* mCollisionModel;
    unsigned int mHash;
    GameSound mSound;
    float mSoundTriggerTime;
    float mSoundTriggerTimeDev;
    GameSound mSoundCollision;
    float mColorVariation;
    bool m_bDynamicLighting;
    float mAIVisFactorCrouch;
    float mAIVisFactorStand;
};
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007adf38
class PropObjectMesh : public PropObject
{
public:
    PropObjectMesh();
    virtual ~PropObjectMesh();

    virtual void Render(const RedCamera* pCamera, float fDist, unsigned int uiFlags);
    virtual float GetDist();
    virtual void Release();
    virtual PblVector3 GetPos();
    virtual const PblMatrix& GetMatrix();

    static PropObjectMesh* Create(PropModelMesh* pModel, float x, float z, float rotation);
    static void ClearCache();

private:
    void Init(PropModelMesh* pModel, float x, float z, float rotation);
    void Setup();
    void CalculateNextTriggerTime();

public:
    PropModelMesh* mModel;
    float mRotation;
    unsigned char mSetupFlag;
    short mCacheId;
    PblMatrix mMatrix;
    RedColor mColor;
    GameSoundControllable mSound;
    float mElapsedTime;
    float mNextTriggerTime;
};
 
 
 
 
 
 

// VTABLE: SWBF2 0x007adee4
class PropCollisionObject : public CollisionObject
{
public:
    PropCollisionObject(PblMatrix& matrix, PropModelMesh* pModel);

    virtual const PblVector3& GetPosition() const;
    virtual const PblMatrix& GetMatrix() const;

    void RecalculateAABB(PblMatrix& matrix, const PblBox& box);

    PblMatrix mMatrix;
};
 
 

float GetPropDistanceScale(PropModelMesh::Type eType);
