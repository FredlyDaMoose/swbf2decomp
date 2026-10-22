#include "pcTerrainShader2.h"

#include <string.h>

#include "pcTerrainShader.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcTerrainShader2::s_uiID;
pcRedShader::GetStateFuncT pcTerrainShader2::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcTerrainShader2::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcTerrainShader2::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcTerrainShader2::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c8a8c
// SYNTHETIC: SWBF2 0x004095a0
// `dynamic initializer for 'pcTerrainShader2::s_uiRenderTypeHash''
unsigned int pcTerrainShader2::s_uiRenderTypeHash = PblHash("Terrain2");

// SYNTHETIC: SWBF2 0x004095c0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x004095e0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c8f80
// SYNTHETIC: SWBF2 0x00409580
// `dynamic initializer for 'pcTerrainShader2::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769c20
// `dynamic atexit destructor for 'pcTerrainShader2::s_shaderDef''
pcRedShaderDef pcTerrainShader2::s_shaderDef("normal_terrain_shader.xml");

// GLOBAL: SWBF2 0x007e1950
static float s_PipelineVersion = 1.1f;
// GLOBAL: SWBF2 0x007e1954
static bool s_Enabled = true;
// GLOBAL: SWBF2 0x007e1955
static bool bLightingEnable = true;

// GLOBAL: SWBF2 0x007e1956

static bool sUseHighTierLighting = true;
// GLOBAL: SWBF2 0x007e1958

static int sDetailTextureStageLimit = 8;

// GLOBAL: SWBF2 0x009c8970
static unsigned int uiBumpLuminosityTextureHash_;

// GLOBAL: SWBF2 0x009c89b0
static pcRedVertexFormat* s_pVertexInputFormat;
// GLOBAL: SWBF2 0x009c89b4
static pcRedVertexFormat* s_pPosVertexInputFormat;
// GLOBAL: SWBF2 0x009c89b8
static pcRedVertexFormat* s_pPosNormalVertexInputFormat;
// GLOBAL: SWBF2 0x009c89bc
static pcRedVertexFormat* s_pFarVertexInputFormat;
// GLOBAL: SWBF2 0x009c89c0
static pcRedVertexFormat* s_pFFLayer0VertexInputFormat;
// GLOBAL: SWBF2 0x009c89c4
static pcRedVertexFormat* s_pFFLayerNVertexInputFormat;
// GLOBAL: SWBF2 0x009c89c8
static RedTexture* s_noiseTextures[16];
// GLOBAL: SWBF2 0x009c8a08
static RedTexture* s_diffuseTextures[16];
// GLOBAL: SWBF2 0x009c8a48
static RedTexture* s_bumpTextures[16];
// GLOBAL: SWBF2 0x009c8a89
static bool sForceSimpleLighting;
// GLOBAL: SWBF2 0x009c8a8a
static bool sDrawNormals;

// GLOBAL: SWBF2 0x009c8a90
// SYNTHETIC: SWBF2 0x004096e0
// `dynamic initializer for 's_ShadowMapTextureTransform''

static PblVector4 s_ShadowMapTextureTransform[3];
// GLOBAL: SWBF2 0x009c8ac0
// SYNTHETIC: SWBF2 0x00409680
// `dynamic initializer for 's_DiffuseTexTransform''

static PblVector4 s_DiffuseTexTransform[16];
// GLOBAL: SWBF2 0x009c8bc0
// SYNTHETIC: SWBF2 0x00409640
// `dynamic initializer for 's_BumpTexTransform''

static PblVector4 s_BumpTexTransform[16];
// GLOBAL: SWBF2 0x009c8cd0
// SYNTHETIC: SWBF2 0x00409600
// `dynamic initializer for 's_BumpLuminosityTextureTransform''

static PblVector4 s_BumpLuminosityTextureTransform[2];

// GLOBAL: SWBF2 0x009c8fac
// SYNTHETIC: SWBF2 0x00409620
// `dynamic initializer for 's_BumpSpecularTextureTransform''

PblVector4 s_BumpSpecularTextureTransform[2];

// GLOBAL: SWBF2 0x009c8cf0
// SYNTHETIC: SWBF2 0x004096b0
// `dynamic initializer for 's_UnreferencedTextureTransformA''

