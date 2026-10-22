#include "RedWater.h"

#include <math.h>
#include <string.h>
#include <d3dx9math.h>

#include "Renderable.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"

// FUNCTION: SWBF2 0x006d0ac0
void RedWater::UpdateEventCache()
{
    int count = m_numWaterEvents;
    int i = 0;
    while (i < count)
    {
        WaterEvent& event = m_eventCache[i];
        if (event.in_range && m_waterTimeStamp.GetElapsed() <= event.expireTime)
        {
            event.in_range = false;
            ++i;
        }
        else
        {
            --count;
            m_numWaterEvents = count;
            event = m_eventCache[count];
        }
    }
}

// VTABLE: SWBF2 0x00772fa0
class WaterRenderable : public Renderable
{
public:
    WaterRenderable() : Renderable(5) {}
    virtual ~WaterRenderable() {}

    // SYNTHETIC: SWBF2 0x006ce0e0
    // WaterRenderable::`scalar deleting destructor'

    virtual void Render(int value);
    virtual bool IsReflected();
    virtual bool HasReflection();
};

// VTABLE: SWBF2 0x00772fc0
class UnderWaterRenderable : public Renderable
{
public:
    UnderWaterRenderable() : Renderable(9) {}
    virtual ~UnderWaterRenderable() {}

    // SYNTHETIC: SWBF2 0x006ce0b0
    // UnderWaterRenderable::`scalar deleting destructor'

    virtual void Render(int value);
    virtual bool IsReflected();
};

static const float WATER_GRAVITY = 9.81f;
static const float INV_SQRT_2 = 0.70710677f;

static const float WATER_HEIGHT_FREQUENCY = 0.3f;
static const float WATER_HEIGHT_AMPLITUDE = 0.06f;

// GLOBAL: SWBF2 0x007df270
bool RedWater::mRenderLowResFlag = true;

// GLOBAL: SWBF2 0x007df284
PblList<RedWater::WaterCustomPatch> RedWater::m_CustomPatches;

// GLOBAL: SWBF2 0x009650b8
PblComplex* RedWater::m_initialOceanPatchFreq;
// GLOBAL: SWBF2 0x009650bc
PblComplex* RedWater::m_oceanPatchFreq;
// GLOBAL: SWBF2 0x009650c0
unsigned char* RedWater::m_foamIntensity;
// GLOBAL: SWBF2 0x009650c4
float* RedWater::m_oceanDispersion;
// GLOBAL: SWBF2 0x009650c8
float RedWater::m_windDirectionX;
// GLOBAL: SWBF2 0x009650cc
float RedWater::m_windDirectionZ;
// GLOBAL: SWBF2 0x009650d0
float RedWater::m_windSpeed;
// GLOBAL: SWBF2 0x009650d4
float RedWater::m_phillipsConstant;
// GLOBAL: SWBF2 0x009650d8
int RedWater::m_numWaterEvents;
// GLOBAL: SWBF2 0x009650dc
float RedWater::m_fWaterHeightOffset;
// GLOBAL: SWBF2 0x009650e0
RedWater::WaterPatch* RedWater::m_patchCache;
// GLOBAL: SWBF2 0x009650e4
int RedWater::m_patchCacheSize;
// GLOBAL: SWBF2 0x009650e8
unsigned char* RedWater::m_waterMap;
// GLOBAL: SWBF2 0x009650ec
unsigned int RedWater::m_waterMapSize;
// GLOBAL: SWBF2 0x009650f0
float RedWater::m_metersPerPatch;
// GLOBAL: SWBF2 0x009650f4
float RedWater::m_patchesPerMeter;
// GLOBAL: SWBF2 0x009650f8
unsigned int RedWater::m_patchesPerCell;
// GLOBAL: SWBF2 0x009650fc
float RedWater::m_cellsPerPatch;
// GLOBAL: SWBF2 0x00965100
int RedWater::m_reductionFactor;
// GLOBAL: SWBF2 0x00965104
float RedWater::m_worldCenterX;
// GLOBAL: SWBF2 0x00965108
float RedWater::m_worldCenterZ;
// GLOBAL: SWBF2 0x0096510c
RedWater::WaterModeT RedWater::m_waterMode;
// GLOBAL: SWBF2 0x00965110
float RedWater::m_waterHeight;
// GLOBAL: SWBF2 0x00965114
float RedWater::m_velocityU;
// GLOBAL: SWBF2 0x00965118
float RedWater::m_velocityV;
// GLOBAL: SWBF2 0x0096511c
float RedWater::m_tilesU;
// GLOBAL: SWBF2 0x00965120
float RedWater::m_tilesV;
// GLOBAL: SWBF2 0x00965124
int RedWater::m_pointsPerPatchX;
// GLOBAL: SWBF2 0x00965128
int RedWater::m_pointsPerPatchZ;
// GLOBAL: SWBF2 0x0096512c
int RedWater::m_divisionsPerPatchX;
// GLOBAL: SWBF2 0x00965130
int RedWater::m_divisionsPerPatchZ;
// GLOBAL: SWBF2 0x00965134
float RedWater::m_minPatchHeight;
// GLOBAL: SWBF2 0x00965138
float RedWater::m_maxPatchHeight;

// GLOBAL: SWBF2 0x0096513c
int RedWater::m_highResLowResRatio;
// GLOBAL: SWBF2 0x00965140
unsigned int RedWater::m_uiTextureHash;
// GLOBAL: SWBF2 0x00965144
unsigned int RedWater::m_uiFoamTextureHash;
// GLOBAL: SWBF2 0x00965148
float RedWater::m_foamTilesU;
// GLOBAL: SWBF2 0x0096514c
float RedWater::m_foamTilesV;
// GLOBAL: SWBF2 0x00965158
bool RedWater::m_oscillationEnable;
// GLOBAL: SWBF2 0x00965159
bool RedWater::m_specularEnable;
// GLOBAL: SWBF2 0x0096515a
bool RedWater::m_bSetSpecularDirection;
// GLOBAL: SWBF2 0x00966178
unsigned int s_numReads;

