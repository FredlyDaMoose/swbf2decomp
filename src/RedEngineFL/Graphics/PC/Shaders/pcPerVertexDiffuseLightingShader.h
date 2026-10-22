#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00785434
class pcPerVertexDiffuseLightingShader : public pcRedShader
{
public:

    virtual ~pcPerVertexDiffuseLightingShader()
    {
    }

    // SYNTHETIC: SWBF2 0x007106b0
    // pcPerVertexDiffuseLightingShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

private:

    // FUNCTION: SWBF2 0x00711380
    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiNumParams,
                                 unsigned int uiMaterialFlags,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fSpecularPower) const;
    virtual void _Read(PblFileChunk&) const;

    // FUNCTION: SWBF2 0x00710900
    virtual pcRedShaderDef* GetShaderDef() const;

    // FUNCTION: SWBF2 0x00710940
    virtual void InitFixedFunctionShaderDef() const;

public:

    unsigned int GetState_Fixed(const RenderItem*) const;
    unsigned int GetState_Prog1(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem*, unsigned int) const;
    void Render_Prog1(const RenderItem*, unsigned int) const;

    mutable void* m_pLightingStatePrev;

    // GLOBAL: SWBF2 0x009ca840
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca844
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca850
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca85c
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca868
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca87c
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009ca8a8
    static pcRedShaderDef s_shaderDef;

    // GLOBAL: SWBF2 0x009ca888
    static pcPerVertexDiffuseLightingShader s_Factory;

private:

public:

    // FUNCTION: SWBF2 0x00710620
    pcPerVertexDiffuseLightingShader(unsigned int uiFlags,
                                     const unsigned int* pParams,
                                     unsigned int uiNumParams,
                                     unsigned int uiMaterialFlags,
                                     const RedColor& color0,
                                     const RedColor& color1,
                                     float fSpecularPower);

private:
    // FUNCTION: SWBF2 0x00710560
    pcPerVertexDiffuseLightingShader();

    // FUNCTION: SWBF2 0x00711410
    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fSpecularPower);

    // FUNCTION: SWBF2 0x00711420
    void _InitPipelineFuncArray();
};
 
 
