#include <string.h>

#include "Battlefront2/Source/Cloth.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

namespace
{

struct PosXYZ
{
    void SetPos(float fX, float fY, float fZ)
    {
        x = fX;
        y = fY;
        z = fZ;
    }
    float x, y, z;
};

struct NormalXYZ
{
    void SetNormal(const PblVector3& vNormal)
    {
        x = vNormal.x;
        y = vNormal.y;
        z = vNormal.z;
    }
    float x, y, z;
};

struct TexCoord
{
    float u, v;
};

struct ClothVertex
{
    PosXYZ pos;
    NormalXYZ normal;
    TexCoord uv;
};
 
}

// FUNCTION: SWBF2 0x0040feb0
void ClothData::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x0040ff00
void EntityCloth::PlatformRender(unsigned char ucViewport, RedColor color, unsigned int uiFlags,
                                 const RedLightingState* pLightingState, const PblSphere& sphere)
{
    if (pLightingState)
    {
        RedRenderer::SetLightingState(*pLightingState);
    }

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x222, sizeof(ClothVertex),
                               (unsigned short)(mData->m_uiNumParticles * 2),
                               (unsigned short)(mData->m_uiNumTriangles * 6), false);

    ClothVertex* pVertices = (ClothVertex*)pPrimitive->BeginVertexData();
    if (!pVertices)
    {
        return;
    }

    const PblVector3* pTexCoords = mData->m_pTexCoords;
    for (unsigned int i = 0; i < mData->m_uiNumParticles; ++i)
    {
        pVertices[i].pos.SetPos(m_pPos[i].x, m_pPos[i].y, m_pPos[i].z);
        pVertices[i].normal.SetNormal(m_pNormal[i]);
        pVertices[i].uv.u = pTexCoords[i].x;
        pVertices[i].uv.v = pTexCoords[i].y;
    }
    ClothVertex* pBack = pVertices + mData->m_uiNumParticles;
    for (unsigned int i = 0; i < mData->m_uiNumParticles; ++i)
    {
        pBack[i].pos.SetPos(m_pPos[i].x, m_pPos[i].y, m_pPos[i].z);
        pBack[i].normal.SetNormal(PblVector3(-m_pNormal[i].x, -m_pNormal[i].y, -m_pNormal[i].z));
        pBack[i].uv.u = pTexCoords[i].x;
        pBack[i].uv.v = pTexCoords[i].y;
    }
    pPrimitive->EndVertexData();

    const unsigned short* pTriangles = mData->m_pTriangleIndices;
    unsigned short* pIndices = pPrimitive->BeginIndexData();
    memcpy(pIndices, pTriangles, mData->m_uiNumTriangles * 3 * sizeof(unsigned short));
    const unsigned short usBase = (unsigned short)mData->m_uiNumParticles;
    unsigned short* pBackIndices = pIndices + mData->m_uiNumTriangles * 3;
    for (unsigned int i = 0; i < mData->m_uiNumTriangles * 3; ++i)
    {
        pBackIndices[i] = (unsigned short)(pTriangles[mData->m_uiNumTriangles * 3 - i - 1] + usBase);
    }
    pPrimitive->EndIndexData();

    RedRenderer::pcRenderPrimitive(pPrimitive, mData->m_pShader,
                                   m_worldMat, color, uiFlags, 0,
                                   0, 0);

    PblMatrix mReflection;
    if (!(uiFlags & 0x200000)
        && FLRenderer::IsReflected(sphere.GetCenter(), sphere.GetRadius(), mReflection, false))
    {
        PblMatrix mReflected;
        D3DXMatrixMultiply(&mReflected, &m_worldMat, &mReflection);
        RedRenderer::pcRenderPrimitive(pPrimitive, mData->m_pShader,
                                       mReflected, color,
                                       uiFlags | 0x10000100, 0, 0, 0);
    }

    pPrimitive->Release();
}
