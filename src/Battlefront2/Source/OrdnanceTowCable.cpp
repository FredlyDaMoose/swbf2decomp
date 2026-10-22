#include <math.h>
#include <stdio.h>
#include <string.h>
#include <d3dx9math.h>

#include "OrdnanceTowCable.h"
#include "Aimer.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "EntityWalker.h"
#include "GameObject.h"
#include "WeaponTowCable.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHermite.h"
#include "PebbleFL/Common/PblRandom.h"

class AttachedEffects;
class Controllable;
class GameModel;
class RedPose;

void sRenderHermiteSegment(PblHermiteVector3& segment, unsigned char ucViewport, float fDist,
                           unsigned int uiFlags);

extern PblVector3 gVectorZero;

// GLOBAL: SWBF2 0x01ef26c0
// SYNTHETIC: SWBF2 0x00405bc0
// `dynamic initializer for 'OrdnanceTowCable::rttiHashOrdnanceTowCable''
PblHash OrdnanceTowCable::rttiHashOrdnanceTowCable("OrdnanceTowCable");
char OrdnanceTowCable::rttiNameOrdnanceTowCable[] = "OrdnanceTowCable";

// GLOBAL: SWBF2 0x01ef26c4
// SYNTHETIC: SWBF2 0x00405bd0
// `dynamic initializer for 'OrdnanceTowCableClass::rttiHashOrdnanceTowCableClass''
PblHash OrdnanceTowCableClass::rttiHashOrdnanceTowCableClass("OrdnanceTowCableClass");
char OrdnanceTowCableClass::rttiNameOrdnanceTowCableClass[] = "OrdnanceTowCableClass";

// GLOBAL: SWBF2 0x01ef2670
// SYNTHETIC: SWBF2 0x00405ba0
// `dynamic initializer for 'OrdnanceTowCable::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769200
// `dynamic atexit destructor for 'OrdnanceTowCable::sMemoryPool''
MemoryPool OrdnanceTowCable::sMemoryPool("OrdnanceTowCable", 0x670);

// GLOBAL: SWBF2 0x01eaed60
unsigned int OrdnanceTowCable::sTextureHash;

// FUNCTION: SWBF2 0x006072a0
bool OrdnanceTowCable::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceTowCable || OrdnanceBullet::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006072d0
unsigned int OrdnanceTowCable::GetDerivedRtti() const
{
    return rttiHashOrdnanceTowCable;
}

// FUNCTION: SWBF2 0x006072e0
const char* OrdnanceTowCable::GetDerivedRttiName() const
{
    return rttiNameOrdnanceTowCable;
}

// SYNTHETIC: SWBF2 0x00609c29
// [thunk]:OrdnanceTowCable::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x00609c31
// [thunk]:OrdnanceTowCable::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x00607300
// OrdnanceTowCable::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00607330
bool OrdnanceTowCableClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceTowCableClass || OrdnanceBulletClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00607360
unsigned int OrdnanceTowCableClass::GetDerivedRtti()
{
    return rttiHashOrdnanceTowCableClass;
}

// FUNCTION: SWBF2 0x00607370
const char* OrdnanceTowCableClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceTowCableClass;
}

// FUNCTION: SWBF2 0x00607380
OrdnanceTowCable::OrdnanceTowCable(OrdnanceTowCableClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc)
{
    sTextureHash = 0;
    if (desc.mFireWeapon->IsRtti(WeaponTowCable::rttiHashWeaponTowCable))
    {
        mFireWeapon = (WeaponTowCable*)desc.mFireWeapon;
    }
    else
    {
        mFireWeapon = NULL;
    }
    mHitWalker = NULL;
    mLastOwnerFirePos = mFireWeapon->mAimer->mFirePos;
    mLastOwnerFireDir = mFireWeapon->mAimer->mDirection;
    mTowCableFallTime = 0.0f;
    mFakeCable = false;
    mDeleteNextUpdate = false;
}

// FUNCTION: SWBF2 0x006074a0
OrdnanceTowCable::~OrdnanceTowCable()
{
    if (mFireWeapon && mOwner)
    {
        if (NetGame::IsEnabled() && !NetGame::IsClient())
        {
            NetGame::CreateEvent_DestroyTowCable(mOwner, mFireWeapon, (unsigned __int64)-1);
        }
        mFireWeapon->SetOrdnance(NULL);
        mFireWeapon = NULL;
    }
    mTowCable.Clear();
}

