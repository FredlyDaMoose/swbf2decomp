#include "pcRefractionShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include <string.h>
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblVector.h"
#include <math.h>

void __cdecl PblTraceP(const char* Format, ...);

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixScaling(PblMatrix* pOut, float sx, float sy, float sz);

// GLOBAL: SWBF2 0x009ca4a8
static D3DMATERIAL9 s_d3dMat;

// GLOBAL: SWBF2 0x009ca4ec
bool s_bumpEnable;

// GLOBAL: SWBF2 0x009ca4ed
bool s_projectedBump;

// GLOBAL: SWBF2 0x009ca570
static unsigned int PIPELINE1_STATE_MAP[16];

// GLOBAL: SWBF2 0x009ca5b0
static unsigned int PIPELINE1_INV_STATE_MAP[16];

// GLOBAL: SWBF2 0x009ca4f0
// SYNTHETIC: SWBF2 0x0040a390
// `dynamic initializer for 's_bumpMat''

PblMatrix s_bumpMat;

// GLOBAL: SWBF2 0x009ca530
// SYNTHETIC: SWBF2 0x0040a3a0
// `dynamic initializer for 's_refractionTexMat''

PblMatrix s_refractionTexMat;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

static unsigned long FloatBits_(float f)
{
    return *(const unsigned long*)&f;
}

unsigned int pcRefractionShader::s_uiID;
pcRedShader::GetStateFuncT pcRefractionShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcRefractionShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcRefractionShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcRefractionShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x0070cae0
pcRefractionShader::pcRefractionShader()
{
    GetStateFuncArray[PIPELINE_FIXED] = 0;
    GetStateFuncArray[PIPELINE_PROG1] = 0;
    GetStateFuncArray[PIPELINE_PROG2] = 0;
    BeginFuncArray[PIPELINE_FIXED] = 0;
    BeginFuncArray[PIPELINE_PROG1] = 0;
    BeginFuncArray[PIPELINE_PROG2] = 0;
    EndFuncArray[PIPELINE_FIXED] = 0;
    EndFuncArray[PIPELINE_PROG1] = 0;
    EndFuncArray[PIPELINE_PROG2] = 0;
    RenderFuncArray[PIPELINE_FIXED] = 0;
    RenderFuncArray[PIPELINE_PROG1] = 0;
    RenderFuncArray[PIPELINE_PROG2] = 0;

    s_uiID = _Register(s_uiRenderTypeHash, this);

    _InitPipelineFuncArray();
}

// FUNCTION: SWBF2 0x0070cbb0
pcRefractionShader::pcRefractionShader(unsigned int uiFlags,
                                       const unsigned int* pParams,
                                       unsigned int uiNumParams,
                                       unsigned int uiMaterialFlags,
                                       const RedColor& color0,
                                       const RedColor& color1,
                                       float fSpecularPower)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
    _Init(uiFlags, pParams, uiNumParams, uiMaterialFlags,
          color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x0070cc90
void pcRefractionShader::Begin_Fixed(unsigned int uiState, unsigned int uiPass) const
{
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    switch (uiState)
    {
    case 0:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 1:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        if (uiPass == 0)
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,
                                                D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        break;

    case 2:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcFFSetTextureTransform(1, RedRenderer::pcGetNearSceneFadeTextureTransform());
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 3:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,
                                            D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;
    }

    if (s_bumpEnable)
    {
        const PblVector3& vZ = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
        const float fAngle = atan2f(vZ.z, vZ.x) - PblMath::fHalfPi;

        PblMatrix mTmp;
        PblMatrix mRot = _IdentityMatrix;
        D3DXMatrixMultiply(&mRot, &mRot, D3DXMatrixRotationZ(&mTmp, fAngle));
        PblMatrix mScale = _IdentityMatrix;
        D3DXMatrixMultiply(&mScale, &mScale,
                           D3DXMatrixScaling(&mTmp, m_distortionScale, m_distortionScale,
                                             m_distortionScale));
        D3DXMatrixMultiply(&mTmp, &mScale, &mRot);
        s_bumpMat = mTmp;

        const PblMatrix mBias(PblVector4{ 0.5f, 0.0f, 0.0f, 0.0f },
                              PblVector4{ 0.0f, -0.5f, 0.0f, 0.0f },
                              PblVector4{ 0.0f, 0.0f, 0.0f, 0.0f },
                              PblVector4{ 0.5f, 0.5f, 1.0f, 1.0f });
        PblMatrix mProj;
        RedRenderer::pcGetDevice()->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&mProj);
        D3DXMatrixMultiply(&mTmp, &mProj, &mBias);
        s_refractionTexMat = mTmp;
    }
}

