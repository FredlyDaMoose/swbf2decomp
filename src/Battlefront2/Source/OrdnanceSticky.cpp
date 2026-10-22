#include "OrdnanceSticky.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "CollisionManager.h"
#include "CollisionModel.h"
#include "Damageable.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "StickInfo.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedModel.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

extern bool netIsLateTurn;
extern bool netFrameLock;
extern PblVector3 gVectorZero;

namespace AIUtil
{

    void GrenadeAlert(const PblVector3& vPos, const GameObject* pObject, float* pBeginTime,
                      float* pLastEventTime);
}

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace
{

    Damageable* AsDamageable(GameObject* pObject)
    {
        return static_cast<Damageable*>(pObject);
    }

    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }
}

// GLOBAL: SWBF2 0x01ef2668
// SYNTHETIC: SWBF2 0x00405b80
// `dynamic initializer for 'OrdnanceSticky::rttiHashOrdnanceSticky''
PblHash OrdnanceSticky::rttiHashOrdnanceSticky("OrdnanceSticky");
char OrdnanceSticky::rttiNameOrdnanceSticky[] = "OrdnanceSticky";

// GLOBAL: SWBF2 0x01ef266c
// SYNTHETIC: SWBF2 0x00405b90
// `dynamic initializer for 'OrdnanceStickyClass::rttiHashOrdnanceStickyClass''
PblHash OrdnanceStickyClass::rttiHashOrdnanceStickyClass("OrdnanceStickyClass");
char OrdnanceStickyClass::rttiNameOrdnanceStickyClass[] = "OrdnanceStickyClass";

// FUNCTION: SWBF2 0x00606d80
OrdnanceStickyClass::OrdnanceStickyClass(unsigned int uHash)
    : OrdnanceShellClass(uHash)
{
    mStartTimerOnContact = true;
    mAlignVertical = true;
    mStickTypes = -1;
}

// FUNCTION: SWBF2 0x00606e10
OrdnanceStickyClass::OrdnanceStickyClass(OrdnanceStickyClass* pBase, unsigned int uHash)
    : OrdnanceShellClass(pBase, uHash)
{
    mSoundCollisionWater = pBase->mSoundCollisionWater;
    mSoundCollisionLeaf = pBase->mSoundCollisionLeaf;
    mSoundCollisionOther = pBase->mSoundCollisionOther;
    mStartTimerOnContact = pBase->mStartTimerOnContact;
    mAlignVertical = pBase->mAlignVertical;
    mStickTypes = pBase->mStickTypes;
}

// FUNCTION: SWBF2 0x005f8c30
bool OrdnanceStickyClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceStickyClass || OrdnanceShellClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00605fb0
unsigned int OrdnanceStickyClass::GetDerivedRtti()
{
    return rttiHashOrdnanceStickyClass;
}

// FUNCTION: SWBF2 0x00605fc0
const char* OrdnanceStickyClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceStickyClass;
}

// FUNCTION: SWBF2 0x00606f00
void OrdnanceStickyClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x2297E47D:
        mSoundCollisionWater.SetID(pszValue, false, false);
        break;
    case 0xB8C524F1:
        mSoundCollisionLeaf.SetID(pszValue, false, false);
        break;
    case 0xEDC6251A:
        mSoundCollisionOther.SetID(pszValue, false, false);
        break;
    case 0x0D0CDE2B:
        mStartTimerOnContact = atoi(pszValue) != 0;
        break;
    case 0xDB873344:
        mAlignVertical = atoi(pszValue) != 0;
        break;
    case 0xD3913CC2:
        mStickTypes = atoi(pszValue) ? -1 : 0;
        break;
    case 0x01E9CAEA:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x1;
        }
        else
        {
            mStickTypes &= ~0x1;
        }
        break;
    case 0xD1B49B73:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x2;
        }
        else
        {
            mStickTypes &= ~0x2;
        }
        break;
    case 0x3B3F45B7:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x4;
        }
        else
        {
            mStickTypes &= ~0x4;
        }
        break;
    case 0x9B4AF9ED:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x8;
        }
        else
        {
            mStickTypes &= ~0x8;
        }
        break;
    case 0x7449C0B3:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x10;
        }
        else
        {
            mStickTypes &= ~0x10;
        }
        break;
    case 0xAD9C5FA5:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x20;
        }
        else
        {
            mStickTypes &= ~0x20;
        }
        break;
    case 0xA8A2AFF4:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x40;
        }
        else
        {
            mStickTypes &= ~0x40;
        }
        break;
    case 0x95706D30:
        if (atoi(pszValue))
        {
            mStickTypes |= 0x80000000;
        }
        else
        {
            mStickTypes &= ~0x80000000;
        }
        break;
    default:
        OrdnanceShellClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00607180
