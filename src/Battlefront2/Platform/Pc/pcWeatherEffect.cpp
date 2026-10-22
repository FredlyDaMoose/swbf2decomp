#include <math.h>

#include "Battlefront2/Source/CameraManager.h"
#include "Battlefront2/Source/GameCamera.h"
#include "Battlefront2/Source/WeatherEffect.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include "RedEngineFL/Application/RedTerrain.h"
#include "pcSunFlare.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcRainShader.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);

namespace
{

struct TexturedVertex
{
    struct VertexC
    {
        short x, y, z, w;
        RedColor color;
        short u, v;

        void SetPos(const PblVector3& vPos)
        {
            x = (short)((vPos.x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
            y = (short)((vPos.y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
            z = (short)((vPos.z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
        }
        void SetColor(const RedColor& c) { color = c; }
        void SetUV(float fU, float fV)
        {
            u = (short)(fU * 2048.0f);
            v = (short)(fV * 2048.0f);
        }
    };

    struct VertexU
    {
        PblVector3 pos;
        RedColor color;
        float u, v;

        void SetPos(const PblVector3& vPos) { pos = vPos; }
        void SetColor(const RedColor& c) { color = c; }
        void SetUV(float fU, float fV)
        {
            u = fU;
            v = fV;
        }
    };
};

struct UntexturedVertex
{
    struct VertexC
    {
        short x, y, z, w;
        RedColor color;

        void SetPos(const PblVector3& vPos)
        {
            x = (short)((vPos.x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
            y = (short)((vPos.y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
            z = (short)((vPos.z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
        }
        void SetColor(const RedColor& c) { color = c; }
    };

    struct VertexU
    {
        PblVector3 pos;
        RedColor color;

        void SetPos(const PblVector3& vPos) { pos = vPos; }
        void SetColor(const RedColor& c) { color = c; }
    };
};

void CompressedPos_SetBoundingBox(const PblBox& box)
{
    PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
    PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                box._vMax.z - box._vMin.z };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;
}
}

// FUNCTION: SWBF2 0x00433ef0
void FogCloudEffect::PlatformInit()
{
    if (!m_pShader)
    {
        if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
        {
            m_pShader = pcRedShader::Create(PblHash("Particle"), 0x9282, &TEXTURE_HASH, 1, 0x10004,
                                            RedColor::WHITE, RedColor::BLACK, 32.0f);
        }
        else
        {
            m_pShader = pcRedShader::Create(PblHash("Particle"), 0x282, &TEXTURE_HASH, 1, 0x10004,
                                            RedColor::WHITE, RedColor::BLACK, 32.0f);
        }
    }
}

// FUNCTION: SWBF2 0x00433f50
void FogCloudEffect::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00433f90
void FogCloudEffect::Render(int value)
{
    if ((unsigned int)value & 0x200000)
    {
        return;
    }

    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    fHeight = 1.0f / fNear * fHeight;
    fWidth = 1.0f / fNear * fWidth;
    float fInvLength = 1.0f / sqrtf(fWidth * fWidth + 1.0f + fHeight * fHeight);

    const float* vRight = mCamera.m[0];
    const float* vUp = mCamera.m[1];
    const float* vAt = mCamera.m[2];
    const float* vPos = mCamera.m[3];

    float fRightX = vRight[0] * fWidth, fRightY = vRight[1] * fWidth, fRightZ = vRight[2] * fWidth;
    PblVector3 vCorners[4];
    vCorners[0].x = ((-vAt[0] + fRightX) + vUp[0] * fHeight) * fInvLength;
    vCorners[0].y = ((-vAt[1] + fRightY) + vUp[1] * fHeight) * fInvLength;
    vCorners[0].z = ((-vAt[2] + fRightZ) + vUp[2] * fHeight) * fInvLength;
    vCorners[1].x = ((-vAt[0] + fRightX) - vUp[0] * fHeight) * fInvLength;
    vCorners[1].y = ((-vAt[1] + fRightY) - vUp[1] * fHeight) * fInvLength;
    vCorners[1].z = ((-vAt[2] + fRightZ) - vUp[2] * fHeight) * fInvLength;
    vCorners[2].x = ((-vAt[0] - fRightX) - vUp[0] * fHeight) * fInvLength;
    vCorners[2].y = ((-vAt[1] - fRightY) - vUp[1] * fHeight) * fInvLength;
    vCorners[2].z = ((-vAt[2] - fRightZ) - vUp[2] * fHeight) * fInvLength;
    vCorners[3].x = ((-vAt[0] - fRightX) + vUp[0] * fHeight) * fInvLength;
    vCorners[3].y = ((-vAt[1] - fRightY) + vUp[1] * fHeight) * fInvLength;
    vCorners[3].z = ((-vAt[2] - fRightZ) + vUp[2] * fHeight) * fInvLength;

    PblBox box;
    box._vMin = PblVector3(vPos[0] - vAt[0] * DISTANCE_NEAR, vPos[1] - vAt[1] * DISTANCE_NEAR,
                           vPos[2] - vAt[2] * DISTANCE_NEAR);
    box._vMax = box._vMin;
    box.AddPoint(PblVector3(vPos[0] - vAt[0] * DISTANCE_FAR, vPos[1] - vAt[1] * DISTANCE_FAR,
                            vPos[2] - vAt[2] * DISTANCE_FAR));
    for (int i = 0; i < 4; ++i)
    {
        box.AddPoint(PblVector3(vPos[0] + vCorners[i].x * DISTANCE_NEAR,
                                vPos[1] + vCorners[i].y * DISTANCE_NEAR,
                                vPos[2] + vCorners[i].z * DISTANCE_NEAR));
        box.AddPoint(PblVector3(vPos[0] + vCorners[i].x * DISTANCE_FAR,
                                vPos[1] + vCorners[i].y * DISTANCE_FAR,
                                vPos[2] + vCorners[i].z * DISTANCE_FAR));
    }

    float fRadius = PARTICLE_SIZE * 0.7071f;
    box._vMin.x -= fRadius;
    box._vMin.y -= fRadius;
    box._vMin.z -= fRadius;
    box._vMax.x += fRadius;
    box._vMax.y += fRadius;
    box._vMax.z += fRadius;

    int iMinX = (int)floorf(BOX_SIZEINV * box._vMin.x);
    int iMaxX = (int)floorf(BOX_SIZEINV * box._vMax.x);
    int iMinZ = (int)floorf(BOX_SIZEINV * box._vMin.z);
    int iMaxZ = (int)floorf(BOX_SIZEINV * box._vMax.z);

    PblBox bbox;
    bbox._vMin = PblVector3(box._vMin.x - fRadius, box._vMin.y - fRadius, box._vMin.z - fRadius);
    bbox._vMax = PblVector3(box._vMax.x + fRadius, box._vMax.y + fRadius, box._vMax.z + fRadius);

    unsigned int uiNumParticles = (iMaxZ - iMinZ + 1) * (iMaxX - iMinX + 1) * PARTICLES_PER_BOX;

    float fTime = _TimeStamp.GetElapsed();
    float fOffsetX = fmodf(VELOCITY.x * fTime, BOX_SIZE);
    float fOffsetZ = fmodf(VELOCITY.z * fTime, BOX_SIZE);
    if (fOffsetX < 0.0f)
    {
        fOffsetX += BOX_SIZE;
    }
    if (fOffsetZ < 0.0f)
    {
        fOffsetZ += BOX_SIZE;
    }
    float fRotation = fmodf(ROTATION_RATE * fTime + 1.0f, 2.0f) - 1.0f;
    if (fRotation < -1.0f)
    {
        fRotation += 2.0f;
    }

    float fNearSq = DISTANCE_NEAR * DISTANCE_NEAR;
    float fFarSq = DISTANCE_FAR * DISTANCE_FAR;
    unsigned short usNumVertices = (unsigned short)(uiNumParticles * 4);
    unsigned short usNumIndices = (unsigned short)(uiNumParticles * 6 - 2);
    float fMidSq = (fNearSq + fFarSq) * 0.5f;
    float fInvHalfRangeSq = 2.0f / (fFarSq - fNearSq);

    pcRedIndexBuffer* pIndexBuffer = pcRedIndexBuffer::Create(usNumIndices, false, true);
    pcRedVertexBuffer* pVertexBuffer;
    if (RedRenderer::pcGetGeometryMode())
    {
        pVertexBuffer = pcRedVertexBuffer::Create(usNumVertices, sizeof(TexturedVertex::VertexC), false, true);
        CompressedPos_SetBoundingBox(bbox);
    }
    else
    {
        pVertexBuffer = pcRedVertexBuffer::Create(usNumVertices, sizeof(TexturedVertex::VertexU), false, true);
    }

    unsigned int uiStartVertex, uiStartIndex;
    void* pVertices = pVertexBuffer->Lock(usNumVertices, uiStartVertex);
    unsigned short* pIndices = pIndexBuffer->Lock(usNumIndices, uiStartIndex);

    PblRandom random = { 0xC71C2831u };
    unsigned short usVertex = 0;
    unsigned short usIndex = 0;
    for (unsigned int uiParticle = 0; uiParticle < PARTICLES_PER_BOX; ++uiParticle)
    {
        float fX = random.GetFloat() * BOX_SIZE + fOffsetX;
        float fZ = random.GetFloat() * BOX_SIZE + fOffsetZ;
        float fAngle = random.GetFloat() * 2.0f;
        float fDirection = (random.GetInt() & 0x10) ? -1.0f : 1.0f;
        if (fX > BOX_SIZE)
        {
            fX -= BOX_SIZE;
        }
        if (fZ > BOX_SIZE)
        {
            fZ -= BOX_SIZE;
        }
        fAngle = fDirection * fRotation + fAngle;
        if (fAngle > 1.0f)
        {
            fAngle -= 2.0f;
        }

        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, fAngle * 3.1415927f);

        float fHalfSize = PARTICLE_SIZE * 0.5f;
        float fA = PARTICLE_SIZE * -0.5f * (fSin + fCos);
        float fB = (fCos - fSin) * fHalfSize;
        float fC = (fSin + fCos) * fHalfSize;
        PblVector3 vDiagA(vRight[0] * fA + vUp[0] * fB, vRight[1] * fA + vUp[1] * fB,
                          vRight[2] * fA + vUp[2] * fB);
        PblVector3 vDiagB(vRight[0] * fB + vUp[0] * fC, vRight[1] * fB + vUp[1] * fC,
                          vRight[2] * fB + vUp[2] * fC);

        for (int iX = iMinX; iX <= iMaxX; ++iX)
        {
            float fPosX = (float)iX * BOX_SIZE + fX;
            if (box._vMin.x > fPosX || fPosX >= box._vMax.x)
            {
                continue;
            }
            for (int iZ = iMinZ; iZ <= iMaxZ; ++iZ)
            {
                float fPosZ = (float)iZ * BOX_SIZE + fZ;
                if (box._vMin.z > fPosZ || fPosZ >= box._vMax.z)
                {
                    continue;
                }
                float fPosY = RedTerrain::GetInterpolatedHeight(fPosX, fPosZ) + HEIGHT_ABOVE_GROUND;
                if (box._vMin.y > fPosY || fPosY >= box._vMax.y)
                {
                    continue;
                }

                float fDY = fPosY - vPos[1];
                float fDX = fPosX - vPos[0];
                float fDZ = fPosZ - vPos[2];
                float fFade = ((fDY * fDY + fDX * fDX + fDZ * fDZ) - fMidSq) * fInvHalfRangeSq;
                fFade = fFade * fFade;
                if (fFade >= 1.0f)
                {
                    continue;
                }

                RedColor color;
                color.b = COLOR.b;
                color.g = COLOR.g;
                color.r = COLOR.r;
                color.a = (unsigned char)(int)((1.0f - fFade) * (float)COLOR.a);

                PblVector3 aCorners[4] = {
                    PblVector3(fPosX + vDiagA.x, fPosY + vDiagA.y, fPosZ + vDiagA.z),
                    PblVector3(fPosX + vDiagB.x, fPosY + vDiagB.y, fPosZ + vDiagB.z),
                    PblVector3(fPosX - vDiagA.x, fPosY - vDiagA.y, fPosZ - vDiagA.z),
                    PblVector3(fPosX - vDiagB.x, fPosY - vDiagB.y, fPosZ - vDiagB.z),
                };
                static const float s_afU[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
                static const float s_afV[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
                if (RedRenderer::pcGetGeometryMode())
                {
                    TexturedVertex::VertexC* pVertex = (TexturedVertex::VertexC*)pVertices;
                    for (int i = 0; i < 4; ++i)
                    {
                        pVertex[i].SetPos(aCorners[i]);
                        pVertex[i].SetColor(color);
                        pVertex[i].SetUV(s_afU[i], s_afV[i]);
                    }
                    pVertices = pVertex + 4;
                }
                else
                {
                    TexturedVertex::VertexU* pVertex = (TexturedVertex::VertexU*)pVertices;
                    for (int i = 0; i < 4; ++i)
                    {
                        pVertex[i].SetPos(aCorners[i]);
                        pVertex[i].SetColor(color);
                        pVertex[i].SetUV(s_afU[i], s_afV[i]);
                    }
                    pVertices = pVertex + 4;
                }

                if (usVertex)
                {
                    pIndices[0] = pIndices[-1];
                    pIndices[1] = usVertex;
                    pIndices += 2;
                    usIndex += 2;
                }
                pIndices[0] = usVertex;
                pIndices[1] = usVertex + 1;
                pIndices[2] = usVertex + 3;
                pIndices[3] = usVertex + 2;
                pIndices += 4;
                usIndex += 4;
                usVertex += 4;
            }
        }
    }

    pVertexBuffer->Unlock();
    pIndexBuffer->Unlock();

    if (usIndex > 2)
    {
        pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, usIndex - 2, RedRenderer::pcGetGeometryMode() ? 0x9282 : 0x282,
            usVertex, uiStartVertex, pVertexBuffer, uiStartIndex, pIndexBuffer);
        pPrimitive->m_bbox = bbox;
        RedRenderer::pcRenderPrimitive(pPrimitive,
                                       m_pShader,
                                       gMatrixIdentity,
                                       RedColor::WHITE,
                                       0x01000000, 0, 0, 0);
        pPrimitive->Release();
    }
    pVertexBuffer->Release();
    pIndexBuffer->Release();
}

// FUNCTION: SWBF2 0x00435670
void PrecipitationEffect::PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

inline PblVector3 CameraManager::GetCameraVelocity(int iCam) const
{
    return mCurGameCamera[iCam]->GetVelocity();
}

// FUNCTION: SWBF2 0x00694e20
void PrecipitationEffect::RainShadowRegion::UpdateVisibility(const RedCamera* pCamera)
{
    m_bVisible = RedRegion::IsVisible(pCamera, 0.0f);
}

// FUNCTION: SWBF2 0x004356b0
void PrecipitationEffect::Render(int value)
{
    if ((unsigned int)value & 0x200000)
    {
        return;
    }

    unsigned short usNumParticles = (unsigned short)ceilf(m_fParticleDensity);
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    GeometryModeT eGeometryMode = RedRenderer::pcGetGeometryMode();
    pcRedPrimitive* pPrimitive;

    if (PARTICLE_TYPE == PRECIPITATION_STREAKS)
    {
        PblVector3 vCameraVelocity(0.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); ++i)
        {
            if (RedRenderer::GetCamera() == CameraManager::Instance()->GetCamera(i))
            {
                vCameraVelocity = CameraManager::Instance()->GetCameraVelocity(i);
                if (CAMERA_CROSSVELOCITY_SCALE != 1.0f || CAMERA_AXIALVELOCITY_SCALE != 1.0f)
                {
                    PblVector3 vLocal;
                    D3DXVec3TransformNormal(&vLocal, &vCameraVelocity,
                                            &RedRenderer::GetCamera()->GetMatrixInverse());
                    vCameraVelocity.x = vLocal.x * CAMERA_CROSSVELOCITY_SCALE;
                    vCameraVelocity.y = vLocal.y * CAMERA_CROSSVELOCITY_SCALE;
                    vCameraVelocity.z = CAMERA_AXIALVELOCITY_SCALE * vLocal.z;
                    PblVector3 vWorld;
                    D3DXVec3TransformNormal(&vWorld, &vCameraVelocity, &mCamera);
                    vCameraVelocity = vWorld;
                }
                break;
            }
        }

        const PblVector3& vWind = WindEffect::Instance()->GetVelocity();
        float fScale = -STREAK_TIME;
        PblVector3 vStreak((vWind.x - vCameraVelocity.x) * fScale,
                           fScale * (-VELOCITY - vCameraVelocity.y),
                           fScale * (vWind.z - vCameraVelocity.z));
        float fLengthSq = vStreak.y * vStreak.y + vStreak.x * vStreak.x + vStreak.z * vStreak.z;
        if (fLengthSq > BOX_SIZE * BOX_SIZE)
        {
            float fRatio = BOX_SIZE / sqrtf(fLengthSq);
            vStreak.x = vStreak.x * fRatio;
            vStreak.y = fRatio * vStreak.y;
            vStreak.z = vStreak.z * fRatio;
        }

        float fBoxMin = BOX_SIZE * -0.5f;
        float fPad = (float)fabs(VELOCITY_RANGE * STREAK_TIME);
        float fLo = fBoxMin - fPad;
        float fHi = BOX_SIZE * 0.5f + fPad;
        PblBox box;
        box._vMin = PblVector3(fLo, fLo, fLo);
        box._vMax = PblVector3(fHi, fHi, fHi);
        box.AddPoint(PblVector3(box._vMin.x + vStreak.x, box._vMin.y + vStreak.y, box._vMin.z + vStreak.z));
        box.AddPoint(PblVector3(box._vMax.x + vStreak.x, box._vMax.y + vStreak.y, box._vMax.z + vStreak.z));

        unsigned short usNumVertices = (unsigned short)(usNumParticles * 2);
        if (eGeometryMode)
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_LINELIST, 0x1082, sizeof(UntexturedVertex::VertexC),
                                                usNumVertices, 0, false);
            CompressedPos_SetBoundingBox(box);
        }
        else
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_LINELIST, 0x82, sizeof(UntexturedVertex::VertexU),
                                                usNumVertices, 0, false);
        }
        pPrimitive->m_bbox = box;

        void* pVertices = pPrimitive->BeginVertexData();
        if (!pVertices)
        {
            return;
        }

        if (RedRenderer::pcGetGeometryMode())
        {
            UntexturedVertex::VertexC* pVertex = (UntexturedVertex::VertexC*)pVertices;
            for (unsigned short i = 0; i < usNumParticles; ++i, pVertex += 2)
            {
                const PrecipitationData& data = m_pData[i];
                PblVector3 vHead(fBoxMin + data.vPosRelative.x * BOX_SIZE,
                                 data.vPosRelative.y * BOX_SIZE + fBoxMin,
                                 fBoxMin + data.vPosRelative.z * BOX_SIZE);
                PblVector3 vTail((vStreak.x + data.vVelocityLocal.x * -STREAK_TIME) + vHead.x,
                                 (data.vVelocityLocal.y * -STREAK_TIME + vStreak.y) + vHead.y,
                                 (data.vVelocityLocal.z * -STREAK_TIME + vStreak.z) + vHead.z);
                RedColor colorHead = COLOR;
                colorHead.a = (unsigned char)(int)(data.fAlpha * 255.0f);
                RedColor colorTail = COLOR;
                colorTail.a = 0;
                pVertex[0].SetPos(vHead);
                pVertex[0].SetColor(colorHead);
                pVertex[1].SetPos(vTail);
                pVertex[1].SetColor(colorTail);
            }
        }
        else
        {
            UntexturedVertex::VertexU* pVertex = (UntexturedVertex::VertexU*)pVertices;
            for (unsigned short i = 0; i < usNumParticles; ++i, pVertex += 2)
            {
                const PrecipitationData& data = m_pData[i];
                PblVector3 vHead(fBoxMin + data.vPosRelative.x * BOX_SIZE,
                                 data.vPosRelative.y * BOX_SIZE + fBoxMin,
                                 fBoxMin + data.vPosRelative.z * BOX_SIZE);
                PblVector3 vTail((vStreak.x + data.vVelocityLocal.x * -STREAK_TIME) + vHead.x,
                                 (data.vVelocityLocal.y * -STREAK_TIME + vStreak.y) + vHead.y,
                                 (data.vVelocityLocal.z * -STREAK_TIME + vStreak.z) + vHead.z);
                RedColor colorHead = COLOR;
                colorHead.a = (unsigned char)(int)(data.fAlpha * 255.0f);
                RedColor colorTail = COLOR;
                colorTail.a = 0;
                pVertex[0].SetPos(vHead);
                pVertex[0].SetColor(colorHead);
                pVertex[1].SetPos(vTail);
                pVertex[1].SetColor(colorTail);
            }
        }
        pPrimitive->EndVertexData();
    }
    else
    {
        float fBoxMin = BOX_SIZE * -0.5f;
        float fPad = PARTICLE_SIZE * 1.414214f;
        float fLo = fBoxMin - fPad;
        float fHi = BOX_SIZE * 0.5f + fPad;
        PblBox box;
        box._vMin = PblVector3(fLo, fLo, fLo);
        box._vMax = PblVector3(fHi, fHi, fHi);

        unsigned short usNumVertices = (unsigned short)(usNumParticles * 4);
        unsigned short usNumIndices = (unsigned short)(usNumParticles * 6 - 2);
        if (eGeometryMode)
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x9282, sizeof(TexturedVertex::VertexC),
                                                usNumVertices, usNumIndices, false);
            CompressedPos::SetBoundingBox(&box);
        }
        else
        {
            pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, sizeof(TexturedVertex::VertexU),
                                                usNumVertices, usNumIndices, false);
        }
        pPrimitive->m_bbox = box;

        void* pVertices = pPrimitive->BeginVertexData();
        if (!pVertices)
        {
            return;
        }
        unsigned short* pIndices = pPrimitive->BeginIndexData();

        const float* vRight = mCamera.m[0];
        const float* vUp = mCamera.m[1];
        unsigned short usVertex = 0;
        for (unsigned short i = 0; i < usNumParticles; ++i)
        {
            const PrecipitationData& data = m_pData[i];
            PblVector3 vCenter(data.vPosRelative.x * BOX_SIZE + fBoxMin,
                               data.vPosRelative.y * BOX_SIZE + fBoxMin,
                               data.vPosRelative.z * BOX_SIZE + fBoxMin);
            float fSin = 0.0f;
            float fCos = 1.0f;
            PblMath::SinCos(&fSin, &fCos, data.fRotation * 6.2831855f);

            float fA = (fCos + fSin) * PARTICLE_SIZE;
            float fB = (fSin - fCos) * PARTICLE_SIZE;
            float fC = (fCos - fSin) * PARTICLE_SIZE;
            PblVector3 vDiagA(fB * vRight[0] + fA * vUp[0], fB * vRight[1] + fA * vUp[1],
                              fB * vRight[2] + fA * vUp[2]);
            PblVector3 vDiagB(fA * vRight[0] + vUp[0] * fC, fA * vRight[1] + vUp[1] * fC,
                              fA * vRight[2] + vUp[2] * fC);

            RedColor color = COLOR;
            color.a = (unsigned char)(int)(data.fAlpha * 255.0f);

            PblVector3 aCorners[4] = {
                PblVector3(vCenter.x - vDiagA.x, vCenter.y - vDiagA.y, vCenter.z - vDiagA.z),
                PblVector3(vCenter.x - vDiagB.x, vCenter.y - vDiagB.y, vCenter.z - vDiagB.z),
                PblVector3(vDiagA.x + vCenter.x, vDiagA.y + vCenter.y, vDiagA.z + vCenter.z),
                PblVector3(vDiagB.x + vCenter.x, vDiagB.y + vCenter.y, vDiagB.z + vCenter.z),
            };
            static const float s_afU[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            static const float s_afV[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
            if (RedRenderer::pcGetGeometryMode())
            {
                TexturedVertex::VertexC* pVertex = (TexturedVertex::VertexC*)pVertices;
                for (int j = 0; j < 4; ++j)
                {
                    pVertex[j].SetPos(aCorners[j]);
                    pVertex[j].SetColor(color);
                    pVertex[j].SetUV(s_afU[j], s_afV[j]);
                }
                pVertices = pVertex + 4;
            }
            else
            {
                TexturedVertex::VertexU* pVertex = (TexturedVertex::VertexU*)pVertices;
                for (int j = 0; j < 4; ++j)
                {
                    pVertex[j].SetPos(aCorners[j]);
                    pVertex[j].SetColor(color);
                    pVertex[j].SetUV(s_afU[j], s_afV[j]);
                }
                pVertices = pVertex + 4;
            }

            if (i)
            {
                pIndices[0] = pIndices[-1];
                pIndices[1] = usVertex;
                pIndices += 2;
            }
            pIndices[0] = usVertex;
            pIndices[1] = usVertex + 1;
            pIndices[2] = usVertex + 3;
            pIndices[3] = usVertex + 2;
            pIndices += 4;
            usVertex += 4;
        }
        pPrimitive->EndVertexData();
        pPrimitive->EndIndexData();
    }

    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    float fZoom = RedRenderer::GetCamera()->GetZoom();
    RedCamera camera(*RedRenderer::GetCamera());

    float fCameraDepth = -(mCamera.m[3][1] * mCamera.m[2][1] + mCamera.m[2][0] * mCamera.m[3][0]
                           + mCamera.m[3][2] * mCamera.m[2][2]);
    float fRenderDistance = (fZoom - 1.0f) * fNear + RENDER_DISTANCE;
    float fCullDistance = fRenderDistance;
    camera.SetNearFarPlane(fZoom * fNear, fRenderDistance);

    PblList<RainShadowRegion>::Itor itVisibleBegin = m_RainShadows.End();
    PblList<RainShadowRegion>::Itor itVisibleEnd = m_RainShadows.End();
    for (PblList<RainShadowRegion>::Itor it = m_RainShadows.Begin(); it != m_RainShadows.End(); ++it)
    {
        it->UpdateVisibility(&camera);
        if (it->IsVisible())
        {
            itVisibleEnd = it;
            ++itVisibleEnd;
            if (itVisibleBegin == m_RainShadows.End())
            {
                itVisibleBegin = it;
            }
        }
    }
    if (itVisibleBegin != itVisibleEnd)
    {
        fRenderDistance = fRenderDistance * 3.0f;
        camera.SetNearFarPlane(fZoom * fNear, fRenderDistance);
    }

    float fFadeStart = fRenderDistance * 0.66f;
    float fFadeScale = 1.0f / (fRenderDistance - fFadeStart);
    RedColor color = COLOR;

    const PblBox& cameraBox = camera.GetCameraBox();
    int iStartX = (int)floorf(cameraBox._vMin.x * BOX_SIZEINV);
    int iEndX = (int)floorf(cameraBox._vMax.x * BOX_SIZEINV);
    int iStartY = (int)floorf(cameraBox._vMin.y * BOX_SIZEINV);
    int iEndY = (int)floorf(cameraBox._vMax.y * BOX_SIZEINV);
    int iStartZ = (int)floorf(cameraBox._vMin.z * BOX_SIZEINV);
    int iEndZ = (int)floorf(cameraBox._vMax.z * BOX_SIZEINV);

    int iStepX = 1;
    int iStepY = 1;
    int iStepZ = 1;
    int iSwap;
    if (mCamera.m[2][0] > 0.0f)
    {
        iSwap = iStartX;
        iStartX = iEndX;
        iEndX = iSwap;
        iStepX = -1;
    }
    iEndX += iStepX;
    if (mCamera.m[2][1] > 0.0f)
    {
        iSwap = iStartY;
        iStartY = iEndY;
        iEndY = iSwap;
        iStepY = -1;
    }
    iEndY += iStepY;
    if (mCamera.m[2][2] > 0.0f)
    {
        iSwap = iStartZ;
        iStartZ = iEndZ;
        iEndZ = iSwap;
        iStepZ = -1;
    }
    iEndZ += iStepZ;

    PblMatrix mWorld = gMatrixIdentity;
    const PblVector3& vCellCenter = *(const PblVector3*)mWorld.m[3];
    float fHalfBox = BOX_SIZE * 0.5f;
    PblVector3 vExtents(fHalfBox, fHalfBox, fHalfBox);

    if (itVisibleBegin == itVisibleEnd)
    {
        for (int iY = iStartY; iY != iEndY; iY += iStepY)
        {
            for (int iX = iStartX; iX != iEndX; iX += iStepX)
            {
                for (int iZ = iStartZ; iZ != iEndZ; iZ += iStepZ)
                {
                    mWorld.m[3][0] = ((float)iX + 0.5f) * BOX_SIZE;
                    mWorld.m[3][1] = ((float)iY + 0.5f) * BOX_SIZE;
                    mWorld.m[3][2] = ((float)iZ + 0.5f) * BOX_SIZE;
                    mWorld.m[3][3] = 1.0f;
                    float fDepth = -(mWorld.m[3][1] * mCamera.m[2][1] + mCamera.m[2][0] * mWorld.m[3][0]
                                     + mCamera.m[2][2] * mWorld.m[3][2]) - fCameraDepth;
                    if (fDepth < fCullDistance
                        && camera.ClassifyAABox(vCellCenter, vExtents) != RedCamera::VISIBILITY_NONE)
                    {
                        if (fFadeStart < fDepth)
                        {
                            color.a = (unsigned char)(int)((fRenderDistance - fDepth) * (float)COLOR.a * fFadeScale);
                        }
                        else
                        {
                            color.a = COLOR.a;
                        }
                        RedRenderer::pcRenderPrimitive(pPrimitive,
                                                       m_pShader,
                                                       mWorld,
                                                       color, 0, 0, 0, 0);
                    }
                }
            }
        }
    }
    else
    {

        struct DistanceData
        {
            float fDistance;
            bool bOccluding;
        };

        int iNumZ = (iEndZ - iStartZ) * iStepZ;
        DistanceData* pDistances =
            (DistanceData*)RedAllocFromHeap(__RedCurrHeap, iNumZ * 2 * sizeof(DistanceData), 0);
        float fStartDepth = -(mCamera.m[2][1] * (((float)iStartY + 0.5f) * BOX_SIZE)
                              + ((float)iStartX + 0.5f) * BOX_SIZE * mCamera.m[2][0]
                              + mCamera.m[2][2] * (((float)iStartZ + 0.5f) * BOX_SIZE))
                            - fCameraDepth;
        float fStepX = (float)fabs(mCamera.m[2][0]) * BOX_SIZE;
        float fStepY = (float)fabs(mCamera.m[2][1]) * BOX_SIZE;
        float fStepZ = (float)fabs(mCamera.m[2][2]) * BOX_SIZE;

        for (int iY = iStartY; iY != iEndY; iY += iStepY)
        {
            float fRowDepth = (float)((iY - iStartY) * iStepY) * fStepY + fStartDepth;
            DistanceData* pPrev = pDistances;
            DistanceData* pCur = pDistances + iNumZ;
            for (int iZ = iStartZ; iZ != iEndZ; iZ += iStepZ)
            {
                int k = (iZ - iStartZ) * iStepZ;
                pPrev[k].bOccluding = true;
                pPrev[k].fDistance = (float)k * fStepZ * 3.0f + fRowDepth - fStepX * 3.0f;
            }

            for (int iX = iStartX; iX != iEndX; iX += iStepX)
            {
                bool bPrevOccluding = true;
                bool bAllBeyond = true;
                float fDistance = (float)((iX - iStartX) * iStepX) * fStepX * 3.0f + fRowDepth - fStepZ * 3.0f;
                for (int iZ = iStartZ; iZ != iEndZ; iZ += iStepZ)
                {
                    int k = (iZ - iStartZ) * iStepZ;
                    float fCell;
                    if (bPrevOccluding)
                    {
                        if (pPrev[k].bOccluding)
                        {
                            fCell = fDistance + fStepZ * 3.0f;
                            if (fStepX * 3.0f + pPrev[k].fDistance > fCell)
                            {
                                fCell = fStepX * 3.0f + pPrev[k].fDistance;
                            }
                        }
                        else
                        {
                            fCell = pPrev[k].fDistance + fStepX;
                        }
                    }
                    else
                    {
                        fCell = fDistance + fStepZ;
                        if (!pPrev[k].bOccluding && fCell > pPrev[k].fDistance + fStepX)
                        {
                            fCell = pPrev[k].fDistance + fStepX;
                        }
                    }
                    pCur[k].bOccluding = true;
                    pCur[k].fDistance = fCell;

                    if (fRenderDistance > fCell)
                    {
                        bAllBeyond = false;
                        mWorld.m[3][0] = ((float)iX + 0.5f) * BOX_SIZE;
                        mWorld.m[3][1] = ((float)iY + 0.5f) * BOX_SIZE;
                        mWorld.m[3][2] = ((float)iZ + 0.5f) * BOX_SIZE;
                        mWorld.m[3][3] = 1.0f;
                        if (camera.ClassifyAABox(vCellCenter, vExtents) != RedCamera::VISIBILITY_NONE)
                        {
                            PblList<RainShadowRegion>::Itor it = itVisibleBegin;
                            for (; it != itVisibleEnd; ++it)
                            {
                                if (it->IsVisible() && it->DoesAABoxIntersect(vCellCenter, vExtents))
                                {
                                    break;
                                }
                            }
                            if (it == itVisibleEnd)
                            {
                                if (fFadeStart < fCell)
                                {
                                    color.a = (unsigned char)(int)((fRenderDistance - fCell) * (float)COLOR.a * fFadeScale);
                                }
                                else
                                {
                                    color.a = COLOR.a;
                                }
                                RedRenderer::pcRenderPrimitive(pPrimitive,
                                                               m_pShader,
                                                               mWorld,
                                                               color, 0, 0, 0, 0);
                            }
                            else
                            {
                                pCur[k].bOccluding = false;
                            }
                        }
                    }
                    fDistance = pCur[k].fDistance;
                    bPrevOccluding = pCur[k].bOccluding;
                }
                if (bAllBeyond)
                {
                    break;
                }
                DistanceData* pSwap = pPrev;
                pPrev = pCur;
                pCur = pSwap;
            }
        }
        RedFreeToHeap(__RedCurrHeap, pDistances, 0);
    }

    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00435590
void PrecipitationEffect::PlatformInit()
{
    if (m_pShader)
    {
        m_pShader->Release();
    }

    if (PARTICLE_TYPE == PRECIPITATION_STREAKS)
    {
        m_pShader = pcRedShader::Create(PblHash("Rain"),
                                        RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ? 0x1082 : 0x82,
                                        0, 0, 0x10004, RedColor::WHITE, RedColor::BLACK, 32.0f);
        ((pcRainShader*)m_pShader)->SetLineWidth(PARTICLE_SIZE);
        ((pcRainShader*)m_pShader)->SetMaxPixelWidth(1.0f);
    }
    else
    {
        m_pShader = pcRedShader::Create(PblHash("Particle"),
                                        RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ? 0x9282 : 0x282,
                                        &TEXTURE_HASH, 1, 0x10004, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
}
