#include "pcTerrainShader.h"

#include <string.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/RedCamera.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcTerrainShader::s_uiID;
pcRedShader::GetStateFuncT pcTerrainShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcTerrainShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcTerrainShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcTerrainShader::RenderFuncArray[MAX_PIPELINES];
pcRedVertexFormat* pcTerrainShader::s_pVertexInputFormat;
pcRedVertexFormat* pcTerrainShader::s_pPosVertexInputFormat;
pcRedVertexFormat* pcTerrainShader::s_pPosNormalVertexInputFormat;
pcRedVertexFormat* pcTerrainShader::s_pFarVertexInputFormat;
pcRedVertexFormat* pcTerrainShader::s_pFFLayer0VertexInputFormat;
pcRedVertexFormat* pcTerrainShader::s_pFFLayerNVertexInputFormat;
RedTexture* pcTerrainShader::s_apTextures[16];
RedTexture* pcTerrainShader::s_pDetailTexture;
RedTexture* pcTerrainShader::s_pBumpSpecularTexture;
RedTexture* pcTerrainShader::s_pShadowMapTexture;
RedTexture* pcTerrainShader::s_pNormalizationCubeMap;
RedTexture* pcTerrainShader::s_pLightVolumeMap;
RedTexture* pcTerrainShader::s_pBumpLuminosityTexture;
RedTexture* pcTerrainShader::s_pFarTexture;
RedTexture* pcTerrainShader::s_pFarDetailTexture;
float pcTerrainShader::s_fDetailFadeDistance;
float pcTerrainShader::s_fStaticBumpFactor;
unsigned int pcTerrainShader::s_uiOptions;
float pcTerrainShader::s_fLightningValue;

// SYNTHETIC: SWBF2 0x00409310
// `dynamic initializer for 'pcTerrainShader::s_BumpSpecularTextureTransform''

PblVector4 pcTerrainShader::s_BumpSpecularTextureTransform[2];

// SYNTHETIC: SWBF2 0x004092f0
// `dynamic initializer for 'pcTerrainShader::s_BumpLuminosityTextureTransform''

PblVector4 pcTerrainShader::s_BumpLuminosityTextureTransform[2];

// SYNTHETIC: SWBF2 0x00409380
// `dynamic initializer for 'pcTerrainShader::s_FarTextureTransform''

PblVector4 pcTerrainShader::s_FarTextureTransform;

// SYNTHETIC: SWBF2 0x00409390
// `dynamic initializer for 'pcTerrainShader::s_TextureTransform''

PblVector4 pcTerrainShader::s_TextureTransform[16][2];

// SYNTHETIC: SWBF2 0x00409370
// `dynamic initializer for 'pcTerrainShader::s_FarDetailTextureTransform''

PblVector4 pcTerrainShader::s_FarDetailTextureTransform;

// SYNTHETIC: SWBF2 0x004093b0
// `dynamic initializer for 'pcTerrainShader::s_mProjectionRestore''

PblMatrix pcTerrainShader::s_mProjectionRestore;

// GLOBAL: SWBF2 0x009c88d0
// SYNTHETIC: SWBF2 0x004093e0
// `dynamic initializer for 'pcTerrainShader::s_uiRenderTypeHash''
unsigned int pcTerrainShader::s_uiRenderTypeHash = PblHash("Terrain");

// SYNTHETIC: SWBF2 0x00409330
// `dynamic initializer for 'pcTerrainShader::s_DetailTextureTransform''

PblVector4 pcTerrainShader::s_DetailTextureTransform[2];
unsigned int pcTerrainShader::s_uiShadowMapWrapMode = 1;

// GLOBAL: SWBF2 0x007e1790
static float kfMainTextureMipBias = -0.9f;
// GLOBAL: SWBF2 0x007e1794
static float kfDetailTextureMipBias = -0.9f;

// GLOBAL: SWBF2 0x009c88f8
static D3DMATERIAL9 s_d3dMat;

static const float kInv255 = 1.0f / 255.0f;

static inline RedColorValue ToColorValue(const RedColor& c)
{
    RedColorValue v;
    v.r = c.r * kInv255;
    v.g = c.g * kInv255;
    v.b = c.b * kInv255;
    v.a = c.a * kInv255;
    return v;
}

static inline void SetColor(pcRedVertexShaderConstantRegister& reg, const RedColorValue& c)
{
    reg.component[0] = c.r;
    reg.component[1] = c.g;
    reg.component[2] = c.b;
    reg.component[3] = c.a;
}

static inline void SetVector(pcRedVertexShaderConstantRegister& reg, const PblVector4& v)
{
    reg.component[0] = v.x;
    reg.component[1] = v.y;
    reg.component[2] = v.z;
    reg.component[3] = v.w;
}