// FUNCTION: SWBF2 0x0070d350
void pcRefractionShader::Begin_Prog1(unsigned int uiState, unsigned int) const
{
    const unsigned int uiFlags = PIPELINE1_INV_STATE_MAP[uiState];

    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);

    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    if (uiFlags & 8)
    {
        const PblVector3& vZ = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
        const float fAngle = atan2f(vZ.z, vZ.x) - PblMath::fHalfPi;

        PblMatrix mTmp;
        PblMatrix mRot = _IdentityMatrix;
        D3DXMatrixMultiply(&mRot, &mRot, D3DXMatrixRotationZ(&mTmp, fAngle));
        PblMatrix mScale = _IdentityMatrix;
        D3DXMatrixMultiply(&mScale, &mScale,
                           D3DXMatrixScaling(&mTmp, m_distortionScale, m_distortionScale,
                                             m_distortionScale));
        D3DXMatrixMultiply(&mTmp, &mScale, &mRot);
        s_bumpMat = mTmp;

        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_BUMPENVMAT00, FloatBits_(s_bumpMat.m[0][0]));
        RedRenderer::pcSetTextureStageState(1, D3DTSS_BUMPENVMAT10, FloatBits_(s_bumpMat.m[1][0]));
        RedRenderer::pcSetTextureStageState(1, D3DTSS_BUMPENVMAT01, FloatBits_(s_bumpMat.m[0][1]));
        RedRenderer::pcSetTextureStageState(1, D3DTSS_BUMPENVMAT11, FloatBits_(s_bumpMat.m[1][1]));
    }

    if (uiFlags & 2)
    {
        // GLOBAL: SWBF2 0x009ca5f0

        static const PblMatrix kClipToTextureMatrix(
            PblVector4{ 0.5f, 0.0f, 0.0f, 0.0f },
            PblVector4{ 0.0f, 0.5f, 0.0f, 0.0f },
            PblVector4{ 0.0f, 0.0f, 0.5f, 0.0f },
            PblVector4{ 0.5f, 0.5f, 0.5f, 1.0f });

        const RedCamera* pCamera = RedRenderer::GetCamera();
        PblMatrix mViewProj;
        D3DXMatrixMultiply(&mViewProj, &pCamera->GetMatrixInverse(),
                           &RedRenderer::GetProjectionMatrix());
        PblMatrix mTex;
        D3DXMatrixMultiply(&mTex, &mViewProj, &kClipToTextureMatrix);
        s_refractionTexMat = mTex;
    }
}

// FUNCTION: SWBF2 0x0070d750
void pcRefractionShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x0070d770
RedShadingState* pcRefractionShader::CreateShadingState() const
{
    RedShadingState* pState = new RedShadingState(GetRenderTypeHash());
    pState->AddColorParam(0xbc6f8a5e) = m_colorRefraction;
    pState->AddIntParam(0x7802a19a) = m_material.materialFlags & 0x44;
    pState->AddIntParam(0x891891e9) = m_material.diffuseMapHash;
    RedTextureTransform& transform = pState->AddTextureTransformParam(0x3f0d728a);
    transform.m_bTransform = m_material.diffuseTexTransform.m_bTransform;
    if (transform.m_bTransform)
    {
        for (int iRow = 0; iRow < 3; ++iRow)
        {
            transform.m_Transform[iRow][0] = m_material.diffuseTexTransform.m_Transform[iRow][0];
            transform.m_Transform[iRow][1] = m_material.diffuseTexTransform.m_Transform[iRow][1];
        }
    }
    return pState;
}

