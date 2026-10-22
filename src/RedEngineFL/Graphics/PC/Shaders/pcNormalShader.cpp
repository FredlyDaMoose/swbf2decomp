#include "pcNormalShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include <string.h>

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"

#include "PebbleFL/Common/PblTimeBase.h"

#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

#include "RedEngineFL/Memory/engineAlloc.h"

#include <math.h>
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"

#include "RedEngineFL/Graphics/RedSegment.h"
#include "RedEngineFL/Graphics/RedCamera.h"

void __cdecl PblTraceP(const char* Format, ...);

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

unsigned int pcNormalShader::s_uiID;
pcRedShader::GetStateFuncT pcNormalShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcNormalShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcNormalShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcNormalShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9378
PblTimeBase::Stamp s_animationTimeStamp;

// GLOBAL: SWBF2 0x009c9390
unsigned int PIPELINE1_STATE_MAP[32];

// GLOBAL: SWBF2 0x009c9410
unsigned int PIPELINE1_INV_STATE_MAP[32];

// GLOBAL: SWBF2 0x009c9490

RedTexture* s_pNullDetailMap;

// GLOBAL: SWBF2 0x009c9498
D3DMATERIAL9 s_d3dMat;

// FUNCTION: SWBF2 0x006fe7e0
pcNormalShader::pcNormalShader()
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

