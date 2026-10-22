#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x0078517c
class pcOceanShader : public pcRedShader
{
public:
    pcOceanShader(unsigned int uiFlags,
                  const unsigned int* pParams,
                  unsigned int uiNumParams,
                  unsigned int uiMaterialFlags,
                  const RedColor& color0,
                  const RedColor& color1,
                  float fArg7);

    virtual ~pcOceanShader()
    {
    }

    // SYNTHETIC: SWBF2 0x0070be70
    // pcOceanShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();
    virtual unsigned int GetZPrepassState() const;
    virtual void RenderZPrepass(const RenderItem* pItem) const;

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
    void Begin_Fixed(unsigned int uiState, unsigned int uiPass) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem* pItem, unsigned int) const;
    void Render_Prog1(const RenderItem* pItem, unsigned int) const;

    void SetBaseMap(RedTexture* pTexture, RedTextureTransform* pTransform);
    void SetFoamMap(RedTexture* pTexture, RedTextureTransform* pTransform);

    pcRedVertexFormat* m_pVertexFormat;
    pcRedVertexFormat* m_pZPrepassVertexInputFormat;
    RedTexture* m_reflectionMap;
    RedTexture* m_foamMap;
    RedTexture* m_baseMap;
    RedTextureTransform m_textureTransform;
    RedTextureTransform m_foamMapTransform;
    RedColorValue m_transmissiveColor;
    RedColorValue m_reflectionColor;

    // GLOBAL: SWBF2 0x009ca258
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca25c
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca268
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca274
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca280
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static pcOceanShader s_Factory;

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

private:
    pcOceanShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);
    void _InitPipelineFuncArray();
};
 
 
 
 
 
 
 
