#include "DeathCamera.h"
#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "GameObject.h"
#include "Trackable.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblQuaternion.h"

static CollisionObject* GetCollisionObject(GameObject* pObject);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0048a2b0
DeathCamera::DeathCamera(CameraManager* pManager, int iView)
    : GameCamera(pManager, iView)
{
    mKiller = 0;
}

// SYNTHETIC: SWBF2 0x0048a2e0
// DeathCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048a310
DeathCamera::~DeathCamera()
{
}

// FUNCTION: SWBF2 0x0048a810
void DeathCamera::SetInfo(Trackable* pOwner, Character* pKiller, const PblMatrix& rMatrix)
{
    mOwner = pOwner;
    mMatrix = rMatrix;
    mTimer = 0.0f;
    mKiller = 0;

    if (!pOwner)
    {
        return;
    }

    GameObject* pObject = pOwner->GetGameObject();
    if (!pObject)
    {
        return;
    }

    const PblMatrix& rObjectMatrix = GetCollisionObject(pObject)->GetMatrix();
    mDeathPos = PblVector3(rObjectMatrix.m[3][0], rObjectMatrix.m[3][1] + 0.8f,
                           rObjectMatrix.m[3][2]);

    PblVector3 vForward(rMatrix.m[2][0], 0.0f, rMatrix.m[2][2]);
    PblVector3 vNormalized;
    D3DXVec3Normalize(&vNormalized, &vForward);
    mDeathMidPos = PblVector3(rObjectMatrix.m[3][0] + vNormalized.x,
                              rObjectMatrix.m[3][1] + 1.0f + 0.5f,
                              rObjectMatrix.m[3][2] + vNormalized.z);

    if (pKiller && pOwner->GetGameObject()->GetControllable()->mCharacter != pKiller)
    {
        mKiller = pKiller;
    }
}

class CollisionManager
{
public:
    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        CollisionObject*& pObject, PblVector3* pvPoint,
                        const GameObject** const ppIgnore, int iNumIgnore, int iMask, bool bFlag);
};

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

PblQuaternion Interpolate(const PblQuaternion& q0, const PblQuaternion& q1, float fT);

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

static CollisionObject* GetCollisionObject(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject);
}

static const PblVector3& GetEyePoint(const Controllable* pUnit)
{
    return pUnit->mEyePoint;
}

// FUNCTION: SWBF2 0x0048a320
void DeathCamera::Update(float fDeltaTime)
{
    mTimer += fDeltaTime;

    if (mOwner)
    {
        GameObject* pObject = mOwner->GetGameObject();
        if (pObject)
        {
            const PblMatrix& rMatrix = GetCollisionObject(pObject)->GetMatrix();
            mDeathPos.x = rMatrix.m[3][0] + 0.0f;
            mDeathPos.y = rMatrix.m[3][1] + 0.8f;
            mDeathPos.z = rMatrix.m[3][2] + 0.0f;
        }
    }

    PblVector3 vOldPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
    PblVector3 vTarget = mDeathPos;
    PblVector3 vEye = mDeathMidPos;
    float fBlend = 1.0f;

    if (mTimer < 1.0f)
    {
        fBlend = mTimer;
    }
    else if (mTimer > 1.5f && mKiller)
    {
        fBlend = (mTimer - 1.5f) * 0.5f;
        if (fBlend >= 1.0f)
        {
            fBlend = 1.0f;
        }

        Controllable* pUnit = mKiller->mVehicle;
        if (!pUnit)
        {
            pUnit = mKiller->mUnit;
        }
        if (pUnit)
        {
            vEye = mDeathPos;
            vTarget = GetEyePoint(pUnit);
        }
    }

    PblVector3 vDelta(vTarget.x - vEye.x, vTarget.y - vEye.y, vTarget.z - vEye.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vDelta);

    CollisionObject* pHitObject;
    PblVector3 vUpper(vEye.x + 0.0f, vEye.y + 0.75f, vEye.z + 0.0f);
    PblVector3 vBack(-vDir.x, -vDir.y, -vDir.z);
    float fHitUpper = CollisionManager::RayHit(vUpper, vBack, 6.0f, pHitObject, NULL, NULL, 0, 0x90, false);
    PblVector3 vLower(vUpper.x, vEye.y - 0.75f, vUpper.z);
    vBack = PblVector3(-vDir.x, -vDir.y, -vDir.z);
    float fHitLower = CollisionManager::RayHit(vLower, vBack, 6.0f, pHitObject, NULL, NULL, 0, 0x90, false);

    float fHit = fHitLower > fHitUpper ? fHitUpper : fHitLower;
    float fDist = fHit * 6.0f - 1.0f;
    PblVector3 vPos(vEye.x - vDir.x * fDist, vEye.y - vDir.y * fDist, vEye.z - vDir.z * fDist);

    vBack = PblVector3(-vDir.x, -vDir.y, -vDir.z);
    PblQuaternion qNew = PblQuaternion(DirectionMatrix(vBack, gVectorUnitY, gVectorZero));
    PblQuaternion qOld = PblQuaternion(mMatrix);
    PblQuaternion qRot = Interpolate(qOld, qNew, fBlend);

    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize(&qUnit, &qRot);
    D3DXMatrixRotationQuaternion(&mNew, &qUnit);
    mNew.m[3][0] = (vPos.x - vOldPos.x) * fBlend + vOldPos.x;
    mNew.m[3][1] = (vPos.y - vOldPos.y) * fBlend + vOldPos.y;
    mNew.m[3][2] = (vPos.z - vOldPos.z) * fBlend + vOldPos.z;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;
}

// FUNCTION: SWBF2 0x0048a7c0
void DeathCamera::SetupCamera(RedCamera* pCamera, float fDeltaTime)
{
    pCamera->SetMatrix(mMatrix);
    pCamera->SetZoom(1.0f);
}

// FUNCTION: SWBF2 0x0048a7f0
void DeathCamera::SetupIconVisibility(int iView)
{
    mManager->m_uiVisibleIconMask[iView] = 0;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0043cd40
PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos)
{
    PblVector3 vFront;
    D3DXVec3Normalize(&vFront, &vDir);

    PblVector3 vCross(vUp.y * vDir.z - vUp.z * vDir.y,
                      vUp.z * vDir.x - vUp.x * vDir.z,
                      vUp.x * vDir.y - vUp.y * vDir.x);
    PblVector3 vRight;
    D3DXVec3Normalize(&vRight, &vCross);

    PblVector3 vNewUp(vFront.y * vRight.z - vFront.z * vRight.y,
                      vFront.z * vRight.x - vFront.x * vRight.z,
                      vFront.x * vRight.y - vFront.y * vRight.x);

    return PblMatrix(vRight, vNewUp, vFront, vPos);
}