// FUNCTION: SWBF2 0x006075a0
void OrdnanceTowCable::TowCableFailure()
{
    if (mTowCable.mTowState == TOW_FALL)
    {
        return;
    }

    CollisionManager::RemoveOrdnance(this);

    if (mOwner && mFireWeapon)
    {
        mFireWeapon->SetOrdnance(NULL);
        mFireWeapon = NULL;
        ((EntityFlyer*)mOwner->GetEntity())->SetTowCableWalker(NULL);
    }

    PblVector3 vSlope(mLastOwnerFireDir.x * -15.0f, mLastOwnerFireDir.y * -15.0f,
                      mLastOwnerFireDir.z * -15.0f);
    mTowCable.AddAttachPoint(0, mLastOwnerFirePos, vSlope);
    ConvertPointsToWorldSpace();

    for (int i = 0; i < 13; i++)
    {
        TowCableAttachPointData& point = mTowCable.mAttachPoints[i];
        if (!point.mIsAttached)
        {
            break;
        }

        PblVector3 vStart(point.mBodyOffset.x + 0.0f, point.mBodyOffset.y + 20.0f,
                          point.mBodyOffset.z + 0.0f);
        PblVector3 vDown(0.0f, -1.0f, 0.0f);
        CollisionObject* pHit;
        float fHit = CollisionManager::RayHit(vStart, vDown, 512.0f, pHit, NULL, NULL, 0, 0x90, true);
        if (fHit < 1.0f)
        {
            point.mFinalHeight = point.mBodyOffset.y + 0.05f + 20.0f - fHit * 512.0f;
        }
        else
        {
            point.mFinalHeight = -512.0f;
        }
    }

    mTowCable.mTowState = TOW_FALL;
}

// FUNCTION: SWBF2 0x00607760
bool OrdnanceTowCable::Collide(CollisionObject* pObject, const PblVector3& vPos, const PblVector3& vNormal,
                               int iBodyId)
{
    if (mFakeCable)
    {
        mDeleteNextUpdate = true;
        return false;
    }

    CollisionModel* pModel = pObject->mCollisionModel;
    CollisionObjectType eType = COLL_TOWCABLE;
    if (pModel && pModel->GetCollisionMask(eType).IsSet(iBodyId))
    {
        Entity* pEntity = pObject->GetEntity();
        if (pEntity && pEntity->IsRtti(EntityWalker::rttiHashEntityWalker))
        {
            mTowCable.mTowState = TOW_WRAP;
            mHitWalker = pEntity->GetGameObject();
            mTowCable.Clear();

            CollisionObject* pWalkerCollision = mHitWalker->GetCollisionObject();
            PblMatrix mBody = pModel->GetWorldMatrix(iBodyId, mHitWalker->GetMatrix(), pWalkerCollision->GetPose());
            PblMatrix mInverse = mBody.GetInverseFast();
            PblVector3 vLocal;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vPos, (const D3DXMATRIX*)&mInverse);
            mTowCable.AddAttachPoint(iBodyId, vLocal, gVectorZero);
            CollisionManager::RemoveOrdnance(this);

            if (mOwner)
            {
                ((EntityFlyer*)mOwner->GetEntity())->SetTowCableWalker(mHitWalker);
            }

            OrdnanceDesc desc;
            if (NetGame::IsEnabled() && !NetGame::IsClient())
            {
                PblVector3 vDir;
                D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);
                NetGame::CreateEvent_ConnectTowCable(mOwner, mFireWeapon, vDir, vPos, iBodyId, mHitWalker,
                                                     (unsigned __int64)-1);
            }
            return false;
        }
    }

    mTowCable.AddAttachPoint(0, GetPosition(), gVectorZero);
    TowCableFailure();
    CollisionManager::RemoveOrdnance(this);
    return false;
}

