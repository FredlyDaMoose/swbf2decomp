#include <float.h>
#include <math.h>
#include <string.h>
#include <d3dx9math.h>

#include "pcRedStencilShadow.h"
#include "pcRedShader.h"
#include "pcRedRenderer.h"
#include "pcRedAutoSettings.h"
#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"
#include "pcRedPrimitive.h"
#include "pcRedVertexBuffer.h"
#include "pcRedIndexBuffer.h"
#include "Shaders/pcStencilShadowShader.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"

void __cdecl PblTraceP(const char* Format, ...);

static const D3DXMATRIXA16 _IdentityMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 0.0f,
                                           0.0f, 0.0f, 0.0f, 1.0f);

static const float kInv255 = 1.0f / 255.0f;

bool RedStencilShadow::s_bShadowsEnabled;

static const PblMatrix& _GetSkinMatrix(unsigned int uiNumBones, const unsigned char* puiWeights,
                                       const unsigned char* puiIndices, const PblMatrix* pMatrices);

// GLOBAL: SWBF2 0x009a0944
unsigned int ShadowData::_iShadowMem;

// GLOBAL: SWBF2 0x009a0948
unsigned int ShadowData::_iShadowSilhouetteVertexCount;

// GLOBAL: SWBF2 0x009a0958
// SYNTHETIC: SWBF2 0x00408a60
// `dynamic initializer for 'RedStencilShadow::s_aTestPlane''

PblVector4 RedStencilShadow::s_aTestPlane[5];

// GLOBAL: SWBF2 0x009a09ac
// SYNTHETIC: SWBF2 0x00408a80
// `dynamic initializer for 'RedStencilShadow::s_vNearPlaneCenter''

PblVector3 RedStencilShadow::s_vNearPlaneCenter;

// GLOBAL: SWBF2 0x009a09b8
// SYNTHETIC: SWBF2 0x00408a90
// `dynamic initializer for 'RedStencilShadow::s_vProjection''

PblVector3 RedStencilShadow::s_vProjection;

// GLOBAL: SWBF2 0x009a094c
float RedStencilShadow::s_fNearPlaneRadius;

// GLOBAL: SWBF2 0x009a0950
int RedStencilShadow::s_iNumTestPlanes;

// GLOBAL: SWBF2 0x007dfa88
float RedStencilShadow::s_fShadowVolumeLength = 50.0f;
// GLOBAL: SWBF2 0x007dfa8c
float RedStencilShadow::s_fShadowVolumeOffset = 0.1f;
// GLOBAL: SWBF2 0x007dfae0
PblVector3 ShadowRenderable::s_vProjectionUnlit(0.0f, -1.0f, 0.0f);
// GLOBAL: SWBF2 0x009a09c4
PblVector3 ShadowRenderable::s_vProjectionLit;

// FUNCTION: SWBF2 0x006e7f70
void ShadowRenderable::SetWorldMatrix(const PblMatrix& matrix)
{
    PblMatrix inverse = matrix.GetInverseFast();
    PblVector3 unlit;
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&unlit),
                            reinterpret_cast<const D3DXVECTOR3*>(&s_vProjectionUnlit),
                            reinterpret_cast<const D3DXMATRIX*>(&inverse));

    if (!m_bSilhouetteChanged || !m_bProjectionChanged)
    {
        PblVector3 oldDirection;
        PblVector3 newDirection;
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&oldDirection),
                          reinterpret_cast<const D3DXVECTOR3*>(&m_vProjectionUnlitLocal));
        D3DXVec3Normalize(reinterpret_cast<D3DXVECTOR3*>(&newDirection),
                          reinterpret_cast<const D3DXVECTOR3*>(&unlit));
        const float cosine = oldDirection.x * newDirection.x
                           + oldDirection.y * newDirection.y
                           + oldDirection.z * newDirection.z;
        const float difference = 1.0f - cosine;
        if (!m_bSilhouetteChanged && difference > 0.0009f)
        {
            m_bSilhouetteChanged = true;
            m_bProjectionChanged = true;
        }
        else if (difference > 0.0f)
        {
            m_bProjectionChanged = true;
        }

        if (!m_bProjectionChanged)
        {
            const float dx = m_vProjectionUnlitLocal.x - unlit.x;
            const float dy = m_vProjectionUnlitLocal.y - unlit.y;
            const float dz = m_vProjectionUnlitLocal.z - unlit.z;
            if (dy * dy + dx * dx + dz * dz > 1.0f)
            {
                m_bProjectionChanged = true;
            }
        }
    }

    m_mWorldMatrix = matrix;
    m_vProjectionUnlitLocal = unlit;
    inverse = matrix.GetInverseFast();
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&m_vProjectionLitLocal),
                            reinterpret_cast<const D3DXVECTOR3*>(&s_vProjectionLit),
                            reinterpret_cast<const D3DXMATRIX*>(&inverse));
}

