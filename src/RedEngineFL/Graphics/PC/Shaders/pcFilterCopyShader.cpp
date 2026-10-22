#include "pcFilterCopyShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include <string.h>
#include "PebbleFL/Common/PblMatrix.h"

void __cdecl PblTraceP(const char* Format, ...);

class PblMatrix;
extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x007e2110
unsigned int s_numSimultaneousTextures = 1;

unsigned int pcFilterCopyShader::s_uiID;
pcRedShader::GetStateFuncT pcFilterCopyShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcFilterCopyShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcFilterCopyShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcFilterCopyShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009ca720
// SYNTHETIC: SWBF2 0x0040a490
// `dynamic initializer for 'pcFilterCopyShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769fb0
// `dynamic atexit destructor for 'pcFilterCopyShader::s_shaderDef''
pcRedShaderDef pcFilterCopyShader::s_shaderDef("filtercopy_shader.xml");

// FUNCTION: SWBF2 0x0070f810
pcFilterCopyShader::pcFilterCopyShader()
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

// FUNCTION: SWBF2 0x0070f8d0
pcFilterCopyShader::pcFilterCopyShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x0070f990
void pcFilterCopyShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetTransform(D3DTS_VIEW,
                                gMatrixIdentity);
    RedRenderer::pcSetTransform(D3DTS_PROJECTION,
                                gMatrixIdentity);

    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    if (s_numSimultaneousTextures == 1)
    {
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;
    }

    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CONSTANT);

    unsigned int i;
    for (i = 1; i < s_numSimultaneousTextures; ++i)
    {
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 0);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_MULTIPLYADD);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_COLORARG0, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CONSTANT);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CONSTANT);
    }
    if (i < RedRenderer::pcGetDeviceCaps().MaxTextureBlendStages)
    {
        RedRenderer::pcSetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

// FUNCTION: SWBF2 0x0070fba0
void pcFilterCopyShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    for (unsigned int i = 0; i < 4; ++i)
    {
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MAXMIPLEVEL, 0);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    }
}

// FUNCTION: SWBF2 0x0070fd00
void pcFilterCopyShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x0070fd20
void pcFilterCopyShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070fd30
void pcFilterCopyShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070fd40
unsigned int pcFilterCopyShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x0070fd60
RenderPhaseT pcFilterCopyShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x0070fd70
unsigned int pcFilterCopyShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x0070fd80
pcRedShaderDef* pcFilterCopyShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x0070fd90
unsigned int pcFilterCopyShader::GetState_Fixed(const RenderItem*) const
{
    m_curSample = 0;
    return 0;
}

// FUNCTION: SWBF2 0x0070fda0
unsigned int pcFilterCopyShader::GetState_Prog1(const RenderItem*) const
{
    m_curSample = 0;
    if (m_numSamples <= 4)
    {
        return 0;
    }
    return m_numSamples % 4;
}

// GLOBAL: SWBF2 0x00785408
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x0070fdc0
void pcFilterCopyShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x10, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x0070fde0
void pcFilterCopyShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    PblMatrix mTransform = gMatrixIdentity;
    RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();
    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);

    RedRenderer::pcSetRenderTarget(m_destTex, 0);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    while (m_curSample < m_numSamples)
    {
        for (unsigned int i = 0; i < s_numSimultaneousTextures; ++i)
        {
            if (m_curSample < m_numSamples)
            {
                RedRenderer::pcSetTexture(i, m_srcTex);
                const float fValue = m_samples[m_curSample].fValue;
                const D3DCOLOR weight = D3DCOLOR_ARGB((int)(fValue * 255.0f), (int)(fValue * 255.0f),
                                                      (int)(fValue * 255.0f), (int)(fValue * 255.0f));
                if (s_numSimultaneousTextures > 1)
                {
                    RedRenderer::pcSetTextureStageState(i, D3DTSS_CONSTANT, weight);
                }
                else
                {
                    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, weight);
                }
            }
            else
            {
                RedRenderer::pcSetTexture(i, 0);
                if (s_numSimultaneousTextures > 1)
                {
                    RedRenderer::pcSetTextureStageState(i, D3DTSS_CONSTANT, 0);
                }
                else
                {
                    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, 0);
                }
            }
            mTransform.m[2][0] = m_samples[m_curSample].fOffsetX / (float)m_srcTex->m_uiWidth;
            mTransform.m[2][1] = m_samples[m_curSample].fOffsetY / (float)m_srcTex->m_uiHeight;
            mTransform.m[2][2] = 1.0f;
            RedRenderer::pcFFSetTextureTransform(i, mTransform);
            RedRenderer::pcSetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            ++m_curSample;
        }
        pItem->pPrim->Draw();
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    }

    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::pcSetViewport(viewport);
}