// FUNCTION: SWBF2 0x00607a10
void OrdnanceTowCable::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (mOwner && (mTowCable.mTowState == TOW_WRAP || mTowCable.mTowState == TOW_ATTACHING))
    {
        PblVector3 vFirePos = mLastOwnerFirePos;
        PblVector3 vEndPos = GetPosition();
        if (mOwner)
        {
            PblMatrix mOwnerMatrix = mOwner->GetMatrix();
            PblMatrix mSmoothed = mOwner->GetSmoothedMatrix();
            PblMatrix mInverse = mOwnerMatrix.GetInverseFast();
            PblVector3 v;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&v, (const D3DXVECTOR3*)&vFirePos, (const D3DXMATRIX*)&mInverse);
            vFirePos = v;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&v, (const D3DXVECTOR3*)&vFirePos, (const D3DXMATRIX*)&mSmoothed);
            vFirePos = v;
        }

        PblVector3 vEndSlope;
        const PblVector3* pEndSlope;
        if (mHitWalker)
        {
            int i;
            for (i = 0; i < 13; i++)
            {
                if (!mTowCable.mAttachPoints[i].mIsAttached)
                {
                    break;
                }
            }
            if (i >= 13)
            {
                return;
            }

            TowCableAttachPointData& point = mTowCable.mAttachPoints[i - 1];
            CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
            CollisionObject* pWalkerCollision = mHitWalker->GetCollisionObject();
            PblMatrix mBody =
                pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pWalkerCollision->GetPose());
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vEndPos, (const D3DXVECTOR3*)&point.mBodyOffset,
                                   (const D3DXMATRIX*)&mBody);
            pEndSlope = &point.mSlopeOut;
        }
        else
        {
            vEndSlope = PblVector3(0.0f, 0.0f, 0.0f);
            pEndSlope = &vEndSlope;
        }

        PblVector3 vFireSlope(mLastOwnerFireDir.x * -15.0f, mLastOwnerFireDir.y * -15.0f,
                              mLastOwnerFireDir.z * -15.0f);
        PblHermiteVector3 segment(vEndPos, vFirePos, *pEndSlope, vFireSlope, 1.0f);
        sRenderHermiteSegment(segment, ucViewport, fDist, uiFlags);
    }

    if (mTowCable.mTowState != TOW_ATTACHING)
    {
        float fFade = fDist;
        if (mTowCableFallTime > 8.0f)
        {
            fFade = (10.0f - mTowCableFallTime) * 0.5f * fDist;
        }
        mTowCable.Render(mHitWalker, ucViewport, fFade, uiFlags);
    }
}

// FUNCTION: SWBF2 0x00607e70
void OrdnanceTowCable::RecalculateCenterAndRadius()
{
    if (!mOwner)
    {
        return;
    }
    if (mTowCable.mTowState == TOW_DAMAGE || mTowCable.mTowState == TOW_FALL)
    {
        return;
    }

    PblVector3 vOwnerPos = mOwner->GetCollisionObject()->GetCollisionCenter();
    float fOwnerRadius = mOwner->GetCollisionObject()->GetCollisionRadius();
    PblVector3 vEndPos = GetPosition();
    float fEndRadius = _Sphere.w;
    if (mHitWalker)
    {
        vEndPos = mHitWalker->GetCollisionObject()->GetCollisionCenter();
        fEndRadius = mHitWalker->GetCollisionObject()->GetCollisionRadius();
    }

    PblVector3 vDelta(vOwnerPos.x - vEndPos.x, vOwnerPos.y - vEndPos.y, vOwnerPos.z - vEndPos.z);
    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vDelta);
    if (vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z < 0.0001f)
    {
        SetRenderRadius(fOwnerRadius > fEndRadius ? fOwnerRadius : fEndRadius);
        SetRenderPos(vOwnerPos);
        return;
    }

    PblVector3 vOwnerEdge(vDir.x * fOwnerRadius + vOwnerPos.x, vDir.y * fOwnerRadius + vOwnerPos.y,
                          vDir.z * fOwnerRadius + vOwnerPos.z);
    PblVector3 vEndEdge(vEndPos.x - vDir.x * fEndRadius, vEndPos.y - vDir.y * fEndRadius,
                        vEndPos.z - vDir.z * fEndRadius);
    float fX = vOwnerEdge.x - vEndEdge.x;
    float fY = vOwnerEdge.y - vEndEdge.y;
    float fZ = vOwnerEdge.z - vEndEdge.z;
    SetRenderRadius(sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f);
    PblVector3 vCenter((vEndEdge.x + vOwnerEdge.x) * 0.5f, (vEndEdge.y + vOwnerEdge.y) * 0.5f,
                       (vEndEdge.z + vOwnerEdge.z) * 0.5f);
    SetRenderPos(vCenter);
}

