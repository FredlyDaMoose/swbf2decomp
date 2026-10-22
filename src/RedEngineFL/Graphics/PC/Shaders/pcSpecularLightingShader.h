#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00785660
class pcSpecularLightingShader : public pcRedShader
{
public:

    virtual ~pcSpecularLightingShader()
    {
    }

    // SYNTHETIC: SWBF2 0x007126d0
    // pcSpecularLightingShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

private:

    // FUNCTION: SWBF2 0x007136e0
    virtual pcRedShader* _Create(unsigned int uiFlags,
                               const unsigned int* pParams,
                               unsigned int uiNumParams,
                               unsigned int uiMaterialFlags,
                               const RedColor& color0,
                               const RedColor& color1,
                                 float fSpecularPower) const;
    virtual void _Read(PblFileChunk&) const;

    // FUNCTION: SWBF2 0x007129b0
    virtual pcRedShaderDef* GetShaderDef() const;

    // FUNCTION: SWBF2 0x00712b80
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

private:
    void PerVertexLighting(const RenderItem* pItem, unsigned int uiPass) const;
    void PerPixelLighting(const RenderItem* pItem, unsigned int uiPass) const;
public:

    // GLOBAL: SWBF2 0x009ca970
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca974
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca980
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca98c
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca998
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009caa0c
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009ca9dc
    static pcRedShaderDef s_shaderDef;

    // GLOBAL: SWBF2 0x009ca9a8
    static pcSpecularLightingShader s_Factory;

private:

public:

    // FUNCTION: SWBF2 0x00712660
    pcSpecularLightingShader(unsigned int uiFlags,
                             const unsigned int* pParams,
                             unsigned int uiNumParams,
                             unsigned int uiMaterialFlags,
                             const RedColor& color0,
                             const RedColor& color1,
                             float fSpecularPower);

private:
    // FUNCTION: SWBF2 0x007125a0
    pcSpecularLightingShader();

    // FUNCTION: SWBF2 0x00713770
    void _InitPipelineFuncArray();
};
 