OrdnanceClass* OrdnanceStickyClass::Derive(unsigned int uHash)
{
    return new OrdnanceStickyClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x00606de0
// OrdnanceStickyClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006071f0
Ordnance* OrdnanceStickyClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceSticky* pSticky = new OrdnanceSticky(this, desc);
    pSticky->ActivateThread(-40, PER_LOCAL_TURN);
    pSticky->AddOrdnanceToCollisionManager();
    return pSticky;
}

// FUNCTION: SWBF2 0x00605fd0
OrdnanceSticky::OrdnanceSticky(OrdnanceStickyClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceShell(pClass, desc)
{
    mStickInfo = 0;
    mNumBounces = 0;
    mLastEventTime = -1.0f;
    mBeginTime = FLT_MAX;
    mDecrementLifeTime = !pClass->mStartTimerOnContact;
}

// FUNCTION: SWBF2 0x00606040
OrdnanceSticky::~OrdnanceSticky()
{
    if (mStickInfo)
    {
        delete mStickInfo;
    }
}

// SYNTHETIC: SWBF2 0x00607286
// [thunk]:OrdnanceSticky::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x0060728e
// [thunk]:OrdnanceSticky::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x00605f80
// OrdnanceSticky::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f8bf0
bool OrdnanceSticky::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceSticky || OrdnanceShell::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00605f60
unsigned int OrdnanceSticky::GetDerivedRtti() const
{
    return rttiHashOrdnanceSticky;
}

// FUNCTION: SWBF2 0x00605f70
const char* OrdnanceSticky::GetDerivedRttiName() const
{
    return rttiNameOrdnanceSticky;
}

// FUNCTION: SWBF2 0x006060b0
bool OrdnanceSticky::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }
    if (GameLoop::GetMissionTime() > mBeginTime)
    {
        SoundEvent();
    }

    float fWaterHeight;
    if (RedWater::GetWaterHeight(mPosition, fWaterHeight) && fWaterHeight - mPosition.y > 0.0f)
    {
        mVelocity = PblVector3(0.0f, -2.0f, 0.0f);
        if (!mDecrementLifeTime)
        {
            ((OrdnanceStickyClass*)mClass)->mSoundCollisionWater.Play(&mPosition, 0, false, true);
            mDecrementLifeTime = true;
        }
    }

    if (!mStickInfo)
    {
        return OrdnanceShell::Update(fDeltaTime);
    }

    if (mStickInfo->mCollHandle.mObject &&
        GameObjectHandleId(mStickInfo->mCollHandle.mObject) != mStickInfo->mCollHandle.mSavedHandleId)
    {
        delete mStickInfo;
        mStickInfo = 0;
        return OrdnanceShell::Update(fDeltaTime);
    }

    mOldPosition = mPosition;
    mPosition = mStickInfo->mLocalPosition;
    CollisionObject* pObject = mStickInfo->mCollObject;
    if (pObject->mCollisionModel)
    {
        PblMatrix mBody = pObject->mCollisionModel->GetWorldMatrix(mStickInfo->mCollBodyID,
                                                                   pObject->GetMatrix(), pObject->GetPose());
        PblVector3 vWorld;
        D3DXVec3TransformCoord(&vWorld, &mPosition, &mBody);
        mPosition = vWorld;
    }
    SetRenderPos(mPosition);

    if (fDeltaTime != 0.0f)
    {
        const float fInv = 1.0f / fDeltaTime;
        mVelocity = PblVector3((mPosition.x - mOldPosition.x) * fInv, fInv * (mPosition.y - mOldPosition.y),
                               (mPosition.z - mOldPosition.z) * fInv);
    }
    return Ordnance::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x006062c0
