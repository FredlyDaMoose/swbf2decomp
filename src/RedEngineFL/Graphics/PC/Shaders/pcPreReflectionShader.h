#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x00785018
class pcPreReflectionShader : public pcRedShader
{
public:

    pcPreReflectionShader(unsigned int uiFlags,
                          const unsigned int* pParams,
                          unsigned int uiNumParams,
                          unsigned int uiMaterialFlags,
                          const RedColor& color0,
                          const RedColor& color1,
                          float fArg7);

    virtual ~pcPreReflectionShader()
    {
    }

    // SYNTHETIC: SWBF2 0x0070b110
    // pcPreReflectionShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual void Bind();

    void SetReflectionID(char id) { m_reflectionID = id; }

private:
    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiNumParams,
                                 unsigned int uiMaterialFlags,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fArg7) const;
    virtual void _Read(PblFileChunk& chunk) const;
    virtual void _StaticInit() const;
    virtual pcRedShaderDef* GetShaderDef() const;
    virtual void InitFixedFunctionShaderDef() const;

public:
    unsigned int GetState_Fixed(const RenderItem*) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem* pItem, unsigned int) const;

    unsigned int GetState_Prog1(const RenderItem*) const;
    void Begin_Prog1(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Prog1(const RenderItem* pItem, unsigned int) const;

    unsigned int m_uiMaterialFlags;
    char m_reflectionID;
    pcRedVertexFormat* m_pVertexFormat;

    // GLOBAL: SWBF2 0x009ca0dc
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009ca0e0
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca0ec
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca0f8
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009ca104
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static pcRedShaderDef s_shaderDef;

    static pcPreReflectionShader s_Factory;

    static unsigned int s_uiRenderTypeHash;

private:

    pcPreReflectionShader();

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fArg7);

    void _InitPipelineFuncArray();
};
 
 
 
 
