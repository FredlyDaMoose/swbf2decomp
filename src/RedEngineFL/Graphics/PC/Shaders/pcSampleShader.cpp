#include "pcSampleShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "PebbleFL/Common/PblMatrix.h"
#include <string.h>

void __cdecl PblTraceP(const char* Format, ...);

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float kInv255 = 1.0f / 255.0f;

unsigned int pcSampleShader::s_uiID;
pcRedShader::GetStateFuncT pcSampleShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcSampleShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcSampleShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcSampleShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009c9b24
// SYNTHETIC: SWBF2 0x00409ca0
// `dynamic initializer for 'pcSampleShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769da0
// `dynamic atexit destructor for 'pcSampleShader::s_shaderDef''
pcRedShaderDef pcSampleShader::s_shaderDef("sample_shader.xml");

// FUNCTION: SWBF2 0x00705510
pcSampleShader::pcSampleShader()
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

// FUNCTION: SWBF2 0x007055d0
pcSampleShader::pcSampleShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00705690
void pcSampleShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    RedRenderer::pcSetTransform(D3DTS_VIEW,
                                gMatrixIdentity);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x00705790
void pcSampleShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
}

// FUNCTION: SWBF2 0x00705810
void pcSampleShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00705830
void pcSampleShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00705840
void pcSampleShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00705850
unsigned int pcSampleShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00705870
RenderPhaseT pcSampleShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x00705880
unsigned int pcSampleShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00705890
pcRedShaderDef* pcSampleShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x007058a0
unsigned int pcSampleShader::GetState_Fixed(const RenderItem*) const
{
    return 0;
}

// FUNCTION: SWBF2 0x007058b0
unsigned int pcSampleShader::GetState_Prog1(const RenderItem*) const
{
    return 0;
}

// GLOBAL: SWBF2 0x00784b8c
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x007058c0
void pcSampleShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x10, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00705b20
void pcSampleShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);
    const float fScaleX = 2.0f / (fMaxX - fMinX);
    const float fScaleY = 2.0f / (fMaxY - fMinY);
    const unsigned int uiWidth = m_inputTexture->m_uiWidth;
    const unsigned int uiHeight = m_inputTexture->m_uiHeight;

    pcRedVertexShaderConstantRegister* pRegs =
        RedRenderer::pcLockVertexShaderConstants(40, 2);
    pRegs[40].component[2] = m_inputOffsetX;
    pRegs[40].component[3] = m_inputOffsetY;
    pRegs[40].component[0] = 1.0f / (float)(uiWidth - 1);
    pRegs[40].component[1] = 1.0f / (float)(uiHeight - 1);
    pRegs[41].component[2] = 0.0f;
    pRegs[41].component[3] = 1.0f;
    pRegs[41].component[1] = -(m_outputY * fScaleY - 1.0f);
    pRegs[41].component[0] = m_outputX * fScaleX - 1.0f;
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, &m_scale.r, 1);
    RedRenderer::pcSetTexture(0, m_inputTexture);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x007058e0
void pcSampleShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);

    PblMatrix mTexture;
    mTexture.m[0][0] = 1.0f / (float)(m_inputTexture->m_uiWidth - 1);
    mTexture.m[0][1] = 0.0f;
    mTexture.m[0][2] = 0.0f;
    mTexture.m[0][3] = 0.0f;
    mTexture.m[1][0] = 0.0f;
    mTexture.m[1][1] = 1.0f / (float)(m_inputTexture->m_uiHeight - 1);
    mTexture.m[1][2] = 0.0f;
    mTexture.m[1][3] = 0.0f;
    mTexture.m[2][0] = 0.0f; mTexture.m[2][1] = 0.0f; mTexture.m[2][2] = 1.0f; mTexture.m[2][3] = 0.0f;
    mTexture.m[3][0] = 0.0f; mTexture.m[3][1] = 0.0f; mTexture.m[3][2] = 0.0f; mTexture.m[3][3] = 1.0f;

    PblMatrix mProjection;
    memset(&mProjection, 0, 2 * sizeof(mProjection.m[0]));
    mProjection.m[2][0] = 0.0f; mProjection.m[2][1] = 0.0f; mProjection.m[2][2] = 1.0f; mProjection.m[2][3] = 0.0f;
    mProjection.m[3][0] = 1.0f / (fMaxX - fMinX) * (m_outputX - 0.5f) - 1.0f;
    mProjection.m[3][1] = -(1.0f / (fMaxY - fMinY) * (m_outputY - 0.5f) - 1.0f);
    mProjection.m[3][2] = 0.0f;
    mProjection.m[3][3] = 1.0f;
    RedRenderer::pcFFSetTextureTransform(0, mTexture);
    RedRenderer::pcSetTransform(D3DTS_PROJECTION, mProjection);

    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                                  D3DCOLOR_ARGB((int)(m_scale.a * 255.0f), (int)(m_scale.r * 255.0f),
                                                (int)(m_scale.g * 255.0f), (int)(m_scale.b * 255.0f)));
    RedRenderer::pcSetTexture(0, m_inputTexture);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00705c90
pcRedShader* pcSampleShader::_Create(unsigned int uiFlags,
                                     const unsigned int* pParams,
                                     unsigned int uiNumParams,
                                     unsigned int uiMaterialFlags,
                                     const RedColor& color0,
                                     const RedColor& color1,
                                     float fArg7) const
{
    return new pcSampleShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                              color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00705d20
void pcSampleShader::_Init(unsigned int uiFlags,
                           const unsigned int*,
                           unsigned int,
                           unsigned int uiMaterialFlags,
                           const RedColor&,
                           const RedColor&,
                           float)
{
    m_uiMaterialFlags = uiMaterialFlags;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
    m_inputTexture = 0;
    m_scale.r = kInv255;
    m_scale.g = kInv255;
    m_scale.b = kInv255;
    m_scale.a = kInv255;
    m_outputX = 0.0f;
    m_outputY = 0.0f;
    m_inputOffsetX = 0.0f;
    m_inputOffsetY = 0.0f;
}

// FUNCTION: SWBF2 0x00705d70
void pcSampleShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcSampleShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcSampleShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcSampleShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcSampleShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcSampleShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcSampleShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcSampleShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcSampleShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00705dd0
void pcSampleShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00705eb0
void pcSampleShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c9b20
// SYNTHETIC: SWBF2 0x00409cc0
// `dynamic initializer for 'pcSampleShader::s_uiRenderTypeHash''
unsigned int pcSampleShader::s_uiRenderTypeHash = PblHash("sample");

// SYNTHETIC: SWBF2 0x00409ce0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409d00
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c9ad0
// SYNTHETIC: SWBF2 0x00409c80
// `dynamic initializer for 'pcSampleShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769d80
// `dynamic atexit destructor for 'pcSampleShader::s_Factory''
pcSampleShader pcSampleShader::s_Factory;

// GLOBAL: SWBF2 0x009c9acc
unsigned int g_pcSampleShader;
