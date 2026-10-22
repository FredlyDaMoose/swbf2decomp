#include <math.h>
#include <stdio.h>

#include "Trackable.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "DeathCamera.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "FirstPerson.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "Tracker.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Graphics/RedCamera.h"

// FUNCTION: SWBF2 0x004832d0
GameObject* Trackable::GetGameObject() { return 0; }

// FUNCTION: SWBF2 0x004832f0
Controllable* Trackable::GetControllable() { return 0; }

// FUNCTION: SWBF2 0x004832e0
const GameObject* Trackable::GetGameObject() const { return 0; }

// FUNCTION: SWBF2 0x00483300
const Controllable* Trackable::GetControllable() const { return 0; }

// FUNCTION: SWBF2 0x00658640
void Trackable::TrackDeath(int iPlayer, Character* pKiller)
{
    Tracker::SetDeathTrackable(this, iPlayer, pKiller);
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixInverse(PblMatrix* pOut, float* pDeterminant, const PblMatrix* pM);

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

extern PblVector3 gVectorZero;

namespace FLRenderer
{
    unsigned int GetCurrentViewport();
}

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

namespace
{

    void* const* VTable(const void* pThis)
    {
        return *reinterpret_cast<void* const* const*>(pThis);
    }

    bool GameObject_IsRtti(const GameObject* pObject, unsigned int uRtti)
    {
        typedef bool(__thiscall * Fn)(const GameObject*, unsigned int);
        return reinterpret_cast<Fn>(VTable(pObject)[0])(pObject, uRtti);
    }

    GameObject* GameObject_GetGameObject(GameObject* pObject)
    {
        typedef GameObject*(__thiscall * Fn)(GameObject*);
        return reinterpret_cast<Fn>(VTable(pObject)[9])(pObject);
    }

    const PblMatrix& GameObject_GetSmoothedMatrix(GameObject* pObject)
    {
        typedef const PblMatrix&(__thiscall * Fn)(GameObject*);
        return reinterpret_cast<Fn>(VTable(pObject)[68])(pObject);
    }

    const PblVector3& Row(const PblMatrix& rMatrix, int iRow)
    {
        return *reinterpret_cast<const PblVector3*>(rMatrix.m[iRow]);
    }
}

static const float Sign(float f)
{
    return f < 0.0f ? -1.0f : 1.0f;
}

// FUNCTION: SWBF2 0x004774b0
bool Trackable::IsFirstPersonAvailablePS2() const
{
    return true;
}

// FUNCTION: SWBF2 0x004774c0
bool Trackable::IsForcedFirstPerson() const
{
    return false;
}

// FUNCTION: SWBF2 0x00483310
CameraTrackSetting* Trackable::GetCameraTrackSettings()
{
    return 0;
}

// FUNCTION: SWBF2 0x00483320
bool Trackable::IsForcedThirdPerson() const
{
    return false;
}

// FUNCTION: SWBF2 0x006585b0
Trackable::~Trackable()
{
    Tracker::RemoveTrackable(this);
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        CameraManager* pManager = CameraManager::sInstance;
        if (pManager->mChaseCamera[i]->GetOwner() == this)
        {
            pManager->mChaseCamera[i]->SetOwner(0);
        }
        if (pManager->mDeathCamera[i]->mOwner == this)
        {
            pManager->mDeathCamera[i]->SetInfo(0, 0, pManager->mChaseCamera[i]->GetMatrix());
        }
    }
}

// SYNTHETIC: SWBF2 0x00483330
// Trackable::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00658630
void Trackable::Track(int iPlayer)
{
    Tracker::SetTrackable(this, iPlayer);
}

// FUNCTION: SWBF2 0x00658660
void Trackable::SetTracker(Tracker* pTracker)
{
    mTracker = pTracker;
}

// FUNCTION: SWBF2 0x00658670
void Trackable::SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix)
{
    GameObject* pObject = GetGameObject();
    Controllable* pControllable = GetControllable();
    if (pObject)
    {
        PblMatrix mObject = GameObject_GetSmoothedMatrix(GameObject_GetGameObject(pObject));
        PblVector3 vDir(-pControllable->mEyeDir.x, -pControllable->mEyeDir.y, -pControllable->mEyeDir.z);
        rMatrix = DirectionMatrix(vDir, Row(mObject, 1), pControllable->mEyePoint);
    }
}