// FUNCTION: SWBF2 0x0070d880
void pcRefractionShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070d890
void pcRefractionShader::End_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetTexture(0, 0);
    RedRenderer::pcSetTexture(1, 0);
    RedRenderer::pcSetTexture(2, 0);
}

// FUNCTION: SWBF2 0x0070d8c0
unsigned int pcRefractionShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x0070d8e0
RenderPhaseT pcRefractionShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x0070d8f0
unsigned int pcRefractionShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x0070d900
unsigned short pcRefractionShader::GetShaderChain(pcRedShader** const ppOut,
                                                  unsigned short,
                                                  const RenderItem* pItem)
{
    bool bRefract = (m_material.materialFlags & 0x44) != 0;
    if (pItem->pShadingState)
    {
        bRefract = pItem->pShadingState->GetIntParam(0x7802a19a) != 0;
    }

    if (bRefract && !(pItem->flags & 0x10220000))
    {
        *ppOut = this;
        return 1;
    }
    *ppOut = RedRenderer::pcGetSystemShader(SYSTEM_SHADER_NORMAL);
    return 1;
}

// FUNCTION: SWBF2 0x0070d960
pcRedShaderDef* pcRefractionShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x0070d970
unsigned int pcRefractionShader::GetState_Fixed(const RenderItem*) const
{
    if (s_bumpEnable && RedRenderer::pcGetRenderOptions().distortion)
    {
        if (!m_material.diffuseMapHash)
        {
            return 3;
        }
        return RedRenderer::pcIsNearScene() ? 1 : 0;
    }
    return RedRenderer::pcIsNearScene() ? 2 : 0;
}

// FUNCTION: SWBF2 0x0070d9c0
unsigned int pcRefractionShader::GetState_Prog1(const RenderItem*) const
{
    if (!RedRenderer::pcIsNearScene())
    {
        return 0;
    }
    if (!RedRenderer::pcGetRenderOptions().distortion)
    {
        return 1;
    }

    unsigned int uiState = (s_projectedBump && s_bumpEnable && m_material.bumpMapHash) ? 11 : 3;
    if (m_material.diffuseMapHash)
    {
        uiState |= 4;
    }
    return PIPELINE1_STATE_MAP[uiState];
}

// FUNCTION: SWBF2 0x0070da20
unsigned int pcRefractionShader::GetZPrepassState() const
{
    return (m_material.materialFlags >> 1) & 1;
}

