#include <math.h>
#include <float.h>
#include <string.h>
#include <stddef.h>
#include <d3dx9math.h>

#include "BirdEffect.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "GameObject.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "Battlefront2Lod.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblHashTable.h"

#include "RedEngineFL/Application/RedTerrain.h"

#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "LensFlare.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float EPSILON = 0.0001f;
static const float fHalfPi = PblMath::fPi * 0.5f;

static inline PblQuaternion PblQuaternion_FromAxisAngle(const PblVector3& vAxis, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle * 0.5f);
    PblQuaternion q;
    q._fX = vAxis.x * fSin;
    q._fY = vAxis.y * fSin;
    q._fZ = vAxis.z * fSin;
    q._fS = fCos;
    return q;
}

static inline PblQuaternion RotateByOmega(const PblQuaternion& q, const PblVector3& vOmega)
{
    float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        return q * PblQuaternion_FromAxisAngle(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }
    return q;
}

static inline PblMatrix MatrixFromQuat(const PblQuaternion& q, const PblVector3& vPos)
{
    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
    mNew.m[3][0] = vPos.x;
    mNew.m[3][1] = vPos.y;
    mNew.m[3][2] = vPos.z;
    mNew.m[3][3] = 1.0f;
    return mNew;
}

static inline void SetWingMatrix(PblMatrix& rMatrix, const PblQuaternion& q)
{
    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            rMatrix.m[i][j] = mNew.m[i][j];
        }
    }
    rMatrix.m[3][0] = 0.0f;
    rMatrix.m[3][1] = 0.0f;
    rMatrix.m[3][2] = 0.0f;
    rMatrix.m[3][3] = 1.0f;
}

static inline void PoseWing(PblMatrix& rMatrix, float fRoll)
{
    rMatrix = gMatrixIdentity;
    PblQuaternion q = PblQuaternion(rMatrix);
    q = q * PblQuaternion_FromAxisAngle(PblVector3(0.0f, 0.0f, 1.0f), fRoll);
    q = q * PblQuaternion_FromAxisAngle(PblVector3(0.0f, 1.0f, 0.0f), fHalfPi * 0.5f);
    SetWingMatrix(rMatrix, q);
}

// FUNCTION: SWBF2 0x00427110
const PblBox& RedSceneObject::GetBBox() const
{
    return *(const PblBox*)0;
}

// FUNCTION: SWBF2 0x00444000
void RedSceneObject::GetAABound(geo::Box&) const
{
}

// FUNCTION: SWBF2 0x00444010
void RedSceneObject::GetSphereBound(geo::Sphere& rSphere) const
{
    rSphere = geo::Sphere(geo::Vector3<float>(_Sphere.x, _Sphere.y, _Sphere.z), _Sphere.w);
}

// FUNCTION: SWBF2 0x00444060
const PblMatrix& RedSceneObject::GetBBoxMatrix() const
{
    return gMatrixIdentity;
}

// FUNCTION: SWBF2 0x00444070
bool RedSceneObject::IsReflected() const
{
    return true;
}

template <class T>
void pvs::BaseDisplayable<T>::OnTraverse(pvs::ITraverser&)
{
}

template <class T>
void pvs::BaseDisplayable<T>::SetActive(int iFrame)
{
    aframe_ = iFrame;
}

template <class T>
bool pvs::BaseDisplayable<T>::IsActive(int iFrame) const
{
    return aframe_ >= iFrame;
}

template <class T>
void pvs::BaseDisplayable<T>::SetProcessed(int iFrame)
{
    pframe_ = iFrame;
}

template <class T>
bool pvs::BaseDisplayable<T>::IsProcessed(int iFrame) const
{
    return pframe_ == iFrame;
}

template <class T>
bool pvs::BaseDisplayable<T>::PortalActive() const
{
    return (flags_ >> 1) & 1;
}

template <class T>
void pvs::BaseDisplayable<T>::SetPortalActive(bool bActive)
{
    if (bActive)
    {
        flags_ |= 2;
    }
    else
    {
        flags_ &= ~2u;
    }
}

template <class T>
bool pvs::BaseDisplayable<T>::PortalClip() const
{
    return (flags_ >> 2) & 1;
}

template <class T>
void pvs::BaseDisplayable<T>::SetPortalClip(bool bClip)
{
    if (bClip)
    {
        flags_ |= 4;
    }
    else
    {
        flags_ &= ~4u;
    }
}

template <class T>
bool pvs::BaseDisplayable<T>::Dynamic() const
{
    return (flags_ >> 3) & 1;
}

template <class T>
void pvs::BaseDisplayable<T>::SetDynamic(bool bDynamic)
{
    if (bDynamic)
    {
        flags_ |= 8;
    }
    else
    {
        flags_ &= ~8u;
    }
}

template <class T>
pvs::Sector* pvs::BaseDisplayable<T>::GetSector()
{
    return sector_;
}

// FUNCTION: SWBF2 0x00446c30
template void pvs::BaseDisplayable<pvs::IDisplayable>::OnTraverse(pvs::ITraverser&);

// FUNCTION: SWBF2 0x00446c40
template void pvs::BaseDisplayable<pvs::IDisplayable>::SetActive(int);