// FUNCTION: SWBF2 0x006e6a60
void ShadowRenderable::Render(pcRedShader* pShader, const PblMatrix& matrix,
                              const PblMatrix* pBoneMatrices, int iNumBones,
                              unsigned int uiFlags)
{
    if (RedRenderer::pcGetRenderOptions().shadowQuality != QUALITY2_HIGH)
    {
        return;
    }

    if (pBoneMatrices)
    {
        m_pShadowData->GenerateSkin(pBoneMatrices, iNumBones, uiFlags);
        m_bSilhouetteChanged = true;
        m_bProjectionChanged = true;
    }

    SetWorldMatrix(matrix);

    if (m_bSilhouetteChanged)
    {
        PblVector3 vLight(s_vProjectionUnlit.x - s_vProjectionLit.x,
                          s_vProjectionUnlit.y - s_vProjectionLit.y,
                          s_vProjectionUnlit.z - s_vProjectionLit.z);
        m_pShadowData->GenerateSilhouette(&m_mWorldMatrix, vLight);
        CreateSilhouette();
        m_bSilhouetteChanged = false;
        m_bProjectionChanged = false;
    }

    if (m_bProjectionChanged)
    {
        ProjectEdges();
        m_bProjectionChanged = false;
    }

    PblMatrix mLit = m_mWorldMatrix;
    mLit.m[3][0] += s_vProjectionLit.x;
    mLit.m[3][1] += s_vProjectionLit.y;
    mLit.m[3][2] += s_vProjectionLit.z;
    PblMatrix mUnlit = m_mWorldMatrix;
    mUnlit.m[3][0] += s_vProjectionUnlit.x;
    mUnlit.m[3][1] += s_vProjectionUnlit.y;
    mUnlit.m[3][2] += s_vProjectionUnlit.z;

    if (m_iNumSilhouetteVertices <= 0)
    {
        return;
    }

    const PblVector3* pVertices = m_pShadowData->_pSkinnedVertices;
    if (!pVertices)
    {
        pVertices = m_pShadowData->_pVertices;
    }

    unsigned int uiStartVertex = 0;
    unsigned int uiStartIndexLit = 0;
    unsigned int uiStartIndexUnlit = 0;
    pcRedVertexBuffer* pVertexBuffer =
        pcRedVertexBuffer::Create(m_pShadowData->_iNumVertices, sizeof(PblVector3), false, true);
    memcpy(pVertexBuffer->Lock(m_pShadowData->_iNumVertices, uiStartVertex), pVertices,
           m_pShadowData->_iNumVertices * sizeof(PblVector3));
    pVertexBuffer->Unlock();

    if (m_iNumIndicesCapLit)
    {
        pcRedIndexBuffer* pIndexBuffer = pcRedIndexBuffer::Create(m_iNumIndicesCapLit, false, true);
        memcpy(pIndexBuffer->Lock(m_iNumIndicesCapLit, uiStartIndexLit), m_pIndices,
               m_iNumIndicesCapLit * sizeof(unsigned short));
        pIndexBuffer->Unlock();
        pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, m_iNumIndicesCapLit / 3,
                                                       2, m_pShadowData->_iNumVertices,
                                                       uiStartVertex, pVertexBuffer,
                                                       uiStartIndexLit, pIndexBuffer);
        pIndexBuffer->Release();
        RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                       mLit,
                                       RedColor::WHITE, 0, 0, 0, 0);
        pPrim->Release();
    }

    if (m_iNumIndicesCapUnlit)
    {
        pcRedIndexBuffer* pIndexBuffer = pcRedIndexBuffer::Create(m_iNumIndicesCapUnlit, false, true);
        memcpy(pIndexBuffer->Lock(m_iNumIndicesCapUnlit, uiStartIndexUnlit),
               m_pIndices + m_iNumIndicesCapLit, m_iNumIndicesCapUnlit * sizeof(unsigned short));
        pIndexBuffer->Unlock();
        pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, m_iNumIndicesCapUnlit / 3,
                                                       2, m_pShadowData->_iNumVertices,
                                                       uiStartVertex, pVertexBuffer,
                                                       uiStartIndexUnlit, pIndexBuffer);
        pIndexBuffer->Release();
        RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                       mUnlit,
                                       RedColor::WHITE, 0, 0, 0, 0);
        pPrim->Release();
    }

    pVertexBuffer->Release();

    if (m_iNumSilhouetteVertices)
    {
        pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 2, sizeof(PblVector3),
                                                       m_iNumSilhouetteVertices, 0, false);
        memcpy(pPrim->BeginVertexData(), m_pSilhouetteVertices,
               m_iNumSilhouetteVertices * sizeof(PblVector3));
        pPrim->EndVertexData();
        RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                       mLit,
                                       RedColor::WHITE, 0, 0, 0, 0);
        pPrim->Release();
    }
}

// FUNCTION: SWBF2 0x006e6ed0
void RedStencilShadow::ShadowSegment::Render(const PblMatrix* pMatrix,
                                              const PblMatrix* pBoneMatrices,
                                              int iNumBones, unsigned int uiFlags) const
{
    if (m_pPrimitive)
    {
        RedRenderer::pcRenderPrimitive(m_pPrimitive,
                                       m_pGPUShadowShader,
                                       *pMatrix,
                                       RedColor::WHITE,
                                       0, iNumBones ? pBoneMatrices : 0,
                                       iNumBones ? iNumBones : 0, 0);
    }
    else
    {
        m_pShadowRenderable->Render(m_pCPUShadowShader, *pMatrix,
                                    iNumBones ? pBoneMatrices : 0,
                                    iNumBones ? iNumBones : 0,
                                    iNumBones ? uiFlags : 0);
    }
}