// GLOBAL: SWBF2 0x007852a8
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[4] =
{
    { 1, { 0x42, 0x00, 0x00, 0x00 } },
    { 2, { 0x21, 0x42, 0x00, 0x00 } },
    { 1, { 0x42, 0x00, 0x00, 0x00 } },
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x0070da30
void pcRefractionShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x18, 4, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x0070da50
bool pcRefractionShader::IsTransparent() const
{
    return true;
}

// FUNCTION: SWBF2 0x0070da60
void pcRefractionShader::RenderZPrepass(const RenderItem* pItem) const
{
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x8000) ? PCREDCULL_NONE : PCREDCULL_BACK);

    if (pMaterial->materialFlags & 2)
    {
        RedColor color;
        color.r = (unsigned char)(int)(pMaterial->diffuseColor.r * 255.0f);
        color.g = (unsigned char)(int)(pMaterial->diffuseColor.g * 255.0f);
        color.b = (unsigned char)(int)(pMaterial->diffuseColor.b * 255.0f);
        color.a = (unsigned char)(int)(pMaterial->diffuseColor.a * 255.0f);
        color.r = (unsigned char)(int)(color.r * (pItem->tweakColor.r * (1.0f / 255.0f)) + 0.5f);
        color.g = (unsigned char)(int)(color.g * (pItem->tweakColor.g * (1.0f / 255.0f)) + 0.5f);
        color.b = (unsigned char)(int)(color.b * (pItem->tweakColor.b * (1.0f / 255.0f)) + 0.5f);
        color.a = (unsigned char)(int)(color.a * (pItem->tweakColor.a * (1.0f / 255.0f)) + 0.5f);

        RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x8000) ? PCREDCULL_NONE : PCREDCULL_BACK);

        unsigned int uiDiffuseMap;
        const RedTextureTransform* pTransform = 0;
        if (pItem->pShadingState)
        {
            uiDiffuseMap = pItem->pShadingState->GetIntParam(0x891891e9);
            pTransform = &pItem->pShadingState->GetTextureTransformParam(0x3f0d728a);
            if (!pTransform->m_bTransform)
            {
                pTransform = 0;
            }
        }
        else
        {
            uiDiffuseMap = pMaterial->diffuseMapHash;
        }

        RedRenderer::pcSetTexture(0, uiDiffuseMap ? RedTexture::Find(uiDiffuseMap) : 0);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, color.a >> 1);

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 3);
        pRegs[39].component[0] = color.r * (1.0f / 255.0f);
        pRegs[39].component[1] = color.g * (1.0f / 255.0f);
        pRegs[39].component[2] = color.b * (1.0f / 255.0f);
        pRegs[39].component[3] = color.a * (1.0f / 255.0f);
        if (pTransform)
        {
            pRegs[40].component[0] = (*pTransform)(0, 0);
            pRegs[40].component[1] = (*pTransform)(1, 0);
            pRegs[40].component[2] = 0.0f;
            pRegs[40].component[3] = (*pTransform)(2, 0);
            pRegs[41].component[0] = (*pTransform)(0, 1);
            pRegs[41].component[1] = (*pTransform)(1, 1);
            pRegs[41].component[2] = 0.0f;
            pRegs[41].component[3] = (*pTransform)(2, 1);
        }
        else
        {
            pRegs[40].component[0] = 1.0f;
            pRegs[40].component[1] = 0.0f;
            pRegs[40].component[2] = 0.0f;
            pRegs[40].component[3] = 0.0f;
            pRegs[41].component[0] = 0.0f;
            pRegs[41].component[1] = 1.0f;
            pRegs[41].component[2] = 0.0f;
            pRegs[41].component[3] = 0.0f;
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    }

    RedRenderer::pcLoadFormat(m_pZPrepassVertexInputFormat);
    pItem->pPrim->Draw();
}

static void FFSetTextureTransform_(unsigned int uiStage, const RedTextureTransform& transform)
{
    const PblMatrix mTransform(PblVector4{ transform(0, 0), transform(0, 1), 0.0f, 0.0f },
                               PblVector4{ transform(1, 0), transform(1, 1), 0.0f, 0.0f },
                               PblVector4{ transform(2, 0), transform(2, 1), 0.0f, 0.0f },
                               PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });
    RedRenderer::pcFFSetTextureTransform(uiStage, mTransform);
}

