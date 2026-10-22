#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00785550
class pcPerPixelDiffuseLightingShader : public pcRedShader
{
public:

    virtual ~pcPerPixelDiffuseLightingShader()
    {
    }

    // SYNTHETIC: SWBF2 0x00711690
    // pcPerPixelDiffuseLightingShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

private:

    // FUNCTION: SWBF2 0x007123d0
    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiNumParams,
                                 unsigned int uiMaterialFlags,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fSpecularPower) const;
    virtual void _Read(PblFileChunk&) const;

    // FUNCTION: SWBF2 0x007118e0
    virtual pcRedShaderDef* GetShaderDef() const;

    // FUNCTION: SWBF2 0x00711970
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

    // GLOBAL: SWBF2 0x009ca8d4
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca8d8
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca8e4
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca8f0
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca8fc
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca938
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009ca90c
    static pcRedShaderDef s_shaderDef;

    // GLOBAL: SWBF2 0x009ca940
    static pcPerPixelDiffuseLightingShader s_Factory;

private:

public:

    // FUNCTION: SWBF2 0x00711620
    pcPerPixelDiffuseLightingShader(unsigned int uiFlags,
                                    const unsigned int* pParams,
                                    unsigned int uiNumParams,
                                    unsigned int uiMaterialFlags,
                                    const RedColor& color0,
                                    const RedColor& color1,
                                    float fSpecularPower);

private:
    // FUNCTION: SWBF2 0x00711560
    pcPerPixelDiffuseLightingShader();

    // FUNCTION: SWBF2 0x00712460
    void _InitPipelineFuncArray();
};
 