PblVector4 s_UnreferencedTextureTransformA;
// GLOBAL: SWBF2 0x009c8cc0
// SYNTHETIC: SWBF2 0x004096a0
// `dynamic initializer for 's_UnreferencedTextureTransformB''

PblVector4 s_UnreferencedTextureTransformB;

// GLOBAL: SWBF2 0x009c8d00
// SYNTHETIC: SWBF2 0x00409660
// `dynamic initializer for 's_DetailTextureTransform''

static PblVector4 s_DetailTextureTransform[4][2];
// GLOBAL: SWBF2 0x009c8d80
// SYNTHETIC: SWBF2 0x00409700
// `dynamic initializer for 's_TextureTransform''

static PblVector4 s_TextureTransform[16][2];
// GLOBAL: SWBF2 0x009c8ff0
// SYNTHETIC: SWBF2 0x004096c0
// `dynamic initializer for 's_NoiseTexTransform''

static PblVector4 s_NoiseTexTransform[16];
// GLOBAL: SWBF2 0x009c90f8
static RedMaterial s_material;
// GLOBAL: SWBF2 0x009c91c4
static RedTexture* s_pDetailTexture;
// GLOBAL: SWBF2 0x009c91c8
static RedTexture* s_apTextures[16];
// GLOBAL: SWBF2 0x009c9208
static RedTexture* s_pShadowMapTexture;

// GLOBAL: SWBF2 0x009c920c
static RedTexture* s_pNormalizationCubeMap;
// GLOBAL: SWBF2 0x009c9210
static RedTexture* s_pLightVolumeMap;
// GLOBAL: SWBF2 0x009c9214
static RedTexture* s_pBumpLuminosityTexture;
// GLOBAL: SWBF2 0x009c9218
static float s_fDetailFadeDistance;
// GLOBAL: SWBF2 0x009c921c
static float s_fStaticBumpFactor;
// GLOBAL: SWBF2 0x009c9220
static unsigned int s_uiOptions;
// GLOBAL: SWBF2 0x009c9224
static unsigned int s_uiShadowMapWrapMode;
// GLOBAL: SWBF2 0x009c9228

static unsigned int s_ShaderDefResetWord;
// GLOBAL: SWBF2 0x009c922c
static RedTexture* s_pNullDetailMap;
// GLOBAL: SWBF2 0x009c9230
static RedTexture* s_pWhiteTexture;

static inline PblVector4 Splat(float f)
{
    PblVector4 v = { f, f, f, f };
    return v;
}

static inline const PblVector4 operator*(const PblVector4& a, const PblVector4& b)
{
    PblVector4 v = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
    return v;
}

static inline RedColorValue ToColorValue(const RedColor& c)
{
    RedColorValue v;
    v.r = c.r * (1.0f / 255.0f);
    v.g = c.g * (1.0f / 255.0f);
    v.b = c.b * (1.0f / 255.0f);
    v.a = c.a * (1.0f / 255.0f);
    return v;
}

static inline void SetVector(pcRedVertexShaderConstantRegister& reg, const PblVector4& v)
{
    reg.component[0] = v.x;
    reg.component[1] = v.y;
    reg.component[2] = v.z;
    reg.component[3] = v.w;
}

// FUNCTION: SWBF2 0x006fcb60
pcTerrainShader2::pcTerrainShader2()
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

