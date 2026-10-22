#include "pcSpecularLightingShader.h"

#include "PebbleFL/Common/PblChunk.h"

#include <math.h>
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/RedSpotLight.h"

void __cdecl PblTraceP(const char* Format, ...);

// GLOBAL: SWBF2 0x009ca9c8
// SYNTHETIC: SWBF2 0x0040a7c0
// `dynamic initializer for 's_uiSpecularCubeMapHash''
static unsigned int s_uiSpecularCubeMapHash = PblHash("specularcubemap");
// GLOBAL: SWBF2 0x009ca9d8
// SYNTHETIC: SWBF2 0x0040a7e0
// `dynamic initializer for 's_uiSpecularSpotMapHash''
static unsigned int s_uiSpecularSpotMapHash = PblHash("specularspot");

static const float kInv255 = 1.0f / 255.0f;
static const float kfSpecularSizeInv = 0.3f;

static const float kfMinSpecularPower = 10.0f;
static const float kfMaxSpecularPower = 100.0f;

// GLOBAL: SWBF2 0x00785620
static const unsigned int MAX_LIGHTS_PER_STATE[10] = { 0, 3, 2, 1, 1, 0, 1, 1, 0, 1 };

unsigned int pcSpecularLightingShader::s_uiID;
pcRedShader::GetStateFuncT pcSpecularLightingShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcSpecularLightingShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcSpecularLightingShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcSpecularLightingShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x007125a0
pcSpecularLightingShader::pcSpecularLightingShader()
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

// FUNCTION: SWBF2 0x00712660
pcSpecularLightingShader::pcSpecularLightingShader(unsigned int uiFlags,
                                                   const unsigned int* pParams,
                                                   unsigned int uiNumParams,
                                                   unsigned int uiMaterialFlags,
                                                   const RedColor& color0,
                                                   const RedColor& color1,
                                                   float fSpecularPower)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x007129b0
