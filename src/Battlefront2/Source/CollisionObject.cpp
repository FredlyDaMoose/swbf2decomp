#include "CollisionObject.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "EntityFlyer.h"
#include "EntityCarrier.h"
#include "FoleyFXOrdnance.h"
#include "Controllable.h"
#include "GameObject.h"
#include "Restrictor.h"
#include <new>
#include "PebbleFL/Common/PblList.h"

// FUNCTION: SWBF2 0x00443fc0
bool CollisionObject::CanProcedurallyAnimate()
{
    return true;
}

// FUNCTION: SWBF2 0x00463a70
void CollisionObject::SetPosition(const PblVector3&)
{
}

// FUNCTION: SWBF2 0x00463c20
void CollisionObject::AddAsteroidToCollisionManager()
{
    CollisionManager::AddAsteroidBody(this);
}

// GLOBAL: SWBF2 0x01e30734
bool CollisionObject::sCollidingWithMesh;

// GLOBAL: SWBF2 0x01e30347
bool CollisionObject::sCollideTrisWithAllPrimitives;

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x0043ec10
CollisionObjectType CollisionObject::GetCollisionType() const
{
    const CollisionObject* pObject = (const CollisionObject*)this;
    return (CollisionObjectType)pObject->GetObjectData(0);
}

// FUNCTION: SWBF2 0x00463ac0
CollisionObject::CollisionObject(CollisionObjectType eType, const CollisionModel* pModel,
                                 FoleyFXColliderClass* const pColliderClass,
                                 FoleyFXCollideeClass* const pCollideeClass)
    : TreeGridObject(eType),
      FoleyFXCollider(pColliderClass),
      FoleyFXCollidee(pCollideeClass)
{
    new (mCollisionObjectNode) PblList<CollisionObject>::Node;
    mProcedurallyAnimated = false;
    mCollisionModel = const_cast<CollisionModel*>(pModel);
    mParent = 0;
    mMass = 1.0f;
    mLastPosition = gVectorZero;
    mLastCollidedWithNonTerrain = true;
    mLastCollidedObjs[0] = 0;
    mLastCollidedObjs[1] = 0;
    mLastCollidedObjs[2] = 0;
}

// FUNCTION: SWBF2 0x00454dd0
void CollisionObject::SetMatrix(const PblMatrix&)
{
}

// FUNCTION: SWBF2 0x00463bb0
float CollisionObject::GetSmallestCollisionDim() const
{
    return GetCollisionRadius() * 2.0f;
}

// FUNCTION: SWBF2 0x00463d00
float CollisionObject::RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fMaxT, PblVector3* pNormal,
                              int* pBodyId, bool b, float f)
{
    if (!mCollisionModel)
    {
        return 1.0f;
    }
    return mCollisionModel->RayHit(GetMatrix(), GetPose(), vStart, vEnd, fMaxT, pNormal, pBodyId, b, f);
}

// FUNCTION: SWBF2 0x00463d60
bool CollisionObject::RayTest(const PblVector3& vStart, const PblVector3& vEnd, float fMaxT)
{
    if (!mCollisionModel)
    {
        return false;
    }
    return mCollisionModel->RayTest(GetMatrix(), GetPose(), vStart, vEnd, fMaxT);
}

