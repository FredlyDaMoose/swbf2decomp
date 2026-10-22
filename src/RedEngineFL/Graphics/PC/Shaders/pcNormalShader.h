#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

#include "RedEngineFL/Graphics/RedMaterial.h"

class pcRedVertexFormat;

// VTABLE: SWBF2 0x007845f8
class pcNormalShader : public pcRedShader
{
public:

    virtual ~pcNormalShader()
    {
    }

    // SYNTHETIC: SWBF2 0x006fe9e0
    // pcNormalShader::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual RedShadingState* CreateShadingState() const;
    virtual void SetData(const RedShaderData&);
    virtual void Bind();
    virtual unsigned int GetZPrepassState() const;
    virtual void RenderZPrepass(const RenderItem*) const;
    virtual bool SortAsZWritingTransparent() const;
    virtual bool IsTransparent() const;
    virtual unsigned short GetShaderChain(pcRedShader** const, unsigned short,
                                          const RenderItem*);
    virtual int GetDiffuseMapHash(int, int);

private:

    // FUNCTION: SWBF2 0x007011d0
    virtual pcRedShader* _Create(unsigned int uiFlags,
                               const unsigned int* pParams,
                               unsigned int uiNumParams,
                               unsigned int uiMaterialFlags,
                               const RedColor& color0,
                               const RedColor& color1,
                                 float fSpecularPower) const;
    virtual void _Read(PblFileChunk&) const;

    // FUNCTION: SWBF2 0x007015a0
    virtual void _StaticInit() const;

    // FUNCTION: SWBF2 0x006ff7c0
    virtual pcRedShaderDef* GetShaderDef() const;

    // FUNCTION: SWBF2 0x006ff990
    virtual void InitFixedFunctionShaderDef() const;

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

    RedMaterial m_material;
    bool m_lightingInSeparatePass;

    // GLOBAL: SWBF2 0x009c926c
    static unsigned int s_uiID;

    // GLOBAL: SWBF2 0x009c9270
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c927c
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9288
    static EndFuncT EndFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c9294
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    // GLOBAL: SWBF2 0x009c938c
    static unsigned int s_uiRenderTypeHash;

    // GLOBAL: SWBF2 0x009c934c
    static pcRedShaderDef s_shaderDef;

    // GLOBAL: SWBF2 0x009c92a8
    static pcNormalShader s_Factory;

private:

public:

    // FUNCTION: SWBF2 0x006fe8b0
    pcNormalShader(unsigned int uiFlags,
                   const unsigned int* pParams,
                   unsigned int uiNumParams,
                   unsigned int uiMaterialFlags,
                   const RedColor& color0,
                   const RedColor& color1,
                   float fSpecularPower);

private:
    // FUNCTION: SWBF2 0x006fe7e0
    pcNormalShader();

    // FUNCTION: SWBF2 0x00701260
    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fSpecularPower);

    // FUNCTION: SWBF2 0x00701460
    void _InitPipelineFuncArray();

    void UpdateAnimation();
};
 
 