static inline void Set(pcRedVertexShaderConstantRegister& reg, float x, float y, float z, float w)
{
    reg.component[0] = x;
    reg.component[1] = y;
    reg.component[2] = z;
    reg.component[3] = w;
}

static inline void SetWrapLinear(unsigned int uiStage)
{
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

static inline void SetMipBias(unsigned int uiStage, const float& fBias)
{
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MIPMAPLODBIAS, *(const unsigned int*)&fBias);
}

static inline void SetShadowMapStage(unsigned int uiStage)
{
    RedRenderer::pcSetTexture(uiStage, pcTerrainShader::s_pShadowMapTexture);
    RedRenderer::pcSetTextureStageState(uiStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_ADDRESSU, pcTerrainShader::s_uiShadowMapWrapMode);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_ADDRESSV, pcTerrainShader::s_uiShadowMapWrapMode);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(uiStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

static inline void SetModulate2XBlend()
{
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_MODULATE2X);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
}

// FUNCTION: SWBF2 0x006f8910
pcTerrainShader::pcTerrainShader()
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

// FUNCTION: SWBF2 0x006f89d0
pcTerrainShader::pcTerrainShader(unsigned int uiFlags,
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

// GLOBAL: SWBF2 0x009c8674
// SYNTHETIC: SWBF2 0x00409440
// `dynamic initializer for 'kuiLinearRamp''
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

static PblMatrix AxesMatrix(float rx, float ry, float rz, float ux, float uy, float uz,
                            float fx, float fy, float fz, float ox, float oy, float oz)
{
    PblMatrix m;
    m.m[0][0] = rx; m.m[0][1] = ry; m.m[0][2] = rz; m.m[0][3] = 0.0f;
    m.m[1][0] = ux; m.m[1][1] = uy; m.m[1][2] = uz; m.m[1][3] = 0.0f;
    m.m[2][0] = fx; m.m[2][1] = fy; m.m[2][2] = fz; m.m[2][3] = 0.0f;
    m.m[3][0] = ox; m.m[3][1] = oy; m.m[3][2] = oz; m.m[3][3] = 1.0f;
    return m;
}

static PblMatrix CameraToWorld()
{
    PblMatrix m = RedRenderer::GetCamera()->GetMatrix();
    m.m[2][0] = -m.m[2][0];
    m.m[2][1] = -m.m[2][1];
    m.m[2][2] = -m.m[2][2];
    m.m[2][3] = 0.0f;
    return m;
}

static PblMatrix XZTextureMatrix(const PblVector4& v)
{
    return AxesMatrix(v.x, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, v.y, 0.0f, v.z, v.w, 0.0f);
}

static void SetCameraSpaceTransform(unsigned int uiStage, const PblMatrix& mWorldToTexture)
{
    const PblMatrix mCamera = CameraToWorld();
    PblMatrix mTexture;
    D3DXMatrixMultiply(&mTexture, &mCamera, &mWorldToTexture);
    RedRenderer::pcFFSetTextureTransform(uiStage, mTexture);
}

static void SetVertexColorWhiteMaterial()
{
    RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
    s_d3dMat.Diffuse = *(const D3DCOLORVALUE*)&RedColorValue::WHITE;
    s_d3dMat.Ambient = *(const D3DCOLORVALUE*)&RedColorValue::WHITE;
    s_d3dMat.Emissive.r = 0.0f;
    s_d3dMat.Emissive.g = 0.0f;
    s_d3dMat.Emissive.b = 0.0f;
    s_d3dMat.Emissive.a = 0.0f;
    RedRenderer::pcGetDevice()->SetMaterial(&s_d3dMat);
}

// FUNCTION: SWBF2 0x006f8a90
void pcTerrainShader::Begin_Fixed(unsigned int uiState, unsigned int uiPass) const
{
    s_pShadowMapTexture = RedShadowMap::GetShadowMapTexture();
    s_uiShadowMapWrapMode = RedShadowMap::GetShadowMapWrapMode();

    if (uiState <= 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetTexture(0, s_pFarTexture);
        SetWrapLinear(0);
        SetCameraSpaceTransform(0, XZTextureMatrix(s_FarTextureTransform));
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

        if (uiState == 1)
        {
            RedRenderer::pcSetTexture(1, s_pFarDetailTexture);
            SetWrapLinear(1);
            SetCameraSpaceTransform(1, XZTextureMatrix(s_FarDetailTextureTransform));
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        else
        {
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        RedRenderer::pcLoadFormat(s_pFarVertexInputFormat);
        return;
    }

    if (uiState >= 6)
    {
        RedRenderer::pcGetDevice()->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&s_mProjectionRestore);
        PblMatrix mProjection = s_mProjectionRestore;
        mProjection.m[2][2] = s_mProjectionRestore.m[2][2] - 6.1035164e-05f;
        RedRenderer::pcGetDevice()->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)&mProjection);

        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        SetVertexColorWhiteMaterial();

        SetWrapLinear(0);
        SetMipBias(0, kfMainTextureMipBias);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        RedRenderer::pcLoadFormat(s_pFFLayerNVertexInputFormat);
        return;
    }

    if (uiPass == 0)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        SetVertexColorWhiteMaterial();
        for (unsigned int i = 0; i < 2; i++)
        {
            SetWrapLinear(i);
            SetMipBias(i, kfMainTextureMipBias);
            RedRenderer::pcSetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            RedRenderer::pcSetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        }
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        RedRenderer::pcLoadFormat(s_pVertexInputFormat);
        return;
    }

    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);

    if (uiPass == 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_MODULATE2X);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
        RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
        RedRenderer::pcSetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1);

        RedRenderer::pcSetTexture(0, RedTexture::Find(kuiLinearRamp));
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        RedRenderer::pcFFSetTextureTransform(0, RedRenderer::pcGetNearSceneFadeTextureTransform());
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0x80ffffff);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

        if (uiState == 4 || uiState == 5)
        {
            RedRenderer::pcSetTexture(1, s_pShadowMapTexture);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, s_uiShadowMapWrapMode);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, s_uiShadowMapWrapMode);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
            SetMipBias(1, kfMainTextureMipBias);
            const PblVector4& vU = RedShadowMap::_vShadowMapTransformU;
            const PblVector4& vV = RedShadowMap::_vShadowMapTransformV;
            const PblVector4& vW = RedShadowMap::_vShadowMapTransformW;
            SetCameraSpaceTransform(1, AxesMatrix(vU.x, vV.x, vW.x, vU.y, vV.y, vW.y,
                                                  vU.z, vV.z, vW.z, vU.w, vV.w, vW.w));
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,
                                                D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);

            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        else
        {
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        RedRenderer::pcLoadFormat(s_pVertexInputFormat);
        return;
    }

    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
    RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_MODULATE2X);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetTexture(0, s_pDetailTexture);
    SetWrapLinear(0);
    SetMipBias(0, kfDetailTextureMipBias);
    const PblVector4& vT0 = s_DetailTextureTransform[0];
    const PblVector4& vT1 = s_DetailTextureTransform[1];
    SetCameraSpaceTransform(0, AxesMatrix(vT0.x, vT1.x, 0.0f, vT0.y, vT1.y, 0.0f,
                                          vT0.z, vT1.z, 0.0f, vT0.w, vT1.w, 0.0f));
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    RedRenderer::pcSetTexture(1, RedTexture::Find(kuiLinearRamp));
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    const float fHalf = s_fDetailFadeDistance * 0.5f;
    const float fScale = 1.0f / (s_fDetailFadeDistance - fHalf);
    const PblMatrix mFade = AxesMatrix(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                       fScale, 0.0f, 0.0f, -(fScale * fHalf), 0.0f, 0.0f);
    RedRenderer::pcFFSetTextureTransform(1, mFade);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0x808080);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    RedRenderer::pcLoadFormat(s_pVertexInputFormat);
}

