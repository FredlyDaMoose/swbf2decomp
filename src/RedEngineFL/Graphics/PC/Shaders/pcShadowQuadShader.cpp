#include "pcShadowQuadShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

void __cdecl PblTraceP(const char* Format, ...);

class PblMatrix;
extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x007e1e80
unsigned int pcShadowQuadShader::s_uiStencilMask = 0xff;

unsigned int pcShadowQuadShader::s_uiID;
pcRedShader::GetStateFuncT pcShadowQuadShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcShadowQuadShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcShadowQuadShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcShadowQuadShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9de8
// SYNTHETIC: SWBF2 0x00409f20
// `dynamic initializer for 'pcShadowQuadShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769e60
// `dynamic atexit destructor for 'pcShadowQuadShader::s_shaderDef''
pcRedShaderDef pcShadowQuadShader::s_shaderDef("shadowquad_shader.xml");

// FUNCTION: SWBF2 0x00707f20
pcShadowQuadShader::pcShadowQuadShader()
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

// FUNCTION: SWBF2 0x00707fe0
pcShadowQuadShader::pcShadowQuadShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x007080a0
void pcShadowQuadShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_STENCILENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_STENCILREF, 0);
    RedRenderer::pcSetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
    RedRenderer::pcSetRenderState(D3DRS_STENCILMASK, pcShadowQuadShader::s_uiStencilMask);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE,
                                  D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                      D3DCOLORWRITEENABLE_BLUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetTransform(D3DTS_VIEW,
                                gMatrixIdentity);
    RedRenderer::pcSetTransform(D3DTS_PROJECTION,
                                gMatrixIdentity);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x00708180
void pcShadowQuadShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_STENCILENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_STENCILREF, 0);
    RedRenderer::pcSetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
    RedRenderer::pcSetRenderState(D3DRS_STENCILMASK, pcShadowQuadShader::s_uiStencilMask);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
}

// FUNCTION: SWBF2 0x00708200
void pcShadowQuadShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00708220
RedShadingState* pcShadowQuadShader::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00708230
void pcShadowQuadShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00708240
void pcShadowQuadShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00708250
unsigned int pcShadowQuadShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00708270
RenderPhaseT pcShadowQuadShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x00708280
unsigned int pcShadowQuadShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00708290
pcRedShaderDef* pcShadowQuadShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x007082a0
unsigned int pcShadowQuadShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x007082b0
unsigned int pcShadowQuadShader::GetState_Prog1(const RenderItem*) const
{
    return 0;
}

// GLOBAL: SWBF2 0x00784ef8
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x007082c0
void pcShadowQuadShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x007082e0
void pcShadowQuadShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    const RedColor& c = pItem->tweakColor;
    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(c.a, c.r, c.g, c.b));
    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00708330
void pcShadowQuadShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    const RedColor& c = pItem->tweakColor;
    float afColor[4];
    afColor[0] = c.r * (1.0f / 255.0f);
    afColor[1] = c.g * (1.0f / 255.0f);
    afColor[2] = c.b * (1.0f / 255.0f);
    afColor[3] = c.a * (1.0f / 255.0f);
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, afColor, 1);
    RedRenderer::pcLoadFormat(m_pVertexInputFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x007083d0
pcRedShader* pcShadowQuadShader::_Create(unsigned int uiFlags,
                                         const unsigned int* pParams,
                                         unsigned int uiNumParams,
                                         unsigned int uiMaterialFlags,
                                         const RedColor& color0,
                                         const RedColor& color1,
                                         float fArg7) const
{
    return new pcShadowQuadShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                  color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00708460
void pcShadowQuadShader::_Init(unsigned int uiFlags,
                               const unsigned int*,
                               unsigned int,
                               unsigned int,
                               const RedColor&,
                               const RedColor&,
                               float)
{
    m_pVertexInputFormat = pcRedVertexFormat::Create(uiFlags, 2);
}

// FUNCTION: SWBF2 0x00708480
void pcShadowQuadShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcShadowQuadShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcShadowQuadShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcShadowQuadShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcShadowQuadShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcShadowQuadShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcShadowQuadShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcShadowQuadShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcShadowQuadShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x007084e0
void pcShadowQuadShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x007085c0
void pcShadowQuadShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9ddc
// SYNTHETIC: SWBF2 0x00409f40
// `dynamic initializer for 'pcShadowQuadShader::s_uiRenderTypeHash''
unsigned int pcShadowQuadShader::s_uiRenderTypeHash = PblHash("ShadowQuad");

// SYNTHETIC: SWBF2 0x00409f60
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409f80
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9dbc
// SYNTHETIC: SWBF2 0x00409f00
// `dynamic initializer for 'pcShadowQuadShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769e40
// `dynamic atexit destructor for 'pcShadowQuadShader::s_Factory''
pcShadowQuadShader pcShadowQuadShader::s_Factory;

// GLOBAL: SWBF2 0x009c9db8
unsigned int g_pcShadowQuadShader;
