#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00784a94
class pcRainShader : public pcRedShader
{
public:

    pcRainShader(unsigned int uiFlags,
                 const unsigned int* pParams,
                 unsigned int uiNumParams,
                 unsigned int uiMaterialFlags,
                 const RedColor& color0,
                 const RedColor& color1,
                 float fArg7);

    virtual ~pcRainShader()
    {
    }

    // SYNTHETIC: SWBF2 0x00704db0
    // pcRainShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

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

public:

    unsigned int GetState_Fixed(const RenderItem*) const;
    unsigned int GetState_Prog1(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem*, unsigned int) const;
    void Render_Prog1(const RenderItem*, unsigned int) const;

    void SetLineWidth(float fWidth) { m_fLineWidth = fWidth; }
    void SetMaxPixelWidth(float fWidth) { m_fMaxPixelWidth = fWidth; }

    pcRedVertexFormat* m_pVertexFormat;
    unsigned int m_uiMaterialFlags;
    float m_fLineWidth;
    float m_fMaxPixelWidth;

    // GLOBAL: SWBF2 0x009c99f4
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c99f8
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9a04
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9a10
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9a1c
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static pcRainShader s_Factory;

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

private:

    pcRainShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