// FUNCTION: SWBF2 0x006081c0
bool OrdnanceTowCable::Update(float fDeltaTime)
{
    if (mDeleteNextUpdate)
    {
        return false;
    }

    if (mOwner)
    {
        if (mFireWeapon)
        {
            mLastOwnerFirePos = mFireWeapon->mAimer->mFirePos;
            mLastOwnerFireDir = mFireWeapon->mAimer->mDirection;
        }
    }
    else if (mTowCable.mTowState != TOW_DAMAGE)
    {
        TowCableFailure();
    }

    if (mTowCable.mTowState != TOW_WRAP && mTowCable.mTowState != TOW_DAMAGE &&
        mTowCable.mTowState != TOW_FALL)
    {
        bool bResult = OrdnanceBullet::Update(fDeltaTime);
        RecalculateCenterAndRadius();
        const PblVector3& vPos = GetPosition();
        float fX = mLastOwnerFirePos.x - vPos.x;
        float fY = mLastOwnerFirePos.y - vPos.y;
        float fZ = mLastOwnerFirePos.z - vPos.z;
        float fDist = sqrtf(fY * fY + fX * fX + fZ * fZ);
        if (fDist > ((OrdnanceTowCableClass*)mClass)->mInitialCableLimit)
        {
            return false;
        }
        return bResult;
    }

    RecalculateCenterAndRadius();
    if (mHitWalker)
    {
        if (!mHitWalker->mIsAlive)
        {
            CalculateWorldSpacePoints();
        }
    }
    else
    {
        TowCableFailure();
    }

    int iNumPoints;
    for (iNumPoints = 0; iNumPoints < 13; iNumPoints++)
    {
        if (!mTowCable.mAttachPoints[iNumPoints].mIsAttached)
        {
            break;
        }
    }

    if (mHitWalker)
    {
        mTowCable.UpdateAttachPoints(fDeltaTime, mHitWalker);
        if (mTowCable.mTowState == TOW_DAMAGE)
        {
            int iPoint = (int)(PblRandom::GetGlobalFloat() * (float)(iNumPoints - 1));
            TowCableAttachPointData& point = mTowCable.mAttachPoints[iPoint];
            CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
            const RedPose* pPose = mHitWalker->GetCollisionObject()->GetPose();

            PblMatrix mBody = pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pPose);
            PblVector3 vPos;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vPos, (const D3DXVECTOR3*)&point.mBodyOffset,
                                   (const D3DXMATRIX*)&mBody);
            mBody = pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pPose);
            PblVector3 vNormal;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vNormal, (const D3DXVECTOR3*)&point.mBodyOffset,
                                    (const D3DXMATRIX*)&mBody);
            PblVector3 vDir;
            D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vNormal);

            DamageDesc damage = GetDamage();
            damage.mBaseValue = mHitWalker->mMaxHealth;
            float fSameTeamDamageScale = Damageable::sSameTeamDamageScale;
            Damageable::SetTeamDamageScale(1.0f);
            if (mOwner)
            {
                for (PblList<Controllable>::Itor itor(mOwner->mControllableList); (Controllable*)itor; ++itor)
                {
                    if (!itor->GetPilot())
                    {
                        continue;
                    }
                    damage.mDamageOwner.mCharacter = itor->GetPilot()->mCharacter;
                    mHitWalker->ApplyDamage(damage, vDir, vPos, point.mBodyID, (unsigned int)-1);
                }
            }
            Damageable::SetTeamDamageScale(fSameTeamDamageScale);
        }
    }
    else
    {
        mTowCable.UpdateAttachPoints(fDeltaTime, NULL);
    }

    if (mTowCable.mTowState == TOW_FALL)
    {
        mTowCableFallTime += fDeltaTime;
    }
    if (mTowCableFallTime > 10.0f)
    {
        return false;
    }

    if (!mOwner || !mHitWalker || mTowCable.mTowState != TOW_WRAP)
    {
        return true;
    }

    if (iNumPoints < 13)
    {
        TowCableAttachPointData& last = mTowCable.mAttachPoints[iNumPoints - 1];
        CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
        CollisionObject* pWalkerCollision = mHitWalker->GetCollisionObject();
        PblMatrix mBody = pModel->GetWorldMatrix(last.mBodyID, mHitWalker->GetMatrix(), pWalkerCollision->GetPose());
        PblVector3 vStart;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vStart, (const D3DXVECTOR3*)&last.mBodyOffset,
                               (const D3DXMATRIX*)&mBody);
        PblVector3 vFirePos = mLastOwnerFirePos;
        int iLastBodyId = last.mBodyID;
        int iPrevBodyId = -1;
        if (iNumPoints > 2)
        {
            iPrevBodyId = mTowCable.mAttachPoints[iNumPoints - 2].mBodyID;
        }

        PblVector3 vFireSlope(mLastOwnerFireDir.x * -15.0f, mLastOwnerFireDir.y * -15.0f,
                              mLastOwnerFireDir.z * -15.0f);
        PblHermiteVector3 segment(vStart, vFirePos, last.mSlopeOut, vFireSlope, 1.0f);
        OrdnanceTowCableClass* pClass = (OrdnanceTowCableClass*)mClass;
        float fLimit = iNumPoints == 1 ? pClass->mInitialCableLimit : pClass->mCableLimit;
        if (segment.GetLengthApproximation(10) > fLimit)
        {
            TowCableFailure();
            return true;
        }

        for (float fT = 0.0f; fT < 1.0f; fT += 0.1f)
        {
            PblVector3 vA = segment.getValue(fT);
            PblVector3 vB = segment.getValue(fT + 0.1f);
            float fX = vB.x - vA.x;
            float fY = vB.y - vA.y;
            float fZ = vB.z - vA.z;
            float fLength = sqrtf(fY * fY + fX * fX + fZ * fZ);
            if (fLength <= 0.0f)
            {
                continue;
            }

            float fInvLength = 1.0f / fLength;
            PblVector3 vDir(fX * fInvLength, fInvLength * fY, fInvLength * fZ);
            PblVector3 vNormal;
            int iBodyId;
            float fHit = mHitWalker->GetCollisionObject()->RayHit(vA, vDir, fLength, &vNormal, &iBodyId, true, 0.0f);
            if (fHit >= 1.0f)
            {
                continue;
            }

            CollisionObjectType eType = COLL_TOWCABLE;
            CollisionMask mask = pModel->GetCollisionMask(eType);
            if (!mask.IsSet(iBodyId))
            {
                TowCableFailure();
                return true;
            }
            if (iBodyId == last.mBodyID)
            {
                continue;
            }

            int iValid = ((EntityWalker*)mHitWalker->GetEntity())
                             ->IsTowCableCollisionValid(iBodyId, iLastBodyId, iPrevBodyId);
            if (iValid > 0)
            {
                PblMatrix mHitBody =
                    pModel->GetWorldMatrix(iBodyId, mHitWalker->GetMatrix(), mHitWalker->GetCollisionObject()->GetPose());
                PblVector3 vHit(vDir.x * fHit + vA.x, vDir.y * fHit + vA.y, vDir.z * fHit + vA.z);
                PblMatrix mInverse = mHitBody.GetInverseFast();
                PblVector3 vLocal;
                D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vHit,
                                       (const D3DXMATRIX*)&mInverse);
                PblVector3 vSlope = segment.getSlope(fHit * 0.1f + fT);
                vSlope.x *= 0.5f;
                vSlope.y *= 0.5f;
                vSlope.z *= 0.5f;
                mTowCable.AddAttachPoint(iBodyId, vLocal, vSlope);
                return true;
            }
            if (iValid < 0)
            {
                TowCableFailure();
                return true;
            }
        }
        return true;
    }

    mTowCable.mTowState = TOW_DAMAGE;
    if (mOwner)
    {
        ((EntityFlyer*)mOwner->GetEntity())->SetTowCableWalker(NULL);
        if (mFireWeapon)
        {
            mFireWeapon->SetOrdnance(NULL);
            mFireWeapon = NULL;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00608e90
void OrdnanceTowCable::ConvertPointsToWorldSpace()
{
    if (mHitWalker)
    {
        CalculateWorldSpacePoints();
    }

    for (int i = 0; i < 13; i++)
    {
        TowCableAttachPointData& point = mTowCable.mAttachPoints[i];
        if (!point.mIsAttached)
        {
            break;
        }
        if (point.mBodyID >= 0)
        {
            point.mBodyOffset = point.mWorldBodyOffset;
            point.mInitialBodyOffset = point.mWorldInitialBodyOffset;
            point.mBodyID = -1;
        }
    }
}

// FUNCTION: SWBF2 0x00608f10
void OrdnanceTowCable::CalculateWorldSpacePoints()
{
    if (!mHitWalker)
    {
        return;
    }

    for (int i = 0; i < 13; i++)
    {
        TowCableAttachPointData& point = mTowCable.mAttachPoints[i];
        if (!point.mIsAttached)
        {
            return;
        }

        if (point.mBodyID < 0)
        {
            point.mWorldBodyOffset = point.mBodyOffset;
            point.mWorldInitialBodyOffset = point.mInitialBodyOffset;
            continue;
        }

        CollisionModel* pModel = mHitWalker->GetCollisionObject()->mCollisionModel;
        const RedPose* pPose = mHitWalker->GetCollisionObject()->GetPose();
        PblMatrix mBody = pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pPose);
        PblVector3 vWorld;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorld, (const D3DXVECTOR3*)&point.mBodyOffset,
                               (const D3DXMATRIX*)&mBody);
        mBody = pModel->GetWorldMatrix(point.mBodyID, mHitWalker->GetMatrix(), pPose);
        PblVector3 vWorldInitial;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorldInitial, (const D3DXVECTOR3*)&point.mInitialBodyOffset,
                               (const D3DXMATRIX*)&mBody);
        point.mWorldBodyOffset = vWorld;
        point.mWorldInitialBodyOffset = vWorldInitial;
    }
}

