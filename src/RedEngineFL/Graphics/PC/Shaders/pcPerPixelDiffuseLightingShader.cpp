#include "pcPerPixelDiffuseLightingShader.h"

#include "PebbleFL/Common/PblChunk.h"

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/RedSpotLight.h"

void __cdecl PblTraceP(const char* Format, ...);

// GLOBAL: SWBF2 0x009ca964
// SYNTHETIC: SWBF2 0x0040a650
// `dynamic initializer for 's_uiDefaultSpotLightMapHash''
static unsigned int s_uiDefaultSpotLightMapHash = PblHash("default_spotlight");
// GLOBAL: SWBF2 0x009ca968
// SYNTHETIC: SWBF2 0x0040a670
// `dynamic initializer for 's_uiLightVolumeMapHash''
static unsigned int s_uiLightVolumeMapHash = PblHash("attenuation_volume");
// GLOBAL: SWBF2 0x009ca93c
// SYNTHETIC: SWBF2 0x0040a690
// `dynamic initializer for 's_uiNormalizationCubeMapHash''
static unsigned int s_uiNormalizationCubeMapHash = PblHash("cube_normalizationmap");

// GLOBAL: SWBF2 0x007854d8
static const unsigned int MAX_LIGHTS_PER_STATE[18] =
{
    3, 2, 1, 3, 2, 1, 3, 2, 1, 3, 2, 1, 3, 2, 1, 3, 2, 1,
};

unsigned int pcPerPixelDiffuseLightingShader::s_uiID;
pcRedShader::GetStateFuncT pcPerPixelDiffuseLightingShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcPerPixelDiffuseLightingShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcPerPixelDiffuseLightingShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcPerPixelDiffuseLightingShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x00711560
pcPerPixelDiffuseLightingShader::pcPerPixelDiffuseLightingShader()
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

// FUNCTION: SWBF2 0x00711620
pcPerPixelDiffuseLightingShader::pcPerPixelDiffuseLightingShader(unsigned int uiFlags,
                                                                 const unsigned int* pParams,
                                                                 unsigned int uiNumParams,
                                                                 unsigned int uiMaterialFlags,
                                                                 const RedColor& color0,
                                                                 const RedColor& color1,
                                                                 float fSpecularPower)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x007116c0
void pcPerPixelDiffuseLightingShader::Begin_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007116d0
void pcPerPixelDiffuseLightingShader::Begin_Prog1(unsigned int uiState, unsigned int uiPass) const
{
    if (uiPass)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                      D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                          D3DCOLORWRITEENABLE_BLUE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                      D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                          D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
    }
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0x7f);

    if (uiState < 6)
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    }
    else
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
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
}

// FUNCTION: SWBF2 0x00711860
void pcPerPixelDiffuseLightingShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00711880
void pcPerPixelDiffuseLightingShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00711890
void pcPerPixelDiffuseLightingShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007118a0
unsigned int pcPerPixelDiffuseLightingShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x007118c0
RenderPhaseT pcPerPixelDiffuseLightingShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x007118d0
unsigned int pcPerPixelDiffuseLightingShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x007118f0
unsigned int pcPerPixelDiffuseLightingShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00711900
unsigned int pcPerPixelDiffuseLightingShader::GetState_Prog1(const RenderItem* pItem) const
{
    const RedLightingState* pLighting = pItem->pLightingState;
    const RedMaterial* pMaterial = pItem->pRootShader->m_material;

    int iNumLights = pLighting->numDirectionalLights + pLighting->numOmniLights;
    if (iNumLights == 0)
    {
        iNumLights = 1;
    }
    int iState = 3 - iNumLights;
    if (iState < 0)
    {
        iState = 0;
    }

    if (pMaterial->materialFlags & 0x10000000)
    {
        iState += 12;
    }
    if (pMaterial->bumpMapHash)
    {
        iState += 6;
    }
    if (pLighting->pProjectedLight || pLighting->numSpotLights)
    {
        iState += 3;
    }
    return iState;
}

// FUNCTION: SWBF2 0x007118e0
pcRedShaderDef* pcPerPixelDiffuseLightingShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// GLOBAL: SWBF2 0x007855e8
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[2] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00711970
void pcPerPixelDiffuseLightingShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x28, 2, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00711990
void pcPerPixelDiffuseLightingShader::Render_Fixed(const RenderItem*, unsigned int) const
{
}