// FUNCTION: SWBF2 0x006e67d0
void RedStencilShadow::Render(const PblMatrix* pMatrix,
                              const PblMatrix** ppBoneMatrices,
                              const RedPose* pPose, bool bFlag) const
{
    if (!GetShadowEnable())
    {
        return;
    }

    const PblVector3& direction = RedLight::GetGlobalDirectionalLight(0)->m_vDirection;
    const float fOffset = s_fShadowVolumeOffset;
    const float fLength = RedRenderer::GetStencilShadowLength() + fOffset;
    ShadowRenderable::s_vProjectionLit = PblVector3(direction.x * fOffset,
                                                      direction.y * fOffset,
                                                      direction.z * fOffset);
    ShadowRenderable::s_vProjectionUnlit = PblVector3(direction.x * fLength,
                                                        direction.y * fLength,
                                                        direction.z * fLength);

    PblMatrix aSkinMatrices[128];
    for (PblList<ShadowSegment>::Node* pNode = m_segmentList.GetFirst();
         pNode->_pObject; pNode = pNode->_pNext)
    {
        const ShadowSegment* pSegment = pNode->_pObject;
        const PblMatrix* pWorld = pMatrix;
        PblMatrix* pSkin = 0;
        const unsigned int uNumBones = pSegment->m_uiNumBones;
        if (!uNumBones)
        {
            const unsigned int uBone = pSegment->m_uiBoneIndex;
            if (pPose)
            {
                if (ppBoneMatrices)
                {
                    if (!ppBoneMatrices[uBone])
                    {
                        PblMatrix bone;
                        D3DXMatrixMultiply((D3DXMATRIX*)&bone,
                                           (const D3DXMATRIX*)&pPose->_GetBoneMatrix(uBone),
                                           (const D3DXMATRIX*)pMatrix);
                        ppBoneMatrices[uBone] = RedRenderer::AllocMatrixInCache(bone);
                    }
                    pWorld = ppBoneMatrices[uBone];
                }
                else
                {
                    PblMatrix bone;
                    D3DXMatrixMultiply((D3DXMATRIX*)&bone,
                                       (const D3DXMATRIX*)&pPose->_GetBoneMatrix(uBone),
                                       (const D3DXMATRIX*)pMatrix);
                    pWorld = RedRenderer::AllocMatrixInCache(bone);
                }
            }
            else if (ppBoneMatrices)
            {
                pWorld = ppBoneMatrices[uBone];
            }
        }
        else
        {
            pSkin = pSegment->m_puiBoneIndexMap ?
                RedRenderer::AllocBoneMatricesInCache(uNumBones) : aSkinMatrices;
            if (pPose)
            {
                if (pSegment->m_hardSkin)
                {
                    pPose->GetHardSkinMatrices(pSkin, pSegment->m_puiBoneIndexMap, uNumBones);
                }
                else
                {
                    pPose->GetSkinMatrices(pSkin, pSegment->m_puiBoneIndexMap, uNumBones);
                }
            }
            else
            {
                m_pModel->GetSkeleton()->GetSkinMatrices(pSkin, pSegment->m_puiBoneIndexMap,
                                                         uNumBones);
            }
        }
        pSegment->Render(pWorld, pSkin, uNumBones, 4);
    }
}

// FUNCTION: SWBF2 0x006e8100
RedStencilShadow::VisibilityTestT RedStencilShadow::ShadowVolumeVisibilityTest(
    const PblVector3& vCenter, float fRadius, float fLength)
{
    const float dx = s_vNearPlaneCenter.x - vCenter.x;
    const float dy = s_vNearPlaneCenter.y - vCenter.y;
    const float dz = s_vNearPlaneCenter.z - vCenter.z;
    const float projection = s_vProjection.x * dx + s_vProjection.y * dy +
                             s_vProjection.z * dz;
    const float distanceSq = dx * dx + dy * dy + dz * dz;
    const float radius = s_fNearPlaneRadius + fRadius;
    const float radiusSq = radius * radius;
    if (projection > 0.0f)
    {
        if (projection < fLength)
        {
            if (radiusSq > distanceSq - projection * projection)
            {
                return TEST_CONTAINS_CAMERA;
            }
        }
        else if (radiusSq > distanceSq - 2.0f * fLength * projection + fLength * fLength)
        {
            return TEST_CONTAINS_CAMERA;
        }
    }
    else if (radiusSq > distanceSq)
    {
        return TEST_CONTAINS_CAMERA;
    }

    for (int i = 0; i < s_iNumTestPlanes; ++i)
    {
        const PblVector4& plane = s_aTestPlane[i];
        if (plane.x * vCenter.x + plane.y * vCenter.y + plane.z * vCenter.z + plane.w > fRadius)
        {
            return TEST_NOT_VISIBLE;
        }
    }
    return TEST_VISIBLE;
}

// FUNCTION: SWBF2 0x006e4f10
RedStencilShadow::RedStencilShadow()
{
}

// FUNCTION: SWBF2 0x006e4f40
ShadowData::ShadowData()
    : _iNumVertices(0), _pVertices(0), _edgeCount(0), _edgeList(0), _puiSkinData(0),
      _uiSkinNumBones(0), _faceCount(0), _facePlanesEntry(0), _fSizeInvSqr(0),
      _facePlanes(0), _pSkinnedVertices(0), _silhouetteAllocated(0), _silhouette(0)
{

    _silhouetteCount = 0;
}

// FUNCTION: SWBF2 0x006e4f80
ShadowRenderable::ShadowRenderable(ShadowData* pShadowData)
{
    m_pShadowData = pShadowData;
    m_bSilhouetteChanged = true;
    m_bProjectionChanged = true;
    m_pSilhouetteVertices = 0;
    m_pIndices = (unsigned short*)RedAllocFromHeap(
        __RedCurrHeap, pShadowData->_triCount * 3 * sizeof(unsigned short), 0);
    m_vProjectionUnlitLocal = PblVector3(0.0f, -1.0f, 0.0f);
    m_vProjectionLitLocal = PblVector3(0.0f, 0.0f, 0.0f);
    m_iNumSilhouetteVerticesAllocated = 0;
    m_iNumSilhouetteVertices = 0;
    m_iNumIndicesCapUnlit = 0;
    m_iNumIndicesCapLit = 0;
}

// FUNCTION: SWBF2 0x006e5020
RedStencilShadow::ShadowSegment::ShadowSegment()
{
    m_uiBoneIndex = 0;
    m_uiNumBones = 0;
    m_puiBoneIndexMap = 0;
    m_pPrimitive = 0;
    m_pShadowRenderable = 0;
    m_pCPUShadowShader = 0;
    m_pGPUShadowShader = 0;
}

// FUNCTION: SWBF2 0x006e5070
RedStencilShadow::~RedStencilShadow()
{
    m_segmentList.Clear();
}

// FUNCTION: SWBF2 0x006e50d0
ShadowData::~ShadowData()
{
    if (_pVertices)
    {
        delete[] _pVertices;
    }
    if (_edgeList)
    {
        delete[] _edgeList;
    }
    if (_puiSkinData)
    {
        delete[] _puiSkinData;
    }
    if (_facePlanes)
    {
        delete[] _facePlanes;
    }
    if (_pSkinnedVertices)
    {
        delete[] _pSkinnedVertices;
    }
    if (_silhouette)
    {
        delete[] _silhouette;
        _iShadowMem -= _silhouetteAllocated * sizeof(SilhouetteVertex);
        _iShadowSilhouetteVertexCount -= _silhouetteAllocated;
    }
}

