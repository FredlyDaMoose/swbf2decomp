#include <string.h>
#include <d3dx9math.h>

#include "pcWaterShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "PebbleFL/Common/PblMath.h"
#include <math.h>

void __cdecl PblTraceP(const char* Format, ...);

// TEMPLATE: SWBF2 0x006fe970
// float RedTextureTransform::operator()(int,int) const (pcWaterShader.obj copy)

class PblMatrix;
extern PblMatrix gMatrixIdentity;

unsigned int pcWaterShader::s_uiID;
pcRedShader::GetStateFuncT pcWaterShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcWaterShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcWaterShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcWaterShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9e60
// SYNTHETIC: SWBF2 0x00409fc0
// `dynamic initializer for 'pcWaterShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769e90
// `dynamic atexit destructor for 'pcWaterShader::s_shaderDef''
pcRedShaderDef pcWaterShader::s_shaderDef("water_shader.xml");

bool pcWaterShader::s_specularEnable = true;
PblVector3 pcWaterShader::s_vSpecularDirection(1.0f, 0.0f, 0.0f);
RedColorValue pcWaterShader::s_refractionColor(1.0f, 1.0f, 1.0f, 1.0f);
RedColorValue pcWaterShader::s_reflectionColor(1.0f, 1.0f, 1.0f, 1.0f);
RedColorValue pcWaterShader::s_fresnelMinMax(0.0f, 0.0f, 0.0f, 1.0f);

// GLOBAL: SWBF2 0x009ca094
// SYNTHETIC: SWBF2 0x0040a040
// `dynamic initializer for 'kuiCubeNormalizationMap''
static unsigned int kuiCubeNormalizationMap = PblHash("cube_normalizationmap");

// GLOBAL: SWBF2 0x009c9e8c
// SYNTHETIC: SWBF2 0x0040a060
// `dynamic initializer for 'kuiLinearRamp''
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// GLOBAL: SWBF2 0x009ca050
// SYNTHETIC: SWBF2 0x0040a080
// `dynamic initializer for '_IdentityMatrix''

static D3DXMATRIXA16 _IdentityMatrix;

// GLOBAL: SWBF2 0x009ca098
static D3DMATERIAL9 s_d3dMat;

// GLOBAL: SWBF2 0x009c9e24
bool s_projectedBumpEnv;

