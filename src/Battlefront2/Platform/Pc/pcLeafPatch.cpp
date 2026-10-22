#include "pcLeafPatch.h"
#include "pcDynamicSprite.h"

#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include <new>
#include "Battlefront2/Source/BirdEffect.h"

// FUNCTION: SWBF2 0x0041a740
LeafPatchRenderable::LeafPatchRenderable(const LeafPatchClass* pClass)
    : Renderable(2), m_fDistanceFarShader(pClass->mNearFadeDist),
      m_pPrimitive(0), m_pPrimitiveFar(0), m_pClass(pClass), m_pVertices(0)
{
    const unsigned int vertexFormat = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED
        ? 0x2a2 : 0xd2a2;
    m_pShader = pcRedShader::Create(0xe68b9c52, vertexFormat, &pClass->mTextureHash,
                                    1, 0x10002, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pShaderFar = pcRedShader::Create(0xe68b9c52, vertexFormat, &pClass->mTextureHash,
                                       1, 0x10002, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_bReflection = false;
    const int count = pClass->mNumParticles;
    m_pIBReverse = pcRedIndexBuffer::Create(6 * count, true, true);
    unsigned short* indices = m_pIBReverse->Lock(6 * count, m_startIndexReverse);
    for (int i = count - 1, j = 0; i >= 0; --i, j += 6)
    {
        const int base = i * 4;
        indices[j] = (unsigned short)base;
        indices[j + 1] = (unsigned short)(base + 1);
        indices[j + 2] = (unsigned short)(base + 2);
        indices[j + 3] = (unsigned short)base;
        indices[j + 4] = (unsigned short)(base + 2);
        indices[j + 5] = (unsigned short)(base + 3);
    }
    m_pIBReverse->Unlock();
    m_pIBForward = pcRedIndexBuffer::Create(6 * count, true, true);
    indices = m_pIBForward->Lock(6 * count, m_startIndexForward);
    for (int i = 0, j = 0; i < count; ++i, j += 6)
    {
        const int base = i * 4;
        indices[j] = (unsigned short)base;
        indices[j + 1] = (unsigned short)(base + 1);
        indices[j + 2] = (unsigned short)(base + 2);
        indices[j + 3] = (unsigned short)base;
        indices[j + 4] = (unsigned short)(base + 2);
        indices[j + 5] = (unsigned short)(base + 3);
    }
    m_pIBForward->Unlock();
    m_pVB = 0;
    mHeap = new PblHeap<int, float>(count, 1000000.0f, 0);
    m_iTexCoordType = pClass->mNumParts == 4 ? 1 : 0;
    Activate(0);
}

// FUNCTION: SWBF2 0x0041a9f0
LeafPatchRenderable::~LeafPatchRenderable()
{
    Deactivate();
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }
    if (m_pPrimitiveFar)
    {
        m_pPrimitiveFar->Release();
    }
    if (m_pIBForward)
    {
        m_pIBForward->Release();
    }
    if (m_pIBReverse)
    {
        m_pIBReverse->Release();
    }
    if (m_pVB)
    {
        m_pVB->Release();
    }
    if (m_pShader)
    {
        m_pShader->Release();
    }
    if (m_pShaderFar)
    {
        m_pShaderFar->Release();
    }
    delete mHeap;
}

// FUNCTION: SWBF2 0x0041ba90
void LeafPatchClass::Render(float fAlpha, const PblMatrix& matrix, unsigned int uiFlags,
                            const RedColorValue& color) const
{
    RedColor packed;
    packed.b = (unsigned char)(int)(color.b * 255.0f);
    packed.g = (unsigned char)(int)(color.g * 255.0f);
    packed.r = (unsigned char)(int)(color.r * 255.0f);
    packed.a = (unsigned char)(int)(color.a * fAlpha * 255.0f);
    mRenderData->RenderInstance(matrix, packed, uiFlags);
}

// FUNCTION: SWBF2 0x0041bb00
void LeafPatchClass::CreateRenderData()
{
    pcDynamicSprite::Init();
    mRenderData = new LeafPatchRenderable(this);
}

// FUNCTION: SWBF2 0x0041b930
bool LeafPatchClass::RenderMaxDist(const PblMatrix& matrix, float& fDist,
                                   unsigned int uiFlags, float fMaxDist,
                                   const RedColorValue& color) const
{
    const PblMatrix& cam = RedRenderer::GetCamera()->_Matrix;
    const float distance = cam.m[2][0] * (cam.m[3][0] - matrix.m[3][0]) +
                           cam.m[2][1] * (cam.m[3][1] - matrix.m[3][1]) +
                           cam.m[2][2] * (cam.m[3][2] - matrix.m[3][2]);
    const float limit = RedRenderer::GetCamera()->_fZoom * fMaxDist;
    if (distance < -mBoundingRadius || distance > limit + mBoundingRadius)
    {
        return false;
    }
    if (mMaxScatterBirds && !(uiFlags & 0x200000))
    {
        BirdParticleManager::SetPotentialScatterPoint(&matrix.GetTranslation(), mMaxScatterBirds);
    }
    PblMatrix scaled = matrix;
    scaled.m[1][0] *= mHeightScale;
    scaled.m[1][1] *= mHeightScale;
    scaled.m[1][2] *= mHeightScale;
    scaled.m[1][3] = 0.0f;
    Render(1.0f, scaled, uiFlags, color);
    return true;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// GLOBAL: SWBF2 0x007eb710
static PblBox sBox(-50.0f, -50.0f, -50.0f, 50.0f, 50.0f, 50.0f);

namespace
{

struct LeafVertexCompressed
{
    short pos[3];
    short pad;
    unsigned char normal[3];
    unsigned char padNormal;
    RedColor color;
    LeafPatchRenderable::CompressedTexCoord uv;
};
struct LeafVertexUncompressed
{
    float pos[3];
    float normal[3];
    RedColor color;
    LeafPatchRenderable::TexCoord uv;
};
}

// FUNCTION: SWBF2 0x0041abb0
void LeafPatchRenderable::Render(int value)
{
    m_bReflection = ((unsigned int)value & 0x200000) != 0;
    Update();
}

// FUNCTION: SWBF2 0x0041abd0
void LeafPatchRenderable::RenderInstance(const PblMatrix& matrix, const RedColor& color,
                                         unsigned int uiFlags) const
{
    if (m_bReflection != ((uiFlags & 0x200000) != 0))
    {
        return;
    }

    PblSphere sphere;
    sphere.x = matrix.m[3][0];
    sphere.y = matrix.m[3][1];
    sphere.z = matrix.m[3][2];
    sphere.w = m_pClass->mBoundingRadius;
    RedRenderer::SetLightingState(*RedLight::GetLightingState(sphere, 4, false, false, false));

    RedRenderer::GetCamera();
    RedRenderer::GetCamera();

    if (color.a == 0xff)
    {
        if (m_pPrimitive)
        {
            RedRenderer::pcRenderPrimitive(m_pPrimitive, m_pShader,
                                           matrix, color,
                                           uiFlags, 0, 0, 0);
        }
    }
    else
    {
        if (m_pPrimitiveFar)
        {
            RedRenderer::pcRenderPrimitive(m_pPrimitiveFar,
                                           m_pShaderFar, matrix,
                                           color, uiFlags, 0, 0, 0);
        }
    }
}

// FUNCTION: SWBF2 0x0041aca0
void LeafPatchRenderable::Update()
{
    const RedCamera* pCamera = RedRenderer::GetCamera();
    const float* vForward = pCamera->_Matrix.m[2];

    for (int i = 0; i < m_pClass->mNumParticles; ++i)
    {
        const PblVector3& vPos = m_pClass->mParticle[i].mPos;
        float fKey = -(vPos.x * vForward[0] + vPos.y * vForward[1] + vPos.z * vForward[2]);
        mHeap->Push(i, fKey);
    }

    Begin();

    while (!mHeap->IsEmpty())
    {
        int iIndex = mHeap->Get(0);
        const LeafPatchClass::LeafParticle& particle = m_pClass->mParticle[iIndex];

        RedColor color;
        color.b = (unsigned char)(int)(particle.mColor.b * 255.0f);
        color.g = (unsigned char)(int)(particle.mColor.g * 255.0f);
        color.r = (unsigned char)(int)(particle.mColor.r * 255.0f);
        color.a = (unsigned char)(int)(particle.mColor.a * 255.0f);

        AddParticle(particle.mPos, particle.mSize, color, particle.mRotation, iIndex, particle.mTexture);
        mHeap->Pop();
    }

    m_pVertices = 0;
    m_pVB->Unlock();
    m_pVB->Release();
    m_pVB = 0;
}

// FUNCTION: SWBF2 0x0041ae90
void LeafPatchRenderable::Begin()
{
    unsigned int uiNumParticles = (unsigned int)m_pClass->mNumParticles;
    unsigned int uiPrimCount = 2 * uiNumParticles;
    unsigned int uiNumVertices = 4 * uiNumParticles;

    unsigned int uiVertexFormat;
    unsigned int uiVertexStride;
    float fScale;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        uiVertexFormat = 0xD2A2;
        uiVertexStride = 20;
        fScale = 655.34998f;
    }
    else
    {
        uiVertexFormat = 0x2A2;
        uiVertexStride = 36;
        fScale = 1.0f;
    }

    m_pVB = pcRedVertexBuffer::Create(uiNumVertices, uiVertexStride, false, true);
    unsigned int uiStartVertex = 0;
    m_pVertices = m_pVB->Lock(uiNumVertices, uiStartVertex);

    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
        m_pPrimitive = 0;
    }
    if (m_pPrimitiveFar)
    {
        m_pPrimitiveFar->Release();
        m_pPrimitiveFar = 0;
    }

    m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, uiPrimCount, uiVertexFormat, uiNumVertices,
                                          uiStartVertex, m_pVB, m_startIndexForward, m_pIBForward);
    m_pPrimitive->m_bbox = sBox;
    m_pPrimitiveFar = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, uiPrimCount, uiVertexFormat, uiNumVertices,
                                             uiStartVertex, m_pVB, m_startIndexReverse, m_pIBReverse);
    m_pPrimitiveFar->m_bbox = sBox;

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblMatrix& mat = pCamera->_Matrix;
    m_vUp = PblVector3(mat.m[1][0] * fScale, mat.m[1][1] * fScale, mat.m[1][2] * fScale);
    m_vFront = PblVector3(mat.m[2][0] * fScale, mat.m[2][1] * fScale, mat.m[2][2] * fScale);

    for (int i = 0; i < 10; ++i)
    {

        float f = (float)(i % 5) * (1.0f - (float)(i & 1) * 2.0f) * 0.3f;
        PblVector3 vDir(mat.m[0][0] + f * mat.m[2][0], mat.m[0][1] + f * mat.m[2][1],
                        mat.m[0][2] + f * mat.m[2][2]);
        D3DXVec3Normalize(&vDir, &vDir);
        m_avRight[i] = PblVector3(vDir.x * fScale, vDir.y * fScale, vDir.z * fScale);
    }
}

