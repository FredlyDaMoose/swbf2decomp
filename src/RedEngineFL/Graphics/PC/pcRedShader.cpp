#include "pcRedShader.h"

#include "RedEngineFL/Memory/engineAlloc.h"

#include "pcRedRenderer.h"

#include <stdlib.h>
#include <string.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "pcRedPixelShader.h"
#include "pcRedVertexShader.h"

void __cdecl PblTraceP(const char* Format, ...);

// SYNTHETIC: SWBF2 0x004072e0
// `dynamic initializer for 'pcRedShader::s_ShaderFactoryByRenderType''

PblHashTable<const pcRedShader, pcRedShader::FACTORY_SLOTS>
    pcRedShader::s_ShaderFactoryByRenderType;
int pcRedShader::s_ShaderFactoryByRenderTypeNumEntries;

unsigned int pcRedShaderDef::s_IDCount;

// FUNCTION: SWBF2 0x006ac5c0
pcRedShader::pcRedShader()
{
    m_material = 0;
}

// FUNCTION: SWBF2 0x006ac5e0
pcRedShaderDef::pcRedShaderDef(const char* pcName)
    : m_uiNameHash(PblHash(pcName))
    , m_states(0)
    , m_uiNumStates(0)
    , m_pixelShaderPool(0)
    , m_vertexShaderPool(0)
    , m_uiVertexFormatSupport(0)
    , m_supportedPipeline(PIPELINE_FIXED)
    , m_uiNumShadersToBind(0)
    , m_apShadersToBind(0)
{
}

// FUNCTION: SWBF2 0x006ac640
pcRedShaderDef::pcRedShaderDef()
    : m_uiNameHash(0)
    , m_states(0)
    , m_uiNumStates(0)
    , m_pixelShaderPool(0)
    , m_vertexShaderPool(0)
    , m_uiVertexFormatSupport(0)
    , m_supportedPipeline(PIPELINE_FIXED)
    , m_uiNumShadersToBind(0)
    , m_apShadersToBind(0)
{
}

// FUNCTION: SWBF2 0x006ac6a0
pcRedShaderDef::~pcRedShaderDef()
{
    Reset();
}

// FUNCTION: SWBF2 0x006ac6e0
void pcRedShaderDef::BindOnRead(pcRedShader* pShader)
{
    if ((m_uiNumShadersToBind & 63) == 0)
    {
        pcRedShader** apShaders =
            (pcRedShader**)malloc((m_uiNumShadersToBind + 64) * sizeof(pcRedShader*));
        if (m_uiNumShadersToBind)
        {
            memcpy(apShaders, m_apShadersToBind, m_uiNumShadersToBind * sizeof(pcRedShader*));
            free(m_apShadersToBind);
        }
        m_apShadersToBind = apShaders;
    }
    m_apShadersToBind[m_uiNumShadersToBind++] = pShader;
}

// FUNCTION: SWBF2 0x006ac740
void __cdecl pcRedShader::Cleanup()
{
    const unsigned int* const puiKeys = s_ShaderFactoryByRenderType._table;
    const pcRedShader* const* const ppValues =
        (const pcRedShader* const*)(puiKeys + FACTORY_SLOTS);

    for (unsigned int i = 0; i < FACTORY_SLOTS; ++i)
    {
        if (puiKeys[i] == 0)
        {
            continue;
        }
        ppValues[i]->GetShaderDef()->Reset();
    }

    pcRedShaderDef::Cleanup();
}

// FUNCTION: SWBF2 0x006ac7c0
void __cdecl pcRedShaderDef::Cleanup()
{
    s_IDCount = 0;
}

// FUNCTION: SWBF2 0x006ac7d0
pcRedShader* __cdecl pcRedShader::Create(unsigned int uiNameHash,
                                         unsigned int uiFlags,
                                         const unsigned int* pParams,
                                         unsigned int uiArg3,
                                         unsigned int uiArg4,
                                         const RedColor& color0,
                                         const RedColor& color1,
                                         float fArg7)
{
    const pcRedShader* pFactory = s_ShaderFactoryByRenderType.Find(uiNameHash);
    if (!pFactory)
    {
        return 0;
    }

    return pFactory->_Create(uiFlags, pParams, uiArg3, uiArg4, color0, color1, fArg7);
}

