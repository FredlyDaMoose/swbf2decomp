#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"
#include "PebbleFL/Common/PblVector.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00784f2c
class pcWaterShader : public pcRedShader
{
public:
    pcWaterShader(unsigned int uiFlags,
                  const unsigned int* pParams,
                  unsigned int uiNumParams,
                  unsigned int uiMaterialFlags,
                  const RedColor& color0,
                  const RedColor& color1,
                  float fArg7);

    virtual ~pcWaterShader()
    {
    }

    // SYNTHETIC: SWBF2 0x007087b0
    // pcWaterShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

    void SetBumpMap(RedTexture* pTex1, RedTextureTransform* pTransform1,
                    RedTexture* pTex2, RedTextureTransform* pTransform2, float fBlend);
    void SetNormalMap(RedTexture* pTex1, RedTextureTransform* pTransform1,
                      RedTexture* pTex2, RedTextureTransform* pTransform2, float fBlend);
    void SetDiffuseMap(RedTexture* pTex1, RedTextureTransform* pTransform1,
                       RedTexture* pTex2, RedTextureTransform* pTransform2, float fBlend);
    void SetSpecularMask(RedTexture* pTex1, RedTextureTransform* pTransform1,
                         RedTexture* pTex2, RedTextureTransform* pTransform2, float fBlend);

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
    unsigned int GetState_Prog2(const RenderItem*) const;
    void Begin_Fixed(unsigned int uiState, unsigned int uiPass) const;
    void Begin_Prog1(unsigned int uiState, unsigned int uiPass) const;
    void Begin_Prog2(unsigned int uiState, unsigned int uiPass) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void End_Prog2(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem* pItem, unsigned int uiPass) const;
    void Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const;
    void Render_Prog2(const RenderItem* pItem, unsigned int uiPass) const;

    pcRedVertexFormat* m_pVertexFormat;
    RedTexture* m_reflectionMap;
    RedTexture* m_projectedNormalMap;
    RedTexture* m_normalMap1;
    RedTexture* m_normalMap2;
    RedTextureTransform m_normalMapTransform1;
    RedTextureTransform m_normalMapTransform2;
    float m_normalMapBlend;
    RedTexture* m_bumpMap1;
    RedTexture* m_bumpMap2;
    RedTextureTransform m_bumpMapTransform1;
    RedTextureTransform m_bumpMapTransform2;
    float m_bumpMapBlend;
    RedTexture* m_diffuseMap1;
    RedTexture* m_diffuseMap2;
    RedTextureTransform m_diffuseMapTransform1;
    RedTextureTransform m_diffuseMapTransform2;
    float m_diffuseMapBlend;
    RedTexture* m_specularMask1;
    RedTexture* m_specularMask2;
    RedTextureTransform m_specularMaskTransform1;
    RedTextureTransform m_specularMaskTransform2;
    float m_specularMaskBlend;

    // GLOBAL: SWBF2 0x009c9e28
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c9e2c
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9e38
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9e44
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9e50
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static pcRedShaderDef s_shaderDef;

    static pcWaterShader s_Factory;

    static unsigned int s_uiRenderTypeHash;

    static void SetFresnelMinMax(float fMin, float fMax);
    static void SetReflectionColor(RedColor color);
    static void SetRefractionColor(RedColor color);
    static void SetSpecularEnable(bool bEnable);

    static void SetSpecularDirection(const PblVector3& vDirection);
    void SetReflectionMap(RedTexture* pTexture);
    void SetProjectedNormalMap(RedTexture* pTexture);

    // GLOBAL: SWBF2 0x007e1ee0
    static bool s_specularEnable;

    // GLOBAL: SWBF2 0x007e1ef4
    static PblVector3 s_vSpecularDirection;

    // GLOBAL: SWBF2 0x007e1f40
    static RedColorValue s_refractionColor;

    // GLOBAL: SWBF2 0x007e1f50
    static RedColorValue s_reflectionColor;

    // GLOBAL: SWBF2 0x007e1f60
    static RedColorValue s_fresnelMinMax;

private:
    pcWaterShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);
    void _InitPipelineFuncArray();
};
 
 
 
 
 
 
 
 
 
 
 
 