// FUNCTION: SWBF2 0x006f99f0
void pcTerrainShader::Begin_Prog1(unsigned int uiState, unsigned int uiPass) const
{
    s_pShadowMapTexture = RedShadowMap::GetShadowMapTexture();
    s_uiShadowMapWrapMode = RedShadowMap::GetShadowMapWrapMode();

    if (uiState <= 1)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);
        RedRenderer::pcSetTexture(0, s_pFarTexture);
        SetWrapLinear(0);

        pcRedVertexShaderConstantRegister* pRegs;
        if (uiState == 1)
        {
            RedRenderer::pcSetTexture(1, s_pFarDetailTexture);
            SetWrapLinear(1);
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 3);
            SetColor(pRegs[39], RedColorValue::WHITE);
            SetVector(pRegs[40], s_FarTextureTransform);
            SetVector(pRegs[41], s_FarDetailTextureTransform);
        }
        else
        {
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 2);
            SetColor(pRegs[39], RedColorValue::WHITE);
            SetVector(pRegs[40], s_FarTextureTransform);
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        RedRenderer::pcLoadFormat(s_pFarVertexInputFormat);
        return;
    }

    if (uiState <= 9)
    {
        if (uiPass != 0)
        {
            SetModulate2XBlend();
            RedRenderer::pcSetTexture(0, s_pDetailTexture);
            SetWrapLinear(0);
            SetMipBias(0, kfDetailTextureMipBias);

            pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 4);
            SetColor(pRegs[39], RedColorValue::WHITE);
            Set(pRegs[40], -2.0f / s_fDetailFadeDistance, 2.0f, 0.0f, 0.0f);
            SetVector(pRegs[41], s_DetailTextureTransform[0]);
            SetVector(pRegs[42], s_DetailTextureTransform[1]);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
            RedRenderer::pcLoadFormat(s_pPosVertexInputFormat);
            return;
        }

        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);

        const bool bShadow = uiState >= 6;
        const bool bDetail = uiState == 5 || uiState == 9;
        const unsigned int uiNumLayers = (uiState & 1) ? 2 : (bShadow ? 3 : 4);
        for (unsigned int i = 0; i < uiNumLayers; i++)
        {
            SetWrapLinear(i);
            SetMipBias(i, kfMainTextureMipBias);
        }
        if (bDetail)
        {
            RedRenderer::pcSetTexture(2, s_pDetailTexture);
            SetWrapLinear(2);
        }
        if (bShadow)
        {
            SetShadowMapStage(3);
        }

        const RedColorValue color = ToColorValue(RedColor::WHITE);
        pcRedVertexShaderConstantRegister* pRegs;
        if (bDetail)
        {
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 8);
            SetColor(pRegs[39], color);
            Set(pRegs[40], -2.0f / s_fDetailFadeDistance, 2.0f, s_fLightningValue, kInv255);
            SetVector(pRegs[45], s_DetailTextureTransform[0]);
            SetVector(pRegs[46], s_DetailTextureTransform[1]);
        }
        else
        {
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 2);
            SetColor(pRegs[39], color);
            Set(pRegs[40], 0.0f, 0.0f, s_fLightningValue, kInv255);
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        if (bShadow)
        {
            RedColorValue ambient = RedLight::GetGlobalAmbientLight()->m_color;
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &ambient.r, 1);
        }
        RedRenderer::pcLoadFormat(s_pVertexInputFormat);
        return;
    }

    if (uiState <= 15)
    {
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
        if (uiState >= 13)
        {
            SetShadowMapStage(2);
        }

        pcRedVertexShaderConstantRegister* pRegs;
        if (uiState == 10 || uiState == 13)
        {
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 2);
            SetColor(pRegs[39], RedColorValue::WHITE);
            Set(pRegs[40], 0.0f, 0.0f, s_fLightningValue, 0.0f);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
            RedRenderer::pcLoadFormat(s_pVertexInputFormat);
            return;
        }

        if (uiState == 11 || uiState == 14)
        {
            RedRenderer::pcSetTexture(0, s_pDetailTexture);
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 4);
            SetColor(pRegs[39], RedColorValue::WHITE);
            Set(pRegs[40], -2.0f / s_fDetailFadeDistance, 2.0f, s_fLightningValue, 0.0f);
            SetVector(pRegs[41], s_DetailTextureTransform[0]);
            SetVector(pRegs[42], s_DetailTextureTransform[1]);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        }
        else
        {
            RedRenderer::pcSetTexture(0, s_pBumpLuminosityTexture);
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 4);
            SetColor(pRegs[39], RedColorValue::WHITE);
            Set(pRegs[40], -2.0f / s_fDetailFadeDistance, 2.0f, s_fLightningValue, 0.0f);
            SetVector(pRegs[41], s_BumpLuminosityTextureTransform[0]);
            SetVector(pRegs[42], s_BumpLuminosityTextureTransform[1]);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);

            RedColorValue ambient[2];
            ambient[0] = RedLight::GetGlobalAmbientLight()->m_color;
            ambient[1] = RedLight::GetGlobalAmbientLight()->m_color;
            RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &ambient[0].r, 2);
        }
        SetWrapLinear(0);
        SetMipBias(0, kfDetailTextureMipBias);
        RedRenderer::pcLoadFormat(s_pVertexInputFormat);
        return;
    }

    if (uiState >= 18 && uiPass < GetShaderDef()->m_states[uiState].uiNumPasses - 1)
    {

        unsigned int uiNumLights = ((uiState - 18) >> 2) + 1;
        if (uiNumLights == 4)
        {
            uiNumLights = (uiPass == 0) * 2 + 1;
        }

        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 7);

        pcRedVertexShaderConstantRegister* pRegs;
        if ((uiState - 2) & 2)
        {
            RedRenderer::pcSetTexture(0, s_pBumpLuminosityTexture);
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 4);
            SetColor(pRegs[39], ToColorValue(RedColor::WHITE));
            Set(pRegs[40], -2.0f / s_fDetailFadeDistance, 2.0f, 0.0f, 0.0f);
            SetVector(pRegs[41], s_BumpLuminosityTextureTransform[0]);
            SetVector(pRegs[42], s_BumpLuminosityTextureTransform[1]);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        else
        {
            RedRenderer::pcSetTexture(0, s_pNormalizationCubeMap);
            pRegs = RedRenderer::pcLockVertexShaderConstants(39, 2);
            SetColor(pRegs[39], ToColorValue(RedColor::WHITE));
            Set(pRegs[40], -2.0f / s_fDetailFadeDistance, 2.0f, 0.0f, 0.0f);
            RedRenderer::pcUnlockVertexShaderConstants(pRegs);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
        }
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

        for (unsigned int i = 1; i <= uiNumLights; i++)
        {
            RedRenderer::pcSetTexture(i, s_pLightVolumeMap);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            RedRenderer::pcSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        }
        RedRenderer::pcLoadFormat(s_pPosNormalVertexInputFormat);
        return;
    }

    SetModulate2XBlend();
    const unsigned int uiNumLayers = (uiState & 1) ? 2 : 4;
    for (unsigned int i = 0; i < uiNumLayers; i++)
    {
        SetWrapLinear(i);
        SetMipBias(i, kfMainTextureMipBias);
    }
    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 2);
    SetColor(pRegs[39], ToColorValue(RedColor::WHITE));
    Set(pRegs[40], 0.0f, 0.0f, 0.0f, kInv255);
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    RedRenderer::pcLoadFormat(s_pVertexInputFormat);
}

