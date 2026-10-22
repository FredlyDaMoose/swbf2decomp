#include "pcRedTerrain.h"

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcTerrainShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcTerrainShader2.h"
#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Util/RedCommandConsole.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "pcFLRender.h"

#include <math.h>

// FUNCTION: SWBF2 0x006d5d60
void RedTerrain::SetBumpDetailTexture(unsigned int uiHash, float fScale, float fStrength)
{
    _TerrainRenderable.SetBumpDetailTexture(uiHash, fScale, fStrength);
}

// FUNCTION: SWBF2 0x006d5d90
void RedTerrain::SetBumpSpecularTexture(unsigned int, float)
{
}

// FUNCTION: SWBF2 0x006d60e0
void TerrainRenderable::_RenderLowRes(unsigned int uiFlags) const
{
    if (m_fFarTerrainMaxDistance <= 0.0f)
    {
        return;
    }

    const RedCamera* pCurrent = RedRenderer::GetCamera();
    RedCamera camera(*pCurrent);
    float fNear, fFar, fWidth, fHeight;
    pCurrent->GetFrustum(fNear, fFar, fWidth, fHeight);
    float fZoom = pCurrent->GetZoom();
    camera.SetZoom(fZoom);
    float fClipNear = uiFlags & 0x200000 ? FLRenderer::GetNearPlane()
                                       : FLRenderer::GetNearSceneFadeStart();
    camera.SetNearFarPlane(fClipNear * fZoom, m_fFarTerrainMaxDistance);

    int dimension = m_iLowResPatchesPerDimension;
    bool reverseX = -pCurrent->GetMatrix().m[2][0] < 0.0f;
    bool reverseZ = -pCurrent->GetMatrix().m[2][2] < 0.0f;
    int xStart = reverseX ? dimension - 1 : 0;
    int xEnd = reverseX ? -1 : dimension;
    int xStep = reverseX ? -1 : 1;
    int zStart = reverseZ ? dimension - 1 : 0;
    int zEnd = reverseZ ? -1 : dimension;
    int zStep = reverseZ ? -1 : 1;
    for (int z = zStart; z != zEnd; z += zStep)
    {
        for (int x = xStart; x != xEnd; x += xStep)
        {
            const LowResPatch& patch = m_aLowResPatches[x + z * dimension];
            if (!patch.pPrim)
            {
                continue;
            }
            const PblVector3& vMin = patch.bbox._vMin;
            const PblVector3& vMax = patch.bbox._vMax;
            PblVector3 vCenter((vMax.x + vMin.x) * 0.5f,
                               (vMax.y + vMin.y) * 0.5f,
                               (vMax.z + vMin.z) * 0.5f);
            PblVector3 vExtent((vMax.x - vMin.x) * 0.5f,
                               (vMax.y - vMin.y) * 0.5f,
                               (vMax.z - vMin.z) * 0.5f);
            if (camera.ClassifyAABox(vCenter, vExtent) == RedCamera::VISIBILITY_NONE)
            {
                continue;
            }
            void* pShader = pcTerrainShader2::IsEnabled() ? (void*)m_pFarShader2
                                                           : (void*)m_pFarShader;
            RedRenderer::pcRenderPrimitive(patch.pPrim,
                                           (pcRedShader*)pShader,
                                           gMatrixIdentity,
                                           RedColor::WHITE,
                                           uiFlags | 0x1000000, 0, 0, 0);
        }
    }
}

// GLOBAL: SWBF2 0x007df330
bool bTerrainEnable = true;

// GLOBAL: SWBF2 0x007df331
bool bForceFullDetail = true;

// SYNTHETIC: SWBF2 0x004081c0
// `dynamic initializer for 'cvForceFullDetail''
static RedCommandConsole cvForceFullDetail("terrain.ForceFullDetail", MultiVar(&bForceFullDetail));

// GLOBAL: SWBF2 0x00998240
// SYNTHETIC: SWBF2 0x004081a0
// `dynamic initializer for '_TerrainRenderable''
// SYNTHETIC: SWBF2 0x00769980
// `dynamic atexit destructor for '_TerrainRenderable''
TerrainRenderable _TerrainRenderable;

// FUNCTION: SWBF2 0x006d34f0
TerrainRenderable::TerrainRenderable()
{
    Init();
}

// FUNCTION: SWBF2 0x006d3510
TerrainRenderable::~TerrainRenderable()
{
    Deinit();
}

// SYNTHETIC: SWBF2 0x006d3570
// TerrainRenderable::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006d35a0
void RedTerrain::Activate()
{
    if (RedTerrain::_bEnable)
    {
        _TerrainRenderable._SetQuality();
    }
}

// FUNCTION: SWBF2 0x006d6480
void TerrainRenderable::_SetQuality()
{
    QualityT eQuality = RedRenderer::pcGetRenderOptions().terrainQuality;
    bool bBumpMapping = RedRenderer::pcGetRenderOptions().bumpMapping;
    if (m_eQuality != eQuality || m_bEnableBumpMapping != bBumpMapping)
    {
        m_eQuality = eQuality;
        m_bEnableBumpMapping = bBumpMapping;
        RedTerrain::RenderDetail = eQuality > QUALITY_LOW;
        if (RedRenderer::pcGetRenderCaps().pipeline != PIPELINE_FIXED && m_eQuality == QUALITY_HIGH)
        {
            _InitPixelLighting(true);
        }
        else
        {
            _InitPixelLighting(false);
        }
    }
}

