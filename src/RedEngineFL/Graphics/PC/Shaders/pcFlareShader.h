#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00784bb8
class pcFlareShader : public pcRedShader
{
public:

    pcFlareShader(unsigned int uiFlags,
                  const unsigned int* pParams,
                  unsigned int uiNumParams,
                  unsigned int uiMaterialFlags,
                  const RedColor& color0,
                  const RedColor& color1,
                  float fArg7);

    virtual ~pcFlareShader()
    {
    }

    // SYNTHETIC: SWBF2 0x00706010
    // pcFlareShader::`scalar deleting destructor'

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
    unsigned int GetState_Prog1(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem*, unsigned int) const;
    void Render_Prog1(const RenderItem*, unsigned int) const;

    void SetTexture(RedTexture* pTexture) { m_texture = pTexture; }

    RedTexture* m_texture;
    unsigned int m_uiMaterialFlags;
    RedColor m_colorDiffuse;
    pcRedVertexFormat* m_pVertexFormat;

    // GLOBAL: SWBF2 0x009c9b50
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c9b54
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9b60
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9b6c
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9b78
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

    static pcFlareShader s_Factory;

private:

    pcFlareShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
 
