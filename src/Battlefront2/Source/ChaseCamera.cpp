#include <math.h>

#include "ChaseCamera.h"
#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "GameObject.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "ScreenTransition.h"
#include "Team.h"
#include "Trackable.h"
#include "Tracker.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

class CollisionManager
{
public:
    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        CollisionObject*& pObject, PblVector3* pvPoint,
                        const GameObject** const ppIgnore, int iNumIgnore, int iMask, bool bFlag);
};

namespace FLRenderer
{
unsigned int GetViewWidth(unsigned int uView);
unsigned int GetViewHeight(unsigned int uView);
}

float GetDefaultFOV();
float GetWidescreenFOVAdjustment(float fFOV);

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);

namespace AIUtil {

// FUNCTION: SWBF2 0x0043ef40
bool IsDebugOutputVisible()
{
    return false;
}
}

// GLOBAL: SWBF2 0x01e5605f
bool gDemoMode;

// GLOBAL: SWBF2 0x01e2fdb1

bool sStepToNextMember;
// GLOBAL: SWBF2 0x01e2fdb3

bool sStepToPreviousMember;
// GLOBAL: SWBF2 0x01e2fdbc

int sCycleDirection;
// GLOBAL: SWBF2 0x01e2fdc0

int sMemberIndex;
// GLOBAL: SWBF2 0x007e5ef0

int sTeamIndex = 1;

static CollisionObject* GetCollisionObject(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject);
}

static bool IsAlive(GameObject* pObject)
{
    return pObject->mIsAlive;
}

typedef const PblVector3& (__fastcall *GetVelocityFn)(GameObject* pThis, void*);

// FUNCTION: SWBF2 0x00453730
ChaseCamera::ChaseCamera(CameraManager* pManager, int iView)
    : GameCamera(pManager, iView)
{
    mOwner = 0;
    mMatrix = gMatrixIdentity;
    mTimer = 1.0f;
    mShake = 0.0f;
    mShakeSuppressUntil = 0.0f;
    mShakeCount = 0;
    mDemoMode = false;
    mCameraSwing = 0.0f;
    mEndDt = -1.0f;
    mCurrentDt = 0.0f;
}

// SYNTHETIC: SWBF2 0x004537e0
// ChaseCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00453810
ChaseCamera::~ChaseCamera()
{
}

