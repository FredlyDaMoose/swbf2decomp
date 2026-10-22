#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Aimer.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"

// GLOBAL: SWBF2 0x01eb0460
// SYNTHETIC: SWBF2 0x00402210
// `dynamic initializer for 'Aimer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766d50
// `dynamic atexit destructor for 'Aimer::sMemoryPool''
MemoryPool Aimer::sMemoryPool("Aimer", 0x220);

extern PblVector3 gVectorZero;

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace AIUtil
{

    PblVector3 DumbDown(const GameObject* pObject, float fDeltaTime, Aimer* pAimer,
                        const PblVector3& vDir);
}

extern bool netIsLocalTurn;
extern bool netIsLateTurn;

class Weapon : public Thread
{
public:
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void Init();
    virtual float GetOrdnanceVelocity();
    virtual float GetOrdnanceGravity();

    char mWeaponState[0x104 - 0x18];
    PblHandle<const GameObject> mTarget;
};

namespace
{

    const CollisionObject* GetCollisionObject(const GameObject* pObject)
    {
        return &static_cast<const CollisionObject&>(*pObject);
    }

    const Controllable* GetControllable(const GameObject* pObject)
    {
        return pObject->GetControllable();
    }

    PblVector3& Row(PblMatrix& m, int i) { return *reinterpret_cast<PblVector3*>(m.m[i]); }
    const PblVector3& Row(const PblMatrix& m, int i) { return *reinterpret_cast<const PblVector3*>(m.m[i]); }

    PblMatrix MakeMatrix(const PblQuaternion& q, const PblVector3& v)
    {
        PblQuaternion qUnit;
        PblMatrix m;
        D3DXMatrixRotationQuaternion(&m, D3DXQuaternionNormalize(&qUnit, &q));
        m.m[3][0] = v.x;
        m.m[3][1] = v.y;
        m.m[3][2] = v.z;
        m.m[3][3] = 1.0f;
        return m;
    }

    void PoseRemove(RedPose& pose, unsigned int uHash)
    {
        if (pose._Table.Remove(uHash))
        {
            --pose._iNumEntries;
        }
    }

    void PoseStore(RedPose& pose, unsigned int uHash, const PblMatrix* pMatrix)
    {
        if (pose._Table.Store(uHash, pMatrix))
        {
            ++pose._iNumEntries;
        }
    }
}

// FUNCTION: SWBF2 0x0043eb80
template const PblMatrix* PblHashTable<const PblMatrix, 128>::Find(unsigned int) const;
// FUNCTION: SWBF2 0x0043eba0
template bool PblHashTable<const PblMatrix, 128>::Store(unsigned int, const PblMatrix*);
// FUNCTION: SWBF2 0x0043ebd0
template bool PblHashTable<const PblMatrix, 128>::Remove(unsigned int);

// GLOBAL: SWBF2 0x01f9af7c
int sCurBarrel;

// FUNCTION: SWBF2 0x0043e4c0
AimerClass::AimerClass()
{
    mOffsetRot._fS = 1.0f;
    mOffsetRot._fX = 0.0f;
    mOffsetRot._fY = 0.0f;
    mOffsetRot._fZ = 0.0f;

    memset(mBarrelInfo, 0, sizeof(mBarrelInfo));

    mNumBarrels = 0;
    mPitchMin = -0.7853982f;
    mPitchMax = 0.7853982f;
    mPitchRate = 1.0f;
    mYawMin = -0.5235988f;
    mYawMax = 0.5235988f;
    mYawRate = 2.0f;
    mShiftYaw = false;
    mRestDir.x = 0.0f;
    mRestDir.y = 0.0f;
    mRestDir.z = 1.0f;
    mOffsetPos.x = 0.0f;
    mOffsetPos.y = 0.0f;
    mOffsetPos.z = 0.0f;
    mMountHash = 0;
    mParentHash = 0;
    mIsInert = false;
    mFireOutsideLimits = false;
    mHierarchyLevel = 0;
    mNextAimerClass = 0;

    sCurBarrel = 0;
}

