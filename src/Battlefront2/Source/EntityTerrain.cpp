#include "EntityTerrain.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x004fc920
EntityTerrain::EntityTerrain()
    : CollisionObject(COLL_TERRAIN, 0, 0, 0)
{
    SetCollisionCenter(gVectorZero);
    SetCollisionRadius(10000.0f);
    Set2DCollisionRadius(10000.0f);
    CollisionManager::SetTerrain(this);
}

// FUNCTION: SWBF2 0x004fca10
bool EntityTerrain::CheckForCollision(CollisionObject* pObject, Restrictor* pRestrictor)
{
    if (!RedTerrain::_bEnable)
    {
        return false;
    }

    const CollisionModel* pModel = pObject->mCollisionModel;
    bool bCollided = false;
    CollisionObjectType eType = COLL_TERRAIN;
    if (!pModel->PrepareCollision(pObject, mCollisionModel, this, pModel->GetCollisionMask(eType),
                                  CollisionMask(1)))
    {
        return false;
    }

    if (pModel)
    {
        const CollisionResult* pResult = pModel->CollideTerrain(pObject, pObject->GetMatrix(), pObject->GetPose());
        while (pResult)
        {
            if (pObject->CollisionCallback(pResult, this, pRestrictor))
            {
                bCollided = true;
            }
            CollisionModel::RegisterCollision(pResult->mColliderId, 0);
            pResult = pModel->CollideTerrain(pObject, pObject->GetMatrix(), pObject->GetPose());
        }
    }
    return bCollided;
}

// FUNCTION: SWBF2 0x004fcb00
float EntityTerrain::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                            PblVector3* pNormal, int* pMaterial, bool)
{
    if (!RedTerrain::_bEnable)
    {
        return 1.0f;
    }
    PblVector3 vEnd(vStart.x + vDir.x * fLength, vStart.y + vDir.y * fLength,
                    vStart.z + vDir.z * fLength);
    float fHit = RedTerrain::RayHit(vStart, vEnd, 0.1f, 3.0f);
    if (fHit < 1.0f && pNormal)
    {
        float fDist = fHit * fLength;
        *pNormal = RedTerrain::GetInterpolatedNormal(vStart.x + vDir.x * fDist,
                                                     vStart.z + fDist * vDir.z);
        if (pMaterial)
        {
            *pMaterial = 0;
        }
    }
    return fHit;
}

// FUNCTION: SWBF2 0x004fcc10
bool EntityTerrain::RayTest(const PblVector3& vStart, const PblVector3& vDir, float fLength)
{
    if (!RedTerrain::_bEnable)
    {
        return false;
    }
    PblVector3 vNormal;
    return RayHit(vStart, vDir, fLength, &vNormal, 0, false) < 1.0f;
}

// FUNCTION: SWBF2 0x004fca00
EntityTerrain::~EntityTerrain()
{
}

// SYNTHETIC: SWBF2 0x004fc9d0
// EntityTerrain::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fc8e0
const PblVector3& EntityTerrain::GetPosition() const
{
    return gVectorZero;
}

// FUNCTION: SWBF2 0x004fc8f0
const PblMatrix& EntityTerrain::GetMatrix() const
{
    return *reinterpret_cast<const PblMatrix*>(&gMatrixIdentity);
}

// FUNCTION: SWBF2 0x004fc900
const CollisionObject* EntityTerrain::GetCollisionObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x004fc910
CollisionObject* EntityTerrain::GetCollisionObject()
{
    return this;
}
