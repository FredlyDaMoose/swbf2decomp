#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "PebbleFL/Common/PblVector.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00772e7c
class pcShieldShader : public pcRedShader
{
public:

    pcShieldShader(unsigned int uiFlags,
                   const unsigned int* pParams,
                   unsigned int uiNumParams,
                   unsigned int uiMaterialFlags,
                   const RedColor& color0,
                   const RedColor& color1,
                   float fArg7);

    virtual ~pcShieldShader()
    {
    }

    // SYNTHETIC: SWBF2 0x006cce30
    // pcShieldShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual RedShadingState* CreateShadingState() const;
    virtual void Bind();

    void SetSpecularPower(float fPower);

private:

    static PblMatrix _GetNearSceneFadeTextureTransform();
    virtual void _Read(PblFileChunk& chunk) const;
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
    unsigned int GetState_Fixed(const RenderItem* pItem) const;
    unsigned int GetState_Prog1(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem* pItem, unsigned int) const;
    void Render_Prog1(const RenderItem* pItem, unsigned int) const;

    pcRedVertexFormat* m_pVertexInputFormat;
    unsigned int m_uiTextureHash;
    unsigned int m_uiMaterialFlags;
    RedColor m_colorDiffuse;
    RedColor m_colorSpecular;
    float m_specPower;
    float m_specSizeInv;
    float m_fOffsetU;
    float m_fOffsetV;
    float m_fViewAngleFactor;
    float m_fVertexMinDistSqr;

    // GLOBAL: SWBF2 0x00964f80
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x00964f84
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x00964f90
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x00964f9c
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x00964fa8
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x00964fb4
    static float s_fFadeStart;

    // GLOBAL: SWBF2 0x00964fb8
    static float s_fFadeEnd;

    // GLOBAL: SWBF2 0x00965010
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x0096501c
    static pcRedShaderDef s_shaderDef;

    // GLOBAL: SWBF2 0x0096504c
    static PblVector3 s_vLightDir;

    // GLOBAL: SWBF2 0x00965058
    static PblVector3 s_vLightX;

    // GLOBAL: SWBF2 0x00965064
    static PblVector3 s_vLightY;

    static pcShieldShader s_Factory;

private:

    pcShieldShader();

    void _InitPipelineFuncArray();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);
};
 
 
 
 
 