// FUNCTION: SWBF2 0x0070ddc0
void pcRefractionShader::Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const
{
    RedColor color;
    color.r = (unsigned char)(int)(m_material.diffuseColor.r * 255.0f);
    color.g = (unsigned char)(int)(m_material.diffuseColor.g * 255.0f);
    color.b = (unsigned char)(int)(m_material.diffuseColor.b * 255.0f);
    color.a = (unsigned char)(int)(m_material.diffuseColor.a * 255.0f);
    color.r = (unsigned char)(int)(color.r * (pItem->tweakColor.r * (1.0f / 255.0f)) + 0.5f);
    color.g = (unsigned char)(int)(color.g * (pItem->tweakColor.g * (1.0f / 255.0f)) + 0.5f);
    color.b = (unsigned char)(int)(color.b * (pItem->tweakColor.b * (1.0f / 255.0f)) + 0.5f);
    color.a = (unsigned char)(int)(color.a * (pItem->tweakColor.a * (1.0f / 255.0f)) + 0.5f);

    s_d3dMat.Diffuse.r = color.r * (1.0f / 255.0f);
    s_d3dMat.Diffuse.g = color.g * (1.0f / 255.0f);
    s_d3dMat.Diffuse.b = color.b * (1.0f / 255.0f);
    s_d3dMat.Diffuse.a = color.a * (1.0f / 255.0f);
    s_d3dMat.Ambient = s_d3dMat.Diffuse;
    RedRenderer::pcGetDevice()->SetMaterial(&s_d3dMat);

    const unsigned int uiRVF = pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    if (uiRVF & 0x100)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
        RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    }
    else if (uiRVF & 0x80)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
        RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, FALSE);
    }

    RedRenderer::pcSetCullMode((m_material.materialFlags & 0x8000) ? PCREDCULL_NONE : PCREDCULL_BACK);

    switch (pItem->uiShaderState)
    {
    case 0:
    case 2:
        RedRenderer::pcSetTexture(0, RedTexture::Find(m_material.diffuseMapHash));
        FFSetTextureTransform_(0, m_material.diffuseTexTransform);
        break;

    case 1:
        if (uiPass != 0)
        {
            RedRenderer::pcSetTexture(0, RedTexture::Find(m_material.diffuseMapHash));
            FFSetTextureTransform_(0, m_material.diffuseTexTransform);
            break;
        }

    case 3:
        RedRenderer::pcSetTexture(0, RedTexture::Find(m_material.bumpMapHash));
        FFSetTextureTransform_(0, m_bumpMapTransform);
        if (m_refractionMap)
        {
            RedRenderer::pcSetTexture(1, m_refractionMap);
        }
        else
        {
            RedRenderer::pcSetTexture(1, RedRenderer::pcGetBuffer(PC_BUFFER_REFRACTION));
        }
        RedRenderer::pcFFSetTextureTransform(1, s_refractionTexMat);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_BUMPENVMAT00, FloatBits_(s_bumpMat.m[0][0]));
        RedRenderer::pcSetTextureStageState(0, D3DTSS_BUMPENVMAT10, FloatBits_(s_bumpMat.m[1][0]));
        RedRenderer::pcSetTextureStageState(0, D3DTSS_BUMPENVMAT01, FloatBits_(s_bumpMat.m[0][1]));
        RedRenderer::pcSetTextureStageState(0, D3DTSS_BUMPENVMAT11, FloatBits_(s_bumpMat.m[1][1]));
        break;
    }

    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070e310
void pcRefractionShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    const unsigned int uiFlags = PIPELINE1_INV_STATE_MAP[pItem->uiShaderState];

    RedColor color;
    color.r = (unsigned char)(int)(m_material.diffuseColor.r * 255.0f);
    color.g = (unsigned char)(int)(m_material.diffuseColor.g * 255.0f);
    color.b = (unsigned char)(int)(m_material.diffuseColor.b * 255.0f);
    color.a = (unsigned char)(int)(m_material.diffuseColor.a * 255.0f);
    color.r = (unsigned char)(int)(color.r * (pItem->tweakColor.r * (1.0f / 255.0f)) + 0.5f);
    color.g = (unsigned char)(int)(color.g * (pItem->tweakColor.g * (1.0f / 255.0f)) + 0.5f);
    color.b = (unsigned char)(int)(color.b * (pItem->tweakColor.b * (1.0f / 255.0f)) + 0.5f);
    color.a = (unsigned char)(int)(color.a * (pItem->tweakColor.a * (1.0f / 255.0f)) + 0.5f);

    const RedColor refraction = pItem->pShadingState
                                    ? pItem->pShadingState->GetColorParam(0xbc6f8a5e)
                                    : m_colorRefraction;
    const float fAlpha = refraction.a * (1.0f / 255.0f);

    RedRenderer::pcSetCullMode((m_material.materialFlags & 0x10000) ? PCREDCULL_NONE
                                                                     : PCREDCULL_BACK);

    if (!(uiFlags & 2))
    {
        const unsigned char ucAlpha = (unsigned char)(int)(color.a * fAlpha);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, ucAlpha < 0xff);

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 3);
        pRegs[39].component[0] = color.r * (1.0f / 255.0f);
        pRegs[39].component[1] = color.g * (1.0f / 255.0f);
        pRegs[39].component[2] = color.b * (1.0f / 255.0f);
        pRegs[39].component[3] = ucAlpha * (1.0f / 255.0f);
        const RedTextureTransform& diffuse = m_material.diffuseTexTransform;
        pRegs[40].component[0] = diffuse(0, 0);
        pRegs[40].component[1] = diffuse(1, 0);
        pRegs[40].component[2] = 0.0f;
        pRegs[40].component[3] = diffuse(2, 0);
        pRegs[41].component[0] = diffuse(0, 1);
        pRegs[41].component[1] = diffuse(1, 1);
        pRegs[41].component[2] = 0.0f;
        pRegs[41].component[3] = diffuse(2, 1);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetTexture(0, RedTexture::Find(m_material.diffuseMapHash));
    }
    else
    {
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 10);
        pRegs[39].component[0] = color.r * (1.0f / 255.0f);
        pRegs[39].component[1] = color.g * (1.0f / 255.0f);
        pRegs[39].component[2] = color.b * (1.0f / 255.0f);
        pRegs[39].component[3] = color.a * (1.0f / 255.0f);
        pRegs[40].component[0] = 1.0f;
        pRegs[40].component[1] = 0.0f;
        pRegs[40].component[2] = m_distortionScale * 8.0f;
        pRegs[40].component[3] = 0.0f;

        PblMatrix mTex;
        D3DXMatrixMultiply(&mTex, pItem->pmObjWorld, &s_refractionTexMat);
        pRegs[41].SetMatrix4x4(mTex);

        const RedTextureTransform& diffuse = m_material.diffuseTexTransform;
        pRegs[45].component[0] = diffuse(0, 0);
        pRegs[45].component[1] = diffuse(1, 0);
        pRegs[45].component[2] = 0.0f;
        pRegs[45].component[3] = diffuse(2, 0);
        pRegs[46].component[0] = diffuse(0, 1);
        pRegs[46].component[1] = diffuse(1, 1);
        pRegs[46].component[2] = 0.0f;
        pRegs[46].component[3] = diffuse(2, 1);
        const RedTextureTransform& bump = m_bumpMapTransform;
        pRegs[47].component[0] = bump(0, 0);
        pRegs[47].component[1] = bump(1, 0);
        pRegs[47].component[2] = 0.0f;
        pRegs[47].component[3] = bump(2, 0);
        pRegs[48].component[0] = bump(0, 1);
        pRegs[48].component[1] = bump(1, 1);
        pRegs[48].component[2] = 0.0f;
        pRegs[48].component[3] = bump(2, 1);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, color.a < 0xff);

        if (uiFlags & 8)
        {
            RedRenderer::pcSetTexture(0, RedTexture::Find(m_material.bumpMapHash));
        }
        else
        {
            RedRenderer::pcSetTexture(0, 0);
        }

        if (m_refractionMap)
        {
            RedRenderer::pcSetTexture(1, m_refractionMap);
        }
        else
        {
            RedRenderer::pcSetTexture(1, RedRenderer::pcGetBuffer(PC_BUFFER_REFRACTION));
        }

        if (uiFlags & 4)
        {
            RedRenderer::pcSetTexture(2, RedTexture::Find(m_material.diffuseMapHash));
        }
        else
        {
            RedRenderer::pcSetTexture(2, 0);
        }

        RedColorValue psConst[2];
        psConst[0].r = 1.0f;
        psConst[0].g = 1.0f;
        psConst[0].b = 1.0f;
        psConst[0].a = fAlpha;
        if (pItem->pLightingState->pProjectedLight)
        {
            const RedLight* pLight = pItem->pLightingState->pProjectedLight;
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, pLight->GetProjectionTextureWrapMode());
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, pLight->GetProjectionTextureWrapMode());
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSW, pLight->GetProjectionTextureWrapMode());
            RedRenderer::pcSetTexture(3, RedTexture::Find(pLight->m_uiProjTexHash));
            psConst[1] = pLight->m_color;
        }
        else
        {
            RedRenderer::pcSetTexture(3, 0);
            psConst[1] = RedColorValue::BLACK;
        }
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &psConst[0].r, 2);
    }

    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

#pragma warning(push)
#pragma warning(disable : 4716)

// FUNCTION: SWBF2 0x0070ea20
int pcRefractionShader::SetBumpMap(unsigned int uiHash, RedTextureTransform* pTransform)
{
    m_material.bumpMapHash = uiHash;
    m_bumpMapTransform.m_bTransform = pTransform->m_bTransform;
    if (m_bumpMapTransform.m_bTransform)
    {
        memcpy(m_bumpMapTransform.m_Transform, pTransform->m_Transform,
               sizeof(m_bumpMapTransform.m_Transform));
    }
}