// FUNCTION: SWBF2 0x007085d0
pcWaterShader::pcWaterShader()
{
    m_normalMapTransform1.m_bTransform = 0;
    m_normalMapTransform2.m_bTransform = 0;
    m_bumpMapTransform1.m_bTransform = 0;
    m_bumpMapTransform2.m_bTransform = 0;
    m_diffuseMapTransform1.m_bTransform = 0;
    m_diffuseMapTransform2.m_bTransform = 0;
    m_specularMaskTransform1.m_bTransform = 0;
    m_specularMaskTransform2.m_bTransform = 0;

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

// FUNCTION: SWBF2 0x007086d0
pcWaterShader::pcWaterShader(unsigned int uiFlags,
                             const unsigned int* pParams,
                             unsigned int uiNumParams,
                             unsigned int uiMaterialFlags,
                             const RedColor& color0,
                             const RedColor& color1,
                             float fArg7)
{
    m_normalMapTransform1.m_bTransform = 0;
    m_normalMapTransform2.m_bTransform = 0;
    m_bumpMapTransform1.m_bTransform = 0;
    m_bumpMapTransform2.m_bTransform = 0;
    m_diffuseMapTransform1.m_bTransform = 0;
    m_diffuseMapTransform2.m_bTransform = 0;
    m_specularMaskTransform1.m_bTransform = 0;
    m_specularMaskTransform2.m_bTransform = 0;
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
    _Init(uiFlags, pParams, uiNumParams, uiMaterialFlags, color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x007087e0
void pcWaterShader::Begin_Fixed(unsigned int uiState, unsigned int uiPass) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    switch (uiState)
    {
    case 0:
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                      D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                          D3DCOLORWRITEENABLE_BLUE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 1:
    case 2:
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

        if (uiPass == 0)
        {
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
            RedRenderer::pcSetTexture(1, RedTexture::Find(kuiLinearRamp));
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX,
                                                D3DTSS_TCI_CAMERASPACEPOSITION);
            RedRenderer::pcFFSetTextureTransform(
                1, RedRenderer::pcGetNearSceneFadeTextureTransform());
            RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                          D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                              D3DCOLORWRITEENABLE_BLUE);
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        else if (uiPass == 1)
        {
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
            RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        else
        {
            RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
            RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                          D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                              D3DCOLORWRITEENABLE_BLUE);
            RedRenderer::pcSetRenderState(D3DRS_SPECULARENABLE, TRUE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00709600
void pcWaterShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00709620
void pcWaterShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00709630
void pcWaterShader::End_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetTexture(0, 0);
    RedRenderer::pcSetTexture(1, 0);
    RedRenderer::pcSetTexture(2, 0);
    RedRenderer::pcSetTexture(3, 0);
}

// FUNCTION: SWBF2 0x00709660
void pcWaterShader::End_Prog2(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00709670
unsigned int pcWaterShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00709690
RenderPhaseT pcWaterShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_POSTSHADOW;
}

// FUNCTION: SWBF2 0x007096a0
unsigned int pcWaterShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x007096b0
pcRedShaderDef* pcWaterShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x007096c0
unsigned int pcWaterShader::GetState_Fixed(const RenderItem*) const
{
    if (!RedRenderer::pcIsNearScene())
    {
        return 0;
    }
    if (s_specularEnable && RedRenderer::pcGetRenderOptions().waterQuality == QUALITY_HIGH)
    {
        return 2;
    }
    return 1;
}

// FUNCTION: SWBF2 0x00709700
unsigned int pcWaterShader::GetState_Prog1(const RenderItem*) const
{
    if (RedRenderer::pcGetRenderOptions().waterQuality == QUALITY_LOW)
    {
        return (s_specularEnable ? 0 : 1) + 8;
    }
    if (!m_projectedNormalMap || !RedRenderer::pcIsNearScene())
    {
        return (s_specularEnable ? 0 : 1) + 4;
    }
    if (!s_projectedBumpEnv ||
        RedRenderer::pcGetRenderOptions().waterQuality == QUALITY_MEDIUM)
    {
        return (s_specularEnable ? 0 : 1) + 2;
    }
    return s_specularEnable ? 0 : 1;
}

// FUNCTION: SWBF2 0x00709780
unsigned int pcWaterShader::GetState_Prog2(const RenderItem*) const
{
    return m_projectedNormalMap ? 3 : 5;
}

// GLOBAL: SWBF2 0x00784fac
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[3] =
{
    { 1, { 0x42, 0x00, 0x00, 0x00 } },
    { 1, { 0x41, 0x41, 0x00, 0x00 } },
    { 3, { 0x41, 0x41, 0x41, 0x00 } },
};

// FUNCTION: SWBF2 0x007097a0
void pcWaterShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x10, 3, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00709e90
void pcWaterShader::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    switch (pItem->uiShaderState)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        if (uiPass == 1)
        {
            pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(43, 2);
            pRegs[43].component[0] = m_bumpMapTransform1(0, 0);
            pRegs[43].component[1] = m_bumpMapTransform1(1, 0);
            pRegs[43].component[2] = 0.0f;
            pRegs[43].component[3] = m_bumpMapTransform1(2, 0);
            pRegs[44].component[0] = m_bumpMapTransform1(0, 1);
            pRegs[44].component[1] = m_bumpMapTransform1(1, 1);
            pRegs[44].component[2] = 0.0f;
            pRegs[44].component[3] = m_bumpMapTransform1(2, 1);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);

            RedRenderer::pcSetTexture(0, m_projectedNormalMap);
            RedRenderer::pcSetTexture(1, RedTexture::Find(kuiCubeNormalizationMap));
            RedRenderer::pcSetTexture(2, pItem->uiShaderState == 1 ? m_bumpMap1 : 0);
            RedRenderer::pcSetTexture(3, m_reflectionMap);
        }
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &s_refractionColor.r, 1);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &s_reflectionColor.r, 1);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(2, &s_fresnelMinMax.r, 1);
        break;

    case 4:
    case 5:
    case 6:
    case 7:
        if (uiPass == 1)
        {
            pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(43, 4);
            pRegs[43].component[0] = m_normalMapTransform1(0, 0);
            pRegs[43].component[1] = m_normalMapTransform1(1, 0);
            pRegs[43].component[2] = 0.0f;
            pRegs[43].component[3] = m_normalMapTransform1(2, 0);
            pRegs[44].component[0] = m_normalMapTransform1(0, 1);
            pRegs[44].component[1] = m_normalMapTransform1(1, 1);
            pRegs[44].component[2] = 0.0f;
            pRegs[44].component[3] = m_normalMapTransform1(2, 1);
            pRegs[45].component[0] = m_normalMapTransform2(0, 0);
            pRegs[45].component[1] = m_normalMapTransform2(1, 0);
            pRegs[45].component[2] = 0.0f;
            pRegs[45].component[3] = m_normalMapTransform2(2, 0);
            pRegs[46].component[0] = m_normalMapTransform2(0, 1);
            pRegs[46].component[1] = m_normalMapTransform2(1, 1);
            pRegs[46].component[2] = 0.0f;
            pRegs[46].component[3] = m_normalMapTransform2(2, 1);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);

            RedRenderer::pcSetTexture(0, m_normalMap1);
            RedRenderer::pcSetTexture(1, m_normalMap2);
            RedRenderer::pcSetTexture(2, RedTexture::Find(kuiCubeNormalizationMap));
            RedRenderer::pcSetTexture(3, m_reflectionMap);

            const float afBlend[4] = { m_normalMapBlend, m_normalMapBlend,
                                       m_normalMapBlend, m_normalMapBlend };
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(3, afBlend, 1);
        }
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &s_refractionColor.r, 1);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &s_reflectionColor.r, 1);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(2, &s_fresnelMinMax.r, 1);
        break;

    case 8:
    case 9:
    {
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(43, 8);
        pRegs[43].component[0] = m_diffuseMapTransform1(0, 0);
        pRegs[43].component[1] = m_diffuseMapTransform1(1, 0);
        pRegs[43].component[2] = 0.0f;
        pRegs[43].component[3] = m_diffuseMapTransform1(2, 0);
        pRegs[44].component[0] = m_diffuseMapTransform1(0, 1);
        pRegs[44].component[1] = m_diffuseMapTransform1(1, 1);
        pRegs[44].component[2] = 0.0f;
        pRegs[44].component[3] = m_diffuseMapTransform1(2, 1);
        pRegs[45].component[0] = m_diffuseMapTransform2(0, 0);
        pRegs[45].component[1] = m_diffuseMapTransform2(1, 0);
        pRegs[45].component[2] = 0.0f;
        pRegs[45].component[3] = m_diffuseMapTransform2(2, 0);
        pRegs[46].component[0] = m_diffuseMapTransform2(0, 1);
        pRegs[46].component[1] = m_diffuseMapTransform2(1, 1);
        pRegs[46].component[2] = 0.0f;
        pRegs[46].component[3] = m_diffuseMapTransform2(2, 1);
        pRegs[47].component[0] = m_specularMaskTransform1(0, 0);
        pRegs[47].component[1] = m_specularMaskTransform1(1, 0);
        pRegs[47].component[2] = 0.0f;
        pRegs[47].component[3] = m_specularMaskTransform1(2, 0);
        pRegs[48].component[0] = m_specularMaskTransform1(0, 1);
        pRegs[48].component[1] = m_specularMaskTransform1(1, 1);
        pRegs[48].component[2] = 0.0f;
        pRegs[48].component[3] = m_specularMaskTransform1(2, 1);
        pRegs[49].component[0] = m_specularMaskTransform2(0, 0);
        pRegs[49].component[1] = m_specularMaskTransform2(1, 0);
        pRegs[49].component[2] = 0.0f;
        pRegs[49].component[3] = m_specularMaskTransform2(2, 0);
        pRegs[50].component[0] = m_specularMaskTransform2(0, 1);
        pRegs[50].component[1] = m_specularMaskTransform2(1, 1);
        pRegs[50].component[2] = 0.0f;
        pRegs[50].component[3] = m_specularMaskTransform2(2, 1);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetTexture(0, 0);
        RedRenderer::pcSetTexture(1, m_diffuseMap2);
        RedRenderer::pcSetTexture(2, m_specularMask1);
        RedRenderer::pcSetTexture(3, m_specularMask2);

        const float afBlend[8] = { m_diffuseMapBlend, m_diffuseMapBlend,
                                   m_diffuseMapBlend, m_diffuseMapBlend,
                                   m_specularMaskBlend, m_specularMaskBlend,
                                   m_specularMaskBlend, m_specularMaskBlend };
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(3, afBlend, 2);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &s_refractionColor.r, 1);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &s_reflectionColor.r, 1);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(2, &s_fresnelMinMax.r, 1);
        break;
    }
    }

    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070a610
