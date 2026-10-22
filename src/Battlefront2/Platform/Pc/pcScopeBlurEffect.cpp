#include "Battlefront2/Source/ScopeBlurEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFilterCopyShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

// GLOBAL: SWBF2 0x007e5e78
static FilterCopyShaderSample BoxFilter[9] = {
    { 0.125f, -3.0f, 3.0f },  { 0.125f, -2.0f, 2.0f },  { 0.125f, -1.0f, 1.0f },
    { 0.125f, 3.0f, -3.0f },  { 0.0f, 0.0f, 0.0f },     { 0.125f, -4.0f, 4.0f },
    { 0.125f, 1.0f, -1.0f },  { 0.125f, 2.0f, -2.0f },  { 0.125f, 4.0f, -4.0f },
};

void ScopeBlurEffect::PlatformReadConfigData(PblConfig::Data& data)
{
    (void)data;
}

// FUNCTION: SWBF2 0x004267b0
void ScopeBlurEffect::PlatformInit()
{
    m_pClearShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x403, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pFilterShader = (pcFilterCopyShader*)pcRedShader::Create(
        PblHash("FilterCopy"), 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pBlurTexture = new RedTexture(256, 256, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                    D3DPOOL_DEFAULT, 0);
}

// FUNCTION: SWBF2 0x00426880
void ScopeBlurEffect::PlatformCleanup()
{
    if (m_pClearShader)
    {
        m_pClearShader->Release();
        m_pClearShader = 0;
    }
    if (m_pFilterShader)
    {
        m_pFilterShader->Release();
        m_pFilterShader = 0;
    }
    delete m_pBlurTexture;
}

// FUNCTION: SWBF2 0x00426910
void ScopeBlurEffect::Render(int value)
{
    unsigned int uiViewport = FLRenderer::GetCurrentViewport();
    if ((unsigned int)value & 0x200000)
    {
        return;
    }
    if (!m_alpha[uiViewport])
    {
        return;
    }

    RedColor color = RedColor::WHITE;
    float fU, fV;
    RedViewport viewport;
    float left, top, right, bottom;
    pcRedPrimitive* pPrimitive;
    float* pVerts;

    if ((RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) &&
        RedRenderer::pcGetRenderOptions().distortion)
    {
        pcCopyBackBuffer();

        pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x202, 20, 4, 0, false);
        pVerts = (float*)pPrimitive->BeginVertexData();
        if (!pVerts)
        {
            return;
        }

        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcGetViewportExtents(left, top, right, bottom);

        fU = (float)m_pBlurTexture->m_uiWidth /
             (float)(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP))->m_uiWidth;
        fV = (float)m_pBlurTexture->m_uiHeight /
             (float)(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP))->m_uiHeight;

        float fX = fU * 2.0f - 1.0f;
        float fY = 1.0f - fV * 2.0f;
        pVerts[0] = -1.0f;  pVerts[1] = 1.0f;   pVerts[2] = 0.0f;
        pVerts[3] = 0.0f;   pVerts[4] = 0.0f;
        pVerts[5] = -1.0f;  pVerts[6] = fY;     pVerts[7] = 0.0f;
        pVerts[8] = 0.0f;   pVerts[9] = 1.0f;
        pVerts[10] = fX;    pVerts[11] = 1.0f;  pVerts[12] = 0.0f;
        pVerts[13] = 1.0f;  pVerts[14] = 0.0f;
        pVerts[15] = fX;    pVerts[16] = fY;    pVerts[17] = 0.0f;
        pVerts[18] = 1.0f;  pVerts[19] = 1.0f;
        pPrimitive->EndVertexData();

        m_pFilterShader->SetTextures(m_pBlurTexture, RedRenderer::pcGetBuffer(PC_BUFFER_TEMP));
        m_pFilterShader->SetSamples(9, BoxFilter);
        RedRenderer::pcRenderPrimitive(pPrimitive, m_pFilterShader, gMatrixIdentity,
                                       RedColor::WHITE, 0x200, 0, 0, 0);
        pPrimitive->Release();

        m_pClearShader->m_pTex = RedRenderer::pcGetBuffer(PC_BUFFER_TEMP);
        color.a = (unsigned char)m_alpha[uiViewport];
    }
    else
    {
        m_pClearShader->m_pTex = RedTexture::Find(0xDE020766);
        fU = 1.0f;
        color = RedColor::BLACK;
        fV = 1.0f;
        color.a = (unsigned char)(int)((float)m_alpha[uiViewport] * 0.6f);
    }

    m_pClearShader->m_pAlphaMaskTex = RedTexture::Find(0xF745F270);

    pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x403, 32, 4, 0, false);
    pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    RedRenderer::pcGetViewport(viewport);
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);

    float fU0 = viewport.fOffsetX * fU;
    float fV0 = viewport.fOffsetY * fV;
    float fU1 = (viewport.fWidth + viewport.fOffsetX) * fU;
    float fV1 = (viewport.fHeight + viewport.fOffsetY) * fV;
    pVerts[0] = left;   pVerts[1] = top;     pVerts[2] = 0.0f;  pVerts[3] = 1.0f;
    pVerts[4] = fU0;    pVerts[5] = fV0;     pVerts[6] = 0.0f;  pVerts[7] = 0.0f;
    pVerts[8] = left;   pVerts[9] = bottom;  pVerts[10] = 0.0f; pVerts[11] = 1.0f;
    pVerts[12] = fU0;   pVerts[13] = fV1;    pVerts[14] = 0.0f; pVerts[15] = 1.0f;
    pVerts[16] = right; pVerts[17] = top;    pVerts[18] = 0.0f; pVerts[19] = 1.0f;
    pVerts[20] = fU1;   pVerts[21] = fV0;    pVerts[22] = 1.0f; pVerts[23] = 0.0f;
    pVerts[24] = right; pVerts[25] = bottom; pVerts[26] = 0.0f; pVerts[27] = 1.0f;
    pVerts[28] = fU1;   pVerts[29] = fV1;    pVerts[30] = 1.0f; pVerts[31] = 1.0f;
    pPrimitive->EndVertexData();

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pClearShader, gMatrixIdentity,
                                   color, 0x200, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00426e10
void ScopeBlurEffect::pcCopyBackBuffer() const
{
    float left, top, right, bottom;
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);

    RECT srcRect;
    srcRect.left = (LONG)left;
    srcRect.top = (LONG)top;
    srcRect.right = (LONG)right;
    srcRect.bottom = (LONG)bottom;
    RECT destRect;
    destRect.left = 0;
    destRect.top = 0;
    destRect.right = m_pBlurTexture->m_uiWidth;
    destRect.bottom = m_pBlurTexture->m_uiHeight;

    IDirect3DSurface9* pBackBuffer;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    IDirect3DSurface9* pSurface;
    ((IDirect3DTexture9*)m_pBlurTexture->m_pTexture)->GetSurfaceLevel(0, &pSurface);
    RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &srcRect, pSurface, &destRect,
                                            D3DTEXF_NONE);
    pBackBuffer->Release();
    pSurface->Release();
}
