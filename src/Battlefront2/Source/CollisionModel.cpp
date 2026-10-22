#include "CollisionModel.h"
#include "PebbleFL/Common/PblVector.h"

#include "CollisionBody.h"
#include "CollisionObject.h"

#include <d3dx9math.h>

// GLOBAL: SWBF2 0x01eb55c0
CollisionResult CollisionModel::sCollisionResult;
// GLOBAL: SWBF2 0x01eb55a0
CollisionResult CollisionModel::sHostCollisionResult;
// GLOBAL: SWBF2 0x01eb57e0
CollisionResult CollisionModel::sClientCollisionResult;
// GLOBAL: SWBF2 0x01eb53a0
// SYNTHETIC: SWBF2 0x00402480
// `dynamic initializer for 'CollisionModel::sHostCollisionMask''
CollisionMask CollisionModel::sHostCollisionMask[64];
// GLOBAL: SWBF2 0x007e5f04
CollisionMask (*CollisionModel::sCollisionMask)[64] = &CollisionModel::sHostCollisionMask;

// FUNCTION: SWBF2 0x004621a0
void CollisionModel::EnterClient()
{
    sCollisionMask = &sClientCollisionMask;
    sHostCollisionResult = sCollisionResult;
    sCollisionResult = sClientCollisionResult;
}