// FUNCTION: SWBF2 0x006d3620
void RedTerrain::Deactivate()
{
    _TerrainRenderable.Deinit();
    _TerrainRenderable.Init();
}

// FUNCTION: SWBF2 0x006d3640
void TerrainRenderable::Deactivate()
{
}

// FUNCTION: SWBF2 0x006d3650
void TerrainRenderable::Deinit()
{
    if (m_aPatches)
    {
        for (int i = 0; i < RedTerrain::_iNumPatches; ++i)
        {
            Patch& patch = m_aPatches[i];
            for (unsigned int j = 0; j < patch.uiNumPasses; ++j)
            {
                if (patch.apPrim[j])
                {
                    patch.apPrim[j]->Release();
                }
                if (patch.apShader[j])
                {
                    patch.apShader[j]->Release();
                }
                if (patch.apShader2[j])
                {
                    patch.apShader2[j]->Release();
                }
            }
            delete[] patch.apPrim;
            delete[] patch.apShader;
            delete[] patch.apShader2;
        }
        delete[] m_aPatches;
        m_aPatches = 0;
    }
    if (m_aLowResPatches)
    {
        int iNum = m_iLowResPatchesPerDimension * m_iLowResPatchesPerDimension;
        for (int i = 0; i < iNum; ++i)
        {
            if (m_aLowResPatches[i].pPrim)
            {
                m_aLowResPatches[i].pPrim->Release();
            }
        }
        delete[] m_aLowResPatches;
        m_aLowResPatches = 0;
    }
    if (m_pIBDefault)
    {
        m_pIBDefault->Release();
        m_pIBDefault = 0;
    }
    if (m_pFarShader)
    {
        m_pFarShader->Release();
        m_pFarShader = 0;
    }
    if (m_pFarShader2)
    {
        m_pFarShader2->Release();
        m_pFarShader2 = 0;
    }
    if (m_pPrePixelLightingShader)
    {
        m_pPrePixelLightingShader->Release();
        m_pPrePixelLightingShader = 0;
    }
    if (m_pPrePixelLightingShader2)
    {
        m_pPrePixelLightingShader2->Release();
        m_pPrePixelLightingShader2 = 0;
    }
}

// FUNCTION: SWBF2 0x006d3910
void TerrainRenderable::Init()
{
    m_aPatches = 0;
    m_aLowResPatches = 0;
    m_pIBDefault = 0;
    m_pFarShader = 0;
    m_pPrePixelLightingShader = 0;
    m_pPrePixelLightingShader2 = 0;
    m_fDetailMaxDistance = 32.0f;
    m_fLowResMapSize = 0.0f;
    m_iLowResPatchSize = 0;
    m_iLowResPatchesPerDimension = 0;
    m_fLowResPointsPerMeter = 0.0f;
    m_bEnablePixelLighting = false;
    m_bBumpTextureAvailable = false;
    m_bEnableFarDetail = false;
    m_fFarTerrainMaxDistance = 0.0f;
    m_uiBumpLuminosityTextureHash = 0;
    m_fBumpLuminosityScale = 1.0f;
    m_fStaticBumpFactor = 0.5f;
    m_eQuality = (QualityT)-1;
}

// FUNCTION: SWBF2 0x006d4ff0
void RedTerrain::Render(unsigned int uiFlags)
{
    if (bTerrainEnable)
    {
        _TerrainRenderable.Render((int)uiFlags);
    }
}

