#include "pcRedDropShadow.h"

#include <math.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcParticleShader.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x009acb8c
DropShadowRenderable* RedDropShadowManager::_Instance;

// GLOBAL: SWBF2 0x007e12c0
static float MIN_COS_VIEW_ANGLE = 0.05f;

// GLOBAL: SWBF2 0x007e12c4
static float FADE_COS_VIEW_ANGLE = 0.15f;

// GLOBAL: SWBF2 0x009acb94
// SYNTHETIC: SWBF2 0x00408d20
// `dynamic initializer for 'kuiShader''
static const unsigned int kuiShader = PblHash("Particle");

struct CompressedTexCoord
{
    short u;
    short v;
};

namespace DropShadowVertex
{
struct VertexC
{
    void SetPos(const PblVector3& v) { pos = v; }
    void SetUV(float fU, float fV)
    {
        uv.u = (short)(fU * 2048.0f);
        uv.v = (short)(fV * 2048.0f);
    }

    PblVector3 pos;
    RedColor color;
    CompressedTexCoord uv;
};

struct VertexU
{
    void SetPos(const PblVector3& v) { pos = v; }
    void SetUV(float fU, float fV)
    {
        u = fU;
        v = fV;
    }

    PblVector3 pos;
    RedColor color;
    float u;
    float v;
};
}
 
 

// FUNCTION: SWBF2 0x006edd10
DropShadowRenderable::DropShadowRenderable()
{
    mShadowSize = 1.6f;
    mShadowOffset = 0.1f;
    mPosOffset = 0.1f;
    m_uiTexture = 0;
    m_pShader = 0;
    mShadowCount = 0;
}

// FUNCTION: SWBF2 0x006ee3e0
void RedDropShadowManager::Init()
{
    _Instance = new DropShadowRenderable();
}

// FUNCTION: SWBF2 0x006edd60
DropShadowRenderable::~DropShadowRenderable()
{
    if (m_pShader)
    {
        m_pShader->Release();
    }
}