// FUNCTION: SWBF2 0x006fe8b0
pcNormalShader::pcNormalShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x006ff7c0
pcRedShaderDef* pcNormalShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// GLOBAL: SWBF2 0x00784668
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[5] =
{
    { 1, { 0x42, 0x00, 0x00, 0x00 } },
    { 1, { 0x42, 0x00, 0x00, 0x00 } },
    { 2, { 0x42, 0x21, 0x00, 0x00 } },
    { 1, { 0x42, 0x00, 0x00, 0x00 } },
    { 2, { 0x42, 0x21, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x006ff990
void pcNormalShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x28, 5, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x007011d0
pcRedShader* pcNormalShader::_Create(unsigned int uiFlags,
                                     const unsigned int* pParams,
                                     unsigned int uiNumParams,
                                     unsigned int uiMaterialFlags,
                                     const RedColor& color0,
                                     const RedColor& color1,
                                     float fSpecularPower) const
{
    return new pcNormalShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                              color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x00701260
void pcNormalShader::_Init(unsigned int uiFlags,
                           const unsigned int* pParams,
                           unsigned int uiNumParams,
                           unsigned int uiMaterialFlags,
                           const RedColor& color0,
                           const RedColor& color1,
                           float fSpecularPower)
{
    s_pNullDetailMap = RedTexture::s_textureHashTable.Find(PblHash("null_detailmap"));

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
        if (m_material.materialFlags & 2)
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

    m_material.diffuseTexTransform.m_bTransform = 0;
    m_material.detailTexTransform.m_bTransform = 0;
    m_material.animationData = 0;
    pcRedShader::m_material = &m_material;
    m_lightingInSeparatePass = false;
}

// FUNCTION: SWBF2 0x00701460
void pcNormalShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcNormalShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcNormalShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcNormalShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcNormalShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcNormalShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcNormalShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcNormalShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcNormalShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x007014c0
void pcNormalShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x007015a0
void pcNormalShader::_StaticInit() const
{
    memset(&s_d3dMat, 0, sizeof(s_d3dMat));

    s_animationTimeStamp.Mark(RedRenderer::GetTimeBase());

    memset(PIPELINE1_STATE_MAP, 0, sizeof(PIPELINE1_STATE_MAP));
    PIPELINE1_STATE_MAP[0] = 4;
    PIPELINE1_STATE_MAP[1] = 5;
    PIPELINE1_STATE_MAP[2] = 6;
    PIPELINE1_STATE_MAP[3] = 7;
    PIPELINE1_STATE_MAP[4] = 8;
    PIPELINE1_STATE_MAP[5] = 9;
    PIPELINE1_STATE_MAP[6] = 10;
    PIPELINE1_STATE_MAP[7] = 11;
    PIPELINE1_STATE_MAP[8] = 12;
    PIPELINE1_STATE_MAP[9] = 13;
    PIPELINE1_STATE_MAP[12] = 14;
    PIPELINE1_STATE_MAP[13] = 15;
    PIPELINE1_STATE_MAP[10] = 16;
    PIPELINE1_STATE_MAP[11] = 17;
    PIPELINE1_STATE_MAP[14] = 18;
    PIPELINE1_STATE_MAP[15] = 19;
    PIPELINE1_STATE_MAP[16] = 0;
    PIPELINE1_STATE_MAP[17] = 1;
    PIPELINE1_STATE_MAP[18] = 2;
    PIPELINE1_STATE_MAP[19] = 3;

    memset(PIPELINE1_INV_STATE_MAP, 0, sizeof(PIPELINE1_INV_STATE_MAP));
    PIPELINE1_INV_STATE_MAP[4] = 0;
    PIPELINE1_INV_STATE_MAP[5] = 1;
    PIPELINE1_INV_STATE_MAP[6] = 2;
    PIPELINE1_INV_STATE_MAP[7] = 3;
    PIPELINE1_INV_STATE_MAP[8] = 4;
    PIPELINE1_INV_STATE_MAP[9] = 5;
    PIPELINE1_INV_STATE_MAP[10] = 6;
    PIPELINE1_INV_STATE_MAP[11] = 7;
    PIPELINE1_INV_STATE_MAP[12] = 8;
    PIPELINE1_INV_STATE_MAP[13] = 9;
    PIPELINE1_INV_STATE_MAP[14] = 12;
    PIPELINE1_INV_STATE_MAP[15] = 13;
    PIPELINE1_INV_STATE_MAP[16] = 10;
    PIPELINE1_INV_STATE_MAP[17] = 11;
    PIPELINE1_INV_STATE_MAP[18] = 14;
    PIPELINE1_INV_STATE_MAP[19] = 15;
    PIPELINE1_INV_STATE_MAP[0] = 16;
    PIPELINE1_INV_STATE_MAP[1] = 17;
    PIPELINE1_INV_STATE_MAP[2] = 18;
    PIPELINE1_INV_STATE_MAP[3] = 19;
}

static const float kInv255 = 1.0f / 255.0f;

// GLOBAL: SWBF2 0x009c9388

RedColor s_colorDiffusePrev;

// GLOBAL: SWBF2 0x009c9494

bool s_bTextureTransformPrev;

// GLOBAL: SWBF2 0x009c9495

bool s_bDetailTextureTransformPrev;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

void* RedShadingState::operator new(unsigned int uiSize)
{
    return sMemoryPool.Allocate(uiSize);
}

// FUNCTION: SWBF2 0x006fe950
void RedShadingState::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

RedShadingState::RedShadingState(unsigned int uiRenderTypeHash)
{
    m_uiRenderTypeHash = uiRenderTypeHash;
    m_uiNumValues = 0;
    m_ParamHash.Clear();
    m_ParamHashNumEntries = 0;
}

// SYNTHETIC: SWBF2 0x006fe9b0
// RedShadingState::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006ff540
void pcNormalShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x006ff560
RedShadingState* pcNormalShader::CreateShadingState() const
{
    RedShadingState* pState = new RedShadingState(GetRenderTypeHash());
    pState->AddIntParam(0x891891e9) = m_material.diffuseMapHash;
    pState->AddTextureTransformParam(0x3f0d728a).m_bTransform = 0;
    return pState;
}

// FUNCTION: SWBF2 0x006ff630
void pcNormalShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006ff640
void pcNormalShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006ff650
unsigned int pcNormalShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x006ff670
RenderPhaseT pcNormalShader::GetRenderPhase(const RenderItem* pItem, unsigned int) const
{
    if ((pItem->pRootShader->m_material->materialFlags & 0x44) || pItem->tweakColor.a < 0xff)
    {
        return RENDERPHASE_TRANSPARENT;
    }
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x006ff6a0
unsigned int pcNormalShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x006ff6b0
unsigned short pcNormalShader::GetShaderChain(pcRedShader** const ppOut, unsigned short,
                                              const RenderItem* pItem)
{
    unsigned short uiCount = 0;
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    m_lightingInSeparatePass = false;
    if (pMaterial->animationData)
    {
        UpdateAnimation();
    }

    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED || !RedRenderer::pcIsNearScene() ||
        (pItem->flags & 0x10220010))
    {
        ppOut[0] = this;
        return 1;
    }

    if (!(pMaterial->materialFlags & 0x44) && pItem->tweakColor.a >= 0xff &&
        !(pMaterial->diffuseColor.a < 1.0f) && !(pItem->flags & 0x20000000) &&
        (pMaterial->materialFlags & 0x20) && RedRenderer::pcGetRenderOptions().bumpMapping)
    {
        ppOut[0] = RedRenderer::pcGetSystemShader(SYSTEM_SHADER_PERPIXELDIFFUSELIGHTING);
        uiCount = 1;
        m_lightingInSeparatePass = true;
    }
    ppOut[uiCount++] = this;

    if (RedRenderer::pcGetRenderOptions().specularQuality)
    {
        if (((pMaterial->materialFlags & 0x80) && pItem->pLightingState->numSpecularLights) ||
            (pMaterial->materialFlags & 0x100))
        {
            ppOut[uiCount++] = RedRenderer::pcGetSystemShader(SYSTEM_SHADER_SPECULARLIGHTING);
        }
    }
    return uiCount;
}

// FUNCTION: SWBF2 0x006ff7d0
unsigned int pcNormalShader::GetState_Fixed(const RenderItem* pItem) const
{
    if (!RedRenderer::pcIsNearScene())
    {
        return 0;
    }

    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    const bool bShadowed = RedShadowMap::IsShadowMapEnabled() && !(pItem->flags & 0x18000018);
    const bool bTransparent = (pMaterial->materialFlags & 0x44) || pItem->tweakColor.a < 0xff ||
                              pMaterial->diffuseColor.a < 1.0f || (pItem->flags & 0x20000000);
    if (bShadowed)
    {
        return bTransparent ? 3 : 4;
    }
    if (bTransparent)
    {
        return 3;
    }
    return (pMaterial->materialFlags & 2) ? 2 : 1;
}

// FUNCTION: SWBF2 0x006ff870
unsigned int pcNormalShader::GetState_Prog1(const RenderItem* pItem) const
{
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    const bool bTransparent = (pMaterial->materialFlags & 0x44) || pItem->tweakColor.a < 0xff ||
                              pMaterial->diffuseColor.a < 1.0f || (pItem->flags & 0x20000000);
    const bool bShadowed = (!bTransparent || (pItem->flags & 0x8000000)) &&
                           RedShadowMap::IsShadowMapEnabled() && !(pItem->flags & 0x10020018);
    const bool bSeparate = m_lightingInSeparatePass && !(pItem->flags & 0x10000000);
    const bool bProjected = pItem->pLightingState->pProjectedLight && !bSeparate &&
                            !(pItem->flags & 0x18020010);

    unsigned int uiState = bTransparent ? 2 : 0;
    if (pMaterial->materialFlags & 2)
    {
        uiState |= 1;
    }
    if (bProjected)
    {
        uiState |= 4;
    }
    if (bShadowed)
    {
        uiState |= 8;
    }
    if (pItem->flags & 0x10)
    {
        uiState |= 0x10;
    }
    return PIPELINE1_STATE_MAP[uiState];
}

// FUNCTION: SWBF2 0x006ff950
int pcNormalShader::GetDiffuseMapHash(int iItem, int)
{
    const RenderItem* pItem = (const RenderItem*)iItem;
    if (!pItem)
    {
        return 0;
    }
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    if (pItem->pShadingState)
    {
        return pItem->pShadingState->GetIntParam(0x891891e9);
    }
    return pMaterial->diffuseMapHash;
}

// FUNCTION: SWBF2 0x006ff980
unsigned int pcNormalShader::GetZPrepassState() const
{
    return (m_material.materialFlags >> 1) & 1;
}

// FUNCTION: SWBF2 0x006ff9b0
bool pcNormalShader::IsTransparent() const
{
    return (m_material.materialFlags & 0x44) != 0;
}

// FUNCTION: SWBF2 0x00700e20
bool pcNormalShader::SortAsZWritingTransparent() const
{
    return (m_material.materialFlags & 0x46) == 0;
}

// FUNCTION: SWBF2 0x006ff9c0
void pcNormalShader::RenderZPrepass(const RenderItem* pItem) const
{
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x10000) ? PCREDCULL_NONE : PCREDCULL_BACK);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) ? FALSE : TRUE);

    if (GetZPrepassState() == 1)
    {
        RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x10000) ? PCREDCULL_NONE : PCREDCULL_BACK);

        const unsigned int uiDiffuseMap = pItem->pShadingState
                                              ? pItem->pShadingState->GetIntParam(0x891891e9)
                                              : pMaterial->diffuseMapHash;
        const RedTexture* pTexture = 0;
        if (uiDiffuseMap)
        {
            pTexture = RedTexture::Find(uiDiffuseMap);
            if (pTexture)
            {
                RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            }
        }
        RedRenderer::pcSetTexture(0, pTexture);

        const RedTextureTransform* pTransform = &pMaterial->diffuseTexTransform;
        if (pItem->pShadingState)
        {
            pTransform = &pItem->pShadingState->GetTextureTransformParam(0x3f0d728a);
        }
        if (!pTransform->m_bTransform)
        {
            pTransform = 0;
        }

        RedRenderer::pcSetTexture(0, pTexture);
        if (pTexture)
        {
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);

        RedColor color;
        color.r = (unsigned char)(int)(pMaterial->diffuseColor.r * 255.0f);
        color.g = (unsigned char)(int)(pMaterial->diffuseColor.g * 255.0f);
        color.b = (unsigned char)(int)(pMaterial->diffuseColor.b * 255.0f);
        color.a = (unsigned char)(int)(pMaterial->diffuseColor.a * 255.0f);
        color = color * pItem->tweakColor;

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 3);
        pRegs[39].component[0] = color.r * kInv255;
        pRegs[39].component[1] = color.g * kInv255;
        pRegs[39].component[2] = color.b * kInv255;
        pRegs[39].component[3] = color.a * kInv255;
        if (pTransform)
        {
            pRegs[40].component[0] = (*pTransform)(0, 0);
            pRegs[40].component[1] = (*pTransform)(1, 0);
            pRegs[40].component[2] = 0.0f;
            pRegs[40].component[3] = (*pTransform)(2, 0);
            pRegs[41].component[0] = (*pTransform)(0, 1);
            pRegs[41].component[1] = (*pTransform)(1, 1);
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
            pRegs[41].component[3] = 0.0f;
        }
        pRegs[41].component[2] = 0.0f;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    }

    RedRenderer::pcLoadFormat(m_pZPrepassVertexInputFormat ? m_pZPrepassVertexInputFormat
                                                           : pItem->pPrim->m_vertexFormat);
    pItem->pPrim->Draw();
}