// FUNCTION: SWBF2 0x00658770
void Trackable::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{
}

// FUNCTION: SWBF2 0x00658780
void Trackable::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix)
{
}

// FUNCTION: SWBF2 0x00658790
void Trackable::SetupCameraZoomFactor(float& rZoom)
{
}

// FUNCTION: SWBF2 0x006587a0
void Trackable::SetupCameraFOV(float& rFOV)
{
}

// FUNCTION: SWBF2 0x006587b0
void Trackable::SetupCameraIconVisibility(unsigned int& rMask)
{
}

// FUNCTION: SWBF2 0x006587c0
void Trackable::SetFirstPersonView(int iPlayer, bool bFirstPerson)
{
    if (!NetGame::IsEnabled() || NetGame::IsClient() || (NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        int iLocal = NetGame::GetJoystickIndex(iPlayer);
        if (iLocal >= 0)
        {
            FirstPerson::Activate(iLocal, bFirstPerson);
        }
    }
}

// FUNCTION: SWBF2 0x00658830
bool Trackable::CheckFirstPerson(int iPlayer) const
{

    Controllable* pControllable = const_cast<Controllable*>(GetControllable());
    if (pControllable)
    {
        if (pControllable->GetControllableClass()->mForceMode == 1)
        {
            return false;
        }
        if (pControllable->GetControllableClass()->mForceMode == 2)
        {
            return true;
        }
    }
    if (IsForcedThirdPerson())
    {
        return false;
    }
    if (IsForcedFirstPerson())
    {
        return true;
    }
    if (iPlayer < 0)
    {
        return false;
    }

    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        !(NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::IsCockpitCam(iPlayer);
    }

    int iLocal = NetGame::GetJoystickIndex(iPlayer);
    if (iLocal < 0)
    {
        if (NetGame::IsEnabled() && NetGame::IsHost())
        {
            return NetGame::IsCockpitCam(iPlayer);
        }
        return false;
    }
    if (iLocal < 1 && playerJoystickNumbers[11 * iLocal] >= 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(playerJoystickNumbers[11 * iLocal]);
        if (pProfile)
        {
            if (GetGameObject() && GameObject_IsRtti(GetGameObject(), EntityFlyer::rttiHashEntityFlyer))
            {
                return pProfile->mGameOptions.mCockpitCamFlyer;
            }
            return pProfile->mGameOptions.mCockpitCam;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00658980
void Trackable::UpdateFirstThirdPerson(int iPlayer)
{
    if (mTracker)
    {
        bool bFirstPerson = CheckFirstPerson(iPlayer);
        if (mTracker->IsFirstPersonView() != bFirstPerson)
        {
            mTracker->SetFirstPersonView(iPlayer, bFirstPerson);
        }
    }
}

// FUNCTION: SWBF2 0x006589c0
bool Trackable::RenderTrackable(unsigned int uView) const
{
    int iLocal = NetGame::GetJoystickIndex(uView);
    bool bThisViewport = FLRenderer::GetCurrentViewport() == (unsigned int)iLocal;
    return !(mTracker && mTracker->IsFirstPersonView() && bThisViewport);
}

// FUNCTION: SWBF2 0x00658a10
void Trackable::SwitchControlView(int iPlayer)
{
    int iLocal = NetGame::GetJoystickIndex(iPlayer);
    if (iLocal >= 0 && iLocal < 1 && playerJoystickNumbers[11 * iLocal] >= 0)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(playerJoystickNumbers[11 * iLocal]);
        if (pProfile)
        {
            if (GetGameObject() && GameObject_IsRtti(GetGameObject(), EntityFlyer::rttiHashEntityFlyer))
            {
                pProfile->mGameOptions.mCockpitCamFlyer = !pProfile->mGameOptions.mCockpitCamFlyer;
            }
            else
            {
                pProfile->mGameOptions.mCockpitCam = !pProfile->mGameOptions.mCockpitCam;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00658a90
CameraTrackSetting::CameraTrackSetting()
{
    mCameraEyePointOffset.x = 0.0f;
    mCameraEyePointOffset.y = 0.0f;
    mCameraEyePointOffset.z = 0.0f;
    mCameraTrackCenter.x = 0.0f;
    mCameraTrackCenter.y = 0.0f;
    mCameraTrackCenter.z = 0.0f;
    mCameraTrackOffset.x = 0.0f;
    mCameraTrackOffset.y = 0.0f;
    mCameraTrackOffset.z = 0.0f;
    mCameraSafePoint.x = 0.0f;
    mCameraSafePoint.y = 1.5f;
    mCameraSafePoint.z = 0.0f;
    mCameraMoveTensionL = 30.0f;
    mCameraMoveTensionR = 30.0f;
    mCameraMoveTensionU = 30.0f;
    mCameraMoveTensionD = 30.0f;
    mCameraMoveTensionF = 30.0f;
    mCameraMoveTensionB = 30.0f;
    mCameraAimTension = 1000.0f;
    mCameraTiltValue = 0.0f;
}

// FUNCTION: SWBF2 0x00658b20
PblVector3 CameraTrackSetting::CalcEyePoint(const PblMatrix& rMatrix)
{
    PblVector3 vPoint;
    D3DXVec3TransformCoord(&vPoint, &mCameraEyePointOffset, &rMatrix);
    return vPoint;
}

// FUNCTION: SWBF2 0x00658b40
PblVector3 CameraTrackSetting::CalcCenterPoint(const PblMatrix& rMatrix)
{
    PblVector3 vPoint;
    D3DXVec3TransformCoord(&vPoint, &mCameraTrackCenter, &rMatrix);
    return vPoint;
}

// FUNCTION: SWBF2 0x00658b60
PblVector3 CameraTrackSetting::CalcCameraSafePoint(const PblMatrix& rMatrix)
{
    PblVector3 vPoint;
    D3DXVec3TransformCoord(&vPoint, &mCameraSafePoint, &rMatrix);
    return vPoint;
}

// FUNCTION: SWBF2 0x00658b80
PblVector3 CameraTrackSetting::CalcCameraPoint(const PblMatrix& rMatrix, const PblMatrix& rRotation)
{
    PblVector3 vOffset;
    D3DXVec3TransformNormal(&vOffset, &mCameraTrackOffset, &rRotation);
    PblVector3 vCenter = CalcCenterPoint(rMatrix);
    PblVector3 vPoint;
    vPoint.x = vOffset.x + vCenter.x;
    vPoint.y = vCenter.y + vOffset.y;
    vPoint.z = vOffset.z + vCenter.z;
    return vPoint;
}

// FUNCTION: SWBF2 0x00658bf0
float CameraTrackSetting::GetCameraTiltValue()
{
    float fTilt = mCameraTiltValue;
    if (CameraManager::sInstance->GetNumCameras() > 1)
    {
        fTilt *= 0.4f;
    }
    return fTilt;
}

// FUNCTION: SWBF2 0x00658c10
void CameraTrackSetting::SetupCameraTrackMatrixHUDTargeting(PblMatrix& rMatrix, const PblMatrix& rTarget,
                                                            const PblVector3& vAimDir)
{
    PblVector3 vDir(-vAimDir.x, -vAimDir.y, -vAimDir.z);
    PblQuaternion qLook = PblQuaternion(DirectionMatrix(vDir, Row(rTarget, 1), gVectorZero));

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, -GetCameraTiltValue() * 0.5f);
    PblQuaternion qTilt;
    qTilt._fX = fSin;
    qTilt._fY = fSin * 0.0f;
    qTilt._fZ = fSin * 0.0f;
    qTilt._fS = fCos;

    PblQuaternion q = qLook * qTilt;
    PblQuaternion qUnit;
    PblMatrix mCamera;
    D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &q));
    mCamera.m[3][0] = gVectorZero.x;
    mCamera.m[3][1] = gVectorZero.y;
    mCamera.m[3][2] = gVectorZero.z;
    mCamera.m[3][3] = 1.0f;

    PblVector3 vPoint = CalcCameraPoint(rTarget, mCamera);
    mCamera.m[3][0] = vPoint.x;
    mCamera.m[3][1] = vPoint.y;
    mCamera.m[3][2] = vPoint.z;
    mCamera.m[3][3] = 1.0f;
    rMatrix = mCamera;
}

// FUNCTION: SWBF2 0x00658dc0
void CameraTrackSetting::SetupCameraTrackMatrixHUDTargeting(const CameraTrackSetting& rOther, float fBlend,
                                                            PblMatrix& rMatrix, const PblMatrix& rTarget,
                                                            const PblVector3& vAimDir)
{
    PblVector3 vTrackCenter = Interpolate(rOther.mCameraTrackCenter, mCameraTrackCenter, fBlend);
    PblVector3 vTrackOffset = Interpolate(rOther.mCameraTrackOffset, mCameraTrackOffset, fBlend);
    float fTilt = (mCameraTiltValue - rOther.mCameraTiltValue) * fBlend + rOther.mCameraTiltValue;
    if (CameraManager::sInstance->GetNumCameras() > 1)
    {
        fTilt *= 0.4f;
    }

    PblVector3 vCenter;
    D3DXVec3TransformCoord(&vCenter, &vTrackCenter, &rTarget);

    PblVector3 vDir(-vAimDir.x, -vAimDir.y, -vAimDir.z);
    PblQuaternion qLook = PblQuaternion(DirectionMatrix(vDir, Row(rTarget, 1), gVectorZero));

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fTilt * -0.5f);
    PblQuaternion qTilt;
    qTilt._fX = fSin;
    qTilt._fY = fSin * 0.0f;
    qTilt._fZ = fSin * 0.0f;
    qTilt._fS = fCos;

    PblQuaternion q = qLook * qTilt;
    PblQuaternion qUnit;
    PblMatrix mCamera;
    D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &q));
    mCamera.m[3][0] = gVectorZero.x;
    mCamera.m[3][1] = gVectorZero.y;
    mCamera.m[3][2] = gVectorZero.z;
    mCamera.m[3][3] = 1.0f;

    PblVector3 vOffset;
    D3DXVec3TransformNormal(&vOffset, &vTrackOffset, &mCamera);
    PblVector3 vPoint;
    D3DXVec3TransformCoord(&vPoint, &vTrackCenter, &rTarget);
    mCamera.m[3][0] = vOffset.x + vPoint.x;
    mCamera.m[3][1] = vOffset.y + vPoint.y;
    mCamera.m[3][2] = vOffset.z + vPoint.z;
    mCamera.m[3][3] = 1.0f;
    rMatrix = mCamera;
}

// FUNCTION: SWBF2 0x00659090
void CameraTrackSetting::SetupCameraTrackMatrix(Trackable* pTrackable, PblMatrix& rMatrix, float fDeltaTime,
                                                const PblMatrix& rTarget, const PblVector3& vAimDir,
                                                float fMoveTensionScale, float fAimTensionScale,
                                                const GameObject* pIgnore)
{
    PblQuaternion qCurrent = PblQuaternion(rMatrix);
    PblVector3 vCurrent = Row(rMatrix, 3);

    PblVector3 vCenter = CalcCenterPoint(rTarget);
    PblVector3 vToCenter(vCenter.x - vCurrent.x, vCenter.y - vCurrent.y, vCenter.z - vCurrent.z);
    bool bSnap = vToCenter.x * vToCenter.x + vToCenter.y * vToCenter.y + vToCenter.z * vToCenter.z > 62500.0f;

    PblVector3 vDir(-vAimDir.x, -vAimDir.y, -vAimDir.z);
    PblQuaternion qTarget = PblQuaternion(DirectionMatrix(vDir, Row(rTarget, 1), gVectorZero));

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, -GetCameraTiltValue() * 0.5f);
    PblQuaternion qTilt;
    qTilt._fX = fSin;
    qTilt._fY = fSin * 0.0f;
    qTilt._fZ = fSin * 0.0f;
    qTilt._fS = fCos;
    qTarget = qTarget * qTilt;

    PblQuaternion q;
    if (bSnap)
    {
        q = qTarget;
    }
    else
    {
        q = Interpolate(qTarget, qCurrent, expf(-(mCameraAimTension * fAimTensionScale * fDeltaTime)));
    }

    PblQuaternion qUnit;
    PblMatrix mCamera;
    D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &q));
    mCamera.m[3][0] = gVectorZero.x;
    mCamera.m[3][1] = gVectorZero.y;
    mCamera.m[3][2] = gVectorZero.z;
    mCamera.m[3][3] = 1.0f;

    PblVector3 vCamera = CalcCameraPoint(rTarget, mCamera);
    PblVector3 vSafe = CalcCameraSafePoint(rTarget);
    PblVector3 vDelta(vCamera.x - vSafe.x, vCamera.y - vSafe.y, vCamera.z - vSafe.z);
    if (vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z > 0.0001f)
    {
        PblVector3 vRay;
        D3DXVec3Normalize(&vRay, &vDelta);

        float fNear = 0.0f;
        float fFar = 0.0f;
        float fWidth = 0.0f;
        float fHeight = 0.0f;
        unsigned int uCamera = CameraManager::sInstance->mUpdatedCameraId;
        CameraManager::sInstance->GetCamera(uCamera)->GetFrustum(fNear, fFar, fWidth, fHeight);

        PblVector3 vNear(vCamera.x - vRay.x * fNear, vCamera.y - vRay.y * fNear, vCamera.z - vRay.z * fNear);
        const GameObject* apIgnore[1] = { pIgnore };

        const PblVector3& vRight = Row(mCamera, 0);
        const PblVector3& vUp = Row(mCamera, 1);
        PblVector3 vCorner;
        switch (pTrackable->whichCornerToCastForCamera)
        {
        case 0:
            vCorner = PblVector3(0.0f, 0.0f, 0.0f);
            break;
        case 1:
            vCorner = PblVector3(vRight.x * 0.4f + vUp.x * 0.4f, vRight.y * 0.4f + vUp.y * 0.4f,
                                 vRight.z * 0.4f + vUp.z * 0.4f);
            break;
        case 2:
            vCorner = PblVector3(vRight.x * -0.4f + vUp.x * 0.4f, vRight.y * -0.4f + vUp.y * 0.4f,
                                 vRight.z * -0.4f + vUp.z * 0.4f);
            break;
        case 3:
            vCorner = PblVector3(vRight.x * 0.4f + vUp.x * -0.4f, vRight.y * 0.4f + vUp.y * -0.4f,
                                 vRight.z * 0.4f + vUp.z * -0.4f);
            break;
        case 4:
            vCorner = PblVector3(vRight.x * -0.4f + vUp.x * -0.4f, vRight.y * -0.4f + vUp.y * -0.4f,
                                 vRight.z * -0.4f + vUp.z * -0.4f);
            break;
        }

        PblVector3 vCastStart(vSafe.x + vCorner.x, vSafe.y + vCorner.y, vSafe.z + vCorner.z);
        PblVector3 vCastEnd(vNear.x + vCorner.x, vNear.y + vCorner.y, vNear.z + vCorner.z);
        CollisionObject* pHitObject;
        pTrackable->minCameraCastDist[pTrackable->whichCornerToCastForCamera] =
            CollisionManager::RayHit(vCastStart, vCastEnd, pHitObject, 0, apIgnore, pIgnore != 0, 0x98, true);

        float fDistance = 1.0f;
        for (int i = 0; i < 5; ++i)
        {
            if (pTrackable->minCameraCastDist[i] < fDistance)
            {
                fDistance = pTrackable->minCameraCastDist[i];
            }
        }
        fDistance = CameraManager::sInstance->UpdateTrackDistance(uCamera, fDistance, fDeltaTime);

        vCamera.x = (vNear.x - vSafe.x) * fDistance + vSafe.x + vRay.x * fNear;
        vCamera.y = (vNear.y - vSafe.y) * fDistance + vSafe.y + vRay.y * fNear;
        vCamera.z = (vNear.z - vSafe.z) * fDistance + vSafe.z + vRay.z * fNear;

        if (++pTrackable->whichCornerToCastForCamera > 4)
        {
            pTrackable->whichCornerToCastForCamera = 0;
        }
    }

    if (!bSnap)
    {
        PblVector3 vMove(vCamera.x - vCurrent.x, vCamera.y - vCurrent.y, vCamera.z - vCurrent.z);
        PblMatrix mInverse;
        D3DXMatrixInverse(&mInverse, 0, &mCamera);
        PblVector3 vLocal;
        D3DXVec3TransformNormal(&vLocal, &vMove, &mInverse);

        float fTension = vLocal.x > 0.0f ? mCameraMoveTensionL : mCameraMoveTensionR;
        vLocal.x = (1.0f - expf(-(fTension * fMoveTensionScale * fDeltaTime))) * vLocal.x;
        fTension = vLocal.y > 0.0f ? mCameraMoveTensionU : mCameraMoveTensionD;
        vLocal.y = (1.0f - expf(-(fTension * fMoveTensionScale * fDeltaTime))) * vLocal.y;
        fTension = vLocal.z > 0.0f ? mCameraMoveTensionF : mCameraMoveTensionB;
        vLocal.z = (1.0f - expf(-(fTension * fMoveTensionScale * fDeltaTime))) * vLocal.z;

        PblVector3 vWorld;
        D3DXVec3TransformNormal(&vWorld, &vLocal, &mCamera);
        vCamera.x = vCurrent.x + vWorld.x;
        vCamera.y = vCurrent.y + vWorld.y;
        vCamera.z = vWorld.z + vCurrent.z;
    }

    mCamera.m[3][0] = vCamera.x;
    mCamera.m[3][1] = vCamera.y;
    mCamera.m[3][2] = vCamera.z;
    mCamera.m[3][3] = 1.0f;
    rMatrix = mCamera;
}

