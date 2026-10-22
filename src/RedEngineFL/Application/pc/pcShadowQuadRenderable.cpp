#include "pcShadowQuadRenderable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"

#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"

#include "RedEngineFL/Memory/engineAlloc.h"
#include "PebbleFL/Common/PblRef.h"

#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFilterCopyShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "PebbleFL/Common/PblHashTable.h"

#include <stddef.h>
#include <d3dx9math.h>

#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedLight.h"

 
 
 

// GLOBAL: SWBF2 0x009c893c
pcShadowQuadRenderable* pcShadowQuadRenderable::s_pInstance;

// GLOBAL: SWBF2 0x009c8944
// SYNTHETIC: SWBF2 0x004094a0
// `dynamic initializer for 'kuiClearShader''
static const unsigned int kuiClearShader = PblHash("Clear");

// GLOBAL: SWBF2 0x009c8948
// SYNTHETIC: SWBF2 0x004094c0
// `dynamic initializer for 'kuiFilterShader''
static const unsigned int kuiFilterShader = PblHash("FilterCopy");

// FUNCTION: SWBF2 0x006fbdf0
pcShadowQuadRenderable::pcShadowQuadRenderable()
    : Renderable(4)
{
    m_pPrimitive = 0;
    m_pShadowShader = 0;
    m_pFilterShader = 0;
    m_pScreenShader = 0;
    m_blurEnable = false;
}

// FUNCTION: SWBF2 0x006fbe30
pcShadowQuadRenderable::~pcShadowQuadRenderable()
{
    if (m_pPrimitive)
    {
        PblRef* p = (PblRef*)m_pPrimitive;
        p->Release();
        m_pPrimitive = 0;
    }
    if (m_pShadowShader)
    {
        PblRef* p = (PblRef*)m_pShadowShader;
        p->Release();
        m_pShadowShader = 0;
    }
    if (m_pFilterShader)
    {
        PblRef* p = (PblRef*)m_pFilterShader;
        p->Release();
        m_pFilterShader = 0;
    }
    if (m_pScreenShader)
    {
        PblRef* p = (PblRef*)m_pScreenShader;
        p->Release();
        m_pScreenShader = 0;
    }
}

// FUNCTION: SWBF2 0x006fbf90
void pcShadowQuadRenderable::Cleanup()
{
    Deactivate();
    if (m_pPrimitive)
    {
        PblRef* p = (PblRef*)m_pPrimitive;
        p->Release();
        m_pPrimitive = 0;
    }
    if (m_pShadowShader)
    {
        PblRef* p = (PblRef*)m_pShadowShader;
        p->Release();
        m_pShadowShader = 0;
    }
    if (m_pFilterShader)
    {
        PblRef* p = (PblRef*)m_pFilterShader;
        p->Release();
        m_pFilterShader = 0;
    }
    if (m_pScreenShader)
    {
        PblRef* p = (PblRef*)m_pScreenShader;
        p->Release();
        m_pScreenShader = 0;
    }
    if (m_pShadowTex)
    {
        delete m_pShadowTex;
        m_pShadowTex = 0;
    }
}

// FUNCTION: SWBF2 0x006fc0a0
void pcShadowQuadRenderable::Init()
{
    m_pShadowShader = pcRedShader::Create(0x9296fc1a, 2, 0, 0, 0,
                                          RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pFilterShader = (pcFilterCopyShader*)pcRedShader::Create(
        kuiFilterShader, 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pScreenShader = (pcClearShader*)pcRedShader::Create(
        kuiClearShader, 3, 0, 0, 0x80000, RedColor::WHITE, RedColor::BLACK, 32.0f);

    m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 2, 0xc, 4, 0, true);
    float* pVerts = (float*)m_pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    pVerts[0] = -1.0f;
    pVerts[1] = -1.0f;
    pVerts[2] = 1.0f;
    pVerts[3] = 1.0f;
    pVerts[4] = -1.0f;
    pVerts[5] = 1.0f;
    pVerts[6] = -1.0f;
    pVerts[7] = 1.0f;
    pVerts[8] = 1.0f;
    pVerts[9] = 1.0f;
    pVerts[10] = 1.0f;
    pVerts[11] = 1.0f;
    m_pPrimitive->EndVertexData();

    IDirect3DSurface9* pBackBuffer;
    IDirect3DSurface9* pBufferSurface;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,
                                              &pBackBuffer);
    ((IDirect3DTexture9*)(RedRenderer::pcGetBuffer(PC_BUFFER_SKY))->m_pTexture)
        ->GetSurfaceLevel(0, &pBufferSurface);

    D3DSURFACE_DESC backDesc;
    D3DSURFACE_DESC bufferDesc;
    pBackBuffer->GetDesc(&backDesc);
    pBufferSurface->GetDesc(&bufferDesc);
    pBackBuffer->Release();
    pBufferSurface->Release();

    if (backDesc.MultiSampleType == bufferDesc.MultiSampleType &&
        !RedVideoConfig::VidMode_IsDriverMultisampling() &&
        bufferDesc.Width == backDesc.Width &&
        bufferDesc.Height == backDesc.Height)
    {
        m_pShadowTex = 0;
    }
    else
    {
        m_pShadowTex = new RedTexture(bufferDesc.Width, bufferDesc.Height,
                                      bufferDesc.Format, 1, D3DUSAGE_RENDERTARGET,
                                      D3DPOOL_DEFAULT, 0);
    }

    Activate(-9997);
    m_blurEnable = false;
}

 
 
 
 
 
 
 
 