// GLOBAL: SWBF2 0x00965160
// SYNTHETIC: SWBF2 0x00408080
// `dynamic initializer for 'RedWater::m_worldMin''

PblVector3 RedWater::m_worldMin;
// GLOBAL: SWBF2 0x00966150
// SYNTHETIC: SWBF2 0x00408070
// `dynamic initializer for 'RedWater::m_worldMax''

PblVector3 RedWater::m_worldMax;

// GLOBAL: SWBF2 0x0096516c
// SYNTHETIC: SWBF2 0x00407fe0
// `dynamic initializer for 'RedWater::_UnderWaterRenderable''
// SYNTHETIC: SWBF2 0x00769920
// `dynamic atexit destructor for 'RedWater::_UnderWaterRenderable''
UnderWaterRenderable RedWater::_UnderWaterRenderable;

// GLOBAL: SWBF2 0x00965188
// SYNTHETIC: SWBF2 0x00408060
// `dynamic initializer for 'RedWater::m_vSpecularDirection''

PblVector3 RedWater::m_vSpecularDirection;

// GLOBAL: SWBF2 0x00965198
PblTimeBase::Stamp RedWater::m_waterTimeStamp;

// GLOBAL: SWBF2 0x009651b0
// SYNTHETIC: SWBF2 0x00408040
// `dynamic initializer for 'RedWater::m_eventCache''
RedWater::WaterEvent RedWater::m_eventCache[100];

// GLOBAL: SWBF2 0x0096615c
// SYNTHETIC: SWBF2 0x00408010
// `dynamic initializer for 'RedWater::_WaterRenderable''
// SYNTHETIC: SWBF2 0x00769940
// `dynamic atexit destructor for 'RedWater::_WaterRenderable''
WaterRenderable RedWater::_WaterRenderable;

// FUNCTION: SWBF2 0x006ce0a0
RedWater::WaterEvent::WaterEvent()
{
}

// FUNCTION: SWBF2 0x006ce110
void RedWater::AddWaterRing(const PblVector3& vPos, float fLifetime, float fRadius)
{
    if (m_waterMode != WATER_MODE_NORMAL)
    {
        return;
    }
    if (m_numWaterEvents >= 100)
    {
        return;
    }

    float fMax = (float)(m_waterMapSize - 1);
    float fX = (vPos.x + m_worldCenterX) * m_patchesPerMeter;
    if (fX < 0.0f)
    {
        fX = 0.0f;
    }
    else if (fX > fMax)
    {
        fX = fMax;
    }
    float fZ = (m_worldCenterZ + vPos.z) * m_patchesPerMeter;
    if (fZ < 0.0f)
    {
        fZ = 0.0f;
    }
    else if (fZ > fMax)
    {
        fZ = fMax;
    }
    if (!WaterExists((int)fX, (int)fZ))
    {
        return;
    }

    WaterEvent& event = m_eventCache[m_numWaterEvents];
    event.type = WATEREVENT_RING;
    event.startTime = m_waterTimeStamp.GetElapsed();
    event.expireTime = m_waterTimeStamp.GetElapsed() + fLifetime;
    event.radius = fRadius;
    event.worldVelocity = PblVector3(0.0f, 0.0f, 0.0f);
    event.worldPos = PblVector3(vPos.x, m_waterHeight, vPos.z);
    event.in_range = 1;
    ++m_numWaterEvents;
}

// FUNCTION: SWBF2 0x006ce270
void RedWater::AddWaterWake(const PblVector3& vPos, const PblVector3& vVelocity, float fLifetime,
                            float fRadius)
{
    if (m_waterMode != WATER_MODE_NORMAL)
    {
        return;
    }
    if (m_numWaterEvents >= 100)
    {
        return;
    }

    float fMax = (float)(m_waterMapSize - 1);
    float fX = (vPos.x + m_worldCenterX) * m_patchesPerMeter;
    if (fX < 0.0f)
    {
        fX = 0.0f;
    }
    else if (fX > fMax)
    {
        fX = fMax;
    }
    float fZ = (m_worldCenterZ + vPos.z) * m_patchesPerMeter;
    if (fZ < 0.0f)
    {
        fZ = 0.0f;
    }
    else if (fZ > fMax)
    {
        fZ = fMax;
    }
    if (!WaterExists((int)fX, (int)fZ))
    {
        return;
    }
    if (!(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 0.0f))
    {
        return;
    }

    WaterEvent& event = m_eventCache[m_numWaterEvents];
    event.type = WATEREVENT_WAKE;
    event.startTime = m_waterTimeStamp.GetElapsed();
    event.expireTime = m_waterTimeStamp.GetElapsed() + fLifetime;
    event.radius = fRadius;
    event.worldVelocity = vVelocity;
    event.worldPos = PblVector3(vVelocity.x * 0.03f + vPos.x, m_waterHeight, vVelocity.z * 0.03f + vPos.z);
    event.in_range = 1;
    ++m_numWaterEvents;
}

