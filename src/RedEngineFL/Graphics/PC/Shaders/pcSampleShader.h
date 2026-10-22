#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/RedMaterial.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00784b24
class pcSampleShader : public pcRedShader
{
public:

    pcSampleShader(unsigned int uiFlags,
                   const unsigned int* pParams,
                   unsigned int uiNumParams,
                   unsigned int uiMaterialFlags,
                   const RedColor& color0,
                   const RedColor& color1,
                   float fArg7);

    virtual ~pcSampleShader()
    {
    }

    // SYNTHETIC: SWBF2 0x00705660
    // pcSampleShader::`scalar deleting destructor'

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

    void SetInputTexture(RedTexture* pTexture) { m_inputTexture = pTexture; }
    void SetOutputPosition(float fX, float fY)
    {
        m_outputX = fX;
        m_outputY = fY;
    }

    void SetInputOffset(float fX, float fY)
    {
        m_inputOffsetX = fX;
        m_inputOffsetY = fY;
    }
    void SetScale(float fScale)
    {
        m_scale.r = fScale;
        m_scale.g = fScale;
        m_scale.b = fScale;
        m_scale.a = fScale;
    }

    RedTexture* m_inputTexture;
    float m_outputX;
    float m_outputY;
    float m_inputOffsetX;
    float m_inputOffsetY;
    RedColorValue m_scale;
    unsigned int m_uiMaterialFlags;
    pcRedVertexFormat* m_pVertexFormat;

    // GLOBAL: SWBF2 0x009c9a98
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c9a9c
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9aa8
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9ab4
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9ac0
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

    static pcSampleShader s_Factory;

private:

    pcSampleShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
 
