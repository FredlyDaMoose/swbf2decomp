#include <float.h>
#include <math.h>

#include "Battlefront2/Source/OrdnanceTowCable.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblHermite.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x01e2fd34
pcRedShader* OrdnanceTowCable::sShader;

// FUNCTION: SWBF2 0x00424ce0
void OrdnanceTowCable::PlatformInit()
{
    sTextureHash = PblHash("white");
    sShader = pcRedShader::Create(PblHash("Particle"),
                                  RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED
                                      ? 0x9282 : 0x282,
                                  &sTextureHash, 1, 0x10004, RedColor::WHITE, RedColor::BLACK,
                                  32.0f);
}

// GLOBAL: SWBF2 0x007eb744
static RedColor kCableColor = { 0, 0, 0, 255 };

struct TexCoord
{
    float u;
    float v;
};

struct CableVertexCompressed
{
    short pos[4];
    RedColor color;
    short u, v;
};
struct CableVertex
{
    float x, y, z;
    RedColor color;
    float u, v;
};

static void CableCompressedPos_Set(short* p, float x, float y, float z)
{
    p[0] = (short)(int)((x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
    p[1] = (short)(int)((y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
    p[2] = (short)(int)((z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
}

static PblVector3 sCableSide(const RedCamera* pCamera, const PblVector3& vSlope, float fThickness)
{
    const PblMatrix& mat = pCamera->_Matrix;
    float bx = -mat.m[2][0];
    float by = -mat.m[2][1];
    float bz = -mat.m[2][2];
    PblVector3 vSide;
    vSide.x = vSlope.z * by - vSlope.y * bz;
    vSide.y = vSlope.x * bz - vSlope.z * bx;
    vSide.z = vSlope.y * bx - vSlope.x * by;
    float fLength = sqrtf(vSide.y * vSide.y + vSide.x * vSide.x + vSide.z * vSide.z);
    if (fLength >= 0.0001f)
    {
        vSide.x = vSide.x * (1.0f / fLength);
        vSide.y = vSide.y * (1.0f / fLength);
        vSide.z = vSide.z * (1.0f / fLength);
    }
    else
    {
        vSide.x = mat.m[1][0];
        vSide.y = mat.m[1][1];
        vSide.z = mat.m[1][2];
    }
    vSide.x *= fThickness;
    vSide.y *= fThickness;
    vSide.z *= fThickness;
    return vSide;
}

// FUNCTION: SWBF2 0x00424d70
pcRedPrimitive* sCreateCableCompressedGeometry(const PblBox& box, const PblVector3* pPoints,
                                               const PblVector3* pSlopes, float fThickness,
                                               const TexCoord* pUV, float fAlpha, int iNumPoints)
{
    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, 16, iNumPoints * 2, 0, false);
    pPrimitive->m_bbox = box;

    PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
    PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                box._vMax.z - box._vMin.z };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;

    CableVertexCompressed* pVert = (CableVertexCompressed*)pPrimitive->BeginVertexData();
    if (pVert)
    {
        const RedCamera* pCamera = RedRenderer::GetCamera();
        for (int i = 0; i < iNumPoints; i++)
        {
            PblVector3 vSide = sCableSide(pCamera, pSlopes[i], fThickness);
            const PblVector3& vPoint = pPoints[i];
            const TexCoord* pRow = (i & 1) ? pUV + 2 : pUV;

            RedColor color = kCableColor;
            color.a = (unsigned char)(int)(fAlpha * 255.0f);

            CableCompressedPos_Set(pVert[0].pos, vPoint.x + vSide.x, vPoint.y + vSide.y, vPoint.z + vSide.z);
            pVert[0].u = (short)(int)(pRow[0].u * 2048.0f);
            pVert[0].v = (short)(int)(pRow[0].v * 2048.0f);
            pVert[0].color = color;

            CableCompressedPos_Set(pVert[1].pos, vPoint.x - vSide.x, vPoint.y - vSide.y, vPoint.z - vSide.z);
            pVert[1].u = (short)(int)(pRow[1].u * 2048.0f);
            pVert[1].v = (short)(int)(pRow[1].v * 2048.0f);
            pVert[1].color = color;
            pVert += 2;
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x004251b0
pcRedPrimitive* sCreateCableGeometry(const PblBox& box, const PblVector3* pPoints, const PblVector3* pSlopes,
                                     float fThickness, const TexCoord* pUV, float fAlpha, int iNumPoints)
{
    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, 24, iNumPoints * 2, 0, false);
    pPrimitive->m_bbox = box;

    CableVertex* pVert = (CableVertex*)pPrimitive->BeginVertexData();
    if (pVert)
    {
        const RedCamera* pCamera = RedRenderer::GetCamera();
        for (int i = 0; i < iNumPoints; i++)
        {
            PblVector3 vSide = sCableSide(pCamera, pSlopes[i], fThickness);
            const PblVector3& vPoint = pPoints[i];
            const TexCoord* pRow = (i & 1) ? pUV + 2 : pUV;

            RedColor color = kCableColor;
            color.a = (unsigned char)(int)(fAlpha * 255.0f);

            pVert[0].x = vPoint.x + vSide.x;
            pVert[0].y = vPoint.y + vSide.y;
            pVert[0].z = vPoint.z + vSide.z;
            pVert[0].u = pRow[0].u;
            pVert[0].v = pRow[0].v;
            pVert[0].color = color;

            pVert[1].x = vPoint.x - vSide.x;
            pVert[1].y = vPoint.y - vSide.y;
            pVert[1].z = vPoint.z - vSide.z;
            pVert[1].u = pRow[1].u;
            pVert[1].v = pRow[1].v;
            pVert[1].color = color;
            pVert += 2;
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x004254a0
void sRenderHermiteSegment(PblHermiteVector3& segment, unsigned char ucViewport, float fDist,
                           unsigned int uiFlags)
{
    int iNumPoints = (int)(segment.GetLengthApproximation(10) * 0.5f);
    if (iNumPoints < 2)
    {
        iNumPoints = 2;
    }
    else if (iNumPoints > 25)
    {
        iNumPoints = 25;
    }

    PblVector3 vSlopes[25];
    PblVector3 vPoints[25];
    float fStep = 1.0f / ((float)iNumPoints - 1.0f);
    float fEnd = fStep * 0.5f + 1.0f;
    PblVector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    PblVector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    int i = 0;
    for (float fT = 0.0f; fEnd > fT; fT += fStep)
    {
        vPoints[i] = segment.getValue(fT);
        if (vPoints[i].x <= vMin.x)
        {
            vMin.x = vPoints[i].x;
        }
        if (vPoints[i].y <= vMin.y)
        {
            vMin.y = vPoints[i].y;
        }
        if (vPoints[i].z <= vMin.z)
        {
            vMin.z = vPoints[i].z;
        }
        if (vMax.x <= vPoints[i].x)
        {
            vMax.x = vPoints[i].x;
        }
        if (vMax.y <= vPoints[i].y)
        {
            vMax.y = vPoints[i].y;
        }
        if (vMax.z <= vPoints[i].z)
        {
            vMax.z = vPoints[i].z;
        }
        vSlopes[i] = segment.getSlope(fT);
        i++;
    }

    const float fThickness = 0.05f;
    PblBox box;
    box._vMin = PblVector3(vMin.x - fThickness, vMin.y - fThickness, vMin.z - fThickness);
    box._vMax = PblVector3(vMax.x + fThickness, vMax.y + fThickness, vMax.z + fThickness);
    TexCoord uv[4] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } };

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        pPrimitive = sCreateCableCompressedGeometry(box, vPoints, vSlopes, fThickness, uv, fDist, iNumPoints);
    }
    else
    {
        pPrimitive = sCreateCableGeometry(box, vPoints, vSlopes, fThickness, uv, fDist, iNumPoints);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   OrdnanceTowCable::sShader,
                                   gMatrixIdentity, RedColor::WHITE,
                                   0x1000010, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00424d30
void OrdnanceTowCable::PlatformCleanup()
{
    if (sShader)
    {
        sShader->Release();
        sShader = 0;
    }
}