// FUNCTION: SWBF2 0x0043e600
AimerClass::~AimerClass()
{
    delete mNextAimerClass;
}

// FUNCTION: SWBF2 0x0043e620
bool AimerClass::SetProperty(unsigned int uProperty, const char* pszValue, const RedModel* pModel, const char*)
{
    AimerClass* pClass = this;
    while (pClass->mNextAimerClass)
    {
        pClass = pClass->mNextAimerClass;
    }

    switch (uProperty)
    {
    case 0x2d6487bd:
        sscanf(pszValue, "%f %f", &pClass->mPitchMin, &pClass->mPitchMax);
        pClass->mPitchMin *= 0.017453292f;
        pClass->mPitchMax *= 0.017453292f;
        return true;

    case 0x1e534b12:
    {
        BarrelInfo& barrel = pClass->mBarrelInfo[sCurBarrel];
        barrel.mHash = PblTEMPHash(pszValue);
        barrel.mFireHash = barrel.mHash;
        if (pModel)
        {
            unsigned int uParent = pModel->GetParentBoneAndOffset(barrel.mHash, barrel.mOffset);
            barrel.mParentHash = uParent;
            if (uParent && pClass->mMountHash)
            {
                while (uParent != pClass->mMountHash && (uParent = pModel->GetParentBone(uParent)) != 0)
                    ;
            }
        }
        break;
    }

    case 0x2e5375da:
    case 0x4274fc96:
    {
        BarrelInfo& barrel = pClass->mBarrelInfo[sCurBarrel];
        barrel.mFireHash = PblTEMPHash(pszValue);
        if (pModel)
        {
            unsigned int uParent = pModel->GetParentBoneAndOffset(barrel.mFireHash, barrel.mFireOffset);
            if (uParent && pClass->mMountHash)
            {
                while (uParent != pClass->mMountHash && (uParent = pModel->GetParentBone(uParent)) != 0)
                    ;
            }
        }
        break;
    }

    case 0x3f38083c:
    {
        float fX, fY, fZ;
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        pClass->mRestDir.x = fX;
        pClass->mRestDir.y = fY;
        pClass->mRestDir.z = fZ;
        return true;
    }

    case 0x407e801e:
        pClass->mIsInert = atoi(pszValue) != 0;
        return true;

    case 0x444f8958:
        pClass->mFireOutsideLimits = atoi(pszValue) != 0;
        return true;

    case 0x4106141f:
        sscanf(pszValue, "%f", &pClass->mPitchRate);
        return true;

    case 0x486a6946:
        sscanf(pszValue, "%f", &pClass->mYawRate);
        return true;

    case 0x665d96ea:
        pClass->mNextAimerClass = new AimerClass;
        return true;

    case 0x5a29d1cb:
        sscanf(pszValue, "%f", &pClass->mBarrelInfo[sCurBarrel].mRecoil);
        break;

    case 0xa9c3675c:
        sscanf(pszValue, "%f %f", &pClass->mYawMin, &pClass->mYawMax);
        pClass->mYawMin *= 0.017453292f;
        pClass->mYawMax *= 0.017453292f;

        pClass->mShiftYaw = pClass->mYawMin > 0.0f && pClass->mYawMin <= PblMath::fPi &&
                            pClass->mYawMax >= PblMath::fPi && pClass->mYawMax < PblMath::fTwoPi;
        return true;

    case 0xc182abc2:
        pClass->mMountHash = PblTEMPHash(pszValue);
        if (pModel)
        {
            PblMatrix mOffset;
            pClass->mParentHash = pModel->GetParentBoneAndOffsetMatrix(pClass->mMountHash, mOffset);
            if (pClass->mParentHash)
            {
                pClass->mOffsetPos = Row(mOffset, 3);
                pClass->mOffsetRot = PblQuaternion(mOffset);
            }
            for (unsigned int uBone = pClass->mMountHash; uBone; uBone = pModel->GetParentBone(uBone))
            {
                ++pClass->mHierarchyLevel;
            }
        }
        return true;

    case 0xef095ea9:
    {
        float fLength;
        sscanf(pszValue, "%f", &fLength);
        BarrelInfo& barrel = pClass->mBarrelInfo[sCurBarrel];
        barrel.mFireOffset.z += fLength;
        barrel.mRecoil = fLength * 0.25f;
        break;
    }

    case 0xe98f377c:
        ++sCurBarrel;
        return true;

    case 0x78ed58ee:
    case 0xe856b41e:
    {
        float fX, fY, fZ;
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        pClass->mOffsetPos.x = fX;
        pClass->mOffsetPos.y = fY;
        pClass->mOffsetPos.z = fZ;
        return true;
    }

    default:
        return false;
    }

    if (pClass->mNumBarrels < sCurBarrel + 1)
    {
        pClass->mNumBarrels = sCurBarrel + 1;
    }
    return true;
}

