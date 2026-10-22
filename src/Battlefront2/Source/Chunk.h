#pragma once

#include <stddef.h>

#include "CollisionBody.h"
#include "CollisionModel.h"
#include "CollisionObject.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include "Thread.h"

class ChunkCluster;
class ChunkDescriptor;
class EntityGeometryClass;
class CollisionResult;
class FLEffectObject;
class GameModel;
class Restrictor;

enum ChunkPhysicsType
{
    PHYSICS_FULL_RIGID = 0,
    PHYSICS_SIMPLE_RIGID = 1,
    PHYSICS_STATIC = 2,
    PHYSICS_LEAF = 3,
    PHYSICS_COCKPIT = 4
};

// VTABLE: SWBF2 0x0079721c
class Chunk : public CollisionObject
{
public:
    Chunk();
    virtual ~Chunk();
    virtual const PblVector3& GetPosition() const;
    virtual void SetPosition(const PblVector3& vPos);
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual const PblMatrix& GetMatrix() const;
    virtual bool IsAChunk() const;
    virtual const PblVector3& GetVelocity() const;
    virtual const PblVector3& GetOmega() const;

    bool IsValid() const { return mIsValid; }
    bool IsMoving() const { return mIsMoving; }

    void ClearAll();
    void FixPosition();
    void SetData(ChunkDescriptor& desc);
    void Update(float fDeltaTime);
    void Render(unsigned char ucAlpha, float fDist, unsigned int uiFlags);

protected:
    void CreateTrailEmitters();
    void CreateSmokeEmitters();
    void DoTerrainImpactEffect(const PblVector3& vPos);
    bool DoSimpleRigidCollision(const CollisionResult* pResult, CollisionObject* pOther,
                                Restrictor* pRestrictor);
    bool DoFullRigidCollision(const CollisionResult* pResult, CollisionObject* pOther,
                              Restrictor* pRestrictor);

public:
    unsigned char mID;
    ChunkCluster* mParentCluster;
    bool mIsValid;
    bool mIsMoving;
    PblMatrix mMatrix;
    PblMatrix mCollisionMatrix;
    PblMatrix mPrimitiveMatrix;
    GameModel* mModel;
    CollisionBodyPrimitive mBodyPrimitive;
    CollisionModel mSimpleCollisionModel;
    CollisionModel* mFullCollisionModel;
    PblVector3 mVelocity;
    bool bCollisionOnUpdate;
    PblVector3 mOmega;
    PblVector3 mGravity;
    float mCoefficientOfRestitution;
    float mCoefficientOfFriction;
    float mSimpleFrictionConstant;
    float mMaxStaticLifetime;
    float mMinDeathCollisionSpeed;
    bool mForceZeroYAxisVelocity;
    PblVector3 mAngularMomentum;
    float mMass;
    float mOneOverMass;
    PblMatrix mInverseBodyInertiaTensor;
    PblMatrix mInverseWorldInertiaTensor;
    unsigned char mMaxTerrainCollisions;
    unsigned char mNumTerrainCollisions;
    unsigned int mTerrainImpactEffect;
    unsigned int mTrailEffect[3];
    unsigned int mSmokeEffect[3];
    ChunkPhysicsType mPhysicsType;
    unsigned int mTrailAttachPoints[3];
    unsigned int mSmokeAttachPoints[3];
    PblHandle<FLEffectObject> mTrailEmitters[3];
    PblHandle<FLEffectObject> mSmokeEmitters[3];
    unsigned char mForwardAxis;
    float mLifeTime;
    float mLinearDamping;
    float mAngularDamping;
    float mTotalUpdateDt;
    bool mJustRendered;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007971b4
class ChunkCluster : public RedSceneObject
{
public:
    ChunkCluster();
    virtual ~ChunkCluster();
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    bool IsValid();
    bool HasStaticChunks();
    void ClearChunks();
    void RecalculateCenterAndRadius();
    void AddChunk(ChunkDescriptor& desc);
    bool Update(float fDeltaTime);

    int mID;
    Chunk mChunks[10];
    EntityGeometryClass* mParentClass;
};
 
 
 
 

// VTABLE: SWBF2 0x00797278
class ChunkClusterManager : public Thread
{
public:
    ChunkClusterManager();
    virtual ~ChunkClusterManager();
    virtual bool Update(float fDeltaTime);

    static bool Create();
    static bool CreateClient(bool bArg);
    static bool Destroy();
    static bool DestroyClient();
    static void EnterClient();
    static void LeaveClient();
    static ChunkCluster* GetFreeCluster(bool bArg);
    static ChunkCluster* GetClusterByID(unsigned char ucID, bool bArg);

    unsigned char mOldestChunkCluster;
    ChunkCluster mChunkClusters[8];

    static ChunkClusterManager* sInstance;
    static ChunkClusterManager* sHostInstance;
    static ChunkClusterManager* sClientInstance;
};
 
 
 