// FUNCTION: SWBF2 0x006fac20
void pcTerrainShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x006fac40
RedShadingState* pcTerrainShader::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x006fac50
void pcTerrainShader::End_Fixed(unsigned int uiState, unsigned int) const
{
    if (uiState >= 6)
    {
        RedRenderer::pcGetDevice()->SetTransform(D3DTS_PROJECTION,
                                                 (const D3DMATRIX*)&s_mProjectionRestore);
    }
}

// FUNCTION: SWBF2 0x006fac80
void pcTerrainShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x006fac90
unsigned int pcTerrainShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x006facb0
RenderPhaseT pcTerrainShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    if ((s_uiOptions & 0x20000) && (m_uiOptions & 0xd) == 1)
    {
        return RENDERPHASE_POSTSHADOW;
    }
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x006face0
unsigned int pcTerrainShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x006facf0
unsigned short pcTerrainShader::GetShaderChain(pcRedShader** const ppOut,
                                               unsigned short,
                                               const RenderItem*)
{
    ppOut[0] = this;
    m_lightingInSeparatePass = false;
    return 1;
}

// FUNCTION: SWBF2 0x006fad10
pcRedShaderDef* pcTerrainShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x006fad20
unsigned int pcTerrainShader::GetState_Fixed(const RenderItem* pItem) const
{
    if (!RedRenderer::pcIsNearScene() || (pItem->flags & 0x20000))
    {
        return m_uiOptions & 1;
    }
    if (m_uiOptions & 0x30)
    {
        return ((m_uiOptions >> 4) & 3) + 5;
    }
    if (m_uiOptions & 1)
    {
        return (s_uiOptions & 0x10000) ? 5 : 3;
    }
    return (s_uiOptions & 0x10000) ? 4 : 2;
}