// FUNCTION: SWBF2 0x00453820
void ChaseCamera::Update(float fDeltaTime)
{
    mTimer += fDeltaTime;

    mShake = 0.0f;
    for (int i = 0; i < mShakeCount; ++i)
    {
        mShake += mShakeAmount[i];
        mShakeAmount[i] -= fDeltaTime * mShakeDecay[i];
        if (mShakeAmount[i] <= 0.0f)
        {
            for (int j = i; j < mShakeCount - 1; ++j)
            {
                mShakeAmount[j] = mShakeAmount[j + 1];
                mShakeDecay[j] = mShakeDecay[j + 1];
            }
            --i;
            --mShakeCount;
        }
    }

    Trackable* pTrackable = 0;
    bool bFound = false;
    if (!gDemoMode && !mDemoMode)
    {
        if ((sStepToNextMember || sStepToPreviousMember) && sCycleDirection == 0)
        {
            sCycleDirection = 1;
        }

        Character* pCharacter;
        if (sCycleDirection > 0)
        {
            Team* pTeam = Team::sTeams[sTeamIndex];
            if (sStepToNextMember)
            {
                int iTries = 0;
                do
                {
                    if (++sMemberIndex == pTeam->mMemberCount)
                    {
                        if (++sTeamIndex > 2)
                        {
                            sTeamIndex = 1;
                        }
                        sMemberIndex = 0;
                        pTeam = Team::sTeams[sTeamIndex];
                    }

                    Controllable* pControllable = pTeam->GetMemberPtr(sMemberIndex)->GetControllable();
                    if (pControllable && pControllable->GetGameObject())
                    {
                        pControllable->GetGameObject();
                    }
                } while (iTries++ < 100 && !AIUtil::IsDebugOutputVisible());
                sStepToNextMember = false;
            }
            if (sStepToPreviousMember)
            {
                int iTries = 0;
                do
                {
                    if (--sMemberIndex < 0)
                    {
                        if (--sTeamIndex < 1)
                        {
                            sTeamIndex = 2;
                        }
                        pTeam = Team::sTeams[sTeamIndex];
                        sMemberIndex = pTeam->mMemberCount - 1;
                    }
                    Controllable* pControllable = pTeam->GetMemberPtr(sMemberIndex)->GetControllable();
                    if (pControllable && pControllable->GetGameObject())
                    {
                        pControllable->GetGameObject();
                    }
                } while (iTries++ < 100 && !AIUtil::IsDebugOutputVisible());
                sStepToPreviousMember = false;
            }
            if (sMemberIndex < 0 || sMemberIndex >= pTeam->mMemberCount)
            {
                goto swing;
            }
            pCharacter = pTeam->mMemberArray[sMemberIndex];
        }
        else if (sCycleDirection < 0)
        {
            pCharacter = NetGame::GetLocalPlayer(mView);
            sCycleDirection = 0;
        }
        else
        {
            goto swing;
        }

        if (!pCharacter)
        {
            goto swing;
        }
        if (pCharacter->mVehicle)
        {
            pTrackable = pCharacter->mVehicle->GetTrackable();
        }
        else if (pCharacter->mUnit)
        {
            pTrackable = pCharacter->mUnit->GetTrackable();
        }
        else
        {
            goto swing;
        }
    }
    else
    {
        mTrackTime -= fDeltaTime;
        if (!(mTrackTime < 0.0f) && mOwner)
        {
            goto swing;
        }

        for (int i = 5; i > 0; --i)
        {
            Character* pCharacter = Team::sTeams[(PblRandom::GetGlobalInt() & 1) + 1]->GetRandomLiveCharacter();
            if (!pCharacter)
            {
                continue;
            }
            if (pCharacter->mVehicle)
            {
                bFound = true;
                pTrackable = pCharacter->mVehicle->GetTrackable();
                i = 0;
            }
            else if (pCharacter->mUnit)
            {
                bFound = true;
                pTrackable = pCharacter->mUnit->GetTrackable();
            }
        }
        if (!bFound)
        {
            goto swing;
        }
    }

    if (pTrackable && pTrackable->GetGameObject() && IsAlive(pTrackable->GetGameObject()))
    {
        SetOwner(pTrackable);
    }

swing:
    mCameraSwing += fDeltaTime * 0.5f;
    if (fDeltaTime > 0.0f)
    {
        mDoSwingFlag = true;
    }
}

// FUNCTION: SWBF2 0x00453c40
static float BiasedRandom()
{
    float f = PblRandom::GetGlobalFloat() * 2.0f - 1.0f;
    return f * f * f;
}

// FUNCTION: SWBF2 0x00453cb0
void ChaseCamera::SetOwner(Trackable* pOwner)
{
    if (mOwner != pOwner)
    {
        if (mOwner)
        {
            mTimer = 0.0f;
        }
        else
        {
            mTimer = 1.0f;
        }
        mOwner = pOwner;
        ClearShake();
        mTrackTime = 10.0f;
    }
}

