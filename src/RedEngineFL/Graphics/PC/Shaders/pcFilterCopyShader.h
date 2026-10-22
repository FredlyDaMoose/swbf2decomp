#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class RedTexture;
class pcRedVertexFormat;

struct FilterCopyShaderSample
{
    float fValue;
    float fOffsetX;
    float fOffsetY;
};

// VTABLE: SWBF2 0x0078539c
class pcFilterCopyShader : public pcRedShader
{
public:

    pcFilterCopyShader(unsigned int uiFlags,
                       const unsigned int* pParams,
                       unsigned int uiNumParams,
                       unsigned int uiMaterialFlags,
                       const RedColor& color0,
                       const RedColor& color1,
                       float fArg7);

    virtual ~pcFilterCopyShader()
    {
    }

    // SYNTHETIC: SWBF2 0x0070f960
    // pcFilterCopyShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

private:
    virtual void _Read(PblFileChunk& chunk) const;

    virtual void _StaticInit() const;

    virtual void InitFixedFunctionShaderDef() const;

    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiNumParams,
                                 unsigned int uiMaterialFlags,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fArg7) const;

    virtual pcRedShaderDef* GetShaderDef() const;

public:
    virtual void SetTextures(RedTexture* pSrc, RedTexture* pDest);
    virtual void SetSamples(unsigned int uiNumSamples,
                            FilterCopyShaderSample* const pSamples);

    unsigned int GetState_Fixed(const RenderItem*) const;
    unsigned int GetState_Prog1(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem*, unsigned int) const;
    void Render_Prog1(const RenderItem*, unsigned int) const;

    pcRedVertexFormat* m_pVertexFormat;
    RedTexture* m_srcTex;
    RedTexture* m_destTex;
    FilterCopyShaderSample m_samples[16];
    unsigned int m_numSamples;

    mutable unsigned int m_curSample;

    // GLOBAL: SWBF2 0x009ca6d8
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca6dc
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca6e8
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca6f4
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca700
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

    static pcFilterCopyShader s_Factory;

private:

    pcFilterCopyShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
 
