#include "Battlefront2/Source/ScreenFilterEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

// GLOBAL: SWBF2 0x01f9ae58
static pcClearShader* s_pClearShader;

// FUNCTION: SWBF2 0x00426ed0
void ScreenFilterEffect::PlatformInit()
{
    s_pClearShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x203, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x00426f00
void ScreenFilterEffect::PlatformCleanup()
{
    if (s_pClearShader)
    {
        s_pClearShader->Release();
        s_pClearShader = 0;
    }
}

// FUNCTION: SWBF2 0x00426f40
void ScreenFilterEffect::PlatformRender(unsigned int) const
{
    unsigned int uiViewport = FLRenderer::GetCurrentViewport();
    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x203, 24, 4, 0, false);
    float* pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    float left, top, right, bottom;
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);

    pVerts[0] = left;   pVerts[1] = top;     pVerts[2] = 0.0f;  pVerts[3] = 1.0f;
    pVerts[4] = 0.0f;   pVerts[5] = 0.0f;
    pVerts[6] = left;   pVerts[7] = bottom;  pVerts[8] = 0.0f;  pVerts[9] = 1.0f;
    pVerts[10] = 0.0f;  pVerts[11] = 1.0f;
    pVerts[12] = right; pVerts[13] = top;    pVerts[14] = 0.0f; pVerts[15] = 1.0f;
    pVerts[16] = 1.0f;  pVerts[17] = 0.0f;
    pVerts[18] = right; pVerts[19] = bottom; pVerts[20] = 0.0f; pVerts[21] = 1.0f;
    pVerts[22] = 1.0f;  pVerts[23] = 1.0f;
    pPrimitive->EndVertexData();

    const FilterData& data = m_data[uiViewport];
    s_pClearShader->m_pTex = RedTexture::Find(data.uiTextureHash);
    RedColor color;
    color.b = data.filterColor.b;
    color.g = data.filterColor.g;
    color.r = data.filterColor.r;
    color.a = (unsigned char)(int)(data.filterColor.a * data.fIntensity);
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pClearShader, gMatrixIdentity,
                                   color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}