// FUNCTION: SWBF2 0x006d5010
void TerrainRenderable::Render(int value)
{
    unsigned int uiFlags = (unsigned int)value;
    if (!bTerrainEnable || !RedTerrain::_bEnable || !m_aPatches)
    {
        return;
    }

    _SetQuality();
    const RedCamera* pCamera = RedRenderer::GetCamera();
    if (!RedRenderer::pcIsNearScene() || (uiFlags & 0x20000))
    {
        if (!bForceFullDetail && !pcTerrainShader2::IsEnabled())
        {
            _RenderLowRes(uiFlags);
            return;
        }
        RedCamera farCamera(*pCamera);
        float fZoom = pCamera->GetZoom();
        float fFadeStart = (uiFlags & 0x200000) ? FLRenderer::GetNearPlane()
                                                : FLRenderer::GetNearSceneFadeStart();
        farCamera.SetNearFarPlane(fFadeStart * fZoom, m_fFarTerrainMaxDistance);
    }

    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    const PblMatrix& mCamera = pCamera->GetMatrix();
    const PblVector3& vRight = mCamera.GetXAxis();
    const PblVector3& vUp = mCamera.GetYAxis();
    const PblVector3& vAt = mCamera.GetZAxis();
    PblVector3 vPos(mCamera.m[3][0], mCamera.m[3][1], mCamera.m[3][2]);

    float fHalfWidth = fFar * 0.5f * fWidth / fNear;
    float fHalfHeight = fFar * 0.5f * fHeight / fNear;

    PblVector3 vCorner(vPos.x - vAt.x * fFar + vRight.x * fHalfWidth + vUp.x * fHalfHeight,
                       vPos.y - vAt.y * fFar + vRight.y * fHalfWidth + vUp.y * fHalfHeight,
                       vPos.z - vAt.z * fFar + vRight.z * fHalfWidth + vUp.z * fHalfHeight);

    PblVector3 avNormal[5];
    float afDist[5];
    avNormal[0] = PblVector3(-vAt.x, -vAt.y, -vAt.z);
    avNormal[1] = PblVector3(vAt.x * fHalfWidth + vRight.x * fFar, vAt.y * fHalfWidth + vRight.y * fFar,
                             vAt.z * fHalfWidth + vRight.z * fFar);
    avNormal[2] = PblVector3(vAt.x * fHalfWidth - vRight.x * fFar, vAt.y * fHalfWidth - vRight.y * fFar,
                             vAt.z * fHalfWidth - vRight.z * fFar);
    avNormal[3] = PblVector3(vAt.x * fHalfHeight + vUp.x * fFar, vAt.y * fHalfHeight + vUp.y * fFar,
                             vAt.z * fHalfHeight + vUp.z * fFar);
    avNormal[4] = PblVector3(vAt.x * fHalfHeight - vUp.x * fFar, vAt.y * fHalfHeight - vUp.y * fFar,
                             vAt.z * fHalfHeight - vUp.z * fFar);
    afDist[0] = avNormal[0].x * vCorner.x + avNormal[0].y * vCorner.y + avNormal[0].z * vCorner.z;
    for (int i = 1; i < 5; ++i)
    {
        afDist[i] = avNormal[i].x * vPos.x + avNormal[i].y * vPos.y + avNormal[i].z * vPos.z;
    }

    const PblBox& box = pCamera->GetCameraBox();
    int iMinX = _CalcPatchX(box._vMin.x);
    int iMaxX = _CalcPatchX(box._vMax.x);
    int iMinZ = _CalcPatchZ(box._vMin.z);
    int iMaxZ = _CalcPatchZ(box._vMax.z);
    const int iNumPatches = RedTerrain::_iPatchesPerDimension;
    if (iMaxX < 0 || iMinX >= iNumPatches || iMaxZ < 0 || iMinZ >= iNumPatches)
    {
        return;
    }
    if (iMinX < 0)
    {
        iMinX = 0;
    }
    if (iMaxX >= iNumPatches)
    {
        iMaxX = iNumPatches - 1;
    }
    if (iMinZ < 0)
    {
        iMinZ = 0;
    }
    if (iMaxZ >= iNumPatches)
    {
        iMaxZ = iNumPatches - 1;
    }

    int iStepX = 1;
    int iStepZ = 1;
    if (-vAt.x < 0.0f)
    {
        int iTemp = iMinX;
        iMinX = iMaxX;
        iMaxX = iTemp;
        iStepX = -1;
    }
    iMaxX += iStepX;
    if (-vAt.z < 0.0f)
    {
        int iTemp = iMinZ;
        iMinZ = iMaxZ;
        iMaxZ = iTemp;
        iStepZ = -1;
    }
    iMaxZ += iStepZ;

    if (RedShadowMap::IsShadowMapEnabled() && !(uiFlags & 0x10))
    {
        if (!pcTerrainShader2::IsEnabled())
        {
            pcTerrainShader::s_uiOptions |= 0x10000;
        }
        pcTerrainShader2::_InitShadowMap(RedShadowMap::GetShadowMapTexture(),
                                         RedShadowMap::GetShadowMapWrapMode(),
                                         RedShadowMap::_vShadowMapTransformU,
                                         RedShadowMap::_vShadowMapTransformV,
                                         RedShadowMap::_vShadowMapTransformW);
    }
    else if (!pcTerrainShader2::IsEnabled())
    {
        pcTerrainShader::s_uiOptions &= ~0x10000;
    }

    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
    {
        uiFlags |= 0x800;
    }
    if (!m_bEnablePixelLighting)
    {
        RedRenderer::SetLightingState(*RedLight::GetLightingState(true));
    }

    float fHalfPatch = (float)RedTerrain::_iPatchPointsPerDimension * RedTerrain::_fMetersPerPoint * 0.5f;
    float afExtentZ[5];
    float afExtentX[5];
    for (int i = 0; i < 5; ++i)
    {
        afExtentZ[i] = (float)fabs(avNormal[i].z * fHalfPatch);
        afExtentX[i] = (float)fabs(avNormal[i].x * fHalfPatch);
    }

    for (int iZ = iMinZ; iZ != iMaxZ; iZ += iStepZ)
    {
        float fZ = (float)((RedTerrain::_iPatchPointsPerDimension >> 1) +
                           (iZ << RedTerrain::_iPatchPointsPerDimensionLog2)) * RedTerrain::_fMetersPerPoint -
                   RedTerrain::_fTerrainCenterZ;
        float afRowDist[5];
        for (int i = 0; i < 5; ++i)
        {
            afRowDist[i] = afDist[i] - (avNormal[i].z * fZ - afExtentZ[i] - afExtentX[i]);
        }

        int iLastPlane = 0;
        for (int iX = iMinX; iX != iMaxX; iX += iStepX)
        {
            Patch& patch = m_aPatches[iZ * iNumPatches + iX];
            float fX = (float)((RedTerrain::_iPatchPointsPerDimension >> 1) +
                               (iX << RedTerrain::_iPatchPointsPerDimensionLog2)) * RedTerrain::_fMetersPerPoint -
                       RedTerrain::_fTerrainCenterX;
            if (!patch.apPrim[0])
            {
                continue;
            }

            float fCenterY = (float)(patch.iHeightMin + patch.iHeightMax) * 0.5f * RedTerrain::_fHeightScale;
            float fExtentY = (float)(patch.iHeightMax - patch.iHeightMin) * 0.5f * RedTerrain::_fHeightScale;

            const PblVector3& vLast = avNormal[iLastPlane];
            if (vLast.x * fX + vLast.y * fCenterY - (float)fabs(vLast.y) * fExtentY > afRowDist[iLastPlane])
            {
                continue;
            }
            int iPlane;
            for (iPlane = 0; iPlane < 5; ++iPlane)
            {
                const PblVector3& vNormal = avNormal[iPlane];
                if (iPlane != iLastPlane &&
                    vNormal.x * fX + vNormal.y * fCenterY - (float)fabs(vNormal.y) * fExtentY > afRowDist[iPlane])
                {
                    break;
                }
            }
            if (iPlane < 5)
            {
                iLastPlane = iPlane;
                continue;
            }

            if (m_bEnablePixelLighting)
            {
                RedRenderer::SetLightingState(
                    *RedLight::GetLightingState(patch.apPrim[0]->m_bbox, 4, true, true, false));
            }
            patch.apShader[0]->m_uiOptions |= 1;

            if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
            {
                PblMatrix mWorld = gMatrixIdentity;
                const PblBox& bbox = patch.apPrim[0]->m_bbox;
                mWorld.m[3][0] = bbox._vMin.x;
                mWorld.m[3][1] = 0.0f;
                mWorld.m[3][2] = bbox._vMin.z;
                mWorld.m[3][3] = 1.0f;
                for (unsigned int uiPass = 0; uiPass < patch.uiNumPasses; ++uiPass)
                {
                    RedRenderer::pcRenderPrimitive(patch.apPrim[uiPass],
                                                   patch.apShader[uiPass],
                                                   mWorld,
                                                   RedColor::WHITE, uiFlags, 0,
                                                   0, 0);
                }
            }
            else if (pcTerrainShader2::IsEnabled())
            {
                RedRenderer::pcRenderPrimitive(patch.apPrim[0],
                                               patch.apShader2[0],
                                               gMatrixIdentity,
                                               RedColor::WHITE,
                                               uiFlags | 0x1000000, 0, 0, 0);
            }
            else
            {
                if (m_bEnablePixelLighting)
                {
                    RedRenderer::pcRenderPrimitive(patch.apPrim[0],
                                                   m_pPrePixelLightingShader,
                                                   gMatrixIdentity,
                                                   RedColor::WHITE,
                                                   uiFlags | 0x1000000, 0, 0, 0);
                }
                RedRenderer::pcRenderPrimitive(patch.apPrim[0],
                                               patch.apShader[0],
                                               gMatrixIdentity,
                                               RedColor::WHITE,
                                               uiFlags | 0x1000000, 0, 0, 0);
            }
        }
    }
}

