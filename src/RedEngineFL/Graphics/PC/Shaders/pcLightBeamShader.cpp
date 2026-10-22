#include "pcLightBeamShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "PebbleFL/Common/PblMatrix.h"
#include <string.h>

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcLightBeamShader::s_uiID;
pcRedShader::GetStateFuncT pcLightBeamShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcLightBeamShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcLightBeamShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcLightBeamShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x00704480
pcLightBeamShader::pcLightBeamShader()
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

// FUNCTION: SWBF2 0x00704540
pcLightBeamShader::pcLightBeamShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00704810
void pcLightBeamShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetTexture(0, 0);
}

// FUNCTION: SWBF2 0x00704850
void pcLightBeamShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00704870
void pcLightBeamShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00704880
void pcLightBeamShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00704890
unsigned int pcLightBeamShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x007048b0
RenderPhaseT pcLightBeamShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x007048c0
unsigned int pcLightBeamShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x007048d0
pcRedShaderDef* pcLightBeamShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// GLOBAL: SWBF2 0x00784a68
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00704900
void pcLightBeamShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(4, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x007048e0
unsigned int pcLightBeamShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x007048f0
unsigned int pcLightBeamShader::GetState_Prog1(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00704920
void pcLightBeamShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    if (m_uiMaterialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
    }
    const RedColor& c = pItem->tweakColor;
    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(c.a, c.r, c.g, c.b));
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00704990
void pcLightBeamShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    if (m_uiMaterialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_ADD);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetFogForBlendMode(RedRenderer::FOGFORBLEND_NORMAL);
    }
    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 1);
    const RedColor& c = pItem->tweakColor;
    pRegs[39].component[0] = c.r * (1.0f / 255.0f);
    pRegs[39].component[1] = c.g * (1.0f / 255.0f);
    pRegs[39].component[2] = c.b * (1.0f / 255.0f);
    pRegs[39].component[3] = c.a * (1.0f / 255.0f);
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00704a50
pcRedShader* pcLightBeamShader::_Create(unsigned int uiFlags,
                                        const unsigned int* pParams,
                                        unsigned int uiNumParams,
                                        unsigned int uiMaterialFlags,
                                        const RedColor& color0,
                                        const RedColor& color1,
                                        float fArg7) const
{
    return new pcLightBeamShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                 color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00704ae0
void pcLightBeamShader::_Init(unsigned int uiFlags,
                              const unsigned int*,
                              unsigned int,
                              unsigned int uiMaterialFlags,
                              const RedColor&,
                              const RedColor&,
                              float)
{
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, (uiFlags & 0x180) | 2);
    m_uiMaterialFlags = uiMaterialFlags;
}

// FUNCTION: SWBF2 0x00704b10
void pcLightBeamShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcLightBeamShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcLightBeamShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcLightBeamShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcLightBeamShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcLightBeamShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcLightBeamShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcLightBeamShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcLightBeamShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00704b70
void pcLightBeamShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00704c50
void pcLightBeamShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9994
// SYNTHETIC: SWBF2 0x00409b60
// `dynamic initializer for 'pcLightBeamShader::s_uiRenderTypeHash''
unsigned int pcLightBeamShader::s_uiRenderTypeHash = PblHash("LightBeam");

// GLOBAL: SWBF2 0x009c99d0
// SYNTHETIC: SWBF2 0x00409b20
// `dynamic initializer for 'pcLightBeamShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769d20
// `dynamic atexit destructor for 'pcLightBeamShader::s_Factory''
pcLightBeamShader pcLightBeamShader::s_Factory;

// GLOBAL: SWBF2 0x009c999c
// SYNTHETIC: SWBF2 0x00409b40
// `dynamic initializer for 'pcLightBeamShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769d40
// `dynamic atexit destructor for 'pcLightBeamShader::s_shaderDef''
pcRedShaderDef pcLightBeamShader::s_shaderDef("lightbeam_shader.xml");

// GLOBAL: SWBF2 0x009c9998
// SYNTHETIC: SWBF2 0x00409bc0
// `dynamic initializer for 'kuiLinearRamp''
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// FUNCTION: SWBF2 0x00704600
void pcLightBeamShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetTexture(0, RedTexture::Find(kuiLinearRamp));
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    const float fStart = RedRenderer::pcGetNearSceneStart();
    const float fScale = 1.0f / (RedRenderer::pcGetNearSceneEnd() - fStart);
    PblMatrix mTransform;
    memset(&mTransform, 0, 2 * sizeof(mTransform.m[0]));
    mTransform.m[2][0] = fScale;
    mTransform.m[2][1] = 0.0f;
    mTransform.m[2][2] = 0.0f;
    mTransform.m[2][3] = 0.0f;
    mTransform.m[3][0] = -(fScale * fStart);
    mTransform.m[3][1] = 0.0f;
    mTransform.m[3][2] = 0.0f;
    mTransform.m[3][3] = 1.0f;
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    RedRenderer::pcFFSetTextureTransform(0, mTransform);
}

// SYNTHETIC: SWBF2 0x00409b80
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409ba0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9990
unsigned int g_pcLightBeamShader;
