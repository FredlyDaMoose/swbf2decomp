#include "Battlefront2/Source/LoadDisplay.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// FUNCTION: SWBF2 0x004238f0
void LoadDisplay::PlatformRender()
{
    RedRenderer::BeginFrame();
    RedRenderer::pcSetNearScene(true);
    RedRenderer::pcClear(7, RedColor::BLACK, 1.0f, 0);
    RedRenderer::BeginScene();
    RenderScreen();
    RedRenderer::Flush();
    RedRenderer::EndScene();
    m_loadingScreen.Render();
    RedRenderer::pcSetNearScene(false);
    RedRenderer::EndFrame();
}

// FUNCTION: SWBF2 0x00423980
void LoadDisplay::PlatformRenderTexture(unsigned int uiTextureHash, float x0, float y0, float x1,
                                        float y1, const RedColor& color, bool bFlag, float u0,
                                        float v0, float u1, float v1, float uScale, float vScale,
                                        float uOffset, float vOffset) const
{
    if (!uiTextureHash)
    {
        return;
    }

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x203, 24, 4, 0, false);
    float* pVerts = (float*)pPrimitive->BeginVertexData();
    if (!pVerts)
    {
        return;
    }

    float left, top, right, bottom;
    RedRenderer::pcGetViewportExtents(left, top, right, bottom);
    float width = right - left;
    float height = bottom - top;

    float sx0 = width * x0 + left;
    float sy0 = height * y0 + top;
    float sx1 = width * x1 + left;
    float sy1 = height * y1 + top;
    float tu0 = u0 * uScale + uOffset;
    float tv0 = v0 * vScale + vOffset;
    float tu1 = u1 * uScale + uOffset;
    float tv1 = v1 * vScale + vOffset;

    pVerts[0] = sx0;  pVerts[1] = sy0;  pVerts[2] = 0.0f;  pVerts[3] = 1.0f;
    pVerts[4] = tu0;  pVerts[5] = tv0;
    pVerts[6] = sx0;  pVerts[7] = sy1;  pVerts[8] = 0.0f;  pVerts[9] = 1.0f;
    pVerts[10] = tu0; pVerts[11] = tv1;
    pVerts[12] = sx1; pVerts[13] = sy0; pVerts[14] = 0.0f; pVerts[15] = 1.0f;
    pVerts[16] = tu1; pVerts[17] = tv0;
    pVerts[18] = sx1; pVerts[19] = sy1; pVerts[20] = 0.0f; pVerts[21] = 1.0f;
    pVerts[22] = tu1; pVerts[23] = tv1;
    pPrimitive->EndVertexData();

    pcClearShader* pShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x203, 0, 0, 0x210004, RedColor::WHITE, RedColor::BLACK, 32.0f);
    pShader->m_pTex = RedTexture::Find(uiTextureHash);
    RedRenderer::pcRenderPrimitive(pPrimitive, pShader, gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);

    pPrimitive->Release();
    pShader->Release();
}
