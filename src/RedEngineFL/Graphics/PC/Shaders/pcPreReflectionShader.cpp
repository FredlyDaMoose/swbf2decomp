#include "pcPreReflectionShader.h"

#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexFormat.h"

void __cdecl PblTraceP(const char* Format, ...);

unsigned int pcPreReflectionShader::s_uiID;
pcRedShader::GetStateFuncT pcPreReflectionShader::GetStateFuncArray[MAX_PIPELINES];
pcRedShader::BeginFuncT pcPreReflectionShader::BeginFuncArray[MAX_PIPELINES];
pcRedShader::EndFuncT pcPreReflectionShader::EndFuncArray[MAX_PIPELINES];
pcRedShader::RenderFuncT pcPreReflectionShader::RenderFuncArray[MAX_PIPELINES];

// GLOBAL: SWBF2 0x009ca11c
// SYNTHETIC: SWBF2 0x0040a0b0
// `dynamic initializer for 'pcPreReflectionShader::s_shaderDef''
// SYNTHETIC: SWBF2 0x00769ec0
// `dynamic atexit destructor for 'pcPreReflectionShader::s_shaderDef''
pcRedShaderDef pcPreReflectionShader::s_shaderDef("prereflection_shader.xml");

// FUNCTION: SWBF2 0x0070afc0
pcPreReflectionShader::pcPreReflectionShader()
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

// FUNCTION: SWBF2 0x0070b080
pcPreReflectionShader::pcPreReflectionShader(unsigned int uiFlags,
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

// FUNCTION: SWBF2 0x0070b140
void pcPreReflectionShader::Begin_Fixed(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
}

// FUNCTION: SWBF2 0x0070b180
void pcPreReflectionShader::Begin_Prog1(unsigned int, unsigned int) const
{
    RedRenderer::pcSetCullMode(PCREDCULL_NONE);
    RedRenderer::pcSetRenderState(D3DRS_FOGENABLE, FALSE);
    RedRenderer::pcSetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    RedRenderer::pcSetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0);
}

// FUNCTION: SWBF2 0x0070b1c0
void pcPreReflectionShader::Bind()
{
    _BindPipeline(GetStateFuncArray, BeginFuncArray, EndFuncArray, RenderFuncArray);
}