// FUNCTION: SWBF2 0x006fcc20
pcTerrainShader2::pcTerrainShader2(unsigned int uiFlags,
                                   const unsigned int* pParams,
                                   unsigned int uiNumParams,
                                   unsigned int uiMaterialFlags,
                                   const RedColor& color0,
                                   const RedColor& color1,
                                   float fSpecularPower)
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
    _Init(uiFlags, pParams, uiNumParams, uiMaterialFlags, color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x006fcce0
void pcTerrainShader2::Begin_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006fccf0
void pcTerrainShader2::Begin_Prog1(unsigned int, unsigned int uiPass) const
{
    if (uiPass == 0)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
        for (int i = 0; i < 4; i++)
        {
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        }
    }
    else if (uiPass == 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        for (int i = 0; i < 2; i++)
        {
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        }

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(41, 4);
        SetVector(pRegs[41], s_DetailTextureTransform[0][0]);
        SetVector(pRegs[42], s_DetailTextureTransform[0][1]);
        SetVector(pRegs[43], s_DetailTextureTransform[1][0]);
        SetVector(pRegs[44], s_DetailTextureTransform[1][1]);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        RedRenderer::pcSetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);

        if (RedShadowMap::IsShadowMapEnabled())
        {
            RedRenderer::pcSetTexture(3, RedShadowMap::GetShadowMapTexture());
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSU, RedShadowMap::GetShadowMapWrapMode());
            RedRenderer::pcSetSamplerState(3, D3DSAMP_ADDRESSV, RedShadowMap::GetShadowMapWrapMode());
            RedRenderer::pcSetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
            RedRenderer::pcSetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        }
        else
        {
            RedRenderer::pcSetTexture(3, s_pWhiteTexture);
        }
    }

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 1);
    const RedColorValue white = ToColorValue(RedColor::WHITE);
    pRegs[39].component[0] = white.r;
    pRegs[39].component[1] = white.g;
    pRegs[39].component[2] = white.b;
    pRegs[39].component[3] = white.a;
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}

// FUNCTION: SWBF2 0x006fd010
void pcTerrainShader2::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x006fd030
RedShadingState* pcTerrainShader2::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x006fd040
void pcTerrainShader2::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006fd050
void pcTerrainShader2::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006fd060
unsigned int pcTerrainShader2::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x006fd080
RenderPhaseT pcTerrainShader2::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x006fd090
unsigned int pcTerrainShader2::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x006fd0a0
unsigned short pcTerrainShader2::GetShaderChain(pcRedShader** const ppOut,
                                                unsigned short,
                                                const RenderItem* pItem)
{
    if (bLightingEnable && s_PipelineVersion < 2.0f)
    {
        m_lightingInSeparatePass = false;
        const bool bSpecial = (pItem->flags & 0x10220010) != 0;
        sForceSimpleLighting = RedRenderer::pcGetRenderOptions().mGraphicsTier < 1;
        sUseHighTierLighting = RedRenderer::pcGetRenderOptions().mGraphicsTier > 1;
        if (sUseHighTierLighting && !sForceSimpleLighting && !bSpecial)
        {
            ppOut[0] = RedRenderer::pcGetSystemShader(SYSTEM_SHADER_PERPIXELDIFFUSELIGHTING);
            ppOut[1] = this;
            m_lightingInSeparatePass = true;
            return 2;
        }
        ppOut[0] = this;
        return 1;
    }
    ppOut[0] = this;
    m_lightingInSeparatePass = false;
    return 1;
}

