#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class RedTexture;
class pcRedVertexFormat;

// VTABLE: SWBF2 0x00785730
class pcHDRShader : public pcRedShader
{
public:

    enum HDR_MODE
    {
        HDR_MODE_CREATEGLOWMASK = 0,
        HDR_MODE_CREATELUMINANCE = 1,
        HDR_MODE_BLOOMFILTER = 2,
        HDR_MODE_SCALEANDBLOOM = 3
    };

    pcHDRShader(unsigned int uiFlags,
                const unsigned int* pParams,
                unsigned int uiNumParams,
                unsigned int uiMaterialFlags,
                const RedColor& color0,
                const RedColor& color1,
                float fArg7);

    virtual ~pcHDRShader()
    {
    }

    // SYNTHETIC: SWBF2 0x00713a00
    // pcHDRShader::`scalar deleting destructor'

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

    void SetTextures(RedTexture* pSource, RedTexture* pDest)
    {
        m_sourceTex = pSource;
        m_destTex = pDest;
    }
    void SetGlowThreshold(float fThreshold) { m_glowThreshold = fThreshold; }
    void SetGlowFactor(float fFactor) { m_glowFactor = fFactor; }
    void SetBloomParameters(float fOffset, float fWeight)
    {
        m_bloomOffset = fOffset;
        m_bloomWeight = fWeight;
    }
    void SetMode(HDR_MODE mode) { m_state = mode; }

    pcRedVertexFormat* m_pVertexFormat;
    RedTexture* m_sourceTex;
    RedTexture* m_destTex;
    float m_glowThreshold;
    float m_glowFactor;
    float m_bloomOffset;
    float m_bloomWeight;
    HDR_MODE m_state;

    // GLOBAL: SWBF2 0x009caa10
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009caa14
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009caa20
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009caa2c
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009caa38
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009caa7c
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009caa48
    static pcRedShaderDef s_shaderDef;

    static pcHDRShader s_Factory;

private:

    pcHDRShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
 
 
 