pcRedShaderDef* pcSpecularLightingShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// GLOBAL: SWBF2 0x007856d8
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[2] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00712b80
void pcSpecularLightingShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x28, 2, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x007136e0
pcRedShader* pcSpecularLightingShader::_Create(unsigned int uiFlags,
                                               const unsigned int* pParams,
                                               unsigned int uiNumParams,
                                               unsigned int uiMaterialFlags,
                                               const RedColor& color0,
                                               const RedColor& color1,
                                               float fSpecularPower) const
{
    return new pcSpecularLightingShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                        color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x00713770
void pcSpecularLightingShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcSpecularLightingShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcSpecularLightingShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcSpecularLightingShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcSpecularLightingShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcSpecularLightingShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcSpecularLightingShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcSpecularLightingShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcSpecularLightingShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x007137d0
void pcSpecularLightingShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00712930
void pcSpecularLightingShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00712970
unsigned int pcSpecularLightingShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00712990
RenderPhaseT pcSpecularLightingShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x007129a0
unsigned int pcSpecularLightingShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00712a80
unsigned int pcSpecularLightingShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00712700
void pcSpecularLightingShader::Begin_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00712950
void pcSpecularLightingShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00712960
void pcSpecularLightingShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007134a0
void pcSpecularLightingShader::Render_Fixed(const RenderItem*, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007129c0
RedColorValue GetSpecularWeights(float fSpecularPower)
{
    const float fT = (fSpecularPower - kfMinSpecularPower) /
                     (kfMaxSpecularPower - kfMinSpecularPower);
    RedColorValue weights;
    weights.r = 0.0f;
    weights.g = 0.0f;
    weights.b = 0.0f;
    weights.a = 0.0f;
    if (fT < 0.333333f)
    {
        weights.g = fT / 0.33333f;
        weights.r = 1.0f - weights.g;
    }
    else if (fT < 0.666666f)
    {
        weights.b = (fT - 0.33333f) / 0.33333f;
        weights.g = 1.0f - weights.b;
    }
    else
    {
        weights.a = (fT - 0.66666f) / 0.33333f;
        weights.b = 1.0f - weights.a;
    }
    return weights;
}

// FUNCTION: SWBF2 0x00712a90
unsigned int pcSpecularLightingShader::GetState_Prog1(const RenderItem* pItem) const
{
    const RedLightingState* pLighting = pItem->pLightingState;
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;

    const bool bEnvMap = (pMaterial->materialFlags & 0x100) &&
                         (pMaterial->envMapHash || pLighting->pEnvMap);
    int iNumLights = pLighting->numSpecularLights;
    if (!(pMaterial->materialFlags & 0x80))
    {
        iNumLights = 0;
    }

    if (pMaterial->bumpMapHash && RedRenderer::pcGetRenderOptions().bumpMapping)
    {
        if (pMaterial->materialFlags & 0x10000000)
        {
            if (iNumLights)
            {
                return bEnvMap ? 9 : 7;
            }
            return bEnvMap ? 8 : 7;
        }
        if (iNumLights)
        {
            return bEnvMap ? 6 : 4;
        }
        return bEnvMap ? 5 : 4;
    }

    if (bEnvMap)
    {
        iNumLights++;
    }
    if (iNumLights < 1)
    {
        iNumLights = 1;
    }
    else if (iNumLights > 3)
    {
        iNumLights = 3;
    }
    return 4 - iNumLights;
}

// FUNCTION: SWBF2 0x00712710
void pcSpecularLightingShader::Begin_Prog1(unsigned int uiState, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                      D3DCOLORWRITEENABLE_BLUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    for (unsigned int i = 1; i < 4; i++)
    {
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    if (uiState > 3)
    {
        RedRenderer::pcSetTexture(3, RedTexture::Find(s_uiSpecularCubeMapHash));
    }
    else
    {
        for (unsigned int i = 1; i < 4; i++)
        {
            RedRenderer::pcSetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
            RedRenderer::pcSetTexture(i, RedTexture::Find(s_uiSpecularSpotMapHash));
        }
    }
}

// FUNCTION: SWBF2 0x00712ba0
void pcSpecularLightingShader::PerPixelLighting(const RenderItem* pItem, unsigned int uiPass) const
{
    const RedLightingState* pLighting = pItem->pLightingState;
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;

    RedColorValue colors[3];
    const float fScale = RedRenderer::GetLightingScale() * (pItem->tweakColor.a * kInv255);
    colors[0].r = pMaterial->specularColor.r * fScale;
    colors[0].g = pMaterial->specularColor.g * fScale;
    colors[0].b = pMaterial->specularColor.b * fScale;
    colors[0].a = (pMaterial->materialFlags & 8) ? 1.0f : 0.0f;

    if (uiPass || pItem->uiShaderState == 5 || pItem->uiShaderState == 8)
    {
        if (pMaterial->envMapHash)
        {
            RedRenderer::pcSetTexture(3, RedTexture::Find(pMaterial->envMapHash));
        }
        else if (pLighting->pEnvMap)
        {
            RedRenderer::pcSetTexture(3, pLighting->pEnvMap);
        }

        const RedColorValue& white = RedColorValue::WHITE;
        colors[2].r = colors[0].r * white.r;
        colors[2].g = colors[0].g * white.g;
        colors[2].b = colors[0].b * white.b;
        colors[2].a = colors[0].a * white.a;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, 3);
        pItem->pPrim->Draw();
        return;
    }

    colors[1] = GetSpecularWeights(pMaterial->specularPower);

    for (int i = 0; i < pLighting->numDirectionalLights; i++)
    {
        const RedDirectionalLight* pLight = pLighting->pDirectionalLights[i];
        if (!pLight->IsSpecularCaster())
        {
            continue;
        }

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(42, 1);
        pRegs[42].component[0] = -pLight->m_vDirection.x;
        pRegs[42].component[1] = -pLight->m_vDirection.y;
        pRegs[42].component[2] = -pLight->m_vDirection.z;
        pRegs[42].component[3] = 0.0f;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        const float fFactor = pLighting->pDirectionalFactor[i];
        colors[2].r = pLight->m_color.r * fFactor;
        colors[2].g = pLight->m_color.g * fFactor;
        colors[2].b = pLight->m_color.b * fFactor;
        colors[2].a = pLight->m_color.a * fFactor;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, 3);
        pItem->pPrim->Draw();
    }

    for (int i = 0; i < pLighting->numOmniLights; i++)
    {
        const RedOmniLight* pLight = pLighting->pOmniLights[i];
        if (!pLight->IsSpecularCaster())
        {
            continue;
        }

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(42, 1);
        pRegs[42].component[0] = pLight->m_pos.x;
        pRegs[42].component[1] = pLight->m_pos.y;
        pRegs[42].component[2] = pLight->m_pos.z;
        pRegs[42].component[3] = pLight->m_fRadiusInvSq;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        colors[2] = pLight->m_color;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, 3);
        pItem->pPrim->Draw();
    }

    for (int i = 0; i < pLighting->numSpotLights; i++)
    {
        const RedSpotLight* pLight = pLighting->pSpotLights[i];
        if (!pLight->IsSpecularCaster())
        {
            continue;
        }

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(42, 1);
        pRegs[42].component[0] = pLight->m_vPos.x;
        pRegs[42].component[1] = pLight->m_vPos.y;
        pRegs[42].component[2] = pLight->m_vPos.z;
        pRegs[42].component[3] = pLight->m_fRadiusInvSq;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        colors[2] = pLight->m_color;
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, 3);
        pItem->pPrim->Draw();
    }
}