// FUNCTION: SWBF2 0x006ac820
RedShadingState* pcRedShader::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x006ac830
unsigned short pcRedShader::GetShaderChain(pcRedShader** const ppOut,
                                           unsigned short,
                                           const RenderItem*)
{
    *ppOut = this;
    return 1;
}

// FUNCTION: SWBF2 0x006ac850
int pcRedShader::GetDiffuseMapHash(int, int)
{
    return *(const int*)m_material;
}

// FUNCTION: SWBF2 0x006ac860
unsigned int pcRedShader::GetZPrepassState() const
{
    return 0xffffffffu;
}

// FUNCTION: SWBF2 0x006ac870
void __cdecl pcRedShader::Init()
{
    const unsigned int uiPipeline = RedRenderer::pcGetRenderCaps().pipeline;

    pcRedShaderDef::Init();

    const unsigned int* const puiKeys = s_ShaderFactoryByRenderType._table;
    const pcRedShader* const* const ppValues =
        (const pcRedShader* const*)(puiKeys + FACTORY_SLOTS);

    for (unsigned int i = 0; i < FACTORY_SLOTS; ++i)
    {
        if (puiKeys[i] == 0)
        {
            continue;
        }

        const pcRedShader* pFactory = ppValues[i];

        pFactory->_StaticInit();
        if (uiPipeline == 0 || pFactory->GetShaderDef()->m_uiNameHash == 0)
        {
            pFactory->InitFixedFunctionShaderDef();
        }
    }
}

// FUNCTION: SWBF2 0x006ac920
void __cdecl pcRedShaderDef::Init()
{
    s_IDCount = 0;
}

// FUNCTION: SWBF2 0x006ac930
void pcRedShaderDef::InitFixedFunction(unsigned int uiVertexFormatSupport,
                                       unsigned int uiNumStates,
                                       const FixedFunctionStateT* const pStates)
{
    m_uiVertexFormatSupport = uiVertexFormatSupport;
    m_supportedPipeline = PIPELINE_FIXED;
    m_uiNumStates = uiNumStates;
    m_states = new StateT[uiNumStates];

    for (unsigned int i = 0; i < uiNumStates; ++i)
    {
        m_states[i].uiID = ++s_IDCount;
        m_states[i].uiNumPasses = pStates[i].uiNumPasses;

        for (unsigned int p = 0; p < pStates[i].uiNumPasses; ++p)
        {
            m_states[i].pass[p].uiPassCaps = pStates[i].uiPassCaps[p];
            m_states[i].pass[p].apVertexShaders = 0;
            m_states[i].pass[p].pPixelShader = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006ac9f0
bool pcRedShader::IsTransparent() const
{
    return false;
}

// FUNCTION: SWBF2 0x006aca00
void pcRedShaderDef::LoadVertexAndPixelShader(unsigned int uiState, unsigned int uiPass,
                                              unsigned int uiVertexShader) const
{
    if (m_supportedPipeline != PIPELINE_FIXED)
    {
        const PassT& pass = m_states[uiState].pass[uiPass];
        RedRenderer::pcLoadVertexShader(pass.apVertexShaders[uiVertexShader]);
        RedRenderer::pcLoadPixelShader(pass.pPixelShader);
    }
    else
    {
        RedRenderer::pcLoadVertexShader(0);
        RedRenderer::pcLoadPixelShader(0);
    }
}

// FUNCTION: SWBF2 0x006aca70
void __cdecl pcRedShader::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x50595452)
        {
            continue;
        }

        char acName[256];
        child.ReadString(acName, 256);
        const unsigned int uiHash = PblHash(acName);
        PblTraceP("pcRedShader [%s]\tHash 0x%08x\n", acName, uiHash);

        const pcRedShader* pFactory = s_ShaderFactoryByRenderType.Find(uiHash);
        if (pFactory)
        {
            if (RedRenderer::pcGetRenderCaps().pipeline != PIPELINE_FIXED)
            {
                pFactory->_Read(chunk);
            }
        }
        else
        {
            PblTraceP("Error: pcRedShader 0x%08x not found\n", uiHash);
        }
    }
}