// FUNCTION: SWBF2 0x0043cfa0
Aimer::Aimer(AimerClass* pClass, const Controllable* pOwner, const PblMatrix* pParentMatrix,
             RedPose* pPose, Aimer* pNextAimer)
{
    mParentMatrix = pParentMatrix;
    mClass = pClass;
    mOwner = pOwner;
    mMountRot._fX = 0.0f;
    mMountRot._fY = 0.0f;
    mMountRot._fZ = 0.0f;
    mMountRot._fS = 1.0f;
    mPose = pPose;
    mWithinLimitsFlag = true;
    mOffsetPos = pClass->mOffsetPos;

    mYaw = 0.0f;
    mPitch = 0.0f;
    if (pClass->mYawMin > 0.0f || pClass->mYawMax < 0.0f)
    {
        mYaw = (pClass->mYawMax + pClass->mYawMin) * 0.5f;
    }
    if (pClass->mPitchMin > 0.0f || pClass->mPitchMax < 0.0f)
    {
        mPitch = (pClass->mPitchMax + pClass->mPitchMin) * 0.5f;
    }
    mOmegaPitch = 0.0f;
    mOmegaYaw = 0.0f;

    PblVector3 v;
    D3DXVec3TransformNormal(&v, &pClass->mRestDir, mParentMatrix);
    mDirection = v;
    mMountPos = mOffsetPos;
    mMountRot = mClass->mOffsetRot;
    D3DXVec3TransformCoord(&v, &mMountPos, mParentMatrix);
    mRootPos = v;
    mFirePos = Row(*mParentMatrix, 3);
    mOldRootPos.x = 0.0f;
    mOldRootPos.y = 0.0f;
    mOldRootPos.z = 0.0f;

    for (int i = 0; i < mClass->mNumBarrels; ++i)
    {
        mRecoil[i] = 0.0f;
    }
    mCurrentBarrel = 0;
    mWeapon = 0;

    if (!pNextAimer)
    {
        pNextAimer = this;
    }
    if (mClass->mNextAimerClass)
    {
        mNextAimer = new Aimer(mClass->mNextAimerClass, pOwner, pParentMatrix, pPose, pNextAimer);
    }
    else
    {
        mNextAimer = pNextAimer;
    }

    bDirect = false;
    mSkip = false;
    mSkipTime = 0.0f;
    mAngVel = gVectorZero;
}

// FUNCTION: SWBF2 0x0043ce90
const PblMatrix Aimer::GetMatrix() const
{
    PblMatrix mRot = MakeMatrix(mMountRot, mMountPos);
    PblMatrix mMount;
    D3DXMatrixMultiply(&mMount, &mRot, mParentMatrix);
    return DirectionMatrix(mDirection, Row(mMount, 1), Row(mMount, 3));
}

// FUNCTION: SWBF2 0x0043d280
bool Aimer::HasSibling() const
{
    return mClass->mNextAimerClass != 0;
}

// FUNCTION: SWBF2 0x0043d220
Aimer::~Aimer()
{
    if (HasSibling())
    {
        delete mNextAimer;
    }
}

// FUNCTION: SWBF2 0x0043d290
void Aimer::SetSoldierInfo(const PblVector3& vFirePos, const PblVector3& vDirection)
{
    mFirePos = vFirePos;
    mRootPos = vFirePos;
    mDirection = vDirection;
    bDirect = true;
}