// FUNCTION: SWBF2 0x006e5160
ShadowRenderable::~ShadowRenderable()
{
    if (m_pShadowData)
    {
        delete m_pShadowData;
    }
    if (m_pSilhouetteVertices)
    {
        RedFreeToHeap(__RedCurrHeap, m_pSilhouetteVertices, 0);
    }
    if (m_pIndices)
    {
        RedFreeToHeap(__RedCurrHeap, m_pIndices, 0);
    }
}

// FUNCTION: SWBF2 0x006e51b0
RedStencilShadow::ShadowSegment::~ShadowSegment()
{
    if (m_pShadowRenderable)
    {
        delete m_pShadowRenderable;
    }
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }
    if (m_pCPUShadowShader)
    {
        m_pCPUShadowShader->Release();
    }
    if (m_pGPUShadowShader)
    {
        m_pGPUShadowShader->Release();
    }
    delete[] m_puiBoneIndexMap;
}

// SYNTHETIC: SWBF2 0x006e5270
// RedStencilShadow::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006e52a0
// ShadowData::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006e52d0
void ShadowData::AddSilhouetteVertex(unsigned short uiVertex, bool bStartSilhouette)
{
    if (_silhouetteCount >= _silhouetteAllocated)
    {
        _iShadowMem += _silhouetteAllocated * sizeof(SilhouetteVertex);
        _iShadowSilhouetteVertexCount += _silhouetteAllocated;
        _silhouetteAllocated *= 2;
        PblTraceP("ShadowData: reallocating silhouette to %d bytes\n", _silhouetteAllocated);
        SilhouetteVertex* pNew = new SilhouetteVertex[_silhouetteAllocated];
        memcpy(pNew, _silhouette, _silhouetteCount * sizeof(SilhouetteVertex));
        delete[] _silhouette;
        _silhouette = pNew;
    }
    _silhouette[_silhouetteCount].vertex = uiVertex;
    _silhouette[_silhouetteCount].bStartSilhouette = bStartSilhouette;
    _silhouetteCount++;
}

// FUNCTION: SWBF2 0x006e5380
void ShadowRenderable::CreateSilhouette()
{
    ShadowData* pShadowData = m_pShadowData;
    m_iNumSilhouetteVertices = 0;
    for (unsigned short i = 0; i < pShadowData->_silhouetteCount; ++i)
    {
        if (i && pShadowData->_silhouette[i].bStartSilhouette)
        {
            m_iNumSilhouetteVertices += 2;
        }
        m_iNumSilhouetteVertices += 2;
    }

    if (m_iNumSilhouetteVertices > m_iNumSilhouetteVerticesAllocated)
    {
        delete[] m_pSilhouetteVertices;
        m_iNumSilhouetteVerticesAllocated = m_iNumSilhouetteVertices * 5 / 4;
        m_pSilhouetteVertices = new PblVector3[m_iNumSilhouetteVerticesAllocated];
    }

    ProjectEdges();

    unsigned short* pLit = m_pIndices;
    m_iNumIndicesCapLit = 0;
    m_iNumIndicesCapUnlit = 0;
    unsigned short* pUnlit = m_pIndices + m_pShadowData->_triCount * 3;
    ShadowData::ShadowEdge* pEdges = m_pShadowData->_edgeList;
    for (unsigned short i = 0; i < m_pShadowData->_edgeCount; ++i)
    {
        ShadowData::ShadowEdge* pEdge = &pEdges[i];
        if (pEdge->mark & 0x40)
        {
            continue;
        }

        bool bVisible = pEdge->visible != 0;
        ShadowData::ShadowEdge* pNext = &pEdges[pEdge->next];
        ShadowData::ShadowEdge* pCur = &pEdges[pNext->next];
        unsigned short c = pCur->vertex;
        unsigned short a = pEdge->vertex;
        unsigned short b = pNext->vertex;
        pNext->mark = 0x40;
        pEdge->mark = 0x40;
        unsigned short uiNext;
        do
        {
            if (bVisible)
            {
                pLit[0] = a;
                pLit[1] = b;
                pLit[2] = c;
                pLit += 3;
                m_iNumIndicesCapLit += 3;
            }
            else
            {
                *--pUnlit = c;
                *--pUnlit = b;
                *--pUnlit = a;
                m_iNumIndicesCapUnlit += 3;
            }
            b = c;
            pCur->mark = 0x40;
            uiNext = pCur->next;
            pCur = &pEdges[uiNext];
            c = pCur->vertex;
        } while (uiNext != i);
    }
}

