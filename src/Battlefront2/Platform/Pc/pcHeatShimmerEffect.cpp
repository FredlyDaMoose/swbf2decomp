#include "Battlefront2/Source/HeatShimmerEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcRefractionShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedTextureTransform.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <math.h>

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3*, const PblVector3*);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix*, float);

// GLOBAL: SWBF2 0x01e2fd18
float HeatShimmerEffect::s_tileU;
// GLOBAL: SWBF2 0x01e2fd1c
unsigned int HeatShimmerEffect::s_bumpMapHash;

// GLOBAL: SWBF2 0x01e2fd20
pcRedPrimitive* HeatShimmerEffect::s_pPrimitive;

// GLOBAL: SWBF2 0x01e2fd24
float HeatShimmerEffect::s_tileV;

// GLOBAL: SWBF2 0x01e2fd28
pcRefractionShader* HeatShimmerEffect::s_pShimmerShader;

// FUNCTION: SWBF2 0x00419a60
void HeatShimmerEffect::PlatformPreReadInit()
{
    s_pShimmerShader = (pcRefractionShader*)pcRedShader::Create(
        PblHash("Refraction"), 0x282, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
    s_pPrimitive = 0;
    s_tileU = 1.0f;
    s_tileV = 1.0f;
    s_distortionScale = 0.003f;
}

// FUNCTION: SWBF2 0x00419ac0
void HeatShimmerEffect::PlatformCleanup()
{
    if (s_pShimmerShader)
    {
        s_pShimmerShader->Release();
        s_pShimmerShader = 0;
    }
    if (s_pPrimitive)
    {
        s_pPrimitive->Release();
        s_pPrimitive = 0;
    }
}

// FUNCTION: SWBF2 0x00419b40
void HeatShimmerEffect::PlatformPostReadInit()
{
    const unsigned int uX = s_tessellationX;
    const unsigned int uY = s_tessellationY;
    const unsigned int uNumVertices = (uX + 1) * (uY + 1);
    const unsigned int uNumIndices = 2 * uX * (uY + 2) - 2;
    if (s_pPrimitive)
    {
        s_pPrimitive->Release();
    }
    s_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, 24,
                                           uNumVertices, uNumIndices, true);

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const float fHalfWidth = (float)tan((double)(pCamera->GetFOV() * 0.5f)) * 200.0f;
    const float fXStep = (fHalfWidth * 2.0f) / (float)uX;
    float fY = s_posY + s_height;
    const float fYStep = -((fY - s_height) / (float)uY);
    struct Vertex { float x, y, z; RedColor color; float u, v; };
    Vertex* pVertices = (Vertex*)s_pPrimitive->BeginVertexData();
    if (pVertices)
    {
        for (unsigned int y = 0; y <= uY; ++y)
        {
            float fFade = 1.0f - (float)fabs(((float)y / (float)uY) * 2.0f - 1.0f);
            unsigned char ucAlpha = (unsigned char)(int)(fFade * 255.0f);
            float fX = -fHalfWidth;
            for (unsigned int x = 0; x <= uX; ++x)
            {
                Vertex& v = pVertices[y * (uX + 1) + x];
                v.x = fX;
                v.y = fY;
                v.z = 0.0f;
                v.color = RedColor::WHITE;
                v.color.a = ucAlpha;
                v.u = (float)x / (float)uX;
                v.v = (float)y / (float)uY;
                fX += fXStep;
            }
            fY += fYStep;
        }
        s_pPrimitive->EndVertexData();
        unsigned short* pIndices = s_pPrimitive->BeginIndexData();
        unsigned int n = 0;
        for (unsigned int y = 0; y < uY; ++y)
        {
            for (unsigned int x = 0; x <= uX; ++x)
            {
                pIndices[n++] = (unsigned short)(y * (uX + 1) + x);
                pIndices[n++] = (unsigned short)((y + 1) * (uX + 1) + x);
            }
            if (y + 1 < uY)
            {
                pIndices[n] = pIndices[n - 1];
                ++n;
                pIndices[n++] = (unsigned short)((y + 1) * (uX + 1));
            }
        }
        s_pPrimitive->EndIndexData();
        s_pShimmerShader->SetDistortionScale(s_distortionScale);
    }
}

// FUNCTION: SWBF2 0x00419eb0
void HeatShimmerEffect::PlatformReadConfigData(PblConfig::Data& data)
{
    if (data.GetId() != 0xa9acf4e1u)
    {
        return;
    }
    s_bumpMapHash = PblHash(data.GetStringArg(0));
    s_tileU = data.GetNumArgs() > 1 ? data.GetFloatArg(1) : 1.0f;
    s_tileV = data.GetNumArgs() > 2 ? data.GetFloatArg(2) : 1.0f;
}

// FUNCTION: SWBF2 0x00419f10
void HeatShimmerEffect::PlatformRender(unsigned int) const
{
    if (!RedRenderer::pcGetRenderOptions().distortion)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblMatrix& cameraMatrix = pCamera->GetMatrix();
    float fAngle;
    if (cameraMatrix.m[0][0] == 0.0f && cameraMatrix.m[0][1] == 1.0f && cameraMatrix.m[0][2] == 0.0f)
    {
        fAngle = PblMath::Atan2(cameraMatrix.m[1][2], cameraMatrix.m[1][0]);
    }
    else
    {
        fAngle = PblMath::Atan2(cameraMatrix.m[0][2], cameraMatrix.m[0][0]);
    }

    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    PblVector3 vForward(cameraMatrix.m[2][0], 0.0f, cameraMatrix.m[2][2]);
    PblVector3 vNormalized;
    D3DXVec3Normalize(&vNormalized, &vForward);
    PblMatrix world(PblVector4{1,0,0,0}, PblVector4{0,1,0,0},
                    PblVector4{0,0,1,0}, PblVector4{0,0,0,1});
    D3DXMatrixRotationY(&world, -fAngle);
    world.m[3][0] = cameraMatrix.m[3][0] - vNormalized.x * (fFar - 10.0f);
    world.m[3][1] = 0.0f;
    world.m[3][2] = cameraMatrix.m[3][2] - vNormalized.z * (fFar - 10.0f);
    world.m[3][3] = 1.0f;

    RedTextureTransform transform;
    transform.m_bTransform = 1;
    transform.m_Transform[0][0] = s_tileU;
    transform.m_Transform[0][1] = 0.0f;
    transform.m_Transform[1][0] = 0.0f;
    transform.m_Transform[1][1] = s_tileV;
    transform.m_Transform[2][0] = s_tileU * 0.0f;
    float fScroll = s_shimmerTimeStamp.GetElapsed() * s_scrollSpeed;
    transform.m_Transform[2][1] = (fScroll - (float)(int)fScroll) * s_tileV;
    s_pShimmerShader->SetBumpMap(s_bumpMapHash, &transform);
    s_pShimmerShader->SetRefractionMap(RedRenderer::pcGetBuffer(PC_BUFFER_SKY));
    s_pShimmerShader->SetDistortionScale(s_distortionScale / pCamera->_fZoom);
    RedColor color = RedColor::WHITE;
    RedRenderer::pcRenderPrimitive(s_pPrimitive,
                                   s_pShimmerShader,
                                   world,
                                   color, 0x210, 0, 0, 0);
}