// FUNCTION: SWBF2 0x0043d2e0
void Aimer::SetupPose(const PblMatrix& mParent, RedPose& pose, bool bRecurse) const
{
    if (mClass->mMountHash && mClass->mParentHash)
    {
        const PblMatrix* pParentBone = pose._Table.Find(mClass->mParentHash);
        if (pParentBone)
        {
            PblMatrix mOffset = MakeMatrix(mClass->mOffsetRot, mOffsetPos);
            PblMatrix mMount;
            D3DXMatrixMultiply(&mMount, &mOffset, pParentBone);
            PblVector3 vDir;
            D3DXVec3TransformNormal(&vDir, &mDirection, &mParent);
            mMountPoseMatrix = DirectionMatrix(vDir, Row(mMount, 1), Row(mMount, 3));

            const PblMatrix* pOld = pose._Table.Find(mClass->mMountHash);
            if (pOld != &mMountPoseMatrix)
            {
                if (pOld)
                {
                    PoseRemove(pose, mClass->mMountHash);
                }
                PoseStore(pose, mClass->mMountHash, &mMountPoseMatrix);
            }
        }
    }

    for (int i = 0; i < mClass->mNumBarrels; ++i)
    {
        const AimerClass::BarrelInfo& barrel = mClass->mBarrelInfo[i];
        if (barrel.mHash && barrel.mParentHash)
        {
            const PblMatrix* pParentBone = pose._Table.Find(barrel.mParentHash);
            if (pParentBone)
            {
                mBarrelPoseMatrix[i] = *pParentBone;
                PblVector3 vOffset = barrel.mOffset;
                if (mRecoil[i] > 0.0f)
                {
                    vOffset.z += -(barrel.mRecoil * mRecoil[i]);
                }
                PblVector3 vPos;
                D3DXVec3TransformCoord(&vPos, &vOffset, &mBarrelPoseMatrix[i]);
                mBarrelPoseMatrix[i].m[3][0] = vPos.x;
                mBarrelPoseMatrix[i].m[3][1] = vPos.y;
                mBarrelPoseMatrix[i].m[3][2] = vPos.z;
                mBarrelPoseMatrix[i].m[3][3] = 1.0f;

                const PblMatrix* pOld = pose._Table.Find(barrel.mHash);
                if (pOld != &mBarrelPoseMatrix[i])
                {
                    if (pOld)
                    {
                        PoseRemove(pose, barrel.mHash);
                    }
                    PoseStore(pose, barrel.mHash, &mBarrelPoseMatrix[i]);
                }
            }
        }
    }

    if (bRecurse && HasSibling())
    {
        mNextAimer->SetupPose(mParent, pose, true);
    }
}

// FUNCTION: SWBF2 0x0043d650
bool Aimer::UpdateFire(Aimer*& pNextAimer, bool bFire)
{
    if (bFire)
    {
        mRecoil[mCurrentBarrel] = 1.0f;
    }

    const int iOldBarrel = mCurrentBarrel;
    bool bNext = false;
    if (++mCurrentBarrel >= mClass->mNumBarrels)
    {
        mCurrentBarrel = 0;
        Aimer* pAimer = this;
        do
        {
            pAimer = pAimer->mNextAimer;
        } while (pAimer->mClass->mIsInert && pAimer != this);
        bNext = true;
        pNextAimer = pAimer;
    }

    if (mCurrentBarrel == iOldBarrel)
    {
        mPrevFirePos = PblVector3(0.0f, 0.0f, 0.0f);
        return bNext;
    }

    const PblMatrix mMatrix = GetMatrix();
    mPrevFirePos = mFirePos;
    const AimerClass::BarrelInfo& barrel = mClass->mBarrelInfo[mCurrentBarrel];
    const PblVector3 vOffset(barrel.mOffset.x + barrel.mFireOffset.x,
                             barrel.mOffset.y + barrel.mFireOffset.y,
                             barrel.mOffset.z + barrel.mFireOffset.z);
    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &vOffset, &mMatrix);
    mFirePos = vPos;
    mPrevFirePos = PblVector3(mFirePos.x - mPrevFirePos.x, mFirePos.y - mPrevFirePos.y,
                              mFirePos.z - mPrevFirePos.z);
    return bNext;
}

