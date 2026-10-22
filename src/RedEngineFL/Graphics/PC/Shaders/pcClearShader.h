#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00784c4c
class pcClearShader : public pcRedShader
{
public:

    enum BlendModeT
    {
        BLENDMODE_NONE = 0,
        BLENDMODE_BLEND = 1,
        BLENDMODE_ADD = 2,
        BLENDMODE_MODULATE = 3,
        BLENDMODE_CONSTANT_BLEND = 4,
    };

    pcClearShader(unsigned int uiFlags,
                  const unsigned int* pParams,
                  unsigned int uiNumParams,
                  unsigned int uiMaterialFlags,
                  const RedColor& color0,
                  const RedColor& color1,
                  float fArg7);

    virtual ~pcClearShader()
    {
    }

    // SYNTHETIC: SWBF2 0x00706990
    // pcClearShader::`scalar deleting destructor'

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

    unsigned int GetState_Fixed(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem*, unsigned int) const;

    void SetBlendMode(BlendModeT mode) { m_blendMode = mode; }

    pcRedVertexFormat* m_pVertexFormat;
    RedTexture* m_pTex;
    RedTexture* m_pAlphaMaskTex;
    unsigned int m_uiMaterialFlags;
    BlendModeT m_blendMode;

    // GLOBAL: SWBF2 0x009c9bec
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c9bf0
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9bfc
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9c08
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9c14
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9c88
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009c9c5c
    static pcRedShaderDef s_shaderDef;

    static pcClearShader s_Factory;

private:

    pcClearShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