void pcWaterShader::Render_Prog2(const RenderItem* pItem, unsigned int uiPass) const
{
    if (pItem->uiShaderState == 3)
    {
        if (uiPass == 1)
        {
            pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 4);
            pRegs[40].component[0] = -s_vSpecularDirection.x;
            pRegs[40].component[1] = -s_vSpecularDirection.y;
            pRegs[40].component[2] = -s_vSpecularDirection.z;
            pRegs[40].component[3] = 0.5f;
            pRegs[41].component[0] = _IdentityMatrix._11;
            pRegs[41].component[1] = _IdentityMatrix._21;
            pRegs[41].component[2] = 0.0f;
            pRegs[41].component[3] = 1.0f;
            pRegs[42].component[0] = _IdentityMatrix._12;
            pRegs[42].component[1] = _IdentityMatrix._22;
            pRegs[42].component[2] = 0.0f;
            pRegs[42].component[3] = 1.0f;
            pRegs[43].component[0] = 0.0f;
            pRegs[43].component[1] = 0.5f;
            pRegs[43].component[2] = -0.5f;
            pRegs[43].component[3] = 0.0f;
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);

            RedRenderer::pcSetTexture(0, m_projectedNormalMap);
            RedRenderer::pcSetTexture(1, m_reflectionMap);

            const float afScale[4] = { 0.03f, 0.03f, 0.03f, 0.03f };
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(7, afScale, 1);
        }
    }
    else if (pItem->uiShaderState == 5 && uiPass == 1)
    {
        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 6);
        pRegs[40].component[0] = -s_vSpecularDirection.x;
        pRegs[40].component[1] = -s_vSpecularDirection.y;
        pRegs[40].component[2] = -s_vSpecularDirection.z;
        pRegs[40].component[3] = 32.0f;
        pRegs[41].component[0] = 0.0f;
        pRegs[41].component[1] = 0.5f;
        pRegs[41].component[2] = -0.5f;
        pRegs[41].component[3] = 0.0f;
        pRegs[42].component[0] = m_normalMapTransform1(0, 0);
        pRegs[42].component[1] = m_normalMapTransform1(1, 0);
        pRegs[42].component[2] = 0.0f;
        pRegs[42].component[3] = m_normalMapTransform1(2, 0);
        pRegs[43].component[0] = m_normalMapTransform1(0, 1);
        pRegs[43].component[1] = m_normalMapTransform1(1, 1);
        pRegs[43].component[2] = 0.0f;
        pRegs[43].component[3] = m_normalMapTransform1(2, 1);
        pRegs[44].component[0] = m_normalMapTransform2(0, 0);
        pRegs[44].component[1] = m_normalMapTransform2(1, 0);
        pRegs[44].component[2] = 0.0f;
        pRegs[44].component[3] = m_normalMapTransform2(2, 0);
        pRegs[45].component[0] = m_normalMapTransform2(0, 1);
        pRegs[45].component[1] = m_normalMapTransform2(1, 1);
        pRegs[45].component[2] = 0.0f;
        pRegs[45].component[3] = m_normalMapTransform2(2, 1);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetTexture(0, m_normalMap1);
        RedRenderer::pcSetTexture(1, m_normalMap2);
        RedRenderer::pcSetTexture(2, m_reflectionMap);
        RedRenderer::pcSetTexture(3, RedTexture::Find(kuiCubeNormalizationMap));

        const float afBlend[4] = { m_normalMapBlend, m_normalMapBlend,
                                   m_normalMapBlend, m_normalMapBlend };
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(7, afBlend, 1);
    }

    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &s_refractionColor.r, 1);
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &s_fresnelMinMax.r, 1);
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(4, &s_reflectionColor.r, 1);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070a9e0
void pcWaterShader::SetBumpMap(RedTexture* pTex1, RedTextureTransform* pTransform1,
                               RedTexture* pTex2, RedTextureTransform* pTransform2,
                               float fBlend)
{
    m_bumpMap1 = pTex1;
    if (pTransform1)
    {
        m_bumpMapTransform1.m_bTransform = pTransform1->m_bTransform;
        if (m_bumpMapTransform1.m_bTransform)
        {
            memcpy(m_bumpMapTransform1.m_Transform, pTransform1->m_Transform,
                   sizeof(m_bumpMapTransform1.m_Transform));
        }
    }
    else
    {
        m_bumpMapTransform1.m_bTransform = 0;
    }

    m_bumpMap2 = pTex2;
    if (pTransform2)
    {
        m_bumpMapTransform2.m_bTransform = pTransform2->m_bTransform;
        if (m_bumpMapTransform2.m_bTransform)
        {
            memcpy(m_bumpMapTransform2.m_Transform, pTransform2->m_Transform,
                   sizeof(m_bumpMapTransform2.m_Transform));
        }
    }
    else
    {
        m_bumpMapTransform2.m_bTransform = 0;
    }

    m_bumpMapBlend = fBlend;
}