// FUNCTION: SWBF2 0x0043d7f0
bool Aimer::GetLeadPos(const PblVector3& vTarget, const PblVector3& vVelocity, PblVector3* pvLead) const
{
    *pvLead = vTarget;
    bool bResult = true;
    if (!mWeapon)
    {
        return bResult;
    }

    const float fSpeed = mWeapon->GetOrdnanceVelocity();
    if (!(fSpeed > 0.0001f))
    {
        return bResult;
    }

    const float dx = vTarget.x - mRootPos.x;
    const float dy = vTarget.y - mRootPos.y;
    const float dz = vTarget.z - mRootPos.z;
    const float fGravity = mWeapon->GetOrdnanceGravity();
    if (fGravity == 0.0f)
    {
        float b = -(vVelocity.x * dx + vVelocity.y * dy + vVelocity.z * dz);
        const float fDistSq = dy * dy + dx * dx + dz * dz;
        const float fDisc = b * b - (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y
                                     + vVelocity.z * vVelocity.z - fSpeed * fSpeed) * fDistSq;
        if (fDisc > 0.0f)
        {
            b += sqrtf(fDisc);
        }
        const float fTime = fDistSq / b;
        if (fTime > 0.0f)
        {
            pvLead->x += vVelocity.x * fTime;
            pvLead->y += vVelocity.y * fTime;
            pvLead->z += vVelocity.z * fTime;
        }
        return bResult;
    }

    float a = dy * fGravity + fSpeed * fSpeed;
    const float fGravitySq = fGravity * fGravity;
    const float fDisc = a * a - (dy * dy + dx * dx + dz * dz) * fGravitySq;
    if (fDisc > 0.0f)
    {
        a -= sqrtf(fDisc);
    }
    else
    {
        bResult = false;
    }
    const float fTimeSq = a * 2.0f / fGravitySq;
    if (!(fTimeSq > 0.0f))
    {
        return false;
    }
    pvLead->y -= fGravity * 0.5f * fTimeSq;
    const float fTime = sqrtf(fTimeSq);
    pvLead->x += vVelocity.x * fTime;
    pvLead->y += vVelocity.y * fTime;
    pvLead->z += vVelocity.z * fTime;
    return bResult;
}