// FUNCTION: SWBF2 0x006e5520
unsigned short ShadowData::GenerateSilhouette(const PblMatrix* pMatrix, const PblVector3& vLight)
{
    _silhouetteCount = 0;
    if (!_iNumVertices || !_edgeCount || !_pVertices || !_edgeList)
    {
        return 0;
    }

    const float lx = pMatrix->m[0][0] * vLight.x + pMatrix->m[0][1] * vLight.y + pMatrix->m[0][2] * vLight.z;
    const float ly = pMatrix->m[1][0] * vLight.x + pMatrix->m[1][1] * vLight.y + pMatrix->m[1][2] * vLight.z;
    const float lz = pMatrix->m[2][0] * vLight.x + pMatrix->m[2][1] * vLight.y + pMatrix->m[2][2] * vLight.z;

    for (unsigned short i = 0; i < _faceCount; ++i)
    {
        const PblVector4& plane = _facePlanes[i];
        const unsigned short uiFirst = _facePlanesEntry[i];
        const bool bVisible = plane.x * lx + plane.y * ly + plane.z * lz < 0.0f;
        unsigned short e = uiFirst;
        do
        {
            _edgeList[e].visible = bVisible;
            _edgeList[e].mark = 0;
            e = _edgeList[e].next;
        } while (e != uiFirst);
    }

    for (unsigned short j = 0; j < _edgeCount; ++j)
    {
        if (_edgeList[j].mark)
        {
            continue;
        }
        unsigned short uiTwin = _edgeList[j].twin;
        _edgeList[uiTwin].mark = 1;
        unsigned char bVisible = _edgeList[j].visible;
        _edgeList[j].mark = 1;
        if (bVisible == _edgeList[uiTwin].visible)
        {
            continue;
        }

        unsigned short e = bVisible ? j : _edgeList[j].twin;
        AddSilhouetteVertex(_edgeList[e].vertex, true);
        for (;;)
        {
            unsigned short uiEnd = _edgeList[e].twin;
            unsigned short k = _edgeList[e].next;
            while (k != uiEnd)
            {
                unsigned short kt = _edgeList[k].twin;
                if (!_edgeList[k].mark && _edgeList[k].visible && !_edgeList[kt].visible)
                {
                    break;
                }
                if (_edgeList[kt].visible == _edgeList[k].visible)
                {
                    _edgeList[kt].mark = 1;
                    _edgeList[k].mark = 1;
                }
                k = _edgeList[kt].next;
            }
            if (k == uiEnd)
            {
                AddSilhouetteVertex(_edgeList[uiEnd].vertex, false);
                break;
            }
            AddSilhouetteVertex(_edgeList[k].vertex, false);
            _edgeList[_edgeList[k].twin].mark = 1;
            _edgeList[k].mark = 1;
            e = k;
        }
    }
    return _silhouetteCount;
}

// FUNCTION: SWBF2 0x006e5780
void ShadowData::GenerateSkin(const PblMatrix* pBoneMatrices, int iNumBoneMatrices,
                              unsigned int uiMaxBones)
{
    if (!_iNumVertices || !_edgeCount || !_pVertices || !_pSkinnedVertices || !_puiSkinData ||
        !_edgeList)
    {
        return;
    }

    if (uiMaxBones > _uiSkinNumBones)
    {
        uiMaxBones = _uiSkinNumBones;
    }
    const int iStride = 2 * _uiSkinNumBones - 1;
    const unsigned char* puiWeights = _puiSkinData;
    const unsigned char* puiIndices = _puiSkinData + _uiSkinNumBones - 1;
    for (int i = 0; i < _iNumVertices; ++i)
    {
        const PblMatrix& mSkin = _GetSkinMatrix(uiMaxBones, puiWeights, puiIndices, pBoneMatrices);
        D3DXVECTOR3 v;
        D3DXVec3TransformCoord(&v, (const D3DXVECTOR3*)&_pVertices[i], (const D3DXMATRIX*)&mSkin);
        _pSkinnedVertices[i] = *(const PblVector3*)&v;
        puiWeights += iStride;
        puiIndices += iStride;
    }

    const PblVector3* pV = _pSkinnedVertices;
    int iPlane = 0;
    for (unsigned short i = 0; i < _edgeCount; ++i)
    {
        if (_edgeList[i].mark & 0x80)
        {
            continue;
        }
        unsigned short uiNext = _edgeList[i].next;
        unsigned short a = _edgeList[i].vertex;
        unsigned short e = _edgeList[uiNext].next;
        unsigned short b = _edgeList[uiNext].vertex;
        unsigned short c = _edgeList[e].vertex;
        _edgeList[uiNext].mark = 0x80;
        _edgeList[i].mark = 0x80;

        const float d1x = pV[a].x - pV[b].x, d1y = pV[a].y - pV[b].y, d1z = pV[a].z - pV[b].z;
        const float d2x = pV[c].x - pV[b].x, d2y = pV[c].y - pV[b].y, d2z = pV[c].z - pV[b].z;
        PblVector4 plane;
        plane.x = d2y * d1z - d2z * d1y;
        plane.y = d2z * d1x - d1z * d2x;
        plane.z = d1y * d2x - d2y * d1x;
        do
        {
            _edgeList[e].mark = 0x80;
            e = _edgeList[e].next;
            a = b;
            b = c;
            c = _edgeList[e].vertex;
        } while (e != i);
        plane.w = -(pV[a].x * plane.x + pV[a].y * plane.y + pV[a].z * plane.z);
        _facePlanes[iPlane++] = plane;
    }
}

// FUNCTION: SWBF2 0x006e5a20
bool RedStencilShadow::GetShadowEnable()
{
    if (RedStencilShadow::s_bShadowsEnabled &&
        RedRenderer::pcGetRenderOptions().shadowQuality == QUALITY2_HIGH &&
        RedRenderer::pcGetRenderCaps().pipeline)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006e5a50
void ShadowRenderable::ProjectEdges()
{
    const PblVector3* pVertices = m_pShadowData->_pSkinnedVertices;
    if (!pVertices)
    {
        pVertices = m_pShadowData->_pVertices;
    }
    const float px = m_vProjectionUnlitLocal.x - m_vProjectionLitLocal.x;
    const float py = m_vProjectionUnlitLocal.y - m_vProjectionLitLocal.y;
    const float pz = m_vProjectionUnlitLocal.z - m_vProjectionLitLocal.z;
    PblVector3* pOut = m_pSilhouetteVertices;
    for (unsigned short i = 0; i < m_pShadowData->_silhouetteCount; ++i)
    {
        const ShadowData::SilhouetteVertex& sv = m_pShadowData->_silhouette[i];
        const PblVector3& v = pVertices[sv.vertex];
        if (i && sv.bStartSilhouette)
        {
            pOut[0] = pOut[-1];
            pOut[1] = v;
            pOut += 2;
        }
        pOut[0] = v;
        pOut[1] = PblVector3(v.x + px, v.y + py, v.z + pz);
        pOut += 2;
    }
}

// FUNCTION: SWBF2 0x006e5b50
void RedStencilShadow::Read(PblFileChunk& chunk)
{
    unsigned int uiNumBones = 0;
    unsigned int uiBoneIndex = 0;
    unsigned char* puiBoneIndexMap = 0;
    ShadowSegment* pSegment = new ShadowSegment;
    const RenderCapsT& caps = RedRenderer::pcGetRenderCaps();
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x44485347:
            if (caps.shadowVolume)
            {
                ReadGPUShadowData(child, pSegment);
            }
            break;
        case 0x44485343:
            if (!caps.shadowVolume)
            {
                ReadCPUShadowData(child, pSegment);
            }
            break;
        case 0x4D414E42:
        {
            char acName[256];
            child.ReadString(acName, 256);
            uiBoneIndex = m_pModel->GetBoneIndex(acName);
            break;
        }
        case 0x50414D42:
            uiNumBones = child._pFile->Read32();
            if (uiNumBones)
            {
                puiBoneIndexMap = new unsigned char[uiNumBones];
                child._pFile->ReadData(puiBoneIndexMap, uiNumBones);
            }
            break;
        }
    }
    pSegment->m_puiBoneIndexMap = puiBoneIndexMap;
    pSegment->m_uiNumBones = uiNumBones;
    pSegment->m_uiBoneIndex = uiBoneIndex;
    if (pSegment->m_pPrimitive || pSegment->m_pShadowRenderable)
    {
        m_segmentList.AddLast(pSegment, pSegment);
    }
    else
    {
        delete pSegment;
    }
}

