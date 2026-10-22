#include "pcRainShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcRainShader::s_uiID;
pcRedShader::GetStateFuncT pcRainShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcRainShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcRainShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcRainShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9a68
// SYNTHETIC: SWBF2 0x00409c00
// `dynamic initializer for 'pcRainShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769d70
// `dynamic atexit destructor for 'pcRainShader::s_shaderDef''
pcRedShaderDef pcRainShader::s_shaderDef("rain_shader.xml");

// FUNCTION: SWBF2 0x00704c60
pcRainShader::pcRainShader()
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

// FUNCTION: SWBF2 0x00704d20
pcRainShader::pcRainShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00704de0
void pcRainShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetTexture(0, 0);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x00704eb0
void pcRainShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetTexture(0, 0);
}

// FUNCTION: SWBF2 0x00704f00
void pcRainShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00704f20
void pcRainShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00704f30
void pcRainShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00704f40
unsigned int pcRainShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00704f60
RenderPhaseT pcRainShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x00704f70
unsigned int pcRainShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00704f80
pcRedShaderDef* pcRainShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x00704f90
unsigned int pcRainShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x00704fa0
unsigned int pcRainShader::GetState_Prog1(const RenderItem*) const
{
    return 0;
}

// GLOBAL: SWBF2 0x00784af8
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00704fb0
void pcRainShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(4, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00704fd0
void pcRainShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    if (m_uiMaterialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    RedColor color = pItem->tweakColor;

    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);
    const float fPixelsPerUnit = (fMaxX - fMinX) * fNear / fWidth;

    const PblMatrix& camAxes = RedRenderer::GetCamera()->GetMatrix();
    const PblMatrix& obj = *pItem->pmObjWorld;
    const PblMatrix& camPos = RedRenderer::GetCamera()->GetMatrix();
    const float fDepth = camAxes.m[2][1] * (camPos.m[3][1] - obj.m[3][1])
                       + camAxes.m[2][0] * (camPos.m[3][0] - obj.m[3][0])
                       + camAxes.m[2][2] * (camPos.m[3][2] - obj.m[3][2]);

    const float fLineWidth = m_fLineWidth * fPixelsPerUnit;
    float fScale = m_fMaxPixelWidth;
    if (!(fLineWidth > fScale * fDepth))
    {
        fScale = fLineWidth / fDepth;
    }

    const float fAlpha = color.a * fScale;
    color.a = fAlpha > 255.0f ? 0xff : (unsigned char)(int)fAlpha;

    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                                  D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00705130
void pcRainShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    if (m_uiMaterialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    RedColor color = pItem->tweakColor;

    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);
    const float fPixelsPerUnit = (fMaxX - fMinX) * fNear / fWidth;

    const PblMatrix& camAxes = RedRenderer::GetCamera()->GetMatrix();
    const PblMatrix& obj = *pItem->pmObjWorld;
    const PblMatrix& camPos = RedRenderer::GetCamera()->GetMatrix();
    const float fDepth = camAxes.m[2][1] * (camPos.m[3][1] - obj.m[3][1])
                       + camAxes.m[2][0] * (camPos.m[3][0] - obj.m[3][0])
                       + camAxes.m[2][2] * (camPos.m[3][2] - obj.m[3][2]);

    const float fLineWidth = m_fLineWidth * fPixelsPerUnit;
    float fScale = m_fMaxPixelWidth;
    if (!(fLineWidth > fScale * fDepth))
    {
        fScale = fLineWidth / fDepth;
    }

    const float fAlpha = color.a * fScale;
    color.a = fAlpha > 255.0f ? 0xff : (unsigned char)(int)fAlpha;

    pcRedVertexShaderConstantRegister* pRegs = RedRenderer::pcLockVertexShaderConstants(39, 1);
    pRegs[39].component[0] = color.r * (1.0f / 255.0f);
    pRegs[39].component[1] = color.g * (1.0f / 255.0f);
    pRegs[39].component[2] = color.b * (1.0f / 255.0f);
    pRegs[39].component[3] = color.a * (1.0f / 255.0f);
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x007052f0
pcRedShader* pcRainShader::_Create(unsigned int uiFlags,
                                   const unsigned int* pParams,
                                   unsigned int uiNumParams,
                                   unsigned int uiMaterialFlags,
                                   const RedColor& color0,
                                   const RedColor& color1,
                                   float fArg7) const
{
    return new pcRainShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                            color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00705380
void pcRainShader::_Init(unsigned int uiFlags,
                         const unsigned int*,
                         unsigned int,
                         unsigned int uiMaterialFlags,
                         const RedColor&,
                         const RedColor&,
                         float)
{
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, (uiFlags & 0x180) | 2);
    m_uiMaterialFlags = uiMaterialFlags;
    m_fLineWidth = 0.02f;
    m_fMaxPixelWidth = 2.0f;
}

// FUNCTION: SWBF2 0x007053c0
void pcRainShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcRainShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcRainShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcRainShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcRainShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcRainShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcRainShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcRainShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcRainShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00705420
void pcRainShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00705500
void pcRainShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9a64
// SYNTHETIC: SWBF2 0x00409c20
// `dynamic initializer for 'pcRainShader::s_uiRenderTypeHash''
unsigned int pcRainShader::s_uiRenderTypeHash = PblHash("Rain");

// SYNTHETIC: SWBF2 0x00409c40
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409c60
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9a30
// SYNTHETIC: SWBF2 0x00409be0
// `dynamic initializer for 'pcRainShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769d50
// `dynamic atexit destructor for 'pcRainShader::s_Factory''
pcRainShader pcRainShader::s_Factory;

// GLOBAL: SWBF2 0x009c9a2c
unsigned int g_pcRainShader;
