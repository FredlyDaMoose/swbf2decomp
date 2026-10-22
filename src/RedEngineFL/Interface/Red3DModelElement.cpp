#include "Red3DModelElement.h"
#include "Red3DModelAnimator.h"

#include <math.h>

#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixScaling(PblMatrix* pOut, float sx, float sy, float sz);

void __cdecl PblTraceP(const char* Format, ...);

namespace
{

PblQuaternion AxisAngle(const PblVector3& vAxis, float fAngle)
{
    const float fLen2 = vAxis.x * vAxis.x + vAxis.y * vAxis.y + vAxis.z * vAxis.z;
    if ((float)fabs(fLen2 - 1.0) > 0.0001f)
    {
        PblTraceP("axis ( %f, %f, %f ) is not of unit length!\n", vAxis.x, vAxis.y, vAxis.z);
    }

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle * 0.5f);
    PblQuaternion q;
    q._fX = vAxis.x * fSin;
    q._fY = vAxis.y * fSin;
    q._fZ = vAxis.z * fSin;
    q._fS = fCos;
    return q;
}
}

// FUNCTION: SWBF2 0x006da8d0
Red3DModelElement::Red3DModelElement()
{
    m_rotation._fX = 0.0f;
    m_rotation._fY = 0.0f;
    m_rotation._fZ = 0.0f;
    m_rotation._fS = 1.0f;

    PblMatrix mIdentity;
    mIdentity.Reset();
    m_DrawMatrix = mIdentity;
    m_PosedMatrix = mIdentity;

    m_bAmAttached = false;
    m_bHavePoseMatrix = false;
    m_bLightingEnabled = false;
    m_uiModelHash = 0;
    m_Model = 0;
    m_scale = 1.0f;
    m_OmegaY = 0.0f;
    m_Yaw = 0.0f;
    m_uiHardpointHash = 0;
    m_AttachedObj = 0;
    m_translation = PblVector3(0.0f, 0.0f, 0.0f);
    m_rotation = AxisAngle(PblVector3(0.0f, 0.0f, 1.0f), 0.0f);
    m_bApplyRotationScale = true;
    mAnimator = 0;
}

// FUNCTION: SWBF2 0x006dab70
void Red3DModelElement::EnsureAnimatorExists()
{
    if (!mAnimator)
    {
        mAnimator = new Red3DModelAnimator;
    }
}

// FUNCTION: SWBF2 0x006dabe0
void Red3DModelElement::RenderUsingContext(const PblMatrix& mContext,
                                           const RedColor& color)
{
    if (!FindModel())
    {
        return;
    }

    unsigned int uiFlags = GetRenderFlags();

    if (m_bAmAttached)
    {
        if (m_bHavePoseMatrix)
        {
            PblMatrix mWorld;
            D3DXMatrixMultiply(&mWorld, &m_PosedMatrix, &m_DrawMatrix);
            m_Model->Render(&mWorld, 0, color, uiFlags, 0);
        }
        return;
    }

    RedPose pose;
    RedPose* pPose = 0;
    if (mAnimator)
    {
        pPose = mAnimator->UpdatePose();
    }

    PblMatrix mWorld = mContext;
    LocalToWorld(mWorld);

    if (m_bApplyRotationScale)
    {
        PblQuaternion qRotation;
        PblMatrix mLocal;
        PblMatrix mTemp;
        D3DXMatrixRotationQuaternion(&mLocal, D3DXQuaternionNormalize(&qRotation, &m_rotation));
        mLocal.m[3][0] = m_translation.x;
        mLocal.m[3][1] = m_translation.y;
        mLocal.m[3][2] = m_translation.z;
        mLocal.m[3][3] = 1.0f;
        if (m_OmegaY != 0.0f)
        {
            D3DXMatrixMultiply(&mLocal, &mLocal, D3DXMatrixRotationY(&mTemp, m_Yaw * 0.017453292f));
            m_Yaw += m_OmegaY;
        }
        D3DXMatrixMultiply(&mLocal, &mLocal, D3DXMatrixScaling(&mTemp, m_scale, m_scale, m_scale));
        D3DXMatrixMultiply(&mTemp, &mLocal, &mWorld);
        mWorld = mTemp;
    }

    if (!pPose)
    {
        pPose = &pose;
        pose._Table.Clear();
        pose._iNumEntries = 0;
        pose.BindToSkeleton(0);
    }

    m_Model->SetupPose(*pPose);
    m_Model->Render(&mWorld, pPose, color, uiFlags, 0);

    if (m_AttachedObj && m_uiHardpointHash)
    {
        const PblMatrix* pHardpoint = pPose->Find(m_uiHardpointHash);
        if (pHardpoint)
        {
            m_AttachedObj->SetPosedMatrix(*pHardpoint);
            m_AttachedObj->SetDrawMatrix(mWorld);
        }
    }
}

// FUNCTION: SWBF2 0x006daee0
void Red3DModelElement::SetDrawMatrix(const PblMatrix& mDraw)
{
    m_DrawMatrix = mDraw;
}

// FUNCTION: SWBF2 0x006daf20
void Red3DModelElement::SetPosedMatrix(const PblMatrix& mPosed)
{
    m_bHavePoseMatrix = true;
    m_PosedMatrix = mPosed;
}

// FUNCTION: SWBF2 0x006da6d0
Red3DModelElement::Red3DModelElement(const char* pcName)
    : Red3DModelElementLite(pcName)
{
    m_rotation._fX = 0.0f;
    m_rotation._fY = 0.0f;
    m_rotation._fZ = 0.0f;
    m_rotation._fS = 1.0f;

    PblMatrix mIdentity;
    mIdentity.Reset();
    m_DrawMatrix = mIdentity;
    m_PosedMatrix = mIdentity;

    m_bAmAttached = false;
    m_bHavePoseMatrix = false;
    m_bLightingEnabled = false;
    m_uiModelHash = 0;
    m_Model = 0;
    m_scale = 1.0f;
    m_OmegaY = 0.0f;
    m_Yaw = 0.0f;
    m_uiHardpointHash = 0;
    m_AttachedObj = 0;
    m_translation = PblVector3(0.0f, 0.0f, 0.0f);
    m_rotation = AxisAngle(PblVector3(0.0f, 0.0f, 1.0f), 0.0f);
    m_bApplyRotationScale = true;
    mAnimator = 0;
}

// FUNCTION: SWBF2 0x006daad0
Red3DModelElement::~Red3DModelElement()
{
    delete mAnimator;
}

// SYNTHETIC: SWBF2 0x006dab40
