#include <d3dx9math.h>

#include "Battlefront2/Source/FishEffect.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// FUNCTION: SWBF2 0x00411f90
void FishParticle::Render(unsigned char, float, unsigned int)
{
    float fSize = mType.mSize * 0.5f;
    mTimeOffCamera = 0.0f;

    PblMatrix mInverse = mMatrix.GetInverseFast();
    PblVector3 vDir;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)mBaseMatrix.m[2],
                            (const D3DXMATRIX*)&mInverse);

    const PblVector3 vRight(1.0f, 0.0f, 0.0f);
    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    const PblVector3 vAt(0.0f, 0.0f, 1.0f);
    float fHalf = fSize * 0.5f;
    float fSegment = fSize * 0.666f;
    float fFin = fSize * 0.1f;

    PblVector3 vertices[12];
    PblVector3& a = vertices[0];
    a.x = vDir.x * fSize + vUp.x * fHalf;
    a.y = vDir.y * fSize + vUp.y * fHalf;
    a.z = vDir.z * fSize + vUp.z * fHalf;
    for (int i = 1; i < 3; i++)
    {
        vertices[i].x = vertices[i - 1].x - vDir.x * fSegment;
        vertices[i].y = vertices[i - 1].y - vDir.y * fSegment;
        vertices[i].z = vertices[i - 1].z - vDir.z * fSegment;
    }
    vertices[3].x = vUp.x * fHalf - vAt.x * fSize;
    vertices[3].y = vUp.y * fHalf - vAt.y * fSize;
    vertices[3].z = vUp.z * fHalf - vAt.z * fSize;

    for (int i = 0; i < 4; i++)
    {
        vertices[i + 4] = vertices[i];
        vertices[i + 4].y = 0.0f;
    }
    for (int i = 5; i < 7; i++)
    {
        vertices[i].x += vRight.x * fFin;
        vertices[i].y += vRight.y * fFin;
        vertices[i].z += vRight.z * fFin;
    }

    for (int i = 0; i < 4; i++)
    {
        vertices[i + 8].x = vertices[i].x - vUp.x * fSize;
        vertices[i + 8].y = vertices[i].y - vUp.y * fSize;
        vertices[i + 8].z = vertices[i].z - vUp.z * fSize;
    }

    TexCoord texCoords[12] = {
        { 0.0f, 0.0f }, { 0.333f, 0.0f }, { 0.666f, 0.0f }, { 1.0f, 0.0f },
        { 0.0f, 0.5f }, { 0.333f, 0.5f }, { 0.666f, 0.5f }, { 1.0f, 0.5f },
        { 0.0f, 1.0f }, { 0.333f, 1.0f }, { 0.666f, 1.0f }, { 1.0f, 1.0f },
    };

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrimitive = CreateCompressedGeometry(vertices, texCoords);
    }
    else
    {
        pPrimitive = CreateGeometry(vertices, texCoords);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   mType.m_pShader,
                                   mMatrix,
                                   RedColor::WHITE,
                                   0x01000010, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00411eb0
void FishParticleManager::PlatformInit()
{
    unsigned int uiFormat =
        RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x9282 : 0x282;
    for (int i = 0; i < sNumFishTypes; i++)
    {
        sFishTypes[i].m_pShader = pcRedShader::Create(PblHash("Normal"), uiFormat, &sFishTypes[i].mTexture, 1,
                                                 0x50002, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
}

// FUNCTION: SWBF2 0x00411f20
void FishParticleManager::PlatformCleanup()
{
    for (int i = 0; i < sNumFishTypes; i++)
    {
        if (sFishTypes[i].m_pShader)
        {
            sFishTypes[i].m_pShader->Release();
            sFishTypes[i].m_pShader = 0;
        }
    }
    sNumFishTypes = 0;
}

// GLOBAL: SWBF2 0x0078c5f4
const RedColor kFishColor = { 0xcc, 0xcc, 0xcc, 0xff };

namespace
{

struct CompressedTexCoord
{
    short u, v;
};

struct FishVertex
{
    PblVector3 pos;
    RedColor color;
    TexCoord uv;
};
 

struct FishCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
 

inline void SetFishVertex(FishVertex& v, const PblVector3& vPos, const TexCoord& uv)
{
    v.pos = vPos;
    v.uv = uv;
    v.color = kFishColor;
}

inline void SetFishVertex(FishCompressedVertex& v, const PblVector3& vPos, const TexCoord& uv)
{
    v.pos.SetPos(vPos.x, vPos.y, vPos.z);
    v.uv.u = (short)(uv.u * 2048.0f);
    v.uv.v = (short)(uv.v * 2048.0f);
    v.color = kFishColor;
}

template <class Vertex>
void FillFishStrip(Vertex* pVerts, PblVector3* p, TexCoord* t, float fSize)
{
    float fBack = fSize * 0.2f;
    SetFishVertex(pVerts[0], p[0], t[0]);
    SetFishVertex(pVerts[1], p[4], t[4]);
    SetFishVertex(pVerts[2], p[1], t[1]);
    SetFishVertex(pVerts[3], p[5], t[5]);
    SetFishVertex(pVerts[4], p[2], t[2]);
    SetFishVertex(pVerts[5], p[6], t[6]);
    SetFishVertex(pVerts[6], p[6], t[6]);
    SetFishVertex(pVerts[7], p[10], t[10]);
    SetFishVertex(pVerts[8], p[5], t[5]);
    SetFishVertex(pVerts[9], p[9], t[9]);
    SetFishVertex(pVerts[10], p[4], t[4]);
    SetFishVertex(pVerts[11], p[8], t[8]);
    SetFishVertex(pVerts[12], p[4], t[4]);
    SetFishVertex(pVerts[13], p[9], t[9]);
    SetFishVertex(pVerts[14], PblVector3(p[5].x - fBack, p[5].y, p[5].z), t[5]);
    SetFishVertex(pVerts[15], p[10], t[10]);
    SetFishVertex(pVerts[16], PblVector3(p[6].x - fBack, p[6].y, p[6].z), t[6]);
    SetFishVertex(pVerts[17], PblVector3(p[6].x - fBack, p[6].y, p[6].z), t[6]);
    SetFishVertex(pVerts[18], p[2], t[2]);
    SetFishVertex(pVerts[19], PblVector3(p[5].x - fBack, p[5].y, p[5].z), t[5]);
    SetFishVertex(pVerts[20], p[1], t[1]);
    SetFishVertex(pVerts[21], p[4], t[4]);
    SetFishVertex(pVerts[22], p[0], t[0]);
    pVerts[23] = pVerts[22];
    SetFishVertex(pVerts[24], p[11], t[11]);
    pVerts[25] = pVerts[24];
    SetFishVertex(pVerts[26], p[7], t[7]);
    SetFishVertex(pVerts[27], p[10], t[10]);
    SetFishVertex(pVerts[28], p[6], t[6]);
    SetFishVertex(pVerts[29], p[6], t[6]);
    SetFishVertex(pVerts[30], p[2], t[2]);
    SetFishVertex(pVerts[31], p[7], t[7]);
    SetFishVertex(pVerts[32], p[3], t[3]);
    SetFishVertex(pVerts[33], p[7], t[7]);
    SetFishVertex(pVerts[34], p[2], t[2]);
    SetFishVertex(pVerts[35], PblVector3(p[6].x - fBack, p[6].y, p[6].z), t[6]);
    SetFishVertex(pVerts[36], PblVector3(p[6].x - fBack, p[6].y, p[6].z), t[6]);
    SetFishVertex(pVerts[37], p[10], t[10]);
    SetFishVertex(pVerts[38], p[7], t[7]);
    SetFishVertex(pVerts[39], p[11], t[11]);
}
}

// FUNCTION: SWBF2 0x00412440
pcRedPrimitive* FishParticle::CreateCompressedGeometry(PblVector3* pVertices, TexCoord* pTexCoords)
{
    float fSize = mType.mSize * 0.5f;
    float fExtent = mType.mSize * 2.0f * 0.5f;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282,
                                                        sizeof(FishCompressedVertex), 40, 0, false);
    PblBox box;
    box._vMin = PblVector3(-fExtent, -fExtent, -fExtent);
    box._vMax = PblVector3(fExtent, fExtent, fExtent);
    pPrimitive->m_bbox = box;
    CompressedPos::SetBoundingBox(&box);

    FishCompressedVertex* pVerts = (FishCompressedVertex*)pPrimitive->BeginVertexData();
    if (pVerts)
    {
        FillFishStrip(pVerts, pVertices, pTexCoords, fSize);
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x00413d00
pcRedPrimitive* FishParticle::CreateGeometry(PblVector3* pVertices, TexCoord* pTexCoords)
{
    float fSize = mType.mSize * 0.5f;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282,
                                                        sizeof(FishVertex), 40, 0, false);
    FishVertex* pVerts = (FishVertex*)pPrimitive->BeginVertexData();
    if (pVerts)
    {
        FillFishStrip(pVerts, pVertices, pTexCoords, fSize);
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}
