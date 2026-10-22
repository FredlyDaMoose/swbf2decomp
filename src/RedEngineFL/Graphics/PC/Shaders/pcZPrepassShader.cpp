#include "pcZPrepassShader.h"

#include "PebbleFL/Common/PblChunk.h"

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include <string.h>

#include "RedEngineFL/Memory/engineAlloc.h"

#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "PebbleFL/Common/PblMatrix.h"

void __cdecl PblTraceP(const char* Format, ...);

class PblMatrix;
extern PblMatrix gMatrixIdentity;

unsigned int pcZPrepassShader::s_uiID;
pcRedShader::GetStateFuncT pcZPrepassShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcZPrepassShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcZPrepassShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcZPrepassShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x00702390
pcZPrepassShader::pcZPrepassShader()
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

// FUNCTION: SWBF2 0x00702460
pcZPrepassShader::pcZPrepassShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00702530
void pcZPrepassShader::Begin_Fixed(unsigned int uiState, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    if (uiState == 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    else
    {
        RedRenderer::pcSetTexture(0, 0);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

// FUNCTION: SWBF2 0x00702680
void pcZPrepassShader::Begin_Prog1(unsigned int uiState, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    if (uiState == 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    else
    {
        RedRenderer::pcSetTexture(0, 0);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }
}

// FUNCTION: SWBF2 0x00702720
void pcZPrepassShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00702740
void pcZPrepassShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00702750
void pcZPrepassShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00702760
unsigned int pcZPrepassShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00702780
RenderPhaseT pcZPrepassShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x00702790
unsigned int pcZPrepassShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x007027a0
pcRedShaderDef* pcZPrepassShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x007027b0
unsigned int pcZPrepassShader::GetState_Fixed(const RenderItem*) const
{
    return m_material.diffuseMapHash != 0;
}

// FUNCTION: SWBF2 0x007027c0
unsigned int pcZPrepassShader::GetState_Prog1(const RenderItem*) const
{
    return m_material.diffuseMapHash != 0;
}

// GLOBAL: SWBF2 0x00784828
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[2] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x007027d0
void pcZPrepassShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x28, 2, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x007027f0
void pcZPrepassShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    const unsigned int uiRVF = pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    RedRenderer::pcSetCullMode((pcRedCullT)(~(m_material.materialFlags >> 15) & 2));
    if (pItem->uiShaderState == 1)
    {
        if (uiRVF & 0x80)
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
        RedTexture* pTexture = RedTexture::Find(m_material.diffuseMapHash);
        const RedTextureTransform& transform = m_material.diffuseTexTransform;
        PblMatrix mTransform = gMatrixIdentity;
        mTransform.m[0][0] = transform(0, 0);
        mTransform.m[0][1] = transform(0, 1);
        mTransform.m[0][2] = 0.0f;
        mTransform.m[0][3] = 0.0f;
        mTransform.m[1][0] = transform(1, 0);
        mTransform.m[1][1] = transform(1, 1);
        mTransform.m[1][2] = 0.0f;
        mTransform.m[1][3] = 0.0f;
        mTransform.m[2][0] = transform(2, 0);
        mTransform.m[2][1] = transform(2, 1);
        mTransform.m[2][2] = 1.0f;
        mTransform.m[2][3] = 0.0f;
        RedRenderer::pcSetTexture(0, pTexture);
        RedRenderer::pcFFSetTextureTransform(0, mTransform);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, FALSE);
    }
    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x007029d0
void pcZPrepassShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    pItem->pRootShader->RenderZPrepass(pItem);
}

// FUNCTION: SWBF2 0x007029f0
pcRedShader* pcZPrepassShader::_Create(unsigned int uiFlags,
                                       const unsigned int* pParams,
                                       unsigned int uiNumParams,
                                       unsigned int uiMaterialFlags,
                                       const RedColor& color0,
                                       const RedColor& color1,
                                       float fSpecularPower) const
{
    return new pcZPrepassShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x00702a80
void pcZPrepassShader::_Init(unsigned int uiFlags,
                             const unsigned int* pParams,
                             unsigned int uiNumParams,
                             unsigned int uiMaterialFlags,
                             const RedColor& color0,
                             const RedColor& color1,
                             float fSpecularPower)
{
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
    m_material.diffuseTexTransform.m_bTransform = 0;
    m_material.detailTexTransform.m_bTransform = 0;
    m_material.animationData = 0;
    m_material.specularPower = fSpecularPower;

    unsigned int uiState = 2;
    if (uiFlags & 0x1c)
    {
        uiState = (uiFlags & 0x1c) | 2;
    }

    if (uiNumParams > 0 && pParams[0] != 0 && (uiMaterialFlags & 2))
    {
        uiState |= 0x200;
        m_material.diffuseMapHash = pParams[0];

        if (!(uiMaterialFlags & 0x200) && (uiFlags & 0x80))
        {
            uiState |= 0x80;
        }
    }

    pcRedShader::m_material = &m_material;
    m_pVertexInputFormat = pcRedVertexFormat::Create(uiFlags, uiState);
}

// FUNCTION: SWBF2 0x00702bf0
void pcZPrepassShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcZPrepassShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcZPrepassShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcZPrepassShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcZPrepassShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcZPrepassShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcZPrepassShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcZPrepassShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcZPrepassShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00702c50
void pcZPrepassShader::_Read(PblFileChunk& chunk) const
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

// SYNTHETIC: SWBF2 0x00409950
// `dynamic initializer for 'pcZPrepassShader::s_uiRenderTypeHash''
unsigned int pcZPrepassShader::s_uiRenderTypeHash = PblHash("zprepass");

// SYNTHETIC: SWBF2 0x00409910
// `dynamic initializer for 'pcZPrepassShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769c90
// `dynamic atexit destructor for 'pcZPrepassShader::s_Factory''
pcZPrepassShader pcZPrepassShader::s_Factory;

// SYNTHETIC: SWBF2 0x00409930
// `dynamic initializer for 'pcZPrepassShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769cb0
// `dynamic atexit destructor for 'pcZPrepassShader::s_shaderDef''
pcRedShaderDef pcZPrepassShader::s_shaderDef("zprepass_shader.xml");

// SYNTHETIC: SWBF2 0x00409990
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x00409970
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// GLOBAL: SWBF2 0x009c95dc
unsigned int g_pcZPrepassShader;