// FUNCTION: SWBF2 0x00712f60
void pcSpecularLightingShader::PerVertexLighting(const RenderItem* pItem, unsigned int) const
{
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    const RedLightingState* pLighting = pItem->pLightingState;
    int iNumPasses = 0;
    const int iMaxLights = MAX_LIGHTS_PER_STATE[pItem->uiShaderState];

    RedColorValue colors[6];
    const float fScale = RedRenderer::GetLightingScale() * (pItem->tweakColor.a * kInv255);
    colors[0].r = pMaterial->specularColor.r * fScale;
    colors[0].g = pMaterial->specularColor.g * fScale;
    colors[0].b = pMaterial->specularColor.b * fScale;
    colors[0].a = (pMaterial->materialFlags & 8) ? 1.0f : 0.0f;

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(42, 4);
    pRegs[42].component[0] = 0.0f;
    pRegs[42].component[1] = 0.0f;
    pRegs[42].component[2] = 0.0f;
    pRegs[42].component[3] = sqrtf(pMaterial->specularPower) * kfSpecularSizeInv;

    int iLight = 0;
    if (pMaterial->materialFlags & 0x80)
    {
        for (int i = 0; i < pLighting->numDirectionalLights; i++)
        {
            const RedDirectionalLight* pLight = pLighting->pDirectionalLights[i];
            if (!pLight->IsSpecularCaster())
            {
                continue;
            }

            const float fFactor = pLighting->pDirectionalFactor[i];
            colors[2 + iLight].r = pLight->m_color.r * fFactor;
            colors[2 + iLight].g = pLight->m_color.g * fFactor;
            colors[2 + iLight].b = pLight->m_color.b * fFactor;
            colors[2 + iLight].a = pLight->m_color.a * fFactor;
            pRegs[42].component[iLight] = 0.0f;
            pRegs[43 + iLight].component[0] = -pLight->m_vDirection.x;
            pRegs[43 + iLight].component[1] = -pLight->m_vDirection.y;
            pRegs[43 + iLight].component[2] = -pLight->m_vDirection.z;
            pRegs[43 + iLight].component[3] = 0.0f;
            iLight++;

            if (iLight >= iMaxLights)
            {
                RedRenderer::pcUnlockVertexShaderConstants(pRegs);
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights + 2);
                pItem->pPrim->Draw();
                pRegs = RedRenderer::pcLockVertexShaderConstants(42, 4);
                iLight = 0;
                iNumPasses++;
            }
        }

        for (int i = 0; i < pLighting->numOmniLights; i++)
        {
            const RedOmniLight* pLight = pLighting->pOmniLights[i];
            if (!pLight->IsSpecularCaster())
            {
                continue;
            }

            colors[2 + iLight] = pLight->m_color;
            pRegs[42].component[iLight] = 0.0f;
            pRegs[43 + iLight].component[0] = pLight->m_pos.x;
            pRegs[43 + iLight].component[1] = pLight->m_pos.y;
            pRegs[43 + iLight].component[2] = pLight->m_pos.z;
            pRegs[43 + iLight].component[3] = pLight->m_fRadiusInvSq;
            iLight++;

            if (iLight >= iMaxLights)
            {
                RedRenderer::pcUnlockVertexShaderConstants(pRegs);
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights + 2);
                pItem->pPrim->Draw();
                pRegs = RedRenderer::pcLockVertexShaderConstants(42, 4);
                iLight = 0;
                iNumPasses++;
            }
        }

        for (int i = 0; i < pLighting->numSpotLights; i++)
        {
            const RedSpotLight* pLight = pLighting->pSpotLights[i];
            if (!pLight->IsSpecularCaster())
            {
                continue;
            }

            colors[2 + iLight] = pLight->m_color;
            pRegs[42].component[iLight] = 0.0f;
            pRegs[43 + iLight].component[0] = pLight->m_vPos.x;
            pRegs[43 + iLight].component[1] = pLight->m_vPos.y;
            pRegs[43 + iLight].component[2] = pLight->m_vPos.z;
            pRegs[43 + iLight].component[3] = pLight->m_fRadiusInvSq;
            iLight++;

            if (iLight >= iMaxLights)
            {
                RedRenderer::pcUnlockVertexShaderConstants(pRegs);
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights + 2);
                pItem->pPrim->Draw();
                pRegs = RedRenderer::pcLockVertexShaderConstants(42, 4);
                iLight = 0;
                iNumPasses++;
            }
        }
    }

    unsigned int uiSpotStage = 0;
    if (pMaterial->materialFlags & 0x100)
    {
        if (pMaterial->envMapHash)
        {
            colors[2 + iLight] = RedColorValue::WHITE;
            pRegs[42].component[iLight] = 1.0f;
            iLight++;
            uiSpotStage = iLight;
            RedRenderer::pcSetTexture(iLight, RedTexture::Find(pMaterial->envMapHash));
            RedRenderer::pcSetTextureStageState(iLight, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        }
        else if (pLighting->pEnvMap)
        {
            colors[2 + iLight] = RedColorValue::WHITE;
            pRegs[42].component[iLight] = 1.0f;
            iLight++;
            uiSpotStage = iLight;
            RedRenderer::pcSetTexture(iLight, pLighting->pEnvMap);
            RedRenderer::pcSetTextureStageState(iLight, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        }
    }

    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    if (iLight > 0 || iNumPasses == 0)
    {
        for (int i = iLight; i < iMaxLights; i++)
        {
            colors[2 + i] = RedColorValue::BLACK;
        }
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights + 2);
        pItem->pPrim->Draw();
    }

    RedRenderer::pcSetTexture(uiSpotStage, RedTexture::Find(s_uiSpecularSpotMapHash));
    RedRenderer::pcSetTextureStageState(uiSpotStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
}

// FUNCTION: SWBF2 0x007134b0
void pcSpecularLightingShader::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    RedRenderer::pcLoadFormat(pItem->pPrim->m_vertexFormat);

    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x10000) ? PCREDCULL_NONE : PCREDCULL_BACK);

    if (pMaterial->bumpMapHash && RedRenderer::pcGetRenderOptions().bumpMapping)
    {
        RedRenderer::pcSetTexture(0, RedTexture::Find(pMaterial->bumpMapHash));
        const RedTextureTransform& transform = (pMaterial->materialFlags & 0x800)
                                                   ? pMaterial->detailTexTransform
                                                   : pMaterial->diffuseTexTransform;
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 2);
        pRegs[40].component[0] = transform(0, 0);
        pRegs[40].component[1] = transform(1, 0);
        pRegs[40].component[2] = 0.0f;
        pRegs[40].component[3] = transform(2, 0);
        pRegs[41].component[0] = transform(0, 1);
        pRegs[41].component[1] = transform(1, 1);
        pRegs[41].component[2] = 0.0f;
        pRegs[41].component[3] = transform(2, 1);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        PerPixelLighting(pItem, uiPass);
    }
    else
    {
        const unsigned int uiDiffuseMap = pItem->pShadingState
                                              ? pItem->pShadingState->GetIntParam(0x891891e9)
                                              : pMaterial->diffuseMapHash;
        RedRenderer::pcSetTexture(0, RedTexture::Find(uiDiffuseMap));
        const RedTextureTransform& transform = pMaterial->diffuseTexTransform;
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 2);
        pRegs[40].component[0] = transform(0, 0);
        pRegs[40].component[1] = transform(1, 0);
        pRegs[40].component[2] = 0.0f;
        pRegs[40].component[3] = transform(2, 0);
        pRegs[41].component[0] = transform(0, 1);
        pRegs[41].component[1] = transform(1, 1);
        pRegs[41].component[2] = 0.0f;
        pRegs[41].component[3] = transform(2, 1);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        PerVertexLighting(pItem, uiPass);
    }
}