// FUNCTION: SWBF2 0x00446c50
template bool pvs::BaseDisplayable<pvs::IDisplayable>::IsActive(int) const;

// FUNCTION: SWBF2 0x00446c70
template void pvs::BaseDisplayable<pvs::IDisplayable>::SetProcessed(int);

// FUNCTION: SWBF2 0x00446c80
template bool pvs::BaseDisplayable<pvs::IDisplayable>::IsProcessed(int) const;

// FUNCTION: SWBF2 0x00446ca0
template bool pvs::BaseDisplayable<pvs::IDisplayable>::PortalActive() const;

// FUNCTION: SWBF2 0x00446cb0
template void pvs::BaseDisplayable<pvs::IDisplayable>::SetPortalActive(bool);

// FUNCTION: SWBF2 0x00446cd0
template bool pvs::BaseDisplayable<pvs::IDisplayable>::PortalClip() const;

// FUNCTION: SWBF2 0x00446ce0
template void pvs::BaseDisplayable<pvs::IDisplayable>::SetPortalClip(bool);

// FUNCTION: SWBF2 0x00446d00
template bool pvs::BaseDisplayable<pvs::IDisplayable>::Dynamic() const;

// FUNCTION: SWBF2 0x00446d10
template void pvs::BaseDisplayable<pvs::IDisplayable>::SetDynamic(bool);

// FUNCTION: SWBF2 0x00446d30
template pvs::Sector* pvs::BaseDisplayable<pvs::IDisplayable>::GetSector();

// GLOBAL: SWBF2 0x01e30314
BirdParticleManager* BirdParticleManager::sInstance;

// GLOBAL: SWBF2 0x01e302ec
int BirdParticleManager::sNumBirdTypes;
// GLOBAL: SWBF2 0x01e302f0
BirdParticleType BirdParticleManager::sBirdTypes[3];

// FUNCTION: SWBF2 0x0044af30
void BirdParticleManager::SetNumBirdTypes(int iNum)
{
    if (iNum < 0)
    {
        iNum = 0;
    }
    else if (iNum > 3)
    {
        iNum = 3;
    }
    BirdParticleManager::sNumBirdTypes = iNum;
}

// FUNCTION: SWBF2 0x0044af50
void BirdParticleManager::SetBirdType(int iIndex, float fSize, const char* pTexture)
{
    if (iIndex >= 0 && iIndex < BirdParticleManager::sNumBirdTypes)
    {
        BirdParticleManager::sBirdTypes[iIndex].mSize = fSize;
        BirdParticleManager::sBirdTypes[iIndex].mTexture = PblHash(pTexture);
    }
}

// FUNCTION: SWBF2 0x0044b470
bool BirdParticleManager::CreateClient()
{
    return true;
}

// FUNCTION: SWBF2 0x0044a580
bool BirdParticle::UpdateHoppingCircleBird()
{
    BirdParticle* pBird = (BirdParticle*)this;
    float fRise = PblMath::Sin((pBird->mFlapTime + fHalfPi) * 10.0f);
    if (fRise > 0.0f)
    {
        PblVector3 vPosition(pBird->mMatrix.m[3][0], pBird->mMatrix.m[3][1],
                             pBird->mMatrix.m[3][2]);
        vPosition.y += fRise * 0.3f * pBird->mType.mSize;
        pBird->mMatrix.m[3][0] = vPosition.x;
        pBird->mMatrix.m[3][1] = vPosition.y;
        pBird->mMatrix.m[3][2] = vPosition.z;
        pBird->mMatrix.m[3][3] = 1.0f;
        pBird->SetRenderPos(vPosition);
    }
    return true;
}

// FUNCTION: SWBF2 0x0044ad40
bool BirdParticle::UpdateScatterBird(float fDeltaTime)
{
    UpdateFlying(fDeltaTime);
    if (mTotalLifetime > 5.0f)
    {
        mIsValid = false;
        mBirdSound.Stop(true);
        Deactivate();
    }
    return true;
}

// FUNCTION: SWBF2 0x0044a0f0
bool BirdParticle::UpdateFlockBird(float fDeltaTime)
{
    UpdateFlying(fDeltaTime);
    if (mTimeOffCamera > 5.0f)
    {
        mIsValid = false;
        mBirdSound.Stop(true);
        Deactivate();
    }
    return true;
}

// FUNCTION: SWBF2 0x0044ad80
bool BirdParticle::Update(float fDeltaTime)
{
    if (!mIsValid)
    {
        return true;
    }
    mTimeOffCamera += fDeltaTime;
    switch (mState)
    {
    case BIRD_SCATTER:
        return UpdateScatterBird(fDeltaTime);
    case BIRD_FLOCK:
        return UpdateFlockBird(fDeltaTime);
    case BIRD_HOP_CIRCLE:
    case BIRD_WALK_CIRCLE:
    case BIRD_TAKEOFF:
        return UpdateStandingBird(fDeltaTime);
    default:
        mIsValid = false;
        mBirdSound.Stop(true);
        Deactivate();
        return true;
    }
}