// GLOBAL: SWBF2 0x009c92a4
// SYNTHETIC: SWBF2 0x00409850
// `dynamic initializer for 'kuiLinearRamp''
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// FUNCTION: SWBF2 0x006fea10
void pcNormalShader::Begin_Fixed(unsigned int uiState, unsigned int uiPass) const
{
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    switch (uiState)
    {
    case 0:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;

    case 1:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTexture(1, RedTexture::Find(kuiLinearRamp));
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcFFSetTextureTransform(1, RedRenderer::pcGetNearSceneFadeTextureTransform());
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;

    case 2:
        if (uiPass != 0)
        {
            break;
        }
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;

    case 3:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTexture(1, RedTexture::Find(kuiLinearRamp));
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcFFSetTextureTransform(1, RedRenderer::pcGetNearSceneFadeTextureTransform());
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;

    case 4:
    {
        if (uiPass != 0)
        {
            break;
        }
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTexture(1, RedShadowMap::GetShadowMapTexture());
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, RedShadowMap::GetShadowMapWrapMode());
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, RedShadowMap::GetShadowMapWrapMode());
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,
                                            D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);

        PblMatrix mCamera = RedRenderer::GetCamera()->GetMatrix();
        mCamera.m[2][0] = -mCamera.m[2][0];
        mCamera.m[2][1] = -mCamera.m[2][1];
        mCamera.m[2][2] = -mCamera.m[2][2];
        mCamera.m[2][3] = 0.0f;
        const PblVector4& vU = RedShadowMap::_vShadowMapTransformU;
        const PblVector4& vV = RedShadowMap::_vShadowMapTransformV;
        const PblVector4& vW = RedShadowMap::_vShadowMapTransformW;
        PblMatrix mShadow;
        mShadow.m[0][0] = vU.x; mShadow.m[0][1] = vV.x; mShadow.m[0][2] = vW.x; mShadow.m[0][3] = 0.0f;
        mShadow.m[1][0] = vU.y; mShadow.m[1][1] = vV.y; mShadow.m[1][2] = vW.y; mShadow.m[1][3] = 0.0f;
        mShadow.m[2][0] = vU.z; mShadow.m[2][1] = vV.z; mShadow.m[2][2] = vW.z; mShadow.m[2][3] = 0.0f;
        mShadow.m[3][0] = vU.w; mShadow.m[3][1] = vV.w; mShadow.m[3][2] = vW.w; mShadow.m[3][3] = 1.0f;
        PblMatrix mTexture;
        D3DXMatrixMultiply(&mTexture, &mCamera, &mShadow);
        RedRenderer::pcFFSetTextureTransform(1, mTexture);

        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;
    }

    default:
        return;
    }

    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 8);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetTexture(0, RedTexture::Find(kuiLinearRamp));
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    RedRenderer::pcFFSetTextureTransform(0, RedRenderer::pcGetNearSceneFadeTextureTransform());
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x006ff280
void pcNormalShader::Begin_Prog1(unsigned int uiState, unsigned int) const
{
    const unsigned int uiFlags = PIPELINE1_INV_STATE_MAP[uiState];
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  (RedRenderer::pcIsNearScene() && !(uiFlags & 2)) ? 0xf : 7);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    if (uiFlags & 4)
    {
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    if (uiFlags & 8)
    {
        RedRenderer::pcSetTexture(3, RedShadowMap::GetShadowMapTexture());
        RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, RedShadowMap::GetShadowMapWrapMode());
        RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, RedShadowMap::GetShadowMapWrapMode());
        RedRenderer::pcSetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 6);
    pRegs[39].component[0] = RedColor::WHITE.r * kInv255;
    pRegs[39].component[1] = RedColor::WHITE.g * kInv255;
    pRegs[39].component[2] = RedColor::WHITE.b * kInv255;
    pRegs[39].component[3] = RedColor::WHITE.a * kInv255;
    pRegs[40].component[0] = 1.0f;
    pRegs[40].component[1] = 0.0f;
    pRegs[40].component[2] = 0.0f;
    pRegs[40].component[3] = 0.0f;
    pRegs[41].component[0] = 1.0f;
    pRegs[41].component[1] = 0.0f;
    pRegs[41].component[2] = 0.0f;
    pRegs[41].component[3] = 0.0f;
    pRegs[42].component[0] = 0.0f;
    pRegs[42].component[1] = 1.0f;
    pRegs[42].component[2] = 0.0f;
    pRegs[42].component[3] = 0.0f;
    pRegs[43].component[0] = 1.0f;
    pRegs[43].component[1] = 0.0f;
    pRegs[43].component[2] = 0.0f;
    pRegs[43].component[3] = 0.0f;
    pRegs[44].component[0] = 0.0f;
    pRegs[44].component[1] = 1.0f;
    pRegs[44].component[2] = 0.0f;
    pRegs[44].component[3] = 0.0f;
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    s_colorDiffusePrev = RedColor::WHITE;
    s_bTextureTransformPrev = false;
    s_bDetailTextureTransformPrev = false;
}

