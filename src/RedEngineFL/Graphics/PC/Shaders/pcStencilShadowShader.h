#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00784da4
class pcStencilShadowShader : public pcRedShader
{
public:

    enum MODE
    {
        MODE_CPU_EXTRUSION = 0,
        MODE_GPU_EXTRUSION_VERTEXNORMAL_UNSKINNED = 1,
        MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDHARD = 2,
        MODE_GPU_EXTRUSION_GENERATENORMAL_SKINNEDHARD = 3,
        MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDSOFT = 4,
        MODE_GPU_EXTRUSION_GENERATENORMAL_SKINNEDSOFT = 5,
        NUM_MODES = 6,
        MODE_FORCE_DWORD = 0x7fffffff
    };

    pcStencilShadowShader(unsigned int uiFlags,
                          const unsigned int* pParams,
                          unsigned int uiNumParams,
                          unsigned int uiMaterialFlags,
                          const RedColor& color0,
                          const RedColor& color1,
                          float fArg7);

    virtual ~pcStencilShadowShader()
    {
    }

    // SYNTHETIC: SWBF2 0x007070b0
    // pcStencilShadowShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

    void SetMode(MODE eMode);

private:
    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiNumParams,
                                 unsigned int uiMaterialFlags,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fArg7) const;
    virtual void _Read(PblFileChunk& chunk) const;
    virtual void _StaticInit() const;
    virtual pcRedShaderDef* GetShaderDef() const;
    virtual void InitFixedFunctionShaderDef() const;

    void RenderVolume(const RenderItem* pItem, unsigned int uiPass) const;

public:
    unsigned int GetState_Fixed(const RenderItem*) const;
    unsigned int GetState_Prog1(const RenderItem* pItem) const;
    void Begin_Fixed(unsigned int, unsigned int uiPass) const;
    void Begin_Prog1(unsigned int, unsigned int uiPass) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const;
    void Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const;

    pcRedVertexFormat* m_pVertexFormat;
    MODE m_mode;

    // GLOBAL: SWBF2 0x009c9c8c
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c9c90
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9c9c
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9ca8
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9cb4
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9cc0
    static pcRedVertexFormat* s_pSkinnedInputFormat[4];

    // GLOBAL: SWBF2 0x009c9d14
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009c9ce0
    static pcRedShaderDef s_shaderDef;

    // GLOBAL: SWBF2 0x009c9d18
    static pcStencilShadowShader s_Factory;

    // GLOBAL: SWBF2 0x007e1d80
    static unsigned int s_uiStencilMask;

private:

    pcStencilShadowShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
public:
    static const unsigned int s_stencilRef;
};
 
 
 