// FUNCTION: SWBF2 0x006fd150
pcRedShaderDef* pcTerrainShader2::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x006fd160
unsigned int pcTerrainShader2::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x006fd170
unsigned int pcTerrainShader2::GetState_Prog1(const RenderItem* pItem) const
{
    const bool bShadow = RedShadowMap::IsShadowMapEnabled() && !(pItem->flags & 0x18020018);
    const bool bProjected = pItem->pLightingState->pProjectedLight != 0
                            && !m_lightingInSeparatePass
                            && !(pItem->flags & 0x18020010);
    if (sDrawNormals)
    {
        return 2;
    }
    if (!sForceSimpleLighting && bLightingEnable)
    {
        if (m_lightingInSeparatePass || bShadow || bProjected || !(pItem->flags & 0x20000))
        {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006fd210
unsigned int pcTerrainShader2::GetZPrepassState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x006fd220
void pcTerrainShader2::InitFixedFunctionShaderDef() const
{
}

// FUNCTION: SWBF2 0x006fd230
bool pcTerrainShader2::IsEnabled()
{
    s_Enabled = s_Enabled && RedRenderer::pcGetRenderCaps().pipeline != 0;
    return s_Enabled;
}

// FUNCTION: SWBF2 0x006fd250
void pcTerrainShader2::RenderZPrepass(const RenderItem* pItem) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_BACK);
    RedRenderer::pcLoadFormat(s_pPosVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x006fd280
void pcTerrainShader2::Render_Fixed(const RenderItem*, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006fd290
void pcTerrainShader2::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    const unsigned int uiState = pItem->uiShaderState;
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::SetMultiplicativeFog((uiState ? 2 : 1) + (m_lightingInSeparatePass ? 1 : 0));

    if (uiPass == 0)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, m_lightingInSeparatePass != 0);

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 9);
        pRegs[40].component[2] = 0.0f;
        pRegs[40].component[3] = 1.0f / 255.0f;
        if (uiState)
        {
            pRegs[40].component[0] = 0.0f;
            pRegs[40].component[1] = 1.0f;
        }
        else
        {
            pRegs[40].component[0] = 1.0f;
            pRegs[40].component[1] = 0.0f;
        }

        for (int i = 0; i < 3; i++)
        {
            const int iLayer = m_uiLayerIndex[i];
            if (iLayer >= 16)
            {
                RedRenderer::pcSetTexture(i, 0);
            }
            else
            {
                RedRenderer::pcSetTexture(i, s_apTextures[iLayer]);
                SetVector(pRegs[41 + 2 * i], s_TextureTransform[iLayer][0]);
                SetVector(pRegs[42 + 2 * i], s_TextureTransform[iLayer][1]);
            }
        }

        RedRenderer::pcSetTexture(3, sDetailTextureStageLimit >= 1 ? s_pDetailTexture : s_pNullDetailMap);
        SetVector(pRegs[47], s_DetailTextureTransform[2][0]);
        SetVector(pRegs[48], s_DetailTextureTransform[2][1]);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    }
    else if (uiPass == 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetTexture(0, sDetailTextureStageLimit >= 2 ? s_pDetailTexture : s_pNullDetailMap);
        RedRenderer::pcSetTexture(1, sDetailTextureStageLimit >= 3 ? s_pDetailTexture : s_pNullDetailMap);

        pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 1);
        if (m_lightingInSeparatePass)
        {
            const float fScale = RedRenderer::GetLightingScale();
            pRegs[40].component[0] = 0.0f;
            pRegs[40].component[1] = fScale;
        }
        else
        {
            pRegs[40].component[0] = 1.0f;
            pRegs[40].component[1] = 0.0f;
        }
        pRegs[40].component[3] = 0.0f;
        pRegs[40].component[2] = 0.0f;

        if (pItem->pLightingState && pItem->pLightingState->pProjectedLight)
        {
            RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSU,
                pItem->pLightingState->pProjectedLight->GetProjectionTextureWrapMode());
            RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSV,
                pItem->pLightingState->pProjectedLight->GetProjectionTextureWrapMode());
            RedRenderer::pcSetSamplerState(2, D3DSAMP_ADDRESSW,
                pItem->pLightingState->pProjectedLight->GetProjectionTextureWrapMode());
            RedRenderer::pcSetTexture(2, RedTexture::s_textureHashTable.Find(
                pItem->pLightingState->pProjectedLight->m_uiProjTexHash));

            RedColorValue color = pItem->pLightingState->pProjectedLight->m_color;
            color.a = (float)((pItem->pLightingState->pProjectedLight->m_uiFlags >> 1) & 1);
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(1, &color.r, 1);
        }
        else
        {
            RedRenderer::pcSetTexture(2, s_pWhiteTexture);
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    }

    RedRenderer::pcLoadFormat(s_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x006fd660
void pcTerrainShader2::SetEnabled(bool bEnabled)
{
    s_Enabled = bEnabled;
    IsEnabled();
}

// FUNCTION: SWBF2 0x006fd690
pcRedShader* pcTerrainShader2::_Create(unsigned int uiFlags,
                                       const unsigned int* pParams,
                                       unsigned int uiNumParams,
                                       unsigned int uiMaterialFlags,
                                       const RedColor& color0,
                                       const RedColor& color1,
                                       float fSpecularPower) const
{
    return new pcTerrainShader2(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x006fd720
void pcTerrainShader2::_Init(unsigned int uiFlags,
                             const unsigned int* pParams,
                             unsigned int uiNumParams,
                             unsigned int,
                             const RedColor& color0,
                             const RedColor& color1,
                             float fSpecularPower)
{
    m_uiOptions = 0;
    switch (uiFlags & 0xffff0fff)
    {
    case 0x122:
        if (s_pVertexInputFormat == 0)
        {
            s_pVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 0x122);
            s_pPosVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 2);
            s_pPosNormalVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 0x22);
        }
        if (!(uiFlags & 0xf000) && s_pFFLayer0VertexInputFormat == 0)
        {
            s_pFFLayer0VertexInputFormat = pcRedVertexFormat::Create(0x122, 0x102);
        }
        break;
    case 0x82:
        if (!(uiFlags & 0xf000) && s_pFFLayerNVertexInputFormat == 0)
        {
            s_pFFLayerNVertexInputFormat = pcRedVertexFormat::Create(0x82, 0x82);
        }
        break;
    case 2:
        if (s_pFarVertexInputFormat == 0)
        {
            s_pFarVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 2);
        }
        break;
    }

    m_lightingInSeparatePass = false;
    for (unsigned int i = 0; i < 4; i++)
    {
        m_uiLayerIndex[i] = 0xff;
        if (pParams && i < uiNumParams)
        {
            m_uiLayerIndex[i] = (unsigned char)pParams[i];
        }
    }
    m_uiOptions = 0;

    s_pNullDetailMap = RedTexture::s_textureHashTable.Find(PblHash("null_detailmap"));
    s_pWhiteTexture = RedTexture::s_textureHashTable.Find(PblHash("white"));
    if (s_pDetailTexture == 0)
    {
        s_pDetailTexture = s_pNullDetailMap;
    }

    s_material.diffuseColor = ToColorValue(color0);
    s_material.specularColor = ToColorValue(color1);
    s_material.specularPower = fSpecularPower;
    m_material = &s_material;
}