// FUNCTION: SWBF2 0x0041b140
void LeafPatchRenderable::AddParticle(const PblVector3& vPos, float fSize, const RedColor& color,
                                      float fRotation, int iParticleIndex, unsigned char ucTexture)
{
    bool bCompressed = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED;
    float fScale = bCompressed ? 655.34998f : 1.0f;

    const PblVector3& vRight = m_avRight[iParticleIndex % 10];
    float upX = m_vUp.x * fSize, upY = m_vUp.y * fSize, upZ = m_vUp.z * fSize;
    float rightX = vRight.x * fSize, rightY = vRight.y * fSize, rightZ = vRight.z * fSize;

    float biasX = vPos.x * fScale + m_vFront.x * 0.1f;
    float biasY = vPos.y * fScale + m_vFront.y * 0.1f;
    float biasZ = vPos.z * fScale + m_vFront.z * 0.1f;

    float fShift = fRotation - 0.5f;
    PblVector3 cornerTop(biasX + upX * 0.5f + rightX * fShift, biasY + upY * 0.5f + rightY * fShift,
                         biasZ + upZ * 0.5f + rightZ * fShift);
    PblVector3 cornerBottom(biasX - upX * 0.5f - rightX * 0.5f, biasY - upY * 0.5f - rightY * 0.5f,
                            biasZ - upZ * 0.5f - rightZ * 0.5f);

    PblVector3 corners[4] = {
        cornerTop,
        PblVector3(cornerTop.x + rightX, cornerTop.y + rightY, cornerTop.z + rightZ),
        PblVector3(cornerBottom.x + rightX, cornerBottom.y + rightY, cornerBottom.z + rightZ),
        cornerBottom,
    };

    static const int kCornerOrder[4] = { 2, 3, 1, 0 };

    const CompressedTexCoord* pCompressedUV = s_CompressedTexCoords[m_iTexCoordType][ucTexture];
    const TexCoord* pUV = s_TexCoords[m_iTexCoordType][ucTexture];

    if (bCompressed)
    {
        float fInvScale = 1.0f / fScale;
        LeafVertexCompressed* pv = (LeafVertexCompressed*)m_pVertices;
        for (int i = 0; i < 4; ++i)
        {
            pv->pos[0] = (short)corners[i].x;
            pv->pos[1] = (short)corners[i].y;
            pv->pos[2] = (short)corners[i].z;

            PblVector3 vNormal(corners[i].x * fInvScale, corners[i].y * fInvScale,
                               corners[i].z * fInvScale);
            D3DXVec3Normalize(&vNormal, &vNormal);
            pv->normal[0] = (unsigned char)((vNormal.z + 1.0f) * 127.5f);
            pv->normal[1] = (unsigned char)((vNormal.y + 1.0f) * 127.5f);
            pv->normal[2] = (unsigned char)((vNormal.x + 1.0f) * 127.5f);

            pv->color = color;
            pv->uv = pCompressedUV[kCornerOrder[i]];
            ++pv;
        }
        m_pVertices = pv;
    }
    else
    {
        LeafVertexUncompressed* pv = (LeafVertexUncompressed*)m_pVertices;
        for (int i = 0; i < 4; ++i)
        {
            pv->pos[0] = corners[i].x;
            pv->pos[1] = corners[i].y;
            pv->pos[2] = corners[i].z;

            PblVector3 vNormal = corners[i];
            D3DXVec3Normalize(&vNormal, &vNormal);
            pv->normal[0] = vNormal.x;
            pv->normal[1] = vNormal.y;
            pv->normal[2] = vNormal.z;

            pv->color = color;
            pv->uv = pUV[kCornerOrder[i]];
            ++pv;
        }
        m_pVertices = pv;
    }
}