// FUNCTION: SWBF2 0x0070ea50
int pcRefractionShader::SetDiffuseMap(unsigned int uiHash, RedTextureTransform* pTransform)
{
    m_material.diffuseMapHash = uiHash;
    m_material.detailTexTransform.m_bTransform = pTransform->m_bTransform;
    if (m_material.detailTexTransform.m_bTransform)
    {
        memcpy(m_material.detailTexTransform.m_Transform, pTransform->m_Transform,
               sizeof(m_material.detailTexTransform.m_Transform));
    }
}

// FUNCTION: SWBF2 0x0070ea90
int pcRefractionShader::SetDistortionScale(float fScale)
{
    m_distortionScale = fScale;
}

// FUNCTION: SWBF2 0x0070eab0
int pcRefractionShader::SetRefractionMap(RedTexture* pTexture)
{
    m_refractionMap = pTexture;
}

#pragma warning(pop)

// FUNCTION: SWBF2 0x0070eac0
bool pcRefractionShader::SortAsZWritingTransparent() const
{
    return false;
}

// FUNCTION: SWBF2 0x0070ead0
pcRedShader* pcRefractionShader::_Create(unsigned int uiFlags,
                                         const unsigned int* pParams,
                                         unsigned int uiNumParams,
                                         unsigned int uiMaterialFlags,
                                         const RedColor& color0,
                                         const RedColor& color1,
                                         float fSpecularPower) const
{
    return new pcRefractionShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                  color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x0070eb60
void pcRefractionShader::_Init(unsigned int uiFlags,
                               const unsigned int* pParams,
                               unsigned int uiNumParams,
                               unsigned int uiMaterialFlags,
                               const RedColor& color0,
                               const RedColor& color1,
                               float fSpecularPower)
{
    unsigned int uiState = (uiFlags & 0x20) ? 0x22 : 2;
    if (uiFlags & 0x1c)
    {
        uiState |= (uiFlags & 0x1c);
    }
    if (uiFlags & 0x200)
    {
        uiState |= 0x200;
    }

    memset(&m_material, 0, sizeof(m_material));

    m_material.diffuseColor.r = color0.r * (1.0f / 255.0f);
    m_material.diffuseColor.g = color0.g * (1.0f / 255.0f);
    m_material.diffuseColor.b = color0.b * (1.0f / 255.0f);
    m_material.diffuseColor.a = color0.a * (1.0f / 255.0f);

    m_material.specularColor.r = color1.r * (1.0f / 255.0f);
    m_material.specularColor.g = color1.g * (1.0f / 255.0f);
    m_material.specularColor.b = color1.b * (1.0f / 255.0f);
    m_material.specularColor.a = color1.a * (1.0f / 255.0f);

    m_material.materialFlags = uiMaterialFlags;
    m_material.specularPower = fSpecularPower;

    if (uiNumParams > 0)
    {
        m_material.diffuseMapHash = pParams[0];
    }
    if (uiNumParams > 1)
    {
        m_material.bumpMapHash = pParams[1];
    }
    if (uiNumParams > 2)
    {
        m_material.detailMapHash = pParams[2];
    }
    if (uiNumParams > 3)
    {
        m_material.envMapHash = pParams[3];
    }

    if (uiFlags & 0x100)
    {
        uiState |= 0x100;
    }
    else if (uiFlags & 0x80)
    {
        uiState |= 0x80;
    }

    if (uiFlags != 0)
    {
        m_pVertexInputFormat = pcRedVertexFormat::Create(uiFlags, uiState);
        if (m_material.materialFlags & 0x44)
        {
            m_pZPrepassVertexInputFormat = 0;
        }
        else if (m_material.materialFlags & 2)
        {
            m_pZPrepassVertexInputFormat = pcRedVertexFormat::Create(uiFlags, uiState & 0x39e);
        }
        else
        {
            m_pZPrepassVertexInputFormat = pcRedVertexFormat::Create(uiFlags, uiState & 0x1e);
        }
    }
    else
    {
        m_pVertexInputFormat = 0;
        m_pZPrepassVertexInputFormat = 0;
    }

    pcRedShader::m_material = &m_material;
    m_colorRefraction.b = 0xff;
    m_colorRefraction.g = 0xff;
    m_colorRefraction.r = 0xff;
    m_colorRefraction.a = 0;
    m_material.diffuseTexTransform.m_bTransform = 0;
    m_material.detailTexTransform.m_bTransform = 0;
    m_material.animationData = 0;
    m_bumpMapTransform.m_bTransform = 0;
    m_refractionMap = 0;
    m_distortionScale = 0.015f;
}

// FUNCTION: SWBF2 0x0070ed70
void pcRefractionShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcRefractionShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcRefractionShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcRefractionShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcRefractionShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcRefractionShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcRefractionShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcRefractionShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcRefractionShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x0070edd0
void pcRefractionShader::_Read(PblFileChunk& chunk) const
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x454d414e)
        {
            continue;
        }

        char acName[256];
        child.ReadString(acName, 256);
        const unsigned int uiHash = PblHash(acName);
        PblTraceP("pcRedShaderDef [%s]\tHash 0x%08x\n", acName, uiHash);

        pcRedShaderDef* pDef = GetShaderDef();
        if (pDef)
        {
            pDef->Read(chunk);
        }
        else
        {
            PblTraceP("Error: pcRedShaderDef 0x%08x not found\n", uiHash);
        }
    }
}

