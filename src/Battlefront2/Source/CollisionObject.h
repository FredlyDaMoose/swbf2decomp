#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"

class CollisionModel;
class CollisionObject;
class CollisionOptimizationInfo;
class CollisionResult;
class Entity;
class FoleyFX;
class FoleyFXCollideeClass;
class FoleyFXColliderClass;
class FoleyFXGroup;
class GameObject;
class PblMatrix;
class PblVector3;
class RedPose;
class Restrictor;
class MemoryPool;
class TreeGridObject;
class TreeGridStack;

enum CollisionObjectType
{
    COLL_UNKNOWN = 0,
    COLL_ITEM = 1,
    COLL_SOFT = 2,
    COLL_ASTEROID = 4,
    COLL_RIGID = 8,
    COLL_STATIC = 16,
    COLL_ATTACHED = 32,
    COLL_SPECIAL = 64,
    COLL_TERRAIN = 128,
    COLL_WATER = 256,
    COLL_LEAFPATCH = 512,
    COLL_ORDNANCE = 1024,
    COLL_TOWCABLE = 2048,
    COLL_SHIELD = 4096,
    COLL_TARGETABLE = 8192,
    COLL_HOLE = 16384
};

class FoleyFXCollidee
{
public:

    FoleyFXCollidee(FoleyFXCollideeClass* pClass);

    FoleyFXCollideeClass* GetFoleyFXCollideeClass() const;

    FoleyFXCollideeClass* mCollideeClass;
};

class FoleyFXCollider
{
public:

    FoleyFXCollider(FoleyFXColliderClass* pClass);

    FoleyFX* GetFoleyFX(unsigned int uRtti) const;

    FoleyFXColliderClass* GetFoleyFXColliderClass() const;
    bool CollisionCallback(CollisionObjectType eType,
                           FoleyFXCollidee* const pCollidee,
                           const PblVector3* const pPos);

    void SetupFoleyFX(FoleyFXGroup* const pGroup);

    FoleyFXColliderClass* mColliderClass;
    FoleyFX* mFoleyFX;
    FoleyFXGroup* mFoleyFXGroup;
};

class TreeGridStack
{
public:

    TreeGridStack() : mStackSize(0), mNext(0) {}
    static void* operator new(unsigned int uiSize);
    static void operator delete(void* pBlock);
    TreeGridStack* AddObject(TreeGridObject* pObj, const PblVector3& rPos, float fSize);
    TreeGridStack* RemoveObject(TreeGridObject* pObj);

    static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow);

    int mStackSize;
    int mData[15][1];
    PblSphere mSphere[15];
    TreeGridObject* mObject[15];
    TreeGridStack* mNext;

    static MemoryPool sMemoryPool;
};
 

class TreeGridObject
{
public:

    TreeGridObject() {}

    // FUNCTION: SWBF2 0x00463a40
    explicit TreeGridObject(int iData) : mStackPtr(0), mStackIdx(-1), mTreeGridCellIndex(-1)
    {
        mData[0] = iData;
    }

    // FUNCTION: SWBF2 0x0043a910
    const PblVector3& GetCollisionCenter() const
    {
        return mStackPtr ? mStackPtr->mSphere[mStackIdx].GetCenter() : mCollisionSphere.GetCenter();
    }

    // FUNCTION: SWBF2 0x0043a930
    const float GetCollisionRadius() const
    {
        return mStackPtr ? mStackPtr->mSphere[mStackIdx].GetRadius() : mCollisionSphere.GetRadius();
    }
    int GetObjectData(int iIndex) const
    {
        return mStackPtr ? mStackPtr->mData[mStackIdx][iIndex] : mData[iIndex];
    }

    void SetObjectData(int iIndex, int iData)
    {
        if (mStackPtr)
        {
            mStackPtr->mData[mStackIdx][iIndex] = iData;
        }
        mData[iIndex] = iData;
    }

    const PblSphere& GetCollisionSphere() const
    {
        return mStackPtr ? mStackPtr->mSphere[mStackIdx] : mCollisionSphere;
    }