// FUNCTION: SWBF2 0x006599e0
void CameraTrackSetting::SetupCameraTrackMatrix(Trackable* pTrackable, const CameraTrackSetting& rOther,
                                                float fBlend, PblMatrix& rMatrix, float fDeltaTime,
                                                const PblMatrix& rTarget, const PblVector3& vAimDir,
                                                float fMoveTensionScale, float fAimTensionScale,
                                                const GameObject* pIgnore)
{
    PblQuaternion qCurrent = PblQuaternion(rMatrix);
    PblVector3 vCurrent = Row(rMatrix, 3);

    PblVector3 vTrackCenter = Interpolate(rOther.mCameraTrackCenter, mCameraTrackCenter, fBlend);
    PblVector3 vTrackOffset = Interpolate(rOther.mCameraTrackOffset, mCameraTrackOffset, fBlend);
    PblVector3 vSafePoint = Interpolate(rOther.mCameraSafePoint, mCameraSafePoint, fBlend);
    float fTilt = (mCameraTiltValue - rOther.mCameraTiltValue) * fBlend + rOther.mCameraTiltValue;
    if (CameraManager::sInstance->GetNumCameras() > 1)
    {
        fTilt *= 0.4f;
    }

    PblVector3 vCenter;
    D3DXVec3TransformCoord(&vCenter, &vTrackCenter, &rTarget);
    PblVector3 vToCenter(vCenter.x - vCurrent.x, vCenter.y - vCurrent.y, vCenter.z - vCurrent.z);
    bool bSnap = vToCenter.y * vToCenter.y + vToCenter.x * vToCenter.x + vToCenter.z * vToCenter.z > 22500.0f;

    PblVector3 vDir(-vAimDir.x, -vAimDir.y, -vAimDir.z);
    PblQuaternion qTarget = PblQuaternion(DirectionMatrix(vDir, Row(rTarget, 1), gVectorZero));

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fTilt * -0.5f);
    PblQuaternion qTilt;
    qTilt._fX = fSin;
    qTilt._fY = fSin * 0.0f;
    qTilt._fZ = fSin * 0.0f;
    qTilt._fS = fCos;
    qTarget = qTarget * qTilt;

    PblQuaternion q;
    if (bSnap)
    {
        q = qTarget;
    }
    else
    {
        q = Interpolate(qTarget, qCurrent, expf(-(mCameraAimTension * fAimTensionScale * fDeltaTime)));
    }

    PblQuaternion qUnit;
    PblMatrix mCamera;
    D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &q));
    mCamera.m[3][0] = gVectorZero.x;
    mCamera.m[3][1] = gVectorZero.y;
    mCamera.m[3][2] = gVectorZero.z;
    mCamera.m[3][3] = 1.0f;

    PblVector3 vOffset;
    D3DXVec3TransformNormal(&vOffset, &vTrackOffset, &mCamera);
    PblVector3 vPoint;
    D3DXVec3TransformCoord(&vPoint, &vTrackCenter, &rTarget);
    PblVector3 vCamera(vOffset.x + vPoint.x, vOffset.y + vPoint.y, vOffset.z + vPoint.z);
    PblVector3 vSafe;
    D3DXVec3TransformCoord(&vSafe, &vSafePoint, &rTarget);

    PblVector3 vDelta(vCamera.x - vSafe.x, vCamera.y - vSafe.y, vCamera.z - vSafe.z);
    if (vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z > 0.0001f)
    {
        PblVector3 vRay;
        D3DXVec3Normalize(&vRay, &vDelta);

        float fNear = 0.0f;
        float fFar = 0.0f;
        float fWidth = 0.0f;
        float fHeight = 0.0f;
        unsigned int uCamera = CameraManager::sInstance->mUpdatedCameraId;
        CameraManager::sInstance->GetCamera(uCamera)->GetFrustum(fNear, fFar, fWidth, fHeight);

        PblVector3 vNear(vCamera.x - vRay.x * fNear, vCamera.y - vRay.y * fNear, vCamera.z - vRay.z * fNear);
        const GameObject* apIgnore[1] = { pIgnore };
        bool bIgnore = pIgnore != 0;
        CollisionObject* pHitObject;

        float fDistance = CollisionManager::RayHit(vSafe, vNear, pHitObject, 0, apIgnore, bIgnore, 0x90, true);

        const PblVector3& vRight = Row(mCamera, 0);
        const PblVector3& vUp = Row(mCamera, 1);
        PblVector3 avCorner[4] = {
            PblVector3(vUp.x * 0.4f + vRight.x * 0.4f, vUp.y * 0.4f + vRight.y * 0.4f,
                       vUp.z * 0.4f + vRight.z * 0.4f),
            PblVector3(vUp.x * 0.4f - vRight.x * 0.4f, vUp.y * 0.4f - vRight.y * 0.4f,
                       vUp.z * 0.4f - vRight.z * 0.4f),
            PblVector3(-vUp.x * 0.4f + vRight.x * 0.4f, -vUp.y * 0.4f + vRight.y * 0.4f,
                       -vUp.z * 0.4f + vRight.z * 0.4f),
            PblVector3(-vUp.x * 0.4f - vRight.x * 0.4f, -vUp.y * 0.4f - vRight.y * 0.4f,
                       -vUp.z * 0.4f - vRight.z * 0.4f),
        };
        for (int i = 0; i < 4; ++i)
        {
            PblVector3 vCastStart(vSafe.x + avCorner[i].x, vSafe.y + avCorner[i].y, vSafe.z + avCorner[i].z);
            PblVector3 vCastEnd(vNear.x + avCorner[i].x, vNear.y + avCorner[i].y, vNear.z + avCorner[i].z);
            float fHit = CollisionManager::RayHit(vCastStart, vCastEnd, pHitObject, 0, apIgnore, bIgnore, 0x90, true);
            if (fDistance > fHit)
            {
                fDistance = fHit;
            }
        }
        fDistance = CameraManager::sInstance->UpdateTrackDistance(uCamera, fDistance, fDeltaTime);

        vCamera.x = (vNear.x - vSafe.x) * fDistance + vSafe.x + vRay.x * fNear;
        vCamera.y = (vNear.y - vSafe.y) * fDistance + vSafe.y + vRay.y * fNear;
        vCamera.z = (vNear.z - vSafe.z) * fDistance + vSafe.z + vRay.z * fNear;
    }

    if (!bSnap)
    {
        PblVector3 vMove(vCamera.x - vCurrent.x, vCamera.y - vCurrent.y, vCamera.z - vCurrent.z);
        PblMatrix mInverse;
        D3DXMatrixInverse(&mInverse, 0, &mCamera);
        PblVector3 vLocal;
        D3DXVec3TransformNormal(&vLocal, &vMove, &mInverse);

        float fTension = vLocal.x > 0.0f ? mCameraMoveTensionL : mCameraMoveTensionR;
        vLocal.x = (1.0f - expf(-(fTension * fMoveTensionScale * fDeltaTime))) * vLocal.x;
        fTension = vLocal.y > 0.0f ? mCameraMoveTensionU : mCameraMoveTensionD;
        vLocal.y = (1.0f - expf(-(fTension * fMoveTensionScale * fDeltaTime))) * vLocal.y;
        fTension = vLocal.z > 0.0f ? mCameraMoveTensionF : mCameraMoveTensionB;
        vLocal.z = (1.0f - expf(-(fTension * fMoveTensionScale * fDeltaTime))) * vLocal.z;

        PblVector3 vWorld;
        D3DXVec3TransformNormal(&vWorld, &vLocal, &mCamera);
        vCamera.x = vCurrent.x + vWorld.x;
        vCamera.y = vWorld.y + vCurrent.y;
        vCamera.z = vWorld.z + vCurrent.z;
    }

    mCamera.m[3][0] = vCamera.x;
    mCamera.m[3][1] = vCamera.y;
    mCamera.m[3][2] = vCamera.z;
    mCamera.m[3][3] = 1.0f;
    rMatrix = mCamera;
}