// FUNCTION: SWBF2 0x007100a0
void pcFilterCopyShader::Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const
{
    RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();
    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);

    RedRenderer::pcSetRenderTarget(m_destTex, 0);
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, uiPass ? TRUE : FALSE);

    while (m_curSample < m_numSamples)
    {
        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(40, 4);
        for (unsigned int i = 0; i < 4; ++i, ++m_curSample)
        {
            if (m_curSample < m_numSamples)
            {
                RedRenderer::pcSetTexture(i, m_srcTex);
                const float fValue = m_samples[m_curSample].fValue;
                const float afWeight[4] = { fValue, fValue, fValue, fValue };
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(i, afWeight, 1);
                pRegs[40 + i].component[0] =
                    m_samples[m_curSample].fOffsetX / (float)m_srcTex->m_uiWidth;
                pRegs[40 + i].component[1] =
                    m_samples[m_curSample].fOffsetY / (float)m_srcTex->m_uiHeight;
            }
            else
            {
                RedRenderer::pcSetTexture(i, 0);
                const float afZero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                RedRenderer::pcGetDevice()->SetPixelShaderConstantF(i, afZero, 1);
                pRegs[40 + i].component[0] = 0.0f;
                pRegs[40 + i].component[1] = 0.0f;
            }
            pRegs[40 + i].component[2] = 0.0f;
            pRegs[40 + i].component[3] = 0.0f;
        }
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        pItem->pPrim->Draw();
        RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        if (m_numSamples - m_curSample < 4)
        {
            break;
        }
    }

    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::pcSetViewport(viewport);
}

// FUNCTION: SWBF2 0x007102b0
void pcFilterCopyShader::SetSamples(unsigned int uiNumSamples,
                                    FilterCopyShaderSample* const pSamples)
{
    memcpy(m_samples, pSamples, uiNumSamples * sizeof(FilterCopyShaderSample));
    m_numSamples = uiNumSamples;
}

// FUNCTION: SWBF2 0x007102e0
void pcFilterCopyShader::SetTextures(RedTexture* pSrc, RedTexture* pDest)
{
    m_srcTex = pSrc;
    m_destTex = pDest;
}

// FUNCTION: SWBF2 0x00710300
pcRedShader* pcFilterCopyShader::_Create(unsigned int uiFlags,
                                         const unsigned int* pParams,
                                         unsigned int uiNumParams,
                                         unsigned int uiMaterialFlags,
                                         const RedColor& color0,
                                         const RedColor& color1,
                                         float fArg7) const
{
    return new pcFilterCopyShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                  color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00710390
void pcFilterCopyShader::_Init(unsigned int uiFlags,
                               const unsigned int*,
                               unsigned int,
                               unsigned int,
                               const RedColor&,
                               const RedColor&,
                               float)
{
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
    m_srcTex = 0;
    m_destTex = 0;
}

// FUNCTION: SWBF2 0x007103c0
void pcFilterCopyShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcFilterCopyShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcFilterCopyShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcFilterCopyShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcFilterCopyShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcFilterCopyShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcFilterCopyShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcFilterCopyShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcFilterCopyShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00710420
void pcFilterCopyShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00710500
void pcFilterCopyShader::_StaticInit() const
{
    s_numSimultaneousTextures = 1;
    if ((RedRenderer::pcGetDeviceCaps().PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT) &&
        (RedRenderer::pcGetDeviceCaps().TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD))
    {
        s_numSimultaneousTextures = RedRenderer::pcGetDeviceCaps().MaxSimultaneousTextures;
    }
    if (s_numSimultaneousTextures > 2)
    {
        s_numSimultaneousTextures = 2;
    }
}

// GLOBAL: SWBF2 0x009ca71c
// SYNTHETIC: SWBF2 0x0040a4b0
// `dynamic initializer for 'pcFilterCopyShader::s_uiRenderTypeHash''
unsigned int pcFilterCopyShader::s_uiRenderTypeHash = PblHash("FilterCopy");

// SYNTHETIC: SWBF2 0x0040a4d0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a4f0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009ca750
// SYNTHETIC: SWBF2 0x0040a470
// `dynamic initializer for 'pcFilterCopyShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769f90
// `dynamic atexit destructor for 'pcFilterCopyShader::s_Factory''
pcFilterCopyShader pcFilterCopyShader::s_Factory;

// GLOBAL: SWBF2 0x009ca710
unsigned int g_pcFilterCopyShader;