// FUNCTION: SWBF2 0x006fd960
void pcTerrainShader2::_InitBumpLuminosityPixelLighting(unsigned int uiHash,
                                                        unsigned int uiBumpLuminosityHash,
                                                        const PblVector4& vScale0,
                                                        const PblVector4& vScale1,
                                                        float fStaticBumpFactor)
{
    if (!IsEnabled())
    {
        pcTerrainShader::_InitBumpLuminosityPixelLighting(uiHash, uiBumpLuminosityHash,
                                                          vScale0, vScale1, fStaticBumpFactor);
    }

    uiBumpLuminosityTextureHash_ = uiBumpLuminosityHash;
    s_pLightVolumeMap = RedTexture::s_textureHashTable.Find(uiHash);
    s_pNormalizationCubeMap = 0;
    s_pBumpLuminosityTexture = RedTexture::s_textureHashTable.Find(uiBumpLuminosityHash);
    s_uiOptions |= 0x40000;
    s_BumpLuminosityTextureTransform[0] = vScale0;
    s_BumpLuminosityTextureTransform[1] = vScale1;
    s_fStaticBumpFactor = fStaticBumpFactor;
    s_material.materialFlags |= 0x10000020;

    if (uiBumpLuminosityTextureHash_)
    {
        s_material.bumpMapHash = uiBumpLuminosityTextureHash_;
        s_material.materialFlags |= 0x10000020;
        s_material.detailTexTransform.SetScale(s_DetailTextureTransform[0][0].x,
                                               s_DetailTextureTransform[0][1].z);
        s_material.diffuseTexTransform.SetScale(s_DetailTextureTransform[0][0].x,
                                                s_DetailTextureTransform[0][1].z);
    }
}

// FUNCTION: SWBF2 0x006fdbb0
void pcTerrainShader2::_InitDetail(unsigned int uiHash,
                                   const PblVector4& vScale0,
                                   const PblVector4& vScale1,
                                   float fFadeDistance)
{
    if (!IsEnabled())
    {
        pcTerrainShader::_InitDetail(uiHash, vScale0, vScale1, fFadeDistance);
    }

    s_pDetailTexture = uiHash ? RedTexture::s_textureHashTable.Find(uiHash) : s_pNullDetailMap;
    s_DetailTextureTransform[2][0] = vScale0 * Splat(0.91f);
    s_DetailTextureTransform[2][1] = vScale1 * Splat(0.91f);
    s_DetailTextureTransform[0][0] = vScale0 * Splat(1.72f);
    s_DetailTextureTransform[0][1] = vScale1 * Splat(1.72f);
    s_DetailTextureTransform[1][0] = vScale0 * Splat(3.2f);
    s_DetailTextureTransform[1][1] = vScale1 * Splat(3.2f);
    s_fDetailFadeDistance = fFadeDistance;
}

