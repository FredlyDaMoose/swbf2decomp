#include "pcParticleShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"

#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblVector.h"

void __cdecl PblTraceP(const char* Format, ...);

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

// GLOBAL: SWBF2 0x009c9740
// SYNTHETIC: SWBF2 0x00409a70
// `dynamic initializer for 's_blurTexMatrix''

static PblMatrix s_blurTexMatrix;

// GLOBAL: SWBF2 0x007e1b90

static float s_FadeNearDistance = 4.0f;
// GLOBAL: SWBF2 0x007e1b94

static float s_FadeFarDistance = 14.0f;

unsigned int pcParticleShader::s_uiID;
pcRedShader::GetStateFuncT pcParticleShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcParticleShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcParticleShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcParticleShader::RenderFuncArray[MAX_PIPELINES];

// FUNCTION: SWBF2 0x00702d30
pcParticleShader::pcParticleShader()
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

// FUNCTION: SWBF2 0x00702df0
pcParticleShader::pcParticleShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x00703270
void pcParticleShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, 1);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    if (m_blurEnable)
    {
        // GLOBAL: SWBF2 0x009c9880

        static const PblMatrix s_BiasToTextureMatrix(
            PblVector4{ 0.5f, 0.0f, 0.0f, 0.0f },
            PblVector4{ 0.0f, 0.5f, 0.0f, 0.0f },
            PblVector4{ 0.0f, 0.0f, 0.5f, 0.0f },
            PblVector4{ 0.5f, 0.5f, 0.0f, 1.0f });

        const RedCamera* pCamera = RedRenderer::GetCamera();
        PblMatrix mViewProj;
        D3DXMatrixMultiply(&mViewProj, &pCamera->GetMatrixInverse(),
                           &RedRenderer::GetProjectionMatrix());
        PblMatrix mTex;
        D3DXMatrixMultiply(&mTex, &mViewProj, &s_BiasToTextureMatrix);
        s_blurTexMatrix = mTex;

        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
    }
}

// FUNCTION: SWBF2 0x00702eb0
void pcParticleShader::Begin_Fixed(unsigned int uiState, unsigned int) const
{
    RedRenderer::pcSetRenderState(D3DRS_COLORVERTEX, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED |
                                                          D3DCOLORWRITEENABLE_GREEN |
                                                          D3DCOLORWRITEENABLE_BLUE);
    RedRenderer::pcSetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    RedRenderer::pcSetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    RedRenderer::pcSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    if (uiState == 1)
    {
        static const PblMatrix mBias(PblVector4{ 0.5f, 0.0f, 0.0f, 0.0f },
                                     PblVector4{ 0.0f, -0.5f, 0.0f, 0.0f },
                                     PblVector4{ 0.0f, 0.0f, 0.0f, 0.0f },
                                     PblVector4{ 0.5f, 0.5f, 1.0f, 1.0f });
        static PblMatrix mProjection;
        static PblMatrix mView;
        RedRenderer::pcGetDevice()->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&mProjection);
        RedRenderer::pcGetDevice()->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&mView);
        PblMatrix mViewProj;
        D3DXMatrixMultiply(&mViewProj, &mView, &mProjection);
        PblMatrix mTex;
        D3DXMatrixMultiply(&mTex, &mViewProj, &mBias);
        s_blurTexMatrix = mTex;

        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
        RedRenderer::pcSetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;
    }
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    RedRenderer::pcSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
    RedRenderer::pcSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RedRenderer::pcSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