// FUNCTION: SWBF2 0x0044a130
bool BirdParticle::UpdateWalkingCircleBird(float fDeltaTime)
{
    mCircleTime += fDeltaTime;
    if (mCircleTime > mTotalCircleTime)
    {
        mCircleTime -= mTotalCircleTime;
    }
    float fAngle = mCircleTime / mTotalCircleTime * PblMath::fTwoPi;

    PblVector3 vPos = mCircleCenter;
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    vPos.x += mCircleRadius * fSin;
    vPos.z += mCircleRadius * fCos;
    vPos.y = RedTerrain::GetHoleHeight(vPos) + mType.mSize * 0.125f;

    PblQuaternion q = PblQuaternion(gMatrixIdentity);
    q = RotateByOmega(q, PblVector3(0.0f, fAngle + fHalfPi, 0.0f));
    q = RotateByOmega(q, PblVector3(fHalfPi * -0.5f, 0.0f, 0.0f));
    q = RotateByOmega(q, PblVector3(0.0f, 0.0f, PblMath::Sin(mFlapTime * 10.0f) * 0.5f));
    mMatrix = MatrixFromQuat(q, vPos);
    SetRenderPos(vPos);
    return true;
}

// FUNCTION: SWBF2 0x0044a620
bool BirdParticle::UpdateStandingBird(float fDeltaTime)
{
    if (mState == BIRD_HOP_CIRCLE || mState == BIRD_WALK_CIRCLE)
    {
        UpdateWalkingCircleBird(fDeltaTime);
        if (mState == BIRD_HOP_CIRCLE)
        {
            UpdateHoppingCircleBird();
        }

        PblVector3 vPos = mMatrix.GetTranslation();
        GameObject* pObject;
        if (CollisionManager::GetObjectsInRange(vPos, 20.0f, &pObject, 1, 0, 10) > 0)
        {
            const PblVector3& vObjectPos = pObject->GetMatrix().GetTranslation();
            PblVector3 vAway(vPos.x - vObjectPos.x, vPos.y - vObjectPos.y, vPos.z - vObjectPos.z);
            vAway.y = 0.0f;
            PblVector3 vDir;
            D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vAway);
            mState = BIRD_TAKEOFF;
            mTotalLifetime = 0.0f;
            mVelocity = PblVector3(vDir.x * 9.0f, vDir.y * 9.0f, vDir.z * 9.0f);
        }
    }
    else if (mState == BIRD_TAKEOFF)
    {
        PblVector3 vDir;
        D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);
        mMatrix.m[2][0] = vDir.x;
        mMatrix.m[2][1] = vDir.y;
        mMatrix.m[2][2] = vDir.z;
        mMatrix.m[2][3] = 0.0f;
        mMatrix.m[1][0] = 0.0f;
        mMatrix.m[1][1] = 1.0f;
        mMatrix.m[1][2] = 0.0f;
        mMatrix.m[1][3] = 0.0f;
        const PblVector3& vY = mMatrix.GetYAxis();
        const PblVector3& vZ = mMatrix.GetZAxis();
        PblVector3 vCross(vY.y * vZ.z - vY.z * vZ.y, vY.z * vZ.x - vY.x * vZ.z, vY.x * vZ.y - vY.y * vZ.x);
        PblVector3 vX;
        D3DXVec3Normalize((D3DXVECTOR3*)&vX, (const D3DXVECTOR3*)&vCross);
        mMatrix.m[0][0] = vX.x;
        mMatrix.m[0][1] = vX.y;
        mMatrix.m[0][2] = vX.z;
        mMatrix.m[0][3] = 0.0f;

        PblQuaternion q = PblQuaternion(mMatrix);
        q = RotateByOmega(q, PblVector3(fHalfPi * -0.5f, 0.0f, 0.0f));
        q = RotateByOmega(q, PblVector3(0.0f, 0.0f, PblMath::Sin(mFlapTime * 20.0f) * 0.5f));
        q = RotateByOmega(q, PblVector3(0.0f, PblMath::Sin(mFlapTime * 5.0f) * 0.3f, 0.0f));

        const PblVector3& vOld = mMatrix.GetTranslation();
        PblVector3 vPos(vOld.x + mVelocity.x * fDeltaTime, vOld.y + mVelocity.y * fDeltaTime,
                        vOld.z + mVelocity.z * fDeltaTime);
        vPos.y = RedTerrain::GetHoleHeight(vPos) + mType.mSize * 0.125f;
        mMatrix = MatrixFromQuat(q, vPos);
        SetRenderPos(vPos);

        if (mTotalLifetime > 1.5f)
        {
            PblVector3 vAway;
            D3DXVec3Normalize((D3DXVECTOR3*)&vAway, (const D3DXVECTOR3*)&mVelocity);
            RespawnScatterBird(vPos, mType, vAway);
            return true;
        }
    }

    mTotalLifetime += fDeltaTime;
    mFlapTime += fDeltaTime;
    if (mFlapTime > PblMath::fTwoPi)
    {
        mFlapTime -= PblMath::fTwoPi;
    }
    mBirdSound.Update(&BirdParticleManager::sInstance->mBirdSound, &mMatrix.GetTranslation(), &mVelocity);
    if (mTimeOffCamera > 10.0f)
    {
        mIsValid = false;
        mBirdSound.Stop(true);
        Deactivate();
    }
    return true;
}