// FUNCTION: SWBF2 0x00453d00
void ChaseCamera::SetupCamera(RedCamera* pCamera, float fDeltaTime)
{
    if (!mOwner)
    {
        pCamera->SetMatrix(mMatrix);
        pCamera->SetZoom(1.0f);
        return;
    }

    PblQuaternion qOld = PblQuaternion(mMatrix);
    PblVector3 vOldPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);

    float fAspectRatio = RedRenderer::GetScreenAspectRatio()
                         * ((float)FLRenderer::GetViewHeight(0) * (float)RedInterfaceScreen::GetScreenWidth())
                         / ((float)FLRenderer::GetViewWidth(0) * (float)RedInterfaceScreen::GetScreenHeight());
    float fZoom = 1.0f;
    mOwner->SetupCameraZoomFactor(fZoom);
    float fFOV = GetDefaultFOV();
    mOwner->SetupCameraFOV(fFOV);
    if (CameraManager::sInstance->m_iNumCam == 1)
    {
        fFOV = GetWidescreenFOVAdjustment(fFOV);
    }
    mOwner->SetupCameraTrackMatrix(mMatrix, fDeltaTime);

    float fBlend = mTimer * 2.0f;
    float fRotBlend = fBlend * fBlend;
    if (gDemoMode || mDemoMode)
    {
        if (!(fBlend < 0.6f))
        {
            fBlend = 0.6f;
        }
        fRotBlend = 0.1f;
    }

    Tracker* pTracker = mOwner->mTracker;
    if ((!pTracker || !pTracker->IsFirstPersonView()) && fBlend < 1.0f && fZoom == 1.0f)
    {
        PblVector3 vPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
        PblVector3 vDelta(vPos.x - vOldPos.x, vPos.y - vOldPos.y, vPos.z - vOldPos.z);
        if (vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z < 900.0f)
        {
            PblQuaternion qNew = PblQuaternion(mMatrix);

            PblQuaternion qRot = Interpolate(qOld, qNew, fRotBlend);
            PblQuaternion qUnit;
            PblMatrix mNew;
            D3DXMatrixRotationQuaternion(&mNew, D3DXQuaternionNormalize(&qUnit, &qRot));
            mNew.m[3][0] = vPos.x;
            mNew.m[3][1] = vPos.y;
            mNew.m[3][2] = vPos.z;
            mNew.m[3][3] = 1.0f;
            mMatrix = mNew;

            vPos.x = (vOldPos.x + vPos.x) * 0.5f;
            vPos.y = (vOldPos.y + vPos.y) * 0.5f;
            vPos.z = (vOldPos.z + vPos.z) * 0.5f;
            PblQuaternion qMid = Interpolate(qOld, qNew, 0.5f);
            PblMatrix mMid;
            D3DXMatrixRotationQuaternion(&mMid, D3DXQuaternionNormalize(&qUnit, &qMid));
            mMid.m[3][0] = vPos.x;
            mMid.m[3][1] = vPos.y;
            mMid.m[3][2] = vPos.z;
            mMid.m[3][3] = 1.0f;

            float fSwing = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z) * 0.5f
                           * (1.0f - fBlend);
            mMatrix.m[3][0] = mMid.m[3][0] + mMid.m[2][0] * fSwing;
            mMatrix.m[3][1] = mMid.m[3][1] + mMid.m[2][1] * fSwing;
            mMatrix.m[3][2] = mMid.m[3][2] + mMid.m[2][2] * fSwing;
            mMatrix.m[3][3] = 1.0f;

            mMatrix.m[3][0] = (mMatrix.m[3][0] - vOldPos.x) * fBlend + vOldPos.x;
            mMatrix.m[3][1] = (mMatrix.m[3][1] - vOldPos.y) * fBlend + vOldPos.y;
            mMatrix.m[3][2] = (mMatrix.m[3][2] - vOldPos.z) * fBlend + vOldPos.z;
            mMatrix.m[3][3] = 1.0f;

            if (mOwner->GetGameObject())
            {
                const PblMatrix& rOwner = GetCollisionObject(mOwner->GetGameObject())->GetMatrix();
                PblVector3 vStart(rOwner.m[3][0] + 0.0f, rOwner.m[3][1] + 1.5f, rOwner.m[3][2] + 0.0f);
                PblVector3 vToCamera(mMatrix.m[3][0] - vStart.x, mMatrix.m[3][1] - vStart.y,
                                     mMatrix.m[3][2] - vStart.z);
                float fDist = sqrtf(vToCamera.x * vToCamera.x + vToCamera.y * vToCamera.y
                                    + vToCamera.z * vToCamera.z);
                if (fDist > 0.0f)
                {
                    float fInv = 1.0f / fDist;
                    PblVector3 vDir(vToCamera.x * fInv, vToCamera.y * fInv, vToCamera.z * fInv);
                    const GameObject* pIgnore = mOwner->GetGameObject();
                    CollisionObject* pHitObject;
                    float fHit = CollisionManager::RayHit(vStart, vDir, fDist, pHitObject, 0, &pIgnore, 1, 0x90,
                                                          false) * fDist;
                    mMatrix.m[3][0] = vDir.x * fHit + vStart.x;
                    mMatrix.m[3][1] = vDir.y * fHit + vStart.y;
                    mMatrix.m[3][2] = vDir.z * fHit + vStart.z;
                    mMatrix.m[3][3] = 1.0f;
                }
            }
        }
        else
        {
            if (gDemoMode || mDemoMode)
            {
                ScreenTransition::sInstance->SetTransition(4, ScreenTransition::BLUR);
                mMatrix.m[3][0] -= mMatrix.m[2][0] * 2.0f;
                mMatrix.m[3][1] -= mMatrix.m[2][1] * 2.0f;
                mMatrix.m[3][2] -= mMatrix.m[2][2] * 2.0f;
                mMatrix.m[3][3] = 1.0f;
            }
            mTimer = 1.0f;
        }
    }

    if (mShake > 0.0f && GameLoop::GetMissionTime() > mShakeSuppressUntil)
    {
        PblMatrix mShakeRot = gMatrixIdentity;
        PblMatrix mAxis;
        D3DXMatrixMultiply(&mShakeRot, &mShakeRot, D3DXMatrixRotationX(&mAxis, BiasedRandom() * mShake * 0.1f));
        D3DXMatrixMultiply(&mShakeRot, &mShakeRot, D3DXMatrixRotationY(&mAxis, BiasedRandom() * mShake * 0.1f));
        D3DXMatrixMultiply(&mShakeRot, &mShakeRot, D3DXMatrixRotationZ(&mAxis, BiasedRandom() * mShake * 0.1f));
        mPreShakeMatrix = mMatrix;
        D3DXMatrixMultiply(&mAxis, &mShakeRot, &mMatrix);
        mMatrix = mAxis;
    }
    else if (mDoSwingFlag)
    {
        mDoSwingFlag = false;
    }

    pCamera->SetMatrix(mMatrix);
    float fLerp = expf(fDeltaTime * -25.0f);
    pCamera->SetZoom((pCamera->GetZoom() - fZoom) * fLerp + fZoom);
    pCamera->SetFOV((pCamera->GetFOV() - fFOV) * fLerp + fFOV, fAspectRatio);
}

// FUNCTION: SWBF2 0x004545b0
void ChaseCamera::SetupInterpolatedCamera(RedCamera* pCamera)
{
    if (mOwner)
    {
        PblMatrix mCamera = pCamera->_Matrix;
        mOwner->SetupInterpolatedCameraTrackMatrix(mCamera);
    }
}

// FUNCTION: SWBF2 0x00454600
void ChaseCamera::SetupIconVisibility(int iView)
{
    unsigned int uMask = 0;
    if (mOwner)
    {
        mOwner->SetupCameraIconVisibility(uMask);
    }
    mManager->SetVisibleIconMask(iView, uMask);
}

// FUNCTION: SWBF2 0x00454650
PblVector3 ChaseCamera::GetVelocity()
{
    if (mOwner && mOwner->GetGameObject())
    {
        GameObject* pObject = mOwner->GetGameObject();
    return pObject->GetVelocity();
    }
    return PblVector3(0.0f, 0.0f, 0.0f);
}
