#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedColor.h"

class RedTexture;
class RedTextureTransform;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00785224
class pcRefractionShader : public pcRedShader
{
public:

    pcRefractionShader(unsigned int uiFlags,
                       const unsigned int* pParams,
                       unsigned int uiNumParams,
                       unsigned int uiMaterialFlags,
                       const RedColor& color0,
                       const RedColor& color1,
                       float fSpecularPower);

    virtual ~pcRefractionShader()
    {
    }

    // SYNTHETIC: SWBF2 0x0070cc60
    // pcRefractionShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual RedShadingState* CreateShadingState() const;
    virtual void Bind();
    virtual unsigned int GetZPrepassState() const;
    virtual void RenderZPrepass(const RenderItem*) const;
    virtual bool SortAsZWritingTransparent() const;
    virtual bool IsTransparent() const;
    virtual unsigned short GetShaderChain(pcRedShader** const ppOut,
                                          unsigned short usCount,
                                          const RenderItem*);

    virtual int SetDiffuseMap(unsigned int, RedTextureTransform*);
    virtual int SetBumpMap(unsigned int, RedTextureTransform*);
    virtual int SetRefractionMap(RedTexture*);
    virtual int SetDistortionScale(float);

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
                                 float fSpecularPower) const;

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

    pcRedVertexFormat* m_pVertexInputFormat;
    pcRedVertexFormat* m_pZPrepassVertexInputFormat;
    RedTexture* m_refractionMap;
    RedTextureTransform m_bumpMapTransform;
    RedColor m_colorRefraction;
    float m_distortionScale;
    RedMaterial m_material;

    // GLOBAL: SWBF2 0x009ca374
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca378
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca384
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca390
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca39c
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca3ac
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009ca3b0
    static pcRedShaderDef s_shaderDef;

    static pcRefractionShader s_Factory;

private:

    pcRefractionShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fSpecularPower);

    void _InitPipelineFuncArray();
};
 
 
 
 
 
 
 
 