// FUNCTION: SWBF2 0x0044b080
void __fastcall BirdParticleManager::SetPotentialScatterPoint(const PblVector3* pPosition, unsigned char ucNumBirds)
{
    BirdParticleManager* pManager = BirdParticleManager::sInstance;
    if (!pManager)
    {
        return;
    }

    CameraManager* pCameraManager = CameraManager::sInstance;
    const int iNumCameras = pCameraManager->m_iNumCam;
    PblMatrix cameraMatrix[4];
    for (int i = 0; i < iNumCameras; ++i)
    {
        cameraMatrix[i] = pCameraManager->mRedCamera[i]->_Matrix;
    }

    int iFarthestPoint = 0;
    int iClosestCameraToPoint = -1;
    int iClosestCameraToNew = -1;
    float fFarthestDistance = 0.0f;
    float fNewDistance = FLT_MAX;
    for (int i = 0; i < 5; ++i)
    {
        PblVector3& rPoint = pManager->mPotentialScatterPoints[i];
        if (pManager->mNumScatterBirds[i])
        {
            const float dx = pPosition->x - rPoint.x;
            const float dy = pPosition->y - rPoint.y;
            const float dz = pPosition->z - rPoint.z;
            if (fabsf(dx * dx + dy * dy + dz * dz) < 0.001f)
            {
                return;
            }
        }

        float fPointDistance = FLT_MAX;
        for (int j = 0; j < iNumCameras; ++j)
        {
            const PblMatrix& rCamera = cameraMatrix[j];
            const float px = rPoint.x - rCamera.m[3][0];
            const float py = rPoint.y - rCamera.m[3][1];
            const float pz = rPoint.z - rCamera.m[3][2];
            const float fDistance = px * px + py * py + pz * pz;
            if (fDistance < fPointDistance)
            {
                fPointDistance = fDistance;
                iClosestCameraToPoint = j;
            }
            const float nx = pPosition->x - rCamera.m[3][0];
            const float ny = pPosition->y - rCamera.m[3][1];
            const float nz = pPosition->z - rCamera.m[3][2];
            const float fDistanceNew = nx * nx + ny * ny + nz * nz;
            if (fDistanceNew < fNewDistance)
            {
                fNewDistance = fDistanceNew;
                iClosestCameraToNew = j;
            }
        }
        if (fPointDistance > fFarthestDistance)
        {
            fFarthestDistance = fPointDistance;
            iFarthestPoint = i;
        }
    }
    const PblMatrix& newCamera = cameraMatrix[iClosestCameraToNew];
    const PblMatrix& pointCamera = cameraMatrix[iClosestCameraToPoint];
    PblVector3 vNew(pPosition->x - newCamera.m[3][0],
                    pPosition->y - newCamera.m[3][1],
                    pPosition->z - newCamera.m[3][2]);
    PblVector3& rOld = pManager->mPotentialScatterPoints[iFarthestPoint];
    PblVector3 vOld(rOld.x - pointCamera.m[3][0],
                    rOld.y - pointCamera.m[3][1],
                    rOld.z - pointCamera.m[3][2]);
    const float fOldDistance = vOld.x*vOld.x + vOld.y*vOld.y + vOld.z*vOld.z;
    const float fNewDistanceSq = vNew.x*vNew.x + vNew.y*vNew.y + vNew.z*vNew.z;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vNew), reinterpret_cast<const D3DXVECTOR3*>(&vNew));
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vOld), reinterpret_cast<const D3DXVECTOR3*>(&vOld));
    const float fOldFacing = pointCamera.m[2][0]*vOld.x + pointCamera.m[2][1]*vOld.y + pointCamera.m[2][2]*vOld.z;
    const float fNewFacing = newCamera.m[2][0]*vNew.x + newCamera.m[2][1]*vNew.y + newCamera.m[2][2]*vNew.z;
    if (!pManager->mNumScatterBirds[iFarthestPoint] || fOldFacing > -0.2f ||
        (fNewFacing <= -0.2f && fOldDistance > fNewDistanceSq))
    {
        rOld = *pPosition;
        pManager->mNumScatterBirds[iFarthestPoint] = ucNumBirds;
    }
}

// FUNCTION: SWBF2 0x0044b5b0
void BirdParticleManager::FlockRandomBirds(const PblVector3& vPosition, const PblVector3& vVelocity)
{
    if (!sInstance)
    {
        return;
    }
    const float fChoice = PblRandom::_Global.GetFloat();
    if (!sNumBirdTypes)
    {
        return;
    }
    const float fStep = 1.0f / (float)sNumBirdTypes;
    float fAccumulated = 0.0f;
    int iType = 0;
    do
    {
        fAccumulated += fStep;
        if (fAccumulated >= fChoice)
        {
            break;
        }
        ++iType;
    } while (iType < sNumBirdTypes);
    if (iType >= sNumBirdTypes)
    {
        return;
    }

    const BirdParticleType& type = sBirdTypes[iType];
    PblVector3 vForward;
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vForward),
                      reinterpret_cast<const D3DXVECTOR3*>(&vVelocity));
    PblVector3 vSide(vForward.z * 1.5f, 0.0f, -vForward.x * 1.5f);
    D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&vSide),
                      reinterpret_cast<const D3DXVECTOR3*>(&vSide));
    vSide.x *= 0.37f; vSide.y *= 0.37f; vSide.z *= 0.37f;
    vForward.x *= 1.5f; vForward.y *= 1.5f; vForward.z *= 1.5f;

    BirdParticle* pBird = GetFreeBird();
    if (pBird)
    {
        pBird->RespawnFlockBird(vPosition, vVelocity, type);
    }
    for (int i = 0; i < 4; ++i)
    {
        pBird = GetFreeBird();
        if (!pBird)
        {
            continue;
        }
        const float fSpacing = (i < 2 ? 2.0f : 4.0f) * type.mSize;
        const float fSide = (i & 1) ? -1.0f : 1.0f;
        const PblVector3 vBirdPosition(vPosition.x + (fSide*vSide.x - vForward.x)*fSpacing,
                                       vPosition.y + (fSide*vSide.y - vForward.y)*fSpacing,
                                       vPosition.z + (fSide*vSide.z - vForward.z)*fSpacing);
        pBird->RespawnFlockBird(vBirdPosition, vVelocity, type);
    }
    sInstance->mFlockTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0044b980