void OrdnanceSticky::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    OrdnanceStickyClass* pClass = (OrdnanceStickyClass*)mClass;
    if (!pClass->mModel)
    {
        return;
    }

    PblMatrix mat;
    if (mStickInfo)
    {
        if (!mStickInfo->mCollObject)
        {
            return;
        }

        PblQuaternion qRot;
        PblMatrix mRot;
        D3DXMatrixRotationQuaternion(&mRot, D3DXQuaternionNormalize(&qRot, &mStickInfo->mLocalRotation));
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                mat.m[i][j] = mRot.m[i][j];
            }
        }
        mat.m[3][0] = mStickInfo->mLocalPosition.x;
        mat.m[3][1] = mStickInfo->mLocalPosition.y;
        mat.m[3][2] = mStickInfo->mLocalPosition.z;
        mat.m[3][3] = 1.0f;

        CollisionObject* pObject = mStickInfo->mCollObject;
        if (pObject->mCollisionModel)
        {
            PblMatrix mBody = pObject->mCollisionModel->GetWorldMatrix(mStickInfo->mCollBodyID,
                                                                       pObject->GetMatrix(), pObject->GetPose());
            D3DXMatrixMultiply(&mat, &mat, &mBody);
        }
    }
    else if (pClass->mAlignVertical)
    {
        PblVector3 vPos = mPosition;
        if (!netFrameLock && NetGame::IsEnabled() &&
            netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
        {
            netInterpolator.GetVec(mInterpIndex, vPos);
        }
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                mat.m[i][j] = i == j ? 1.0f : 0.0f;
            }
        }
        mat.m[3][0] = vPos.x;
        mat.m[3][1] = vPos.y;
        mat.m[3][2] = vPos.z;
        mat.m[3][3] = 1.0f;
    }
    else
    {
        OrdnanceBullet::Render(ucViewport, fDist, uiFlags);
        return;
    }
    ((OrdnanceStickyClass*)mClass)->mModel->Render(&mat, 0, RedColor::WHITE, uiFlags, 0);
}

