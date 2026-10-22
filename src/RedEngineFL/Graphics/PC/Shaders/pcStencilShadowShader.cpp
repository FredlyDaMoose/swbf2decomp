#include "pcStencilShadowShader.h"

#include <d3d9.h>

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/RedSpotLight.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcStencilShadowShader::s_uiID;
pcRedShader::GetStateFuncT pcStencilShadowShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcStencilShadowShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcStencilShadowShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcStencilShadowShader::RenderFuncArray[MAX_PIPELINES];
pcRedVertexFormat* pcStencilShadowShader::s_pSkinnedInputFormat[4];
unsigned int pcStencilShadowShader::s_uiStencilMask = 0xff;

// GLOBAL: SWBF2 0x009c9cd8
static bool s_bDebugShadowVolumes;

// GLOBAL: SWBF2 0x009c9cdc
static unsigned long s_bTwoSidedStencilTest;

// GLOBAL: SWBF2 0x007e1d88
static unsigned int STATES_ARRAY[3][pcStencilShadowShader::NUM_MODES] =
{
    { 0, 1, 2, 3, 4, 5 },
    { 0, 6, 7, 8, 9, 10 },
    { 0, 11, 12, 13, 14, 15 },
};

// GLOBAL: SWBF2 0x007e1e30

static bool s_DrawFlaggedDebugVolumes = true;
// GLOBAL: SWBF2 0x007e1e31

static bool s_DrawUnflaggedDebugVolumes = true;
// GLOBAL: SWBF2 0x007e1e34

static float s_DebugFrontFaceColor[4] = { 0.2f, 0.2f, 1.0f, 0.3f };
// GLOBAL: SWBF2 0x007e1e44

static float s_DebugBackFaceColor[4] = { 1.0f, 0.0f, 0.0f, 0.3f };
// GLOBAL: SWBF2 0x007e1e54

static float s_DebugNearWireColor[4] = { 1.0f, 0.0f, 1.0f, 0.3f };
// GLOBAL: SWBF2 0x007e1e64

static float s_DebugFarWireColor[4] = { 0.8f, 0.2f, 0.8f, 0.1f };