void BirdParticleManager::SpawnGroundBirds()
{
    int iCam = PblRandom::GetGlobalInt() % CameraManager::Instance()->GetNumCameras();
    PblMatrix mCamera = CameraManager::Instance()->GetCamera(iCam)->GetMatrix();
    const D3DXVECTOR3& vDir = *(const D3DXVECTOR3*)&mCamera.m[2][0];
    const PblVector3& vCamPos = *(const PblVector3*)&mCamera.m[3][0];
    const D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);

    D3DXVECTOR3 vTemp;
    PblVector3 vSide;
    D3DXVec3Normalize((D3DXVECTOR3*)&vSide, D3DXVec3Cross(&vTemp, &vUp, &vDir));
    PblVector3 vForward;
    D3DXVec3Normalize((D3DXVECTOR3*)&vForward, D3DXVec3Cross(&vTemp, &vUp, (const D3DXVECTOR3*)&vSide));

    PblVector3 vCenter(vCamPos.x + vForward.x * 45.0f, vCamPos.y + vForward.y * 45.0f,
                       vCamPos.z + vForward.z * 45.0f);

    int iNumGround = 0;
    for (int i = 0; i < 15; ++i)
    {
        const BirdParticle& rBird = sInstance->mBirdParticles[i];
        if (rBird.mIsValid && (rBird.mState == BIRD_HOP_CIRCLE || rBird.mState == BIRD_WALK_CIRCLE))
        {
            ++iNumGround;
        }
    }

    for (int i = 3 - iNumGround; i > 0; --i)
    {
        float fForward = PblRandom::GetGlobalFloat();
        float fSide = PblRandom::GetGlobalFloat();
        PblVector3 vPos(vCenter.x + (vSide.x * fSide + vForward.x * fForward) * 8.0f,
                        vCenter.y + (vSide.y * fSide + vForward.y * fForward) * 8.0f,
                        vCenter.z + (vSide.z * fSide + vForward.z * fForward) * 8.0f);
        vPos.y = RedTerrain::GetHoleHeight(vPos);

        CollisionObject* pHit = 0;
        const PblVector3 vDown(0.0f, -1.0f, 0.0f);
        CollisionManager::RayHit(vPos, vDown, 1024.0f, pHit, 0, 0, 0, 0x190, false);
        if (pHit && pHit->GetCollisionType() == COLL_TERRAIN)
        {
            float fChoice = PblRandom::GetGlobalFloat();
            BirdParticle* pBird = GetFreeBird();
            if (sNumBirdTypes)
            {
                const float fStep = 1.0f / (float)sNumBirdTypes;
                float fAccumulated = 0.0f;
                for (int iType = 0; iType < sNumBirdTypes; ++iType)
                {
                    fAccumulated += fStep;
                    if (fAccumulated >= fChoice)
                    {
                        vPos.y += sBirdTypes[iType].mSize * 0.125f;
                        pBird->RespawnGroundBird(vPos, sBirdTypes[iType]);
                        break;
                    }
                }
            }
        }
    }
    sInstance->mStandTimer = 0.0f;
}

// GLOBAL: SWBF2 0x007e5ef8
float BirdParticleManager::sFlockMinHeight = -1025.0f;

extern NoLodData noloddata;

// FUNCTION: SWBF2 0x00448980
BirdParticle::BirdParticle()
    : RedSceneObject(&noloddata)
{
    mIsValid = false;
}

// SYNTHETIC: SWBF2 0x004489b0
// BirdParticle::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004489e0
BirdParticle::~BirdParticle()
{
    mBirdSound.Stop(true);
}

// FUNCTION: SWBF2 0x00448a50
void BirdParticle::StartBirdSound()
{
    mBirdSound.Play(&BirdParticleManager::sInstance->mBirdSound, 0, 0, true);
}

