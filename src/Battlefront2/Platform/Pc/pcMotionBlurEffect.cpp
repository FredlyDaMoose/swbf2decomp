#include "Battlefront2/Source/MotionBlurEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x01f9ae54
static pcClearShader* s_pClearShader;

// FUNCTION: SWBF2 0x00423bd0
void MotionBlurEffect::PlatformInit()
{
    s_pClearShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x403, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x00423c00
void MotionBlurEffect::PlatformCleanup()
{
    if (s_pClearShader)
    {
        s_pClearShader->Release();
        s_pClearShader = 0;
    }
}

// FUNCTION: SWBF2 0x00423c40
void MotionBlurEffect::PlatformRender(unsigned int) const
{
    const unsigned int uiView = FLRenderer::GetCurrentViewport();
    if (m_curBlurAmount[uiView] == 0.0f)
    {
        return;
    }
    if (!(RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES))
    {
        return;
    }
    if (!RedRenderer::pcGetRenderOptions().motionBlur)
    {
        return;
    }

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x403, 32, 4, 0, false);
    float* pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);

    pVerts[0] = fMinX;
    pVerts[1] = fMinY;
    pVerts[2] = 0.0f;
    pVerts[3] = 1.0f;
    pVerts[4] = viewport.fOffsetX;
    pVerts[5] = viewport.fOffsetY;
    pVerts[6] = 0.0f;
    pVerts[7] = 0.0f;
    pVerts[8] = fMinX;
    pVerts[9] = fMaxY;
    pVerts[10] = 0.0f;
    pVerts[11] = 1.0f;
    pVerts[12] = viewport.fOffsetX;
    pVerts[13] = viewport.fHeight + viewport.fOffsetY;
    pVerts[14] = 0.0f;
    pVerts[15] = 1.0f;
    pVerts[16] = fMaxX;
    pVerts[17] = fMinY;
    pVerts[18] = 0.0f;
    pVerts[19] = 1.0f;
    pVerts[20] = viewport.fWidth + viewport.fOffsetX;
    pVerts[21] = viewport.fOffsetY;
    pVerts[22] = 1.0f;
    pVerts[23] = 0.0f;
    pVerts[24] = fMaxX;
    pVerts[25] = fMaxY;
    pVerts[26] = 0.0f;
    pVerts[27] = 1.0f;
    pVerts[28] = viewport.fWidth + viewport.fOffsetX;
    pVerts[29] = viewport.fHeight + viewport.fOffsetY;
    pVerts[30] = 1.0f;
    pVerts[31] = 1.0f;
    pPrimitive->EndVertexData();

    s_pClearShader->m_pTex = FLRenderer::pcGetPreviousFrameTexture();
    s_pClearShader->m_pAlphaMaskTex = RedTexture::Find(0xF745F270);

    RedColor color;
    color.b = RedColor::WHITE.b;
    color.g = RedColor::WHITE.g;
    color.r = RedColor::WHITE.r;
    color.a = (unsigned char)(m_curBlurAmount[uiView] * 255.0f);
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pClearShader, gMatrixIdentity,
                                   color, 0x200, 0, 0, 0);
    pPrimitive->Release();
}
