#include "FreeCamera.h"

#include <math.h>
#include <d3dx9math.h>

#include "CameraManager.h"
#include "CollisionObject.h"
#include "FLInputManager.h"
#include "FLJoystick.h"
#include "GameLoop.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"

class GameObject;

class CollisionManager
{
public:
    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        CollisionObject*& pObject, PblVector3* pvPoint,
                        const GameObject** const ppIgnore, int iNumIgnore, int iMask, bool bFlag);
};

// GLOBAL: SWBF2 0x01e55f6a
bool FreeCamera::sQuakeControl;
// GLOBAL: SWBF2 0x01e55f6b
bool FreeCamera::sStop;
// GLOBAL: SWBF2 0x01e55fa0
bool FreeCamera::sInvertAxis;

const float FreeCamera::sMoveScale = 15.0f;
const float FreeCamera::sTurnScale = 1.5f;
const float FreeCamera::sHeightScale = 3.0f;

// GLOBAL: SWBF2 0x01e55fa2
static bool gIsFollowingObj;
// GLOBAL: SWBF2 0x01e55fa3
static bool gSetTargetObj;
// GLOBAL: SWBF2 0x01e55fa1
static bool gFollowingTethered;
// GLOBAL: SWBF2 0x01e51e27
static bool gSetTetherPosition;

// GLOBAL: SWBF2 0x01e55fa8
static RedOmniLight* s_pFreeCamLight;
// GLOBAL: SWBF2 0x01e55fa4
static int s_pFreeCamLightFreeze;

// GLOBAL: SWBF2 0x01faa478

static CollisionObject* sFollowedObject;
// GLOBAL: SWBF2 0x01faa47c

static bool sResetFollowPosition;
// GLOBAL: SWBF2 0x01faa47d

static bool sResetFollowRotation;

namespace AIDebug {

// FUNCTION: SWBF2 0x00438670
bool AllowUpdateFreeCamSpeeds()
{
    return true;
}
}

// FUNCTION: SWBF2 0x0052d650
FreeCamera::FreeCamera(CameraManager* pManager, int iView)
    : GameCamera(pManager, iView)
{
    mPitch = 0.0f;
    mYaw = 0.0f;
    mMultiplier = 1.0f;
    mTurnMultiplier = 1.0f;
    mButtonFasterDown = false;
    mButtonSlowerDown = false;
    mVelocity = gVectorZero;
    mMatrix.Reset();
}

// FUNCTION: SWBF2 0x0052d630
PblVector3 FreeCamera::GetVelocity()
{
    return mVelocity;
}

// SYNTHETIC: SWBF2 0x0052d6b0
// FreeCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052d6e0
void FreeCamera::SetMatrix(const PblMatrix& rMatrix)
{
    mMatrix = rMatrix;
    mPitch = asinf(-mMatrix.m[2][1]);
    mYaw = atan2f(mMatrix.m[2][0], mMatrix.m[2][2]);
}