// GLOBAL: SWBF2 0x009caa0c
// SYNTHETIC: SWBF2 0x0040a6f0
// `dynamic initializer for 'pcSpecularLightingShader::s_uiRenderTypeHash''
unsigned int pcSpecularLightingShader::s_uiRenderTypeHash = PblHash("specularlighting");

// GLOBAL: SWBF2 0x009ca9a8
// SYNTHETIC: SWBF2 0x0040a6b0
// `dynamic initializer for 'pcSpecularLightingShader::s_Factory''
// SYNTHETIC: SWBF2 0x0076a020
// `dynamic atexit destructor for 'pcSpecularLightingShader::s_Factory''
pcSpecularLightingShader pcSpecularLightingShader::s_Factory;

// GLOBAL: SWBF2 0x009ca9dc
// SYNTHETIC: SWBF2 0x0040a6d0
// `dynamic initializer for 'pcSpecularLightingShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x0076a040
// `dynamic atexit destructor for 'pcSpecularLightingShader::s_shaderDef''
pcRedShaderDef pcSpecularLightingShader::s_shaderDef("specularlighting_shader.xml");

// GLOBAL: SWBF2 0x009ca96c
unsigned int g_pcSpecularLightingShader;

// GLOBAL: SWBF2 0x009caa08
// SYNTHETIC: SWBF2 0x0040a780
// `dynamic initializer for 's_uiLightVolumeMapHash''
static unsigned int s_uiLightVolumeMapHash = PblHash("attenuation_volume");

// GLOBAL: SWBF2 0x009ca9cc
// SYNTHETIC: SWBF2 0x0040a7a0
// `dynamic initializer for 's_uiNormalizationCubeMapHash''
static unsigned int s_uiNormalizationCubeMapHash = PblHash("cube_normalizationmap");

static const float kfSpecPowerDefault = 32.0f;

// GLOBAL: SWBF2 0x009ca9c4
// SYNTHETIC: SWBF2 0x0040a750
// `dynamic initializer for 'kfSpecSizeInvDefault''
static float kfSpecSizeInvDefault = sqrtf(kfSpecPowerDefault) * kfSpecularSizeInv;
