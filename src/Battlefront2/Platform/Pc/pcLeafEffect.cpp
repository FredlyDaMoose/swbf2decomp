#include "Battlefront2/Source/LeafEffect.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

namespace
{
struct LeafVertex
{
    float x, y, z;
    RedColor color;
    float u, v;
};

struct CompressedLeafVertex
{
    short x, y, z, pad;
    RedColor color;
    unsigned int uv;
};
}

// FUNCTION: SWBF2 0x0041a380
pcRedPrimitive* FallingLeafParticle::CreateGeometry(float fSize, RedColor color)
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, 24, 4, 0, false);
    LeafVertex* pVertices = (LeafVertex*)pPrimitive->BeginVertexData();
    if (pVertices)
    {
        pVertices[0].x = -fSize;
        pVertices[0].y = 0.0f;
        pVertices[0].z = -fSize;
        pVertices[0].color = color;
        pVertices[0].u = 0.0f;
        pVertices[0].v = 0.0f;

        pVertices[1].x = -fSize;
        pVertices[1].y = 0.0f;
        pVertices[1].z = fSize;
        pVertices[1].color = color;
        pVertices[1].u = 1.0f;
        pVertices[1].v = 0.0f;

        pVertices[2].x = fSize;
        pVertices[2].y = 0.0f;
        pVertices[2].z = -fSize;
        pVertices[2].color = color;
        pVertices[2].u = 0.0f;
        pVertices[2].v = 1.0f;

        pVertices[3].x = fSize;
        pVertices[3].y = 0.0f;
        pVertices[3].z = fSize;
        pVertices[3].color = color;
        pVertices[3].u = 1.0f;
        pVertices[3].v = 1.0f;
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x0041a470
pcRedPrimitive* FallingLeafParticle::CreateCompressedGeometry(float fSize, RedColor color)
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, 16, 4, 0, false);
    pPrimitive->m_bbox = PblBox(-0.25f, -0.25f, -0.25f, 0.25f, 0.25f, 0.25f);

    CompressedPos::s_vMin.x = pPrimitive->m_bbox._vMin.x;
    CompressedPos::s_vMin.y = pPrimitive->m_bbox._vMin.y;
    CompressedPos::s_vMin.z = pPrimitive->m_bbox._vMin.z;
    CompressedPos::s_vScale.x = (pPrimitive->m_bbox._vMax.x - pPrimitive->m_bbox._vMin.x) * (1.0f / 65535.0f);
    CompressedPos::s_vScale.y = (pPrimitive->m_bbox._vMax.y - pPrimitive->m_bbox._vMin.y) * (1.0f / 65535.0f);
    CompressedPos::s_vScale.z = (pPrimitive->m_bbox._vMax.z - pPrimitive->m_bbox._vMin.z) * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;

    CompressedLeafVertex* pVertices = (CompressedLeafVertex*)pPrimitive->BeginVertexData();
    if (pVertices)
    {
        for (int i = 0; i < 4; ++i)
        {
            const float x = (i & 2) ? fSize : -fSize;
            const float z = (i & 1) ? fSize : -fSize;
            pVertices[i].x = (short)(int)((x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
            pVertices[i].y = (short)(int)((0.0f - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
            pVertices[i].z = (short)(int)((z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
            pVertices[i].color = color;
        }
        pVertices[0].uv = 0;
        pVertices[1].uv = 0x800;
        pVertices[2].uv = 0x08000000;
        pVertices[3].uv = 0x08000800;
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x0041a2c0
void FallingLeafParticle::Render(unsigned char, float, unsigned int)
{
    unsigned char ucGrey = (unsigned char)(int)((mRandomness * 0.6f + 0.4f) * 255.0f);
    mRenderTimer = 0.0f;

    RedColor color;
    color.b = ucGrey;
    color.g = ucGrey;
    color.r = ucGrey;
    color.a = 0xff;

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrimitive = CreateCompressedGeometry(0.17f, color);
    }
    else
    {
        pPrimitive = CreateGeometry(0.17f, color);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pShader, mMatrix, RedColor::WHITE, 0x200, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x0041a280
void FallingLeafParticle::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
    }
}

// FUNCTION: SWBF2 0x0041a230
void FallingLeafParticle::PlatformInit()
{
    const unsigned int uiFormat = RedRenderer::pcGetGeometryMode() ? 0x9282 : 0x282;
    m_pShader = pcRedShader::Create(PblHash("Particle"), uiFormat, &mTextureHash, 1, 0x10004,
                                    RedColor::WHITE, RedColor::BLACK, 32.0f);
}