// FUNCTION: SWBF2 0x006e5d30
void ShadowData::Read(PblFileChunk& chunk)
{
    PblBox box;
    box._vMin = PblVector3(FLT_MAX, FLT_MAX, FLT_MAX);
    box._vMax = PblVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    PblVector3* pVertices = 0;
    ShadowEdge* pEdges = 0;
    unsigned char* puiSkinData = 0;
    unsigned int uiSkinNumBones = 0;
    PblVector3* pSkinnedVertices = 0;

    PblTraceP("Model has shadow data\n");
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* pFile = child._pFile;
        switch (child._iId)
        {
        case 0x4E494B53:
        {
            unsigned int uiNumVertices = pFile->Read32();
            uiSkinNumBones = pFile->Read32();
            int iSize = (2 * uiSkinNumBones - 1) * uiNumVertices;
            puiSkinData = new unsigned char[iSize];
            pFile->ReadData(puiSkinData, iSize);
            pSkinnedVertices = new PblVector3[uiNumVertices];
            break;
        }
        case 0x49444853:
            for (unsigned short i = 0; i < _edgeCount; ++i)
            {
                pEdges[i].vertex = pFile->Read16();
                pEdges[i].next = pFile->Read16();
                pEdges[i].twin = pFile->Read16();
                pEdges[i].mark = 0;
                pEdges[i].visible = 0;
                pFile->Read16();
            }
            break;
        case 0x4F464E49:
            _iNumVertices = (unsigned short)pFile->Read32();
            _edgeCount = (unsigned short)((unsigned short)pFile->Read32() >> 2);
            pVertices = new PblVector3[_iNumVertices];
            pEdges = new ShadowEdge[_edgeCount];
            break;
        case 0x56444853:
            for (unsigned short i = 0; i < _iNumVertices; ++i)
            {
                pVertices[i].x = pFile->ReadFloat();
                pVertices[i].y = pFile->ReadFloat();
                pVertices[i].z = pFile->ReadFloat();
                box.AddPoint(pVertices[i]);
            }
            break;
        }
    }

    const float dx = box._vMax.x - box._vMin.x;
    const float dy = box._vMax.y - box._vMin.y;
    const float dz = box._vMax.z - box._vMin.z;
    _pSkinnedVertices = pSkinnedVertices;
    _puiSkinData = puiSkinData;
    _uiSkinNumBones = uiSkinNumBones;
    _pVertices = pVertices;
    _edgeList = pEdges;
    _faceCount = 0;
    _triCount = 0;
    _facePlanes = 0;
    _facePlanesEntry = 0;
    _fSizeInvSqr = 1.0f / (dx * dx + dy * dy + dz * dz);

    for (unsigned short i = 0; i < _edgeCount; ++i)
    {
        if (_edgeList[i].mark)
        {
            continue;
        }
        int iCount = 0;
        unsigned short e = i;
        do
        {
            ++iCount;
            _edgeList[e].mark = 1;
            e = _edgeList[e].next;
        } while (e != i);
        ++_faceCount;
        _triCount += iCount - 2;
    }

    _facePlanes = new PblVector4[_faceCount];
    _facePlanesEntry = new unsigned short[_faceCount];

    int iFace = 0;
    for (unsigned short i = 0; i < _edgeCount; ++i)
    {
        if (!_edgeList[i].mark)
        {
            continue;
        }
        unsigned short uiNext = _edgeList[i].next;
        float fBest = -1.0f;
        unsigned short a = _edgeList[i].vertex;
        unsigned short e = _edgeList[uiNext].next;
        unsigned short b = _edgeList[uiNext].vertex;
        unsigned short c = _edgeList[e].vertex;
        _edgeList[uiNext].mark = 0;
        _edgeList[i].mark = 0;
        PblVector3 vBest;
        do
        {
            const float d1x = pVertices[a].x - pVertices[b].x;
            const float d1y = pVertices[a].y - pVertices[b].y;
            const float d1z = pVertices[a].z - pVertices[b].z;
            const float d2x = pVertices[c].x - pVertices[b].x;
            const float d2y = pVertices[c].y - pVertices[b].y;
            const float d2z = pVertices[c].z - pVertices[b].z;
            PblVector3 n;
            n.x = d2y * d1z - d2z * d1y;
            n.y = d2z * d1x - d1z * d2x;
            n.z = d1y * d2x - d2y * d1x;
            const float fLenSq = n.x * n.x + n.y * n.y + n.z * n.z;
            if (fLenSq > fBest)
            {
                fBest = fLenSq;
                vBest = n;
            }
            _edgeList[e].mark = 0;
            e = _edgeList[e].next;
            a = b;
            b = c;
            c = _edgeList[e].vertex;
        } while (e != i);

        PblVector4 plane;
        plane.x = _fSizeInvSqr * vBest.x;
        plane.y = vBest.y * _fSizeInvSqr;
        plane.z = vBest.z * _fSizeInvSqr;
        plane.w = -(pVertices[a].x * plane.x + pVertices[a].y * plane.y + pVertices[a].z * plane.z);
        _facePlanes[iFace] = plane;
        _facePlanesEntry[iFace] = i;
        ++iFace;
    }

    _silhouetteAllocated = (unsigned short)((_edgeCount + 3) / 4 + 1);
    _silhouette = new SilhouetteVertex[_silhouetteAllocated];
    _iShadowMem += _silhouetteAllocated * sizeof(SilhouetteVertex);
    _iShadowSilhouetteVertexCount += _silhouetteAllocated;
}

