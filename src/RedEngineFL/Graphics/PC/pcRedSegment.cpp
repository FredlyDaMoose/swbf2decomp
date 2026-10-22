#include "pcRedSegment.h"
#include "pcRedShader.h"
#include "pcRedPrimitive.h"
#include "pcRedVertexFormat.h"
#include "pcRedRenderer.h"

#include "PebbleFL/Common/PblRef.h"

#include "RedEngineFL/Memory/engineAlloc.h"
#include <string.h>
#include <d3dx9math.h>
#include "pcRedVertexBuffer.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/RedSegment.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "pcRedIndexBuffer.h"

pcShaderSegment::~pcShaderSegment()
{
    Free();
}

// FUNCTION: SWBF2 0x00717b50
RedShadingState* pcShaderSegment::CreateShadingState() const
{
    if (m_pShader)
    {
        return m_pShader->CreateShadingState();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00717b80
void pcShaderSegment::Free()
{
    if (m_pPrimitive)
    {
        PblRef* p = (PblRef*)m_pPrimitive;
        p->Release();
        m_pPrimitive = 0;
    }
    if (m_pShader)
    {
        PblRef* p = (PblRef*)m_pShader;
        p->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00717ca0
unsigned int pcShaderSegment::GetRenderTypeHash() const
{
    return m_pShader->GetRenderTypeHash();
}

// FUNCTION: SWBF2 0x00717cd0
bool pcShaderSegment::IsLit() const
{
    return (m_uiSegmentRenderFlags & 0x10) == 0;
}

// FUNCTION: SWBF2 0x00717d00
bool pcShaderSegment::IsTransparent() const
{
    if (m_pShader && m_pShader->IsTransparent())
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00717d50
bool pcShaderSegment::IsVertexLit() const
{
    if (m_pPrimitive &&
        (m_pPrimitive->m_vertexFormat->m_uiVertexBufferRVF & 0x100))
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00718890
void pcShaderSegment::Render(const PblMatrix* pWorld, const PblMatrix* pBoneMatrices,
                             int iNumBones, unsigned int uiFlags, const RedColor& color,
                             RedShadingState* pState)
{
    uiFlags |= m_uiSegmentRenderFlags;
    if (m_uiNumBones > 0)
    {
        RedRenderer::pcRenderPrimitive(m_pPrimitive, m_pShader, *pWorld, color, uiFlags,
                                       pBoneMatrices, iNumBones, pState);
    }
    else
    {
        RedRenderer::pcRenderPrimitive(m_pPrimitive, m_pShader, *pWorld, color, uiFlags,
                                       0, 0, pState);
    }
}

static const D3DXMATRIXA16 _IdentityMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 0.0f,
                                           0.0f, 0.0f, 0.0f, 1.0f);

static const float kInv255 = 1.0f / 255.0f;

static const PblMatrix& _GetSkinMatrix(unsigned int uiNumBones, const unsigned char* puiWeights,
                                       const unsigned char* puiIndices, const PblMatrix* pMatrices);

inline PblVector3 operator-(const PblVector3& a,
                                 const PblVector3& b)
{
    PblVector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

struct CompressedPos
{
    short x;
    short y;
    short z;
    short w;

    static PblVector3 s_vMin;
    static PblVector3 s_vScale;
    static PblVector3 s_vScaleInv;
};

// GLOBAL: SWBF2 0x007de270

// GLOBAL: SWBF2 0x007de27c

// GLOBAL: SWBF2 0x007f5870

PblVector3 CompressedPos::s_vMin;
PblVector3 CompressedPos::s_vScale;
PblVector3 CompressedPos::s_vScaleInv;

// FUNCTION: SWBF2 0x00717980
pcSoftwareSkinnedShaderSegment::pcSoftwareSkinnedShaderSegment()
    : m_pVertexData(0), m_puiSkinData(0), m_uiSkinNumBones(0),
      m_pIndexBuffer(0), m_PrimType(5), m_uiStartIndex(0),
      m_uiPrimCount(0), m_uiNumVertices(0), m_uiNumIndices(0),
      m_pShader(0), m_uiSegmentRenderFlags(0)
{
}

pcSoftwareSkinnedShaderSegment::~pcSoftwareSkinnedShaderSegment()
{
    Free();
}

// FUNCTION: SWBF2 0x00717b60
RedShadingState* pcSoftwareSkinnedShaderSegment::CreateShadingState() const
{
    if (m_pShader)
    {
        return m_pShader->CreateShadingState();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00717c00
void pcSoftwareSkinnedShaderSegment::Free()
{
    if (m_pVertexData)
    {
        delete[] m_pVertexData;
        m_pVertexData = 0;
    }
    if (m_puiSkinData)
    {
        delete[] m_puiSkinData;
        m_puiSkinData = 0;
    }
    if (m_pIndexBuffer)
    {
        PblRef* p = m_pIndexBuffer;
        p->Release();
        m_pIndexBuffer = 0;
    }
    if (m_pShader)
    {
        PblRef* p = (PblRef*)m_pShader;
        p->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00717cb0
unsigned int pcSoftwareSkinnedShaderSegment::GetRenderTypeHash() const
{
    return m_pShader->GetRenderTypeHash();
}

// FUNCTION: SWBF2 0x00717ce0
bool pcSoftwareSkinnedShaderSegment::IsLit() const
{
    return (m_uiSegmentRenderFlags & 0x10) == 0;
}

// FUNCTION: SWBF2 0x00717d20
bool pcSoftwareSkinnedShaderSegment::IsTransparent() const
{
    if (m_pShader && m_pShader->IsTransparent())
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00717d70
bool pcSoftwareSkinnedShaderSegment::IsVertexLit() const
{
    return (m_uiVertexFormat & 0x100) != 0;
}

// FUNCTION: SWBF2 0x007188d0
void pcSoftwareSkinnedShaderSegment::Render(const PblMatrix* pWorld,
                                            const PblMatrix* pBoneMatrices, int iNumBones,
                                            unsigned int uiFlags, const RedColor& color,
                                            RedShadingState* pState)
{
    uiFlags |= m_uiSegmentRenderFlags;
    pcRedPrimitive* model =
        _CreateSkinnedPrimitive(pBoneMatrices, iNumBones, m_uiSkinNumBones);
    RedRenderer::pcRenderPrimitive(model, m_pShader, *pWorld, color, uiFlags,
                                   pBoneMatrices, iNumBones, pState);
    PblRef* p = (PblRef*)model;
    p->Release();
}

// FUNCTION: SWBF2 0x00718960
pcRedPrimitive*
pcSoftwareSkinnedShaderSegment::_CreateSkinnedPrimitive(
    const PblMatrix* pBoneMatrices, int iNumBones, unsigned int count)
{
    if (count > m_uiSkinNumBones)
    {
        count = m_uiSkinNumBones;
    }

    PblVector3 t0, t1, t2, t3, t4, t5, t6;
    unsigned int stride = m_uiVertexSize >> 2;
    int normalOffset = 0;
    int tangentOffset = 0;
    int binormalOffset = 0;

    int offset;
    if (m_uiVertexFormat & 0x1000)
    {
        CompressedPos::s_vMin = m_bbox._vMin;
        PblVector3 extent =
            (m_bbox._vMax - m_bbox._vMin) * (1.0f / 65535.0f);
        CompressedPos::s_vScale = extent;
        PblVector3 inverse;
        inverse.x = 1.0f / extent.x;
        inverse.y = 1.0f / extent.y;
        inverse.z = 1.0f / extent.z;
        CompressedPos::s_vScaleInv = inverse;
        offset = 2;
    }
    else
    {
        offset = 3;
    }

    if (m_uiVertexFormat & 0x20)
    {
        normalOffset = offset;
        if (m_uiVertexFormat & 0x4000)
        {
            offset += 1;
        }
        else
        {
            offset += 3;
        }
    }
    if (m_uiVertexFormat & 0x40)
    {
        tangentOffset = offset;
        if (m_uiVertexFormat & 0x4000)
        {
            offset += 1;
        }
        else
        {
            offset += 3;
        }
        binormalOffset = offset;
    }

    unsigned int lockOffset = 0;
    pcRedVertexBuffer* buffer =
        pcRedVertexBuffer::Create(m_uiNumVertices, m_uiVertexSize, false, true);
    float* vertex = (float*)buffer->Lock(m_uiNumVertices, lockOffset);
    memcpy(vertex, m_pVertexData, m_uiNumVertices * stride * 4);

    const unsigned char* bones = m_puiSkinData;
    const unsigned char* weights = bones + count - 1;

    switch (m_uiVertexFormat & 0x5060)
    {
    case 0:
        for (unsigned int i = 0; i < m_uiNumVertices; ++i)
        {
            const PblMatrix* matrix = (const PblMatrix*)&_GetSkinMatrix(
                m_uiSkinNumBones, bones, weights, pBoneMatrices);
            PblVector3* position = (PblVector3*)vertex;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&t0, (const D3DXVECTOR3*)position, (const D3DXMATRIX*)matrix);
            *position = t0;
            vertex += stride;
            bones += count * 2 - 1;
            weights += count * 2 - 1;
        }
        break;

    case 0x20:
        for (unsigned int i = 0; i < m_uiNumVertices; ++i)
        {
            const PblMatrix* matrix = (const PblMatrix*)&_GetSkinMatrix(
                m_uiSkinNumBones, bones, weights, pBoneMatrices);
            PblVector3* position = (PblVector3*)vertex;
            PblVector3* normal =
                (PblVector3*)(vertex + normalOffset);
            D3DXVec3TransformCoord((D3DXVECTOR3*)&t6, (const D3DXVECTOR3*)position, (const D3DXMATRIX*)matrix);
            *position = t6;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&t4, (const D3DXVECTOR3*)normal, (const D3DXMATRIX*)matrix);
            *normal = t4;
            vertex += stride;
            bones += count * 2 - 1;
            weights += count * 2 - 1;
        }
        break;

    case 0x60:
        for (unsigned int i = 0; i < m_uiNumVertices; ++i)
        {
            const PblMatrix* matrix = (const PblMatrix*)&_GetSkinMatrix(
                m_uiSkinNumBones, bones, weights, pBoneMatrices);
            PblVector3* position = (PblVector3*)vertex;
            PblVector3* normal =
                (PblVector3*)(vertex + normalOffset);
            PblVector3* tangent =
                (PblVector3*)(vertex + tangentOffset);
            PblVector3* binormal =
                (PblVector3*)(vertex + binormalOffset);
            D3DXVec3TransformCoord((D3DXVECTOR3*)&t3, (const D3DXVECTOR3*)position, (const D3DXMATRIX*)matrix);
            *position = t3;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&t1, (const D3DXVECTOR3*)normal, (const D3DXMATRIX*)matrix);
            *normal = t1;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&t2, (const D3DXVECTOR3*)tangent, (const D3DXMATRIX*)matrix);
            *tangent = t2;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&t5, (const D3DXVECTOR3*)binormal, (const D3DXMATRIX*)matrix);
            *binormal = t5;
            vertex += stride;
            bones += count * 2 - 1;
            weights += count * 2 - 1;
        }
        break;
    }

    buffer->Unlock();
    pcRedPrimitive* model = pcRedPrimitive::Create(
        (D3DPRIMITIVETYPE)m_PrimType, m_uiPrimCount, m_uiVertexFormat,
        m_uiNumVertices, lockOffset, buffer, m_uiStartIndex,
        (pcRedIndexBuffer*)m_pIndexBuffer);
    model->m_bbox = m_bbox;

    PblRef* p = (PblRef*)buffer;
    p->Release();

    return model;
}

// FUNCTION: SWBF2 0x00718da0
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

void __cdecl PblTraceP(const char* Format, ...);

// FUNCTION: SWBF2 0x00717b40
RedShadingState* RedSegment::CreateShadingState() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00717b70
void RedSegment::Free()
{
}

// FUNCTION: SWBF2 0x00717cc0
bool RedSegment::IsLit() const
{
    return true;
}

// FUNCTION: SWBF2 0x00717cf0
bool RedSegment::IsTransparent() const
{
    return false;
}

// FUNCTION: SWBF2 0x00717d40
bool RedSegment::IsVertexLit() const
{
    return false;
}

bool RedModel::pcGetShareBufferEnable()
{
    return pc_shareBuffers;
}

static inline const PblBox& _GetModelBox(const RedModel* pModel)
{
    return pModel->_BoundingBox;
}

// FUNCTION: SWBF2 0x00717d80
RedSegment* __cdecl RedSegment::Read(PblFileChunk& chunk, const RedModel* pModel)
{
    PblBox bbox = _GetModelBox(pModel);
    RedColor diffuse = RedColor::WHITE;
    RedColor specular = RedColor::BLACK;
    bool bHaveVertices = false;
    bool bSoftwareSkinned = false;
    unsigned int uiPrimType = 0;
    unsigned int uiNumVertices = 0;
    unsigned int uiPrimCount = 0;
    unsigned int uiStartVertex = 0;
    unsigned int uiStartIndex = 0;
    unsigned int uiNumIndices = 0;
    unsigned int uiVertexFormat = 0;
    unsigned int uiVertexSize = 0;
    pcRedVertexBuffer* pVertexBuffer = 0;
    pcRedIndexBuffer* pIndexBuffer = 0;
    unsigned int uiNumBones = 0;
    unsigned char* puiBoneIndexMap = 0;
    unsigned int uiBoneIndex = 0;
    void* pVertexData = 0;
    unsigned char* puiSkinData = 0;
    unsigned int uiSkinNumBones = 0;
    unsigned int uiRenderType = 0;
    unsigned int auiTextures[4] = { 0, 0, 0, 0 };
    unsigned int uiMaterialFlags = 0;
    unsigned int uiMaterialName = 0;
    float fSpecularPower = 10.0f;
    RedShaderData shaderData;
    shaderData.uiData0 = 0;
    shaderData.uiData1 = 0;
    shaderData.attachedLightName = 0;
    char acRenderType[256];
    char acName[256];

    const bool bForceFlags = pModel && pModel->_uiNameHash == 0x33B99965;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* pFile = child._pFile;
        switch (child._iId)
        {
        case 0x4C52544D:
        {
            uiMaterialFlags = pFile->Read32();
            if (bForceFlags && (uiMaterialFlags & 4))
            {
                uiMaterialFlags |= 0xC000;
            }
            unsigned int uiColor = pFile->Read32();
            diffuse.b = (unsigned char)(uiColor >> 16);
            diffuse.g = (unsigned char)(uiColor >> 8);
            diffuse.r = (unsigned char)uiColor;
            diffuse.a = (unsigned char)(uiColor >> 24);
            uiColor = pFile->Read32();
            specular.b = (unsigned char)(uiColor >> 16);
            specular.g = (unsigned char)(uiColor >> 8);
            specular.r = (unsigned char)uiColor;
            specular.a = (unsigned char)(uiColor >> 24);
            fSpecularPower = (float)pFile->Read32();
            shaderData.uiData0 = (unsigned char)pFile->Read32();
            shaderData.uiData1 = (unsigned char)pFile->Read32();
            child.ReadString(acName, 256);
            shaderData.attachedLightName = PblHash(acName);
            break;
        }
        case 0x50595452:
            child.ReadString(acRenderType, 256);
            uiRenderType = PblHash(acRenderType);
            break;
        case 0x4D414E54:
        {
            unsigned int uiIndex = pFile->Read32();
            child.ReadString(acName, 256);
            if (uiIndex > 3)
            {
                PblTraceP("ERROR: Model has out of range texture index [%d]!\n", uiIndex);
            }
            else
            {
                auiTextures[uiIndex] = PblHash(acName);
            }
            break;
        }
        case 0x54414456:
        {
            unsigned int uiCount = pFile->Read32();
            unsigned int uiSize = pFile->Read32();
            unsigned int uiFormat = pFile->Read32();
            if (!bHaveVertices &&
                (!(uiFormat & 0xF000) ||
                 RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED))
            {
                uiVertexSize = uiSize;
                uiVertexFormat = uiFormat;
                bSoftwareSkinned = true;
                bHaveVertices = true;
                pVertexData = new unsigned char[uiCount * uiSize];
                pFile->ReadData(pVertexData, uiCount * uiSize);
            }
            break;
        }
        case 0x46554256:
        {
            unsigned int uiCount = pFile->Read32();
            unsigned int uiSize = pFile->Read32();
            unsigned int uiFormat = pFile->Read32();
            if ((uiFormat & 0x1C) &&
                RedRenderer::pcGetRenderCaps().skinning == SKINNING_SOFTWARE)
            {
                break;
            }
            if ((uiFormat & 0x40) &&
                RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
            {
                break;
            }
            const GeometryModeT geometry = RedRenderer::pcGetRenderCaps().geometry;
            if ((uiFormat & 0xF000) ? geometry == GEOMETRYMODE_COMPRESSED
                                    : !bHaveVertices)
            {
                uiVertexSize = uiSize;
                uiVertexFormat = uiFormat;
                bHaveVertices = true;
                pVertexBuffer = pcRedVertexBuffer::Create(
                    uiCount, uiSize, true, RedModel::pcGetShareBufferEnable());
                void* pData = pVertexBuffer->Lock(uiCount, uiStartVertex);
                pFile->ReadData(pData, uiCount * uiSize);
                pVertexBuffer->Unlock();
            }
            break;
        }
        case 0x46554249:
        {
            uiNumIndices = pFile->Read32();
            pIndexBuffer = pcRedIndexBuffer::Create(
                uiNumIndices, true, RedModel::pcGetShareBufferEnable());
            void* pData = pIndexBuffer->Lock(uiNumIndices, uiStartIndex);
            pFile->ReadData(pData, uiNumIndices * 2);
            pIndexBuffer->Unlock();
            break;
        }
        case 0x4F464E49:
            uiPrimType = pFile->Read32();
            uiNumVertices = pFile->Read32();
            uiPrimCount = pFile->Read32();
            break;
        case 0x4E494B53:
        {
            unsigned int uiCount = pFile->Read32();
            uiSkinNumBones = pFile->Read32();
            if (bSoftwareSkinned)
            {
                const unsigned int uiSize = uiCount * (uiSkinNumBones * 2 - 1);
                puiSkinData = new unsigned char[uiSize];
                pFile->ReadData(puiSkinData, uiSize);
            }
            break;
        }
        case 0x50414D42:
            uiNumBones = pFile->Read32();
            if (uiNumBones)
            {
                puiBoneIndexMap = new unsigned char[uiNumBones];
                pFile->ReadData(puiBoneIndexMap, uiNumBones);
            }
            break;
        case 0x4D414E42:
            child.ReadString(acName, 256);
            uiBoneIndex = pModel->GetBoneIndex(acName);
            break;
        case 0x4D414E4D:
            child.ReadString(acName, 256);
            uiMaterialName = PblHash(acName);
            break;
        case 0x584F4242:
            bbox._vMin.x = pFile->ReadFloat();
            bbox._vMin.y = pFile->ReadFloat();
            bbox._vMin.z = pFile->ReadFloat();
            bbox._vMax.x = pFile->ReadFloat();
            bbox._vMax.y = pFile->ReadFloat();
            bbox._vMax.z = pFile->ReadFloat();
            break;
        }
    }

    unsigned int uiRenderFlags = 0;
    if (uiMaterialFlags & 0x1000)
    {
        uiRenderFlags = 0x8000000;
    }
    else if (uiMaterialFlags & 0x2000)
    {
        uiRenderFlags = 0x10000000;
    }
    if (!(uiMaterialFlags & 1))
    {
        uiRenderFlags |= 0x10;
    }

    pcRedShader::Create(uiRenderType, uiVertexFormat, auiTextures, 4,
                        uiMaterialFlags, diffuse, specular, fSpecularPower);
    pcRedShader* pShader =
        pcRedShader::Create(uiRenderType, uiVertexFormat, auiTextures, 4,
                            uiMaterialFlags, diffuse, specular, 32.0f);
    if (!pShader)
    {
        pShader = pcRedShader::Create(0xE68B9C52, uiVertexFormat, auiTextures, 4,
                                      uiMaterialFlags, RedColor::WHITE,
                                      RedColor::BLACK, 32.0f);
    }
    pShader->SetData(shaderData);

    if (pVertexData && puiSkinData)
    {
        pcSoftwareSkinnedShaderSegment* pSegment = new pcSoftwareSkinnedShaderSegment;
        pSegment->m_uiMaterialNameHash = uiMaterialName;
        pSegment->m_uiBoneIndex = uiBoneIndex;
        pSegment->m_uiNumBones = uiNumBones;
        pSegment->m_puiBoneIndexMap = puiBoneIndexMap;
        pSegment->m_pVertexData = (unsigned char*)pVertexData;
        pSegment->m_puiSkinData = puiSkinData;
        pSegment->m_uiVertexSize = uiVertexSize;
        pSegment->m_uiSkinNumBones = uiSkinNumBones;
        pSegment->m_pIndexBuffer = (PblRef*)pIndexBuffer;
        pSegment->mBox = bbox;
        pSegment->m_PrimType = uiPrimType;
        pSegment->m_uiStartIndex = uiStartIndex;
        pSegment->m_uiPrimCount = uiPrimCount;
        pSegment->m_uiNumVertices = uiNumVertices;
        pSegment->m_uiNumIndices = uiNumIndices;
        pSegment->m_uiVertexFormat = uiVertexFormat;
        memcpy(&pSegment->m_bbox, &_GetModelBox(pModel), sizeof(PblBox));
        pSegment->m_uiSegmentRenderFlags = uiRenderFlags;
        pSegment->m_pShader = pShader;
        return pSegment;
    }

    pcShaderSegment* pSegment = new pcShaderSegment;
    pSegment->m_uiMaterialNameHash = uiMaterialName;
    pSegment->m_uiBoneIndex = uiBoneIndex;
    pSegment->m_uiNumBones = uiNumBones;
    pSegment->m_puiBoneIndexMap = puiBoneIndexMap;
    pSegment->mBox = bbox;
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        (D3DPRIMITIVETYPE)uiPrimType, uiPrimCount, uiVertexFormat, uiNumVertices,
        uiStartVertex, pVertexBuffer, uiStartIndex, pIndexBuffer);
    pSegment->m_pPrimitive = pPrimitive;
    pPrimitive->m_bbox = _GetModelBox(pModel);
    pVertexBuffer->Release();
    if (pIndexBuffer)
    {
        pIndexBuffer->Release();
    }
    pSegment->m_uiSegmentRenderFlags = uiRenderFlags;
    pSegment->m_pShader = pShader;
    return pSegment;
}

// FUNCTION: SWBF2 0x00718950
void RedSegment::SetData(const RedShaderData&)
{
}