// FUNCTION: SWBF2 0x006acb60
void pcRedShaderDef::Read(PblFileChunk& chunk)
{
    unsigned int uiNumPipelines = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x45504950)
        {
            ReadPipeline(child, uiNumPipelines);
        }
        else if (child._iId == 0x4f464e49)
        {
            uiNumPipelines = (unsigned int)child._pFile->Read32();
            m_uiVertexFormatSupport = child._pFile->Read8();
        }
    }

    if (m_uiNumShadersToBind)
    {
        for (unsigned int i = 0; i < m_uiNumShadersToBind; ++i)
        {
            m_apShadersToBind[i]->Bind();
        }

        free(m_apShadersToBind);
        m_apShadersToBind = 0;
        m_uiNumShadersToBind = 0;
    }
}

// FUNCTION: SWBF2 0x006acc20
void pcRedShaderDef::ReadPipeline(PblFileChunk& chunk, unsigned int)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4f464e49:
        {
            const unsigned int uiPipeline = (unsigned int)child._pFile->Read32();
            const PipelineT eCaps = RedRenderer::pcGetRenderCaps().pipeline;
            if (eCaps == PIPELINE_FIXED || (eCaps == PIPELINE_PROG1 && uiPipeline >= PIPELINE_PROG2))
            {
                return;
            }

            m_supportedPipeline = (PipelineT)uiPipeline;
            m_uiNumStates = (unsigned int)child._pFile->Read32();
            m_uiVertexShaderPoolSize = (unsigned int)child._pFile->Read32();
            m_uiPixelShaderPoolSize = (unsigned int)child._pFile->Read32();
            m_states = new StateT[m_uiNumStates];
            m_pixelShaderPool = new pcRedPixelShader*[m_uiPixelShaderPoolSize];
            m_vertexShaderPool = new pcRedVertexShader*[m_uiVertexShaderPoolSize];
            break;
        }
        case 0x54415453:
            ReadState(child);
            break;
        case 0x5f505350:
        {
            int i = 0;
            while (child.HasMoreChildren())
            {
                PblFileChunk program = child.ReadNextChild();
                if (program._iId == 0x5f5f5350)
                {
                    m_pixelShaderPool[i] = new pcRedPixelShader;
                    m_pixelShaderPool[i]->Read(program);
                    ++i;
                }
            }
            break;
        }
        case 0x5f505356:
        {
            int i = 0;
            while (child.HasMoreChildren())
            {
                PblFileChunk program = child.ReadNextChild();
                if (program._iId == 0x5f5f5356)
                {
                    m_vertexShaderPool[i] = new pcRedVertexShader;
                    m_vertexShaderPool[i]->Read(program);
                    ++i;
                }
            }
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x006ace30
void pcRedShaderDef::ReadState(PblFileChunk& chunk)
{
    int iState = -1;
    int iPass = -1;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x4f464e49)
        {
            iState = child._pFile->Read32();
            m_states[iState].uiNumPasses = (unsigned int)child._pFile->Read32();
            m_states[iState].uiID = ++s_IDCount;
        }
        else if (child._iId == 0x53534150)
        {
            while (child.HasMoreChildren())
            {
                PblFileChunk item = child.ReadNextChild();
                if (item._iId == 0x4f464e49)
                {
                    ++iPass;
                    PassT& pass = m_states[iState].pass[iPass];
                    pass.uiPassCaps = 0;
                    pass.apVertexShaders = 0;
                    pass.pPixelShader = 0;
                    pass.uiPassCaps = (unsigned int)item._pFile->Read32();
                    pass.apVertexShaders = new pcRedVertexShader*[255];
                    memset(pass.apVertexShaders, 0, 255 * sizeof(pcRedVertexShader*));
                }
                else if (item._iId == 0x5f535050)
                {
                    m_states[iState].pass[iPass].pPixelShader =
                        m_pixelShaderPool[item._pFile->Read32()];
                }
                else if (item._iId == 0x5f535650)
                {
                    const int iSlot = item._pFile->Read32();
                    const int iProgram = item._pFile->Read32();
                    m_states[iState].pass[iPass].apVertexShaders[iSlot] = m_vertexShaderPool[iProgram];
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x006ad010
void pcRedShader::RenderZPrepass(const RenderItem*) const
{
}

// FUNCTION: SWBF2 0x006ad020
void pcRedShaderDef::Reset()
{
    for (unsigned int i = 0; i < m_uiNumStates; ++i)
    {
        for (unsigned int j = 0; j < m_states[i].uiNumPasses; ++j)
        {
            delete[] m_states[i].pass[j].apVertexShaders;
        }
    }

    for (unsigned int i = 0; i < m_uiPixelShaderPoolSize; ++i)
    {
        delete m_pixelShaderPool[i];
    }

    for (unsigned int i = 0; i < m_uiVertexShaderPoolSize; ++i)
    {
        delete m_vertexShaderPool[i];
    }

    delete[] m_states;
    m_states = 0;
    delete[] m_pixelShaderPool;
    m_pixelShaderPool = 0;
    delete[] m_vertexShaderPool;
    m_vertexShaderPool = 0;
    m_uiNumStates = 0;
    m_uiVertexFormatSupport = 0;
    m_supportedPipeline = PIPELINE_FIXED;
    m_uiNumShadersToBind = 0;
    m_apShadersToBind = 0;
}

// FUNCTION: SWBF2 0x006ad130
void pcRedShader::SetData(const RedShaderData&)
{
}

// FUNCTION: SWBF2 0x006ad140
bool pcRedShader::SortAsZWritingTransparent() const
{
    return false;
}

// FUNCTION: SWBF2 0x006ad150
void pcRedShader::_BindPipeline(const GetStateFuncT* pGetState,
                                const BeginFuncT* pBegin,
                                const EndFuncT* pEnd,
                                const RenderFuncT* pRender)
{
    const int iPipeline = (int)RedRenderer::pcGetRenderCaps().pipeline;

    if (iPipeline != 0 && GetShaderDef()->m_uiNameHash != 0 &&
        GetShaderDef()->m_uiNumStates == 0)
    {
        GetStateFunc = 0;
        BeginFunc = 0;
        EndFunc = 0;
        RenderFunc = 0;
        GetShaderDef()->BindOnRead(this);
        return;
    }

    for (int i = PIPELINE_PROG2; i >= PIPELINE_FIXED; --i)
    {
        if (iPipeline < i || pRender[i] == 0)
        {
            continue;
        }
        if ((int)GetShaderDef()->m_supportedPipeline < i)
        {
            continue;
        }

        GetStateFunc = pGetState[i];
        BeginFunc = pBegin[i];
        EndFunc = pEnd[i];
        RenderFunc = pRender[i];
        return;
    }
}

// FUNCTION: SWBF2 0x006ad200
unsigned int __cdecl pcRedShader::_Register(unsigned int uiHash, const pcRedShader* pFactory)
{
    if (s_ShaderFactoryByRenderType.Store(uiHash, pFactory))
    {
        ++s_ShaderFactoryByRenderTypeNumEntries;
    }

    return (unsigned int)s_ShaderFactoryByRenderTypeNumEntries;
}

// FUNCTION: SWBF2 0x006ad230
void pcRedShader::_StaticInit() const
{
}

// LIBRARY: SWBF2 0x00752ad4
// _vsnprintf

extern unsigned int g_pcNormalShader;
extern unsigned int g_pcInterfaceShader;
extern unsigned int g_pcZPrepassShader;
extern unsigned int g_pcParticleShader;
extern unsigned int g_pcTerrainShader2;
extern unsigned int g_pcSkyFogShader;
extern unsigned int g_pcLightBeamShader;
extern unsigned int g_pcRainShader;
extern unsigned int g_pcSampleShader;
extern unsigned int g_pcFlareShader;
extern unsigned int g_pcClearShader;
extern unsigned int g_pcStencilShadowShader;
extern unsigned int g_pcShadowQuadShader;
extern unsigned int g_pcWaterShader;
extern unsigned int g_pcPreReflectionShader;
extern unsigned int g_pcNormalMapAdderShader;
extern unsigned int g_pcOceanShader;
extern unsigned int g_pcRefractionShader;
extern unsigned int g_pcDecalShader;
extern unsigned int g_pcFilterCopyShader;
extern unsigned int g_pcShieldShader;
extern unsigned int g_pcPerVertexDiffuseLightingShader;
extern unsigned int g_pcPerPixelDiffuseLightingShader;
extern unsigned int g_pcSpecularLightingShader;
extern unsigned int g_pcHDRShader;
extern unsigned int g_pcTerrainShader;

// FUNCTION: SWBF2 0x006ad2c0
static int _link_pcNormalShader() { return ++g_pcNormalShader; }

// GLOBAL: SWBF2 0x007f5040
// SYNTHETIC: SWBF2 0x004073c0
// `dynamic initializer for 's_pcNormalShaderLinked''
static unsigned int s_pcNormalShaderLinked = _link_pcNormalShader();

// FUNCTION: SWBF2 0x006ad290
static int _link_pcInterfaceShader() { return ++g_pcInterfaceShader; }

// GLOBAL: SWBF2 0x007f5858
// SYNTHETIC: SWBF2 0x00407390
// `dynamic initializer for 's_pcInterfaceShaderLinked''
static unsigned int s_pcInterfaceShaderLinked = _link_pcInterfaceShader();

// FUNCTION: SWBF2 0x006ad3d0
static int _link_pcZPrepassShader() { return ++g_pcZPrepassShader; }

// GLOBAL: SWBF2 0x007f5034
// SYNTHETIC: SWBF2 0x004074d0
// `dynamic initializer for 's_pcZPrepassShaderLinked''
static unsigned int s_pcZPrepassShaderLinked = _link_pcZPrepassShader();

// FUNCTION: SWBF2 0x006ad2e0
static int _link_pcParticleShader() { return ++g_pcParticleShader; }

// GLOBAL: SWBF2 0x007f5044
// SYNTHETIC: SWBF2 0x004073e0
// `dynamic initializer for 's_pcParticleShaderLinked''
static unsigned int s_pcParticleShaderLinked = _link_pcParticleShader();

// FUNCTION: SWBF2 0x006ad3a0
static int _link_pcTerrainShader2() { return ++g_pcTerrainShader2; }

// GLOBAL: SWBF2 0x007f504c
// SYNTHETIC: SWBF2 0x004074a0
// `dynamic initializer for 's_pcTerrainShader2Linked''
static unsigned int s_pcTerrainShader2Linked = _link_pcTerrainShader2();

// FUNCTION: SWBF2 0x006ad370
static int _link_pcSkyFogShader() { return ++g_pcSkyFogShader; }

// GLOBAL: SWBF2 0x007f585c
// SYNTHETIC: SWBF2 0x00407470
// `dynamic initializer for 's_pcSkyFogShaderLinked''
static unsigned int s_pcSkyFogShaderLinked = _link_pcSkyFogShader();

// FUNCTION: SWBF2 0x006ad2a0
static int _link_pcLightBeamShader() { return ++g_pcLightBeamShader; }

// GLOBAL: SWBF2 0x007f5030
// SYNTHETIC: SWBF2 0x004073a0
// `dynamic initializer for 's_pcLightBeamShaderLinked''
static unsigned int s_pcLightBeamShaderLinked = _link_pcLightBeamShader();

// FUNCTION: SWBF2 0x006ad320
static int _link_pcRainShader() { return ++g_pcRainShader; }

// GLOBAL: SWBF2 0x007f503c
// SYNTHETIC: SWBF2 0x00407420
// `dynamic initializer for 's_pcRainShaderLinked''
static unsigned int s_pcRainShaderLinked = _link_pcRainShader();

// FUNCTION: SWBF2 0x006ad340
static int _link_pcSampleShader() { return ++g_pcSampleShader; }

// GLOBAL: SWBF2 0x007f5000
// SYNTHETIC: SWBF2 0x00407440
// `dynamic initializer for 's_pcSampleShaderLinked''
static unsigned int s_pcSampleShaderLinked = _link_pcSampleShader();

// FUNCTION: SWBF2 0x006ad270
static int _link_pcFlareShader() { return ++g_pcFlareShader; }

// GLOBAL: SWBF2 0x007f5854
// SYNTHETIC: SWBF2 0x00407370
// `dynamic initializer for 's_pcFlareShaderLinked''
static unsigned int s_pcFlareShaderLinked = _link_pcFlareShader();

// FUNCTION: SWBF2 0x006ad240
static int _link_pcClearShader() { return ++g_pcClearShader; }

// GLOBAL: SWBF2 0x007f5860
// SYNTHETIC: SWBF2 0x00407340
// `dynamic initializer for 's_pcClearShaderLinked''
static unsigned int s_pcClearShaderLinked = _link_pcClearShader();

// FUNCTION: SWBF2 0x006ad390
static int _link_pcStencilShadowShader() { return ++g_pcStencilShadowShader; }

// GLOBAL: SWBF2 0x007f5024
// SYNTHETIC: SWBF2 0x00407490
// `dynamic initializer for 's_pcStencilShadowShaderLinked''
static unsigned int s_pcStencilShadowShaderLinked = _link_pcStencilShadowShader();

// FUNCTION: SWBF2 0x006ad350
static int _link_pcShadowQuadShader() { return ++g_pcShadowQuadShader; }

// GLOBAL: SWBF2 0x007f5014
// SYNTHETIC: SWBF2 0x00407450
// `dynamic initializer for 's_pcShadowQuadShaderLinked''
static unsigned int s_pcShadowQuadShaderLinked = _link_pcShadowQuadShader();

// FUNCTION: SWBF2 0x006ad3c0
static int _link_pcWaterShader() { return ++g_pcWaterShader; }

// GLOBAL: SWBF2 0x007f4ff8
// SYNTHETIC: SWBF2 0x004074c0
// `dynamic initializer for 's_pcWaterShaderLinked''
static unsigned int s_pcWaterShaderLinked = _link_pcWaterShader();

// FUNCTION: SWBF2 0x006ad310
static int _link_pcPreReflectionShader() { return ++g_pcPreReflectionShader; }

// GLOBAL: SWBF2 0x007f502c
// SYNTHETIC: SWBF2 0x00407410
// `dynamic initializer for 's_pcPreReflectionShaderLinked''
static unsigned int s_pcPreReflectionShaderLinked = _link_pcPreReflectionShader();

// FUNCTION: SWBF2 0x006ad2b0
static int _link_pcNormalMapAdderShader() { return ++g_pcNormalMapAdderShader; }

// GLOBAL: SWBF2 0x007f4ff4
// SYNTHETIC: SWBF2 0x004073b0
// `dynamic initializer for 's_pcNormalMapAdderShaderLinked''
static unsigned int s_pcNormalMapAdderShaderLinked = _link_pcNormalMapAdderShader();

// FUNCTION: SWBF2 0x006ad2d0
static int _link_pcOceanShader() { return ++g_pcOceanShader; }

// GLOBAL: SWBF2 0x007f5028
// SYNTHETIC: SWBF2 0x004073d0
// `dynamic initializer for 's_pcOceanShaderLinked''
static unsigned int s_pcOceanShaderLinked = _link_pcOceanShader();

// FUNCTION: SWBF2 0x006ad330
static int _link_pcRefractionShader() { return ++g_pcRefractionShader; }

// GLOBAL: SWBF2 0x007f5020
// SYNTHETIC: SWBF2 0x00407430
// `dynamic initializer for 's_pcRefractionShaderLinked''
static unsigned int s_pcRefractionShaderLinked = _link_pcRefractionShader();

// FUNCTION: SWBF2 0x006ad250
static int _link_pcDecalShader() { return ++g_pcDecalShader; }

// GLOBAL: SWBF2 0x007f501c
// SYNTHETIC: SWBF2 0x00407350
// `dynamic initializer for 's_pcDecalShaderLinked''
static unsigned int s_pcDecalShaderLinked = _link_pcDecalShader();

// FUNCTION: SWBF2 0x006ad260
static int _link_pcFilterCopyShader() { return ++g_pcFilterCopyShader; }

// GLOBAL: SWBF2 0x007f4ff0
// SYNTHETIC: SWBF2 0x00407360
// `dynamic initializer for 's_pcFilterCopyShaderLinked''
static unsigned int s_pcFilterCopyShaderLinked = _link_pcFilterCopyShader();

// FUNCTION: SWBF2 0x006ad360
static int _link_pcShieldShader() { return ++g_pcShieldShader; }

// GLOBAL: SWBF2 0x007f5038
// SYNTHETIC: SWBF2 0x00407460
// `dynamic initializer for 's_pcShieldShaderLinked''
static unsigned int s_pcShieldShaderLinked = _link_pcShieldShader();

// FUNCTION: SWBF2 0x006ad300
static int _link_pcPerVertexDiffuseLightingShader() { return ++g_pcPerVertexDiffuseLightingShader; }

// GLOBAL: SWBF2 0x007f5048
// SYNTHETIC: SWBF2 0x00407400
// `dynamic initializer for 's_pcPerVertexDiffuseLightingShaderLinked''
static unsigned int s_pcPerVertexDiffuseLightingShaderLinked = _link_pcPerVertexDiffuseLightingShader();

// FUNCTION: SWBF2 0x006ad2f0
static int _link_pcPerPixelDiffuseLightingShader() { return ++g_pcPerPixelDiffuseLightingShader; }

// GLOBAL: SWBF2 0x007f5018
// SYNTHETIC: SWBF2 0x004073f0
// `dynamic initializer for 's_pcPerPixelDiffuseLightingShaderLinked''
static unsigned int s_pcPerPixelDiffuseLightingShaderLinked = _link_pcPerPixelDiffuseLightingShader();

// FUNCTION: SWBF2 0x006ad380
static int _link_pcSpecularLightingShader() { return ++g_pcSpecularLightingShader; }

// GLOBAL: SWBF2 0x007f5010
// SYNTHETIC: SWBF2 0x00407480
// `dynamic initializer for 's_pcSpecularLightingShaderLinked''
static unsigned int s_pcSpecularLightingShaderLinked = _link_pcSpecularLightingShader();

// FUNCTION: SWBF2 0x006ad280
static int _link_pcHDRShader() { return ++g_pcHDRShader; }

// GLOBAL: SWBF2 0x007f500c
// SYNTHETIC: SWBF2 0x00407380
// `dynamic initializer for 's_pcHDRShaderLinked''
static unsigned int s_pcHDRShaderLinked = _link_pcHDRShader();

// FUNCTION: SWBF2 0x006ad3b0
static int _link_pcTerrainShader() { return ++g_pcTerrainShader; }

// GLOBAL: SWBF2 0x007f4ffc
// SYNTHETIC: SWBF2 0x004074b0
// `dynamic initializer for 's_pcTerrainShaderLinked''
static unsigned int s_pcTerrainShaderLinked = _link_pcTerrainShader();
