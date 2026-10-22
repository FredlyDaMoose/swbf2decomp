#pragma once

#include <stddef.h>
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

class PblVector4;
class RedTexture;

// VTABLE: SWBF2 0x0078447c
class pcTerrainShader2 : public pcRedShader
{
public:

    pcTerrainShader2(unsigned int uiFlags,
                     const unsigned int* pParams,
                     unsigned int uiNumParams,
                     unsigned int uiMaterialFlags,
                     const RedColor& color0,
                     const RedColor& color1,
                     float fSpecularPower);

    virtual ~pcTerrainShader2()
    {
    }

    // SYNTHETIC: SWBF2 0x006fccb0
    // pcTerrainShader2::`scalar deleting destructor'

    virtual RenderPhaseT GetRenderPhase(const RenderItem*, unsigned int) const;
    virtual unsigned int GetID(unsigned int uiPass) const;
    virtual unsigned int GetRenderTypeHash() const;
    virtual RedShadingState* CreateShadingState() const;
    virtual void Bind();
    virtual unsigned int GetZPrepassState() const;
    virtual void RenderZPrepass(const RenderItem* pItem) const;
    virtual unsigned short GetShaderChain(pcRedShader** const ppOut,
                                          unsigned short uiMax,
                                          const RenderItem* pItem);

    unsigned int GetState_Fixed(const RenderItem*) const;
    unsigned int GetState_Prog1(const RenderItem* pItem) const;
    void Begin_Fixed(unsigned int, unsigned int) const;
    void Begin_Prog1(unsigned int uiState, unsigned int uiPass) const;
    void End_Fixed(unsigned int, unsigned int) const;
    void End_Prog1(unsigned int, unsigned int) const;
    void Render_Fixed(const RenderItem*, unsigned int) const;
    void Render_Prog1(const RenderItem* pItem, unsigned int uiPass) const;

    static bool __cdecl IsEnabled();
    static void __cdecl SetEnabled(bool bEnabled);

    static void __cdecl _InitMaps(float fScale);
    static void __cdecl _InitNoiseMap(int iIndex, unsigned int uiHash,
                                      const PblVector4& vScale, const PblVector4& vOffset);
    static void __cdecl _InitDiffuseMap(int iIndex, unsigned int uiHash,
                                        const PblVector4& vScale);
    static void __cdecl _InitLayer(int iIndex, unsigned int uiHash,
                                   const PblVector4& vScale0, const PblVector4& vScale1);
    static void __cdecl _InitDetail(unsigned int uiHash, const PblVector4& vScale0,
                                    const PblVector4& vScale1, float fFadeDistance);
    static void __cdecl _InitBumpLuminosityPixelLighting(unsigned int uiHash,
                                                         unsigned int uiBumpLuminosityHash,
                                                         const PblVector4& vScale0,
                                                         const PblVector4& vScale1,
                                                         float fStaticBumpFactor);
    static void __cdecl _InitPixelLighting(unsigned int uiHashA, unsigned int uiHashB);
    static void __cdecl _InitShadowMap(RedTexture* pTexture, unsigned int uiWrapMode,
                                       const PblVector4& vU, const PblVector4& vV,
                                       const PblVector4& vW);

    unsigned char m_uiLayerIndex[4];
    unsigned char m_uiOptions;
    bool m_lightingInSeparatePass;

    // GLOBAL: SWBF2 0x009c8974
    static unsigned int s_uiID;
    // GLOBAL: SWBF2 0x009c8978
    static GetStateFuncT GetStateFuncArray[MAX_PIPELINES];
    // GLOBAL: SWBF2 0x009c8984
    static BeginFuncT BeginFuncArray[MAX_PIPELINES];
    // GLOBAL: SWBF2 0x009c8990
    static EndFuncT EndFuncArray[MAX_PIPELINES];
    // GLOBAL: SWBF2 0x009c899c
    static RenderFuncT RenderFuncArray[MAX_PIPELINES];

    static unsigned int s_uiRenderTypeHash;

    static pcRedShaderDef s_shaderDef;

    static pcTerrainShader2 s_Factory;

private:

    pcTerrainShader2();

    virtual pcRedShader* _Create(unsigned int uiFlags,
                                 const unsigned int* pParams,
                                 unsigned int uiNumParams,
                                 unsigned int uiMaterialFlags,
                                 const RedColor& color0,
                                 const RedColor& color1,
                                 float fSpecularPower) const;
    virtual void _Read(PblFileChunk& chunk) const;
    virtual void _StaticInit() const;
    virtual pcRedShaderDef* GetShaderDef() const;
    virtual void InitFixedFunctionShaderDef() const;

    void _Init(unsigned int uiFlags,
               const unsigned int* pParams,
               unsigned int uiNumParams,
               unsigned int uiMaterialFlags,
               const RedColor& color0,
               const RedColor& color1,
               float fSpecularPower);
    void _InitPipelineFuncArray();
};
 
 
 
 