// FUNCTION: SWBF2 0x006fc2e0
void pcShadowQuadRenderable::Render(int value)
{
    if (!RedStencilShadow::GetShadowEnable())
    {
        RedDropShadowManager::Render(value);
        return;
    }

    RedColor colour;
    if (RedRenderer::pcGetRenderCaps().pipeline == 0)
    {
        colour.b = RedColor::BLACK.b;
    colour.g = RedColor::BLACK.g;
    colour.r = RedColor::BLACK.r;
        colour.a = 0x80;
        RedRenderer::pcRenderPrimitive(m_pPrimitive,
                                       m_pShadowShader,
                                       gMatrixIdentity,
                                       colour, 0x200,
                                       0, 0, 0);
        RedDropShadowManager::Render(value | 0x200);
        return;
    }

    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    RedTexture* pSavedTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pSavedDepth = RedRenderer::pcGetDepthTarget();

    RedTexture* target = (RedTexture*)m_pShadowTex;
    if (!target)
    {
        target = RedRenderer::pcGetBuffer(PC_BUFFER_SKY);
        RedRenderer::pcSetRenderTarget(target, pSavedDepth);
    }

    float left;
    float top;
    float right;
    float bottom;
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);

    PblMatrix scaleOffset(
        PblVector4{ viewport.fWidth * 0.5f, 0.0f, 0.0f, 0.0f },
        PblVector4{ 0.0f, viewport.fHeight * 0.5f, 0.0f, 0.0f },
        PblVector4{ 0.0f, 0.0f, 0.5f, 0.0f },
        PblVector4{ viewport.fWidth * 0.5f + viewport.fOffsetX +
                         0.5f / (float)target->m_uiWidth,
                     viewport.fHeight * 0.5f + viewport.fOffsetY +
                         0.5f / (float)target->m_uiHeight,
                     0.0f, 1.0f });

    PblMatrix projection;
    PblMatrix view;
    const RedCamera* camera = RedRenderer::GetCamera();
    D3DXMatrixMultiply((D3DXMATRIX*)&view, (const D3DXMATRIX*)&camera->_MatrixInverse,
                       (const D3DXMATRIX*)&RedRenderer::GetProjectionMatrix());
    D3DXMatrixMultiply((D3DXMATRIX*)&projection, (const D3DXMATRIX*)&view,
                       (const D3DXMATRIX*)&scaleOffset);
    RedShadowMap::SetScreenSpaceShadowMap((RedTexture*)target,
                                         *(const PblMatrix*)&projection);

    RedShadowMap::_uiShadowMapWrapMode = 3;

    pcRedPrimitive* mesh =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 3, 0x10, 4, 0, false);
    PblVector4* vertex = (PblVector4*)mesh->BeginVertexData();
    if (!vertex)
    {
        return;
    }

    vertex[0].Set(left, top, 0.0f, 1.0f);
    vertex[1].Set(left, bottom, 0.0f, 1.0f);
    vertex[2].Set(right, top, 0.0f, 1.0f);
    vertex[3].Set(right, bottom, 0.0f, 1.0f);
    mesh->EndVertexData();

    RedRenderer::pcFlush(0);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);

    colour = RedColor(0, 0, 0, 0xff);
    RedRenderer::pcRenderPrimitive(mesh, m_pScreenShader,
                                   gMatrixIdentity,
                                   colour, 0x200, 0,
                                   0, 0);

    float alpha = RedRenderer::GetShadowIntensity() * 255.0f;
    colour.b = RedColor::BLACK.b;
    colour.g = RedColor::BLACK.g;
    colour.r = RedColor::BLACK.r;
    colour.a = (unsigned char)(int)alpha;
    RedRenderer::pcRenderPrimitive(m_pPrimitive,
                                   m_pShadowShader,
                                   gMatrixIdentity,
                                   colour, 0x200, 0,
                                   0, 0);
    RedDropShadowManager::Render(value | 0x200);

    if (m_pShadowTex)
    {
        RECT source = { (int)left, (int)top, (int)right, (int)bottom };
        RECT dest = { 0, 0, (int)target->m_uiWidth, (int)target->m_uiHeight };

        IDirect3DSurface9* backBuffer;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
        IDirect3DSurface9* surface;
        ((IDirect3DTexture9*)target->m_pTexture)->GetSurfaceLevel(0, &surface);
        RedRenderer::pcGetDevice()->StretchRect(backBuffer, &source, surface, &dest, D3DTEXF_NONE);
        backBuffer->Release();
        surface->Release();

        colour = RedColor(0, 0, 0, 0);
        RedRenderer::pcRenderPrimitive(mesh,
                                       m_pScreenShader,
                                       gMatrixIdentity,
                                       colour, 0x200,
                                       0, 0, 0);
    }

    mesh->Release();
    RedRenderer::pcSetRenderTarget(pSavedTarget, pSavedDepth);
    RedRenderer::pcSetViewport(viewport);
    RedRenderer::pcFlush(0);
    RedRenderer::pcSetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
}