// FUNCTION: SWBF2 0x0043db20
bool Aimer::Update(float fDeltaTime)
{
    if (bDirect || netIsLateTurn)
    {
        return true;
    }

    if (NetGame::IsEnabled() && NetGame::IsClient() && !netIsLocalTurn)
    {
        if (!mOwner)
        {
            return true;
        }
        const GameObject* pObject = 0;

        Controllable* pOwner = const_cast<Controllable*>(mOwner);
        if (pOwner->GetPilot())
        {
            pObject = pOwner->GetPilot()->GetGameObject();
        }
        if (!pObject)
        {
            pObject = mOwner->GetGameObject();
        }
        if (NetGame::IsSkipped(pObject, fDeltaTime))
        {
            return true;
        }
    }

    if (mOwner->mPlayerId < 0)
    {
        if (mSkip)
        {
            mSkipTime = fDeltaTime;
            return true;
        }
        fDeltaTime += mSkipTime;
        mSkipTime = 0.0f;
    }

    const PblVector3 vAimPoint = mOwner->mTargetInfo.mAimPoint;

    if (mPose && mParentMatrix == &GetCollisionObject(mOwner->GetGameObject())->GetMatrix()
        && mClass->mParentHash)
    {
        const PblMatrix* pParentBone = mPose->_Table.Find(mClass->mParentHash);
        if (pParentBone)
        {
            PblMatrix mOffset = MakeMatrix(mClass->mOffsetRot, mOffsetPos);
            PblMatrix mMount;
            D3DXMatrixMultiply(&mMount, &mOffset, pParentBone);
            mMountPos = Row(mMount, 3);
            mMountRot = PblQuaternion(mMount);
        }
    }

    PblMatrix mRot = MakeMatrix(mMountRot, mMountPos);
    PblMatrix mMount;
    D3DXMatrixMultiply(&mMount, &mRot, mParentMatrix);
    mRootPos = Row(mMount, 3);

    if (mWeapon)
    {
        PblHandle<const GameObject>& hTarget = mWeapon->mTarget;
        if (hTarget.mObject
            && hTarget.mObject->mHandleId != hTarget.mSavedHandleId)
        {
            hTarget.mObject = 0;
            hTarget.mSavedHandleId = 0;
        }
    }

    const float t = mDirection.x * (mFirePos.x - mRootPos.x) + mDirection.y * (mFirePos.y - mRootPos.y)
                  + mDirection.z * (mFirePos.z - mRootPos.z);
    PblVector3 vDelta(vAimPoint.x - (mFirePos.x - mDirection.x * t),
                      vAimPoint.y - (mFirePos.y - mDirection.y * t),
                      vAimPoint.z - (mFirePos.z - mDirection.z * t));
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vDelta);

    const UnitController* pCtrl = UnitControllerPtr(mOwner);
    if (pCtrl)
    {
        const GameObject* pTarget = pCtrl->GetAttackHelperTarget();
        if (pCtrl->mLodState >= 4
            || (pTarget && GetControllable(pTarget) && GetControllable(pTarget)->mPlayerId >= 0))
        {
            vDir = AIUtil::DumbDown(mOwner->GetGameObject(), fDeltaTime, this, vDir);
        }
    }

    const PblVector3 vAxis(mDirection.y * vDir.z - mDirection.z * vDir.y,
                           mDirection.z * vDir.x - mDirection.x * vDir.z,
                           mDirection.x * vDir.y - mDirection.y * vDir.x);
    const float fOldPitch = mPitch;
    const float fOldYaw = mYaw;

    const float fPitchMax = mClass->mPitchRate * fDeltaTime;
    float fPitch = mMount.m[0][0] * vAxis.x + mMount.m[0][1] * vAxis.y + mMount.m[0][2] * vAxis.z;
    if (-fPitchMax > fPitch)
    {
        fPitch = -fPitchMax;
    }
    else if (fPitch > fPitchMax)
    {
        fPitch = fPitchMax;
    }
    mPitch = fOldPitch - fPitch;

    const float fYawMax = mClass->mYawRate * fDeltaTime;
    float fYaw = mMount.m[1][0] * vAxis.x + mMount.m[1][1] * vAxis.y + mMount.m[1][2] * vAxis.z;
    if (-fYawMax > fYaw)
    {
        fYaw = -fYawMax;
    }
    else if (fYaw > fYawMax)
    {
        fYaw = fYawMax;
    }
    mYaw = fOldYaw + fYaw;

    if (fabs(fOldPitch - mPitch) < 0.0001f && fabs(fOldYaw - mYaw) < 0.0001f)
    {
        const float fx = mRootPos.x - mOldRootPos.x;
        const float fy = mRootPos.y - mOldRootPos.y;
        const float fz = mRootPos.z - mOldRootPos.z;
        if (fy * fy + fx * fx + fz * fz < 0.0001f)
        {
            return true;
        }
    }

    mOldRootPos = mRootPos;
    mWithinLimitsFlag = ClampDirection(mMount);

    if (mClass->mNumBarrels > 0)
    {
        for (int i = 0; i < mClass->mNumBarrels; ++i)
        {
            mRecoil[i] -= fDeltaTime * 4.0f;
        }

        const PblMatrix mAim = DirectionMatrix(mDirection, Row(mMount, 1), mRootPos);
        const AimerClass::BarrelInfo& barrel = mClass->mBarrelInfo[mCurrentBarrel];
        const PblVector3 vOffset(barrel.mOffset.x + barrel.mFireOffset.x,
                                 barrel.mOffset.y + barrel.mFireOffset.y,
                                 barrel.mOffset.z + barrel.mFireOffset.z);
        PblVector3 vPos;
        D3DXVec3TransformCoord(&vPos, &vOffset, &mAim);
        mFirePos = vPos;
    }
    else
    {
        mFirePos = mRootPos;
    }

    if (mPose)
    {
        SetupPose(GetCollisionObject(mOwner->GetGameObject())->GetMatrix().GetInverseFast(), *mPose, false);
    }
    return true;
}