// FUNCTION: SWBF2 0x004621f0
void CollisionModel::LeaveClient()
{
    sCollisionMask = &sHostCollisionMask;
    sClientCollisionResult = sCollisionResult;
    sCollisionResult = sHostCollisionResult;
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

static inline const PblMatrix operator*(const PblMatrix& a, const PblMatrix& b)
{
    PblMatrix result;
    D3DXMatrixMultiply((D3DXMATRIX*)&result, (const D3DXMATRIX*)&a, (const D3DXMATRIX*)&b);
    return result;
}

static inline bool IsEmpty(const CollisionMask& mask)
{
    return mask.mMask[0] == 0 && mask.mMask[1] == 0;
}

static inline const PblVector3 Translation(const PblMatrix& m)
{
    return PblVector3(m.m[3][0], m.m[3][1], m.m[3][2]);
}

// FUNCTION: SWBF2 0x00462240
CollisionModel::CollisionModel()
{
    mNumBodies = 0;

    mSoftMask.mMask[0] = 0;
    mSoftMask.mMask[1] = 0;
    mRigidMask.mMask[0] = 0;
    mRigidMask.mMask[1] = 0;
    mStaticMask.mMask[0] = 0;
    mStaticMask.mMask[1] = 0;
    mTerrainMask.mMask[0] = 0;
    mTerrainMask.mMask[1] = 0;
    mOrdnanceMask.mMask[0] = 0;
    mOrdnanceMask.mMask[1] = 0;
    mTowCableMask.mMask[0] = 0;
    mTowCableMask.mMask[1] = 0;
    mTargetableMask.mMask[0] = 0;
    mTargetableMask.mMask[1] = 0;
    mFlyingOrdnanceMask.mMask[0] = 0;
    mFlyingOrdnanceMask.mMask[1] = 0;

    mDenyFlyerLand = false;
    mDeathOnFlyerLand = false;
    mIsFlyerFlag = false;
    mUseVehicleCollisionAgainstFlyers = false;
}

// FUNCTION: SWBF2 0x004623d0
CollisionModel::~CollisionModel()
{
    while (mNumBodies != 0)
    {
        --mNumBodies;
        CollisionBody* pBody = mBody[mNumBodies];
        if (pBody)
        {
            delete pBody;
        }
    }
}

// SYNTHETIC: SWBF2 0x004623a0
// CollisionModel::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00462420
void CollisionModel::ClearAll(bool bDeleteBodies)
{
    if (bDeleteBodies)
    {
        while (mNumBodies != 0)
        {
            --mNumBodies;
            CollisionBody* pBody = mBody[mNumBodies];
            if (pBody)
            {
                delete pBody;
            }
        }
    }
    else
    {
        mNumBodies = 0;
    }

    mSoftMask.mMask[0] = 0;
    mSoftMask.mMask[1] = 0;
    mRigidMask.mMask[0] = 0;
    mRigidMask.mMask[1] = 0;
    mStaticMask.mMask[0] = 0;
    mStaticMask.mMask[1] = 0;
    mTerrainMask.mMask[0] = 0;
    mTerrainMask.mMask[1] = 0;
    mOrdnanceMask.mMask[0] = 0;
    mOrdnanceMask.mMask[1] = 0;
    mTowCableMask.mMask[0] = 0;
    mTowCableMask.mMask[1] = 0;
    mTargetableMask.mMask[0] = 0;
    mTargetableMask.mMask[1] = 0;
    mFlyingOrdnanceMask.mMask[0] = 0;
    mFlyingOrdnanceMask.mMask[1] = 0;
}

// FUNCTION: SWBF2 0x00462520
bool CollisionModel::AddBody(CollisionBody* pBody)
{
    if (mNumBodies == 64)
    {
        return false;
    }

    mBody[mNumBodies] = pBody;
    if (pBody && pBody->mMask)
    {
        unsigned char ucMask = pBody->mMask;
        if (ucMask & 0x01)
        {
            mSoftMask.SetId(mNumBodies);
        }
        if (ucMask & 0x02)
        {
            mRigidMask.SetId(mNumBodies);
        }
        if (ucMask & 0x04)
        {
            mStaticMask.SetId(mNumBodies);
        }
        if (ucMask & 0x08)
        {
            mTerrainMask.SetId(mNumBodies);
        }
        if (ucMask & 0x10)
        {
            mOrdnanceMask.SetId(mNumBodies);
        }
        if (ucMask & 0x20)
        {
            mFlyingOrdnanceMask.SetId(mNumBodies);
        }
    }
    else
    {
        mTerrainMask.SetId(mNumBodies);
        mStaticMask.SetId(mNumBodies);
        mRigidMask.SetId(mNumBodies);
        mSoftMask.SetId(mNumBodies);
        mOrdnanceMask.SetId(mNumBodies);
    }
    ++mNumBodies;
    return true;
}

// FUNCTION: SWBF2 0x00462720
int CollisionModel::GetNumBodies() const
{
    return mNumBodies;
}

// FUNCTION: SWBF2 0x00462730
bool CollisionModel::CheckForCollision_SSSC(CollisionObject* pCollider, CollisionObject* pReceiver,
                                            Restrictor* pRestrictor, CollisionOptimizationInfo* pInfo) const
{
    if (pCollider->GetPose() || pReceiver->GetPose())
    {
        return CheckForCollision(pCollider, pReceiver, pRestrictor, pInfo);
    }

    const PblMatrix& colliderMatrix = pCollider->GetMatrix();
    const PblMatrix& receiverMatrix = pReceiver->GetMatrix();
    const CollisionModel* pOtherModel = pReceiver->mCollisionModel;
    bool bUseIgnores = pInfo && pInfo->ignoreMasks;
    bool bUseBodyCache = pInfo && pInfo->bodyMatrices;
    bool bUseOtherCache = pInfo && pInfo->otherModelMatrices;

    for (unsigned char i = 0; i < mNumBodies; ++i)
    {
        const CollisionMask& mask = (*sCollisionMask)[i];
        if (IsEmpty(mask))
        {
            continue;
        }

        sCollisionResult.mColliderId = i;
        CollisionBody* pBody = mBody[i];
        const PblMatrix* pMatrix;
        PblMatrix matrix;
        if (bUseBodyCache)
        {
            if (pInfo->refreshMatrices[i])
            {
                pInfo->bodyMatrices[i] = pBody->mMatrix * colliderMatrix;
                pInfo->refreshMatrices[i] = false;
            }
            pMatrix = &pInfo->bodyMatrices[i];
        }
        else
        {
            matrix = pBody->mMatrix * colliderMatrix;
            pMatrix = &matrix;
        }

        for (unsigned char j = 0; j < pOtherModel->mNumBodies; ++j)
        {
            if (!mask.IsSet(j))
            {
                continue;
            }
            if (bUseIgnores && pInfo->ignoreMasks[j].IsSet(i))
            {
                continue;
            }

            CollisionBody* pOther = pOtherModel->mBody[j];
            const PblMatrix* pOtherMatrix;
            PblMatrix otherMatrix;
            if (bUseOtherCache)
            {
                pOtherMatrix = &pInfo->otherModelMatrices[j];
            }
            else
            {
                otherMatrix = pOther->mMatrix * receiverMatrix;
                pOtherMatrix = &otherMatrix;
            }

            if (!pOther->bIsMesh)
            {
                float dx = pMatrix->m[3][0] - pOtherMatrix->m[3][0];
                float dy = pMatrix->m[3][1] - pOtherMatrix->m[3][1];
                float dz = pMatrix->m[3][2] - pOtherMatrix->m[3][2];
                float fDist2 = dx * dx + dy * dy + dz * dz;
                float fReach = ((CollisionBodyPrimitive*)pOther)->GetRadius() +
                               ((CollisionBodyPrimitive*)pBody)->GetRadius();
                if (fDist2 > fReach * fReach)
                {
                    fReach = ((CollisionBodyPrimitive*)pBody)->GetRadius() * 1.5f +
                             ((CollisionBodyPrimitive*)pOther)->GetRadius();
                    if (fDist2 > fReach * fReach && bUseIgnores)
                    {
                        pInfo->ignoreMasks[j].SetId(i);
                    }
                    continue;
                }
            }

            sCollisionResult.mReceiverId = j;
            if (pOther->ReceiveCollision(*pMatrix, ((CollisionBodyPrimitive*)pBody)->mPrimitive, pCollider,
                                         *pOtherMatrix, pReceiver, pRestrictor))
            {
                int k;
                for (k = 0; k < 3; ++k)
                {
                    if (pCollider->mLastCollidedObjs[k] == pReceiver)
                    {
                        break;
                    }
                }
                if (k == 3)
                {
                    pCollider->mLastCollidedObjs[2] = pCollider->mLastCollidedObjs[1];
                    pCollider->mLastCollidedObjs[1] = pCollider->mLastCollidedObjs[0];
                    pCollider->mLastCollidedObjs[0] = pReceiver;
                }
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00462b90
bool CollisionModel::CheckForCollision(CollisionObject* pCollider, CollisionObject* pReceiver,
                                       Restrictor* pRestrictor, CollisionOptimizationInfo* pInfo) const
{
    const CollisionModel* pOtherModel = pReceiver->mCollisionModel;
    bool bUseIgnores = pInfo && pInfo->ignoreMasks;
    bool bUseBodyCache = pInfo && pInfo->bodyMatrices;

    for (unsigned char i = 0; i < mNumBodies; ++i)
    {
        const CollisionMask& mask = (*sCollisionMask)[i];
        if (IsEmpty(mask) || mBody[i]->bIsMesh)
        {
            continue;
        }

        sCollisionResult.mColliderId = i;
        const PblMatrix* pMatrix;
        PblMatrix matrix;
        if (bUseBodyCache)
        {
            if (pInfo->refreshMatrices[i])
            {
                pInfo->bodyMatrices[i] = mBody[i]->GetWorldMatrix(pCollider->GetMatrix(), pCollider->GetPose());
                pInfo->refreshMatrices[i] = false;
            }
            pMatrix = &pInfo->bodyMatrices[i];
        }
        else
        {
            matrix = mBody[i]->GetWorldMatrix(pCollider->GetMatrix(), pCollider->GetPose());
            pMatrix = &matrix;
        }

        CollisionBody* pBody = mBody[i];
        for (unsigned char j = 0; j < pOtherModel->mNumBodies; ++j)
        {
            if (!mask.IsSet(j))
            {
                continue;
            }
            if (bUseIgnores && pInfo->ignoreMasks[j].IsSet(i))
            {
                continue;
            }

            sCollisionResult.mReceiverId = j;
            CollisionBody* pOther = pOtherModel->mBody[j];
            PblMatrix otherMatrix = pOther->GetWorldMatrix(pReceiver->GetMatrix(), pReceiver->GetPose());
            if (!pOther->bIsMesh)
            {
                float dx = otherMatrix.m[3][0] - pMatrix->m[3][0];
                float dy = otherMatrix.m[3][1] - pMatrix->m[3][1];
                float dz = otherMatrix.m[3][2] - pMatrix->m[3][2];
                float fDist2 = dx * dx + dy * dy + dz * dz;
                float fReach = ((CollisionBodyPrimitive*)pOther)->GetRadius() +
                               ((CollisionBodyPrimitive*)pBody)->GetRadius();
                if (fDist2 > fReach * fReach)
                {
                    fReach = ((CollisionBodyPrimitive*)pBody)->GetRadius() * 1.5f +
                             ((CollisionBodyPrimitive*)pOther)->GetRadius();
                    if (fDist2 > fReach * fReach && bUseIgnores)
                    {
                        pInfo->ignoreMasks[j].SetId(i);
                    }
                    continue;
                }
            }

            if (pOther->ReceiveCollision(*pMatrix, ((CollisionBodyPrimitive*)pBody)->mPrimitive, pCollider,
                                         otherMatrix, pReceiver, pRestrictor))
            {
                int k;
                for (k = 0; k < 3; ++k)
                {
                    if (pCollider->mLastCollidedObjs[k] == pReceiver)
                    {
                        break;
                    }
                }
                if (k == 3)
                {
                    pCollider->mLastCollidedObjs[2] = pCollider->mLastCollidedObjs[1];
                    pCollider->mLastCollidedObjs[1] = pCollider->mLastCollidedObjs[0];
                    pCollider->mLastCollidedObjs[0] = pReceiver;
                }
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00463020
const CollisionResult* CollisionModel::CollideTerrain(CollisionObject* pObject, const PblMatrix& matrix,
                                                      const RedPose* pPose) const
{
    for (unsigned char i = 0; i < mNumBodies; ++i)
    {
        if (IsEmpty((*sCollisionMask)[i]))
        {
            continue;
        }

        PblMatrix world = mBody[i]->GetWorldMatrix(matrix, pPose);
        bool bHit = mBody[i]->CollideTerrain(world, sCollisionResult.mSeparationNormal, sCollisionResult.mOverlap,
                                             sCollisionResult.mCollisionPosition);
        pObject->SetTerrainHeight(i, -sCollisionResult.mOverlap);
        if (bHit)
        {
            sCollisionResult.mColliderId = i;
            sCollisionResult.mReceiverId = 0;
            return &sCollisionResult;
        }
    }
    return NULL;
}

// FUNCTION: SWBF2 0x00463120
int CollisionModel::GetMaskId(unsigned int uiName) const
{
    for (int i = 0; i < mNumBodies; ++i)
    {
        if (mBody[i]->mName == uiName)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x00463150
const float CollisionModel::GetHeightFromMaskID(int id) const
{
    if (id >= 0 && id < mNumBodies)
    {
        return mBody[id]->GetHeight();
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x00463190
unsigned int CollisionModel::GetParentNameFromMaskId(int id) const
{
    if (id >= 0 && id < mNumBodies)
    {
        return mBody[id]->mParentHashName;
    }
    return 0;
}

// FUNCTION: SWBF2 0x004631c0
bool CollisionModel::PrepareCollision(CollisionObject*, const CollisionModel*, CollisionObject*,
                                      const CollisionMask& colliderMask, const CollisionMask& receiverMask) const
{
    for (int i = 0; i < mNumBodies; ++i)
    {
        if (colliderMask.IsSet(i))
        {
            (*sCollisionMask)[i] = receiverMask;
        }
        else
        {
            (*sCollisionMask)[i].mMask[0] = 0;
            (*sCollisionMask)[i].mMask[1] = 0;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00463240
float CollisionModel::RayHit(const PblMatrix& matrix, const RedPose* pPose, const PblVector3& vStart,
                             const PblVector3& vDir, float fLength, PblVector3* pNormal, int* pBodyId,
                             bool bOrdnance, float fRadius) const
{
    PblMatrix inverse = matrix.GetInverseFast();
    PblVector3 vLocalStart;
    PblVector3 vLocalDir;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocalStart, (const D3DXVECTOR3*)&vStart, (const D3DXMATRIX*)&inverse);
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vLocalDir, (const D3DXVECTOR3*)&vDir, (const D3DXMATRIX*)&inverse);

    float fHit = 1.0f;
    PblVector3 vNormal;
    for (int i = 0; i < mNumBodies; ++i)
    {
        if (bOrdnance && !mOrdnanceMask.IsSet(i))
        {
            continue;
        }

        float fBodyHit = mBody[i]->RayHit(pPose, vLocalStart, vLocalDir, fLength, &vNormal, fRadius);
        if (fBodyHit < 1.0f)
        {
            fLength = fBodyHit * fLength;
            fHit = fBodyHit * fHit;
            if (pBodyId)
            {
                *pBodyId = i;
            }
            if (pNormal)
            {
                *pNormal = vNormal;
            }
        }
    }

    if (fHit < 1.0f && pNormal)
    {
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vNormal, (const D3DXVECTOR3*)pNormal, (const D3DXMATRIX*)&matrix);
        *pNormal = vNormal;
    }
    return fHit;
}

// FUNCTION: SWBF2 0x004633e0
bool CollisionModel::RayTest(const PblMatrix& matrix, const RedPose* pPose, const PblVector3& vStart,
                             const PblVector3& vDir, float fLength) const
{
    PblMatrix inverse = matrix.GetInverseFast();
    PblVector3 vLocalStart;
    PblVector3 vLocalDir;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocalStart, (const D3DXVECTOR3*)&vStart, (const D3DXMATRIX*)&inverse);
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vLocalDir, (const D3DXVECTOR3*)&vDir, (const D3DXMATRIX*)&inverse);

    for (int i = 0; i < mNumBodies; ++i)
    {
        if (mBody[i]->RayTest(pPose, vLocalStart, vLocalDir, fLength))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00463480
void CollisionModel::RegisterCollision(int iCollider, int iReceiver)
{
    (*sCollisionMask)[iCollider].ClearId(iReceiver);
}

// FUNCTION: SWBF2 0x004634b0
int CollisionModel::GetTargetBodyID(const PblVector3& vPos, const PblVector3& vDir, const PblMatrix& matrix,
                                    const RedPose* pPose) const
{
    int iBest = -1;
    float fBest = 0.0f;
    for (int i = 0; i < mNumBodies; ++i)
    {
        if (!mTargetableMask.IsSet(i))
        {
            continue;
        }

        PblMatrix world = mBody[i]->GetWorldMatrix(matrix, pPose);
        PblVector3 vTo(world.m[3][0] - vPos.x, world.m[3][1] - vPos.y, world.m[3][2] - vPos.z);
        PblVector3 vNorm;
        D3DXVec3Normalize((D3DXVECTOR3*)&vNorm, (const D3DXVECTOR3*)&vTo);
        vTo = vNorm;
        float fDot = vDir.y * vTo.y + vDir.x * vTo.x + vDir.z * vTo.z;
        if (fDot > fBest)
        {
            fBest = fDot;
            iBest = i;
        }
    }
    return iBest;
}

// FUNCTION: SWBF2 0x004635e0
PblVector3 CollisionModel::GetTargetPoint(const PblVector3& vPos, const PblVector3& vDir, const PblMatrix& matrix,
                                          const RedPose* pPose) const
{
    PblVector3 vPoint = Translation(matrix);
    float fBest = 0.0f;
    for (int i = 0; i < mNumBodies; ++i)
    {
        if (!mTargetableMask.IsSet(i))
        {
            continue;
        }

        PblMatrix world = mBody[i]->GetWorldMatrix(matrix, pPose);
        PblVector3 vTo(world.m[3][0] - vPos.x, world.m[3][1] - vPos.y, world.m[3][2] - vPos.z);
        PblVector3 vNorm;
        D3DXVec3Normalize((D3DXVECTOR3*)&vNorm, (const D3DXVECTOR3*)&vTo);
        vTo = vNorm;
        float fDot = vDir.y * vTo.y + vDir.x * vTo.x + vDir.z * vTo.z;
        if (fDot > fBest)
        {
            vPoint = Translation(world);
            fBest = fDot;
        }
    }
    return vPoint;
}

// FUNCTION: SWBF2 0x00463720
const PblMatrix& CollisionModel::GetBodyMatrix(int id) const
{
    if (id < mNumBodies && id >= 0)
    {
        return mBody[id]->mMatrix;
    }
    return _IdentityMatrix;
}

// FUNCTION: SWBF2 0x00463750
PblMatrix CollisionModel::GetWorldMatrix(int id, const PblMatrix& matrix, const RedPose* pPose) const
{
    if (id < mNumBodies && id >= 0)
    {
        return mBody[id]->GetWorldMatrix(matrix, pPose);
    }
    return matrix;
}

// FUNCTION: SWBF2 0x004637b0
PblMatrix CollisionModel::GetLocalMatrix(int id, const RedPose* pPose) const
{
    if (id < mNumBodies && id >= 0)
    {
        return mBody[id]->GetLocalMatrix(pPose);
    }
    return _IdentityMatrix;
}

// FUNCTION: SWBF2 0x00463820
const CollisionMask& CollisionModel::GetCollisionMask(const CollisionObjectType& eType) const
{
    switch (eType)
    {
    case COLL_ASTEROID:
    case COLL_RIGID:
        return mRigidMask;
    case COLL_STATIC:
        return mStaticMask;
    case COLL_SPECIAL:
        return mFlyingOrdnanceMask;
    case COLL_TERRAIN:
        return mTerrainMask;
    case COLL_ORDNANCE:
        return mOrdnanceMask;
    case COLL_TOWCABLE:
        return mTowCableMask;
    case COLL_TARGETABLE:
        return mTargetableMask;
    default:
        return mSoftMask;
    }
}

// FUNCTION: SWBF2 0x00463900
void CollisionModel::SetCollisionMask(const CollisionObjectType& eType, const CollisionMask& mask)
{
    switch (eType)
    {
    case COLL_ASTEROID:
    case COLL_RIGID:
        mRigidMask = mask;
        break;
    case COLL_STATIC:
        mStaticMask = mask;
        break;
    case COLL_TERRAIN:
        mTerrainMask = mask;
        break;
    case COLL_ORDNANCE:
        mOrdnanceMask = mask;
        break;
    case COLL_TOWCABLE:
        mTowCableMask = mask;
        break;
    case COLL_TARGETABLE:
        mTargetableMask = mask;
        break;
    default:
        mSoftMask = mask;
        break;
    }
}

// FUNCTION: SWBF2 0x00463a00
CollisionBody* CollisionModel::SetBody(int id, CollisionBody* pBody)
{
    if (id >= mNumBodies || id < 0)
    {
        return 0;
    }
    CollisionBody* pOld = mBody[id];
    mBody[id] = pBody;
    return pOld;
}

// FUNCTION: SWBF2 0x004639e0
CollisionBody* CollisionModel::GetBody(int id)
{
    if (id >= mNumBodies || id < 0)
    {
        return 0;
    }
    return mBody[id];
}

// GLOBAL: SWBF2 0x01eb55e0
// SYNTHETIC: SWBF2 0x004024b0
// `dynamic initializer for 'CollisionModel::sClientCollisionMask''
CollisionMask CollisionModel::sClientCollisionMask[64];

// SYNTHETIC: SWBF2 0x00478410
// CollisionModel::operator=