// FUNCTION: SWBF2 0x0052d770
PblMatrix FreeCamera::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0052d7b0
void FreeCamera::Update(float fDeltaTime)
{
    static PblVector3 sTetherOffset;

    GUIInputs* pGuiInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mView);
    ProcessedInputs* pInputs = FLInputManager::s_instance.GetPlayerInputs(mView);
    if (!pGuiInputs)
    {
        return;
    }

    PblVector3 vOldPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);

    if (gSetTetherPosition)
    {
        gSetTetherPosition = false;
        if (sFollowedObject)
        {
            PblMatrix mat = GetMatrix();
            const PblVector3& vPos = sFollowedObject->GetPosition();
            gFollowingTethered = true;
            sTetherOffset = PblVector3(vPos.x - mat.m[3][0], vPos.y - mat.m[3][1], vPos.z - mat.m[3][2]);
        }
    }

    if (gSetTargetObj)
    {
        gSetTargetObj = false;
        PblMatrix mat = GetMatrix();
        PblVector3 vPos(mat.m[3][0], mat.m[3][1], mat.m[3][2]);
        PblVector3 vDir(-mat.m[2][0], -mat.m[2][1], -mat.m[2][2]);
        PblVector3 vHit(0.0f, 1.0f, 0.0f);
        CollisionObject* pObject;
        CollisionManager::RayHit(vPos, vDir, 500.0f, pObject, &vHit, NULL, 0, 0, false);
        if (pObject)
        {
            sFollowedObject = pObject;
            gIsFollowingObj = true;
        }
    }

    if (gIsFollowingObj && sFollowedObject)
    {
        if (gFollowingTethered)
        {
            const PblVector3& vPos = sFollowedObject->GetPosition();
            mMatrix.m[3][0] = vPos.x - sTetherOffset.x;
            mMatrix.m[3][1] = vPos.y - sTetherOffset.y;
            mMatrix.m[3][2] = vPos.z - sTetherOffset.z;
            mMatrix.m[3][3] = 1.0f;
        }

        PblMatrix mat = GetMatrix();
        float fHalfHeight = sFollowedObject->GetCollisionRadius() * 0.5f;
        const PblVector3& vTarget = sFollowedObject->GetPosition();
        D3DXVECTOR3 vAt(vTarget.x + 0.0f, vTarget.y + fHalfHeight, vTarget.z + 0.0f);
        D3DXVECTOR3 vDir(mat.m[3][0] - vAt.x, mat.m[3][1] - vAt.y, mat.m[3][2] - vAt.z);
        D3DXVECTOR3 vZ, vX, vY, vTmp;
        D3DXVec3Normalize(&vZ, &vDir);
        mat.m[2][0] = vZ.x; mat.m[2][1] = vZ.y; mat.m[2][2] = vZ.z; mat.m[2][3] = 0.0f;

        D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);
        D3DXVec3Cross(&vTmp, &vUp, &vZ);
        D3DXVec3Normalize(&vX, &vTmp);
        mat.m[0][0] = vX.x; mat.m[0][1] = vX.y; mat.m[0][2] = vX.z; mat.m[0][3] = 0.0f;

        D3DXVec3Cross(&vTmp, &vZ, &vX);
        D3DXVec3Normalize(&vY, &vTmp);
        mat.m[1][0] = vY.x; mat.m[1][1] = vY.y; mat.m[1][2] = vY.z; mat.m[1][3] = 0.0f;

        SetMatrix(mat);
        return;
    }

    if (!GameLoop::IsGameOver() && !sStop)
    {
        UpdateControlSpeed(*pGuiInputs);
        UpdateOrientation(*pInputs, fDeltaTime);
        UpdateTranslation(*pInputs, fDeltaTime);
    }

    if (fDeltaTime > 0.0f)
    {
        float fInvDeltaTime = 1.0f / fDeltaTime;
        mVelocity = PblVector3((mMatrix.m[3][0] - vOldPos.x) * fInvDeltaTime,
                               (mMatrix.m[3][1] - vOldPos.y) * fInvDeltaTime,
                               (mMatrix.m[3][2] - vOldPos.z) * fInvDeltaTime);
    }

    if (sResetFollowRotation || sResetFollowPosition)
    {
        sResetFollowPosition = false;
        sResetFollowRotation = false;
    }

    if (s_pFreeCamLight)
    {

        PblMatrix mat = GetMatrix();
        PblVector3 vPos(mat.m[3][0], mat.m[3][1], mat.m[3][2]);
        PblVector3 vDir(-mat.m[2][0], -mat.m[2][1], -mat.m[2][2]);
        CollisionObject* pObject;
        float fDist = CollisionManager::RayHit(vPos, vDir, 20.0f, pObject, NULL, NULL, 0, 0x90, false) * 20.0f - 0.75f;
        PblVector3 vLightPos(vPos.x - mat.m[2][0] * fDist,
                             vPos.y - mat.m[2][1] * fDist,
                             vPos.z - mat.m[2][2] * fDist);
        if (!s_pFreeCamLightFreeze)
        {
            s_pFreeCamLight->SetPosition(vLightPos);
        }
        RedDebug::DrawSphere(vLightPos, 0.1f, RedColor::YELLOW);
    }
}