// FUNCTION: SWBF2 0x006ffd60
void pcNormalShader::Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const
{
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;

    RedColor color;
    color.r = (unsigned char)(int)(pMaterial->diffuseColor.r * 255.0f);
    color.g = (unsigned char)(int)(pMaterial->diffuseColor.g * 255.0f);
    color.b = (unsigned char)(int)(pMaterial->diffuseColor.b * 255.0f);
    color.a = (unsigned char)(int)(pMaterial->diffuseColor.a * 255.0f);
    color = color * pItem->tweakColor;

    s_d3dMat.Diffuse.r = color.r * kInv255;
    s_d3dMat.Diffuse.g = color.g * kInv255;
    s_d3dMat.Diffuse.b = color.b * kInv255;
    s_d3dMat.Diffuse.a = color.a * kInv255;
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
        const D3DCOLOR ambient = D3DCOLOR_ARGB(color.a, color.r, color.g, color.b);
        if (ambient != D3DCOLOR_ARGB(RedColor::WHITE.a, RedColor::WHITE.r, RedColor::WHITE.g,
                                     RedColor::WHITE.b) &&
            !(pItem->uiEnvState & 0x1f))
        {
            RedRenderer::pcSetRenderState(D3DRS_AMBIENT, ambient);
        }
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, FALSE);
    }

    RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x10000) ? PCREDCULL_NONE : PCREDCULL_BACK);

    bool bTexture = true;
    switch (pItem->uiShaderState)
    {
    case 0:
    {
        const unsigned int uiMatFlags = pMaterial->materialFlags;
        const bool bBlend = (uiMatFlags & 0x44) || color.a < 0xff || (pItem->flags & 0x20000000);
        const bool bAlphaTest = ((uiMatFlags >> 1) & 1) != 0;
        const bool bTransparent = (uiMatFlags & 0x46) != 0;
        if (bBlend)
        {
            RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, !bTransparent && !(pItem->flags & 0x20));
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            if (pMaterial->materialFlags & 0x40)
            {
                RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
                RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
            }
            else
            {
                RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
            }
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) ? FALSE : TRUE);
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        if (bTransparent)
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, bAlphaTest ? color.a >> 1 : 1);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        }
        break;
    }

    case 2:
        if (uiPass)
        {
            bTexture = false;
            break;
        }

    case 1:
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) ? FALSE : TRUE);
        break;

    case 3:
    {
        const bool bAlphaTest = ((pMaterial->materialFlags >> 1) & 1) != 0;
        const bool bTransparent = (pMaterial->materialFlags & 0x46) != 0;
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, !bTransparent && !(pItem->flags & 0x20));
        if (pMaterial->materialFlags & 0x40)
        {
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
        }
        if (bTransparent)
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, bAlphaTest ? color.a >> 1 : 1);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        }
        break;
    }

    case 4:
        if (uiPass)
        {
            bTexture = false;
            break;
        }
        if (pMaterial->materialFlags & 2)
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        }
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) ? FALSE : TRUE);
        break;

    default:
        bTexture = false;
        break;
    }

    if (bTexture)
    {
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

        if (pTransform)
        {
            PblMatrix mTransform = gMatrixIdentity;
            mTransform.m[0][0] = (*pTransform)(0, 0);
            mTransform.m[0][1] = (*pTransform)(0, 1);
            mTransform.m[0][2] = 0.0f;
            mTransform.m[0][3] = 0.0f;
            mTransform.m[1][0] = (*pTransform)(1, 0);
            mTransform.m[1][1] = (*pTransform)(1, 1);
            mTransform.m[1][2] = 0.0f;
            mTransform.m[1][3] = 0.0f;
            mTransform.m[2][0] = (*pTransform)(2, 0);
            mTransform.m[2][1] = (*pTransform)(2, 1);
            mTransform.m[2][2] = 1.0f;
            mTransform.m[2][3] = 0.0f;
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcFFSetTextureTransform(0, mTransform);
        }
        else
        {
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        }
    }

    RedRenderer::pcLoadFormat(m_pVertexInputFormat ? m_pVertexInputFormat : pItem->pPrim->m_vertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x007003c0
void pcNormalShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    const unsigned int uiFlags = PIPELINE1_INV_STATE_MAP[pItem->uiShaderState];
    RedRenderer::pcLoadFormat(m_pVertexInputFormat ? m_pVertexInputFormat : pItem->pPrim->m_vertexFormat);

    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    const bool bBlend = ((uiFlags >> 1) & 1) != 0;
    const unsigned int uiMatFlags = pMaterial->materialFlags;
    const bool bAlphaTest = ((uiMatFlags >> 1) & 1) != 0;
    const bool bTransparent = (uiMatFlags & 0x46) != 0;
    const bool bFlag10 = ((uiMatFlags >> 4) & 1) != 0;

    RedColor color;
    color.r = (unsigned char)(int)(pMaterial->diffuseColor.r * 255.0f);
    color.g = (unsigned char)(int)(pMaterial->diffuseColor.g * 255.0f);
    color.b = (unsigned char)(int)(pMaterial->diffuseColor.b * 255.0f);
    color.a = (unsigned char)(int)(pMaterial->diffuseColor.a * 255.0f);
    color = color * pItem->tweakColor;

    const bool bSeparate = m_lightingInSeparatePass && !(pItem->flags & 0x10000000);
    RedRenderer::pcSetCullMode((uiMatFlags & 0x10000) ? PCREDCULL_NONE : PCREDCULL_BACK);

    RedColorValue psConst;
    if (bBlend)
    {
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, !bTransparent && !(pItem->flags & 0x20));
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        if (pMaterial->materialFlags & 0x40)
        {
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
        }
        psConst.b = bTransparent ? 1.0f : 0.0f;
        psConst.a = bFlag10 ? 1.0f : 0.0f;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &psConst.r, 1);
    }
    else if (bSeparate)
    {
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) ? FALSE : TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
        psConst.b = bTransparent ? 1.0f : 0.0f;
        psConst.a = bFlag10 ? 1.0f : 0.0f;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &psConst.r, 1);
    }
    else
    {
        psConst.b = 0.0f;
        psConst.a = bFlag10 ? 1.0f : 0.0f;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &psConst.r, 1);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) ? FALSE : TRUE);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
    }

    if (bAlphaTest)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }

    unsigned int uiDiffuseMap;
    const RedTextureTransform* pTransform = &pMaterial->diffuseTexTransform;
    if (pItem->pShadingState)
    {
        uiDiffuseMap = pItem->pShadingState->GetIntParam(0x891891e9);
        pTransform = &pItem->pShadingState->GetTextureTransformParam(0x3f0d728a);
    }
    else
    {
        uiDiffuseMap = pMaterial->diffuseMapHash;
    }
    const RedTexture* pDiffuse = uiDiffuseMap ? RedTexture::Find(uiDiffuseMap) : 0;
    const RedTexture* pDetail = pMaterial->detailMapHash ? RedTexture::Find(pMaterial->detailMapHash)
                                                          : s_pNullDetailMap;
    RedRenderer::pcSetTexture(0, pDiffuse);
    RedRenderer::pcSetTexture(1, pDetail);

    const RedTextureTransform& detail = pMaterial->detailTexTransform;
    pcRedVertexShaderConstantRegister* pRegs;
    if (!pTransform->m_bTransform && !detail.m_bTransform && !s_bTextureTransformPrev && !s_bDetailTextureTransformPrev &&
        D3DCOLOR_ARGB(s_colorDiffusePrev.a, s_colorDiffusePrev.r, s_colorDiffusePrev.g,
                      s_colorDiffusePrev.b) == D3DCOLOR_ARGB(color.a, color.r, color.g, color.b))
    {
        pRegs = RedRenderer::pcLockVertexShaderConstants(40, 1);
        if (bSeparate)
        {
            pRegs[40].component[0] = 0.0f;
            pRegs[40].component[1] = RedRenderer::GetLightingScale();
        }
        else
        {
            pRegs[40].component[0] = 1.0f;
            pRegs[40].component[1] = 0.0f;
        }
        pRegs[40].component[3] = 0.0f;
        pRegs[40].component[2] = 0.0f;
    }
    else
    {
        s_bTextureTransformPrev = pTransform->m_bTransform != 0;
        s_bDetailTextureTransformPrev = detail.m_bTransform != 0;
        s_colorDiffusePrev = color;

        pRegs = RedRenderer::pcLockVertexShaderConstants(39, 6);
        pRegs[39].component[0] = color.r * kInv255;
        pRegs[39].component[1] = color.g * kInv255;
        pRegs[39].component[2] = color.b * kInv255;
        pRegs[39].component[3] = color.a * kInv255;
        if (bSeparate)
        {
            pRegs[40].component[0] = 0.0f;
            pRegs[40].component[1] = RedRenderer::GetLightingScale();
        }
        else
        {
            pRegs[40].component[0] = 1.0f;
            pRegs[40].component[1] = 0.0f;
        }
        pRegs[40].component[3] = 0.0f;
        pRegs[40].component[2] = 0.0f;

        pRegs[41].component[0] = (*pTransform)(0, 0);
        pRegs[41].component[1] = (*pTransform)(1, 0);
        pRegs[41].component[2] = 0.0f;
        pRegs[41].component[3] = (*pTransform)(2, 0);
        pRegs[42].component[0] = (*pTransform)(0, 1);
        pRegs[42].component[1] = (*pTransform)(1, 1);
        pRegs[42].component[2] = 0.0f;
        pRegs[42].component[3] = (*pTransform)(2, 1);
        pRegs[43].component[0] = detail(0, 0);
        pRegs[43].component[1] = detail(1, 0);
        pRegs[43].component[2] = 0.0f;
        pRegs[43].component[3] = detail(2, 0);
        pRegs[44].component[0] = detail(0, 1);
        pRegs[44].component[1] = detail(1, 1);
        pRegs[44].component[2] = 0.0f;
        pRegs[44].component[3] = detail(2, 1);
    }
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    if (uiFlags & 4)
    {
        const RedLight* pLight = pItem->pLightingState->pProjectedLight;
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, pLight->GetProjectionTextureWrapMode());
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, pLight->GetProjectionTextureWrapMode());
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSW, pLight->GetProjectionTextureWrapMode());
        RedRenderer::pcSetTexture(2, RedTexture::Find(pLight->m_uiProjTexHash));
        psConst = pLight->m_color;
        psConst.a = (float)(int)pLight->IsShadowCaster();
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &psConst.r, 1);
    }
    else
    {
        RedRenderer::pcSetTexture(2, RedTexture::Find(PblHash("white")));
    }

    if (pItem->flags & 4)
    {
        if (bTransparent && color.a == 0xff && (pMaterial->materialFlags & 0x4000))
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0xfc);
            RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
            pItem->pPrim->Draw();
            RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);
            RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
        if ((pMaterial->materialFlags & 0x18000) == 0x18000)
        {
            RedRenderer::pcSetCullMode(PCREDCULL_FRONT);
            pItem->pPrim->Draw();
            RedRenderer::pcSetCullMode(PCREDCULL_BACK);
        }
    }
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00700c40
void pcNormalShader::SetData(const RedShaderData& data)
{
    if (m_material.materialFlags & 0xf000000)
    {
        m_material.animationData = new RedMaterialAnimationData();
        m_material.animationData->diffuseColor = m_material.diffuseColor;
        m_material.animationData->scrollRateU = 0;
        m_material.animationData->scrollRateV = 0;
        m_material.animationData->minIntensity = 0xff;
        m_material.animationData->blinkRate = 0;
        m_material.animationData->numRows = 1;
        m_material.animationData->numCols = 1;
        m_material.animationData->frameRate = 0;
        m_material.animationData->attachedLightName = 0;
        m_material.animationData->attachedLight = 0;

        if (m_material.materialFlags & 0x1000000)
        {
            m_material.animationData->scrollRateU = data.uiData0;
            m_material.animationData->scrollRateV = data.uiData1;
        }
        else if (m_material.materialFlags & 0x4000000)
        {
            m_material.animationData->numRows = (unsigned char)(int)(float)sqrt((double)(float)data.uiData0);
            m_material.animationData->numCols = m_material.animationData->numRows;
            m_material.animationData->frameRate = data.uiData1;
        }
        else if (m_material.materialFlags & 0x2000000)
        {
            m_material.animationData->minIntensity = data.uiData0;
            m_material.animationData->blinkRate = data.uiData1;
        }

        if (m_material.materialFlags & 0x8000000)
        {
            m_material.animationData->attachedLightName = data.attachedLightName;
        }
    }
    else
    {
        float fScaleX = 1.0f;
        float fScaleY = 1.0f;
        if (data.uiData0)
        {
            fScaleX = (float)data.uiData0;
        }
        if (data.uiData1)
        {
            fScaleY = (float)data.uiData1;
        }
        if (fScaleX != 1.0f || fScaleY != 1.0f)
        {
            m_material.detailTexTransform.SetScale(fScaleX, fScaleY);
        }
    }
}