// FUNCTION: SWBF2 0x0070aa80
void pcWaterShader::SetDiffuseMap(RedTexture* pTex1, RedTextureTransform* pTransform1,
                                  RedTexture* pTex2, RedTextureTransform* pTransform2,
                                  float fBlend)
{
    m_diffuseMap1 = pTex1;
    if (pTransform1)
    {
        m_diffuseMapTransform1.m_bTransform = pTransform1->m_bTransform;
        if (m_diffuseMapTransform1.m_bTransform)
        {
            memcpy(m_diffuseMapTransform1.m_Transform, pTransform1->m_Transform,
                   sizeof(m_diffuseMapTransform1.m_Transform));
        }
    }
    else
    {
        m_diffuseMapTransform1.m_bTransform = 0;
    }

    m_diffuseMap2 = pTex2;
    if (pTransform2)
    {
        m_diffuseMapTransform2.m_bTransform = pTransform2->m_bTransform;
        if (m_diffuseMapTransform2.m_bTransform)
        {
            memcpy(m_diffuseMapTransform2.m_Transform, pTransform2->m_Transform,
                   sizeof(m_diffuseMapTransform2.m_Transform));
        }
    }
    else
    {
        m_diffuseMapTransform2.m_bTransform = 0;
    }

    m_diffuseMapBlend = fBlend;
}