// FUNCTION: SWBF2 0x006ce410
void RedWater::BuildPatchCache(const RedCamera* pCamera, bool bFarScene, VISIBILITYTEST_MODE mode)
{
    int iMinX, iMinZ, iMaxX, iMaxZ;
    GetVisibleExtents(pCamera, iMinX, iMinZ, iMaxX, iMaxZ);
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);

    unsigned int uiFar = 0;
    PblList<WaterCustomPatch>::Itor itCustom(m_CustomPatches);
    VisibilityCacheEntryT* pCache = m_aVisibilityCache;
    unsigned int uiNear = m_uiVisibilityCacheSize_FarScene;
    m_patchCacheSize = 0;
    PblVector3 vExtents(m_metersPerPatch * 0.5f, (m_maxPatchHeight - m_minPatchHeight) * 0.5f,
                        m_metersPerPatch * 0.5f);

    for (int z = iMinZ; z < iMaxZ; ++z)
    {
        for (int x = iMinX; x < iMaxX; ++x)
        {
            if (!WaterExists(x, z))
            {
                continue;
            }

            PblVector3 vMin((float)x * m_metersPerPatch - m_worldCenterX,
                            m_fWaterHeightOffset + m_waterHeight + m_minPatchHeight,
                            (float)z * m_metersPerPatch - m_worldCenterZ);
            PblVector3 vCenter(m_metersPerPatch * 0.5f + vMin.x,
                               (m_maxPatchHeight - m_minPatchHeight) * 0.5f + vMin.y,
                               m_metersPerPatch * 0.5f + vMin.z);
            if (pCamera->ClassifyXZBox(vCenter, vExtents) == RedCamera::VISIBILITY_NONE)
            {
                continue;
            }
            if (m_patchCacheSize >= 1024)
            {
                return;
            }

            const PblMatrix& mCamera = pCamera->GetMatrix();
            float dx = vCenter.x - mCamera.m[3][0];
            float dy = vCenter.y - mCamera.m[3][1];
            float dz = vCenter.z - mCamera.m[3][2];
            float fDistanceSq = dx * dx + dy * dy + dz * dz;

            int iKey = x + z * m_waterMapSize;
            void* pCustomMesh = 0;
            for (; (WaterCustomPatch*)itCustom; ++itCustom)
            {
                int iCustomKey = itCustom->x + itCustom->z * m_waterMapSize;
                if (iCustomKey > iKey)
                {
                    break;
                }
                if (iCustomKey == iKey)
                {
                    pCustomMesh = itCustom->pCustomMesh;
                    break;
                }
            }

            unsigned short uiVisibilityID = 0;
            unsigned short uiVisibilityFarID = 0;
            if (mode == VISIBILITYTEST_ASSIGNID)
            {
                VisibilityCacheEntryT& entry = pCache[m_uiVisibilityCacheSize++];
                entry.x = (short)x;
                entry.z = (short)z;
                entry.uiVisibilityID = (unsigned short)m_uiVisibilityCacheSize;
                uiVisibilityID = (unsigned short)m_uiVisibilityCacheSize;
            }
            else if (mode == VISIBILITYTEST_GETIDS)
            {
                for (; uiNear < m_uiVisibilityCacheSize; ++uiNear)
                {
                    int iEntryKey = pCache[uiNear].x + pCache[uiNear].z * m_waterMapSize;
                    if (iEntryKey > iKey)
                    {
                        break;
                    }
                    if (iEntryKey == iKey)
                    {
                        uiVisibilityID = pCache[uiNear].uiVisibilityID;
                        break;
                    }
                }
                for (; uiFar < m_uiVisibilityCacheSize_FarScene; ++uiFar)
                {
                    int iEntryKey = pCache[uiFar].x + pCache[uiFar].z * m_waterMapSize;
                    if (iEntryKey > iKey)
                    {
                        break;
                    }
                    if (iEntryKey == iKey)
                    {
                        uiVisibilityFarID = pCache[uiFar].uiVisibilityID;
                        break;
                    }
                }
            }

            WaterPatch& patch = m_patchCache[m_patchCacheSize++];
            patch.pCustomMesh = pCustomMesh;
            patch.fDistanceSq = fDistanceSq;
            patch.bClipsNearPlane = false;
            patch.bClipped = false;
            patch.bbox._vMin = vMin;
            patch.bbox._vMax = PblVector3(vMin.x + m_metersPerPatch,
                                          (m_maxPatchHeight - m_minPatchHeight) + vMin.y,
                                          vMin.z + m_metersPerPatch);
            patch.uiVisibilityID = uiVisibilityID;
            patch.uiVisibilityFarID = uiVisibilityFarID;
        }
    }
}

// FUNCTION: SWBF2 0x006ce7e0
void RedWater::Cleanup()
{
    OceanCleanup();
    PlatformCleanup();
    _WaterRenderable.Deactivate();
    _UnderWaterRenderable.Deactivate();
    if (m_waterMap)
    {
        delete[] m_waterMap;
        m_waterMap = 0;
    }
    if (m_patchCache)
    {
        delete[] m_patchCache;
        m_patchCache = 0;
    }
    m_CustomPatches.RemoveAll();
}

// FUNCTION: SWBF2 0x006ce840
template void PblList<RedWater::WaterCustomPatch>::RemoveAll();

// FUNCTION: SWBF2 0x006ce880
void RedWater::GetVisibleExtents(const RedCamera* pCamera, int& iMinX, int& iMinZ, int& iMaxX,
                                 int& iMaxZ)
{
    PblBox box = pCamera->GetCameraBox();

    float f = (m_worldCenterX + box._vMin.x) * m_patchesPerMeter;
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > (float)m_waterMapSize)
    {
        f = (float)m_waterMapSize;
    }
    iMinX = (int)floorf(f);

    f = (m_worldCenterZ + box._vMin.z) * m_patchesPerMeter;
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > (float)m_waterMapSize)
    {
        f = (float)m_waterMapSize;
    }
    iMinZ = (int)floorf(f);

    f = (m_worldCenterX + box._vMax.x) * m_patchesPerMeter;
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > (float)m_waterMapSize)
    {
        f = (float)m_waterMapSize;
    }
    iMaxX = (int)ceilf(f);

    f = (m_worldCenterZ + box._vMax.z) * m_patchesPerMeter;
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > (float)m_waterMapSize)
    {
        f = (float)m_waterMapSize;
    }
    iMaxZ = (int)ceilf(f);
}