// FUNCTION: SWBF2 0x007119a0
void pcPerPixelDiffuseLightingShader::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    RedRenderer::pcLoadFormat(pItem->pPrim->m_vertexFormat);

    const RedMaterial* pMaterial = pItem->pRootShader->m_material;
    if (pMaterial->bumpMapHash)
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
    }
    else
    {
        RedRenderer::pcSetTexture(0, RedTexture::Find(s_uiNormalizationCubeMapHash));
    }

    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, (pMaterial->materialFlags & 2) != 0);
    RedRenderer::pcSetCullMode((pMaterial->materialFlags & 0x8000) ? PCREDCULL_NONE : PCREDCULL_BACK);

    const RedLightingState* pLighting = pItem->pLightingState;
    int iNumPasses = 0;
    const bool bVertexLit = (pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF & 0x100) != 0;
    const int iMaxLights = MAX_LIGHTS_PER_STATE[pItem->uiShaderState];
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) == 0);

    RedColorValue colors[3];
    const RedColorValue& black = RedColorValue::BLACK;

    if (uiPass == 0)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(19, 8);
        if (bVertexLit)
        {
            pRegs[19].component[0] = black.r;
            pRegs[19].component[1] = black.g;
            pRegs[19].component[2] = black.b;
            pRegs[19].component[3] = 1.0f;
            pRegs[20].component[0] = black.r;
            pRegs[20].component[1] = black.g;
            pRegs[20].component[2] = black.b;
            pRegs[20].component[3] = black.a;
        }
        else
        {
            const RedAmbientLight* pAmbient =
                &pLighting->ambientLight;
            pRegs[19].component[0] = pAmbient->m_topColor.r;
            pRegs[19].component[1] = pAmbient->m_topColor.g;
            pRegs[19].component[2] = pAmbient->m_topColor.b;
            pRegs[19].component[3] = 1.0f;
            pRegs[20].component[0] = pAmbient->m_bottomColor.r;
            pRegs[20].component[1] = pAmbient->m_bottomColor.g;
            pRegs[20].component[2] = pAmbient->m_bottomColor.b;
            pRegs[20].component[3] = pAmbient->m_bottomColor.a;
        }

        int iLight = 0;
        for (int i = 0; i < pLighting->numDirectionalLights; i++)
        {
            const RedDirectionalLight* pLight = pLighting->pDirectionalLights[i];
            if (pLight->m_uiProjTexHash)
            {
                continue;
            }
            if ((pLight->m_uiFlags & RedLight::LIGHT_FLAG_STATIC) && bVertexLit)
            {
                continue;
            }

            const float fFactor = pLighting->pDirectionalFactor[i];
            colors[iLight].r = pLight->m_color.r * fFactor;
            colors[iLight].g = pLight->m_color.g * fFactor;
            colors[iLight].b = pLight->m_color.b * fFactor;
            colors[iLight].a = pLight->m_color.a * fFactor;
            pRegs[21 + 2 * iLight].component[0] = pLight->m_vDirection.x;
            pRegs[21 + 2 * iLight].component[1] = pLight->m_vDirection.y;
            pRegs[21 + 2 * iLight].component[2] = pLight->m_vDirection.z;
            pRegs[21 + 2 * iLight].component[3] = 0.0f;
            pRegs[22 + 2 * iLight].component[0] = 1.0f;
            pRegs[22 + 2 * iLight].component[1] = 0.0f;
            pRegs[22 + 2 * iLight].component[2] = 0.0f;
            pRegs[22 + 2 * iLight].component[3] = 1.0f;
            iLight++;
            RedRenderer::pcSetTexture(iLight, RedTexture::Find(s_uiNormalizationCubeMapHash));

            if (iLight >= iMaxLights)
            {
                RedRenderer::pcUnlockVertexShaderConstants(pRegs);
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights);
                pItem->pPrim->Draw();
                RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
                pRegs = RedRenderer::pcLockVertexShaderConstants(19, 8);
                iLight = 0;
                iNumPasses++;
                pRegs[19].component[0] = black.r;
                pRegs[19].component[1] = black.g;
                pRegs[19].component[2] = black.b;
                pRegs[19].component[3] = 0.0f;
            }
        }

        for (int i = 0; i < pLighting->numOmniLights; i++)
        {
            const RedOmniLight* pLight = pLighting->pOmniLights[i];
            if (pLight->m_uiProjTexHash)
            {
                continue;
            }
            if ((pLight->m_uiFlags & RedLight::LIGHT_FLAG_STATIC) && bVertexLit)
            {
                continue;
            }

            colors[iLight] = pLight->m_color;
            pRegs[21 + 2 * iLight].component[0] = pLight->m_pos.x;
            pRegs[21 + 2 * iLight].component[1] = pLight->m_pos.y;
            pRegs[21 + 2 * iLight].component[2] = pLight->m_pos.z;
            pRegs[21 + 2 * iLight].component[3] = 1.0f;
            pRegs[22 + 2 * iLight].component[0] = pLight->m_fRadiusInv * 0.5f;
            pRegs[22 + 2 * iLight].component[1] = 0.5f;
            pRegs[22 + 2 * iLight].component[2] = 0.0f;
            pRegs[22 + 2 * iLight].component[3] = 1.0f;
            iLight++;
            RedRenderer::pcSetTexture(iLight, RedTexture::Find(s_uiLightVolumeMapHash));

            if (iLight >= iMaxLights)
            {
                RedRenderer::pcUnlockVertexShaderConstants(pRegs);
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights);
                pItem->pPrim->Draw();
                RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
                pRegs = RedRenderer::pcLockVertexShaderConstants(19, 8);
                iLight = 0;
                iNumPasses++;
                pRegs[19].component[0] = black.r;
                pRegs[19].component[1] = black.g;
                pRegs[19].component[2] = black.b;
                pRegs[19].component[3] = 0.0f;
            }
        }

        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        if (iLight > 0 || iNumPasses == 0)
        {
            for (int i = iLight; i < iMaxLights; i++)
            {
                colors[i] = black;
            }
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, iMaxLights);
            pItem->pPrim->Draw();
        }
        return;
    }

    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);

    for (int i = 0; i < pLighting->numSpotLights; i++)
    {
        const RedSpotLight* pLight = pLighting->pSpotLights[i];
        if ((pLight->m_uiFlags & RedLight::LIGHT_FLAG_STATIC) && bVertexLit)
        {
            continue;
        }

        colors[0] = pLight->m_color;
        colors[0].a = (float)pLight->m_bBidirectional;

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(19, 9);
        pRegs[19].component[0] = black.r;
        pRegs[19].component[1] = black.g;
        pRegs[19].component[2] = black.b;
        pRegs[19].component[3] = 0.0f;
        pRegs[21].component[0] = pLight->m_vPos.x;
        pRegs[21].component[1] = pLight->m_vPos.y;
        pRegs[21].component[2] = pLight->m_vPos.z;
        pRegs[21].component[3] = 1.0f;
        pRegs[22].component[0] = pLight->m_fRadiusInv * 0.5f;
        pRegs[22].component[1] = 0.5f;
        pRegs[22].component[2] = 0.0f;
        pRegs[22].component[3] = 1.0f;
        pRegs[23].component[0] = pLight->m_vDirection.x;
        pRegs[23].component[1] = pLight->m_vDirection.y;
        pRegs[23].component[2] = pLight->m_vDirection.z;
        pRegs[23].component[3] = pLight->m_fRadiusInv * 0.5f;
        pRegs[24].SetMatrix4x4(pLight->GetProjectionMatrix());
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetTexture(1, RedTexture::Find(s_uiLightVolumeMapHash));
        RedRenderer::pcSetTexture(2, RedTexture::Find(pLight->m_uiProjTexHash
                                                          ? pLight->m_uiProjTexHash
                                                          : s_uiDefaultSpotLightMapHash));
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, 1);
        pItem->pPrim->Draw();
    }

    const RedLight* pProjected = pLighting->pProjectedLight;
    if (!pProjected)
    {
        return;
    }

    if (pProjected->m_eType == RedLight::TYPE_DIRECTIONAL)
    {
        const RedDirectionalLight* pLight = static_cast<const RedDirectionalLight*>(pProjected);
        colors[0] = pLight->m_color;
        colors[0].a = 1.0f;

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(19, 9);
        pRegs[19].component[0] = black.r;
        pRegs[19].component[1] = black.g;
        pRegs[19].component[2] = black.b;
        pRegs[19].component[3] = 0.0f;
        pRegs[21].component[0] = pLight->m_vDirection.x;
        pRegs[21].component[1] = pLight->m_vDirection.y;
        pRegs[21].component[2] = pLight->m_vDirection.z;
        pRegs[21].component[3] = 0.0f;
        pRegs[22].component[0] = 1.0f;
        pRegs[22].component[1] = 0.0f;
        pRegs[22].component[2] = 0.0f;
        pRegs[22].component[3] = 1.0f;
        pRegs[24].SetMatrix4x4(pLight->GetProjectionMatrix());
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetTexture(1, RedTexture::Find(s_uiNormalizationCubeMapHash));
        RedRenderer::pcSetTexture(2, RedTexture::Find(pLight->m_uiProjTexHash));
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    }
    else if (pProjected->m_eType == RedLight::TYPE_POINT)
    {
        const RedOmniLight* pLight = static_cast<const RedOmniLight*>(pProjected);
        colors[0] = pLight->m_color;
        colors[0].a = 1.0f;

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(19, 9);
        pRegs[19].component[0] = black.r;
        pRegs[19].component[1] = black.g;
        pRegs[19].component[2] = black.b;
        pRegs[19].component[3] = 0.0f;
        pRegs[21].component[0] = pLight->m_pos.x;
        pRegs[21].component[1] = pLight->m_pos.y;
        pRegs[21].component[2] = pLight->m_pos.z;
        pRegs[21].component[3] = 1.0f;
        pRegs[22].component[0] = pLight->m_fRadiusInv * 0.5f;
        pRegs[22].component[1] = 0.5f;
        pRegs[22].component[2] = 0.0f;
        pRegs[22].component[3] = 1.0f;
        pRegs[24].SetMatrix4x4(pLight->GetProjectionMatrix());
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetTexture(1, RedTexture::Find(s_uiLightVolumeMapHash));
        RedRenderer::pcSetTexture(2, RedTexture::Find(pLight->m_uiProjTexHash));
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }
    else
    {
        return;
    }

    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, 1);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x007123d0