// FUNCTION: SWBF2 0x006fad90
unsigned int pcTerrainShader::GetState_Prog1(const RenderItem* pItem) const
{
    const unsigned int uiEnvState = pItem->uiEnvState;
    if (!RedRenderer::pcIsNearScene())
    {
        return m_uiOptions & 1;
    }

    if ((s_uiOptions & 0x20000) && (m_uiOptions & 1))
    {
        if (m_uiOptions & 4)
        {
            if (s_uiOptions & 0x40000)
            {
                return (s_uiOptions & 0x10000) ? 15 : 12;
            }
            return (s_uiOptions & 0x10000) ? 14 : 11;
        }

        unsigned int uiNumLights = uiEnvState & 3;
        if (uiNumLights == 0)
        {
            return m_uiLayerIndex[2] < 16 ? 16 : 17;
        }
        if (uiNumLights == 3 && pItem->pLightingState->pOmniLights[3])
        {
            uiNumLights = 4;
        }
        if (s_uiOptions & 0x40000)
        {
            return (m_uiLayerIndex[2] >= 16) + 4 * uiNumLights + 16;
        }
        return (m_uiLayerIndex[2] >= 16) + 4 * uiNumLights + 14;
    }

    return ((s_uiOptions & 0x10000) ? 4 : 0) + ((m_uiOptions & 1) ? 4 : 2)
           + (m_uiLayerIndex[2] >= 16);
}

// FUNCTION: SWBF2 0x006faea0
unsigned int pcTerrainShader::GetZPrepassState() const
{
    return 0;
}

// GLOBAL: SWBF2 0x00784348
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[8] =
{
    { 1, { 0x021, 0x000, 0x000, 0x000 } },
    { 1, { 0x021, 0x000, 0x000, 0x000 } },
    { 2, { 0x121, 0x142, 0x000, 0x000 } },
    { 3, { 0x121, 0x142, 0x021, 0x000 } },
    { 2, { 0x121, 0x142, 0x000, 0x000 } },
    { 3, { 0x121, 0x142, 0x021, 0x000 } },
    { 1, { 0x121, 0x000, 0x000, 0x000 } },
    { 1, { 0x121, 0x000, 0x000, 0x000 } },
};