// FUNCTION: SWBF2 0x0043e1b0
bool Aimer::ClampDirection(const PblMatrix& mMatrix)
{
    bool bWithin = true;
    float fPitchMin = mClass->mPitchMin;
    float fPitchMax = mClass->mPitchMax;
    float fYawMin = mClass->mYawMin;
    float fYawMax = mClass->mYawMax;

    if (Character* pCharacter = mOwner->mCharacter)
    {
        if (Controllable* pUnit = pCharacter->GetControllable())
        {
            if (pUnit->GetControllableClass()->mControlsUnit)
            {
                pUnit->GetPitchYawLimits(fPitchMin, fPitchMax, fYawMin, fYawMax);
                if (mOwner->mPlayerId >= 0)
                {
                    return true;
                }
            }
        }
    }

    if (fPitchMin == fPitchMax)
    {
        mPitch = fPitchMin;
    }
    else if (fPitchMin > mPitch)
    {
        mPitch = fPitchMin;
        bWithin = false;
    }
    else if (mPitch > fPitchMax)
    {
        mPitch = fPitchMax;
        bWithin = false;
    }

    if (mClass->mShiftYaw && mYaw < 0.0f)
    {
        mYaw += 6.2831855f;
    }
    if (fYawMin == fYawMax)
    {
        mYaw = fYawMin;
    }
    if (fYawMin > mYaw)
    {
        mYaw = fYawMin;
        bWithin = false;
    }
    else if (mYaw > fYawMax)
    {
        mYaw = fYawMax;
        bWithin = false;
    }

    float fSinYaw, fCosYaw, fSinPitch, fCosPitch;
    PblMath::SinCos(&fSinYaw, &fCosYaw, mYaw);
    PblMath::SinCos(&fSinPitch, &fCosPitch, mPitch);
    const PblVector3 vLocal(fCosPitch * fSinYaw, fSinPitch, fCosPitch * fCosYaw);
    PblVector3 vDir;
    D3DXVec3TransformNormal(&vDir, &vLocal, &mMatrix);
    mDirection = vDir;
    return bWithin;
}

// FUNCTION: SWBF2 0x0043e380
void Aimer::ActivatePhysics()
{
    ActivateThread(-20 - mClass->mHierarchyLevel, PER_LOCAL_TURN);
    if (HasSibling())
    {
        mNextAimer->ActivatePhysics();
    }
}

// FUNCTION: SWBF2 0x0043e3d0
void Aimer::DeactivatePhysics()
{
    DeactivateThread();
    if (HasSibling())
    {
        mNextAimer->DeactivatePhysics();
    }
}

// FUNCTION: SWBF2 0x0043e400
void Aimer::SetWeapon(Weapon* pWeapon)
{
    mWeapon = pWeapon;
    if (HasSibling())
    {
        mNextAimer->SetWeapon(pWeapon);
    }
}

// FUNCTION: SWBF2 0x0043e440
void Aimer::SetSkip(bool bSkip)
{
    mSkip = bSkip;
}

// FUNCTION: SWBF2 0x0043e450
void Aimer::ResetSkip()
{
    mSkip = false;
    mSkipTime = 0.0f;
}

// FUNCTION: SWBF2 0x0043e470
const PblVector3& Aimer::GetAngVel() const
{
    return mAngVel;
}

// FUNCTION: SWBF2 0x0043e480
void Aimer::SetAngVel(const PblVector3& vAngVel)
{
    mAngVel = vAngVel;
}

// FUNCTION: SWBF2 0x0043e4a0
bool Aimer::WeaponShouldFire() const
{
    return mWithinLimitsFlag || mClass->mFireOutsideLimits;
}