// SYNTHETIC: SWBF2 0x006eddf0
// DropShadowRenderable::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006ede20
void DropShadowRenderable::AddShadow(unsigned int uiTexture, const PblVector3& vPos,
                                     const PblVector3& vNormal, const PblVector3& vDir,
                                     float fStart, float fEnd, float, float fAlpha)
{
    float fDot = vNormal.y * vDir.y + vDir.x * vNormal.x + vDir.z * vNormal.z;
    PblVector3 vAlong(vDir.x * vNormal.y - gVectorUnitY.x * fDot,
                      vDir.y * vNormal.y - gVectorUnitY.y * fDot,
                      vDir.z * vNormal.y - gVectorUnitY.z * fDot);
    float fLengthSq = vAlong.x * vAlong.x + vAlong.y * vAlong.y + vAlong.z * vAlong.z;
    if (0.0f > fLengthSq)
    {
        return;
    }

    const PblMatrix& camMatrix = RedRenderer::GetCamera()->_Matrix;
    PblVector3 vToShadow(vPos.x - camMatrix.m[3][0],
                         vPos.y - camMatrix.m[3][1],
                         vPos.z - camMatrix.m[3][2]);
    PblVector3 vView;
    D3DXVec3Normalize(&vView, &vToShadow);
    float fCosView = -(vNormal.x * vView.x + vView.y * vNormal.y + vView.z * vNormal.z);
    if (MIN_COS_VIEW_ANGLE >= fCosView)
    {
        return;
    }

    if (++mShadowCount > 32)
    {
        mShadowCount = 32;
    }
    _DropShadow& shadow = mShadowsList[mShadowCount - 1];

    float fLength = sqrt(fLengthSq);
    float fInvLength = 1.0f / fLength;
    vAlong.x *= fInvLength;
    vAlong.y *= fInvLength;
    vAlong.z *= fInvLength;

    float fScale = 2.0f;
    if (!(fLength > -fDot * 2.0f))
    {
        fScale = 1.0f / -fDot * fLength;
    }

    PblVector3 vCross(vNormal.z * vDir.y - vNormal.y * vDir.z,
                      vDir.z * vNormal.x - vNormal.z * vDir.x,
                      vNormal.y * vDir.x - vDir.y * vNormal.x);
    PblVector3 vSide;
    D3DXVec3Normalize(&vSide, &vCross);

    if (FADE_COS_VIEW_ANGLE > fCosView)
    {
        fAlpha = (fCosView - MIN_COS_VIEW_ANGLE) / (FADE_COS_VIEW_ANGLE - MIN_COS_VIEW_ANGLE) * fAlpha;
    }
    RedColor color;
    color.b = 0;
    color.g = 0;
    color.r = 0;
    color.a = (unsigned char)(int)(fAlpha * 255.0f);
    shadow.color = color;

    const PblMatrix& camMatrix2 = RedRenderer::GetCamera()->_Matrix;
    float fOffset = (fStart - mPosOffset) * fScale * mShadowSize;
    PblVector3 vStart(vAlong.x * fOffset + vPos.x + camMatrix2.m[2][0] * mShadowOffset,
                      vAlong.y * fOffset + vPos.y + camMatrix2.m[2][1] * mShadowOffset,
                      vAlong.z * fOffset + vPos.z + camMatrix2.m[2][2] * mShadowOffset);
    float fRun = fEnd - fStart;
    PblVector3 vRun(vAlong.x * fScale * fRun * mShadowSize,
                    vAlong.y * fScale * fRun * mShadowSize,
                    vAlong.z * fScale * fRun * mShadowSize);

    shadow.vtxpos[0] = PblVector3(vStart.x - vSide.x, vStart.y - vSide.y, vStart.z - vSide.z);
    shadow.vtxpos[1] = PblVector3(vSide.x + vStart.x, vSide.y + vStart.y, vSide.z + vStart.z);
    shadow.vtxpos[2] = PblVector3(vSide.x + vStart.x + vRun.x, vSide.y + vStart.y + vRun.y,
                                  vSide.z + vStart.z + vRun.z);
    shadow.vtxpos[3] = PblVector3(vStart.x - vSide.x + vRun.x, vStart.y - vSide.y + vRun.y,
                                  vStart.z - vSide.z + vRun.z);

    if (uiTexture != m_uiTexture)
    {
        m_uiTexture = uiTexture;
        if (m_pShader)
        {
            m_pShader->Release();
            m_pShader = 0;
        }
        if (m_uiTexture)
        {
            if (RedRenderer::pcGetGeometryMode())
            {
                m_pShader = pcRedShader::Create(kuiShader, 0x8282, &m_uiTexture, 1, 0x10004,
                                                RedColor::WHITE, RedColor::BLACK, 32.0f);
            }
            else
            {
                m_pShader = pcRedShader::Create(kuiShader, 0x282, &m_uiTexture, 1, 0x10004,
                                                RedColor::WHITE, RedColor::BLACK, 32.0f);
            }
        }
    }
}

template <class VERTEX>
static void DropShadowFillQuads(const DropShadowRenderable::_DropShadow* pShadows, int iCount,
                                float fShade, VERTEX* pVertex, unsigned short* pIndices)
{
    unsigned short usBase = 0;
    for (int i = 0; i < iCount; ++i)
    {
        const DropShadowRenderable::_DropShadow& shadow = pShadows[i];
        RedColor color;
        color.b = shadow.color.b;
        color.g = shadow.color.g;
        color.r = shadow.color.r;
        color.a = (unsigned char)(int)(shadow.color.a * fShade);
        pVertex[0].SetPos(shadow.vtxpos[0]);
        pVertex[0].color = color;
        pVertex[0].SetUV(0.8f, 1.0f);
        pVertex[1].SetPos(shadow.vtxpos[1]);
        pVertex[1].color = color;
        pVertex[1].SetUV(0.2f, 1.0f);
        pVertex[2].SetPos(shadow.vtxpos[2]);
        pVertex[2].color = color;
        pVertex[2].SetUV(0.2f, 0.0f);
        pVertex[3].SetPos(shadow.vtxpos[3]);
        pVertex[3].color = color;
        pVertex[3].SetUV(0.8f, 0.0f);
        pVertex += 4;
        if (usBase)
        {
            pIndices[0] = pIndices[-1];
            pIndices[1] = usBase;
            pIndices += 2;
        }
        pIndices[0] = usBase;
        pIndices[1] = (unsigned short)(usBase + 1);
        pIndices[2] = (unsigned short)(usBase + 3);
        pIndices[3] = (unsigned short)(usBase + 2);
        pIndices += 4;
        usBase += 4;
    }
}