// FUNCTION: SWBF2 0x0065a640
void CameraTrackSetting::SetupAmbientShift(PblMatrix& rMatrix, PblVector3& vShift, PblVector3& vTarget,
                                           PblVector3& vSpeed, float fMaxSpeed, float fDeltaTime) const
{
    for (int i = 0; i < 3; ++i)
    {
        SetupAmbientShiftComponent((&vShift.x)[i], (&vTarget.x)[i], (&vSpeed.x)[i], fMaxSpeed, fDeltaTime);
    }

    PblVector3 vWorld;
    D3DXVec3TransformNormal(&vWorld, &vShift, &rMatrix);
    rMatrix.m[3][0] += vWorld.x;
    rMatrix.m[3][1] += vWorld.y;
    rMatrix.m[3][2] += vWorld.z;
    rMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0065a6f0
void CameraTrackSetting::SetupAmbientShiftComponent(float& rShift, float& rTarget, float& rSpeed, float fMaxSpeed,
                                                    float fDeltaTime) const
{
    float fTarget = rTarget;
    if ((fTarget > 0.0f && rShift > fTarget) || (fTarget < 0.0f && fTarget > rShift))
    {
        if (rSpeed == 0.0f)
        {
            rTarget = -fTarget;
        }
        else
        {
            rSpeed -= fMaxSpeed * 2.0f * fDeltaTime;
            if (rSpeed < 0.0f)
            {
                rSpeed = 0.0f;
            }
        }
    }
    else
    {
        rSpeed += fMaxSpeed * 2.0f * fDeltaTime;
        if (rSpeed > fMaxSpeed)
        {
            rSpeed = fMaxSpeed;
        }
    }

    if ((float)fabs(rTarget) > 0.0001f)
    {
        rShift += rSpeed * Sign(rTarget) * fDeltaTime;
    }
    else
    {
        rShift = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0065a7f0
bool CameraTrackSetting::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float fX, fY, fZ;
    float* pA;
    float* pB;
    switch (uProperty)
    {
    case 0x41568C97:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mCameraEyePointOffset = PblVector3(fX, fY, fZ);
        mCameraTrackCenter = PblVector3(fX, fY, fZ);
        return true;
    case 0xE85D5895:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mCameraTrackCenter = PblVector3(fX, fY, fZ);
        return true;
    case 0xFD3D9507:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mCameraTrackOffset = PblVector3(fX, fY, fZ);
        return true;
    case 0x21050BF6:
        if (sscanf(pszValue, "%f", &fX))
        {
            mCameraMoveTensionL = fX;
            mCameraMoveTensionR = fX;
            mCameraMoveTensionU = fX;
            mCameraMoveTensionD = fX;
            mCameraMoveTensionF = fX;
            mCameraMoveTensionB = fX;
        }
        return true;
    case 0x88F1308A:
        pA = &mCameraMoveTensionL;
        pB = &mCameraMoveTensionR;
        break;
    case 0x89F1321D:
        pA = &mCameraMoveTensionU;
        pB = &mCameraMoveTensionD;
        break;
    case 0x86F12D64:
        pA = &mCameraMoveTensionF;
        pB = &mCameraMoveTensionB;
        break;
    case 0x5E3171CC:
        sscanf(pszValue, "%f", &mCameraAimTension);
        return true;
    case 0x359D5227:
        sscanf(pszValue, "%f", &mCameraTiltValue);
        mCameraTiltValue *= 0.017453292f;
        return true;
    default:
        return false;
    }
    if (sscanf(pszValue, "%f %f", pA, pB) == 1)
    {
        *pB = *pA;
    }
    return true;
}