// FUNCTION: SWBF2 0x00463da0
bool CollisionObject::CheckForCollision(CollisionObject* pOther, Restrictor* pRestrictor,
                                        CollisionOptimizationInfo* pOptInfo)
{
    bool bCollided = false;

    if (!mCollisionModel || !pOther->mCollisionModel)
    {
        return bCollided;
    }
    if (mParent == pOther || pOther->mParent == this)
    {
        return false;
    }

    CollisionObjectType eThisType = (CollisionObjectType)GetObjectData(0);
    CollisionObjectType eOtherType = (CollisionObjectType)pOther->GetObjectData(0);
    CollisionObjectType eOrigOtherType = eOtherType;

    if (mCollisionModel->mIsFlyerFlag)
    {
        const EntityFlyer* pFlyer = (const EntityFlyer*)GetGameObject();
        if (pFlyer && pFlyer->GetClass()->IsGuidedMissile())
        {
            eThisType = COLL_ORDNANCE;
            eOtherType = COLL_STATIC;
        }
        else if (pOther->mCollisionModel->mIsFlyerFlag)
        {
            eThisType = COLL_STATIC;
            eOtherType = COLL_STATIC;
        }
        else if (eOrigOtherType == COLL_SOFT)
        {
            eThisType = COLL_STATIC;
        }
        else if (pOther->mCollisionModel->mUseVehicleCollisionAgainstFlyers)
        {
            eThisType = COLL_RIGID;
        }
    }
    else if (pOther->mCollisionModel->mIsFlyerFlag)
    {
        if (mCollisionModel->mUseVehicleCollisionAgainstFlyers)
        {
            eOtherType = COLL_RIGID;
        }
        else
        {
            eOtherType = (eThisType == COLL_SOFT) ? COLL_STATIC : eOrigOtherType;
        }
    }

    if (!mCollisionModel->PrepareCollision(this, pOther->mCollisionModel, pOther,
                                           mCollisionModel->GetCollisionMask(eOtherType),
                                           pOther->mCollisionModel->GetCollisionMask(eThisType)))
    {
        return false;
    }

    sCollidingWithMesh = false;
    sCollideTrisWithAllPrimitives = CollideTrisWithAllPrimitives();

    for (;;)
    {
        bool bHit;
        if (GetObjectData(0) == COLL_SOFT && !mCollisionModel->mIsFlyerFlag)
        {
            bHit = mCollisionModel->CheckForCollision_SSSC(this, pOther, pRestrictor, pOptInfo);
        }
        else
        {
            bHit = mCollisionModel->CheckForCollision(this, pOther, pRestrictor, pOptInfo);
        }
        if (!bHit)
        {
            break;
        }

        bCollided = true;
        CollisionResult& result = CollisionModel::sCollisionResult;
        CollisionModel::RegisterCollision(result.mColliderId, result.mReceiverId);
        if (!sCollidingWithMesh)
        {
            CollisionCallback(&result, pOther, pRestrictor);
        }

        unsigned char uId = result.mColliderId;
        result.mColliderId = result.mReceiverId;
        result.mReceiverId = uId;
        result.mSeparationNormal.x = -result.mSeparationNormal.x;
        result.mSeparationNormal.y = -result.mSeparationNormal.y;
        result.mSeparationNormal.z = -result.mSeparationNormal.z;
        pOther->CollisionCallback(&result, this, pRestrictor);

        if (pOptInfo && mCollisionModel)
        {
            for (int i = 0; i < mCollisionModel->GetNumBodies(); ++i)
            {
                pOptInfo->refreshMatrices[i] = true;
            }
        }
    }
    return bCollided;
}

// FUNCTION: SWBF2 0x00454df0
const Entity* CollisionObject::GetEntity() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00454de0
Entity* CollisionObject::GetEntity()
{
    return 0;
}

// FUNCTION: SWBF2 0x00454e10
const GameObject* CollisionObject::GetGameObject() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00454e00
GameObject* CollisionObject::GetGameObject()
{
    return 0;
}

// FUNCTION: SWBF2 0x00443fd0
void CollisionObject::SetTerrainHeight(int, float)
{
}

// FUNCTION: SWBF2 0x00443fe0
bool CollisionObject::CollideTrisWithAllPrimitives() const
{
    return true;
}

// FUNCTION: SWBF2 0x00443ff0
bool CollisionObject::IsAChunk() const
{
    return false;
}

// FUNCTION: SWBF2 0x00454e20
const RedPose* CollisionObject::GetPose() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00463c70
void CollisionObject::RecalculateAABB()
{
    const PblVector3& vCenter = GetCollisionCenter();
    float fRadius = GetCollisionRadius();
    mAABB._vMin = PblVector3(vCenter.x - fRadius, vCenter.y - fRadius, vCenter.z - fRadius);
    mAABB._vMax = PblVector3(vCenter.x + fRadius, vCenter.y + fRadius, vCenter.z + fRadius);
}

// FUNCTION: SWBF2 0x00463bf0
void CollisionObject::AddItemToCollisionManager()
{
    CollisionManager::AddItemBody(this);
}

// FUNCTION: SWBF2 0x00463c00
void CollisionObject::AddSoftToCollisionManager()
{
    CollisionManager::AddSoftBody(this);
}

// FUNCTION: SWBF2 0x00463c10
void CollisionObject::AddRigidToCollisionManager()
{
    CollisionManager::AddRigidBody(this);
}

// FUNCTION: SWBF2 0x00463c30
void CollisionObject::AddStaticToCollisionManager()
{
    CollisionManager::AddStaticBody(this);
}

// FUNCTION: SWBF2 0x00463c40
void CollisionObject::AddAttachedToCollisionManager()
{
    CollisionManager::AddAttachedBody(this);
}

