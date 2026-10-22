#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x007850bc
class pcNormalMapAdderShader : public pcRedShader
{
public:

    pcNormalMapAdderShader(unsigned int uiFlags,
                           const unsigned int* pParams,
                           unsigned int uiNumParams,
                           unsigned int uiMaterialFlags,
                           const RedColor& color0,
                           const RedColor& color1,
                           float fArg7);

    void SetNormalMap(RedTexture* pTex1, RedTexture* pTex2, float fBlend);
    void SetTextureTransform(RedTextureTransform* pTransform1,
                             RedTextureTransform* pTransform2);

    virtual ~pcNormalMapAdderShader()
    {
    }

    // SYNTHETIC: SWBF2 0x0070b650
    // pcNormalMapAdderShader::`scalar deleting destructor'

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

    pcRedVertexFormat* m_pVertexFormat[2];
    RedTexture* m_normalMap1;
    RedTexture* m_normalMap2;
    RedTextureTransform m_textureTransform1;
    RedTextureTransform m_textureTransform2;
    float m_blend;
    int m_iShaderState;

    // GLOBAL: SWBF2 0x009ca178
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca17c
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca188
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca194
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca1a0
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

    static pcNormalMapAdderShader s_Factory;

private:

    pcNormalMapAdderShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
 
 
 