// GLOBAL: SWBF2 0x00784cbc
static const D3DVERTEXELEMENT9 s_softskinned_facenormal_ShadowFormat[7] =
{
    { 0, 0, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
    { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
    { 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 1 },
    { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 1 },
    D3DDECL_END()
};

// GLOBAL: SWBF2 0x00784cf4
static const D3DVERTEXELEMENT9 s_hardskinned_facenormal_ShadowFormat[4] =
{
    { 0, 0, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
    D3DDECL_END()
};

// GLOBAL: SWBF2 0x00784d14
static const D3DVERTEXELEMENT9 s_hardskinned_generatenormal_ShadowFormat[5] =
{
    { 0, 0, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
    { 0, 12, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
    { 0, 20, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2 },
    D3DDECL_END()
};

// GLOBAL: SWBF2 0x00784d40
static const D3DVERTEXELEMENT9 s_softskinned_generatenormal_ShadowFormat[10] =
{
    { 0, 0, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
    { 0, 16, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
    { 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 1 },
    { 0, 28, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 1 },
    { 0, 32, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2 },
    { 0, 40, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 2 },
    { 0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 2 },
    D3DDECL_END()
};

// FUNCTION: SWBF2 0x00706f60
pcStencilShadowShader::pcStencilShadowShader()
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

// FUNCTION: SWBF2 0x00707020
pcStencilShadowShader::pcStencilShadowShader(unsigned int uiFlags,
                                             const unsigned int* pParams,
                                             unsigned int uiNumParams,
                                             unsigned int uiMaterialFlags,
                                             const RedColor& color0,
                                             const RedColor& color1,
                                             float fArg7)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
    _Init(uiFlags, pParams, uiNumParams, uiMaterialFlags, color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x007070e0
void pcStencilShadowShader::Begin_Fixed(unsigned int, unsigned int uiPass) const
{
    if (s_bDebugShadowVolumes)
    {
        RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                      D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                          D3DCOLORWRITEENABLE_BLUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetCullMode(uiPass ? PCREDCULL_BACK : PCREDCULL_FRONT);
        return;
    }

    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
    RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
    RedRenderer::pcSetRenderState(D3DRS_STENCILENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    RedRenderer::pcSetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    RedRenderer::pcSetRenderState(D3DRS_STENCILREF, s_uiStencilMask);
    RedRenderer::pcSetRenderState(D3DRS_STENCILMASK, s_uiStencilMask);
    RedRenderer::pcSetRenderState(D3DRS_STENCILWRITEMASK, s_uiStencilMask);
    if (uiPass)
    {
        RedRenderer::pcSetCullMode(PCREDCULL_BACK);
        RedRenderer::pcSetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
    }
    else
    {
        RedRenderer::pcSetCullMode(PCREDCULL_FRONT);
        RedRenderer::pcSetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
    }
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x00707220
void pcStencilShadowShader::Begin_Prog1(unsigned int, unsigned int uiPass) const
{
    if (s_bDebugShadowVolumes)
    {
        RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                      D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                          D3DCOLORWRITEENABLE_BLUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetCullMode(uiPass ? PCREDCULL_BACK : PCREDCULL_FRONT);
        return;
    }

    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_STENCILENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_TWOSIDEDSTENCILMODE, s_bTwoSidedStencilTest);
    RedRenderer::pcSetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    RedRenderer::pcSetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    if (s_bTwoSidedStencilTest)
    {
        RedRenderer::pcSetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
        RedRenderer::pcSetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
    }
    RedRenderer::pcSetRenderState(D3DRS_STENCILREF, 1);
    RedRenderer::pcSetRenderState(D3DRS_STENCILMASK, 0xffffffff);
    RedRenderer::pcSetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
}

// FUNCTION: SWBF2 0x00707310
void pcStencilShadowShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00707330
void pcStencilShadowShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00707340
void pcStencilShadowShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00707350
unsigned int pcStencilShadowShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00707370
RenderPhaseT pcStencilShadowShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return s_bDebugShadowVolumes ? RENDERPHASE_TRANSPARENT : RENDERPHASE_SHADOW;
}

// FUNCTION: SWBF2 0x00707390
unsigned int pcStencilShadowShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x007073a0
pcRedShaderDef* pcStencilShadowShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x007073b0
unsigned int pcStencilShadowShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x007073c0
unsigned int pcStencilShadowShader::GetState_Prog1(const RenderItem* pItem) const
{
    const RedLightingState* pLights = pItem->pLightingState;
    if (!pLights->pOmniLights[0] && !pLights->pSpotLights[0])
    {
        return STATES_ARRAY[0][m_mode];
    }
    if (pLights->pDirectionalLights[0])
    {
        return STATES_ARRAY[2][m_mode];
    }
    return STATES_ARRAY[1][m_mode];
}

// GLOBAL: SWBF2 0x00784e1c
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 2, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00707410
void pcStencilShadowShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00707430
void pcStencilShadowShader::RenderVolume(const RenderItem* pItem, unsigned int) const
{
    if (s_bDebugShadowVolumes)
    {
        if (!((pItem->flags & 0x400) ? s_DrawFlaggedDebugVolumes : s_DrawUnflaggedDebugVolumes))
        {
            return;
        }
        RedRenderer::pcSetCullMode(PCREDCULL_BACK);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, s_DebugBackFaceColor, 1);
        pItem->pPrim->Draw();
        RedRenderer::pcSetCullMode(PCREDCULL_FRONT);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, s_DebugFrontFaceColor, 1);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
        if (s_bTwoSidedStencilTest)
        {
            RedRenderer::pcSetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
            RedRenderer::pcSetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR);
            RedRenderer::pcSetCullMode(PCREDCULL_NONE);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
            RedRenderer::pcSetCullMode(PCREDCULL_BACK);
            pItem->pPrim->Draw();
            RedRenderer::pcSetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
            RedRenderer::pcSetCullMode(PCREDCULL_FRONT);
        }
    }

    pItem->pPrim->Draw();

    if (s_bDebugShadowVolumes)
    {
        RedRenderer::pcSetCullMode(PCREDCULL_NONE);
        RedRenderer::pcSetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
        RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, s_DebugNearWireColor, 1);
        pItem->pPrim->Draw();
        RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, s_DebugFarWireColor, 1);
        pItem->pPrim->Draw();
        RedRenderer::pcSetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
}

static unsigned long ColorValueToARGB(const RedColorValue& c)
{
    return D3DCOLOR_ARGB((int)(c.a * 255.0f), (int)(c.r * 255.0f), (int)(c.g * 255.0f),
                         (int)(c.b * 255.0f));
}

// FUNCTION: SWBF2 0x007075a0
void pcStencilShadowShader::Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const
{
    if (s_bDebugShadowVolumes)
    {
        static const RedColorValue backColor(0.2f, 0.2f, 1.0f, 0.3f);
        static const RedColorValue frontColor(1.0f, 0.0f, 0.0f, 0.3f);
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                                      ColorValueToARGB(uiPass ? backColor : frontColor));
    }

    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();

    if (s_bDebugShadowVolumes)
    {
        static const RedColorValue visibleColor(1.0f, 0.0f, 1.0f, 0.3f);
        static const RedColorValue hiddenColor(0.8f, 0.2f, 0.8f, 0.1f);
        RedRenderer::pcSetCullMode(PCREDCULL_NONE);
        RedRenderer::pcSetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
        RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, ColorValueToARGB(visibleColor));
        pItem->pPrim->Draw();
        RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, ColorValueToARGB(hiddenColor));
        pItem->pPrim->Draw();
        RedRenderer::pcSetCullMode(uiPass ? PCREDCULL_BACK : PCREDCULL_FRONT);
        RedRenderer::pcSetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    }
}

// FUNCTION: SWBF2 0x00707850
void pcStencilShadowShader::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    const float fLength = pItem->fStencilShadowLength;

    const bool bDirectional =
        pItem->uiShaderState >= 11 ? uiPass == 0 : pItem->uiShaderState <= 5;
    if (bDirectional)
    {
        for (int i = 0; pItem->pLightingState->pDirectionalLights[i] && i < 2; i++)
        {
            const RedDirectionalLight* pLight = pItem->pLightingState->pDirectionalLights[i];
            if (!pLight->IsShadowCaster())
            {
                continue;
            }
            pcRedVertexShaderConstantRegister* pRegs =
                RedRenderer::pcLockVertexShaderConstants(40, 2);
            pRegs[40].SetFloats(pLight->m_vDirection.x, pLight->m_vDirection.y,
                                pLight->m_vDirection.z);
            pRegs[41].SetFloats(fLength, 0.0f, 0.0f);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
            RenderVolume(pItem, uiPass);
        }
        return;
    }

    for (int i = 0; pItem->pLightingState->pOmniLights[i] && i < 4; i++)
    {
        const RedOmniLight* pLight = pItem->pLightingState->pOmniLights[i];
        if (!pLight->IsShadowCaster())
        {
            continue;
        }
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 2);
        pRegs[40].SetFloats(pLight->m_pos.x, pLight->m_pos.y, pLight->m_pos.z);
        pRegs[40].component[3] = pLight->m_radius;
        pRegs[41].SetFloats(0.0f, 0.0f, 0.0f);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        RenderVolume(pItem, uiPass);
    }

    for (int i = 0; pItem->pLightingState->pSpotLights[i] && i < 1; i++)
    {
        const RedSpotLight* pLight = pItem->pLightingState->pSpotLights[i];
        if (!pLight->IsShadowCaster())
        {
            continue;
        }
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 2);
        pRegs[40].SetFloats(pLight->m_vPos.x, pLight->m_vPos.y, pLight->m_vPos.z);
        pRegs[40].component[3] = pLight->GetShadowRadius();
        pRegs[41].SetFloats(0.0f, 0.0f, 0.0f);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        RenderVolume(pItem, uiPass);
    }
}