// FUNCTION: SWBF2 0x00700e30
void pcNormalShader::UpdateAnimation()
{
    RedMaterialAnimationData* pAnim = m_material.animationData;
    const float fTime = s_animationTimeStamp.GetElapsed();
    float fScrollU = 0.0f;
    float fScrollV = 0.0f;

    if (m_material.materialFlags & 0x1000000)
    {
        fScrollU -= pAnim->scrollRateU * kInv255 * fTime;
        fScrollV -= pAnim->scrollRateV * kInv255 * fTime;
    }
    else if (m_material.materialFlags & 0x4000000)
    {
        const int iNumRows = pAnim->numRows;
        const int iNumCols = pAnim->numCols;
        const int iFrame = (int)(pAnim->frameRate * fTime) % (iNumRows * iNumCols);
        const int iRow = iFrame / iNumCols;
        const int iCol = iFrame % iNumCols;
        fScrollU += (float)iCol / (float)iNumCols;
        fScrollV += (float)iRow / (float)iNumRows;
    }
    else if (m_material.materialFlags & 0x2000000)
    {
        static float s_fBlinkScale = PblMath::fTwoPi * 10.0f * kInv255;
        const float fSin = PblMath::Sin(pAnim->blinkRate * s_fBlinkScale * fTime) * 0.5f + 0.5f;
        const float fIntensity = pAnim->minIntensity * kInv255 * (1.0f - fSin) + fSin;
        m_material.diffuseColor.r = pAnim->diffuseColor.r * fIntensity;
        m_material.diffuseColor.g = pAnim->diffuseColor.g * fIntensity;
        m_material.diffuseColor.b = pAnim->diffuseColor.b * fIntensity;
    }

    if (m_material.materialFlags & 0x8000000)
    {
        static RedColorValue s_minColor(0.2f, 0.2f, 0.2f, 1.0f);
        if (!m_material.animationData->attachedLight)
        {
            m_material.animationData->attachedLight = (RedLight*)RedLight::Find(pAnim->attachedLightName);
        }
        const RedLight* pLight = m_material.animationData->attachedLight;
        if (pLight)
        {
            const float fIntensity = pLight->GetIntensity();
            m_material.diffuseColor.r = (pAnim->diffuseColor.r - s_minColor.r) * fIntensity + s_minColor.r;
            m_material.diffuseColor.g = (pAnim->diffuseColor.g - s_minColor.g) * fIntensity + s_minColor.g;
            m_material.diffuseColor.b = (pAnim->diffuseColor.b - s_minColor.b) * fIntensity + s_minColor.b;
        }
    }

    if (fScrollU > 1.0f || fScrollU < -1.0f)
    {
        fScrollU -= (float)(int)fScrollU;
    }
    if (fScrollV > 1.0f || fScrollV < -1.0f)
    {
        fScrollV -= (float)(int)fScrollV;
    }

    RedTextureTransform& transform = m_material.diffuseTexTransform;
    if (!transform.m_bTransform)
    {
        transform.m_bTransform = 1;
        transform.m_Transform[0][0] = 1.0f;
        transform.m_Transform[1][0] = 0.0f;
        transform.m_Transform[0][1] = 0.0f;
        transform.m_Transform[1][1] = 1.0f;
    }
    transform.m_Transform[2][0] = fScrollU;
    transform.m_Transform[2][1] = fScrollV;
}

// GLOBAL: SWBF2 0x009c938c
// SYNTHETIC: SWBF2 0x004097f0
// `dynamic initializer for 'pcNormalShader::s_uiRenderTypeHash''
unsigned int pcNormalShader::s_uiRenderTypeHash = PblHash("Normal");

// GLOBAL: SWBF2 0x009c92a8
// SYNTHETIC: SWBF2 0x004097b0
// `dynamic initializer for 'pcNormalShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769c30
// `dynamic atexit destructor for 'pcNormalShader::s_Factory''
pcNormalShader pcNormalShader::s_Factory;

// GLOBAL: SWBF2 0x009c934c
// SYNTHETIC: SWBF2 0x004097d0
// `dynamic initializer for 'pcNormalShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769c50
// `dynamic atexit destructor for 'pcNormalShader::s_shaderDef''
pcRedShaderDef pcNormalShader::s_shaderDef("normal_shader.xml");

// SYNTHETIC: SWBF2 0x00409810
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409830
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9268
unsigned int g_pcNormalShader;