// GLOBAL: SWBF2 0x007884f0
const LeafPatchRenderable::TexCoord LeafPatchRenderable::s_TexCoords[2][4][4] = {
    {
        { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
    },
    {
        { { 0.0f, 1.0f }, { 0.5f, 1.0f }, { 0.0f, 0.5f }, { 0.5f, 0.5f } },
        { { 0.5f, 1.0f }, { 1.0f, 1.0f }, { 0.5f, 0.5f }, { 1.0f, 0.5f } },
        { { 0.0f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, 0.0f }, { 0.5f, 0.0f } },
        { { 0.5f, 0.5f }, { 1.0f, 0.5f }, { 0.5f, 0.0f }, { 1.0f, 0.0f } },
    },
};

// GLOBAL: SWBF2 0x007885f8
const LeafPatchRenderable::CompressedTexCoord LeafPatchRenderable::s_CompressedTexCoords[2][4][4] = {
    {
        { { 0, 2047 }, { 2047, 2047 }, { 0, 0 }, { 2047, 0 } },
        { { 2047, 2047 }, { 0, 2047 }, { 2047, 0 }, { 0, 0 } },
        { { 0, 2047 }, { 2047, 2047 }, { 0, 0 }, { 2047, 0 } },
        { { 2047, 2047 }, { 0, 2047 }, { 2047, 0 }, { 0, 0 } },
    },
    {
        { { 0, 2047 }, { 1023, 2047 }, { 0, 1024 }, { 1023, 1024 } },
        { { 1024, 2047 }, { 2047, 2047 }, { 1024, 1024 }, { 2047, 1024 } },
        { { 0, 1023 }, { 1023, 1023 }, { 0, 0 }, { 1023, 0 } },
        { { 1024, 1023 }, { 2047, 1023 }, { 1024, 0 }, { 2047, 0 } },
    },
};

// FUNCTION: SWBF2 0x0041bb80
void LeafPatchClass::DestroyRenderData()
{
    if (mRenderData)
    {
        delete mRenderData;
        mRenderData = 0;
    }
}