// FUNCTION: SWBF2 0x00448a80
void BirdParticle::RespawnGroundBird(const PblVector3& vPos, const BirdParticleType& type)
{
    mState = BIRD_WALK_CIRCLE;
    mTimeOffCamera = 0.0f;
    mTotalLifetime = 0.0f;
    mFlapTime = PblRandom::GetGlobalFloat() * PblMath::fTwoPi;
    mVelocity = PblVector3(0.0f, 0.0f, 0.0f);

    PblQuaternion q = PblQuaternion(gMatrixIdentity);
    q = RotateByOmega(q, PblVector3(0.0f, PblRandom::GetGlobalFloat() * PblMath::fTwoPi, 0.0f));
    q = RotateByOmega(q, PblVector3(fHalfPi * -0.5f, 0.0f, 0.0f));

    mTotalCircleTime = (PblRandom::GetGlobalFloat() + 0.7f) * 10.0f;
    mCircleRadius = PblRandom::GetGlobalFloat() * 2.0f;
    mCircleCenter = vPos;
    mCircleTime = PblRandom::GetGlobalFloat() * mTotalCircleTime;

    PblVector3 vNewPos = mCircleCenter;
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, mCircleTime / mTotalCircleTime * PblMath::fTwoPi);
    vNewPos.x += mCircleRadius * fSin;
    vNewPos.z += mCircleRadius * fCos;
    vNewPos.y = RedTerrain::GetHoleHeight(vNewPos) + type.mSize * 0.125f;

    mMatrix = MatrixFromQuat(q, vNewPos);
    mInitMatrix = mMatrix;
    mIsValid = true;
    mType = type;

    SetRenderRadius(mType.mSize);
    SetRenderPos(vNewPos);
    Activate();
    StartBirdSound();
    mBirdSound.Update(&BirdParticleManager::sInstance->mBirdSound, &mMatrix.GetTranslation(), &mVelocity);

    PoseWing(mWing1Matrix, fHalfPi * 0.9f);
    PoseWing(mWing2Matrix, fHalfPi * 0.9f + PblMath::fPi);
}

// FUNCTION: SWBF2 0x00449380
void BirdParticle::RespawnFlockBird(const PblVector3& vPos, const PblVector3& vVelocity,
                                    const BirdParticleType& type)
{
    PblVector3 vUp(0.0f, 1.0f, 0.0f);

    mState = BIRD_FLOCK;
    mTimeOffCamera = 0.0f;
    mTotalLifetime = 0.0f;
    mFlapTime = PblRandom::GetGlobalFloat() * PblMath::fTwoPi;
    mVelocity = vVelocity;

    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);
    PblVector3 vCross(vUp.y * vDir.z - vUp.z * vDir.y, vUp.z * vDir.x - vUp.x * vDir.z,
                      vUp.x * vDir.y - vUp.y * vDir.x);
    PblVector3 vRight;
    D3DXVec3Normalize((D3DXVECTOR3*)&vRight, (const D3DXVECTOR3*)&vCross);

    mMatrix.m[0][0] = vRight.x;
    mMatrix.m[0][1] = vRight.y;
    mMatrix.m[0][2] = vRight.z;
    mMatrix.m[0][3] = 0.0f;
    mMatrix.m[1][0] = vUp.x;
    mMatrix.m[1][1] = vUp.y;
    mMatrix.m[1][2] = vUp.z;
    mMatrix.m[1][3] = 0.0f;
    mMatrix.m[2][0] = vDir.x;
    mMatrix.m[2][1] = vDir.y;
    mMatrix.m[2][2] = vDir.z;
    mMatrix.m[2][3] = 0.0f;
    mMatrix.m[3][0] = vPos.x;
    mMatrix.m[3][1] = vPos.y;
    mMatrix.m[3][2] = vPos.z;
    mMatrix.m[3][3] = 1.0f;
    mIsValid = true;
    mInitMatrix = mMatrix;
    mType = type;

    SetRenderRadius(mType.mSize);
    SetRenderPos(vPos);
    Activate();
    StartBirdSound();
    mBirdSound.Update(&BirdParticleManager::sInstance->mBirdSound, &mMatrix.GetTranslation(), &mVelocity);
}

// FUNCTION: SWBF2 0x004495b0
void BirdParticle::RespawnScatterBird(const PblVector3& vPos, const BirdParticleType& type,
                                      const PblVector3& vAway)
{
    mState = BIRD_SCATTER;
    mTimeOffCamera = 0.0f;
    mTotalLifetime = 0.0f;
    mFlapTime = PblRandom::GetGlobalFloat() * PblMath::fTwoPi;

    PblQuaternion q = PblQuaternion(gMatrixIdentity);
    q = RotateByOmega(q, PblVector3(0.0f, PblRandom::GetGlobalFloat() * PblMath::fTwoPi, 0.0f));
    q = RotateByOmega(q, PblVector3((-0.25f - PblRandom::GetGlobalFloat() * 0.25f) * fHalfPi, 0.0f, 0.0f));
    mMatrix = MatrixFromQuat(q, vPos);

    mRandomness = PblRandom::GetGlobalFloat();

    if (vAway.y >= -EPSILON &&
        mMatrix.m[2][0] * vAway.x + mMatrix.m[2][1] * vAway.y + mMatrix.m[2][2] * vAway.z < 0.0f)
    {

        PblVector3 vX(-mMatrix.m[0][0], mMatrix.m[0][1], -mMatrix.m[0][2]);
        PblVector3 vY(-mMatrix.m[1][0], mMatrix.m[1][1], -mMatrix.m[1][2]);
        PblVector3 vZ(-mMatrix.m[2][0], mMatrix.m[2][1], -mMatrix.m[2][2]);
        mMatrix.m[0][0] = vX.x;
        mMatrix.m[0][1] = vX.y;
        mMatrix.m[0][2] = vX.z;
        mMatrix.m[0][3] = 0.0f;
        mMatrix.m[1][0] = vY.x;
        mMatrix.m[1][1] = vY.y;
        mMatrix.m[1][2] = vY.z;
        mMatrix.m[1][3] = 0.0f;
        mMatrix.m[2][0] = vZ.x;
        mMatrix.m[2][1] = vZ.y;
        mMatrix.m[2][2] = vZ.z;
        mMatrix.m[2][3] = 0.0f;
    }

    mInitMatrix = mMatrix;
    float fSpeed = mRandomness * 8.0f + 8.0f;
    mVelocity = PblVector3(mInitMatrix.m[2][0] * fSpeed, mInitMatrix.m[2][1] * fSpeed, mInitMatrix.m[2][2] * fSpeed);
    mIsValid = true;
    mType = type;

    SetRenderRadius(mType.mSize);
    SetRenderPos(vPos);
    Activate();
    StartBirdSound();
    mBirdSound.Update(&BirdParticleManager::sInstance->mBirdSound, &mMatrix.GetTranslation(), &mVelocity);
}

