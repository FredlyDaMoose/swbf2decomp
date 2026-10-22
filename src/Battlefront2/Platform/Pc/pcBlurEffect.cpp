#include "Battlefront2/Source/BlurEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFilterCopyShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// GLOBAL: SWBF2 0x007e5820
FilterCopyShaderSample BoxFilter[8] = {
    { 0.125f, -0.5f, -0.5f }, { 0.125f, -0.5f, 0.5f },
    { 0.125f, 0.5f, 0.5f },   { 0.125f, 0.5f, -0.5f },
    { 0.125f, -1.5f, 0.0f },  { 0.125f, 0.0f, -1.5f },
    { 0.125f, 1.5f, 0.0f },   { 0.125f, 0.0f, 1.5f },
};

// FUNCTION: SWBF2 0x0040f7a0
void BlurEffect::PlatformInit()
{
    m_constantBlend = 0.25f;
    m_downSizeFactor = 0.25f;
    m_pFilterShader = (pcFilterCopyShader*)pcRedShader::Create(
        PblHash("FilterCopy"), 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pClearShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x203, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x0040f810
void BlurEffect::PlatformCleanup()
{
    if (m_pFilterShader)
    {
        m_pFilterShader->Release();
        m_pFilterShader = 0;
    }
    if (m_pClearShader)
    {
        m_pClearShader->Release();
        m_pClearShader = 0;
    }
}

// FUNCTION: SWBF2 0x0040f890
void BlurEffect::PlatformReadConfigData(PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case 0x7E1194B8u:
        m_constantBlend = data.GetFloatArg(0);
        break;
    case 0x7E6C7187u:
        m_downSizeFactor = data.GetFloatArg(0);
        break;
    case 0x055CEA38u:
        break;
    }
}

// FUNCTION: SWBF2 0x0040f8d0
void BlurEffect::Render(int value)
{
    if ((unsigned int)value & 0x200000)
    {
        return;
    }
    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
    {
        return;
    }

    RedColor color = RedColor::WHITE;
    pcCopyBackBuffer();

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x202, 20, 4, 0, false);
    float* pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    float left, top, right, bottom;
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);

    float fU1 = (right - left) * m_downSizeFactor /
                (float)(RedRenderer::pcGetBuffer(PC_BUFFER_SKY))->m_uiWidth;
    float fV1 = (bottom - top) * m_downSizeFactor /
                (float)(RedRenderer::pcGetBuffer(PC_BUFFER_SKY))->m_uiHeight;
    float fU0 = (right - left) * m_downSizeFactor /
                (float)(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP))->m_uiWidth;
    float fV0 = (bottom - top) * m_downSizeFactor /
                (float)(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP))->m_uiHeight;

    float fX = fU1 * 2.0f - 1.0f;
    float fY = 1.0f - fV1 * 2.0f;
    pVerts[0] = -1.0f;  pVerts[1] = 1.0f;   pVerts[2] = 0.0f;
    pVerts[3] = 0.0f;   pVerts[4] = 0.0f;
    pVerts[5] = -1.0f;  pVerts[6] = fY;     pVerts[7] = 0.0f;
    pVerts[8] = 0.0f;   pVerts[9] = fV0;
    pVerts[10] = fX;    pVerts[11] = 1.0f;  pVerts[12] = 0.0f;
    pVerts[13] = fU0;   pVerts[14] = 0.0f;
    pVerts[15] = fX;    pVerts[16] = fY;    pVerts[17] = 0.0f;
    pVerts[18] = fU0;   pVerts[19] = fV0;
    pPrimitive->EndVertexData();

    m_pFilterShader->SetTextures(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP),
                                 RedRenderer::pcGetBuffer(PC_BUFFER_SKY));
    m_pFilterShader->SetSamples(8, BoxFilter);
    RedRenderer::pcRenderPrimitive(pPrimitive, m_pFilterShader, gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
    pPrimitive->Release();

    m_pClearShader->m_pTex = RedRenderer::pcGetBuffer(PC_BUFFER_SKY);
    color.a = (unsigned char)(int)(m_constantBlend * 255.0f);
    m_pClearShader->SetBlendMode(pcClearShader::BLENDMODE_CONSTANT_BLEND);

    pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x203, 24, 4, 0, false);
    pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    RedRenderer::pcGetViewport(viewport);
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);

    pVerts[0] = left;   pVerts[1] = top;     pVerts[2] = 0.0f;  pVerts[3] = 1.0f;
    pVerts[4] = 0.0f;   pVerts[5] = 0.0f;
    pVerts[6] = left;   pVerts[7] = bottom;  pVerts[8] = 0.0f;  pVerts[9] = 1.0f;
    pVerts[10] = 0.0f;  pVerts[11] = fV1;
    pVerts[12] = right; pVerts[13] = top;    pVerts[14] = 0.0f; pVerts[15] = 1.0f;
    pVerts[16] = fU1;   pVerts[17] = 0.0f;
    pVerts[18] = right; pVerts[19] = bottom; pVerts[20] = 0.0f; pVerts[21] = 1.0f;
    pVerts[22] = fU1;   pVerts[23] = fV1;
    pPrimitive->EndVertexData();

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pClearShader, gMatrixIdentity,
                                   color, 0x200, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x0040fd10
void BlurEffect::pcCopyBackBuffer() const
{
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);

    RECT dest;
    dest.left = 0;
    dest.top = 0;
    RECT src;
    src.left = (int)fMinX;
    src.top = (int)fMinY;
    src.right = (int)fMaxX;
    src.bottom = (int)fMaxY;
    dest.right = (int)((fMaxX - fMinX) * m_downSizeFactor);
    dest.bottom = (int)((fMaxY - fMinY) * m_downSizeFactor);

    RedTexture* pBuffer = RedRenderer::pcGetBuffer(PC_BUFFER_TEMP);
    IDirect3DSurface9* pBackBuffer;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    IDirect3DSurface9* pDest;
    ((IDirect3DTexture9*)pBuffer->m_pTexture)->GetSurfaceLevel(0, &pDest);
    RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &src, pDest, &dest, D3DTEXF_NONE);
    pBackBuffer->Release();
    pDest->Release();
}