// FUNCTION: SWBF2 0x0070ab30
void pcWaterShader::SetNormalMap(RedTexture* pTex1, RedTextureTransform* pTransform1,
                                 RedTexture* pTex2, RedTextureTransform* pTransform2,
                                 float fBlend)
{
    m_normalMap1 = pTex1;
    if (pTransform1)
    {
        m_normalMapTransform1.m_bTransform = pTransform1->m_bTransform;
        if (m_normalMapTransform1.m_bTransform)
        {
            memcpy(m_normalMapTransform1.m_Transform, pTransform1->m_Transform,
                   sizeof(m_normalMapTransform1.m_Transform));
        }
    }
    else
    {
        m_normalMapTransform1.m_bTransform = 0;
    }

    m_normalMap2 = pTex2;
    if (pTransform2)
    {
        m_normalMapTransform2.m_bTransform = pTransform2->m_bTransform;
        if (m_normalMapTransform2.m_bTransform)
        {
            memcpy(m_normalMapTransform2.m_Transform, pTransform2->m_Transform,
                   sizeof(m_normalMapTransform2.m_Transform));
        }
    }
    else
    {
        m_normalMapTransform2.m_bTransform = 0;
    }

    m_normalMapBlend = fBlend;
}

// FUNCTION: SWBF2 0x0070abc0
void pcWaterShader::SetSpecularMask(RedTexture* pTex1, RedTextureTransform* pTransform1,
                                    RedTexture* pTex2, RedTextureTransform* pTransform2,
                                    float fBlend)
{
    m_specularMask1 = pTex1;
    if (pTransform1)
    {
        m_specularMaskTransform1.m_bTransform = pTransform1->m_bTransform;
        if (m_specularMaskTransform1.m_bTransform)
        {
            memcpy(m_specularMaskTransform1.m_Transform, pTransform1->m_Transform,
                   sizeof(m_specularMaskTransform1.m_Transform));
        }
    }
    else
    {
        m_specularMaskTransform1.m_bTransform = 0;
    }

    m_specularMask2 = pTex2;
    if (pTransform2)
    {
        m_specularMaskTransform2.m_bTransform = pTransform2->m_bTransform;
        if (m_specularMaskTransform2.m_bTransform)
        {
            memcpy(m_specularMaskTransform2.m_Transform, pTransform2->m_Transform,
                   sizeof(m_specularMaskTransform2.m_Transform));
        }
    }
    else
    {
        m_specularMaskTransform2.m_bTransform = 0;
    }

    m_specularMaskBlend = fBlend;
}