// FUNCTION: SWBF2 0x00609110
OrdnanceTowCableClass::OrdnanceTowCableClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mInitialCableLimit = 50.0f;
    mCableLimit = 45.0f;
}

// SYNTHETIC: SWBF2 0x006091b0
// OrdnanceTowCableClass::~OrdnanceTowCableClass

// SYNTHETIC: SWBF2 0x00609140
// OrdnanceTowCableClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00609170
OrdnanceTowCableClass::OrdnanceTowCableClass(OrdnanceTowCableClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mInitialCableLimit = pBase->mInitialCableLimit;
    mCableLimit = pBase->mCableLimit;
}

// FUNCTION: SWBF2 0x006091c0
OrdnanceClass* OrdnanceTowCableClass::Derive(unsigned int uHash)
{
    return new OrdnanceTowCableClass(this, uHash);
}

// FUNCTION: SWBF2 0x00609230
Ordnance* OrdnanceTowCableClass::Build(const OrdnanceDesc& desc)
{
    if (OrdnanceTowCable::sMemoryPool.mUsed >= OrdnanceTowCable::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceTowCable* pCable = new OrdnanceTowCable(this, desc);
    pCable->ActivateThread(-40, PER_LOCAL_TURN);
    pCable->AddOrdnanceToCollisionManager();
    return pCable;
}

// FUNCTION: SWBF2 0x006092d0
void OrdnanceTowCableClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xE4C659B4:
        sscanf(pszValue, "%f", &mInitialCableLimit);
        break;
    case 0xD9B746B2:
        sscanf(pszValue, "%f", &mCableLimit);
        break;
    default:
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00609320
TowCableRenderObject::TowCableRenderObject()
{
    mTowState = TOW_ATTACHING;
    Clear();
}

// FUNCTION: SWBF2 0x00609340
void TowCableRenderObject::Clear()
{
    memset(mAttachPoints, 0, sizeof(mAttachPoints));
}

// FUNCTION: SWBF2 0x00609360
void TowCableRenderObject::AddAttachPoint(int iBodyID, const PblVector3& vOffset, const PblVector3& vSlope)
{
    int i;
    for (i = 0; i < 13; i++)
    {
        if (!mAttachPoints[i].mIsAttached)
        {
            break;
        }
    }
    if (i >= 13)
    {
        return;
    }

    TowCableAttachPointData& point = mAttachPoints[i];
    point.mBodyID = iBodyID;
    point.mIsAttached = true;
    point.mInitialBodyOffset = vOffset;
    point.mBodyOffset = vOffset;
    point.mSlopeIn = vSlope;
    point.mSlopeOut = vSlope;
    point.mFinalSlope = vSlope;
    point.mFinalSlope.y = 0.0f;
    float fLength = sqrtf(point.mFinalSlope.x * point.mFinalSlope.x + point.mFinalSlope.y * point.mFinalSlope.y +
                          point.mFinalSlope.z * point.mFinalSlope.z);
    if (fLength > 0.0f)
    {
        float fScale = 10.0f / fLength;
        point.mFinalSlope.x *= fScale;
        point.mFinalSlope.y *= fScale;
        point.mFinalSlope.z *= fScale;
    }
    point.mFallSpeed = 0.0f;
    point.mFinalHeight = 0.0f;
}

// FUNCTION: SWBF2 0x006094a0
void TowCableRenderObject::UpdateAttachPoints(float fDeltaTime, GameObject* pObject)
{
    if ((mTowState == TOW_WRAP || mTowState == TOW_DAMAGE) && pObject)
    {
        for (int i = 1; i < 13; i++)
        {
            TowCableAttachPointData& point = mAttachPoints[i];
            if (!point.mIsAttached)
            {
                break;
            }

            float fStep = fDeltaTime * 30.0f;
            PblVector3& vIn = point.mSlopeIn;
            float fLength = sqrtf(vIn.x * vIn.x + vIn.y * vIn.y + vIn.z * vIn.z);
            if (fStep < fLength)
            {
                vIn.x -= fStep * vIn.x * (1.0f / fLength);
                vIn.y -= fStep * vIn.y * (1.0f / fLength);
                vIn.z -= fStep * vIn.z * (1.0f / fLength);
            }
            else
            {
                vIn = gVectorZero;
            }

            PblVector3& vOut = mAttachPoints[i - 1].mSlopeOut;
            fLength = sqrtf(vOut.x * vOut.x + vOut.y * vOut.y + vOut.z * vOut.z);
            if (fStep < fLength)
            {
                vOut.x -= fStep * vOut.x * (1.0f / fLength);
                vOut.y -= fStep * vOut.y * (1.0f / fLength);
                vOut.z -= fStep * vOut.z * (1.0f / fLength);
            }
            else
            {
                vOut = gVectorZero;
            }

            PblVector3 vTarget = ((EntityWalker*)pObject->GetEntity())->GetTowCableCollisionOffset(point.mBodyID);
            vTarget.y = point.mBodyOffset.y;
            PblVector3 vDiff;
            vDiff.x = vTarget.x - point.mBodyOffset.x;
            vDiff.y = vTarget.y - point.mBodyOffset.y;
            vDiff.z = vTarget.z - point.mBodyOffset.z;
            fLength = sqrtf(vDiff.x * vDiff.x + vDiff.y * vDiff.y + vDiff.z * vDiff.z);
            if (fStep < fLength)
            {
                point.mBodyOffset.x += vDiff.x * fStep * (1.0f / fLength);
                point.mBodyOffset.y += vDiff.y * fStep * (1.0f / fLength);
                point.mBodyOffset.z += vDiff.z * fStep * (1.0f / fLength);
            }
            else
            {
                point.mBodyOffset = vTarget;
            }
        }
    }
    else if (mTowState == TOW_FALL)
    {
        float fPrevFallSpeed = -5.0f;
        for (int i = 12; i >= 0; i--)
        {
            TowCableAttachPointData& point = mAttachPoints[i];
            if (!point.mIsAttached || !(fPrevFallSpeed < -4.0f))
            {
                continue;
            }

            point.mFallSpeed -= fDeltaTime * 9.8f;
            point.mInitialBodyOffset.y += point.mFallSpeed * fDeltaTime;
            if (point.mBodyOffset.y > point.mFinalHeight)
            {
                point.mBodyOffset.y = point.mInitialBodyOffset.y;
                if (point.mFinalHeight > point.mInitialBodyOffset.y)
                {
                    point.mBodyOffset.y = point.mFinalHeight;
                }
            }
            else
            {
                point.mInitialBodyOffset.y = point.mBodyOffset.y;
            }

            const PblVector3& vFinal = point.mFinalSlope;
            float fLength = sqrtf(vFinal.x * vFinal.x + vFinal.y * vFinal.y + vFinal.z * vFinal.z);
            PblVector3& vIn = point.mSlopeIn;
            if (sqrtf(vIn.x * vIn.x + vIn.y * vIn.y + vIn.z * vIn.z) + fDeltaTime > fLength)
            {
                vIn = vFinal;
            }
            else
            {
                vIn.x += fDeltaTime * vFinal.x * (1.0f / fLength);
                vIn.y += fDeltaTime * vFinal.y * (1.0f / fLength);
                vIn.z += fDeltaTime * vFinal.z * (1.0f / fLength);
            }

            PblVector3& vOut = point.mSlopeOut;
            fLength = sqrtf(vOut.x * vOut.x + vOut.y * vOut.y + vOut.z * vOut.z);
            if (fLength + fDeltaTime > fLength)
            {
                vOut = vFinal;
            }
            else
            {
                vOut.x += fDeltaTime * vFinal.x * (1.0f / fLength);
                vOut.y += fDeltaTime * vFinal.y * (1.0f / fLength);
                vOut.z += fDeltaTime * vFinal.z * (1.0f / fLength);
            }

            PblVector3 vDiff;
            vDiff.x = point.mInitialBodyOffset.x - point.mBodyOffset.x;
            vDiff.y = point.mInitialBodyOffset.y - point.mBodyOffset.y;
            vDiff.z = point.mInitialBodyOffset.z - point.mBodyOffset.z;
            fLength = sqrtf(vDiff.x * vDiff.x + vDiff.y * vDiff.y + vDiff.z * vDiff.z);
            if (fDeltaTime < fLength)
            {
                point.mBodyOffset.x += vDiff.x * fDeltaTime * (1.0f / fLength);
                point.mBodyOffset.y += vDiff.y * fDeltaTime * (1.0f / fLength);
                point.mBodyOffset.z += vDiff.z * fDeltaTime * (1.0f / fLength);
            }
            else
            {
                point.mBodyOffset = point.mInitialBodyOffset;
            }

            fPrevFallSpeed = point.mFallSpeed;
        }
    }
}

// FUNCTION: SWBF2 0x00609a80
void TowCableRenderObject::Render(GameObject* pObject, unsigned char ucViewport, float fDist,
                                  unsigned int uiFlags)
{
    PblVector3 vPrev;
    if (mAttachPoints[0].mIsAttached)
    {
        if (mAttachPoints[0].mBodyID >= 0)
        {
            if (!pObject)
            {
                return;
            }
            CollisionObject* pCollision = pObject->GetCollisionObject();
            CollisionModel* pModel = pObject->GetCollisionObject()->mCollisionModel;
            PblMatrix mBody = pModel->GetWorldMatrix(mAttachPoints[0].mBodyID, pObject->GetMatrix(),
                                                     pCollision->GetPose());
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vPrev, (const D3DXVECTOR3*)&mAttachPoints[0].mBodyOffset,
                                   (const D3DXMATRIX*)&mBody);
        }
        else
        {
            vPrev = mAttachPoints[0].mBodyOffset;
        }
    }

    for (int i = 1; i < 13; i++)
    {
        TowCableAttachPointData& point = mAttachPoints[i];
        if (!point.mIsAttached)
        {
            return;
        }

        PblVector3 vStart = vPrev;
        if (point.mBodyID >= 0)
        {
            if (!pObject)
            {
                return;
            }
            CollisionObject* pCollision = pObject->GetCollisionObject();
            CollisionModel* pModel = pObject->GetCollisionObject()->mCollisionModel;
            PblMatrix mBody = pModel->GetWorldMatrix(point.mBodyID, pObject->GetMatrix(), pCollision->GetPose());
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vPrev, (const D3DXVECTOR3*)&point.mBodyOffset,
                                   (const D3DXMATRIX*)&mBody);
        }
        else
        {
            vPrev = point.mBodyOffset;
        }

        PblHermiteVector3 segment(vStart, vPrev, mAttachPoints[i - 1].mSlopeOut, point.mSlopeIn, 1.0f);
        sRenderHermiteSegment(segment, ucViewport, fDist, uiFlags);
    }
}

// FUNCTION: SWBF2 0x00424b60
float PblHermiteVector3::GetLengthApproximation(int iSteps) const
{
    float fLength = 0.0f;
    for (int i = 0; i < iSteps; i++)
    {
        PblVector3 v0 = getValue((float)i / (float)iSteps);
        PblVector3 v1 = getValue((float)(i + 1) / (float)iSteps);
        float dx = v1.x - v0.x;
        float dy = v1.y - v0.y;
        float dz = v1.z - v0.z;
        fLength += sqrtf(dx * dx + dy * dy + dz * dz);
    }
    return fLength;
}