// FUNCTION: SWBF2 0x006d5da0
void RedTerrain::SetFarTerrainMaxDistance(float fDist)
{
    _TerrainRenderable.SetFarTerrainMaxDistance(fDist);
}

// FUNCTION: SWBF2 0x006d5dc0
void RedTerrain::SetFarTerrainTexture(unsigned int uiHash, float fA, unsigned int uiB, float fC)
{
    _TerrainRenderable.SetFarTerrainTexture(uiHash, fA, uiB, fC);
}

// FUNCTION: SWBF2 0x006d5df0
void TerrainRenderable::SetFarTerrainTexture(unsigned int uiHash, float fA, unsigned int uiB, float fC)
{
    float fScale = fA / ((float)RedTerrain::_iPointsPerDimension * RedTerrain::_fMetersPerPoint);
    float fOffset = RedTerrain::_fMetersPerPoint * 0.5f * fScale;
    if (!pcTerrainShader2::IsEnabled())
    {
        pcTerrainShader::_InitFarRendering(uiHash, fScale, fScale, fOffset - 0.5f, -0.5f - fOffset);
    }
    float fDetailScale = RedTerrain::_fPointsPerMeter * fC;
    if (!pcTerrainShader2::IsEnabled())
    {
        pcTerrainShader::_InitFarDetail(uiB, fDetailScale, fDetailScale, 0.0f, 0.0f);
    }
    m_bEnableFarDetail = uiB != 0;
    if (m_pFarShader)
    {
        m_pFarShader->m_uiOptions |= 8;
        if (m_bEnableFarDetail)
        {
            m_pFarShader->m_uiOptions |= 1;
        }
        else
        {
            m_pFarShader->m_uiOptions &= ~1;
        }
    }
}

