#ifndef SWBF2_GRAPHICS_PC_PCREDSHADER_H
#define SWBF2_GRAPHICS_PC_PCREDSHADER_H

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"

#include "pcRedAutoSettings.h"

class PblFileChunk;
class RedShadingState;
class pcRedPixelShader;
class pcRedShader;
class pcRedShaderDef;
class pcRedVertexFormat;
class pcRedVertexShader;
struct RedMaterial;
struct RedShaderData;
struct RenderItem;

class PblMatrix;
class pcRedPrimitive;
struct RedLightingState;
struct RenderItem
{
    pcRedPrimitive* pPrim;
    pcRedShader* pRootShader;
    pcRedShader* pShaderChain[3];
    unsigned short uiShaderChainSize;
    unsigned short uiCurrentShader;
    unsigned int uiShaderState;
    PblMatrix* pmObjWorld;
    PblMatrix* pBoneMatrices;
    unsigned short numBoneMatrices;
    RedColor tweakColor;
    RedShadingState* pShadingState;
    unsigned int flags;
    IDirect3DQuery9* pVisibilityQuery;
    RedLightingState* pLightingState;
    unsigned int uiEnvState;
    float fStencilShadowLength;
    unsigned int unknown44;

    RenderItem();

    void Free();

    ~RenderItem();
};
 
 
 

 
 
 
 
 
 
 
 
 
 
 
 
 
 

enum RenderPhaseT
{
    RENDERPHASE_OPAQUE = 1,
    RENDERPHASE_PRESHADOW_TRANSPARENT = 2,
    RENDERPHASE_SHADOW = 4,
    RENDERPHASE_POSTSHADOW = 8,
    RENDERPHASE_TRANSPARENT = 16,
    RENDERPHASE_POSTTRANSPARENT = 32,
    RENDERPHASE_ALL = 63,
    RENDERPHASE_FORCE_DWORD = 2147483647
};

class pcRedShaderDef
{
public:

    enum { MAX_PASSES = 4 };

    pcRedShaderDef();
    pcRedShaderDef(const char* pcName);
    ~pcRedShaderDef();

    struct PassT
    {
        unsigned int uiPassCaps;
        pcRedVertexShader** apVertexShaders;
        pcRedPixelShader* pPixelShader;
    };

    struct StateT
    {
        unsigned int uiID;
        unsigned int uiNumPasses;
        PassT pass[MAX_PASSES];
    };

    struct FixedFunctionStateT
    {
        unsigned int uiNumPasses;
        unsigned int uiPassCaps[MAX_PASSES];
    };

    // FUNCTION: SWBF2 0x006ac930
    void InitFixedFunction(unsigned int uiVertexFormatSupport,
                           unsigned int uiNumStates,
                           const FixedFunctionStateT* const pStates);

    void BindOnRead(pcRedShader* pShader);

    void LoadVertexAndPixelShader(unsigned int uiState, unsigned int uiPass,
                                  unsigned int uiVertexShader) const;

    void Reset();

    static void __cdecl Cleanup();

    void Read(PblFileChunk& chunk);

private:

    void ReadPipeline(PblFileChunk& chunk, unsigned int uiNumPipelines);

    void ReadState(PblFileChunk& chunk);

public:
    unsigned int m_uiNameHash;
    StateT* m_states;
    unsigned int m_uiNumStates;
    unsigned int m_uiVertexShaderPoolSize;
    unsigned int m_uiPixelShaderPoolSize;
    pcRedPixelShader** m_pixelShaderPool;
    pcRedVertexShader** m_vertexShaderPool;
    unsigned int m_uiVertexFormatSupport;
    PipelineT m_supportedPipeline;
    unsigned int m_uiNumShadersToBind;
    pcRedShader** m_apShadersToBind;

    // GLOBAL: SWBF2 0x007f4fe8
    static unsigned int s_IDCount;

    // FUNCTION: SWBF2 0x006ac920
    static void __cdecl Init();
};
 
 
 
 
 
 

// VTABLE: SWBF2 0x00771bd8
class pcRedShader : public PblRef
{

    friend class RedRenderer;

public:

    typedef unsigned int (pcRedShader::*GetStateFuncT)(const RenderItem*) const;
    typedef void (pcRedShader::*BeginFuncT)(unsigned int, unsigned int) const;
    typedef void (pcRedShader::*EndFuncT)(unsigned int, unsigned int) const;
    typedef void (pcRedShader::*RenderFuncT)(const RenderItem*, unsigned int) const;

    // FUNCTION: SWBF2 0x006ac5c0
    pcRedShader();

    virtual ~pcRedShader()
    {
    }

    // SYNTHETIC: SWBF2 0x006ac6b0
    // pcRedShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const = 0;
    virtual unsigned int GetID(unsigned int) const = 0;
    virtual unsigned int GetRenderTypeHash() const = 0;

    virtual RedShadingState* CreateShadingState() const;

    virtual void SetData(const RedShaderData&);

    virtual void Bind() = 0;

    virtual unsigned int GetZPrepassState() const;

    virtual void RenderZPrepass(const RenderItem*) const;

    virtual bool SortAsZWritingTransparent() const;
    virtual bool IsTransparent() const;

    virtual unsigned short GetShaderChain(pcRedShader** const ppOut,
                                          unsigned short usCount,
                                          const RenderItem*);

    virtual int GetDiffuseMapHash(int, int);

    unsigned int GetPassCaps(unsigned int uiState, unsigned int uiPass) const;

    // FUNCTION: SWBF2 0x006ac870
    static void __cdecl Init();

protected:

    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiArg3,
                                 unsigned int uiArg4,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fArg7) const = 0;

    virtual void _Read(PblFileChunk&) const = 0;

    virtual void _StaticInit() const;

    virtual pcRedShaderDef* GetShaderDef() const = 0;
    virtual void InitFixedFunctionShaderDef() const = 0;

public:

    // FUNCTION: SWBF2 0x006ac7d0
    static pcRedShader* __cdecl Create(unsigned int uiNameHash,
                                       unsigned int uiFlags,
                                       const unsigned int* pParams,
                                       unsigned int uiArg3,
                                       unsigned int uiArg4,
                                       const RedColor& color0,
                                       const RedColor& color1,
                                       float fArg7);

    enum { FACTORY_SLOTS = 256 };

    // GLOBAL: SWBF2 0x007f5054
    static PblHashTable<const pcRedShader, FACTORY_SLOTS> s_ShaderFactoryByRenderType;

    // GLOBAL: SWBF2 0x007f5050
    static int s_ShaderFactoryByRenderTypeNumEntries;

protected:

    // FUNCTION: SWBF2 0x006ad200
    static unsigned int __cdecl _Register(unsigned int uiHash, const pcRedShader* pFactory);

    // FUNCTION: SWBF2 0x006ad150
    void _BindPipeline(const GetStateFuncT* pGetState,
                       const BeginFuncT* pBegin,
                       const EndFuncT* pEnd,
                       const RenderFuncT* pRender);

public:

    static void __cdecl Read(PblFileChunk& chunk);
    static void __cdecl Cleanup();

    GetStateFuncT GetStateFunc;
    BeginFuncT BeginFunc;
    EndFuncT EndFunc;
    RenderFuncT RenderFunc;

    RedMaterial* m_material;
};

  
 
 
 
 

#endif