pcRedShader* pcPerPixelDiffuseLightingShader::_Create(unsigned int uiFlags,
                                                      const unsigned int* pParams,
                                                      unsigned int uiNumParams,
                                                      unsigned int uiMaterialFlags,
                                                      const RedColor& color0,
                                                      const RedColor& color1,
                                                      float fSpecularPower) const
{
    return new pcPerPixelDiffuseLightingShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                               color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x00712460
void pcPerPixelDiffuseLightingShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcPerPixelDiffuseLightingShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcPerPixelDiffuseLightingShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcPerPixelDiffuseLightingShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcPerPixelDiffuseLightingShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcPerPixelDiffuseLightingShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcPerPixelDiffuseLightingShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcPerPixelDiffuseLightingShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcPerPixelDiffuseLightingShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x007124c0
void pcPerPixelDiffuseLightingShader::_Read(PblFileChunk& chunk) const
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

// SYNTHETIC: SWBF2 0x0040a5f0
// `dynamic initializer for 'pcPerPixelDiffuseLightingShader::s_uiRenderTypeHash''
unsigned int pcPerPixelDiffuseLightingShader::s_uiRenderTypeHash = PblHash("perpixeldiffuselighting");

// SYNTHETIC: SWBF2 0x0040a5b0
// `dynamic initializer for 'pcPerPixelDiffuseLightingShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769ff0
// `dynamic atexit destructor for 'pcPerPixelDiffuseLightingShader::s_Factory''
pcPerPixelDiffuseLightingShader pcPerPixelDiffuseLightingShader::s_Factory;

// SYNTHETIC: SWBF2 0x0040a5d0
// `dynamic initializer for 'pcPerPixelDiffuseLightingShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x0076a010
// `dynamic atexit destructor for 'pcPerPixelDiffuseLightingShader::s_shaderDef''
pcRedShaderDef pcPerPixelDiffuseLightingShader::s_shaderDef("perpixeldiffuselighting_shader.xml");

// SYNTHETIC: SWBF2 0x0040a630
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x0040a610
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// GLOBAL: SWBF2 0x009ca908
unsigned int g_pcPerPixelDiffuseLightingShader;
