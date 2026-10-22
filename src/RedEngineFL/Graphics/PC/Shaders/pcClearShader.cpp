#include "pcClearShader.h"
#include "PebbleFL/Common/PblMath.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcClearShader::s_uiID;
pcRedShader::GetStateFuncT pcClearShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcClearShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcClearShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcClearShader::RenderFuncArray[MAX_PIPELINES];
// GLOBAL: SWBF2 0x009c9c5c
// SYNTHETIC: SWBF2 0x00409de0
// `dynamic initializer for 'pcClearShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769e00
// `dynamic atexit destructor for 'pcClearShader::s_shaderDef''
pcRedShaderDef pcClearShader::s_shaderDef;

// FUNCTION: SWBF2 0x00706840
pcClearShader::pcClearShader()
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

// FUNCTION: SWBF2 0x00706900
pcClearShader::pcClearShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x007069c0
void pcClearShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_LIGHTING, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
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
    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
    {
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    }
}

// FUNCTION: SWBF2 0x00706aa0
void pcClearShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00706ac0
void pcClearShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00706ad0
unsigned int pcClearShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00706af0
RenderPhaseT pcClearShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x00706b00
unsigned int pcClearShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00706b10
pcRedShaderDef* pcClearShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x00706b20
unsigned int pcClearShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// GLOBAL: SWBF2 0x00784ca0
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00706b30
void pcClearShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00706b50
void pcClearShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    const RedColor& c = pItem->tweakColor;
    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(c.a, c.r, c.g, c.b));

    if (m_pTex)
    {
        RedRenderer::pcSetTexture(0, m_pTex);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    }
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

    if (m_pAlphaMaskTex)
    {
        RedRenderer::pcSetTexture(1, m_pAlphaMaskTex);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    else
    {
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }

    switch (m_blendMode)
    {
    case BLENDMODE_NONE:
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        break;
    case BLENDMODE_CONSTANT_BLEND:
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        break;
    case BLENDMODE_BLEND:
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        break;
    case BLENDMODE_ADD:
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        break;
    case BLENDMODE_MODULATE:
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        break;
    }

    if (m_uiMaterialFlags & 0x200000)
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
    else
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }

    RedRenderer::pcLoadFormat(m_pVertexFormat);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  (m_uiMaterialFlags & 0x80000)
                                      ? D3DCOLORWRITEENABLE_ALPHA
                                      : D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                            D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00706d70
pcRedShader* pcClearShader::_Create(unsigned int uiFlags,
                                    const unsigned int* pParams,
                                    unsigned int uiNumParams,
                                    unsigned int uiMaterialFlags,
                                    const RedColor& color0,
                                    const RedColor& color1,
                                    float fArg7) const
{
    return new pcClearShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                             color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00706e00
void pcClearShader::_Init(unsigned int uiFlags,
                          const unsigned int*,
                          unsigned int,
                          unsigned int uiMaterialFlags,
                          const RedColor&,
                          const RedColor&,
                          float)
{
    m_uiMaterialFlags = uiMaterialFlags;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
    m_pTex = 0;
    m_pAlphaMaskTex = 0;
    m_blendMode = (m_uiMaterialFlags & 4) ? BLENDMODE_BLEND : BLENDMODE_NONE;
}

// FUNCTION: SWBF2 0x00706e40
void pcClearShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcClearShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcClearShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcClearShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcClearShader::Render_Fixed);
}

// FUNCTION: SWBF2 0x00706e70
void pcClearShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00706f50
void pcClearShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9c88
// SYNTHETIC: SWBF2 0x00409e00
// `dynamic initializer for 'pcClearShader::s_uiRenderTypeHash''
unsigned int pcClearShader::s_uiRenderTypeHash = PblHash("clear");

// GLOBAL: SWBF2 0x009c9c24
// SYNTHETIC: SWBF2 0x00409dc0
// `dynamic initializer for 'pcClearShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769de0
// `dynamic atexit destructor for 'pcClearShader::s_Factory''
pcClearShader pcClearShader::s_Factory;

// SYNTHETIC: SWBF2 0x00409e20
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409e40
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9c20
unsigned int g_pcClearShader;