// FUNCTION: SWBF2 0x006064d0
bool OrdnanceSticky::CheckCollision()
{
    PblVector3 vStart = GetOldPosition();
    const PblVector3& vEnd = GetPosition();
    PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const float fDist = sqrtf(vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z);
    if (fDist > 0.0f)
    {
        const float fInv = 1.0f / fDist;
        vDir.x *= fInv;
        vDir.y *= fInv;
        vDir.z *= fInv;

        CollisionObject* pHitObject = 0;
        const GameObject* pIgnore = GetOwner();
        const float fLength = fDist + 0.2f;
        vStart.x -= vDir.x * 0.1f;
        vStart.y -= vDir.y * 0.1f;
        vStart.z -= vDir.z * 0.1f;
        PblVector3 vNormal;
        const float fHit = CollisionManager::RayHit(vStart, vDir, fLength, pHitObject, &vNormal, &pIgnore, 1,
                                                    mCheckStaticCollision ? 0x13BA : 0x132A, true);
        if (fHit < 1.0f)
        {
            const float fAlong = fHit * fLength;
            PblVector3 vHit(vDir.x * fAlong + vStart.x, vDir.y * fAlong + vStart.y,
                            vDir.z * fAlong + vStart.z);
            return !Collide(pHitObject, vHit, vNormal, CollisionManager::GetLastRayHitBodyId());
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006066e0
bool OrdnanceSticky::Collide(CollisionObject* pObject, const PblVector3& vPos,
                             const PblVector3& vNormal, int iBodyId)
{
    if (mBeginTime > GameLoop::GetMissionTime())
    {
        mBeginTime = GameLoop::GetMissionTime();
    }

    OrdnanceStickyClass* pClass = (OrdnanceStickyClass*)mClass;
    if (pObject->GetCollisionType() == COLL_WATER)
    {
        mVelocity = PblVector3(0.0f, -2.0f, 0.0f);
        pClass->mSoundCollisionWater.Play(&vPos, 0, false, true);
        mDecrementLifeTime = true;
        return false;
    }
    if (pObject->GetCollisionType() == COLL_LEAFPATCH)
    {
        pClass->mSoundCollisionLeaf.Play(&vPos, 0, false, true);
        return false;
    }
    pClass->mSoundCollisionOther.Play(&vPos, 0, false, true);

    bool bStick = false;
    if (pObject->GetCollisionType() == COLL_TERRAIN)
    {
        bStick = (pClass->mStickTypes & 0x80000000) != 0;
    }
    else
    {
        GameObject* pVictim = pObject->GetGameObject();
        if (pVictim)
        {
            bStick = (pClass->mStickTypes & (1 << AsDamageable(pVictim)->mHealthType)) != 0;
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &mVelocity);
            AsDamageable(pVictim)->ApplyDamage(GetDamage(), vDir, vPos, iBodyId, 0xFFFFFFFF);
        }
    }

    if (bStick && StickInfo::sMemoryPool.mUsed < StickInfo::sMemoryPool.mCount)
    {
        CollisionModel* pModel = pObject->mCollisionModel;

        const PblVector3& vY = gVectorUnitY;
        const float fCrossX = vNormal.y * vY.z - vNormal.z * vY.y;
        const float fCrossY = vNormal.z * vY.x - vNormal.x * vY.z;
        const float fCrossZ = vNormal.x * vY.y - vNormal.y * vY.x;
        PblMatrix mStick(fCrossY * fCrossY + fCrossX * fCrossX + fCrossZ * fCrossZ < 0.0001f
                             ? DirectionMatrix(vNormal, gVectorUnitZ, vPos)
                             : DirectionMatrix(vNormal, gVectorUnitY, vPos));

        mStickInfo = new StickInfo;
        GameObject* pStuckTo = pObject->GetGameObject();
        mStickInfo->mCollHandle.mObject = pStuckTo;
        mStickInfo->mCollHandle.mSavedHandleId = pStuckTo ? GameObjectHandleId(pStuckTo) : 0;
        mStickInfo->mCollObject = pObject;
        if (pModel)
        {
            PblMatrix mBody = pModel->GetWorldMatrix(iBodyId, pObject->GetMatrix(), pObject->GetPose());
            const PblMatrix mInverse = mBody.GetInverseFast();
            D3DXMatrixMultiply(&mStick, &mStick, &mInverse);
            mStickInfo->mCollBodyID = iBodyId;
        }
        else
        {
            mStickInfo->mCollBodyID = -1;
        }
        mStickInfo->mLocalPosition = PblVector3(mStick.m[3][0], mStick.m[3][1], mStick.m[3][2]);
        mStickInfo->mLocalRotation = PblQuaternion(mStick);

        mPosition = PblVector3(vPos.x + vNormal.x * 0.01f, vPos.y + vNormal.y * 0.01f,
                               vPos.z + vNormal.z * 0.01f);
        mVelocity = gVectorZero;
        CollisionManager::RemoveOrdnance(this);
        mDecrementLifeTime = true;
        return false;
    }

    if (mNumBounces < 4)
    {
        const float fDot = mVelocity.x * vNormal.x + mVelocity.y * vNormal.y + mVelocity.z * vNormal.z;
        if (fDot < 0.0f)
        {
            const float fRebound = 0.37f;
            const float fFriction = 1.15f;
            mPosition = PblVector3(vPos.x + vNormal.x * 0.1f, vPos.y + vNormal.y * 0.1f,
                                   vPos.z + vNormal.z * 0.1f);

            mVelocity.x -= vNormal.x * fDot;
            mVelocity.y -= vNormal.y * fDot;
            mVelocity.z -= vNormal.z * fDot;
            float fSlow = -fFriction * fDot * (fRebound + 1.0f) /
                          sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                mVelocity.z * mVelocity.z);
            fSlow = fSlow < 1.0f ? fSlow : 1.0f;
            mVelocity.x -= fSlow * mVelocity.x;
            mVelocity.y -= mVelocity.y * fSlow;
            mVelocity.z -= mVelocity.z * fSlow;

            const float fBounce = fDot * fRebound;
            mVelocity.x -= vNormal.x * fBounce;
            mVelocity.y -= vNormal.y * fBounce;
            mVelocity.z -= vNormal.z * fBounce;
            if (mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z < 1.0f)
            {
                ++mNumBounces;
            }
        }
        else
        {
            mNumBounces = 4;
        }
    }

    if (mNumBounces >= 4)
    {
        CollisionManager::RemoveOrdnance(this);
        mGravity = 0.0f;
        mVelocity.z = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.x = 0.0f;
        mDecrementLifeTime = true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00606d40
void OrdnanceSticky::SoundEvent()
{
    AIUtil::GrenadeAlert(mPosition, GetOwner(), &mBeginTime, &mLastEventTime);
}

// SYNTHETIC: SWBF2 0x00606ef0
// OrdnanceStickyClass::~OrdnanceStickyClass
