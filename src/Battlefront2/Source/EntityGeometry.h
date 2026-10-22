#pragma once

#include <stddef.h>

#include "CollisionModel.h"
#include "CollisionObject.h"
#include "Entity.h"
#include "FoleyFX.h"
#include "GameModel.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class AttachedEffects;
class AttachedEffectsClass;
class CollisionBodyPrimitive;
class GameModel;
class GameObject;
class RedLodData;
class RedPose;

class ChunkDescriptor
{
public:

    void Init();

    PblMatrix mMatrix;
    PblVector3 mVelocity;
    GameModel* mModel;
    CollisionBodyPrimitive* mPrimitive;
    unsigned char mMaxTerrainCollisions;
    unsigned int mTerrainImpactEffect;

    int mPhysicsType;
    PblVector3 mOmega;
    PblVector3 mGravity;
    CollisionModel* mFullCollisionModel;
    float mCoefficientOfRestitution;
    float mCoefficientOfFriction;
    float mSimpleFrictionConstant;
    float mMinDeathCollisionSpeed;
    bool mForceZeroYAxisVelocity;
    float mMaxStaticLifetime;
    unsigned int mTrailEffect[3];
    unsigned int mSmokeEffect[3];
    unsigned int mTrailAttachPoints[3];
    unsigned int mSmokeAttachPoints[3];
    bool mNoBuildingCollisions;
    bool mKeepSoldierCollisions;
    float mLinearDamping;
    float mAngularDamping;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079b634
class EntityGeometryClass : public EntityClass,
                            public FoleyFXCollideeClass,
                            public FoleyFXColliderClass
{
public:

    struct HitLocationData
    {
        int mBodyId;
        float mDamageMultiplier;
    };

    EntityGeometryClass(unsigned int uHash);

    EntityGeometryClass(EntityGeometryClass* pBase, unsigned int uHash);
    virtual ~EntityGeometryClass();

    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual const GameModel* GetModel();

    int GetNumChunks();

    virtual bool IsRtti(unsigned int uRtti);

    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();

    virtual void PostReadSetup();

    float GetPriorityMod() { return _fPriorityMod; }
    const PblVector3& GetTargetPointOffset() const { return mTargetPointOffset; }

protected:

    bool AddBuildingCollision(const char* pszValue);
    bool AddVehicleCollision(const char* pszValue);
    bool AddTerrainCollision(const char* pszValue);
    bool AddOrdnanceCollision(const char* pszValue);
    bool AddSoldierCollision(const char* pszValue);
    bool AddTargetableCollision(const char* pszValue);

public:

    static PblHash rttiHashEntityGeometryClass;
    static char rttiNameEntityGeometryClass[];

    ChunkDescriptor* mChunkDescriptors[10];
    CollisionModel* mFullChunkCollisions[10];
    float mInitialChunkSpeed[10];
    float mChunkVelocityFactor[10];
    float mChunkUpFactor[10];
    bool m_bDynamicLighting;
    float mMaxShadowLength;
    float _fPriorityMod;
    unsigned int mChunkAttachPoints[10];
    GameModel* mModel;
    int mLODClass;
    CollisionModel mCollisionModel;
    float mPotentialCollisionRadius;
    HitLocationData* mHitLocations;
    int mNumHitLocations;
    AttachedEffectsClass* m_pAttachedEffectsClass;
    PblVector3 mTargetPointOffset;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class EntityGeometry : public EntityEx, public CollisionObject, public RedSceneObject
{
public:
    EntityGeometry(EntityGeometryClass* pClass, const EntityDesc& desc, CollisionObjectType eType,
                   const RedLodData* pLodData);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityGeometry();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    // FUNCTION: SWBF2 0x00444120
    virtual Entity* GetEntity() { return this; }
    // SYNTHETIC: SWBF2 0x00446e2c
    // [thunk]:EntityGeometry::GetEntity`adjustor{12}'

    // FUNCTION: SWBF2 0x00444110
    virtual const Entity* GetEntity() const { return this; }

    // FUNCTION: SWBF2 0x00497e40
    virtual GameObject* GetGameObject() { return 0; }
    // FUNCTION: SWBF2 0x00497e30
    virtual const GameObject* GetGameObject() const { return 0; }

    // FUNCTION: SWBF2 0x00444130
    virtual const CollisionObject* GetCollisionObject() const { return this; }

    // FUNCTION: SWBF2 0x00444140
    virtual CollisionObject* GetCollisionObject() { return this; }
    // FUNCTION: SWBF2 0x00444150
    virtual const GameModel* GetModel() const { return mModel; }

    // FUNCTION: SWBF2 0x00444160
    virtual void SetModel(const GameModel* pModel)
    {
        mModel = const_cast<GameModel*>(pModel);
        _pLodData = &mModel->_mLodData;
    }
    virtual void SetupPose(RedPose& pose) const;
    virtual const PblVector3& GetVelocity() const;

    // FUNCTION: SWBF2 0x00475dc0
    virtual void SetVelocity(const PblVector3&) {}
    virtual int GetTargetBodyID(const PblVector3& vPos, const PblVector3& vDir) const;
    virtual PblVector3 GetTargetPoint(const PblVector3& vPos, const PblVector3& vDir, int iBodyId) const;
    virtual PblVector3 GetTargetPointMissVector() const;

    // FUNCTION: SWBF2 0x00444100
    virtual const PblVector3& GetPosition() const { return mMatrix.GetTranslation(); }

    virtual void SetPosition(const PblVector3& vPos);

    virtual void SetMatrix(const PblMatrix& matrix);
    virtual float GetSmallestCollisionDim() const;
    // FUNCTION: SWBF2 0x00475da0
    virtual const PblMatrix& GetMatrix() const { return mMatrix; }
    // FUNCTION: SWBF2 0x00444180
    virtual const RedPose* GetPose() const { return 0; }

    void BuildEffects();

    bool SetupShadow(const GameObject* pObject, const GameModel* pModel, unsigned char ucLod,
                     unsigned int& uFlags, unsigned int uShadowTexture);

    void SetupLightingState(unsigned int& uFlags);

    void CreateChunkCluster(float fScale);

    PblMatrix mMatrix;
    GameModel* mModel;
    float mSmallestCollisionDim;
    AttachedEffects* m_pAttachedEffects;

    static PblHash rttiHashEntityGeometry;
    static char rttiNameEntityGeometry[];
};
 
 
 
 
 