// FUNCTION: SWBF2 0x006cea90
bool RedWater::GetWaterHeight(const PblVector3& vPos, float& fHeight)
{
    if (!m_waterMap)
    {
        return false;
    }

    float fMax = (float)m_waterMapSize - 1.0f;
    float fX = (vPos.x + m_worldCenterX) * m_patchesPerMeter;
    if (fX < 0.0f)
    {
        fX = 0.0f;
    }
    else if (fX > fMax)
    {
        fX = fMax;
    }
    float fZ = (vPos.z + m_worldCenterZ) * m_patchesPerMeter;
    if (fZ < 0.0f)
    {
        fZ = 0.0f;
    }
    else if (fZ > fMax)
    {
        fZ = fMax;
    }
    if (!WaterExists((int)fX, (int)fZ))
    {
        return false;
    }

    fHeight = m_fWaterHeightOffset + m_waterHeight;
    return true;
}

// FUNCTION: SWBF2 0x006ceb50
bool RedWater::GetWaterHeightAndVelocity(const PblVector3& vPos, float& fHeight, PblVector3& vVelocity)
{
    if (!m_waterMap)
    {
        return false;
    }

    float fMax = (float)m_waterMapSize - 1.0f;
    float fX = (vPos.x + m_worldCenterX) * m_patchesPerMeter;
    if (fX < 0.0f)
    {
        fX = 0.0f;
    }
    else if (fX > fMax)
    {
        fX = fMax;
    }
    float fZ = (vPos.z + m_worldCenterZ) * m_patchesPerMeter;
    if (fZ < 0.0f)
    {
        fZ = 0.0f;
    }
    else if (fZ > fMax)
    {
        fZ = fMax;
    }
    if (!WaterExists((int)fX, (int)fZ))
    {
        return false;
    }

    fHeight = m_fWaterHeightOffset + m_waterHeight;
    D3DXVECTOR3 vFlow(m_velocityU, 0.0f, m_velocityV);
    D3DXVECTOR3 vDir;
    D3DXVec3Normalize(&vDir, &vFlow);
    vVelocity = *(PblVector3*)&vDir;
    return true;
}

// FUNCTION: SWBF2 0x006cec80
bool WaterRenderable::HasReflection()
{
    return true;
}

// FUNCTION: SWBF2 0x006cec90
void RedWater::Init()
{
    m_waterMap = 0;
    m_patchCache = 0;
    m_waterTimeStamp.Mark(RedRenderer::GetTimeBase());
    m_numWaterEvents = 0;
    m_fWaterHeightOffset = 0.0f;
    memset(m_eventCache, 0, sizeof(m_eventCache));
    m_divisionsPerPatchX = 16;
    m_divisionsPerPatchZ = 16;
    m_pointsPerPatchX = 17;
    m_pointsPerPatchZ = 17;
    m_waterMode = WATER_MODE_NORMAL;
    RedWater::m_highResLowResRatio = 2;
    RedWater::m_uiFoamTextureHash = 0;
    RedWater::m_foamTilesV = 1.0f;
    RedWater::m_foamTilesU = 1.0f;
    s_numReads = 0;
    mRenderLowResFlag = true;
    RedWater::m_bSetSpecularDirection = false;
    RedWater::m_oscillationEnable = true;
    RedWater::m_specularEnable = false;
    m_CustomPatches.RemoveAll();
    PlatformPreReadInit();
}

// FUNCTION: SWBF2 0x006ced80
bool UnderWaterRenderable::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x006ced90
bool WaterRenderable::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x006ceda0
void RedWater::OceanCleanup()
{
    if (m_waterMode == WATER_MODE_OCEAN)
    {
        delete[] m_initialOceanPatchFreq;
        delete[] m_oceanDispersion;
        delete[] m_foamIntensity;
        delete[] m_oceanPatchFreq;
    }
}