// FUNCTION: SWBF2 0x00707c80
void pcStencilShadowShader::SetMode(MODE eMode)
{
    m_mode = eMode;
    if ((unsigned int)(eMode - MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDHARD) <= 3)
    {
        m_pVertexFormat = s_pSkinnedInputFormat[eMode - MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDHARD];
    }
}

// FUNCTION: SWBF2 0x00707ca0
pcRedShader* pcStencilShadowShader::_Create(unsigned int uiFlags,
                                            const unsigned int* pParams,
                                            unsigned int uiNumParams,
                                            unsigned int uiMaterialFlags,
                                            const RedColor& color0,
                                            const RedColor& color1,
                                            float fArg7) const
{
    return new pcStencilShadowShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                     color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00707d30
void pcStencilShadowShader::_Init(unsigned int uiFlags,
                                  const unsigned int*,
                                  unsigned int,
                                  unsigned int,
                                  const RedColor&,
                                  const RedColor&,
                                  float)
{
    if (uiFlags & 0x800)
    {
        m_mode = MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDHARD;
        m_pVertexFormat = s_pSkinnedInputFormat[0];
    }
    else
    {
        m_pVertexFormat = pcRedVertexFormat::Create(uiFlags);
        m_mode = MODE_GPU_EXTRUSION_VERTEXNORMAL_UNSKINNED;
    }
}

// FUNCTION: SWBF2 0x00707d70
void pcStencilShadowShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcStencilShadowShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcStencilShadowShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcStencilShadowShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcStencilShadowShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcStencilShadowShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcStencilShadowShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcStencilShadowShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcStencilShadowShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00707dd0
void pcStencilShadowShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00707eb0
void pcStencilShadowShader::_StaticInit() const
{
    s_pSkinnedInputFormat[0] =
        pcRedVertexFormat::Create(0x5800, s_hardskinned_facenormal_ShadowFormat);
    s_pSkinnedInputFormat[1] =
        pcRedVertexFormat::Create(0x5800, s_hardskinned_generatenormal_ShadowFormat);
    s_pSkinnedInputFormat[2] =
        pcRedVertexFormat::Create(0x5800, s_softskinned_facenormal_ShadowFormat);
    s_pSkinnedInputFormat[3] =
        pcRedVertexFormat::Create(0x5800, s_softskinned_generatenormal_ShadowFormat);
    s_bTwoSidedStencilTest =
        (RedRenderer::pcGetDeviceCaps().StencilCaps & D3DSTENCILCAPS_TWOSIDED) != 0;
}

// SYNTHETIC: SWBF2 0x00409ea0
// `dynamic initializer for 'pcStencilShadowShader::s_uiRenderTypeHash''
unsigned int pcStencilShadowShader::s_uiRenderTypeHash = PblHash("StencilShadow");

// SYNTHETIC: SWBF2 0x00409e60
// `dynamic initializer for 'pcStencilShadowShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769e10
// `dynamic atexit destructor for 'pcStencilShadowShader::s_Factory''
pcStencilShadowShader pcStencilShadowShader::s_Factory;

// SYNTHETIC: SWBF2 0x00409e80
// `dynamic initializer for 'pcStencilShadowShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769e30
// `dynamic atexit destructor for 'pcStencilShadowShader::s_shaderDef''
pcRedShaderDef pcStencilShadowShader::s_shaderDef("stencilshadow_shader.xml");

// SYNTHETIC: SWBF2 0x00409ee0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x00409ec0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// GLOBAL: SWBF2 0x009c9cd4
unsigned int g_pcStencilShadowShader;