// FUNCTION: SWBF2 0x006e63f0
void RedStencilShadow::ReadCPUShadowData(PblFileChunk& chunk, ShadowSegment* pSegment)
{
    unsigned int uiFlags = 0;
    ShadowData* pShadowData = 0;
    int iSkinType = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x41544144:
            pShadowData = new ShadowData;
            pShadowData->Read(child);
            uiFlags = 2;
            break;
        case 0x4F464E49:
            iSkinType = child._pFile->Read32();
            break;
        }
    }
    pSegment->m_pShadowRenderable = new ShadowRenderable(pShadowData);
    pSegment->m_hardSkin = iSkinType == 1;
    pSegment->m_pCPUShadowShader = pcRedShader::Create(
        PblHash("stencilshadow"), uiFlags, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    ((pcStencilShadowShader*)pSegment->m_pCPUShadowShader)
        ->SetMode(pcStencilShadowShader::MODE_CPU_EXTRUSION);
}

// FUNCTION: SWBF2 0x006e64f0
void RedStencilShadow::ReadGPUShadowData(PblFileChunk& chunk, ShadowSegment* pSegment)
{
    unsigned int uiVertexFormat = 0;
    pcRedVertexBuffer* pVertexBuffer = 0;
    pcRedIndexBuffer* pIndexBuffer = 0;
    int iSkinType = 0;
    int iNormalMode = 0;
    unsigned int uiPrimType = 0;
    unsigned int uiPrimCount = 0;
    unsigned int uiNumVertices = 0;
    unsigned int uiStartVertex = 0;
    unsigned int uiStartIndex = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* pFile = child._pFile;
        switch (child._iId)
        {
        case 0x4F464E49:
            iSkinType = pFile->Read32();
            iNormalMode = pFile->Read32();
            if (iSkinType && iNormalMode != 2)
            {
                return;
            }
            uiPrimType = pFile->Read32();
            uiNumVertices = pFile->Read32();
            uiPrimCount = pFile->Read32();
            break;
        case 0x46554256:
        {
            unsigned int uiCount = pFile->Read32();
            unsigned int uiSize = pFile->Read32();
            uiVertexFormat = pFile->Read32();
            pVertexBuffer = pcRedVertexBuffer::Create(uiCount, uiSize, true, true);
            void* pData = pVertexBuffer->Lock(uiCount, uiStartVertex);
            pFile->ReadData(pData, uiSize * uiCount);
            pVertexBuffer->Unlock();
            break;
        }
        case 0x46554249:
        {
            unsigned int uiNumIndices = pFile->Read32();
            pIndexBuffer = pcRedIndexBuffer::Create(uiNumIndices, true, true);
            void* pData = pIndexBuffer->Lock(uiNumIndices, uiStartIndex);
            pFile->ReadData(pData, uiNumIndices * 2);
            pIndexBuffer->Unlock();
            break;
        }
        }
    }

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        (D3DPRIMITIVETYPE)uiPrimType, uiPrimCount, uiVertexFormat, uiNumVertices,
        uiStartVertex, pVertexBuffer, uiStartIndex, pIndexBuffer);
    pSegment->m_pPrimitive = pPrimitive;
    pPrimitive->m_bbox = m_pModel->_BoundingBox;
    pVertexBuffer->Release();
    if (pIndexBuffer)
    {
        pIndexBuffer->Release();
    }
    pSegment->m_hardSkin = iSkinType == 1;
    pSegment->m_pGPUShadowShader = pcRedShader::Create(
        PblHash("stencilshadow"), uiVertexFormat, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    pcStencilShadowShader* pShader = (pcStencilShadowShader*)pSegment->m_pGPUShadowShader;
    if (iSkinType == 0)
    {
        pShader->SetMode(pcStencilShadowShader::MODE_GPU_EXTRUSION_VERTEXNORMAL_UNSKINNED);
    }
    else if (iSkinType == 1)
    {
        if (iNormalMode == 1)
        {
            pShader->SetMode(pcStencilShadowShader::MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDHARD);
        }
        else if (iNormalMode == 2)
        {
            pShader->SetMode(pcStencilShadowShader::MODE_GPU_EXTRUSION_GENERATENORMAL_SKINNEDHARD);
        }
    }
    else if (iSkinType == 2)
    {
        if (iNormalMode == 1)
        {
            pShader->SetMode(pcStencilShadowShader::MODE_GPU_EXTRUSION_FACENORMAL_SKINNEDSOFT);
        }
        else if (iNormalMode == 2)
        {
            pShader->SetMode(pcStencilShadowShader::MODE_GPU_EXTRUSION_GENERATENORMAL_SKINNEDSOFT);
        }
    }
}