// FUNCTION: SWBF2 0x006cede0
void RedWater::OceanCreatePatch(PblComplex* pFreq)
{
    int iNumPoints = m_pointsPerPatchX * m_pointsPerPatchZ;
    int iNumDivisions = m_divisionsPerPatchX * m_divisionsPerPatchZ;
    PblVector3* pPositions = new PblVector3[iNumPoints];
    PblVector3* pNormals = new PblVector3[iNumPoints];

    m_fWaterHeightOffset = 0.0f;
    m_minPatchHeight = 100000.0f;
    m_maxPatchHeight = -100000.0f;
    for (int i = 0; i < iNumDivisions; ++i)
    {
        if (pFreq[i].real < m_minPatchHeight)
        {
            m_minPatchHeight = pFreq[i].real;
        }
        if (pFreq[i].real > m_maxPatchHeight)
        {
            m_maxPatchHeight = pFreq[i].real;
        }
    }
    float fRange = m_maxPatchHeight - m_minPatchHeight;

    for (int x = 0; x < m_pointsPerPatchX; ++x)
    {
        int ix = x;
        if (ix >= m_divisionsPerPatchX)
        {
            ix -= m_divisionsPerPatchX;
        }
        for (int z = 0; z < m_pointsPerPatchZ; ++z)
        {
            int iz = z;
            if (iz >= m_divisionsPerPatchZ)
            {
                iz -= m_divisionsPerPatchZ;
            }
            PblVector3& vPos = pPositions[x * m_pointsPerPatchZ + z];
            vPos.x = (float)x / ((float)m_pointsPerPatchX - 1.0f);
            vPos.y = (pFreq[ix * m_divisionsPerPatchZ + iz].real - m_minPatchHeight) / fRange;
            vPos.z = (float)z / ((float)m_pointsPerPatchZ - 1.0f);
        }
    }

    for (int x = 0; x < m_pointsPerPatchX; ++x)
    {
        for (int z = 0; z < m_pointsPerPatchZ; ++z)
        {
            int iRow = m_divisionsPerPatchZ * ((m_divisionsPerPatchX + x) % m_divisionsPerPatchX);
            int iRowNext = m_divisionsPerPatchZ * ((m_divisionsPerPatchX + x + 1) % m_divisionsPerPatchX);
            int iRowPrev = m_divisionsPerPatchZ * ((m_divisionsPerPatchX + x - 1) % m_divisionsPerPatchX);
            int iCol = (m_divisionsPerPatchZ + z) % m_divisionsPerPatchZ;
            int iColPrev = (m_divisionsPerPatchZ + z - 1) % m_divisionsPerPatchZ;
            int iColNext = (m_divisionsPerPatchZ + z + 1) % m_divisionsPerPatchZ;
            float fDX = m_metersPerPatch / (float)m_divisionsPerPatchX;
            float fDZ = m_metersPerPatch / (float)m_divisionsPerPatchZ;

            D3DXVECTOR3 vCenter(0.0f, pFreq[iRow + iCol].real, 0.0f);
            D3DXVECTOR3 vA = D3DXVECTOR3(0.0f, pFreq[iRow + iColPrev].real, -fDZ) - vCenter;
            D3DXVECTOR3 vB = D3DXVECTOR3(fDX, pFreq[iRowNext + iColPrev].real, -fDZ) - vCenter;
            D3DXVECTOR3 vC = D3DXVECTOR3(fDX, pFreq[iRowNext + iCol].real, 0.0f) - vCenter;
            D3DXVECTOR3 vD = D3DXVECTOR3(-fDX, pFreq[iRowPrev + iCol].real, 0.0f) - vCenter;
            D3DXVECTOR3 vE = D3DXVECTOR3(0.0f, pFreq[iRow + iColNext].real, fDZ) - vCenter;
            D3DXVECTOR3 vF = D3DXVECTOR3(-fDX, pFreq[iRowPrev + iColNext].real, fDZ) - vCenter;

            D3DXVECTOR3 vAD, vDF, vFE, vEC, vCB, vBA;
            D3DXVec3Cross(&vAD, &vA, &vD);
            D3DXVec3Cross(&vDF, &vD, &vF);
            D3DXVec3Cross(&vFE, &vF, &vE);
            D3DXVec3Cross(&vEC, &vE, &vC);
            D3DXVec3Cross(&vCB, &vC, &vB);
            D3DXVec3Cross(&vBA, &vB, &vA);
            D3DXVECTOR3 vNormal = vAD + vDF + vFE + vEC + vCB + vBA;
            D3DXVECTOR3 vUnit;
            D3DXVec3Normalize(&vUnit, &vNormal);
            pNormals[x * m_pointsPerPatchZ + z] = *(PblVector3*)&vUnit;
        }
    }

    for (int x = 0; x < m_pointsPerPatchX; ++x)
    {
        for (int z = 0; z < m_pointsPerPatchZ; ++z)
        {
            int iRow = m_divisionsPerPatchZ * ((m_divisionsPerPatchX + x) % m_divisionsPerPatchX);
            int iCol = (m_divisionsPerPatchZ + z) % m_divisionsPerPatchZ;
            float fHeight = pFreq[iRow + iCol].real;
            int i = x * m_pointsPerPatchZ + z;
            if ((fHeight - pFreq[iRow + (m_divisionsPerPatchZ + z - 1) % m_divisionsPerPatchZ].real) -
                        (pFreq[iRow + (m_divisionsPerPatchZ + z + 1) % m_divisionsPerPatchZ].real - fHeight) >
                    3.0f ||
                (fHeight - pFreq[m_divisionsPerPatchZ * ((m_divisionsPerPatchX + x - 1) % m_divisionsPerPatchX) + iCol].real) -
                        (pFreq[m_divisionsPerPatchZ * ((m_divisionsPerPatchX + x + 1) % m_divisionsPerPatchX) + iCol].real - fHeight) >
                    3.0f)
            {
                if (m_foamIntensity[i] > 249)
                {
                    m_foamIntensity[i] = 255;
                }
                else
                {
                    m_foamIntensity[i] += 6;
                }
            }
            else
            {
                if (m_foamIntensity[i] < 4)
                {
                    m_foamIntensity[i] = 0;
                }
                else
                {
                    m_foamIntensity[i] -= 4;
                }
            }
        }
    }

    PlatformUpdatePatchGeometry(pPositions, pNormals);
    delete[] pPositions;
    delete[] pNormals;
}

// FUNCTION: SWBF2 0x006cf4f0
float RedWater::OceanDispersion(float kx, float kz)
{
    return sqrtf(sqrtf(kx * kx + kz * kz) * 9.81f);
}

// FUNCTION: SWBF2 0x006cf540
void RedWater::OceanInit()
{
    PblRandom rng;

    if (m_waterMode == WATER_MODE_OCEAN)
    {
        m_initialOceanPatchFreq = new PblComplex[m_divisionsPerPatchX * m_divisionsPerPatchZ];
        m_foamIntensity = new unsigned char[m_pointsPerPatchX * m_pointsPerPatchZ];
        m_oceanDispersion = new float[m_divisionsPerPatchX * m_divisionsPerPatchZ];
        m_oceanPatchFreq = new PblComplex[m_divisionsPerPatchX * m_divisionsPerPatchZ];
        m_foamIntensity[0] = 0;

        for (int x = 0; x < m_divisionsPerPatchX; ++x)
        {
            for (int z = 0; z < m_divisionsPerPatchZ; ++z)
            {
                float kx = (float)((m_divisionsPerPatchX / 2 + x) % m_divisionsPerPatchX -
                                   m_divisionsPerPatchX / 2) *
                           PblMath::fTwoPi / m_metersPerPatch;
                float kz = (float)((m_divisionsPerPatchZ / 2 + z) % m_divisionsPerPatchZ -
                                   m_divisionsPerPatchZ / 2) *
                           PblMath::fTwoPi / m_metersPerPatch;
                float fAmplitude = (float)sqrt(OceanPhillipsSpectrum(kx, kz)) * INV_SQRT_2;
                int i = x * m_divisionsPerPatchZ + z;
                m_initialOceanPatchFreq[i].real = rng.GetNormal(0.0f, 1.0f) * fAmplitude;
                m_initialOceanPatchFreq[i].imag = rng.GetNormal(0.0f, 1.0f) * fAmplitude;
                m_oceanDispersion[i] = OceanDispersion(kx, kz);
            }
        }
    }
}