// FUNCTION: SWBF2 0x00703520
void pcParticleShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
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
    RedRenderer::pcSetRenderState(D3DRS_ALPHAREF, (m_uiMaterialFlags & 2) ? 0x7f : 1);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (m_uiMaterialFlags & 0x40000) ? TRUE : FALSE);

    const RedColor color = pItem->tweakColor;
    if (pItem->uiShaderState == 1)
    {
        PblMatrix mView;
        RedRenderer::pcGetDevice()->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&mView);
        PblMatrix mInvView = mView.GetInverseFast();
        mInvView.m[3][0] = 0.0f; mInvView.m[3][1] = 0.0f; mInvView.m[3][2] = 0.0f; mInvView.m[3][3] = 1.0f;
        PblMatrix mInvWorld = pItem->pmObjWorld->GetInverseFast();
        mInvWorld.m[3][0] = 0.0f; mInvWorld.m[3][1] = 0.0f; mInvWorld.m[3][2] = 0.0f; mInvWorld.m[3][3] = 1.0f;
        PblMatrix m1, m2, m3;
        D3DXMatrixMultiply(&m1, &mInvView, &mInvWorld);
        D3DXMatrixMultiply(&m2, &m1, pItem->pmObjWorld);
        D3DXMatrixMultiply(&m3, &m2, &s_blurTexMatrix);
        RedRenderer::pcFFSetTextureTransform(1, m3);
        RedRenderer::pcSetTexture(0, RedTexture::Find(m_uiTextureHash));
        RedRenderer::pcSetTexture(1, m_blurBuffer);
    }
    else
    {
        RedRenderer::pcSetTexture(0, RedTexture::Find(m_uiTextureHash));
    }
    RedRenderer::pcSetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00703440
void pcParticleShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x00703460
void pcParticleShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00703470
void pcParticleShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x00703480
unsigned int pcParticleShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x007034a0
RenderPhaseT pcParticleShader::GetRenderPhase(const RenderItem* pItem, unsigned int) const
{
    if (m_uiMaterialFlags & 0x44)
    {
        return RENDERPHASE_TRANSPARENT;
    }
    return (pItem->tweakColor.a < 0xff) ? RENDERPHASE_TRANSPARENT : RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x007034c0
unsigned int pcParticleShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x007034d0
pcRedShaderDef* pcParticleShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x007034e0
unsigned int pcParticleShader::GetState_Fixed(const RenderItem*) const
{
    return m_blurEnable != 0;
}

// FUNCTION: SWBF2 0x007034f0
unsigned int pcParticleShader::GetState_Prog1(const RenderItem*) const
{
    return m_blurEnable != 0;
}

// GLOBAL: SWBF2 0x007848c8
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[2] =
{
    { 1, { 0x21, 0x00, 0x00, 0x00 } },
    { 1, { 0x41, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x00703500
void pcParticleShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0x14, 2, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x00703710
void pcParticleShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    if (m_shadowEnable)
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        RedRenderer::pcSetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
    else
    {
        RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED |
                                                              D3DCOLORWRITEENABLE_GREEN |
                                                              D3DCOLORWRITEENABLE_BLUE);
        RedRenderer::pcSetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
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
    }
    RedRenderer::pcSetRenderState(D3DRS_ALPHATESTENABLE, (m_uiMaterialFlags & 2) ? TRUE : FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, (m_uiMaterialFlags & 0x40000) ? TRUE : FALSE);

    float fFadeScale = 0.0f;
    float fFadeOffset;
    if (m_nearFadeEnable)
    {
        fFadeScale = 1.0f / (s_FadeFarDistance - s_FadeNearDistance);
        fFadeOffset = -(s_FadeNearDistance * fFadeScale);
    }
    else
    {
        fFadeOffset = 1.0f;
    }

    const RedColor& color = pItem->tweakColor;
    if (m_blurEnable)
    {
        RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(39, 7);
        pRegs[39].component[0] = color.r * (1.0f / 255.0f);
        pRegs[39].component[1] = color.g * (1.0f / 255.0f);
        pRegs[39].component[2] = color.b * (1.0f / 255.0f);
        pRegs[39].component[3] = color.a * (1.0f / 255.0f);
        pRegs[40].component[0] = fFadeScale;
        pRegs[40].component[1] = fFadeOffset;
        pRegs[40].component[2] = 0.0f;
        pRegs[40].component[3] = 0.0f;
        pRegs[41].component[0] = m_fTextureScale;
        pRegs[41].component[1] = m_fTextureScale;
        pRegs[41].component[2] = 0.0f;
        pRegs[41].component[3] = 0.0f;
        PblMatrix mTex;
        D3DXMatrixMultiply(&mTex, pItem->pmObjWorld, &s_blurTexMatrix);
        pRegs[42].SetMatrix4x4(mTex);
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        RedRenderer::pcSetTexture(0, RedTexture::s_textureHashTable.Find(m_uiTextureHash));
        RedRenderer::pcSetTexture(1, m_blurBuffer);
    }
    else
    {
        pcRedVertexShaderConstantRegister* pRegs =
            RedRenderer::pcLockVertexShaderConstants(39, 3);
        pRegs[39].component[0] = color.r * (1.0f / 255.0f);
        pRegs[39].component[1] = color.g * (1.0f / 255.0f);
        pRegs[39].component[2] = color.b * (1.0f / 255.0f);
        pRegs[39].component[3] = color.a * (1.0f / 255.0f);
        pRegs[40].component[0] = fFadeScale;
        pRegs[40].component[1] = fFadeOffset;
        pRegs[40].component[2] = 0.0f;
        pRegs[40].component[3] = 0.0f;
        pRegs[41].component[0] = m_fTextureScale;
        pRegs[41].component[1] = m_fTextureScale;
        pRegs[41].component[2] = 0.0f;
        pRegs[41].component[3] = 0.0f;
        RedRenderer::pcUnlockVertexShaderConstants(pRegs);
        RedRenderer::pcSetTexture(0, RedTexture::s_textureHashTable.Find(m_uiTextureHash));
    }

    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x00703a70
pcRedShader* pcParticleShader::_Create(unsigned int uiFlags,
                                       const unsigned int* pParams,
                                       unsigned int uiNumParams,
                                       unsigned int uiMaterialFlags,
                                       const RedColor& color0,
                                       const RedColor& color1,
                                       float fArg7) const
{
    return new pcParticleShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x00703b00
void pcParticleShader::_Init(unsigned int uiFlags,
                             const unsigned int* pParams,
                             unsigned int uiNumParams,
                             unsigned int uiMaterialFlags,
                             const RedColor&,
                             const RedColor&,
                             float)
{
    m_uiTextureHash = uiNumParams ? pParams[0] : 0;
    m_uiMaterialFlags = uiMaterialFlags;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, uiFlags);
    m_nearFadeEnable = false;
    m_fTextureScale = 1.0f;
    m_blurEnable = false;
    m_blurAmount = 0;
    m_blurScale = 1.0f;
    m_shadowEnable = false;
}

// FUNCTION: SWBF2 0x00703b60
void pcParticleShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcParticleShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcParticleShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcParticleShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcParticleShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcParticleShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcParticleShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcParticleShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcParticleShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x00703bc0
void pcParticleShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x00703ca0
void pcParticleShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009c96ec
// SYNTHETIC: SWBF2 0x004099f0
// `dynamic initializer for 'pcParticleShader::s_uiRenderTypeHash''
unsigned int pcParticleShader::s_uiRenderTypeHash = PblHash("Particle");

// GLOBAL: SWBF2 0x009c96f0
// SYNTHETIC: SWBF2 0x004099b0
// `dynamic initializer for 'pcParticleShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769cc0
// `dynamic atexit destructor for 'pcParticleShader::s_Factory''
pcParticleShader pcParticleShader::s_Factory;

// GLOBAL: SWBF2 0x009c9780
// SYNTHETIC: SWBF2 0x004099d0
// `dynamic initializer for 'pcParticleShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769ce0
// `dynamic atexit destructor for 'pcParticleShader::s_shaderDef''
pcRedShaderDef pcParticleShader::s_shaderDef("particle_shader.xml");

// GLOBAL: SWBF2 0x009c9734
// SYNTHETIC: SWBF2 0x00409a50
// `dynamic initializer for 'kuiLinearRamp''
static unsigned int kuiLinearRamp = PblHash("linear_ramp");

// SYNTHETIC: SWBF2 0x00409a10
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409a30
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c96e8
unsigned int g_pcParticleShader;
