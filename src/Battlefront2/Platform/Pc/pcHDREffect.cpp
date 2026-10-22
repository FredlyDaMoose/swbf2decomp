#include "Battlefront2/Source/HDREffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcHDRShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// GLOBAL: SWBF2 0x01f9ae34
RedTexture* s_glowTextures[2];

// FUNCTION: SWBF2 0x00419110
void HDREffect::PlatformInit()
{
    m_pHDRShader = (pcHDRShader*)pcRedShader::Create(
        PblHash("hdr"), 0x201, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_downSizeFactor = 0.25f;
    m_numBloomPasses = 5;
    m_maxTotalWeight = 1.2f;
    m_glowThreshold = 0.5f;
    m_glowFactor = 1.0f;

    IDirect3DSurface9* pBackBuffer;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    IDirect3DSurface9* pBufferSurface;
    ((IDirect3DTexture9*)(RedRenderer::pcGetBuffer(PC_BUFFER_SKY))->m_pTexture)
        ->GetSurfaceLevel(0, &pBufferSurface);
    D3DSURFACE_DESC backDesc;
    pBackBuffer->GetDesc(&backDesc);
    D3DSURFACE_DESC bufferDesc;
    pBufferSurface->GetDesc(&bufferDesc);
    pBackBuffer->Release();
    pBufferSurface->Release();

    s_glowTextures[0] = new RedTexture(bufferDesc.Width, bufferDesc.Height, bufferDesc.Format,
                                       1, D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, 0);
    s_glowTextures[1] = RedRenderer::pcGetBuffer(PC_BUFFER_SKY);
}

// FUNCTION: SWBF2 0x00419260
void HDREffect::PlatformCleanup()
{
    if (m_pHDRShader)
    {
        m_pHDRShader->Release();
        m_pHDRShader = 0;
    }
    delete s_glowTextures[0];
}

// FUNCTION: SWBF2 0x004192c0
void HDREffect::Render(int value)
{
    if ((unsigned int)value & 0x200000)
    {
        return;
    }
    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
    {
        return;
    }

    float fOffset = (1.0f - m_downSizeFactor) * 0.5f;
    float fSpread = (float)m_numBloomPasses * 0.015f * (1.0f - m_downSizeFactor);

    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);

    RedColor black;
    black.b = 0;
    black.g = 0;
    black.r = 0;
    black.a = 0xff;

    RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();
    RedViewport oldViewport;
    RedRenderer::pcGetViewport(oldViewport);
    RedRenderer::pcSetRenderTarget(s_glowTextures[0], 0);
    RedRenderer::pcClear(D3DCLEAR_TARGET, black, 1.0f, 0);
    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::pcSetViewport(oldViewport);

    pcCopyBackBuffer(s_glowTextures[1]);

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x201, 16, 4, 0, false);
    float* pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }
    pVerts[0] = fOffset;
    pVerts[1] = fOffset;
    pVerts[2] = viewport.fOffsetX;
    pVerts[3] = viewport.fOffsetY;
    pVerts[4] = m_downSizeFactor * viewport.fWidth + fOffset;
    pVerts[5] = fOffset;
    pVerts[6] = viewport.fWidth + viewport.fOffsetX;
    pVerts[7] = viewport.fOffsetY;
    pVerts[8] = fOffset;
    pVerts[9] = m_downSizeFactor * viewport.fHeight + fOffset;
    pVerts[10] = viewport.fOffsetX;
    pVerts[11] = viewport.fHeight + viewport.fOffsetY;
    pVerts[12] = m_downSizeFactor * viewport.fWidth + fOffset;
    pVerts[13] = m_downSizeFactor * viewport.fHeight + fOffset;
    pVerts[14] = viewport.fWidth + viewport.fOffsetX;
    pVerts[15] = viewport.fHeight + viewport.fOffsetY;
    pPrimitive->EndVertexData();

    m_pHDRShader->SetTextures(s_glowTextures[1], s_glowTextures[0]);
    m_pHDRShader->SetGlowThreshold(m_glowThreshold);
    m_pHDRShader->SetMode(pcHDRShader::HDR_MODE_CREATEGLOWMASK);
    RedRenderer::pcRenderPrimitive(pPrimitive, m_pHDRShader, gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
    pPrimitive->Release();

    pOldTarget = RedRenderer::pcGetRenderTarget();
    pOldDepth = RedRenderer::pcGetDepthTarget();
    RedRenderer::pcGetViewport(oldViewport);
    RedRenderer::pcSetRenderTarget(s_glowTextures[1], 0);
    RedRenderer::pcClear(D3DCLEAR_TARGET, black, 1.0f, 0);
    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::pcSetViewport(oldViewport);

    pcRedPrimitive* pBloom =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x201, 16, 4, 0, false);
    pVerts = (float*)pBloom->BeginVertexData();
    if (!pVerts)
    {
        return;
    }
    float fLo = fOffset - fSpread;
    float fHi = fSpread + fOffset;
    pVerts[0] = fLo;
    pVerts[1] = fLo;
    pVerts[2] = fLo;
    pVerts[3] = fLo;
    pVerts[4] = m_downSizeFactor * viewport.fWidth + fHi;
    pVerts[5] = fLo;
    pVerts[6] = m_downSizeFactor * viewport.fWidth + fHi;
    pVerts[7] = fLo;
    pVerts[8] = fLo;
    pVerts[9] = m_downSizeFactor * viewport.fHeight + fHi;
    pVerts[10] = fLo;
    pVerts[11] = m_downSizeFactor * viewport.fHeight + fHi;
    pVerts[12] = m_downSizeFactor * viewport.fWidth + fHi;
    pVerts[13] = m_downSizeFactor * viewport.fHeight + fHi;
    pVerts[14] = m_downSizeFactor * viewport.fWidth + fHi;
    pVerts[15] = m_downSizeFactor * viewport.fHeight + fHi;
    pBloom->EndVertexData();

    RedTexture* pSource = s_glowTextures[0];
    RedTexture* pDest = s_glowTextures[1];
    m_pHDRShader->SetMode(pcHDRShader::HDR_MODE_BLOOMFILTER);
    for (unsigned int i = 0; i < m_numBloomPasses; ++i)
    {
        m_pHDRShader->SetTextures(pSource, pDest);
        m_pHDRShader->SetBloomParameters(
            (float)i + 0.5f,
            (m_maxTotalWeight - 1.0f) * (float)i / (float)m_numBloomPasses + 0.25f);
        RedRenderer::pcRenderPrimitive(pBloom, m_pHDRShader, gMatrixIdentity,
                                       RedColor::WHITE, 0x200, 0, 0, 0);
        if (i < m_numBloomPasses - 1)
        {
            RedTexture* pTemp = pSource;
            pSource = pDest;
            pDest = pTemp;
        }
    }
    pBloom->Release();

    pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x201, 16, 4, 0, false);
    pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }
    pVerts[0] = viewport.fOffsetX;
    pVerts[1] = viewport.fOffsetY;
    pVerts[2] = fOffset;
    pVerts[3] = fOffset;
    pVerts[4] = viewport.fWidth + viewport.fOffsetX;
    pVerts[5] = viewport.fOffsetY;
    pVerts[6] = m_downSizeFactor * viewport.fWidth + fOffset;
    pVerts[7] = fOffset;
    pVerts[8] = viewport.fOffsetX;
    pVerts[9] = viewport.fHeight + viewport.fOffsetY;
    pVerts[10] = fOffset;
    pVerts[11] = m_downSizeFactor * viewport.fHeight + fOffset;
    pVerts[12] = viewport.fWidth + viewport.fOffsetX;
    pVerts[13] = viewport.fHeight + viewport.fOffsetY;
    pVerts[14] = m_downSizeFactor * viewport.fWidth + fOffset;
    pVerts[15] = m_downSizeFactor * viewport.fHeight + fOffset;
    pPrimitive->EndVertexData();

    m_pHDRShader->SetTextures(pDest, 0);
    m_pHDRShader->SetMode(pcHDRShader::HDR_MODE_SCALEANDBLOOM);
    m_pHDRShader->SetGlowFactor(m_glowFactor);
    RedRenderer::pcRenderPrimitive(pPrimitive, m_pHDRShader, gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00419950
void HDREffect::pcCopyBackBuffer(const RedTexture* pTexture) const
{
    IDirect3DSurface9* pBackBuffer;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    IDirect3DSurface9* pSurface;
    ((IDirect3DTexture9*)pTexture->m_pTexture)->GetSurfaceLevel(0, &pSurface);
    D3DSURFACE_DESC desc;
    pSurface->GetDesc(&desc);
    float left, top, right, bottom;
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);
    RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, 0, pSurface, 0, D3DTEXF_NONE);
    pBackBuffer->Release();
    pSurface->Release();
}