// FUNCTION: SWBF2 0x0052dd80
void FreeCamera::UpdateControlSpeed(const GUIInputs& rInputs)
{
    if (!AIDebug::AllowUpdateFreeCamSpeeds())
    {
        return;
    }

    if (mButtonFasterDown && !rInputs.IsFreeEyeMoveFasterButtonDown())
    {
        mMultiplier *= 1.25f;
        mButtonFasterDown = false;
    }
    else if (rInputs.IsFreeEyeMoveFasterButtonDown())
    {
        mButtonFasterDown = true;
    }

    if (mButtonSlowerDown && !rInputs.IsFreeEyeMoveSlowerButtonDown())
    {
        mMultiplier *= 0.8f;
        mButtonSlowerDown = false;
    }
    else if (rInputs.IsFreeEyeMoveSlowerButtonDown())
    {
        mButtonSlowerDown = true;
    }

    if (mTurnButtonFasterDown && !rInputs.IsFreeEyeTurnFasterButtonDown())
    {
        mTurnMultiplier *= 1.25f;
        mTurnButtonFasterDown = false;
    }
    else if (rInputs.IsFreeEyeTurnFasterButtonDown())
    {
        mTurnButtonFasterDown = true;
    }

    if (mTurnButtonSlowerDown && !rInputs.IsFreeEyeTurnSlowerButtonDown())
    {
        mTurnMultiplier *= 0.8f;
        mTurnButtonSlowerDown = false;
    }
    else if (rInputs.IsFreeEyeTurnSlowerButtonDown())
    {
        mTurnButtonSlowerDown = true;
    }
}

// FUNCTION: SWBF2 0x0052dea0
void FreeCamera::UpdateOrientation(const ProcessedInputs& rInputs, float fDeltaTime)
{
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(mView);
    float fMouseY = pJoystick->GetMouseY();
    if (sInvertAxis)
    {
        fMouseY = -fMouseY;
    }

    float fTurn = mTurnMultiplier * fDeltaTime * sTurnScale;
    mYaw -= fTurn * pJoystick->GetMouseX();
    mPitch -= fTurn * fMouseY;

    D3DXMATRIX mRot, mRotY;
    D3DXMatrixRotationX(&mRot, mPitch);
    D3DXMatrixMultiply(&mRot, &mRot, D3DXMatrixRotationY(&mRotY, mYaw));

    for (int i = 0; i < 3; i++)
    {
        mMatrix.m[i][0] = mRot.m[i][0];
        mMatrix.m[i][1] = mRot.m[i][1];
        mMatrix.m[i][2] = mRot.m[i][2];
        mMatrix.m[i][3] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0052dfa0
void FreeCamera::UpdateTranslation(const ProcessedInputs& rInputs, float fDeltaTime)
{
    GUIInputs* pGuiInputs = FLInputManager::s_instance.GetPlayerGuiInputs(mView);
    int iHeight = (int)pGuiInputs->IsFreeEyeUpButtonDown() - (int)pGuiInputs->IsFreeEyeDownButtonDown();

    D3DXVECTOR3 vMove(rInputs.GetStrafeAxis() * sMoveScale,
                      (float)iHeight * sHeightScale,
                      -rInputs.GetMoveAxis() * sMoveScale);

    D3DXMATRIX mRotY;
    const D3DXMATRIX* pMatrix;
    if (sQuakeControl)
    {
        pMatrix = (const D3DXMATRIX*)&mMatrix;
    }
    else
    {
        pMatrix = D3DXMatrixRotationY(&mRotY, mYaw);
    }

    D3DXVECTOR3 vOut;
    D3DXVec3TransformNormal(&vOut, &vMove, pMatrix);
    vMove = vOut;

    float fScale = mMultiplier * fDeltaTime;
    mMatrix.m[3][0] += vMove.x * fScale;
    mMatrix.m[3][1] += vMove.y * fScale;
    mMatrix.m[3][2] += vMove.z * fScale;
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0052e0e0
void FreeCamera::SetupCamera(RedCamera* pCamera, float fDeltaTime)
{
    pCamera->SetMatrix(mMatrix);
    pCamera->SetZoom(1.0f);
}

// FUNCTION: SWBF2 0x0052e110
void FreeCamera::SetupIconVisibility(int iView)
{
    mManager->m_uiVisibleIconMask[iView] = 0;
}
