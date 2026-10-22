#include "pcFlareShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcFlareShader::s_uiID;
pcRedShader::GetStateFuncT pcFlareShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcFlareShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcFlareShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcFlareShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9bc0
// SYNTHETIC: SWBF2 0x00409d40
// `dynamic initializer for 'pcFlareShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769dd0
// `dynamic atexit destructor for 'pcFlareShader::s_shaderDef''
pcRedShaderDef pcFlareShader::s_shaderDef("flare_shader.xml");

// FUNCTION: SWBF2 0x00705ec0
pcFlareShader::pcFlareShader()
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

// FUNCTION: SWBF2 0x00705f80
pcFlareShader::pcFlareShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00706040
void pcFlareShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
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
    RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
}

// FUNCTION: SWBF2 0x00706160
void pcFlareShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 0);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

// FUNCTION: SWBF2 0x007061d0
void pcFlareShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x007061f0
void pcFlareShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00706200
void pcFlareShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00706210
unsigned int pcFlareShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00706230
RenderPhaseT pcFlareShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x00706240
unsigned int pcFlareShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00706250
pcRedShaderDef* pcFlareShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x00706260
unsigned int pcFlareShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00706270
unsigned int pcFlareShader::GetState_Prog1(const RenderItem*) const
{
    return m_texture == 0;
}

// GLOBAL: SWBF2 0x00784c20
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00706280
void pcFlareShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x24, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x007062a0
void pcFlareShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    const RedColor& c = pItem->tweakColor;
    RedColor color;
    color.r = (unsigned char)(int)(c.r * (1.0f / 255.0f) * m_colorDiffuse.r + 0.5f);
    color.g = (unsigned char)(int)(c.g * (1.0f / 255.0f) * m_colorDiffuse.g + 0.5f);
    color.b = (unsigned char)(int)(c.b * (1.0f / 255.0f) * m_colorDiffuse.b + 0.5f);
    color.a = (unsigned char)(int)(c.a * (1.0f / 255.0f) * m_colorDiffuse.a + 0.5f);

    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, (m_uiMaterialFlags & 0x20000) ? FALSE : TRUE);

    if (m_uiMaterialFlags & 0x100000)
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    }
    else
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    if (m_uiMaterialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    if (m_texture)
    {
        RedRenderer::pcSetTexture(0, m_texture);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    }

    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                                  D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00706450
void pcFlareShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    const RedColor& c = pItem->tweakColor;
    RedColor color;
    color.r = (unsigned char)(int)(m_colorDiffuse.r * (c.r * (1.0f / 255.0f)) + 0.5f);
    color.g = (unsigned char)(int)(m_colorDiffuse.g * (c.g * (1.0f / 255.0f)) + 0.5f);
    color.b = (unsigned char)(int)(m_colorDiffuse.b * (c.b * (1.0f / 255.0f)) + 0.5f);
    color.a = (unsigned char)(int)(m_colorDiffuse.a * (c.a * (1.0f / 255.0f)) + 0.5f);

    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, (m_uiMaterialFlags & 0x20000) ? FALSE : TRUE);

    if (m_uiMaterialFlags & 0x100000)
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    }
    else
    {
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    if (m_uiMaterialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 1);
    pRegs[39].component[0] = color.r * (1.0f / 255.0f);
    pRegs[39].component[1] = color.g * (1.0f / 255.0f);
    pRegs[39].component[2] = color.b * (1.0f / 255.0f);
    pRegs[39].component[3] = color.a * (1.0f / 255.0f);
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    RedRenderer::pcSetTexture(0, m_texture);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00706630
pcRedShader* pcFlareShader::_Create(unsigned int uiFlags,
                                    const unsigned int* pParams,
                                    unsigned int uiNumParams,
                                    unsigned int uiMaterialFlags,
                                    const RedColor& color0,
                                    const RedColor& color1,
                                    float fArg7) const
{
    return new pcFlareShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                             color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x007066c0
void pcFlareShader::_Init(unsigned int uiFlags,
                          const unsigned int*,
                          unsigned int,
                          unsigned int uiMaterialFlags,
                          const RedColor& color0,
                          const RedColor&,
                          float)
{
    m_uiMaterialFlags = uiMaterialFlags;
    m_texture = 0;
    m_colorDiffuse = color0;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
}

// FUNCTION: SWBF2 0x007066f0
void pcFlareShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcFlareShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcFlareShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcFlareShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcFlareShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcFlareShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcFlareShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcFlareShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcFlareShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00706750
void pcFlareShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00706830
void pcFlareShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9bbc
// SYNTHETIC: SWBF2 0x00409d60
// `dynamic initializer for 'pcFlareShader::s_uiRenderTypeHash''
unsigned int pcFlareShader::s_uiRenderTypeHash = PblHash("flare");

// SYNTHETIC: SWBF2 0x00409d80
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409da0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9b90
// SYNTHETIC: SWBF2 0x00409d20
// `dynamic initializer for 'pcFlareShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769db0
// `dynamic atexit destructor for 'pcFlareShader::s_Factory''
pcFlareShader pcFlareShader::s_Factory;

// GLOBAL: SWBF2 0x009c9b84
unsigned int g_pcFlareShader;