// FUNCTION: SWBF2 0x006cf7a0
float RedWater::OceanPhillipsSpectrum(float kx, float kz)
{
    float fL = m_windSpeed * m_windSpeed / WATER_GRAVITY;
    float fK2 = kx * kx + kz * kz;
    if (fK2 == 0.0f)
    {
        return 0.0f;
    }
    float fK = (float)sqrt(fK2);
    float fKDotW = (float)fabs(kz / fK * m_windDirectionZ + kx / fK * m_windDirectionX);
    float fl = fL / 75.0f;
    float fP = (float)exp(-1.0f / (fL * fL * fK2)) * m_phillipsConstant * fKDotW * fKDotW / (fK2 * fK2);
    return fP * (float)exp(-(fl * fK2 * fl));
}

// FUNCTION: SWBF2 0x006cf8f0
void RedWater::OceanUpdate(float fTime)
{
    for (int x = 0; x < m_divisionsPerPatchX; ++x)
    {
        for (int z = 0; z < m_divisionsPerPatchZ; ++z)
        {
            int mx = m_divisionsPerPatchX - x;
            if (mx >= m_divisionsPerPatchX)
            {
                mx -= m_divisionsPerPatchX;
            }
            int mz = m_divisionsPerPatchZ - z;
            if (mz >= m_divisionsPerPatchZ)
            {
                mz -= m_divisionsPerPatchZ;
            }
            int i = x * m_divisionsPerPatchZ + z;
            int j = mx * m_divisionsPerPatchZ + mz;

            float fConjReal = m_initialOceanPatchFreq[j].real;
            float fConjImag = -m_initialOceanPatchFreq[j].imag;
            float fSin, fCos;
            PblMath::SinCos(&fSin, &fCos, m_oceanDispersion[i] * fTime);
            const PblComplex& h0 = m_initialOceanPatchFreq[i];
            m_oceanPatchFreq[i].real = fCos * h0.real - h0.imag * fSin;
            m_oceanPatchFreq[i].imag = h0.imag * fCos + fSin * h0.real;
            m_oceanPatchFreq[i].real += fSin * fConjImag + fCos * fConjReal;
            m_oceanPatchFreq[i].imag += fCos * fConjImag - fSin * fConjReal;
        }
    }
    PblFourier::InvFFT2D(m_oceanPatchFreq, m_divisionsPerPatchX, m_divisionsPerPatchZ, false);
}

// FUNCTION: SWBF2 0x006cfa70
float RedWater::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                       PblVector3* pNormal)
{
    if (!m_waterMap)
    {
        return 2.0f;
    }

    float fHit = fLength + 100.0f;
    float fDY = m_fWaterHeightOffset + m_waterHeight - vStart.y;
    if (fDY > 0.0f)
    {
        return 2.0f;
    }
    float t = fDY / vDir.y;
    if (t < 0.0f)
    {
        return 2.0f;
    }

    float fX = vDir.x * t + vStart.x;
    float fZ = vDir.z * t + vStart.z;
    float fMax = (float)m_waterMapSize - 1.0f;
    float fPatchX = (fX + m_worldCenterX) * m_patchesPerMeter;
    if (fPatchX < 0.0f)
    {
        fPatchX = 0.0f;
    }
    else if (fPatchX > fMax)
    {
        fPatchX = fMax;
    }
    float fPatchZ = (m_worldCenterZ + fZ) * m_patchesPerMeter;
    if (fPatchZ < 0.0f)
    {
        fPatchZ = 0.0f;
    }
    else if (fPatchZ > fMax)
    {
        fPatchZ = fMax;
    }
    if (!WaterExists((int)fPatchX, (int)fPatchZ))
    {
        return 2.0f;
    }

    if (t > fLength)
    {
        return 2.0f;
    }
    if (t < fHit)
    {
        fHit = t;
    }
    if (fLength < fHit)
    {
        return 2.0f;
    }
    if (pNormal)
    {
        *pNormal = PblVector3(0.0f, 1.0f, 0.0f);
    }
    return fHit / fLength;
}

