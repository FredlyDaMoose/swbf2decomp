#include "pcInterfaceShader.h"

#include "PebbleFL/Common/PblChunk.h"

#include "RedEngineFL/Memory/engineAlloc.h"

#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "PebbleFL/Common/PblMatrix.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcInterfaceShader::s_uiID;
pcRedShader::GetStateFuncT pcInterfaceShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcInterfaceShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcInterfaceShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcInterfaceShader::RenderFuncArray[MAX_PIPELINES];
pcRedVertexFormat* pcInterfaceShader::s_pBitmapTexVertexFormat;
pcRedVertexFormat* pcInterfaceShader::s_pBitmapPosVertexFormat;
pcRedVertexFormat* pcInterfaceShader::s_pMaskedVertexFormat;
pcRedVertexFormat* pcInterfaceShader::s_pVectorVertexFormat;

// FUNCTION: SWBF2 0x00701780
pcInterfaceShader::pcInterfaceShader()
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

// FUNCTION: SWBF2 0x00701840
pcInterfaceShader::pcInterfaceShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00701900
void pcInterfaceShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
}

// FUNCTION: SWBF2 0x007019b0
void pcInterfaceShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ZENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

// FUNCTION: SWBF2 0x00701a60
void pcInterfaceShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00701a80
void pcInterfaceShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00701a90
void pcInterfaceShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00701aa0
unsigned int pcInterfaceShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x00701ac0
RenderPhaseT pcInterfaceShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_TRANSPARENT;
}

// FUNCTION: SWBF2 0x00701ad0
unsigned int pcInterfaceShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x00701ae0
pcRedShaderDef* pcInterfaceShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x00701af0
unsigned int pcInterfaceShader::GetState_Fixed(const RenderItem*) const
{
    if (m_mode == INTERFACE_MODE_BITMAP && m_textureHash)
    {
        return 3;
    }
    return m_mode;
}

// FUNCTION: SWBF2 0x00701b10
unsigned int pcInterfaceShader::GetState_Prog1(const RenderItem*) const
{
    if (m_mode == INTERFACE_MODE_BITMAP && m_textureHash)
    {
        return 3;
    }
    return m_mode;
}

// GLOBAL: SWBF2 0x00784760
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[4] =
{
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00701b30
void pcInterfaceShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x28, 4, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00701b50
void pcInterfaceShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    if (!pItem)
    {
        return;
    }

    RedRenderer::pcSetTexture(0, m_textureHash ? RedTexture::Find(m_textureHash) : 0);
    RedRenderer::pcSetTexture(1, m_bitmapMaskHash ? RedTexture::Find(m_bitmapMaskHash) : 0);
    RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, (m_materialFlags & 0x40) ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);

    switch (m_mode)
    {
    case INTERFACE_MODE_BITMAP:
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, FALSE);
        RedRenderer::pcLoadFormat(m_textureHash ? s_pBitmapTexVertexFormat : s_pBitmapPosVertexFormat);
        if (m_textureHash)
        {
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
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case INTERFACE_MODE_VECTOR:
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
        RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
        RedRenderer::pcLoadFormat(s_pVectorVertexFormat);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case INTERFACE_MODE_MASKEDBITMAP:
        RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, FALSE);
        RedRenderer::pcLoadFormat(s_pMaskedVertexFormat);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;
    }

    if (m_shadowEnable)
    {
        PblMatrix mShadow = *pItem->pmObjWorld;
        mShadow.m[3][0] += 1.5f;
        mShadow.m[3][1] -= 1.5f;
        RedRenderer::pcSetTransform(D3DTS_WORLD, mShadow);
        RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                                      D3DCOLOR_ARGB(pItem->tweakColor.a >> 1, RedColor::BLACK.r,
                                                    RedColor::BLACK.g, RedColor::BLACK.b));
        pItem->pPrim->Draw();
    }
    RedRenderer::pcSetTransform(D3DTS_WORLD, *pItem->pmObjWorld);
    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR,
                                  D3DCOLOR_ARGB(pItem->tweakColor.a, pItem->tweakColor.r,
                                                pItem->tweakColor.g, pItem->tweakColor.b));
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00701ee0
void pcInterfaceShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);

    if (m_materialFlags & 0x40)
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    RedRenderer::pcSetTexture(0, m_textureHash ? RedTexture::Find(m_textureHash) : 0);
    RedRenderer::pcSetTexture(1, m_bitmapMaskHash ? RedTexture::Find(m_bitmapMaskHash) : 0);

    switch (m_mode)
    {
    case INTERFACE_MODE_MASKEDBITMAP:
        RedRenderer::pcLoadFormat(s_pMaskedVertexFormat);
        break;
    case INTERFACE_MODE_VECTOR:
        RedRenderer::pcLoadFormat(s_pVectorVertexFormat);
        break;
    case INTERFACE_MODE_BITMAP:
        RedRenderer::pcLoadFormat(m_textureHash ? s_pBitmapTexVertexFormat
                                                : s_pBitmapPosVertexFormat);
        break;
    }

    if (m_shadowEnable)
    {
        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(40, 3);
        pRegs[40].component[0] = 1.0f;
        pRegs[40].component[1] = 1.0f;
        pRegs[40].component[2] = 2.0f;
        pRegs[40].component[3] = -2.0f;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);

        float shadowColor[4];
        shadowColor[0] = RedColor::BLACK.r * (1.0f / 255.0f);
        shadowColor[1] = RedColor::BLACK.g * (1.0f / 255.0f);
        shadowColor[2] = RedColor::BLACK.b * (1.0f / 255.0f);
        shadowColor[3] = (unsigned char)(pItem->tweakColor.a >> 1) * (1.0f / 255.0f);
        RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, shadowColor, 1);
        pItem->pPrim->Draw();
    }

    pcRedVertexShaderConstantRegister* pRegs =
        RedRenderer::pcLockVertexShaderConstants(40, 3);
    pRegs[40].component[0] = 1.0f;
    pRegs[40].component[1] = 1.0f;
    pRegs[40].component[2] = 0.0f;
    pRegs[40].component[3] = 0.0f;
    RedRenderer::pcUnlockVertexShaderConstants(pRegs);

    const RedColor& c = pItem->tweakColor;
    float color[4];
    color[0] = c.r * (1.0f / 255.0f);
    color[1] = c.g * (1.0f / 255.0f);
    color[2] = c.b * (1.0f / 255.0f);
    color[3] = c.a * (1.0f / 255.0f);
    RedRenderer::pcGetDevice()->SetPixelShaderConstantF(0, color, 1);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00702120