// FUNCTION: SWBF2 0x006ee460
void DropShadowRenderable::Render(int value)
{
    unsigned int uFlags = (unsigned int)value;
    if (mShadowCount <= 0)
    {
        return;
    }

    const PblBox& camBox = RedRenderer::GetCamera()->_box;
    PblBox box;
    box._vMax = PblVector3(camBox._vMax.x + 20.0f, camBox._vMax.y + 20.0f, camBox._vMax.z + 20.0f);
    box._vMin = PblVector3(camBox._vMin.x - 20.0f, camBox._vMin.y - 20.0f, camBox._vMin.z - 20.0f);

    unsigned short usNumVertices = (unsigned short)(mShadowCount * 4);
    unsigned short usNumIndices = (unsigned short)(mShadowCount * 6 - 2);
    pcRedPrimitive* pPrimitive;
    if (RedRenderer::pcGetGeometryMode())
    {
        pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x8282, sizeof(DropShadowVertex::VertexC),
                                            usNumVertices, usNumIndices, false);
    }
    else
    {
        pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x282, sizeof(DropShadowVertex::VertexU),
                                            usNumVertices, usNumIndices, false);
    }
    pPrimitive->m_bbox = box;

    void* pVertices = pPrimitive->BeginVertexData();
    if (!pVertices)
    {
        return;
    }
    unsigned short* pIndices = pPrimitive->BeginIndexData();

    float fShade = 1.0f;
    if (RedStencilShadow::GetShadowEnable())
    {
        ((pcParticleShader*)m_pShader)->SetShadowMode(true);
    }
    else
    {
        const RedColorValue& sun = ((RedLight*)RedLight::m_sGlobalDirectionals[0])->m_color;
        const RedColorValue& amb = ((RedLight*)RedLight::m_sGlobalAmbient)->m_color;
        RedColor sunColor;
        sunColor.b = (unsigned char)(int)(sun.b * 255.0f);
        sunColor.g = (unsigned char)(int)(sun.g * 255.0f);
        sunColor.r = (unsigned char)(int)(sun.r * 255.0f);
        RedColor ambColor;
        ambColor.b = (unsigned char)(int)(amb.b * 255.0f);
        ambColor.g = (unsigned char)(int)(amb.g * 255.0f);
        ambColor.r = (unsigned char)(int)(amb.r * 255.0f);
        float fSun = (sunColor.r * 0.32f + sunColor.g * 0.43f + sunColor.b * 0.25f) / 255.0f;
        float fAmb = (ambColor.r * 0.32f + ambColor.g * 0.43f + ambColor.b * 0.25f) / 255.0f;
        ((pcParticleShader*)m_pShader)->SetShadowMode(false);
        fShade = fSun / (fAmb + fSun);
    }

    if (RedRenderer::pcGetGeometryMode())
    {
        DropShadowFillQuads(mShadowsList, mShadowCount, fShade,
                            (DropShadowVertex::VertexC*)pVertices, pIndices);
    }
    else
    {
        DropShadowFillQuads(mShadowsList, mShadowCount, fShade,
                            (DropShadowVertex::VertexU*)pVertices, pIndices);
    }

    pPrimitive->EndVertexData();
    pPrimitive->EndIndexData();

    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   m_pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE,
                                   uFlags | 0x10, 0, 0, 0);
    pPrimitive->Release();
    mShadowCount = 0;
}

// FUNCTION: SWBF2 0x006ee350
void RedDropShadowManager::AddShadow(unsigned int uiTexture, const PblVector3& vPos,
                                     const PblVector3& vNormal, const PblVector3& vDir,
                                     float fStart, float fEnd, float fArg7, float fAlpha)
{
    if (RedRenderer::pcGetRenderOptions().shadowQuality)
    {
        _Instance->AddShadow(uiTexture, vPos, vNormal, vDir, fStart, fEnd, fArg7, fAlpha);
    }
}

// FUNCTION: SWBF2 0x006ee3b0
void RedDropShadowManager::Cleanup()
{
    FLRenderer::RemoveRenderable(_Instance);
    delete _Instance;
    _Instance = 0;
}

// FUNCTION: SWBF2 0x006ee970
void RedDropShadowManager::Render(unsigned int value)
{

    if (RedDropShadowManager::_Instance)
    {
        RedDropShadowManager::_Instance->Render((int)value);
    }
}