// FUNCTION: SWBF2 0x00449bf0
void BirdParticle::UpdateFlying(float fDeltaTime)
{
    PblVector3 vInitPos = mInitMatrix.GetTranslation();
    float fFlap = (mRandomness * 8.0f + 8.0f) * mFlapTime;

    PoseWing(mWing1Matrix, PblMath::Sin(fFlap) * (fHalfPi * 0.5f));
    float fWing2 = PblMath::Sin(fFlap - 0.5654867f) * (fHalfPi * 0.7f);
    PoseWing(mWing2Matrix, fWing2);

    float fBob = PblMath::Sin(fWing2) * 0.1f;
    const PblVector3& vUp = mInitMatrix.GetYAxis();
    PblVector3 vPos(mVelocity.x * mTotalLifetime + (vInitPos.x - vUp.x * fBob),
                    mVelocity.y * mTotalLifetime + (vInitPos.y - vUp.y * fBob),
                    mVelocity.z * mTotalLifetime + (vInitPos.z - vUp.z * fBob));
    mMatrix.m[3][0] = vPos.x;
    mMatrix.m[3][1] = vPos.y;
    mMatrix.m[3][2] = vPos.z;
    mMatrix.m[3][3] = 1.0f;
    SetRenderPos(vPos);

    mTotalLifetime += fDeltaTime;
    mFlapTime += fDeltaTime;
    if (mFlapTime > PblMath::fTwoPi)
    {
        mFlapTime -= PblMath::fTwoPi;
    }

    mBirdSound.Update(&BirdParticleManager::sInstance->mBirdSound, &mMatrix.GetTranslation(), &mVelocity);
}

// FUNCTION: SWBF2 0x0044adf0
BirdParticleManager::BirdParticleManager()
{
    mNextFreeBird = mBirdParticles;
    memset(mNumScatterBirds, 0, sizeof(mNumScatterBirds));
    mScatterTimer = 0.0f;
    mFlockTimer = 0.0f;
    mStandTimer = 0.0f;
}

// SYNTHETIC: SWBF2 0x0044ae90
// BirdParticleManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0044aec0
BirdParticleManager::~BirdParticleManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0044af90
bool BirdParticleManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = new BirdParticleManager;
    sInstance->ActivateThread(0, PER_FRAME);
    memset(sBirdTypes, 0, sizeof(sBirdTypes));
    sNumBirdTypes = 0;
    sFlockMinHeight = -1025.0f;
    return true;
}

// FUNCTION: SWBF2 0x0044b050
bool BirdParticleManager::Destroy()
{
    if (!sInstance)
    {
        return false;
    }
    PlatformCleanup();
    delete sInstance;
    sInstance = 0;
    return true;
}

// FUNCTION: SWBF2 0x0044b430
BirdParticle* BirdParticleManager::GetFreeBird()
{
    BirdParticleManager* pManager = BirdParticleManager::sInstance;
    if (!pManager)
    {
        return NULL;
    }
    BirdParticle* pFree = pManager->mNextFreeBird;
    if (pFree)
    {
        for (int i = 0; i < 15; ++i)
        {
            BirdParticle* pBird = &pManager->mBirdParticles[i];
            if (!pBird->mIsValid && pBird != pFree)
            {
                pManager->mNextFreeBird = pBird;
            }
        }
    }
    return pFree;
}

// FUNCTION: SWBF2 0x0044b480
void BirdParticleManager::ScatterRandomBirds(int iNum, const PblVector3& vPos)
{
    BirdParticleManager* pManager = BirdParticleManager::sInstance;
    if (!pManager)
    {
        return;
    }
    bool bScattered = false;
    unsigned int uRandom = 0;
    for (int i = 0; i < iNum; ++i)
    {
        BirdParticle* pBird = GetFreeBird();
        if (!pBird)
        {
            continue;
        }
        uRandom = PblRandom::GetGlobalInt();
        union { unsigned int u; float f; } randomFloat;
        randomFloat.u = (uRandom >> 9) | 0x3f800000u;
        if (BirdParticleManager::sNumBirdTypes)
        {
            int iType = 1;
            float fThreshold = 0.0f;
            float fStep = 1.0f / (float)BirdParticleManager::sNumBirdTypes;
            while (iType <= BirdParticleManager::sNumBirdTypes)
            {
                fThreshold += fStep;
                if (fThreshold >= randomFloat.f - 1.0f)
                {
                    pBird->RespawnScatterBird(vPos, BirdParticleManager::sBirdTypes[iType - 1],
                                              PblVector3(0.0f, -1.0f, 0.0f));
                    break;
                }
                ++iType;
            }
        }
        bScattered = true;
    }
    if (bScattered)
    {
        pManager->mScatterSound.Play(&vPos, NULL, true, true);
    }
    pManager->mScatterTimer = 0.0f;
}

