#include "Battlefront2/Source/SkyDome.h"
#include "PebbleFL/Common/PblVector.h"

#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcSkyFogShader.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "Battlefront2/Source/ParticleEmitterObject.h"
#include "Battlefront2/Source/SkyStars.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x01eafd84
// SYNTHETIC: SWBF2 0x004020e0
// `dynamic initializer for 's_SkyFogPlane''
// SYNTHETIC: SWBF2 0x00766cf0
// `dynamic atexit destructor for 's_SkyFogPlane''
SkyFogRenderable s_SkyFogPlane;

// FUNCTION: SWBF2 0x004285e0
SkyFogRenderable::~SkyFogRenderable()
{
    if (PblRef* p = m_pPrimitive)
    {
        p->Release();
    }
    if (PblRef* p = m_pShader)
    {
        p->Release();
    }
}

// FUNCTION: SWBF2 0x00428690
void SkyFogRenderable::Init()
{
    m_pShader = pcRedShader::Create(PblHash("SkyFog"), 0x202, 0, 0, 0,
                                    RedColor::WHITE, RedColor::BLACK, 32.0f);
    static_cast<pcSkyFogShader*>(m_pShader)->SetTexture(
        RedRenderer::pcGetBuffer(PC_BUFFER_SKY));

    m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x202, 0x14, 4,
                                          0, true);
    m_pPrimitive->m_bbox._vMin = PblVector3(-1.0f, -1.0f, -1.0f);
    m_pPrimitive->m_bbox._vMax = PblVector3(1.0f, 1.0f, 1.0f);

    PblVector3 vMin = { -1.0f, -1.0f, -1.0f };
    PblVector3 vExtent = { 2.0f, 2.0f, 2.0f };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;

    float* v = (float*)m_pPrimitive->BeginVertexData();
    if (!v)
    {
        return;
    }

    v[0] = -1.0f;  v[1] = -1.0f;  v[2] = 1.0f;  v[3] = 0.0f;  v[4] = 1.0f;
    v[5] = 1.0f;   v[6] = -1.0f;  v[7] = 1.0f;  v[8] = 1.0f;  v[9] = 1.0f;
    v[10] = -1.0f; v[11] = 1.0f;  v[12] = 1.0f; v[13] = 0.0f; v[14] = 0.0f;
    v[15] = 1.0f;  v[16] = 1.0f;  v[17] = 1.0f; v[18] = 1.0f; v[19] = 0.0f;

    m_pPrimitive->EndVertexData();

    Activate(9999);
}

// FUNCTION: SWBF2 0x00428950
void SkyDome::Init()
{
    s_SkyFogPlane.Init();
}

extern PblMatrix gMatrixIdentity;

// FUNCTION: SWBF2 0x004288f0
void SkyFogRenderable::Render(int)
{
    RedRenderer::pcRenderPrimitive(m_pPrimitive, m_pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

extern "C" PblMatrix* __stdcall D3DXMatrixRotationAxis(PblMatrix* pOut, const PblVector3* pV, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// FUNCTION: SWBF2 0x00428970
void SkyDome::Render(int value)
{
    const unsigned int uiFlags = (unsigned int)value;

    bool bFogEnable = RedRenderer::GetFogEnable();
    RedRenderer::SetFogEnable(false);
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

    if (mNumDomeModels > 0)
    {
        PblMatrix mat = _IdentityMatrix;
        bool bNearClip = RedRenderer::pcGetUserNearClipPlaneEnable();
        RedRenderer::pcSetUserNearClipPlaneEnable(false);

        for (int i = 0; i < mNumDomeModels; ++i)
        {
            DomeModel& rDome = mDomeModels[i];
            if (!rDome.mModel)
            {
                continue;
            }

            mat = _IdentityMatrix;
            D3DXMatrixRotationAxis(&mat, &rDome.mRotationAxis,
                                   RedRenderer::GetTimeBase()->GetTime() * rDome.mRotationSpeed *
                                       PblMath::fTwoPi);

            const PblMatrix& camMatrix = pCamera->GetMatrix();
            PblVector3 pos = camMatrix.GetTranslation();
            if (rDome.mType == STATIC_HEIGHT)
            {
                float fHeight = rDome.mOffset + mDomeHeight;
                pos.y = fHeight - (fHeight - pos.y) * rDome.mMovementScale;
            }
            mat.m[3][0] = pos.x;
            mat.m[3][1] = pos.y;
            mat.m[3][2] = pos.z;
            mat.m[3][3] = 1.0f;

            for (int j = 0; j < rDome.mNumEffectData; ++j)
            {
                EffectData& rData = rDome.mEffectData[j];
                if (!rData.effect)
                {
                    continue;
                }

                PblMatrix boneMat;
                rDome.mModel->GetBoneMatrix(rData.hardPoint, boneMat);
                PblMatrix product;
                D3DXMatrixMultiply(&product, &boneMat, &mat);
                boneMat = product;
                rData.pos = boneMat.GetTranslation();

                const PblVector3& camPos = camMatrix.GetTranslation();
                const PblVector3& camZ = camMatrix.GetZAxis();
                if (camZ.y * (rData.pos.y - camPos.y) + (rData.pos.x - camPos.x) * camZ.x +
                        camZ.z * (rData.pos.z - camPos.z) < 0.0f)
                {
                    rData.effect->Update(0.0f);
                    rData.effect->Render(0, 1.0f, 0);
                }
            }

            if (i == 0)
            {
                RedColor color = m_colorSkyDome;
                rDome.mModel->Render(&mat, 0, color, 0x20211, 0);
                if (m_pStars)
                {
                    m_pStars->Render(uiFlags);
                }
                if (uiFlags & 0x400000)
                {
                    RedRenderer::pcSetUserNearClipPlaneEnable(bNearClip);
                    RedRenderer::Flush();
                    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
                    RedRenderer::SetFogEnable(bFogEnable);
                    return;
                }
            }
            else
            {
                rDome.mModel->Render(&mat, 0, RedColor::WHITE, 0x20211, 0);
            }
        }

        RedRenderer::pcSetUserNearClipPlaneEnable(bNearClip);
        RedParticleRenderer::RenderAll();
        RedRenderer::Flush();
    }
    else if (m_pStars)
    {
        bool bNearClip = RedRenderer::pcGetUserNearClipPlaneEnable();
        RedRenderer::pcSetUserNearClipPlaneEnable(false);
        m_pStars->Render(uiFlags);
        RedRenderer::pcSetUserNearClipPlaneEnable(bNearClip);
        RedRenderer::Flush();
    }

    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    RedRenderer::SetFogEnable(bFogEnable);

    float fFarSceneRange = FLRenderer::GetFarSceneRange();
    if (FLRenderer::GetNearSceneFadeStart() < fFarSceneRange)
    {
        RedTerrain::Render(uiFlags | 0x20000);
        RedRenderer::pcFlush(RENDERPHASE_OPAQUE);
        if (!(uiFlags & 0x200000))
        {
            RedWater::Render(uiFlags | 0x20000);
        }
    }
}

// FUNCTION: SWBF2 0x00428860
void SkyFogRenderable::Cleanup()
{
    Deactivate();

    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
        m_pPrimitive = 0;
    }
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00428960
void SkyDome::Cleanup()
{
    s_SkyFogPlane.Cleanup();
}
