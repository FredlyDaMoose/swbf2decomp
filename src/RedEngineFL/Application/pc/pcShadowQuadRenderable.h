#ifndef SWBF2_GRAPHICS_PC_PCSHADOWQUADRENDERABLE_H
#define SWBF2_GRAPHICS_PC_PCSHADOWQUADRENDERABLE_H

#include "RedEngineFL/Application/Renderable.h"

#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

class pcRedPrimitive;
class pcRedShader;
class pcFilterCopyShader;
class pcClearShader;
class RedTexture;

// VTABLE: SWBF2 0x00784404
class pcShadowQuadRenderable : public Renderable
{
public:

    virtual ~pcShadowQuadRenderable();

    // SYNTHETIC: SWBF2 0x006fbf60
    // pcShadowQuadRenderable::`scalar deleting destructor'

    virtual void Render(int value);

    pcShadowQuadRenderable();

    void Init();

    void Cleanup();

    static void Create();

    static void Destroy();

    static pcShadowQuadRenderable* s_pInstance;

    static pcShadowQuadRenderable* Instance() { return s_pInstance; }
    void EnableBlur(bool bEnable) { m_blurEnable = bEnable; }

    pcRedPrimitive* m_pPrimitive;
    pcRedShader* m_pShadowShader;
    pcFilterCopyShader* m_pFilterShader;
    pcClearShader* m_pScreenShader;
    RedTexture* m_pShadowTex;
    bool m_blurEnable;
};

 #endif