// FUNCTION: SWBF2 0x0044bdb0
bool BirdParticleManager::Update(float fDeltaTime)
{
    mScatterTimer += fDeltaTime;
    mFlockTimer += fDeltaTime;
    mStandTimer += fDeltaTime;

    if (sNumBirdTypes)
    {
        if (mScatterTimer > 20.0f)
        {
            int iPoint = (int)(PblRandom::GetGlobalFloat() * 5.0f);
            if (iPoint >= 5)
            {
                iPoint = 4;
            }
            ScatterRandomBirds(mNumScatterBirds[iPoint], mPotentialScatterPoints[iPoint]);
        }

        if (mFlockTimer > 25.0f)
        {
            int iCam = PblRandom::GetGlobalInt() % CameraManager::Instance()->GetNumCameras();
            PblMatrix mCamera = CameraManager::Instance()->GetCamera(iCam)->GetMatrix();
            const D3DXVECTOR3& vDir = *(const D3DXVECTOR3*)&mCamera.m[2][0];
            const PblVector3& vCamPos = *(const PblVector3*)&mCamera.m[3][0];
            const D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);

            D3DXVECTOR3 vTemp;
            PblVector3 vSide;
            D3DXVec3Normalize((D3DXVECTOR3*)&vSide, D3DXVec3Cross(&vTemp, &vUp, &vDir));
            PblVector3 vForward;
            D3DXVec3Normalize((D3DXVECTOR3*)&vForward, D3DXVec3Cross(&vTemp, &vUp, (const D3DXVECTOR3*)&vSide));

            if (PblRandom::GetGlobalFloat() <= 0.5f)
            {
                vSide = PblVector3(-vSide.x, -vSide.y, -vSide.z);
            }

            float fAhead = PblRandom::GetGlobalFloat() + 1.5f;
            PblVector3 vPos(vCamPos.x + (vForward.x * fAhead - vSide.x * 1.1f) * 20.0f,
                            vCamPos.y + (vForward.y * fAhead - vSide.y * 1.1f) * 20.0f,
                            vCamPos.z + (vForward.z * fAhead - vSide.z * 1.1f) * 20.0f);

            if (sFlockMinHeight < -1024.0f)
            {
                vPos.y = RedTerrain::GetHoleHeight(vPos);
                float fRaise = PblRandom::GetGlobalFloat() * 5.0f + 10.0f;
                vPos.x += vUp.x * fRaise;
                vPos.y += vUp.y * fRaise;
                vPos.z += vUp.z * fRaise;
            }
            else
            {
                vPos.y = PblRandom::GetGlobalFloat() * 5.0f + sFlockMinHeight;
            }

            while (CollisionManager::RayTest(vPos, vSide, 200.0f, 0, 0, 0x90))
            {
                vPos.x += vUp.x * 5.0f;
                vPos.y += vUp.y * 5.0f;
                vPos.z += vUp.z * 5.0f;
            }

            FlockRandomBirds(vPos, PblVector3(vSide.x * 5.0f, vSide.y * 5.0f, vSide.z * 5.0f));
        }

        if (mStandTimer > 35.0f)
        {
            SpawnGroundBirds();
        }

        for (int i = 0; i < 15; ++i)
        {
            mBirdParticles[i].Update(fDeltaTime);
            if (!mBirdParticles[i].mIsValid)
            {
                mNextFreeBird = &mBirdParticles[i];
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00446d70
int __fastcall ReadIteratorSecondWord(const unsigned int* pThis)
{
    return pThis[1];
}

// FUNCTION: SWBF2 0x00446d80
unsigned int* __fastcall SetIteratorNodeWord(unsigned int* pThis, void*, unsigned int value)
{
    *pThis = value;
    return pThis;
}

// FUNCTION: SWBF2 0x00446d90
unsigned int __fastcall sub_446D90(const unsigned int* pThis)
{
    return *(const unsigned int*)(*pThis + 12);
}

// FUNCTION: SWBF2 0x00446da0
unsigned int __fastcall sub_446DA0(const unsigned int* pThis)
{
    return *(const unsigned int*)(*pThis + 12);
}

// FUNCTION: SWBF2 0x00446db0
unsigned int __fastcall sub_446DB0(const unsigned int* pThis)
{
    return *(const unsigned int*)(*pThis + 12);
}

// FUNCTION: SWBF2 0x00446dc0
unsigned int* __fastcall AdvanceIteratorNodeWord(unsigned int* pThis)
{
    *pThis = *(const unsigned int*)(*pThis + 4);
    return pThis;
}