// FUNCTION: SWBF2 0x0070b1e0
void pcPreReflectionShader::End_Fixed(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070b1f0
void pcPreReflectionShader::End_Prog1(unsigned int, unsigned int) const
{
}

// FUNCTION: SWBF2 0x0070b200
unsigned int pcPreReflectionShader::GetID(unsigned int uiPass) const
{
    return s_shaderDef.m_states[uiPass].uiID;
}

// FUNCTION: SWBF2 0x0070b220
RenderPhaseT pcPreReflectionShader::GetRenderPhase(const RenderItem*, unsigned int) const
{
    return RENDERPHASE_OPAQUE;
}

// FUNCTION: SWBF2 0x0070b230
unsigned int pcPreReflectionShader::GetRenderTypeHash() const
{
    return s_uiRenderTypeHash;
}

// FUNCTION: SWBF2 0x0070b240
pcRedShaderDef* pcPreReflectionShader::GetShaderDef() const
{
    return &s_shaderDef;
}

// FUNCTION: SWBF2 0x0070b250
unsigned int pcPreReflectionShader::GetState_Fixed(const RenderItem*) const
{
    return 1;
}

// FUNCTION: SWBF2 0x0070b260
unsigned int pcPreReflectionShader::GetState_Prog1(const RenderItem*) const
{
    return 1;
}

// GLOBAL: SWBF2 0x00785090
static const pcRedShaderDef::FixedFunctionStateT kFixedFunctionStates[1] =
{
    { 1, { 0x11, 0x00, 0x00, 0x00 } },
};

// FUNCTION: SWBF2 0x0070b270
void pcPreReflectionShader::InitFixedFunctionShaderDef() const
{
    GetShaderDef()->InitFixedFunction(0, 1, kFixedFunctionStates);
}

// FUNCTION: SWBF2 0x0070b290
void pcPreReflectionShader::Render_Fixed(const RenderItem* pItem, unsigned int) const
{
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070b2b0
void pcPreReflectionShader::Render_Prog1(const RenderItem* pItem, unsigned int) const
{
    RedRenderer::pcLoadFormat(m_pVertexFormat);
    pItem->pPrim->Draw();
}

// FUNCTION: SWBF2 0x0070b2d0
pcRedShader* pcPreReflectionShader::_Create(unsigned int uiFlags,
                                            const unsigned int* pParams,
                                            unsigned int uiNumParams,
                                            unsigned int uiMaterialFlags,
                                            const RedColor& color0,
                                            const RedColor& color1,
                                            float fArg7) const
{
    return new pcPreReflectionShader(uiFlags, pParams, uiNumParams, uiMaterialFlags,
                                     color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x0070b360
void pcPreReflectionShader::_Init(unsigned int uiFlags,
                                  const unsigned int*,
                                  unsigned int,
                                  unsigned int uiMaterialFlags,
                                  const RedColor&,
                                  const RedColor&,
                                  float)
{
    m_uiMaterialFlags = uiMaterialFlags;
    m_pVertexFormat = pcRedVertexFormat::Create(uiFlags, 2);
}

// FUNCTION: SWBF2 0x0070b390
void pcPreReflectionShader::_InitPipelineFuncArray()
{
    GetStateFuncArray[PIPELINE_FIXED] =
        static_cast<GetStateFuncT>(&pcPreReflectionShader::GetState_Fixed);
    BeginFuncArray[PIPELINE_FIXED] =
        static_cast<BeginFuncT>(&pcPreReflectionShader::Begin_Fixed);
    EndFuncArray[PIPELINE_FIXED] =
        static_cast<EndFuncT>(&pcPreReflectionShader::End_Fixed);
    RenderFuncArray[PIPELINE_FIXED] =
        static_cast<RenderFuncT>(&pcPreReflectionShader::Render_Fixed);

    GetStateFuncArray[PIPELINE_PROG1] =
        static_cast<GetStateFuncT>(&pcPreReflectionShader::GetState_Prog1);
    BeginFuncArray[PIPELINE_PROG1] =
        static_cast<BeginFuncT>(&pcPreReflectionShader::Begin_Prog1);
    EndFuncArray[PIPELINE_PROG1] =
        static_cast<EndFuncT>(&pcPreReflectionShader::End_Prog1);
    RenderFuncArray[PIPELINE_PROG1] =
        static_cast<RenderFuncT>(&pcPreReflectionShader::Render_Prog1);
}

// FUNCTION: SWBF2 0x0070b3f0
void pcPreReflectionShader::_Read(PblFileChunk& chunk) const
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

// FUNCTION: SWBF2 0x0070b4d0
void pcPreReflectionShader::_StaticInit() const
{
}

// GLOBAL: SWBF2 0x009ca170
// SYNTHETIC: SWBF2 0x0040a0d0
// `dynamic initializer for 'pcPreReflectionShader::s_uiRenderTypeHash''
unsigned int pcPreReflectionShader::s_uiRenderTypeHash = PblHash("PreReflection");

// SYNTHETIC: SWBF2 0x0040a0f0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x0040a110
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009ca148
// SYNTHETIC: SWBF2 0x0040a090
// `dynamic initializer for 'pcPreReflectionShader::s_Factory''
// SYNTHETIC: SWBF2 0x00769ea0
// `dynamic atexit destructor for 'pcPreReflectionShader::s_Factory''
pcPreReflectionShader pcPreReflectionShader::s_Factory;

// GLOBAL: SWBF2 0x009ca110
unsigned int g_pcPreReflectionShader;
