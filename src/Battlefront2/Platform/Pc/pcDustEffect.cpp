#include "pcDustEffect.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcParticleShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x007eafe8
static PblBox sBox(-400.0f, -400.0f, -400.0f, 400.0f, 400.0f, 400.0f);

static void CompressedPosXY_SetBoundingBox(const PblBox& box)
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

static void CompressedPos_Set(short* p, float x, float y, float z)
{
    p[0] = (short)((x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
    p[1] = (short)((y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
    p[2] = (short)((z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
}

struct DustVertexCompressed
{
    short pos[4];
    RedColor color;
    short u, v;
};
struct DustVertex
{
    float x, y, z;
    RedColor color;
    float u, v;
};

// FUNCTION: SWBF2 0x00410c20
DustEffectRenderData::DustEffectRenderData(const DustEffectClass* pClass)
    : Renderable(2)
{
    m_pClass = pClass;
    m_pPrimitive = 0;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        m_pShader = (pcParticleShader*)pcRedShader::Create(PblHash("Particle"), 0x9282,
                                                            &m_pClass->mTextureHash, 1, 0x10004,
                                                            RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        m_pShader = (pcParticleShader*)pcRedShader::Create(PblHash("Particle"), 0x282,
                                                            &m_pClass->mTextureHash, 1, 0x10004,
                                                            RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    m_pShader->NearFadeEnable(true);
    mHeap = new PblHeap<int, float>(m_pClass->mNumParticles, 1000000.0f, 0);
    m_pVertices = 0;
    Activate(0);
}

// FUNCTION: SWBF2 0x00410d40
DustEffectRenderData::~DustEffectRenderData()
{
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }
    if (m_pShader)
    {
        m_pShader->Release();
    }
    delete mHeap;
    Deactivate();
}

// SYNTHETIC: SWBF2 0x00411640
// DustEffectRenderData::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00410e20
void DustEffectRenderData::Render(int value)
{
    unsigned int uFlags = (unsigned int)value;
    if (uFlags & 0x200000)
    {
        if (m_pPrimitive)
        {
            m_pPrimitive->Release();
            m_pPrimitive = 0;
        }
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const float* vForward = pCamera->_Matrix.m[2];
    for (int i = 0; i < m_pClass->mNumParticles; ++i)
    {
        const PblVector3& vPos = m_pClass->mParticle[i].mPos;
        float fKey = -(vPos.y * vForward[1] + vPos.x * vForward[0] + vPos.z * vForward[2]);
        mHeap->Push(i, fKey);
    }

    Begin();
    while (!mHeap->IsEmpty())
    {
        const DustEffectClass::DustParticle& particle = m_pClass->mParticle[mHeap->Get(0)];
        RedColor color;
        color.b = (unsigned char)(int)(particle.mColor.b * 255.0f);
        color.g = (unsigned char)(int)(particle.mColor.g * 255.0f);
        color.r = (unsigned char)(int)(particle.mColor.r * 255.0f);
        color.a = (unsigned char)(int)(particle.mColor.a * 255.0f);
        AddParticle(particle.mPos, particle.mSize, color, 0.0f);
        mHeap->Pop();
    }
    End();
}

// FUNCTION: SWBF2 0x00411020
void DustEffectRenderData::Begin() const
{
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }

    unsigned int uiNumParticles = m_pClass->mNumParticles;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x9282, 16,
                                              uiNumParticles * 4, uiNumParticles * 6, false);
        CompressedPosXY_SetBoundingBox(sBox);
    }
    else
    {
        m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x282, 24,
                                              uiNumParticles * 4, uiNumParticles * 6, false);
    }
    m_pPrimitive->m_bbox = sBox;

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblMatrix& mat = pCamera->_Matrix;
    m_vUp = PblVector3(mat.m[1][0], mat.m[1][1], mat.m[1][2]);
    m_vRight = PblVector3(mat.m[0][0], mat.m[0][1], mat.m[0][2]);
    m_pVertices = m_pPrimitive->BeginVertexData();
}

// FUNCTION: SWBF2 0x004111c0
void DustEffectRenderData::AddParticle(const PblVector3& vPos, float fSize, const RedColor& color,
                                       float fOffset) const
{
    float upX = m_vUp.x * fSize;
    float upY = m_vUp.y * fSize;
    float upZ = m_vUp.z * fSize;
    float rightX = m_vRight.x * fSize;
    float rightY = m_vRight.y * fSize;
    float rightZ = m_vRight.z * fSize;
    float fShift = fOffset - 0.5f;

    float x = (vPos.x - upX * 0.5f) + rightX * fShift;
    float y = (vPos.y - upY * 0.5f) + rightY * fShift;
    float z = (vPos.z - upZ * 0.5f) + rightZ * fShift;

    if (RedRenderer::pcGetGeometryMode())
    {
        DustVertexCompressed* v = (DustVertexCompressed*)m_pVertices;
        CompressedPos_Set(v->pos, x, y, z);
        v->color = color;
        v->u = 0x800;
        v->v = 0;
        m_pVertices = v + 1;

        v = (DustVertexCompressed*)m_pVertices;
        CompressedPos_Set(v->pos, x + rightX, y + rightY, z + rightZ);
        v->color = color;
        v->u = 0x800;
        v->v = 0x800;
        m_pVertices = v + 1;

        v = (DustVertexCompressed*)m_pVertices;
        CompressedPos_Set(v->pos, (x + upX) + rightX, (y + upY) + rightY, (z + upZ) + rightZ);
        v->color = color;
        v->u = 0;
        v->v = 0x800;
        m_pVertices = v + 1;

        v = (DustVertexCompressed*)m_pVertices;
        CompressedPos_Set(v->pos, x + upX, y + upY, z + upZ);
        v->color = color;
        v->u = 0;
        v->v = 0;
        m_pVertices = v + 1;
    }
    else
    {
        DustVertex* v = (DustVertex*)m_pVertices;
        v->x = x;
        v->y = y;
        v->z = z;
        v->color = color;
        v->u = 1.0f;
        v->v = 0.0f;
        m_pVertices = v + 1;

        v = (DustVertex*)m_pVertices;
        v->x = x + rightX;
        v->y = y + rightY;
        v->z = z + rightZ;
        v->color = color;
        v->u = 1.0f;
        v->v = 1.0f;
        m_pVertices = v + 1;

        v = (DustVertex*)m_pVertices;
        v->x = (x + upX) + rightX;
        v->y = (y + upY) + rightY;
        v->z = (z + upZ) + rightZ;
        v->color = color;
        v->u = 0.0f;
        v->v = 1.0f;
        m_pVertices = v + 1;

        v = (DustVertex*)m_pVertices;
        v->x = x + upX;
        v->y = y + upY;
        v->z = z + upZ;
        v->color = color;
        v->u = 0.0f;
        v->v = 0.0f;
        m_pVertices = v + 1;
    }
}

// FUNCTION: SWBF2 0x004115d0
void DustEffectRenderData::End() const
{
    m_pPrimitive->EndVertexData();
    unsigned short* pIndices = m_pPrimitive->BeginIndexData();
    for (int i = 0; i < m_pClass->mNumParticles; ++i)
    {
        unsigned short uBase = (unsigned short)(i * 4);
        pIndices[0] = uBase;
        pIndices[1] = uBase + 1;
        pIndices[2] = uBase + 2;
        pIndices[3] = uBase;
        pIndices[4] = uBase + 2;
        pIndices[5] = uBase + 3;
        pIndices += 6;
    }
    m_pPrimitive->EndIndexData();
    m_pVertices = 0;
}

// FUNCTION: SWBF2 0x00411670
void DustEffectClass::Render(const PblMatrix& matrix, float fAlpha, unsigned int uiFlags) const
{
    RedColor color;
    color.b = RedColor::WHITE.b;
    color.g = RedColor::WHITE.g;
    color.r = RedColor::WHITE.r;
    color.a = (unsigned char)(int)(fAlpha * 255.0f);
    if (mRenderData->m_pPrimitive)
    {
        RedRenderer::pcRenderPrimitive(mRenderData->m_pPrimitive,
                                       mRenderData->m_pShader,
                                       matrix,
                                       color, 0, 0, 0, 0);
    }
}

// FUNCTION: SWBF2 0x004116d0
void DustEffectClass::CreateRenderData()
{
    mRenderData = new DustEffectRenderData(this);
}

// FUNCTION: SWBF2 0x00411740
void DustEffectClass::DestroyRenderData()
{
    if (mRenderData)
    {
        delete mRenderData;
        mRenderData = 0;
    }
}

// FUNCTION: SWBF2 0x00411860
template void PblHeap<int, float>::Pop();

// FUNCTION: SWBF2 0x00411760
template PblHeap<int, float>::PblHeap(int, float, void*);
// FUNCTION: SWBF2 0x004117c0
template PblHeap<int, float>::~PblHeap();
// FUNCTION: SWBF2 0x00411800
template void PblHeap<int, float>::Push(int&, float);

// FUNCTION: SWBF2 0x004117d0
template bool PblHeap<int, float>::IsEmpty();
// FUNCTION: SWBF2 0x004117e0
template int PblHeap<int, float>::Get(int);