// FUNCTION: SWBF2 0x0070ac70
pcRedShader* pcWaterShader::_Create(unsigned int uiFlags,
                                    const unsigned int* pParams,
                                    unsigned int uiNumParams,
                                    unsigned int uiMaterialFlags,
                                    const RedColor& color0,
                                    const RedColor& color1,
                                    float fArg7) const
{
    return new pcWaterShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                             color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x0070ad00
void pcWaterShader::_Init(unsigned int uiFlags,
                          const unsigned int*,
                          unsigned int,
                          unsigned int,
                          const RedColor&,
                          const RedColor&,
                          float)
{
    m_normalMap1 = 0;
    m_normalMap2 = 0;
    m_normalMapTransform1.m_bTransform = 0;
    m_normalMapTransform2.m_bTransform = 0;
    m_bumpMap1 = 0;
    m_bumpMap2 = 0;
    m_bumpMapTransform1.m_bTransform = 0;
    m_bumpMapTransform2.m_bTransform = 0;
    m_specularMask1 = 0;
    m_specularMask2 = 0;
    m_specularMaskTransform1.m_bTransform = 0;
    m_specularMaskTransform2.m_bTransform = 0;
    m_diffuseMap1 = 0;
    m_diffuseMap2 = 0;
    m_diffuseMapTransform1.m_bTransform = 0;
    m_diffuseMapTransform2.m_bTransform = 0;
    m_reflectionMap = 0;
    m_projectedNormalMap = 0;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, 0x222);
}

// FUNCTION: SWBF2 0x0070adc0
void pcWaterShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcWaterShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcWaterShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcWaterShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcWaterShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcWaterShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcWaterShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcWaterShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcWaterShader::Render_Prog1);

    GetStateFuncArray[PIPELINE_PROG2] =
        static_cast<GetStateFuncT>(&pcWaterShader::GetState_Prog2);
    BeginFuncArray[PIPELINE_PROG2] =
        static_cast<BeginFuncT>(&pcWaterShader::Begin_Prog2);
    EndFuncArray[PIPELINE_PROG2] =
        static_cast<EndFuncT>(&pcWaterShader::End_Prog2);
    RenderFuncArray[PIPELINE_PROG2] =
        static_cast<RenderFuncT>(&pcWaterShader::Render_Prog2);
}

// FUNCTION: SWBF2 0x0070ae40
void pcWaterShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x0070af20
void pcWaterShader::_StaticInit() const
{
    static const RedColorValue kWhite(1.0f, 1.0f, 1.0f, 1.0f);
    static const RedColorValue kFresnel(0.0f, 0.0f, 0.0f, 1.0f);

    s_refractionColor = kWhite;
    s_reflectionColor = kWhite;
    s_vSpecularDirection = PblVector3(1.0f, 0.0f, 0.0f);
    s_fresnelMinMax = kFresnel;
    s_specularEnable = true;

    memset(&s_d3dMat, 0, sizeof(s_d3dMat));
    s_d3dMat.Power = 32.0f;
    s_d3dMat.Specular = *reinterpret_cast<const D3DCOLORVALUE*>(&RedColorValue::WHITE);

    s_projectedBumpEnv =
        !(RedRenderer::pcGetDeviceCaps().TextureCaps & D3DPTEXTURECAPS_NOPROJECTEDBUMPENV);
}

// GLOBAL: SWBF2 0x009ca090
// SYNTHETIC: SWBF2 0x00409fe0
// `dynamic initializer for 'pcWaterShader::s_uiRenderTypeHash''
unsigned int pcWaterShader::s_uiRenderTypeHash = PblHash("Water");