// FUNCTION: SWBF2 0x006d5f10
int TerrainRenderable::_CalcPatchX(float fX)
{
    return (int)((RedTerrain::_fTerrainCenterX + fX) * RedTerrain::_fPointsPerMeter) >>
           RedTerrain::_iPatchPointsPerDimensionLog2;
}

// FUNCTION: SWBF2 0x006d5f40
int TerrainRenderable::_CalcPatchZ(float fZ)
{
    return (int)((RedTerrain::_fTerrainCenterZ + fZ) * RedTerrain::_fPointsPerMeter) >>
           RedTerrain::_iPatchPointsPerDimensionLog2;
}

// FUNCTION: SWBF2 0x006d5f70
void TerrainRenderable::_InitPixelLighting(bool bEnable)
{
    m_bEnablePixelLighting = bEnable;
    if (m_bBumpTextureAvailable && m_bEnableBumpMapping)
    {
        float fScale = m_fBumpLuminosityScale * RedTerrain::_fPointsPerMeter;
        PblVector4 vScale0 = { fScale, 0.0f, 0.0f, 0.0f };
        PblVector4 vScale1 = { 0.0f, 0.0f, fScale, 0.0f };
        pcTerrainShader2::_InitBumpLuminosityPixelLighting(0x785887DA, m_uiBumpLuminosityTextureHash,
                                                           vScale0, vScale1, m_fStaticBumpFactor);
    }
    else
    {
        pcTerrainShader2::_InitPixelLighting(0x785887DA, 0x63DD533C);
    }

    if (!pcTerrainShader2::IsEnabled())
    {
        if (bEnable)
        {
            pcTerrainShader::s_uiOptions |= 0x20000;
        }
        else
        {
            pcTerrainShader::s_uiOptions &= ~0x20000;
        }
    }

    if (!m_pPrePixelLightingShader)
    {
        unsigned int uiFlags = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x5122 : 0x122;
        m_pPrePixelLightingShader2 = (pcTerrainShader2*)pcRedShader::Create(
            PblHash("Terrain2"), uiFlags, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        m_pPrePixelLightingShader = (pcTerrainShader*)pcRedShader::Create(
            PblHash("Terrain"), uiFlags, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    m_pPrePixelLightingShader->m_uiOptions |= 4;
    m_pPrePixelLightingShader->m_uiOptions |= 1;
}

// FUNCTION: SWBF2 0x006d60c0
void RedTerrain::_ReadPatchInfo(PblFileChunk& chunk)
{
    _TerrainRenderable.Read(chunk);
}

// FUNCTION: SWBF2 0x006d6440
void TerrainRenderable::_Reset()
{
    Deinit();
    m_fDetailMaxDistance = 32.0f;
    m_fLowResMapSize = 0.0f;
    m_iLowResPatchSize = 0;
    m_iLowResPatchesPerDimension = 0;
    m_fLowResPointsPerMeter = 0.0f;
    m_bEnablePixelLighting = false;
    m_bEnableFarDetail = false;
    m_eQuality = (QualityT)-1;
}

namespace
{

pcRedIndexBuffer* ReadIndexBuffer(PblFileChunk& chunk, unsigned int uiNumIndices, unsigned int& ruiStartIndex)
{
    pcRedIndexBuffer* pIB = pcRedIndexBuffer::Create(uiNumIndices, true, true);
    chunk._pFile->ReadData(pIB->Lock(uiNumIndices, ruiStartIndex), uiNumIndices * sizeof(unsigned short));
    pIB->Unlock();
    return pIB;
}
}

// FUNCTION: SWBF2 0x006d3990
void TerrainRenderable::Read(PblFileChunk& chunk)
{
    _Reset();
    pcTerrainShader2::_InitMaps((float)RedTerrain::_iPointsPerDimension * RedTerrain::_fMetersPerPoint);

    m_aPatches = new Patch[RedTerrain::_iNumPatches];
    RedTerrain::mTerrainBytesUsed += RedTerrain::_iNumPatches * sizeof(Patch);

    for (int i = 0; i < RedTerrain::_iNumLayers; ++i)
    {
        float s = 1.0f / RedTerrain::_fLayerMetersPerTexture[i];
        PblVector4 vScale0 = { 0.0f, 0.0f, 0.0f, 0.0f };
        PblVector4 vScale1 = { 0.0f, 0.0f, 0.0f, 0.0f };
        switch (RedTerrain::_uiLayerTextureOrientation[i])
        {
        case 1:
            vScale0.x = s;
            vScale1.y = s;
            break;
        case 2:
            vScale0.y = s;
            vScale1.z = s;
            break;
        case 3:
            vScale0.z = s;
            vScale1.x = s;
            break;
        case 4:
            vScale0.y = s;
            vScale1.x = s;
            break;
        case 5:
            vScale0.z = s;
            vScale1.y = s;
            break;
        case 6:
            vScale0.x = -s;
            vScale1.z = s;
            break;
        case 7:
            vScale0.x = -s;
            vScale1.y = s;
            break;
        case 8:
            vScale0.y = -s;
            vScale1.z = s;
            break;
        case 9:
            vScale0.z = -s;
            vScale1.x = s;
            break;
        case 10:
            vScale0.y = -s;
            vScale1.x = s;
            break;
        case 11:
            vScale0.z = -s;
            vScale1.y = s;
            break;
        default:
            vScale0.x = s;
            vScale1.z = s;
            break;
        }
        pcTerrainShader2::_InitLayer(i, RedTerrain::_uiLayerTextureHash[i], vScale0, vScale1);
    }

    m_fDetailMaxDistance = (float)RedTerrain::_iPatchPointsPerDimension * RedTerrain::_fMetersPerPoint;
    if (m_fDetailMaxDistance > 48.0f)
    {
        m_fDetailMaxDistance = 48.0f;
    }
    else if (m_fDetailMaxDistance < 16.0f)
    {
        m_fDetailMaxDistance = 16.0f;
    }
    {
        PblVector4 vScale0 = { RedTerrain::_fPointsPerMeter, 0.0f, 0.0f, 0.0f };
        PblVector4 vScale1 = { 0.0f, 0.0f, RedTerrain::_fPointsPerMeter, 0.0f };
        pcTerrainShader2::_InitDetail(RedTerrain::_uiDetailTextureHash, vScale0, vScale1, m_fDetailMaxDistance);
    }

    int iPatch = 0;
    unsigned int uiDefaultStartIndex = 0;
    unsigned int uiDefaultNumIndices = 0;
    PblBox lowResBox;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();

        if (child._iId == 0x48435450)
        {
            unsigned int auiLayer[4] = { 0, 0, 0, 0 };
            unsigned int auiFormat[3] = { 0, 0, 0 };
            unsigned int auiNumVertices[3] = { 0, 0, 0 };
            unsigned int auiStartVertex[3] = { 0, 0, 0 };
            pcRedVertexBuffer* apVB[3] = { 0, 0, 0 };
            unsigned int auiNumIndices[3] = { 0, 0, 0 };
            pcRedIndexBuffer* apIB[3] = { 0, 0, 0 };
            unsigned int auiStartIndex[3] = { 0, 0, 0 };
            unsigned int uiNumLayers = 0;
            unsigned int uiNumPasses = 0;
            int iNumVB = 0;
            int iNumUncompressedVB = 0;
            int iHeightShift = 0;
            Patch& patch = m_aPatches[iPatch];

            while (child.HasMoreChildren())
            {
                PblFileChunk leaf = child.ReadNextChild();
                if (leaf._iId == 0x4F464E49)
                {
                    uiNumLayers = leaf._pFile->Read8();
                    for (unsigned char j = 0; j < uiNumLayers; ++j)
                    {
                        unsigned char uiLayer = leaf._pFile->Read8();
                        if (uiLayer < 16)
                        {
                            auiLayer[j] = uiLayer;
                        }
                    }
                    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED && uiNumLayers > 2)
                    {
                        uiNumPasses = uiNumLayers - 1;
                    }
                    else
                    {
                        uiNumPasses = 1;
                    }
                    patch.uiNumPasses = uiNumPasses;
                    patch.apPrim = new pcRedPrimitive*[uiNumPasses];
                    RedTerrain::mTerrainBytesUsed += uiNumPasses * sizeof(pcRedPrimitive);
                    patch.apShader = new pcTerrainShader*[uiNumPasses];
                    patch.apShader2 = new pcTerrainShader2*[uiNumPasses];
                    RedTerrain::mTerrainBytesUsed += uiNumPasses * sizeof(pcTerrainShader);
                    patch.iHeightMin = leaf._pFile->Read16();
                    patch.iHeightMax = leaf._pFile->Read16();
                    iHeightShift = leaf._pFile->Read16();
                }
                else if (leaf._iId == 0x46554256)
                {
                    unsigned int uiNumVertices = leaf._pFile->Read32();
                    unsigned int uiVertexSize = leaf._pFile->Read32();
                    unsigned int uiFormat = leaf._pFile->Read32();
                    bool bUncompressed = (uiFormat & 0xF000) == 0;
                    if (bUncompressed)
                    {
                        ++iNumUncompressedVB;
                    }
                    if ((RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED) == bUncompressed)
                    {
                        auiNumVertices[iNumVB] = uiNumVertices;
                        auiFormat[iNumVB] = uiFormat;
                        if (uiNumVertices)
                        {
                            pcRedVertexBuffer* pVB = pcRedVertexBuffer::Create(uiNumVertices, uiVertexSize, true, true);
                            apVB[iNumVB] = pVB;
                            leaf._pFile->ReadData(pVB->Lock(uiNumVertices, auiStartVertex[iNumVB]),
                                                  uiNumVertices * uiVertexSize);
                            pVB->Unlock();
                        }
                        ++iNumVB;
                    }
                }
                else if (leaf._iId == 0x46554249)
                {
                    if (iNumVB == iNumUncompressedVB)
                    {
                        unsigned int uiNumIndices = leaf._pFile->Read32();
                        auiNumIndices[iNumVB - 1] = uiNumIndices;
                        if (uiNumIndices)
                        {
                            apIB[iNumVB - 1] = ReadIndexBuffer(leaf, uiNumIndices, auiStartIndex[iNumVB - 1]);
                        }
                    }
                }
            }

            float fSize = (float)RedTerrain::_iPatchPointsPerDimension * RedTerrain::_fMetersPerPoint;
            float fX = (float)((iPatch & RedTerrain::_iPatchesPerDimensionMask) * RedTerrain::_iPatchPointsPerDimension) *
                           RedTerrain::_fMetersPerPoint -
                       RedTerrain::_fTerrainCenterX;
            float fZ = (float)((iPatch >> RedTerrain::_iPatchesPerDimensionLog2) * RedTerrain::_iPatchPointsPerDimension) *
                           RedTerrain::_fMetersPerPoint -
                       RedTerrain::_fTerrainCenterZ;
            float fHeightMin = (float)patch.iHeightMin;
            PblBox box(fX, fHeightMin * RedTerrain::_fHeightScale, fZ, fX + fSize,
                       (65535.0f / (float)(1 << iHeightShift) + fHeightMin) * RedTerrain::_fHeightScale, fZ + fSize);

            for (unsigned int uiPass = 0; uiPass < uiNumPasses; ++uiPass)
            {
                RedRenderer::pcGetRenderCaps();
                if (!apVB[uiPass])
                {
                    patch.apPrim[uiPass] = 0;
                    patch.apShader[uiPass] = 0;
                    patch.apShader2[uiPass] = 0;
                    continue;
                }

                if (apIB[uiPass])
                {
                    patch.apPrim[uiPass] = pcRedPrimitive::Create(
                        D3DPT_TRIANGLESTRIP, auiNumIndices[uiPass] - 2, auiFormat[uiPass], auiNumVertices[uiPass],
                        auiStartVertex[uiPass], apVB[uiPass], auiStartIndex[uiPass], apIB[uiPass]);
                    apVB[uiPass]->Release();
                    apIB[uiPass]->Release();
                }
                else
                {
                    patch.apPrim[uiPass] = pcRedPrimitive::Create(
                        D3DPT_TRIANGLESTRIP, uiDefaultNumIndices - 2, auiFormat[uiPass], auiNumVertices[uiPass],
                        auiStartVertex[uiPass], apVB[uiPass], uiDefaultStartIndex, m_pIBDefault);
                    apVB[uiPass]->Release();
                }
                patch.apPrim[uiPass]->m_bbox = box;

                if (uiPass)
                {
                    patch.apShader2[uiPass] = (pcTerrainShader2*)pcRedShader::Create(
                        PblHash("Terrain2"), auiFormat[uiPass], &auiLayer[uiPass + 1], 1, 0, RedColor::WHITE,
                        RedColor::BLACK, 32.0f);
                    patch.apShader[uiPass] = (pcTerrainShader*)pcRedShader::Create(
                        PblHash("Terrain"), auiFormat[uiPass], &auiLayer[uiPass + 1], 1, 0, RedColor::WHITE,
                        RedColor::BLACK, 32.0f);
                    pcTerrainShader* pShader = patch.apShader[uiPass];
                    pShader->m_uiOptions = (unsigned char)((pShader->m_uiOptions & 0xCF) | (uiPass == 1 ? 0x10 : 0x20));
                }
                else
                {
                    unsigned int uiNumParams =
                        (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED && uiNumLayers >= 2) ? 2 : uiNumLayers;
                    patch.apShader2[0] = (pcTerrainShader2*)pcRedShader::Create(
                        PblHash("Terrain2"), auiFormat[0], auiLayer, uiNumParams, 0, RedColor::WHITE,
                        RedColor::BLACK, 32.0f);
                }
                unsigned int uiNumParams =
                    (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED && uiNumLayers >= 2) ? 2 : uiNumLayers;
                patch.apShader[0] = (pcTerrainShader*)pcRedShader::Create(
                    PblHash("Terrain"), auiFormat[0], auiLayer, uiNumParams, 0, RedColor::WHITE, RedColor::BLACK,
                    32.0f);
            }
            ++iPatch;
        }
        else if (child._iId == 0x4E4D4F43)
        {
            while (child.HasMoreChildren())
            {
                PblFileChunk leaf = child.ReadNextChild();
                if (leaf._iId == 0x46554249)
                {
                    uiDefaultNumIndices = leaf._pFile->Read32();
                    m_pIBDefault = ReadIndexBuffer(leaf, uiDefaultNumIndices, uiDefaultStartIndex);
                }
            }
        }
        else if (child._iId == 0x52574F4C)
        {
            while (child.HasMoreChildren())
            {
                PblFileChunk sub = child.ReadNextChild();
                if (sub._iId == 0x4F464E49)
                {
                    m_iLowResPatchesPerDimension = sub._pFile->Read32();
                    m_iLowResPatchSize = sub._pFile->Read32();
                    m_fLowResMapSize = sub._pFile->ReadFloat();
                    m_fLowResPointsPerMeter =
                        (float)(m_iLowResPatchSize * m_iLowResPatchesPerDimension) / m_fLowResMapSize;
                    float fMinY = sub._pFile->ReadFloat();
                    float fMaxY = sub._pFile->ReadFloat();
                    lowResBox = PblBox(-RedTerrain::_fTerrainCenterX, fMinY, -RedTerrain::_fTerrainCenterZ,
                                       m_fLowResMapSize - RedTerrain::_fTerrainCenterX, fMaxY,
                                       m_fLowResMapSize - RedTerrain::_fTerrainCenterZ);
                    m_aLowResPatches = new LowResPatch[m_iLowResPatchesPerDimension * m_iLowResPatchesPerDimension];
                    RedTerrain::mTerrainBytesUsed +=
                        m_iLowResPatchesPerDimension * m_iLowResPatchesPerDimension * sizeof(LowResPatch);
                }
                else if (sub._iId == 0x53484350)
                {
                    pcRedIndexBuffer* pSharedIB = 0;
                    unsigned int uiSharedStartIndex = 0;
                    unsigned int uiSharedNumIndices = 0;
                    LowResPatch* pLowRes = m_aLowResPatches;
                    while (sub.HasMoreChildren())
                    {
                        PblFileChunk lp = sub.ReadNextChild();
                        if (lp._iId == 0x4354504C)
                        {
                            unsigned int uiFormat = 0;
                            pcRedIndexBuffer* pIB = 0;
                            unsigned int uiStartVertex = 0;
                            unsigned int uiNumVertices = 0;
                            unsigned int uiStartIndex = 0;
                            pcRedVertexBuffer* pVB = 0;
                            unsigned int uiNumIndices = 0;
                            while (lp.HasMoreChildren())
                            {
                                PblFileChunk leaf = lp.ReadNextChild();
                                if (leaf._iId == 0x46554249)
                                {
                                    uiNumIndices = leaf._pFile->Read32();
                                    if (uiNumIndices)
                                    {
                                        pIB = ReadIndexBuffer(leaf, uiNumIndices, uiStartIndex);
                                    }
                                }
                                else if (leaf._iId == 0x46554256)
                                {
                                    unsigned int uiCount = leaf._pFile->Read32();
                                    unsigned int uiVertexSize = leaf._pFile->Read32();
                                    unsigned int uiVBFormat = leaf._pFile->Read32();
                                    if (pVB)
                                    {
                                        continue;
                                    }
                                    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED &&
                                        (uiVBFormat & 0xF000))
                                    {
                                        continue;
                                    }
                                    uiNumVertices = uiCount;
                                    uiFormat = uiVBFormat;
                                    if (uiCount)
                                    {
                                        pVB = pcRedVertexBuffer::Create(uiCount, uiVertexSize, true, true);
                                        leaf._pFile->ReadData(pVB->Lock(uiCount, uiStartVertex), uiCount * uiVertexSize);
                                        pVB->Unlock();
                                    }
                                }
                                else if (leaf._iId == 0x4F464E49)
                                {
                                    PblBox& bbox = pLowRes->bbox;
                                    bbox._vMin.x = leaf._pFile->ReadFloat();
                                    bbox._vMax.x = leaf._pFile->ReadFloat();
                                    bbox._vMin.y = leaf._pFile->ReadFloat();
                                    bbox._vMax.y = leaf._pFile->ReadFloat();
                                    bbox._vMin.z = leaf._pFile->ReadFloat();
                                    bbox._vMax.z = leaf._pFile->ReadFloat();
                                }
                            }

                            if (pIB)
                            {
                                pLowRes->pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, uiNumIndices - 2, uiFormat,
                                                                        uiNumVertices, uiStartVertex, pVB, uiStartIndex,
                                                                        pIB);
                                pIB->Release();
                            }
                            else
                            {
                                pLowRes->pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, uiSharedNumIndices - 2,
                                                                        uiFormat, uiNumVertices, uiStartVertex, pVB,
                                                                        uiSharedStartIndex, pSharedIB);
                            }
                            pVB->Release();
                            if (pLowRes->pPrim)
                            {
                                pLowRes->pPrim->m_bbox = lowResBox;
                            }
                            ++pLowRes;
                        }
                        else if (lp._iId == 0x4E4D4F43)
                        {
                            while (lp.HasMoreChildren())
                            {
                                PblFileChunk leaf = lp.ReadNextChild();
                                if (leaf._iId == 0x46554249)
                                {
                                    uiSharedNumIndices = leaf._pFile->Read32();
                                    pSharedIB = ReadIndexBuffer(leaf, uiSharedNumIndices, uiSharedStartIndex);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    unsigned int uiFarFlags2 = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x5122 : 0x122;
    m_pFarShader2 = (pcTerrainShader2*)pcRedShader::Create(PblHash("Terrain2"), uiFarFlags2, 0, 0, 0,
                                                           RedColor::WHITE, RedColor::BLACK, 32.0f);
    unsigned int uiFarFlags = RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x1002 : 0x2;
    m_pFarShader = (pcTerrainShader*)pcRedShader::Create(PblHash("Terrain"), uiFarFlags, 0, 0, 0, RedColor::WHITE,
                                                         RedColor::BLACK, 32.0f);
    if (m_pFarShader)
    {
        m_pFarShader->m_uiOptions |= 8;
        if (m_bEnableFarDetail)
        {
            m_pFarShader->m_uiOptions |= 1;
        }
        else
        {
            m_pFarShader->m_uiOptions &= ~1;
        }
    }
}