static inline void _AddTestPlane(bool bFlip, const D3DXVECTOR3& vEdge, const D3DXVECTOR3& vPoint)
{
    const D3DXVECTOR3& vProjection = *(const D3DXVECTOR3*)&RedStencilShadow::s_vProjection;
    D3DXVECTOR3 vCross;
    if (bFlip)
    {
        D3DXVec3Cross(&vCross, &vProjection, &vEdge);
    }
    else
    {
        D3DXVec3Cross(&vCross, &vEdge, &vProjection);
    }
    D3DXVECTOR3 vNormal;
    D3DXVec3Normalize(&vNormal, &vCross);

    PblVector4& plane = RedStencilShadow::s_aTestPlane[RedStencilShadow::s_iNumTestPlanes++];
    plane.x = vNormal.x;
    plane.y = vNormal.y;
    plane.z = vNormal.z;
    plane.w = -D3DXVec3Dot(&vPoint, &vNormal);
}

// FUNCTION: SWBF2 0x006e6f40
void RedStencilShadow::SetVisibilityTestCamera(const RedCamera* pCamera)
{
    s_vProjection = RedLight::GetGlobalDirectionalLight(0)->m_vDirection;

    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    s_fNearPlaneRadius = (float)sqrt(fWidth * fWidth + fHeight * fHeight);

    const PblMatrix& mCamera = pCamera->GetMatrix();
    const D3DXVECTOR3& vRight = *(const D3DXVECTOR3*)mCamera.m[0];
    const D3DXVECTOR3& vUp = *(const D3DXVECTOR3*)mCamera.m[1];
    const D3DXVECTOR3& vFwd = *(const D3DXVECTOR3*)mCamera.m[2];
    const D3DXVECTOR3& vPos = *(const D3DXVECTOR3*)mCamera.m[3];

    *(D3DXVECTOR3*)&s_vNearPlaneCenter = vPos - vFwd * fNear;

    D3DXVECTOR3 vLocal;
    D3DXVec3TransformNormal(&vLocal, (const D3DXVECTOR3*)&s_vProjection,
                            (const D3DXMATRIX*)&pCamera->GetMatrixInverse());

    float fScale = 0.5f / fNear;
    float fHalfWidth = fScale * fWidth;
    float fHalfHeight = fScale * fHeight;

    bool bFar = vLocal.z > 0.0f;
    bool bRight = vLocal.z * fHalfWidth + vLocal.x < 0.0f;
    bool bLeft = vLocal.z * fHalfWidth - vLocal.x < 0.0f;
    bool bTop = vLocal.y + vLocal.z * fHalfHeight < 0.0f;
    bool bBottom = vLocal.z * fHalfHeight - vLocal.y < 0.0f;

    s_iNumTestPlanes = 0;

    if (bTop != bRight)
    {
        _AddTestPlane(bTop, vRight * fHalfWidth + vUp * fHalfHeight - vFwd, vPos);
    }
    if (bRight != bBottom)
    {
        _AddTestPlane(bRight, vRight * fHalfWidth - vUp * fHalfHeight - vFwd, vPos);
    }
    if (bBottom != bLeft && s_iNumTestPlanes < 2)
    {
        _AddTestPlane(bBottom, -vRight * fHalfWidth - vUp * fHalfHeight - vFwd, vPos);
    }
    if (bLeft != bTop && s_iNumTestPlanes < 2)
    {
        _AddTestPlane(bLeft, -vRight * fHalfWidth + vUp * fHalfHeight - vFwd, vPos);
    }

    if (bTop != bFar)
    {
        _AddTestPlane(bTop, -vRight, vPos + vUp * (fHalfHeight * fFar) - vFwd * fFar);
    }
    if (bRight != bFar)
    {
        _AddTestPlane(bRight, vUp, vPos + vRight * (fHalfWidth * fFar) - vFwd * fFar);
    }
    if (bBottom != bFar)
    {
        _AddTestPlane(bBottom, vRight, vPos - vUp * (fHalfHeight * fFar) - vFwd * fFar);
    }
    if (bLeft != bFar && s_iNumTestPlanes < 5)
    {
        _AddTestPlane(bLeft, -vUp, vPos - vRight * (fHalfWidth * fFar) - vFwd * fFar);
    }
}

// FUNCTION: SWBF2 0x006e8270
static const PblMatrix& _GetSkinMatrix(unsigned int uiNumBones, const unsigned char* puiWeights,
                                const unsigned char* puiIndices, const PblMatrix* pMatrices)
{
    static PblMatrix s_mSkin = *(const PblMatrix*)&_IdentityMatrix;

    if (uiNumBones == 1 || puiWeights[0] == 0xFF)
    {
        return pMatrices[puiIndices[0]];
    }

    unsigned int i = 1;
    float fWeight = puiWeights[0] * kInv255;
    const PblMatrix* pBone = &pMatrices[puiIndices[0]];
    for (int r = 0; r < 4; ++r)
    {
        s_mSkin.m[r][0] = pBone->m[r][0] * fWeight;
        s_mSkin.m[r][1] = pBone->m[r][1] * fWeight;
        s_mSkin.m[r][2] = pBone->m[r][2] * fWeight;
    }

    for (unsigned int uiTotal = puiWeights[0]; uiTotal < 0xFF; uiTotal += puiWeights[i - 1])
    {
        pBone = &pMatrices[puiIndices[i]];
        fWeight = puiWeights[i] * kInv255;
        if (i >= uiNumBones - 1)
        {
            const float fScale = (1.0f - fWeight) / (uiTotal * kInv255);
            for (int r = 0; r < 4; ++r)
            {
                s_mSkin.m[r][0] = s_mSkin.m[r][0] * fScale + pBone->m[r][0] * fWeight;
                s_mSkin.m[r][1] = s_mSkin.m[r][1] * fScale + pBone->m[r][1] * fWeight;
                s_mSkin.m[r][2] = s_mSkin.m[r][2] * fScale + pBone->m[r][2] * fWeight;
            }
            break;
        }
        ++i;
        for (int r = 0; r < 4; ++r)
        {
            s_mSkin.m[r][0] += pBone->m[r][0] * fWeight;
            s_mSkin.m[r][1] += pBone->m[r][1] * fWeight;
            s_mSkin.m[r][2] += pBone->m[r][2] * fWeight;
        }
    }
    return s_mSkin;
}

// SYNTHETIC: SWBF2 0x00408aa0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00408ac0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''