// SYNTHETIC: SWBF2 0x0040a000
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a020
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9f10
// SYNTHETIC: SWBF2 0x00409fa0
// `dynamic initializer for 'pcWaterShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769e70
// `dynamic atexit destructor for 'pcWaterShader::s_Factory''
pcWaterShader pcWaterShader::s_Factory;

// FUNCTION: SWBF2 0x007092a0
void pcWaterShader::Begin_Prog2(unsigned int uiState, unsigned int uiPass) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    if (uiPass == 0)
    {
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_MODULATE);
        return;
    }
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    if (uiState != 3 || uiPass != 1)
    {
        return;
    }

    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    const float fAngle = (float)atan2((double)mCamera.m[2][2], (double)mCamera.m[2][0]) - PblMath::fHalfPi;
    D3DXMATRIXA16 mRotation, mFacing, mScale, mScaled;
    D3DXMatrixIdentity(&mFacing);
    D3DXMatrixIdentity(&mScaled);
    D3DXMatrixMultiply(&mFacing, &mFacing, D3DXMatrixRotationZ(&mRotation, fAngle));
    D3DXMatrixMultiply(&mScaled, &mScaled, D3DXMatrixScaling(&mScale, 0.03f, 0.03f, 0.03f));
    D3DXMatrixMultiply(&_IdentityMatrix, &mScaled, &mFacing);
}

// FUNCTION: SWBF2 0x00708c00
void pcWaterShader::Begin_Prog1(unsigned int uiState, unsigned int uiPass) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, uiPass == 0);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                      D3DCOLORWRITEENABLE_BLUE);
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
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    bool bReflectionStages = false;
    switch (uiState)
    {
    case 0: case 1: case 4: case 5:
        if (uiPass == 0)
        {
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
            break;
        }
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        if (uiState == 1)
        {
            const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
            PblMatrix mFacing(gMatrixIdentity);
            PblMatrix mScaled(gMatrixIdentity);
            const float fAngle = (float)atan2((double)mCamera.m[2][2], (double)mCamera.m[2][0]) - PblMath::fHalfPi;
            D3DXMATRIXA16 mRotation, mScale;
            D3DXMatrixMultiply((D3DXMATRIX*)&mFacing, (D3DXMATRIX*)&mFacing, D3DXMatrixRotationZ(&mRotation, fAngle));
            D3DXMatrixMultiply((D3DXMATRIX*)&mScaled, (D3DXMATRIX*)&mScaled,
                               D3DXMatrixScaling(&mScale, 0.03f, 0.03f, 0.03f));
            D3DXMatrixMultiply(&_IdentityMatrix, (D3DXMATRIX*)&mScaled, (D3DXMATRIX*)&mFacing);

            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);

            RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            float fBump = _IdentityMatrix._11;
            RedRenderer::pcSetTextureStageState(3, D3DTSS_BUMPENVMAT00, *(DWORD*)&fBump);
            fBump = _IdentityMatrix._21;
            RedRenderer::pcSetTextureStageState(3, D3DTSS_BUMPENVMAT10, *(DWORD*)&fBump);
            fBump = _IdentityMatrix._12;
            RedRenderer::pcSetTextureStageState(3, D3DTSS_BUMPENVMAT01, *(DWORD*)&fBump);
            fBump = _IdentityMatrix._22;
            RedRenderer::pcSetTextureStageState(3, D3DTSS_BUMPENVMAT11, *(DWORD*)&fBump);
            break;
        }
        bReflectionStages = true;
        break;

    case 2: case 3: case 6: case 7:
        if (uiPass == 0)
        {
            RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
            RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
            break;
        }
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        if (uiState == 3)
        {
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            break;
        }
        bReflectionStages = true;
        break;

    case 8: case 9:
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        RedRenderer::pcSetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        break;
    }
    if (bReflectionStages)
    {
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
    }

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 3);
    pRegs[40].component[0] = 0.0f;
    pRegs[40].component[1] = 0.5f;
    pRegs[40].component[2] = -0.5f;
    pRegs[40].component[3] = 0.0f;
    const float fStart = FLRenderer::GetNearSceneFadeStart() * RedRenderer::GetCamera()->_fZoom;
    const float fEnd = FLRenderer::GetNearSceneFadeEnd() * RedRenderer::GetCamera()->_fZoom;
    bool bFade = false;
    if (fStart > 0.0f && fEnd > fStart)
    {
        const float fScale = 1.0f / (fEnd - fStart);
        if (RedRenderer::pcIsNearScene())
        {
            pRegs[41].component[2] = -fScale;
            pRegs[41].component[3] = fScale * fEnd;
            bFade = true;
        }
    }
    if (!bFade)
    {
        pRegs[41].component[3] = 1.0f;
        pRegs[41].component[2] = 0.0f;
    }
    pRegs[42].component[0] = -s_vSpecularDirection.x;
    pRegs[42].component[1] = -s_vSpecularDirection.y;
    pRegs[42].component[2] = -s_vSpecularDirection.z;
    pRegs[42].component[3] = 32.0f;
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
}

