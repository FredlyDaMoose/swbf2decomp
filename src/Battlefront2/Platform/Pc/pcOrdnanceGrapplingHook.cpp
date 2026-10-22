#include <math.h>

#include "Battlefront2/Source/OrdnanceGrapplingHook.h"
#include "Battlefront2/Source/GameObject.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x01e2fd30
pcRedShader* OrdnanceGrapplingHook::sShader;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                       const PblMatrix* pM);

// GLOBAL: SWBF2 0x007eb740
static RedColor kCableColor = { 0xcc, 0xcc, 0xcc, 0xff };

struct TexCoord
{
    float u, v;
};

namespace
{
struct HookVertex
{
    PblVector3 pos;
    RedColor color;
    TexCoord uv;
};
 
}

// FUNCTION: SWBF2 0x004243a0
pcRedPrimitive* sCreateGrapplingHookGeometry(const PblBox& box, const PblVector3& vStart,
                            const PblVector3& vEnd, int iSegments, const TexCoord* pTexCoords,
                            float fAlpha, const PblVector3& vDirection,
                            const PblVector3& vWaveDirection, const PblVector3& vOffset,
                            float fWaveHeight, float fWaveLength, bool bWavy, float fLength)
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, 24,
                                                         2 * iSegments + 2, 0, false);
    pPrimitive->m_bbox = box;
    HookVertex* pVertices = (HookVertex*)pPrimitive->BeginVertexData();
    if (pVertices)
    {
        const float fSegmentLength = fLength / (float)iSegments;
        const unsigned char ucAlpha = (unsigned char)(int)(fAlpha * 255.0f);
        for (int i = 0; i <= iSegments; ++i)
        {
            float fRatio = (float)i / (float)iSegments;
            float fDistance = (float)i * fSegmentLength;
            PblVector3 vCenter(vStart.x + vDirection.x * fDistance,
                               vStart.y + vDirection.y * fDistance,
                               vStart.z + vDirection.z * fDistance);
            if (i == iSegments)
            {
                vCenter = vEnd;
            }
            if (bWavy)
            {
                float fWave = PblMath::Sin((float)i * 6.2831855f * fSegmentLength)
                            * (fWaveHeight / fWaveLength)
                            * PblMath::Sin(fRatio * 3.1415927f);
                vCenter.x += vWaveDirection.x * fWave;
                vCenter.y += vWaveDirection.y * fWave;
                vCenter.z += vWaveDirection.z * fWave;
            }

            HookVertex& front = pVertices[i * 2];
            HookVertex& back = pVertices[i * 2 + 1];
            front.pos.x = vCenter.x + vOffset.x * 0.02f;
            front.pos.y = vCenter.y + vOffset.y * 0.02f;
            front.pos.z = vCenter.z + vOffset.z * 0.02f;
            back.pos.x = front.pos.x - vOffset.x * 0.04f;
            back.pos.y = front.pos.y - vOffset.y * 0.04f;
            back.pos.z = front.pos.z - vOffset.z * 0.04f;
            front.color.b = back.color.b = 0xcc;
            front.color.g = back.color.g = 0xcc;
            front.color.r = back.color.r = 0xcc;
            front.color.a = back.color.a = ucAlpha;
            front.uv = pTexCoords[(i & 1) ? 2 : 0];
            back.uv = pTexCoords[(i & 1) ? 3 : 1];
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

namespace
{
struct HookVertexCompressed
{
    short pos[4];
    RedColor color;
    short u, v;
};
 
}

static void HookCompressedPos_Set(short* p, float x, float y, float z)
{
    p[0] = (short)(int)((x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
    p[1] = (short)(int)((y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
    p[2] = (short)(int)((z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
}

// FUNCTION: SWBF2 0x00423f30
pcRedPrimitive* sCreateGrapplingHookCompressedGeometry(const PblBox& box, const PblVector3& vStart,
                           const PblVector3& vEnd, int iSegments, const TexCoord* pTexCoords,
                           float fAlpha, const PblVector3& vDirection,
                           const PblVector3& vWaveDirection, const PblVector3& vOffset,
                           float fWaveHeight, float fWaveLength, bool bWavy, float fLength)
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, 16,
                                                         2 * iSegments + 2, 0, false);
    pPrimitive->m_bbox = box;

    PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
    PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                box._vMax.z - box._vMin.z };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;

    HookVertexCompressed* pVertices = (HookVertexCompressed*)pPrimitive->BeginVertexData();
    if (pVertices)
    {
        const float fSegmentLength = fLength / (float)iSegments;
        for (int i = 0; i <= iSegments; ++i)
        {
            float fRatio = (float)i / (float)iSegments;
            float fDistance = (float)i * fSegmentLength;
            PblVector3 vCenter(vStart.x + vDirection.x * fDistance,
                               vStart.y + vDirection.y * fDistance,
                               vStart.z + vDirection.z * fDistance);
            if (i == iSegments)
            {
                vCenter = vEnd;
            }
            if (bWavy)
            {
                float fWave = PblMath::Sin((float)i * 6.2831855f * fSegmentLength)
                            * (fWaveHeight / fWaveLength)
                            * PblMath::Sin(fRatio * 3.1415927f);
                vCenter.x += vWaveDirection.x * fWave;
                vCenter.y += vWaveDirection.y * fWave;
                vCenter.z += vWaveDirection.z * fWave;
            }

            const float fx = vCenter.x + vOffset.x * 0.02f;
            const float fy = vCenter.y + vOffset.y * 0.02f;
            const float fz = vCenter.z + vOffset.z * 0.02f;
            const TexCoord* pRow = (i & 1) ? pTexCoords + 2 : pTexCoords;

            HookVertexCompressed& front = pVertices[i * 2];
            HookVertexCompressed& back = pVertices[i * 2 + 1];
            HookCompressedPos_Set(front.pos, fx, fy, fz);
            front.u = (short)(int)(pRow[0].u * 2048.0f);
            front.v = (short)(int)(pRow[0].v * 2048.0f);
            front.color = kCableColor;
            front.color.a = (unsigned char)(int)(fAlpha * 255.0f);

            HookCompressedPos_Set(back.pos, fx - vOffset.x * 0.04f, fy - vOffset.y * 0.04f,
                                  fz - vOffset.z * 0.04f);
            back.u = (short)(int)(pRow[1].u * 2048.0f);
            back.v = (short)(int)(pRow[1].v * 2048.0f);
            back.color = kCableColor;
            back.color.a = (unsigned char)(int)(fAlpha * 255.0f);
        }
        pPrimitive->EndVertexData();
    }
    return pPrimitive;
}

// FUNCTION: SWBF2 0x00424690
void OrdnanceGrapplingHook::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!mOwner)
    {
        return;
    }

    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    PblVector3 vCameraRight(mCamera.m[0][0], mCamera.m[0][1], mCamera.m[0][2]);
    PblVector3 vStart = GetPosition();
    PblVector3 vEnd;
    D3DXVec3TransformCoord(&vEnd, &mWeaponOffset, &mOwner->GetMatrix());
    if (mState == HOOK_LIFTING)
    {
        vEnd.x += mVelocity.x * mDeltaTime;
        vEnd.y += mVelocity.y * mDeltaTime;
        vEnd.z += mVelocity.z * mDeltaTime;
    }

    PblVector3 vDirection(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    PblVector3 vSide(vDirection.y * vUp.z - vDirection.z * vUp.y,
                     vDirection.z * vUp.x - vDirection.x * vUp.z,
                     vDirection.x * vUp.y - vDirection.y * vUp.x);
    PblVector3 vWave(vSide.y * vDirection.z - vSide.z * vDirection.y,
                     vSide.z * vDirection.x - vSide.x * vDirection.z,
                     vSide.x * vDirection.y - vSide.y * vDirection.x);
    PblVector3 vWaveDirection;
    D3DXVec3Normalize(&vWaveDirection, &vWave);

    float fLength = sqrtf(vDirection.x * vDirection.x + vDirection.y * vDirection.y
                          + vDirection.z * vDirection.z);
    if (fLength < 0.0001f)
    {
        return;
    }
    float fInvLength = 1.0f / fLength;
    vDirection.x *= fInvLength;
    vDirection.y *= fInvLength;
    vDirection.z *= fInvLength;

    int iSegments = (int)(fLength * 8.0f);
    if (iSegments < 2)
    {
        iSegments = 2;
    }
    else if (iSegments > 80)
    {
        iSegments = 80;
    }
    if (mMaxLifetime < 0.0001f)
    {
        return;
    }

    PblBox box;
    box._vMin = PblVector3((vEnd.x > vStart.x ? vStart.x : vEnd.x) - 1.0f,
                           (vEnd.y > vStart.y ? vStart.y : vEnd.y) - 1.0f,
                           (vEnd.z > vStart.z ? vStart.z : vEnd.z) - 1.0f);
    box._vMax = PblVector3((vEnd.x > vStart.x ? vEnd.x : vStart.x) + 1.0f,
                           (vEnd.y > vStart.y ? vEnd.y : vStart.y) + 1.0f,
                           (vEnd.z > vStart.z ? vEnd.z : vStart.z) + 1.0f);
    TexCoord uv[4] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } };

    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        pPrimitive = sCreateGrapplingHookCompressedGeometry(box, vStart, vEnd, iSegments, uv, fDist, vDirection,
                                vWaveDirection, vCameraRight, mLifeTime, mMaxLifetime,
                                mState == HOOK_ATTACHING, fLength);
    }
    else
    {
        pPrimitive = sCreateGrapplingHookGeometry(box, vStart, vEnd, iSegments, uv, fDist, vDirection,
                                vWaveDirection, vCameraRight, mLifeTime, mMaxLifetime,
                                mState == HOOK_ATTACHING, fLength);
    }

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   sShader,
                                   gMatrixIdentity, RedColor::WHITE,
                                   0x1000010, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00423ea0
void OrdnanceGrapplingHook::PlatformInit()
{
    sShader = pcRedShader::Create(PblHash("Particle"),
                                  RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED
                                      ? 0x9282 : 0x282,
                                  &sTextureHash, 1, 0x10004, RedColor::WHITE, RedColor::BLACK,
                                  32.0f);
}

// FUNCTION: SWBF2 0x00423ef0
void OrdnanceGrapplingHook::PlatformCleanup()
{
    if (sShader)
    {
        sShader->Release();
        sShader = 0;
    }
}
