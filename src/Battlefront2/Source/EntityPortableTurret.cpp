#include "EntityPortableTurret.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblVector.h"

// FUNCTION: SWBF2 0x004d89d0
EntityPortableTurretClass::EntityPortableTurretClass(unsigned int uHash)
    : EntityBuildingArmedDynamicClass(uHash)
{
    mLifeTime = FLT_MAX;
    mGravity = 1.0f;
    mRebound = 0.3f;
    mFriction = 0.5f;
}

// GLOBAL: SWBF2 0x01ebbf94
// SYNTHETIC: SWBF2 0x00402cb0
// `dynamic initializer for 'EntityPortableTurretClass::rttiHashEntityPortableTurretClass''
PblHash EntityPortableTurretClass::rttiHashEntityPortableTurretClass("EntityPortableTurretClass");
char EntityPortableTurretClass::rttiNameEntityPortableTurretClass[] = "EntityPortableTurretClass";

// GLOBAL: SWBF2 0x01ebbfe8
// SYNTHETIC: SWBF2 0x00402ca0
// `dynamic initializer for 'EntityPortableTurret::rttiHashEntityPortableTurret''
PblHash EntityPortableTurret::rttiHashEntityPortableTurret("EntityPortableTurret");
char EntityPortableTurret::rttiNameEntityPortableTurret[] = "EntityPortableTurret";

// GLOBAL: SWBF2 0x01ebbff0
// SYNTHETIC: SWBF2 0x00402c80
// `dynamic initializer for 'EntityPortableTurret::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f90
// `dynamic atexit destructor for 'EntityPortableTurret::sMemoryPool''
MemoryPool EntityPortableTurret::sMemoryPool("EntityPortableTurret", 0x6d0);

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern PblVector3 gVectorZero;

extern bool netFrameLock;
extern bool netIsLocalTurn;
extern bool netIsLateTurn;

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

// FUNCTION: SWBF2 0x004d7b90
bool EntityPortableTurret::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityPortableTurret || EntityBuildingArmedDynamic::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004d7cf0
EntityPortableTurret::EntityPortableTurret(EntityPortableTurretClass* pClass, const EntityDesc& desc)
    : EntityBuildingArmedDynamic(pClass, desc)
{
    mClass = pClass;
    mLifeTimer = 0.0f;
    mVelocity = gVectorZero;
    mOwnerChar = 0;
    mStationary = false;
    mBecomeStationaryNow = false;

    PblVector3& vRight = *(PblVector3*)mMatrix.m[0];
    PblVector3& vUp = *(PblVector3*)mMatrix.m[1];
    PblVector3& vFront = *(PblVector3*)mMatrix.m[2];
    vUp = gVectorUnitY;
    mMatrix.m[1][3] = 0.0f;
    PblVector3 vCross(vUp.y * vFront.z - vUp.z * vFront.y, vUp.z * vFront.x - vUp.x * vFront.z,
                      vUp.x * vFront.y - vUp.y * vFront.x);
    PblVector3 vNorm;
    D3DXVec3Normalize(&vNorm, &vCross);
    vRight = vNorm;
    mMatrix.m[0][3] = 0.0f;
    vCross = PblVector3(vRight.y * vUp.z - vRight.z * vUp.y, vRight.z * vUp.x - vRight.x * vUp.z,
                        vRight.x * vUp.y - vRight.y * vUp.x);
    D3DXVec3Normalize(&vNorm, &vCross);
    vFront = vNorm;
    mMatrix.m[2][3] = 0.0f;

    PblBox box;
    box._vMin = PblVector3(FLT_MAX, FLT_MAX, FLT_MAX);
    box._vMax = PblVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    bool bHaveBody = false;
    for (int i = 0; i < mCollisionModel->GetNumBodies(); ++i)
    {
        CollisionBody* pBody = mCollisionModel->GetBody(i);
        if (!pBody)
        {
            continue;
        }
        bHaveBody = true;
        PblBox bodyBox;
        if (pBody->bIsMesh)
        {
            bodyBox = static_cast<CollisionBodyMesh*>(pBody)->mMesh->GetWorldspaceAABB(
                pBody->GetWorldMatrix(mMatrix, &mPose));
        }
        else
        {
            bodyBox = static_cast<CollisionBodyPrimitive*>(pBody)->mPrimitive.GetWorldspaceAABB(
                pBody->GetWorldMatrix(mMatrix, &mPose));
        }
        box._vMin.x = bodyBox._vMin.x > box._vMin.x ? box._vMin.x : bodyBox._vMin.x;
        box._vMin.y = bodyBox._vMin.y > box._vMin.y ? box._vMin.y : bodyBox._vMin.y;
        box._vMin.z = bodyBox._vMin.z > box._vMin.z ? box._vMin.z : bodyBox._vMin.z;
        box._vMax.x = box._vMax.x > bodyBox._vMax.x ? box._vMax.x : bodyBox._vMax.x;
        box._vMax.y = box._vMax.y > bodyBox._vMax.y ? box._vMax.y : bodyBox._vMax.y;
        box._vMax.z = box._vMax.z > bodyBox._vMax.z ? box._vMax.z : bodyBox._vMax.z;
    }
    if (bHaveBody)
    {
        mAABB = box;
    }
}