    // FUNCTION: SWBF2 0x00443f80
    void SetCollisionCenter(const PblVector3& rCenter)
    {
        if (mStackPtr)
        {
            PblSphere& rSphere = mStackPtr->mSphere[mStackIdx];
            rSphere.x = rCenter.x;
            rSphere.y = rCenter.y;
            rSphere.z = rCenter.z;
        }
        mCollisionSphere.x = rCenter.x;
        mCollisionSphere.y = rCenter.y;
        mCollisionSphere.z = rCenter.z;
    }
    void SetCollisionRadius(float fRadius)
    {
        if (mStackPtr)
        {
            mStackPtr->mSphere[mStackIdx].w = fRadius;
        }
        mCollisionSphere.w = fRadius;
    }
    void Set2DCollisionRadius(float fRadius) { m2dRadius = fRadius; }

    TreeGridStack* mStackPtr;
    int mStackIdx;
    PblSphere mCollisionSphere;
    float m2dRadius;
    int mData[1];
    int mTreeGridCellIndex;
};
 

class CollisionObject : public TreeGridObject, public FoleyFXCollider, public FoleyFXCollidee
{
public:

    virtual ~CollisionObject();
    virtual const PblVector3 & GetPosition() const = 0;
    virtual void SetPosition(const PblVector3 &);
    virtual void SetMatrix(const PblMatrix &);
    virtual bool CanProcedurallyAnimate();
    virtual float GetSmallestCollisionDim() const;
    virtual bool CollisionCallback(const CollisionResult *, CollisionObject *, Restrictor *);
    virtual float RayHit(const PblVector3 &, const PblVector3 &, float, PblVector3 *, int *, bool, float);
    virtual bool RayTest(const PblVector3 &, const PblVector3 &, float);
    virtual bool CheckForCollision(CollisionObject *, Restrictor *, CollisionOptimizationInfo *);
    virtual void SetTerrainHeight(int, float);
    virtual Entity * GetEntity();
    virtual const Entity * GetEntity() const;
    virtual GameObject * GetGameObject();
    virtual const GameObject * GetGameObject() const;
    virtual bool CollideTrisWithAllPrimitives() const;
    virtual const PblMatrix & GetMatrix() const = 0;
    virtual const RedPose * GetPose() const;
    virtual bool IsAChunk() const;

    virtual void RecalculateAABB();

    CollisionObject(CollisionObjectType eType, const CollisionModel* pModel,
                    FoleyFXColliderClass* const pColliderClass,
                    FoleyFXCollideeClass* const pCollideeClass);

    void AddStaticToCollisionManager();

    void AddSoftToCollisionManager();

    void AddRigidToCollisionManager();

    void AddItemToCollisionManager();
    void AddAsteroidToCollisionManager();
    void AddAttachedToCollisionManager();
    void AddShieldToCollisionManager();

    void SetMass(float fMass) { mMass = fMass; }

    float GetMass() const { return mMass; }
    void RemoveFromCollisionManager();
    bool IsInCollisionList();

    void SeparateObject(const PblVector3& vNormal, float fOverlap);
    void RestrictObject(const PblVector3& vNormal, float fOverlap, Restrictor* pRestrictor);

    void SetCollisionModel(CollisionModel* pModel);

    void PlayImpactEffects(const PblVector3* pPos, const PblVector3* pVel) const;

    void SetCollisionType(CollisionObjectType eType)
    {
        if (mStackPtr)
        {
            mStackPtr->mData[mStackIdx][0] = eType;
        }
        mData[0] = eType;
    }
    CollisionObjectType GetCollisionType() const;

    // FUNCTION: SWBF2 0x0045db10
    const CollisionModel* GetCollisionModel() const { return mCollisionModel; }

    char mCollisionObjectNode[16];
    CollisionModel* mCollisionModel;
    CollisionObject* mParent;
    float mMass;
    PblBox mAABB;
    bool mProcedurallyAnimated : 1;
    bool mLastCollidedWithNonTerrain : 1;
    PblVector3 mLastPosition;
    CollisionObject* mLastCollidedObjs[3];

    static bool sCollidingWithMesh;
    static bool sCollideTrisWithAllPrimitives;
};
 
 
 
 
 