static PblMatrix WaterTextureMatrix(const RedTextureTransform& t)
{
    PblMatrix m;
    m.m[0][0] = t(0, 0); m.m[0][1] = t(0, 1); m.m[0][2] = 0.0f; m.m[0][3] = 0.0f;
    m.m[1][0] = t(1, 0); m.m[1][1] = t(1, 1); m.m[1][2] = 0.0f; m.m[1][3] = 0.0f;
    m.m[2][0] = t(2, 0); m.m[2][1] = t(2, 1); m.m[2][2] = 0.0f; m.m[2][3] = 0.0f;
    m.m[3][0] = 0.0f;    m.m[3][1] = 0.0f;    m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
    return m;
}

// FUNCTION: SWBF2 0x007097c0
void pcWaterShader::Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const
{
    RedColor refraction;
    refraction.r = (unsigned char)(int)(s_refractionColor.r * 255.0f);
    refraction.g = (unsigned char)(int)(s_refractionColor.g * 255.0f);
    refraction.b = (unsigned char)(int)(s_refractionColor.b * 255.0f);
    refraction.a = (unsigned char)(int)(s_refractionColor.a * 255.0f);
    const RedColor color = pItem->tweakColor * refraction;

    s_d3dMat.Diffuse.r = color.r * (1.0f / 255.0f);
    s_d3dMat.Diffuse.g = color.g * (1.0f / 255.0f);
    s_d3dMat.Diffuse.b = color.b * (1.0f / 255.0f);
    s_d3dMat.Diffuse.a = color.a * (1.0f / 255.0f);
    s_d3dMat.Ambient = s_d3dMat.Diffuse;
    RedRenderer::pcGetDevice()->SetMaterial(&s_d3dMat);

    if (uiPass == 1)
    {
        const PblMatrix m1 = WaterTextureMatrix(m_specularMaskTransform1);
        const PblMatrix m2 = WaterTextureMatrix(m_specularMaskTransform2);
        RedRenderer::pcSetTexture(0, m_specularMask1);
        RedRenderer::pcSetTexture(1, m_specularMask2);
        RedRenderer::pcFFSetTextureTransform(0, m1);
        RedRenderer::pcFFSetTextureTransform(1, m2);
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
            (unsigned int)(unsigned char)(int)(m_specularMaskBlend * 255.0f) << 24);
    }
    else
    {
        if (uiPass == 0)
        {
            const PblMatrix m = WaterTextureMatrix(m_diffuseMapTransform2);
            RedRenderer::pcSetTexture(0, m_diffuseMap2);
            RedRenderer::pcFFSetTextureTransform(0, m);
            const unsigned int uiBlend = (unsigned char)(int)(m_diffuseMapBlend * 255.0f);
            RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                (uiBlend << 24) | (uiBlend << 16) | (uiBlend << 8) | uiBlend);
        }

        const RedColorValue& ambient = RedLight::GetGlobalAmbientLight()->m_color;
        RedRenderer::pcSetRenderState(D3DRS_AMBIENT,
            D3DCOLOR_ARGB((unsigned char)(int)(ambient.a * 255.0f),
                          (unsigned char)(int)(ambient.r * 255.0f),
                          (unsigned char)(int)(ambient.g * 255.0f),
                          (unsigned char)(int)(ambient.b * 255.0f)));
        RedRenderer::pcGetDevice()->LightEnable(0, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_LIGHTING, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, FALSE);
    }

    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
    RedRenderer::pcGetDevice()->LightEnable(0, FALSE);
}

// GLOBAL: SWBF2 0x009c9e20
unsigned int g_pcWaterShader;