// FUNCTION: SWBF2 0x006faeb0
void pcTerrainShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(8, 8, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x006faed0
void pcTerrainShader::RenderZPrepass(const RenderItem* pItem) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_BACK);
    RedRenderer::pcLoadFormat(s_pPosVertexInputFormat);
    pItem->pPrim->Draw();
}

static PblMatrix LayerTextureMatrix(unsigned int uiLayer)
{
    const PblVector4& v0 = pcTerrainShader::s_TextureTransform[uiLayer][0];
    const PblVector4& v1 = pcTerrainShader::s_TextureTransform[uiLayer][1];
    return AxesMatrix(v0.x, v1.x, 0.0f, v0.y, v1.y, 0.0f, v0.z, v1.z, 0.0f, v0.w, v1.w, 0.0f);
}

// FUNCTION: SWBF2 0x006faf00
void pcTerrainShader::Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const
{
    const unsigned int uiState = pItem->uiShaderState;
    if (uiState <= 1)
    {
        s_d3dMat.Diffuse.r = pItem->tweakColor.r * kInv255;
        s_d3dMat.Diffuse.g = pItem->tweakColor.g * kInv255;
        s_d3dMat.Diffuse.b = pItem->tweakColor.b * kInv255;
        s_d3dMat.Diffuse.a = pItem->tweakColor.a * kInv255;
        s_d3dMat.Ambient = s_d3dMat.Diffuse;
        s_d3dMat.Emissive.r = 0.0f;
        s_d3dMat.Emissive.g = 0.0f;
        s_d3dMat.Emissive.b = 0.0f;
        s_d3dMat.Emissive.a = 0.0f;
        RedRenderer::pcGetDevice()->SetMaterial(&s_d3dMat);
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, !((pItem->flags >> 5) & 1));
    }
    else if (uiState >= 6)
    {
        const unsigned int uiLayer = m_uiLayerIndex[0];
        RedRenderer::pcSetTexture(0, s_apTextures[uiLayer]);
        SetCameraSpaceTransform(0, LayerTextureMatrix(uiLayer));
    }
    else if (uiPass == 0)
    {
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, !((pItem->flags >> 5) & 1));
        const unsigned int uiNumLayers = m_uiLayerIndex[1] < 16 ? 2 : 1;
        for (unsigned int i = 0; i < uiNumLayers; i++)
        {
            const unsigned int uiLayer = m_uiLayerIndex[i];
            RedRenderer::pcSetTexture(i, s_apTextures[uiLayer]);
            SetCameraSpaceTransform(i, LayerTextureMatrix(uiLayer));
        }
        if (uiNumLayers == 2)
        {
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        }
        else
        {
            RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
    }
    else if (uiPass == 1)
    {
        s_d3dMat.Diffuse.r = pItem->tweakColor.r * kInv255;
        s_d3dMat.Diffuse.g = pItem->tweakColor.g * kInv255;
        s_d3dMat.Diffuse.b = pItem->tweakColor.b * kInv255;
        s_d3dMat.Diffuse.a = 1.0f;
        s_d3dMat.Ambient = s_d3dMat.Diffuse;
        s_d3dMat.Emissive.r = 0.0f;
        s_d3dMat.Emissive.g = 0.0f;
        s_d3dMat.Emissive.b = 0.0f;
        s_d3dMat.Emissive.a = 0.0f;
        RedRenderer::pcGetDevice()->SetMaterial(&s_d3dMat);
    }

    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x006fb430