// FUNCTION: SWBF2 0x006cfbd0
void RedWater::Read(PblFileChunk& chunk)
{
    bool bReadLayer = false;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* pFile = child._pFile;
        switch (child._iId)
        {
        case 0x4F464E49:
            m_waterMapSize = pFile->Read32();
            m_patchesPerCell = pFile->Read32();
            m_metersPerPatch = pFile->ReadFloat();
            m_worldCenterX = pFile->ReadFloat();
            m_worldCenterZ = pFile->ReadFloat();
            m_reductionFactor = pFile->Read32();
            m_patchesPerMeter = 1.0f / m_metersPerPatch;
            m_cellsPerPatch = 1.0f / (float)m_patchesPerCell;
            break;

        case 0x50414D57:
            m_waterMap = new unsigned char[m_waterMapSize * m_waterMapSize / m_patchesPerCell];
            pFile->ReadData(m_waterMap, m_waterMapSize * m_waterMapSize / m_patchesPerCell);
            break;

        case 0x5259414C:
            if (!bReadLayer)
            {
                char acName[256];
                child.ReadString(acName, sizeof(acName));
                pFile->Read32();
                m_waterHeight = pFile->ReadFloat();
                pFile->ReadFloat();
                pFile->ReadFloat();
                pFile->ReadFloat();
                pFile->ReadFloat();
                pFile->Read8();
                pFile->Read8();
                pFile->Read8();
                pFile->Read8();
                bReadLayer = true;
            }
            break;

        case 0x54415043:
            for (unsigned int z = 0; z < m_waterMapSize; ++z)
            {
                for (unsigned int x = 0; x < m_waterMapSize; ++x)
                {
                    unsigned char uiCell =
                        m_waterMap[z * (m_waterMapSize / m_patchesPerCell) + x / m_patchesPerCell];
                    if ((uiCell >> ((x & 1) ? 0 : 4)) & 8)
                    {
                        void* pMesh = PlatformReadCustomPatch(&child);
                        if (pMesh)
                        {
                            WaterCustomPatch* pPatch = new WaterCustomPatch;
                            pPatch->x = x;
                            pPatch->z = z;
                            pPatch->pCustomMesh = pMesh;
                            m_CustomPatches.AddLast(&pPatch->Node, pPatch);
                        }
                    }
                }
            }
            break;
        }
    }

    m_worldMin = PblVector3(-m_worldCenterX, m_waterHeight - 1.0f, -m_worldCenterZ);
    m_worldMax = PblVector3(m_worldCenterX, m_waterHeight + 20.0f, m_worldCenterZ);
    m_patchCache = new WaterPatch[1024];

    if (++s_numReads == 2)
    {
        if (m_waterMap && m_waterMode == WATER_MODE_NORMAL)
        {
            _WaterRenderable.Activate(100001);
            _UnderWaterRenderable.Activate(0);
        }
        if (RedWater::m_highResLowResRatio > m_divisionsPerPatchX)
        {
            RedWater::m_highResLowResRatio = m_divisionsPerPatchX;
        }
        float fReduction = (float)m_reductionFactor;
        m_divisionsPerPatchX *= m_reductionFactor;
        m_tilesU = fReduction * m_tilesU;
        m_divisionsPerPatchZ *= m_reductionFactor;
        m_pointsPerPatchX = m_divisionsPerPatchX + 1;
        m_pointsPerPatchZ = m_divisionsPerPatchZ + 1;
        m_tilesV = fReduction * m_tilesV;
        m_velocityU = m_velocityU / fReduction;
        m_velocityV = m_velocityV / fReduction;
        OceanInit();
        PlatformPostReadInit();
        Update(0.0f);
    }
}

// FUNCTION: SWBF2 0x006d00d0
void RedWater::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x13901A72:
            m_windSpeed = data.GetFloatArg(0);
            break;
        case 0x12E39AF3:
            RedWater::m_oscillationEnable = data.GetUint32Arg(0) != 0;
            break;
        case 0x009036EB:
            m_divisionsPerPatchX = data.GetUint32Arg(0);
            m_divisionsPerPatchZ = data.GetUint32Arg(1);
            break;
        case 0x32741C32:
            m_velocityU = data.GetFloatArg(0);
            m_velocityV = data.GetFloatArg(1);
            break;
        case 0x35A83D43:
            m_uiTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x415E2B30:
        {
            float fX = data.GetFloatArg(0);
            float fZ = data.GetFloatArg(1);
            float fLength = (float)sqrt(fZ * fZ + fX * fX);
            m_windDirectionX = data.GetFloatArg(0) / fLength;
            m_windDirectionZ = data.GetFloatArg(1) / fLength;
            break;
        }
        case 0x56E7ACB7:
            RedWater::m_uiFoamTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x713A7CC9:
            m_tilesU = data.GetFloatArg(0);
            m_tilesV = data.GetFloatArg(1);
            break;
        case 0x807B1583:
            RedWater::m_highResLowResRatio = data.GetUint32Arg(0);
            break;
        case 0xA015E2FF:
            m_waterMode = data.GetUint32Arg(0) ? WATER_MODE_ICE : WATER_MODE_NORMAL;
            break;
        case 0xA01D9243:
            mRenderLowResFlag = false;
            break;
        case 0xA69C431A:
            m_waterMode = data.GetUint32Arg(0) ? WATER_MODE_OCEAN : WATER_MODE_NORMAL;
            break;
        case 0xD25EF90A:
            m_phillipsConstant = data.GetFloatArg(0);
            break;
        case 0xD4DDEC5F:
            RedWater::m_specularEnable = data.GetUint32Arg(0) != 0;
            break;
        case 0xDCB91B88:
            RedWater::m_foamTilesU = data.GetFloatArg(0);
            RedWater::m_foamTilesV = data.GetFloatArg(1);
            break;
        default:
            PlatformReadConfigData(data);
            break;
        }
    }

    if (++s_numReads == 2)
    {
        if (m_waterMap && m_waterMode == WATER_MODE_NORMAL)
        {
            _WaterRenderable.Activate(100001);
            _UnderWaterRenderable.Activate(0);
        }
        if (RedWater::m_highResLowResRatio > m_divisionsPerPatchX)
        {
            RedWater::m_highResLowResRatio = m_divisionsPerPatchX;
        }
        float fReduction = (float)m_reductionFactor;
        m_divisionsPerPatchZ *= m_reductionFactor;
        m_tilesU = fReduction * m_tilesU;
        m_divisionsPerPatchX *= m_reductionFactor;
        m_pointsPerPatchX = m_divisionsPerPatchX + 1;
        m_pointsPerPatchZ = m_divisionsPerPatchZ + 1;
        m_tilesV = fReduction * m_tilesV;
        m_velocityU = m_velocityU / fReduction;
        m_velocityV = m_velocityV / fReduction;
        OceanInit();
        PlatformPostReadInit();
        Update(0.0f);
    }
}