// FUNCTION: SWBF2 0x006fdcc0
void pcTerrainShader2::_InitDiffuseMap(int iIndex, unsigned int uiHash, const PblVector4& vScale)
{
    s_diffuseTextures[iIndex] = uiHash ? RedTexture::s_textureHashTable.Find(uiHash) : 0;
    const float fHalf = RedTerrain::_fMetersPerPoint * 0.5f;
    PblVector4 v;
    v.x = vScale.x;
    v.y = vScale.y;
    v.z = fHalf - 0.5f + 0.0f;
    v.w = -0.5f - fHalf + 0.0f;
    s_DiffuseTexTransform[iIndex] = v;
}

// FUNCTION: SWBF2 0x006fdd60
void pcTerrainShader2::_InitLayer(int iIndex, unsigned int uiHash,
                                  const PblVector4& vScale0, const PblVector4& vScale1)
{
    if (!IsEnabled())
    {
        pcTerrainShader::_InitLayer(iIndex, uiHash, vScale0, vScale1);
    }

    s_apTextures[iIndex] = uiHash ? RedTexture::s_textureHashTable.Find(uiHash) : 0;
    s_TextureTransform[iIndex][0] = vScale0;
    s_TextureTransform[iIndex][1] = vScale1;
}

// FUNCTION: SWBF2 0x006fddd0
void pcTerrainShader2::_InitMaps(float fScale)
{
    if (!RedTexture::s_textureHashTable.Find(PblHash("height0")))
    {
        s_PipelineVersion = 1.1f;
    }
    if (!RedTexture::s_textureHashTable.Find(PblHash("diffuse0")))
    {
        s_PipelineVersion = 1.1f;
    }
    if (!RedTexture::s_textureHashTable.Find(PblHash("fractal0")))
    {
        s_PipelineVersion = 1.1f;
        return;
    }
    if (s_PipelineVersion < 2.0f)
    {
        return;
    }

    const PblVector4 vZero = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float fInv = 1.0f / fScale;
    PblVector4 vScale = { fInv, fInv, 0.0f, 0.0f };
    _InitNoiseMap(0, PblHash("height0"), vScale, vZero);
    _InitNoiseMap(1, PblHash("terrain_amb"), vScale, vZero);
    _InitNoiseMap(3, PblHash("fractal0"), vScale, vZero);
    vScale.x = vScale.y = 1.1f / fScale;
    _InitNoiseMap(4, PblHash("noise0"), vScale, vZero);
    vScale.x = vScale.y = 2.0f / fScale;
    _InitNoiseMap(5, PblHash("noise0"), vScale, vZero);
    vScale.x = vScale.y = 3.9f / fScale;
    _InitNoiseMap(6, PblHash("noise0"), vScale, vZero);

    vScale.x = vScale.y = 99.0f / fScale;
    _InitDiffuseMap(0, PblHash("diffuse0"), vScale);
    _InitDiffuseMap(1, PblHash("diffuse1"), vScale);
    _InitDiffuseMap(2, PblHash("diffuse2"), vScale);
    _InitDiffuseMap(3, PblHash("diffuse3"), vScale);
}

// FUNCTION: SWBF2 0x006fe140
void pcTerrainShader2::_InitNoiseMap(int iIndex, unsigned int uiHash,
                                     const PblVector4& vScale, const PblVector4& vOffset)
{
    RedTexture* pTexture = uiHash ? RedTexture::s_textureHashTable.Find(uiHash) : 0;
    s_noiseTextures[iIndex] = pTexture;

    float fWidth = 1.0f;
    float fHeight = 1.0f;
    if (pTexture)
    {
        fWidth = (float)pTexture->m_uiWidth;
        fHeight = (float)pTexture->m_uiHeight;
    }
    float fBaseWidth = fWidth;
    float fBaseHeight = fHeight;
    if (s_noiseTextures[0])
    {
        fBaseWidth = (float)s_noiseTextures[0]->m_uiWidth;
        fBaseHeight = (float)s_noiseTextures[0]->m_uiHeight;
    }

    PblVector4 v;
    v.x = fBaseWidth / fWidth * vScale.x;
    v.y = fBaseHeight / fHeight * vScale.y;
    v.z = vOffset.x / fWidth + 0.5f + 0.0f;
    v.w = vOffset.y / fHeight - 0.5f + 0.0f;
    s_NoiseTexTransform[iIndex] = v;
}