// SYNTHETIC: SWBF2 0x004d7c50
// EntityPortableTurret::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004d8d10
// [thunk]:EntityPortableTurret::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x004d8d1b
// [thunk]:EntityPortableTurret::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004d8d23
// [thunk]:EntityPortableTurret::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x004d8d39
// [thunk]:EntityPortableTurret::`scalar deleting destructor' adjustor{148}

// SYNTHETIC: SWBF2 0x004d8d2e
// [thunk]:EntityPortableTurret::`scalar deleting destructor'`adjustor{608}'

// FUNCTION: SWBF2 0x004d8230
bool EntityPortableTurret::Update(float fDeltaTime)
{
    if (mClass->mLifeTime > 0.0f)
    {
        mLifeTimer += fDeltaTime;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) &&
        ((!IsAlive() && !mClass->mDestroyedModel && !mClass->mUnbuiltModel) || mLifeTimer > mClass->mLifeTime ||
         mMatrix.m[3][1] < -500.0f))
    {
        Kill();
        return false;
    }

    if (mBecomeStationaryNow)
    {
        mStationary = true;
        mBecomeStationaryNow = false;
        CollisionManager::RemoveBody(this);
        CollisionManager::AddRigidBody(this);
    }

    if (!mStationary)
    {
        float fHalfDrop = mClass->mGravity * -9.8f * 0.5f * fDeltaTime;
        mVelocity.y += fHalfDrop;
        PblVector3 vStep(mVelocity.x * fDeltaTime, mVelocity.y * fDeltaTime, mVelocity.z * fDeltaTime);
        const PblVector3& vPos = GetPosition();
        SetPosition(PblVector3(vPos.x + vStep.x, vPos.y + vStep.y, vPos.z + vStep.z));
        mAABB._vMax.x += mVelocity.x * fDeltaTime;
        mAABB._vMax.y += mVelocity.y * fDeltaTime;
        mAABB._vMax.z += mVelocity.z * fDeltaTime;
        mAABB._vMin.x += mVelocity.x * fDeltaTime;
        mAABB._vMin.y += mVelocity.y * fDeltaTime;
        mAABB._vMin.z += mVelocity.z * fDeltaTime;
        mVelocity.y += fHalfDrop;
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mMatrix.m[3][1] < -500.0f)
        {
            Kill();
            return false;
        }
    }

    if (netFrameLock || !NetGame::IsEnabled())
    {
        return true;
    }
    if (!(NetGame::IsClient() && netIsLocalTurn))
    {
        if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
        {
            return true;
        }
        if (mNetUniqueId >= 0)
        {
            mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
        }
    }
    gameObjectInterpolator.SetNext(mNetUniqueId, mStationary ? 0 : &mMatrix);
    return true;
}

// FUNCTION: SWBF2 0x004d85a0
bool EntityPortableTurret::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                             Restrictor* pRestrictor)
{
    if (mStationary)
    {
        return false;
    }

    PblVector3 vOldPos = GetPosition();

    if (pOther->GetCollisionType() == COLL_LEAFPATCH)
    {
        mClass->mCollisionFoliageSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
    }

    if (pOther->GetCollisionType() != COLL_SOFT && pOther->GetCollisionType() != COLL_RIGID)
    {
        const PblVector3& vNormal = pResult->mSeparationNormal;
        float fDot = vNormal.x * mVelocity.x + mVelocity.y * vNormal.y + mVelocity.z * vNormal.z;
        if (fDot < 0.0f)
        {
            (pOther->GetCollisionType() == COLL_WATER ? mClass->mCollisionWaterSound : mClass->mCollisionSound)
                .Play((const PblVector3*)mMatrix.m[3], 0, false, true);
            PblVector3 vUnit;
            D3DXVec3Normalize(&vUnit, &vNormal);
            if (vUnit.y > 0.7f)
            {
                mMatrix = NormalMatrix(*(const PblVector3*)mMatrix.m[2], vNormal, *(const PblVector3*)mMatrix.m[3]);
                mVelocity = gVectorZero;
                mBecomeStationaryNow = true;
            }
            else
            {
                float fRebound = mClass->mRebound;
                float fFriction = mClass->mFriction * fDot * (fRebound + 1.0f);
                mVelocity.x -= vNormal.x * fDot;
                mVelocity.y -= vNormal.y * fDot;
                mVelocity.z -= vNormal.z * fDot;
                float fScale = -(fFriction / sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                                   mVelocity.z * mVelocity.z));
                if (!(1.0f > fScale))
                {
                    fScale = 1.0f;
                }
                mVelocity.x -= fScale * mVelocity.x;
                mVelocity.y -= mVelocity.y * fScale;
                mVelocity.z -= mVelocity.z * fScale;
                float fBounce = fRebound * fDot;
                mVelocity.x -= vNormal.x * fBounce;
                mVelocity.y -= vNormal.y * fBounce;
                mVelocity.z -= vNormal.z * fBounce;
            }
        }
    }

    bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    const PblVector3& vPos = GetPosition();
    PblVector3 vDelta(vPos.x - vOldPos.x, vPos.y - vOldPos.y, vPos.z - vOldPos.z);
    mAABB._vMax.x += vDelta.x;
    mAABB._vMax.y += vDelta.y;
    mAABB._vMax.z += vDelta.z;
    mAABB._vMin.x += vDelta.x;
    mAABB._vMin.y += vDelta.y;
    mAABB._vMin.z += vDelta.z;
    return bResult;
}