// FUNCTION: SWBF2 0x0070eeb0
void pcRefractionShader::_StaticInit() const
{
    memset(&s_d3dMat, 0, sizeof(s_d3dMat));

    memset(PIPELINE1_STATE_MAP, 0, sizeof(PIPELINE1_STATE_MAP));
    PIPELINE1_STATE_MAP[0] = 0;
    PIPELINE1_STATE_MAP[1] = 1;
    PIPELINE1_STATE_MAP[3] = 2;
    PIPELINE1_STATE_MAP[11] = 3;
    PIPELINE1_STATE_MAP[7] = 4;
    PIPELINE1_STATE_MAP[15] = 5;

    memset(PIPELINE1_INV_STATE_MAP, 0, sizeof(PIPELINE1_INV_STATE_MAP));
    PIPELINE1_INV_STATE_MAP[0] = 0;
    PIPELINE1_INV_STATE_MAP[1] = 1;
    PIPELINE1_INV_STATE_MAP[2] = 3;
    PIPELINE1_INV_STATE_MAP[3] = 11;
    PIPELINE1_INV_STATE_MAP[4] = 7;
    PIPELINE1_INV_STATE_MAP[5] = 15;

    s_projectedBump =
        (RedRenderer::pcGetDeviceCaps().TextureCaps & D3DPTEXTURECAPS_NOPROJECTEDBUMPENV) == 0;

    s_bumpEnable =
        RedRenderer::pcGetRenderCaps().pipeline != PIPELINE_FIXED ||
        ((RedRenderer::pcGetDeviceCaps().TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP) &&
         s_projectedBump);
}

// GLOBAL: SWBF2 0x009ca3ac
// SYNTHETIC: SWBF2 0x0040a310
// `dynamic initializer for 'pcRefractionShader::s_uiRenderTypeHash''
unsigned int pcRefractionShader::s_uiRenderTypeHash = PblHash("Refraction");

// GLOBAL: SWBF2 0x009ca3e0
// SYNTHETIC: SWBF2 0x0040a2d0
// `dynamic initializer for 'pcRefractionShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769f30
// `dynamic atexit destructor for 'pcRefractionShader::s_Factory''
pcRefractionShader pcRefractionShader::s_Factory;

// GLOBAL: SWBF2 0x009ca3b0
// SYNTHETIC: SWBF2 0x0040a2f0
// `dynamic initializer for 'pcRefractionShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769f50
// `dynamic atexit destructor for 'pcRefractionShader::s_shaderDef''
pcRedShaderDef pcRefractionShader::s_shaderDef("refration_shader.xml");

// GLOBAL: SWBF2 0x009ca3dc
// SYNTHETIC: SWBF2 0x0040a370
// `dynamic initializer for 'kuiLinearRamp''
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// SYNTHETIC: SWBF2 0x0040a330
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a350
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009ca370
unsigned int g_pcRefractionShader;