void pcTerrainShader::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    const unsigned int uiState = pItem->uiShaderState;
    unsigned int uiNumLayers = 0;

    if (uiState <= 1 || (uiState > 9 && uiState <= 15))
    {
        RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) == 0);
    }
    else if (uiState <= 9)
    {
        if (uiPass == 0)
        {
            RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (pItem->flags & 0x20) == 0);
            if (uiState & 1)
            {
                uiNumLayers = m_uiLayerIndex[1] < 16 ? 2 : 1;
            }
            else if (uiState < 6 && m_uiLayerIndex[3] < 16)
            {
                uiNumLayers = 4;
            }
            else
            {
                uiNumLayers = 3;
            }
        }
    }
    else if (uiState >= 18 && uiPass < GetShaderDef()->m_states[uiState].uiNumPasses - 1)
    {
        const RedLightingState* pLightingState = pItem->pLightingState;
        unsigned int uiNumLights = ((uiState - 18) >> 2) + 1;
        if (uiNumLights == 4)
        {
            uiNumLights = (uiPass == 0) * 2 + 1;
        }
        const unsigned int uiFirst = uiPass * 3;

        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(43, uiNumLights);
        RedColorValue colors[3];
        RedOmniLight* const* ppLights = &pLightingState->pOmniLights[uiFirst];
        if (uiNumLights != 1)
        {
            if (uiNumLights != 2)
            {
                Set(pRegs[45], ppLights[2]->m_pos.x, ppLights[2]->m_pos.y, ppLights[2]->m_pos.z,
                    ppLights[2]->m_fRadiusInv);
                colors[2] = ppLights[2]->m_color;
            }
            Set(pRegs[44], ppLights[1]->m_pos.x, ppLights[1]->m_pos.y, ppLights[1]->m_pos.z,
                ppLights[1]->m_fRadiusInv);
            colors[1] = ppLights[1]->m_color;
        }
        Set(pRegs[43], ppLights[0]->m_pos.x, ppLights[0]->m_pos.y, ppLights[0]->m_pos.z,
            ppLights[0]->m_fRadiusInv);
        colors[0] = ppLights[0]->m_color;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &colors[0].r, uiNumLights);
    }
    else
    {
        if (uiState & 1)
        {
            uiNumLayers = m_uiLayerIndex[1] < 16 ? 2 : 1;
        }
        else
        {
            uiNumLayers = m_uiLayerIndex[3] < 16 ? 4 : 3;
        }
    }

    if (uiNumLayers)
    {
        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(41, uiNumLayers * 2);
        for (unsigned int i = 0; i < uiNumLayers; i++)
        {
            const int iLayer = m_uiLayerIndex[i];
            if (iLayer < 16)
            {
                RedRenderer::pcSetTexture(i, s_apTextures[iLayer]);
                SetVector(pRegs[41 + 2 * i], s_TextureTransform[iLayer][0]);
                SetVector(pRegs[42 + 2 * i], s_TextureTransform[iLayer][1]);
            }
            else
            {
                RedRenderer::pcSetTexture(i, 0);
            }
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    }

    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x006fb7a0
pcRedShader* pcTerrainShader::_Create(unsigned int uiFlags,
                                      const unsigned int* pParams,
                                      unsigned int uiNumParams,
                                      unsigned int uiMaterialFlags,
                                      const RedColor& color0,
                                      const RedColor& color1,
                                      float fSpecularPower) const
{
    return new pcTerrainShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                               color0, color1, fSpecularPower);
}

// FUNCTION: SWBF2 0x006fb830
void pcTerrainShader::_Init(unsigned int uiFlags,
                            const unsigned int* pParams,
                            unsigned int uiNumParams,
                            unsigned int,
                            const RedColor&,
                            const RedColor&,
                            float)
{
    m_uiOptions = 0;
    switch (uiFlags & 0xffff0fff)
    {
    case 2:
        if (s_pFarVertexInputFormat == 0)
        {
            s_pFarVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 2);
        }
        break;
    case 0x82:
        if (!(uiFlags & 0xf000))
        {
            m_uiOptions = 0x10;
            if (s_pFFLayerNVertexInputFormat == 0)
            {
                s_pFFLayerNVertexInputFormat = pcRedVertexFormat::Create(0x82, 0x82);
            }
        }
        break;
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
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        m_uiLayerIndex[i] = 0xff;
        if (pParams && i < uiNumParams)
        {
            m_uiLayerIndex[i] = (unsigned char)pParams[i];
        }
    }
}

// FUNCTION: SWBF2 0x006fb930
void pcTerrainShader::_InitBumpLuminosityPixelLighting(unsigned int uiHash,
                                                       unsigned int uiBumpLuminosityHash,
                                                       const PblVector4& vScale0,
                                                       const PblVector4& vScale1,
                                                       float fStaticBumpFactor)
{
    if (RedRenderer::pcGetRenderCaps().volumemaps)
    {
        s_pLightVolumeMap = RedTexture::s_textureHashTable.Find(uiHash);
        s_pBumpLuminosityTexture = RedTexture::s_textureHashTable.Find(uiBumpLuminosityHash);
        s_BumpLuminosityTextureTransform[0] = vScale0;
        s_BumpLuminosityTextureTransform[1] = vScale1;
        s_uiOptions |= 0x40000;
        s_fStaticBumpFactor = fStaticBumpFactor;
    }
}

// FUNCTION: SWBF2 0x006fb9b0
void pcTerrainShader::_InitDetail(unsigned int uiHash,
                                  const PblVector4& vScale0,
                                  const PblVector4& vScale1,
                                  float fFadeDistance)
{
    s_pDetailTexture = uiHash ? RedTexture::s_textureHashTable.Find(uiHash) : 0;
    s_DetailTextureTransform[0] = vScale0;
    s_DetailTextureTransform[1] = vScale1;
    s_fDetailFadeDistance = fFadeDistance;
}