// FUNCTION: SWBF2 0x004d7bf0
unsigned int EntityPortableTurret::GetDerivedRtti() const
{
    return rttiHashEntityPortableTurret;
}

// FUNCTION: SWBF2 0x004d7c00
const char* EntityPortableTurret::GetDerivedRttiName() const
{
    return rttiNameEntityPortableTurret;
}

// FUNCTION: SWBF2 0x004d7c20
const PblVector3& EntityPortableTurret::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x004d7c30
void EntityPortableTurret::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x004d8220
void EntityPortableTurret::RecalculateAABB()
{
}

// FUNCTION: SWBF2 0x004d7c80
bool EntityPortableTurretClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityPortableTurretClass || EntityBuildingArmedDynamicClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004d7cd0
unsigned int EntityPortableTurretClass::GetDerivedRtti()
{
    return rttiHashEntityPortableTurretClass;
}

// FUNCTION: SWBF2 0x004d7ce0
const char* EntityPortableTurretClass::GetDerivedRttiName()
{
    return rttiNameEntityPortableTurretClass;
}

// FUNCTION: SWBF2 0x004d8a70
EntityPortableTurretClass::EntityPortableTurretClass(EntityPortableTurretClass* pBase, unsigned int uHash)
    : EntityBuildingArmedDynamicClass(pBase, uHash)
{
    mLifeTime = pBase->mLifeTime;
    mGravity = pBase->mGravity;
    mRebound = pBase->mRebound;
    mFriction = pBase->mFriction;
    mCollisionSound = pBase->mCollisionSound;
    mCollisionFoliageSound = pBase->mCollisionFoliageSound;
    mCollisionWaterSound = pBase->mCollisionWaterSound;
}

// FUNCTION: SWBF2 0x004d8b50
EntityPortableTurretClass::~EntityPortableTurretClass()
{
}

// SYNTHETIC: SWBF2 0x004d8a40
// EntityPortableTurretClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004d8b60
EntityClass* EntityPortableTurretClass::Derive(unsigned int uHash)
{
    return new EntityPortableTurretClass(this, uHash);
}

// FUNCTION: SWBF2 0x004d8bd0
Entity* EntityPortableTurretClass::Build(const EntityDesc& desc)
{
    if (EntityPortableTurret::sMemoryPool.mUsed >= EntityPortableTurret::sMemoryPool.mCount)
    {
        return 0;
    }

    EntityPortableTurret* pTurret = new EntityPortableTurret(this, desc);
    if (pTurret)
    {
        CollisionManager::AddSoftBody(pTurret);
        pTurret->ActivatePhysics();
    }
    return pTurret;
}

// FUNCTION: SWBF2 0x004d8c70
void EntityPortableTurretClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x5212ABEEu:
        sscanf(pszValue, "%f", &mLifeTime);
        break;
    case 0xD209FF45u:
        sscanf(pszValue, "%f", &mGravity);
        break;
    case 0x678DE338u:
        sscanf(pszValue, "%f", &mRebound);
        break;
    case 0xA51BE2BBu:
        sscanf(pszValue, "%f", &mFriction);
        break;
    case 0xC28F0C96u:
    case 0xEDC6251Au:
        mCollisionSound.SetID(pszValue, false, false);
        break;
    case 0x2297E47Du:
        mCollisionWaterSound.SetID(pszValue, false, false);
        break;
    default:
        EntityBuildingArmedClass::SetProperty(uProperty, pszValue);
        break;
    }
}
