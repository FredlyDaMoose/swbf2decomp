#include "pcSkyFogShader.h"

#include "PebbleFL/Common/PblChunk.h"

#include "RedEngineFL/Memory/engineAlloc.h"

#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"

#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

#include "PebbleFL/Common/PblMatrix.h"

void __cdecl PblTraceP(const char* Format, ...);

class PblMatrix;
extern PblMatrix gMatrixIdentity;

unsigned int pcSkyFogShader::s_uiID;
pcRedShader::GetStateFuncT pcSkyFogShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcSkyFogShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcSkyFogShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcSkyFogShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9904
// SYNTHETIC: SWBF2 0x00409aa0
// `dynamic initializer for 'pcSkyFogShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769d10
// `dynamic atexit destructor for 'pcSkyFogShader::s_shaderDef''
pcRedShaderDef pcSkyFogShader::s_shaderDef("skyfog_shader.xml");

// FUNCTION: SWBF2 0x00703cb0
pcSkyFogShader::pcSkyFogShader()
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

// FUNCTION: SWBF2 0x00703d70
pcSkyFogShader::pcSkyFogShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x007040c0
pcRedShaderDef* pcSkyFogShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// GLOBAL: SWBF2 0x007849b0
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x007040f0
void pcSkyFogShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x10, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00704270
pcRedShader* pcSkyFogShader::_Create(unsigned int uiFlags,
                                     const unsigned int* pParams,
                                     unsigned int uiNumParams,
                                     unsigned int uiMaterialFlags,
                                     const RedColor& color0,
                                     const RedColor& color1,
                                     float fArg7) const
{
    return new pcSkyFogShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                              color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00704300
void pcSkyFogShader::_Init(unsigned int uiFlags,
                           const unsigned int*,
                           unsigned int,
                           unsigned int,
                           const RedColor&,
                           const RedColor&,
                           float)
{
    m_pVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 0x202);
    m_pSkyTexture = 0;
}

// FUNCTION: SWBF2 0x00704330
void pcSkyFogShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcSkyFogShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcSkyFogShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcSkyFogShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcSkyFogShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcSkyFogShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcSkyFogShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcSkyFogShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcSkyFogShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00704390
void pcSkyFogShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x007040a0
RenderPhaseT pcSkyFogShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x00704080
unsigned int pcSkyFogShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x007040b0
unsigned int pcSkyFogShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00704050
RedShadingState* pcSkyFogShader::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00704030
void pcSkyFogShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x007040d0
unsigned int pcSkyFogShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x007040e0
unsigned int pcSkyFogShader::GetState_Prog1(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00703fa0
void pcSkyFogShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTexture(0, m_pSkyTexture);
}

// FUNCTION: SWBF2 0x00703e30
void pcSkyFogShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                      D3DCOLORWRITEENABLE_BLUE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTexture(0, m_pSkyTexture);

    PblMatrix mProjection;
    mProjection.m[0][0] = 1.0f; mProjection.m[0][1] = 0.0f; mProjection.m[0][2] = 0.0f; mProjection.m[0][3] = 0.0f;
    mProjection.m[1][0] = 0.0f; mProjection.m[1][1] = 1.0f; mProjection.m[1][2] = 0.0f; mProjection.m[1][3] = 0.0f;
    mProjection.m[2][0] = 0.0f; mProjection.m[2][1] = 0.0f; mProjection.m[2][2] = 1.0f; mProjection.m[2][3] = 0.0f;
    mProjection.m[3][0] = 0.0f; mProjection.m[3][1] = 0.0f; mProjection.m[3][2] = -1.0f; mProjection.m[3][3] = 1.0f;
    RedRenderer::pcSetTransform(D3DTS_VIEW, gMatrixIdentity);
    RedRenderer::pcSetTransform(D3DTS_PROJECTION, mProjection);

    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0xff000000);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x00704060
void pcSkyFogShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00704070
void pcSkyFogShader::End_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetTexture(0, 0);
}

// FUNCTION: SWBF2 0x00704110
void pcSkyFogShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00704130
void pcSkyFogShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    const PblMatrix proj = RedRenderer::GetProjectionMatrix();
    const float fNear = RedRenderer::pcGetNearSceneStart();

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(40, 3);
    pRegs[40].component[0] = fNear;
    pRegs[40].component[1] = fNear;
    pRegs[40].component[2] = fNear;
    pRegs[40].component[3] = 1.0f;
    pRegs[41].component[0] = proj.m[0][2];
    pRegs[41].component[1] = proj.m[1][2];
    pRegs[41].component[2] = -proj.m[2][2];
    pRegs[41].component[3] = proj.m[3][2];
    pRegs[42].component[0] = viewport.fWidth;
    pRegs[42].component[1] = viewport.fHeight;
    pRegs[42].component[2] = viewport.fOffsetX;
    pRegs[42].component[3] = viewport.fOffsetY;
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00704470
void pcSkyFogShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9900
// SYNTHETIC: SWBF2 0x00409ac0
// `dynamic initializer for 'pcSkyFogShader::s_uiRenderTypeHash''
unsigned int pcSkyFogShader::s_uiRenderTypeHash = PblHash("SkyFog");

// SYNTHETIC: SWBF2 0x00409ae0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409b00
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9938
// SYNTHETIC: SWBF2 0x00409a80
// `dynamic initializer for 'pcSkyFogShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769cf0
// `dynamic atexit destructor for 'pcSkyFogShader::s_Factory''
pcSkyFogShader pcSkyFogShader::s_Factory;

// GLOBAL: SWBF2 0x009c98fc
unsigned int g_pcSkyFogShader;