// FUNCTION: SWBF2 0x006fba10
void pcTerrainShader::_InitFarDetail(unsigned int uiHash, float fX, float fY, float fZ, float fW)
{
    s_pFarDetailTexture = RedTexture::s_textureHashTable.Find(uiHash);
    s_FarDetailTextureTransform.x = fX;
    s_FarDetailTextureTransform.y = fY;
    s_FarDetailTextureTransform.z = fZ;
    s_FarDetailTextureTransform.w = fW;
}

// FUNCTION: SWBF2 0x006fba70
void pcTerrainShader::_InitFarRendering(unsigned int uiHash, float fX, float fY, float fZ, float fW)
{
    s_pFarTexture = RedTexture::s_textureHashTable.Find(uiHash);
    s_FarTextureTransform.x = fX;
    s_FarTextureTransform.y = fY;
    s_FarTextureTransform.z = fZ;
    s_FarTextureTransform.w = fW;
}

// FUNCTION: SWBF2 0x006fbad0
void pcTerrainShader::_InitLayer(int iIndex, unsigned int uiHash,
                                 const PblVector4& vScale0, const PblVector4& vScale1)
{
    s_apTextures[iIndex] = uiHash ? RedTexture::s_textureHashTable.Find(uiHash) : 0;
    s_TextureTransform[iIndex][0] = vScale0;
    s_TextureTransform[iIndex][1] = vScale1;
}

// FUNCTION: SWBF2 0x006fbb20
void pcTerrainShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcTerrainShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcTerrainShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcTerrainShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcTerrainShader::Render_Fixed);
    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcTerrainShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcTerrainShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcTerrainShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcTerrainShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x006fbb80
void pcTerrainShader::_InitPixelLighting(unsigned int uiLightVolumeHash,
                                         unsigned int uiNormalizationCubeHash)
{
    if (RedRenderer::pcGetRenderCaps().cubemaps && RedRenderer::pcGetRenderCaps().volumemaps)
    {
        s_pLightVolumeMap = RedTexture::s_textureHashTable.Find(uiLightVolumeHash);
        s_pNormalizationCubeMap = RedTexture::s_textureHashTable.Find(uiNormalizationCubeHash);
        s_uiOptions &= ~0x40000;
    }
}

// FUNCTION: SWBF2 0x006fbbe0
void pcTerrainShader::_InitShadowMap(RedTexture* pTexture, unsigned int uiWrapMode,
                                     const PblVector4&, const PblVector4&, const PblVector4&)
{
    s_pShadowMapTexture = pTexture;
    s_uiShadowMapWrapMode = uiWrapMode;
}

// FUNCTION: SWBF2 0x006fbc00
void pcTerrainShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x006fbce0
void pcTerrainShader::_StaticInit() const
{
    s_pVertexInputFormat = 0;
    s_pPosVertexInputFormat = 0;
    s_pPosNormalVertexInputFormat = 0;
    s_pFarVertexInputFormat = 0;
    s_pFFLayer0VertexInputFormat = 0;
    s_pFFLayerNVertexInputFormat = 0;
    memset(s_apTextures, 0, sizeof(s_apTextures));
    s_pDetailTexture = 0;
    s_pBumpSpecularTexture = 0;
    s_pShadowMapTexture = 0;
    s_pNormalizationCubeMap = 0;
    s_pLightVolumeMap = 0;
    s_pFarTexture = 0;
    s_pFarDetailTexture = 0;
    memset(s_TextureTransform, 0, sizeof(s_TextureTransform));
    s_fDetailFadeDistance = 32.0f;
    const PblVector4 vZero = { 0.0f, 0.0f, 0.0f, 0.0f };
    s_DetailTextureTransform[0] = vZero;
    s_fStaticBumpFactor = 0.5f;
    s_DetailTextureTransform[1] = vZero;
    s_uiOptions = 0;
    s_BumpSpecularTextureTransform[0] = vZero;
    s_fLightningValue = 0.0f;
    s_BumpSpecularTextureTransform[1] = vZero;
    s_FarTextureTransform = vZero;
    s_FarDetailTextureTransform = vZero;
    memset(&s_d3dMat, 0, sizeof(s_d3dMat));
}

// GLOBAL: SWBF2 0x009c8620
// SYNTHETIC: SWBF2 0x00409350
// `dynamic initializer for 'pcTerrainShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769bd0
// `dynamic atexit destructor for 'pcTerrainShader::s_Factory''
pcTerrainShader pcTerrainShader::s_Factory;

// SYNTHETIC: SWBF2 0x004093c0
// `dynamic initializer for 'pcTerrainShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769bf0
// `dynamic atexit destructor for 'pcTerrainShader::s_shaderDef''
pcRedShaderDef pcTerrainShader::s_shaderDef("terrain_shader.xml");

// SYNTHETIC: SWBF2 0x00409420
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x00409400
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// GLOBAL: SWBF2 0x009c8510
unsigned int g_pcTerrainShader;