// FUNCTION: SWBF2 0x00463c50
void CollisionObject::AddShieldToCollisionManager()
{
    CollisionManager::AddShieldBody(this);
}

// FUNCTION: SWBF2 0x00463ba0
CollisionObject::~CollisionObject()
{
    RemoveFromCollisionManager();
}

// SYNTHETIC: SWBF2 0x00463b70
// CollisionObject::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00463c60
void CollisionObject::RemoveFromCollisionManager()
{
    CollisionManager::RemoveBody(this);
}

// FUNCTION: SWBF2 0x00464460
bool CollisionObject::IsInCollisionList()
{
    return *(void**)mCollisionObjectNode != 0;
}

// FUNCTION: SWBF2 0x00464470
void CollisionObject::PlayImpactEffects(const PblVector3* pPos, const PblVector3* pVel) const
{
    FoleyFXImpact* pFoleyFX = (FoleyFXImpact*)GetFoleyFX(0x7c13cb48);
    if (pFoleyFX)
    {
        pFoleyFX->mImpact.Play(pPos, pVel, false, true);
    }
}

// FUNCTION: SWBF2 0x004642d0
void CollisionObject::SeparateObject(const PblVector3& vNormal, float fOverlap)
{
    PblVector3 vMove(vNormal.x * fOverlap, vNormal.y * fOverlap, vNormal.z * fOverlap);
    const PblVector3& vPos = GetPosition();
    SetPosition(PblVector3(vPos.x + vMove.x, vPos.y + vMove.y, vPos.z + vMove.z));
}

// FUNCTION: SWBF2 0x00464350
void CollisionObject::RestrictObject(const PblVector3& vNormal, float fOverlap, Restrictor* pRestrictor)
{
    if (pRestrictor)
    {
        PblVector3 vMove = pRestrictor->GetRestrictedMovement(vNormal, fOverlap);
        const PblVector3& vPos = GetPosition();
        SetPosition(PblVector3(vPos.x + vMove.x, vPos.y + vMove.y, vPos.z + vMove.z));
        pRestrictor->AddRestriction(vNormal);
    }
    else
    {
        PblVector3 vMove(vNormal.x * fOverlap, vNormal.y * fOverlap, vNormal.z * fOverlap);
        const PblVector3& vPos = GetPosition();
        SetPosition(PblVector3(vPos.x + vMove.x, vPos.y + vMove.y, vPos.z + vMove.z));
    }
}

// FUNCTION: SWBF2 0x00464450
void CollisionObject::SetCollisionModel(CollisionModel* pModel)
{
    mCollisionModel = pModel;
}

// FUNCTION: SWBF2 0x00464010
bool CollisionObject::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                        Restrictor* pRestrictor)
{
    if (GetGameObject() && GetGameObject()->GetControllable() && pOther && pOther->GetGameObject())
    {
        GetGameObject()->GetControllable()->SetCurrentCollision(pOther->GetGameObject());
    }

    CollisionObjectType eThisType = GetCollisionType();
    FoleyFXCollidee* pCollidee = pOther;
    CollisionObjectType eOtherType = pOther->GetCollisionType();
    FoleyFXCollider::CollisionCallback(eOtherType, pCollidee, &GetPosition());

    switch (eThisType)
    {
    case COLL_ITEM:
        switch (pOther->GetCollisionType())
        {
        case COLL_STATIC:
        case COLL_TERRAIN:
            RestrictObject(pResult->mSeparationNormal, pResult->mOverlap, pRestrictor);
            return true;
        }
        break;

    case COLL_SOFT:
        switch (pOther->GetCollisionType())
        {
        case COLL_SOFT:
            SeparateObject(pResult->mSeparationNormal,
                           pResult->mOverlap * pOther->mMass / (mMass + pOther->mMass));
            return true;
        case COLL_ASTEROID:
        case COLL_RIGID:
        case COLL_STATIC:
        case COLL_TERRAIN:
            RestrictObject(pResult->mSeparationNormal, pResult->mOverlap, pRestrictor);
            return true;
        }
        break;

    case COLL_ASTEROID:
    case COLL_RIGID:
        switch (pOther->GetCollisionType())
        {
        case COLL_ASTEROID:
        case COLL_RIGID:
            SeparateObject(pResult->mSeparationNormal,
                           pResult->mOverlap * pOther->mMass / (mMass + pOther->mMass));
            return true;
        case COLL_STATIC:
        case COLL_TERRAIN:
            RestrictObject(pResult->mSeparationNormal, pResult->mOverlap, pRestrictor);
            return true;
        }
        break;
    }
    return false;
}