// FUNCTION: SWBF2 0x006fe270
void pcTerrainShader2::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcTerrainShader2::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcTerrainShader2::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcTerrainShader2::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcTerrainShader2::Render_Fixed);
    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcTerrainShader2::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcTerrainShader2::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcTerrainShader2::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcTerrainShader2::Render_Prog1);
}

// FUNCTION: SWBF2 0x006fe2d0
void pcTerrainShader2::_InitPixelLighting(unsigned int uiHashA, unsigned int uiHashB)
{
    if (!IsEnabled())
    {
        pcTerrainShader::_InitPixelLighting(uiHashA, uiHashB);
        return;
    }
    if (RedRenderer::pcGetRenderCaps().cubemaps && RedRenderer::pcGetRenderCaps().volumemaps)
    {
        s_material.materialFlags |= 0x10000020;
        s_pLightVolumeMap = RedTexture::s_textureHashTable.Find(uiHashA);
        s_pNormalizationCubeMap = RedTexture::s_textureHashTable.Find(uiHashB);
    }
}

// FUNCTION: SWBF2 0x006fe340
void pcTerrainShader2::_InitShadowMap(RedTexture* pTexture, unsigned int uiWrapMode,
                                      const PblVector4& vU, const PblVector4& vV,
                                      const PblVector4& vW)
{
    if (!IsEnabled())
    {
        pcTerrainShader::_InitShadowMap(pTexture, uiWrapMode, vU, vV, vW);
        return;
    }
    s_pShadowMapTexture = pTexture;
    s_uiShadowMapWrapMode = uiWrapMode;
}

// FUNCTION: SWBF2 0x006fe370
void pcTerrainShader2::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x006fe450
void pcTerrainShader2::_StaticInit() const
{
    s_pVertexInputFormat = 0;
    s_pPosVertexInputFormat = 0;
    s_pPosNormalVertexInputFormat = 0;
    s_pFarVertexInputFormat = 0;
    s_pFFLayer0VertexInputFormat = 0;
    s_pFFLayerNVertexInputFormat = 0;
    memset(s_noiseTextures, 0, sizeof(s_noiseTextures));
    memset(s_diffuseTextures, 0, sizeof(s_diffuseTextures));
    memset(s_bumpTextures, 0, sizeof(s_bumpTextures));
    memset(s_NoiseTexTransform, 0, sizeof(s_NoiseTexTransform));
    memset(s_DiffuseTexTransform, 0, sizeof(s_DiffuseTexTransform));
    memset(s_BumpTexTransform, 0, sizeof(s_BumpTexTransform));
    memset(s_DetailTextureTransform, 0, sizeof(s_DetailTextureTransform));
    memset(s_ShadowMapTextureTransform, 0, sizeof(s_ShadowMapTextureTransform));
    memset(s_TextureTransform, 0, sizeof(s_TextureTransform));
    memset(s_apTextures, 0, sizeof(s_apTextures));
    s_pDetailTexture = 0;
    s_pShadowMapTexture = 0;
    s_pNormalizationCubeMap = 0;
    s_pLightVolumeMap = 0;
    s_fDetailFadeDistance = 32.0f;
    s_fStaticBumpFactor = 0.5f;
    s_uiOptions = 0;
    s_uiShadowMapWrapMode = 1;
    s_ShaderDefResetWord = 0;
    memset(&s_material, 0, sizeof(s_material));
    s_material.materialFlags = 0;
    s_material.diffuseTexTransform.m_bTransform = 0;
    s_material.detailTexTransform.m_bTransform = 0;
    s_material.animationData = 0;
}

// GLOBAL: SWBF2 0x009c8fcc
// SYNTHETIC: SWBF2 0x00409560
// `dynamic initializer for 'pcTerrainShader2::s_Factory''
// SYNTHETIC: SWBF2 0x00769c00
// `dynamic atexit destructor for 'pcTerrainShader2::s_Factory''
pcTerrainShader2 pcTerrainShader2::s_Factory;

// GLOBAL: SWBF2 0x009c89a8
unsigned int g_pcTerrainShader2;

// GLOBAL: SWBF2 0x009c9170
// SYNTHETIC: SWBF2 0x00409720
// `dynamic initializer for 's_mProjectionRestore''

static PblMatrix s_mProjectionRestore;