// FUNCTION: SWBF2 0x006d0480
void RedWater::Render(unsigned int uiView)
{
    if (m_waterMode == WATER_MODE_OCEAN)
    {
        OceanCreatePatch(m_oceanPatchFreq);
    }
    PlatformRender(uiView);
}

// FUNCTION: SWBF2 0x006d04a0
void UnderWaterRenderable::Render(int value)
{
    RedWater::RenderUnderWaterView(value);
}

// FUNCTION: SWBF2 0x006d04c0
void WaterRenderable::Render(int value)
{
    RedWater::Render(value);
}

// FUNCTION: SWBF2 0x006d04e0
void RedWater::RenderUnderWaterView(unsigned int uiView)
{
    if (!m_waterMap || (uiView & 0x20000) || m_waterMode != WATER_MODE_NORMAL)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);

    float fWaterHeight = m_fWaterHeightOffset + m_waterHeight;
    float fAbove = pCamera->GetMatrix().m[3][1] - fWaterHeight;
    if (fAbove > 0.0f &&
        fAbove * fAbove > (fWidth * fWidth + fHeight * fHeight) * 0.25f + fNear * fNear)
    {
        return;
    }

    float fHalfWidth = fWidth * 0.5f;
    float fHalfHeight = fHeight * 0.5f;
    PblMatrix mCamera = pCamera->GetMatrix();
    float fZoom = pCamera->GetZoom();
    if (fZoom != 1.0f)
    {
        float fUnzoomedNear = fNear;
        fNear *= fZoom;
        fFar *= fZoom;
        fHalfWidth *= fZoom;
        fHalfHeight *= fZoom;
        mCamera = pCamera->GetMatrix();
        float fPush = fNear - fUnzoomedNear;
        mCamera.m[3][0] += mCamera.m[2][0] * fPush;
        mCamera.m[3][1] += mCamera.m[2][1] * fPush;
        mCamera.m[3][2] += mCamera.m[2][2] * fPush;
        mCamera.m[3][3] = 1.0f;
    }

    static const PblVector3 s_screenCorners[4] = {
        PblVector3(-1.0f, -1.0f, 0.0f), PblVector3(-1.0f, 1.0f, 0.0f),
        PblVector3(1.0f, 1.0f, 0.0f), PblVector3(1.0f, -1.0f, 0.0f)};

    PblVector3 vWorldCorners[4];
    PblVector3 vLocal;
    vLocal = PblVector3(-fHalfWidth, -fHalfHeight, -fNear);
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorldCorners[0], (const D3DXVECTOR3*)&vLocal,
                           (const D3DXMATRIX*)&mCamera);
    vLocal = PblVector3(-fHalfWidth, fHalfHeight, -fNear);
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorldCorners[1], (const D3DXVECTOR3*)&vLocal,
                           (const D3DXMATRIX*)&mCamera);
    vLocal = PblVector3(fHalfWidth, fHalfHeight, -fNear);
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorldCorners[2], (const D3DXVECTOR3*)&vLocal,
                           (const D3DXMATRIX*)&mCamera);
    vLocal = PblVector3(fHalfWidth, -fHalfHeight, -fNear);
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vWorldCorners[3], (const D3DXVECTOR3*)&vLocal,
                           (const D3DXMATRIX*)&mCamera);

    PblVector3 vVerts[5];
    int iNumVerts = 0;
    bool bOverWater = false;
    int iPrev = 3;
    float fPrev = vWorldCorners[3].y - fWaterHeight;
    for (int i = 0; i < 4; ++i)
    {
        float fCur = vWorldCorners[i].y - fWaterHeight;
        if ((fCur > 0.0f) != (fPrev > 0.0f))
        {
            float fInv = 1.0f / (fCur - fPrev);
            float fA = fInv * fCur;
            float fB = fInv * fPrev;
            vVerts[iNumVerts].x = s_screenCorners[iPrev].x * fA - s_screenCorners[i].x * fB;
            vVerts[iNumVerts].y = s_screenCorners[iPrev].y * fA - s_screenCorners[i].y * fB;
            vVerts[iNumVerts].z = s_screenCorners[iPrev].z * fA - s_screenCorners[i].z * fB;
            ++iNumVerts;
        }
        if (fCur <= 0.0f)
        {
            vVerts[iNumVerts++] = s_screenCorners[i];
        }

        float fSurface;
        if (GetWaterHeight(vWorldCorners[i], fSurface))
        {
            bOverWater = true;
        }
        iPrev = i;
        fPrev = fCur;
    }

    if (bOverWater && iNumVerts)
    {
        PlatformRenderUnderWaterView(vVerts, iNumVerts);
    }
}

// FUNCTION: SWBF2 0x006d0a10
bool RedWater::Update(float fElapsed)
{
    if (m_waterMode == WATER_MODE_OCEAN)
    {
        m_fWaterHeightOffset = 0.0f;
        OceanUpdate(fElapsed);
        return true;
    }

    if (m_waterMode != WATER_MODE_ICE && RedWater::m_oscillationEnable)
    {
        m_fWaterHeightOffset =
            PblMath::Sin(PblMath::fTwoPi * m_waterTimeStamp.GetElapsed() * WATER_HEIGHT_FREQUENCY) *
            WATER_HEIGHT_AMPLITUDE;
    }
    else
    {
        m_fWaterHeightOffset = 0.0f;
    }

    m_maxPatchHeight = 1.0f;
    m_minPatchHeight = 0.0f;
    return true;
}

// FUNCTION: SWBF2 0x006d0ba0
bool RedWater::WaterExists(int x, int z)
{
    unsigned char* pRow = m_waterMap + z * (m_waterMapSize / m_patchesPerCell);
    unsigned char uiCell = pRow[x / m_patchesPerCell];
    if (x & 1)
    {
        return (uiCell & 0x0f) != 0;
    }
    return (uiCell & 0xf0) != 0;
}