pcRedShader* pcInterfaceShader::_Create(unsigned int uiFlags,
                                        const unsigned int* pParams,
                                        unsigned int uiNumParams,
                                        unsigned int uiMaterialFlags,
                                        const RedColor& color0,
                                        const RedColor& color1,
                                        float fArg7) const
{
    return new pcInterfaceShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                 color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x007021b0
void pcInterfaceShader::_Init(unsigned int,
                              const unsigned int* pParams,
                              unsigned int uiNumParams,
                              unsigned int uiMaterialFlags,
                              const RedColor&,
                              const RedColor&,
                              float)
{
    m_textureHash = (uiNumParams > 0) ? pParams[0] : 0;
    m_bitmapMaskHash = (uiNumParams > 1) ? pParams[1] : 0;
    m_shadowEnable = false;
    m_mode = INTERFACE_MODE_BITMAP;
    m_materialFlags = uiMaterialFlags;
}

// FUNCTION: SWBF2 0x007021f0
void pcInterfaceShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcInterfaceShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcInterfaceShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcInterfaceShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcInterfaceShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcInterfaceShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcInterfaceShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcInterfaceShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcInterfaceShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00702250
void pcInterfaceShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00702330
void pcInterfaceShader::_StaticInit() const
{
    s_pBitmapTexVertexFormat = pcRedVertexFormat::Create(0x202, 0x202);
    s_pBitmapPosVertexFormat = pcRedVertexFormat::Create(0x202, 2);
    s_pMaskedVertexFormat = pcRedVertexFormat::Create(0x402, 0x402);
    s_pVectorVertexFormat = pcRedVertexFormat::Create(0x82, 0x82);
}

// SYNTHETIC: SWBF2 0x004098b0
// `dynamic initializer for 'pcInterfaceShader::s_uiRenderTypeHash''
unsigned int pcInterfaceShader::s_uiRenderTypeHash = PblHash("Interface");

// SYNTHETIC: SWBF2 0x00409870
// `dynamic initializer for 'pcInterfaceShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769c60
// `dynamic atexit destructor for 'pcInterfaceShader::s_Factory''
pcInterfaceShader pcInterfaceShader::s_Factory;

// SYNTHETIC: SWBF2 0x00409890
// `dynamic initializer for 'pcInterfaceShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769c80
// `dynamic atexit destructor for 'pcInterfaceShader::s_shaderDef''
pcRedShaderDef pcInterfaceShader::s_shaderDef("interface_shader.xml");

// SYNTHETIC: SWBF2 0x004098f0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// SYNTHETIC: SWBF2 0x004098d0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// GLOBAL: SWBF2 0x009c9538
unsigned int g_pcInterfaceShader;
