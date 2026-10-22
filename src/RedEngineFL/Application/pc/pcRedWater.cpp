#include <d3d9.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcNormalMapAdderShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcOceanShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcPreReflectionShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcWaterShader.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcFLRender.h"

// FUNCTION: SWBF2 0x00720b80
void __cdecl RedWater::pcBeginVisibilityTest(unsigned int uiID)
{
    IDirect3DQuery9*& query = RedWater::m_aVisibilityCache[uiID - 1].pVisibilityQuery;
    if (!query)
    {
        RedRenderer::pcGetDevice()->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query);
    }
    query->Issue(D3DISSUE_BEGIN);
}

// FUNCTION: SWBF2 0x00720bc0
bool RedWater::pcDeviceSupportsVisibilityTesting()
{
    return RedRenderer::pcGetDevice()->CreateQuery(D3DQUERYTYPE_OCCLUSION, 0) == D3D_OK;
}

// FUNCTION: SWBF2 0x00720c00
unsigned int __cdecl RedWater::pcGetVisibilityTestResult(unsigned int uiID)
{
    IDirect3DQuery9* query = RedWater::m_aVisibilityCache[uiID - 1].pVisibilityQuery;
    unsigned int visible = 0;
    if (!query || query->GetData(&visible, sizeof(visible), 0) != D3D_OK)
    {
        return 100;
    }
    return visible;
}

// FUNCTION: SWBF2 0x00720be0
void __cdecl RedWater::pcEndVisibilityTest(unsigned int uiID)
{
    RedWater::m_aVisibilityCache[uiID - 1].pVisibilityQuery->Issue(D3DISSUE_END);
}

// GLOBAL: SWBF2 0x009cab90
float RedWater::m_fresnelMin;
// GLOBAL: SWBF2 0x009cab94
float RedWater::m_fresnelMax;
// GLOBAL: SWBF2 0x009cab98
unsigned int RedWater::m_uiSpecularMaskTextures[50];
// GLOBAL: SWBF2 0x009cac60
unsigned int RedWater::m_uiNumSpecularMaskTextures;
// GLOBAL: SWBF2 0x009cac64
float RedWater::m_fSpecularMaskTexturesFrameRate;
// GLOBAL: SWBF2 0x009cac68
unsigned int RedWater::m_uiBumpMapTextures[50];
// GLOBAL: SWBF2 0x009cad30
unsigned int RedWater::m_uiNumBumpMapTextures;
// GLOBAL: SWBF2 0x009cad34
float RedWater::m_fBumpMapTexturesFrameRate;
// GLOBAL: SWBF2 0x009cad38
unsigned int RedWater::m_uiNormalMapTextures[50];
// GLOBAL: SWBF2 0x009cae00
unsigned int RedWater::m_uiNumNormalMapTextures;
// GLOBAL: SWBF2 0x009cae04
float RedWater::m_fNormalMapTexturesFrameRate;
// GLOBAL: SWBF2 0x009cae08
float RedWater::m_fSpecularMaskTileU;
// GLOBAL: SWBF2 0x009cae0c
float RedWater::m_fSpecularMaskTileV;
// GLOBAL: SWBF2 0x009cae10
float RedWater::m_fSpecularMaskScrollU;
// GLOBAL: SWBF2 0x009cae14
float RedWater::m_fSpecularMaskScrollV;
// GLOBAL: SWBF2 0x009cae18
float RedWater::m_farSceneRange;

// GLOBAL: SWBF2 0x009cae1c
char RedWater::m_reflectionID;
// GLOBAL: SWBF2 0x009cae1d
bool RedWater::m_bEnableVisibilityTest;
// GLOBAL: SWBF2 0x009cae20
pcRedPrimitive* RedWater::m_pLowResPrim;
// GLOBAL: SWBF2 0x009cae24
pcRedPrimitive* RedWater::m_pHighResPrim;
// GLOBAL: SWBF2 0x009cae28
RedWater::VisibilityCacheEntryT* RedWater::m_aVisibilityCache;
// GLOBAL: SWBF2 0x009cae2c
unsigned int RedWater::m_uiVisibilityCacheSize;
// GLOBAL: SWBF2 0x009cae30
unsigned int RedWater::m_uiVisibilityCacheSize_FarScene;
// GLOBAL: SWBF2 0x009cae7c
RedColor RedWater::m_refractionColor;
// GLOBAL: SWBF2 0x009cae84
RedColor RedWater::m_reflectionColor;
// GLOBAL: SWBF2 0x009cae94
RedColor RedWater::m_underwaterColor;

// GLOBAL: SWBF2 0x007e25b4
static bool bEnable = true;

static const float WATEREVENT_MAX_VISIBLE_DISTANCE = 120.0f;
// GLOBAL: SWBF2 0x007e25c8
static float WATEREVENT_MAX_VISIBLE_DISTANCESQR =
    WATEREVENT_MAX_VISIBLE_DISTANCE * WATEREVENT_MAX_VISIBLE_DISTANCE;
// GLOBAL: SWBF2 0x007e2624
static float WATERSPLASH_OUTER_ANGLE = 0.78539819f;
// GLOBAL: SWBF2 0x007e2628
static float WATERSPLASH_INNER_ANGLE = 1.4835298f;

static const float WATERRING_SPEED = 1.5f;
static const float WATERRING_MAX_ALPHA = 0.75f;
static const float WATERRING_MAX_ALPHA_AT_TIME = 0.15f;
static const float WATERRING_MAX_ALPHA_AT_LIFETIME = 3.0f;
static const float WATERRING_HEIGHT_BIAS = 0.01f;

static const float WATERRING_INNERWIDTH = 0.35f;
static const float WATERRING_OUTERWIDTH = 0.35f;
static const float WATERDISTORTIONRING_INNERWIDTH = 1.0f;
static const float WATERDISTORTIONRING_OUTERWIDTH = 0.5f;
static const float WATERSPLASH_MAX_SEGMENTS_RADIUS = 2.0f;
// GLOBAL: SWBF2 0x007e25cc
static float WATERRING_TEX_STEP = 0.25f;
// GLOBAL: SWBF2 0x007e2610
static float WATERRING_SPEEDINV = 1.0f / WATERRING_SPEED;
// GLOBAL: SWBF2 0x007e2618
static float WATERRING_MAX_ALPHASCALE_BEFORE_TIME = 1.0f / WATERRING_MAX_ALPHA_AT_TIME;
// GLOBAL: SWBF2 0x007e261c
static float WATERRING_MAX_ALPHASCALE_AFTER_TIME = 1.0f / (1.0f - WATERRING_MAX_ALPHA_AT_TIME);
// GLOBAL: SWBF2 0x007e2620
static float WATERSPLASH_MAX_SEGMENTS_RADIUSINV = 1.0f / WATERSPLASH_MAX_SEGMENTS_RADIUS;

// GLOBAL: SWBF2 0x009cae8c
// SYNTHETIC: SWBF2 0x0040ab20
// `dynamic initializer for 'WATERSPLASH_COLOR''
static RedColor WATERSPLASH_COLOR = RedColor::LTGREY;

// GLOBAL: SWBF2 0x007e262c

bool s_bWaterShaderSetupPending = true;

// GLOBAL: SWBF2 0x009cae38
static pcWaterShader* s_pNormalWaterShader;
// GLOBAL: SWBF2 0x009cae3c
static pcOceanShader* s_pOceanWaterShader;
// GLOBAL: SWBF2 0x009cae40
static pcRedShader* s_pWaterSplashShader;
// GLOBAL: SWBF2 0x009cae44
static pcRedShader* s_pWaterRingShader;
// GLOBAL: SWBF2 0x009cae48
static pcPreReflectionShader* s_pPreReflectionShader;
// GLOBAL: SWBF2 0x009cae4c
static pcNormalMapAdderShader* s_pNormalMapAdderShader;
// GLOBAL: SWBF2 0x009cae50
static pcNormalMapAdderShader* s_pWaterDistortionRingShader;
// GLOBAL: SWBF2 0x009cae54
static pcClearShader* s_pUnderWaterShader;

// GLOBAL: SWBF2 0x009cae74
// SYNTHETIC: SWBF2 0x0040ab90
// `dynamic initializer for 'kuiNormalMapAdderShader''
unsigned int kuiNormalMapAdderShader = PblHash("NormalMapAdder");
// GLOBAL: SWBF2 0x009cae80
// SYNTHETIC: SWBF2 0x0040abb0
// `dynamic initializer for 'kuiOceanShader''
unsigned int kuiOceanShader = PblHash("Ocean");
// GLOBAL: SWBF2 0x009cae88
// SYNTHETIC: SWBF2 0x0040abd0
// `dynamic initializer for 'kuiPreReflectionShader''
unsigned int kuiPreReflectionShader = PblHash("PreReflection");
// GLOBAL: SWBF2 0x009cae78
// SYNTHETIC: SWBF2 0x0040abf0
// `dynamic initializer for 'kuiUnderWaterShader''
unsigned int kuiUnderWaterShader = PblHash("Clear");
// GLOBAL: SWBF2 0x009cae58
// SYNTHETIC: SWBF2 0x0040ac10
// `dynamic initializer for 'kuiWaterDistortionRingTextureHash''
unsigned int kuiWaterDistortionRingTextureHash = PblHash("fx_ringbump");
// GLOBAL: SWBF2 0x009cae5c
// SYNTHETIC: SWBF2 0x0040ac30
// `dynamic initializer for 'kuiWaterRingShader''
unsigned int kuiWaterRingShader = PblHash("Particle");
// GLOBAL: SWBF2 0x009cae64
// SYNTHETIC: SWBF2 0x0040ac50
// `dynamic initializer for 'kuiWaterRingTextureHash''
unsigned int kuiWaterRingTextureHash = PblHash("FXrippl1");
// GLOBAL: SWBF2 0x009cae90
// SYNTHETIC: SWBF2 0x0040ac70
// `dynamic initializer for 'kuiWaterShader''
unsigned int kuiWaterShader = PblHash("Water");
// GLOBAL: SWBF2 0x009cae68
// SYNTHETIC: SWBF2 0x0040ac90
// `dynamic initializer for 'kuiWaterSplashShader''
unsigned int kuiWaterSplashShader = PblHash("Particle");
// GLOBAL: SWBF2 0x009cae60
// SYNTHETIC: SWBF2 0x0040acb0
// `dynamic initializer for 'kuiWaterSplashTextureHash''
unsigned int kuiWaterSplashTextureHash = PblHash("fx_splash");

static const float kInv255 = 1.0f / 255.0f;

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
struct CompressedNormal
{
    unsigned char z;
    unsigned char y;
    unsigned char x;
    unsigned char w;
};
struct CompressedTexCoord
{
    short u;
    short v;
};
struct CompressedOceanVertex
{
    CompressedPos pos;
    CompressedNormal normal;
    RedColor color;
    CompressedTexCoord uv;
};
 

struct OceanVertex
{
    PblVector3 pos;
    PblVector3 normal;
    RedColor color;
    float u, v;
};
struct CompressedWaterVertex
{
    CompressedPos pos;
    CompressedNormal normal;
    CompressedTexCoord uv;
};
struct WaterVertex
{
    PblVector3 pos;
    PblVector3 normal;
    float u, v;
};
struct CompressedWaterRingVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
struct WaterRingVertex
{
    PblVector3 pos;
    RedColor color;
    float u, v;
};
 
 
 
 
 

// FUNCTION: SWBF2 0x00719c70
template <>
pcRedPrimitive* RedWater::pcCreateDynamicPatchPrimitive<CompressedOceanVertex, 53922>(
    PblVector3* pPositions, PblVector3* pNormals, unsigned int uiPointsX,
    unsigned int uiPointsZ, unsigned int uiStep)
{
    unsigned int uiNumX = (uiPointsX - 1) / uiStep + 1;
    unsigned int uiNumZ = (uiPointsZ - 1) / uiStep + 1;
    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 53922,
                                                   sizeof(CompressedOceanVertex),
                                                   uiNumX * uiNumZ,
                                                   2 * ((uiNumX - 1) * uiNumZ + uiNumX) - 4,
                                                   false);
    CompressedOceanVertex* pVert = (CompressedOceanVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return pPrim;
    }

    PblVector3 vMin(0.0f, m_fWaterHeightOffset + m_waterHeight + m_minPatchHeight, 0.0f);
    PblVector3 vSize(m_metersPerPatch + 0.001f, m_maxPatchHeight - m_minPatchHeight,
                     m_metersPerPatch + 0.001f);
    PblVector3 vMax(vMin.x + vSize.x, vMin.y + vSize.y, vMin.z + vSize.z);
    pPrim->m_bbox._vMin = vMin;
    pPrim->m_bbox._vMax = vMax;

    PblVector3 vBoxMin = { vMin.x, vMin.y, vMin.z };
    PblVector3 vExtent = { vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z };
    CompressedPos::s_vMin = vBoxMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;

    float fHeightScale = vExtent.y;
    float fHeightBase = vMin.y;
    for (unsigned int x = 0; x < uiPointsX; x += uiStep)
    {
        for (unsigned int z = 0; z < uiPointsZ; z += uiStep)
        {
            unsigned int i = z + x * m_pointsPerPatchZ;
            const PblVector3& rPos = pPositions[i];
            const PblVector3& rNormal = pNormals[i];
            pVert->pos.x = (short)((rPos.x * m_metersPerPatch - CompressedPos::s_vMin.x)
                                   * CompressedPos::s_vScaleInv.x - 32767.5f);
            pVert->pos.y = (short)((rPos.y * fHeightScale + fHeightBase - CompressedPos::s_vMin.y)
                                   * CompressedPos::s_vScaleInv.y - 32767.5f);
            pVert->pos.z = (short)((rPos.z * m_metersPerPatch - CompressedPos::s_vMin.z)
                                   * CompressedPos::s_vScaleInv.z - 32767.5f);
            pVert->normal.x = (unsigned char)((rNormal.x + 1.0f) * 127.5f);
            pVert->normal.y = (unsigned char)((rNormal.y + 1.0f) * 127.5f);
            pVert->normal.z = (unsigned char)((rNormal.z + 1.0f) * 127.5f);
            pVert->uv.u = (short)(rPos.x * 2048.0f);
            pVert->uv.v = (short)(rPos.z * 2048.0f);
            RedColor color = RedColor::WHITE;
            color.a = m_foamIntensity[i];
            pVert->color = color;
            ++pVert;
        }
    }
    pPrim->EndVertexData();

    unsigned short* pIndex = pPrim->BeginIndexData();
    unsigned int n = 0;
    for (unsigned int x = 0; x < uiNumX - 1; ++x)
    {
        unsigned int z;
        for (z = 0; z < uiNumZ; ++z)
        {
            pIndex[n++] = (unsigned short)(z + x * uiNumZ);
            pIndex[n++] = (unsigned short)(z + (x + 1) * uiNumZ);
        }
        if (x != uiNumX - 2)
        {
            pIndex[n++] = (unsigned short)(z + (x + 1) * uiNumZ - 1);
            pIndex[n++] = (unsigned short)((x + 1) * uiNumZ);
        }
    }
    pPrim->EndIndexData();
    return pPrim;
}

// FUNCTION: SWBF2 0x0071a040
template <>
pcRedPrimitive* RedWater::pcCreateDynamicPatchPrimitive<OceanVertex, 674>(
    PblVector3* pPositions, PblVector3* pNormals, unsigned int uiPointsX,
    unsigned int uiPointsZ, unsigned int uiStep)
{
    unsigned int uiNumX = (uiPointsX - 1) / uiStep + 1;
    unsigned int uiNumZ = (uiPointsZ - 1) / uiStep + 1;
    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 674,
                                                   sizeof(OceanVertex), uiNumX * uiNumZ,
                                                   2 * ((uiNumX - 1) * uiNumZ + uiNumX) - 4,
                                                   false);
    OceanVertex* pVert = (OceanVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return pPrim;
    }

    PblVector3 vMin(0.0f, m_fWaterHeightOffset + m_waterHeight + m_minPatchHeight, 0.0f);
    PblVector3 vSize(m_metersPerPatch + 0.001f, m_maxPatchHeight - m_minPatchHeight,
                     m_metersPerPatch + 0.001f);
    PblVector3 vMax(vMin.x + vSize.x, vMin.y + vSize.y, vMin.z + vSize.z);
    pPrim->m_bbox._vMin = vMin;
    pPrim->m_bbox._vMax = vMax;
    PblVector3 vBoxMin = {vMin.x, vMin.y, vMin.z};
    PblVector3 vExtent = {vMax.x - vMin.x, vMax.y - vMin.y,
                               vMax.z - vMin.z};
    PblVector3 vScale = vExtent * (1.0f / 65535.0f);
    PblVector3 vScaleInv = {1.0f / vScale.x, 1.0f / vScale.y,
                                 1.0f / vScale.z};
    CompressedPos::s_vMin = vBoxMin;
    CompressedPos::s_vScale = vScale;
    CompressedPos::s_vScaleInv = vScaleInv;
    for (unsigned int x = 0; x < uiPointsX; x += uiStep)
    {
        for (unsigned int z = 0; z < uiPointsZ; z += uiStep)
        {
            unsigned int i = z + x * m_pointsPerPatchZ;
            const PblVector3& pos = pPositions[i];
            const PblVector3& normal = pNormals[i];
            pVert->pos.x = pos.x * m_metersPerPatch;
            pVert->pos.y = pos.y * vExtent.y + vMin.y;
            pVert->pos.z = pos.z * m_metersPerPatch;
            pVert->normal = normal;
            pVert->u = pos.x;
            pVert->v = pos.z;
            pVert->color = RedColor::WHITE;
            pVert->color.a = m_foamIntensity[i];
            ++pVert;
        }
    }
    pPrim->EndVertexData();

    unsigned short* pIndex = pPrim->BeginIndexData();
    unsigned int n = 0;
    for (unsigned int x = 0; x < uiNumX - 1; ++x)
    {
        unsigned int z;
        for (z = 0; z < uiNumZ; ++z)
        {
            pIndex[n++] = (unsigned short)(z + x * uiNumZ);
            pIndex[n++] = (unsigned short)(z + (x + 1) * uiNumZ);
        }
        if (x != uiNumX - 2)
        {
            pIndex[n++] = (unsigned short)(z + (x + 1) * uiNumZ - 1);
            pIndex[n++] = (unsigned short)((x + 1) * uiNumZ);
        }
    }
    pPrim->EndIndexData();
    return pPrim;
}

static void pcWriteWaterPatchIndices(pcRedPrimitive* pPrim,
                                     unsigned int uiPointsX, unsigned int uiPointsZ)
{
    unsigned short* pIndex = pPrim->BeginIndexData();
    unsigned int n = 0;
    for (unsigned int x = 0; x < uiPointsX - 1; ++x)
    {
        unsigned int z;
        for (z = 0; z < uiPointsZ; ++z)
        {
            pIndex[n++] = (unsigned short)(z + x * uiPointsZ);
            pIndex[n++] = (unsigned short)(z + (x + 1) * uiPointsZ);
        }
        if (x != uiPointsX - 2)
        {
            pIndex[n++] = (unsigned short)(z + (x + 1) * uiPointsZ - 1);
            pIndex[n++] = (unsigned short)((x + 1) * uiPointsZ);
        }
    }
    pPrim->EndIndexData();
}

static void pcSetStaticWaterPatchBox(pcRedPrimitive* pPrim)
{
    PblVector3 vMin(0.0f, 0.0f, 0.0f);
    PblVector3 vMax(RedWater::m_metersPerPatch + 0.01f, 1.0f,
                    RedWater::m_metersPerPatch + 0.01f);
    pPrim->m_bbox._vMin = vMin;
    pPrim->m_bbox._vMax = vMax;
    PblVector3 vBoxMin = {0.0f, 0.0f, 0.0f};
    PblVector3 vExtent = {vMax.x - vMin.x, vMax.y - vMin.y,
                               vMax.z - vMin.z};
    PblVector3 vScale = vExtent * (1.0f / 65535.0f);
    PblVector3 vScaleInv = {1.0f / vScale.x, 65535.0f,
                                  1.0f / vScale.z};
    CompressedPos::s_vMin = vBoxMin;
    CompressedPos::s_vScale = vScale;
    CompressedPos::s_vScaleInv = vScaleInv;
}

// FUNCTION: SWBF2 0x0071a360
template <>
pcRedPrimitive* RedWater::pcCreateStaticPatchPrimitive<CompressedWaterVertex, 53794>(
    unsigned int uiPointsX, unsigned int uiPointsZ)
{
    pcRedPrimitive* pPrim = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 53794, sizeof(CompressedWaterVertex),
        uiPointsX * uiPointsZ,
        2 * ((uiPointsX - 1) * uiPointsZ + uiPointsX) - 4, true);
    CompressedWaterVertex* pVert =
        (CompressedWaterVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return pPrim;
    }
    pcSetStaticWaterPatchBox(pPrim);
    float fStepX = m_metersPerPatch / (float)(uiPointsX - 1);
    float fStepZ = m_metersPerPatch / (float)(uiPointsZ - 1);
    float fX = 0.0f;
    for (unsigned int x = 0; x < uiPointsX; ++x, fX += fStepX)
    {
        float fZ = 0.0f;
        for (unsigned int z = 0; z < uiPointsZ; ++z, fZ += fStepZ)
        {
            pVert->pos.x = (short)(fX * CompressedPos::s_vScaleInv.x - 32767.5f);
            pVert->pos.y = (short)(-32767.5f);
            pVert->pos.z = (short)(fZ * CompressedPos::s_vScaleInv.z - 32767.5f);
            pVert->normal.z = 127;
            pVert->normal.y = 255;
            pVert->normal.x = 127;
            pVert->uv.u = (short)(((float)x / (float)(uiPointsX - 1)) * 2048.0f);
            pVert->uv.v = (short)(((float)z / (float)(uiPointsZ - 1)) * 2048.0f);
            ++pVert;
        }
    }
    pPrim->EndVertexData();
    pcWriteWaterPatchIndices(pPrim, uiPointsX, uiPointsZ);
    return pPrim;
}

// FUNCTION: SWBF2 0x0071a6c0
template <>
pcRedPrimitive* RedWater::pcCreateStaticPatchPrimitive<WaterVertex, 546>(
    unsigned int uiPointsX, unsigned int uiPointsZ)
{
    pcRedPrimitive* pPrim = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 546, sizeof(WaterVertex), uiPointsX * uiPointsZ,
        2 * ((uiPointsX - 1) * uiPointsZ + uiPointsX) - 4, true);
    WaterVertex* pVert = (WaterVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return pPrim;
    }
    pcSetStaticWaterPatchBox(pPrim);
    float fStepX = m_metersPerPatch / (float)(uiPointsX - 1);
    float fStepZ = m_metersPerPatch / (float)(uiPointsZ - 1);
    float fX = 0.0f;
    for (unsigned int x = 0; x < uiPointsX; ++x, fX += fStepX)
    {
        float fZ = 0.0f;
        for (unsigned int z = 0; z < uiPointsZ; ++z, fZ += fStepZ)
        {
            pVert->pos.x = fX;
            pVert->pos.y = 0.0f;
            pVert->pos.z = fZ;
            pVert->normal.x = 0.0f;
            pVert->normal.y = 1.0f;
            pVert->normal.z = 0.0f;
            pVert->u = (float)x / (float)(uiPointsX - 1);
            pVert->v = (float)z / (float)(uiPointsZ - 1);
            ++pVert;
        }
    }
    pPrim->EndVertexData();
    pcWriteWaterPatchIndices(pPrim, uiPointsX, uiPointsZ);
    return pPrim;
}

struct TexCoord
{
    float u;
    float v;
};
struct CompressedWaterDistortionRingVertex
{
    CompressedPos pos;
    CompressedNormal normal;
    CompressedNormal binormal_s;
    CompressedNormal binormal_t;
    RedColor color;
    CompressedTexCoord uv;
};
struct WaterDistortionRingVertex
{
    PblVector3 pos;
    PblVector3 normal;
    PblVector3 binormal_s;
    PblVector3 binormal_t;
    RedColor color;
    TexCoord uv;
};
struct CompressedWaterSplashVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
struct WaterSplashVertex
{
    PblVector3 pos;
    RedColor color;
    TexCoord uv;
};
 
 
 
 
 
 

static void pcSetRingBoundingBox(pcRedPrimitive* pPrim, const PblVector3& vWorldMin,
                                 const PblVector3& vWorldMax)
{
    pPrim->m_bbox._vMin = vWorldMin;
    pPrim->m_bbox._vMax = vWorldMax;
    PblVector3 vBoxMin = {vWorldMin.x, vWorldMin.y, vWorldMin.z};
    PblVector3 vExtent = {vWorldMax.x - vWorldMin.x, vWorldMax.y - vWorldMin.y,
                               vWorldMax.z - vWorldMin.z};
    PblVector3 vScale = vExtent * (1.0f / 65535.0f);
    PblVector3 vScaleInv = {1.0f / vScale.x, 1.0f / vScale.y,
                                 1.0f / vScale.z};
    CompressedPos::s_vMin = vBoxMin;
    CompressedPos::s_vScale = vScale;
    CompressedPos::s_vScaleInv = vScaleInv;
}

static void pcSetPos(CompressedPos& out, const PblVector3& v)
{
    out.x = (short)((v.x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x
                    - 32767.5f);
    out.y = (short)((v.y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y
                    - 32767.5f);
    out.z = (short)((v.z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z
                    - 32767.5f);
}
static void pcSetPos(PblVector3& out, const PblVector3& v)
{
    out = v;
}
static void pcSetNormal(CompressedNormal& out, const PblVector3& v)
{
    out.z = (unsigned char)((v.z + 1.0f) * 127.5f);
    out.y = (unsigned char)((v.y + 1.0f) * 127.5f);
    out.x = (unsigned char)((v.x + 1.0f) * 127.5f);
}
static void pcSetNormal(PblVector3& out, const PblVector3& v)
{
    out = v;
}
static void pcSetTexCoord(CompressedTexCoord& out, float u, float v)
{
    out.u = (short)(u * 2048.0f);
    out.v = (short)(v * 2048.0f);
}
static void pcSetTexCoord(TexCoord& out, float u, float v)
{
    out.u = u;
    out.v = v;
}

template <class T>
static void pcSetDistortionRingVertex(T& vert, const PblVector3& vPos, float fSin, float fCos,
                                      float fDist, RedColor color, float u, float v)
{
    pcSetPos(vert.pos, PblVector3(vPos.x + fCos * fDist, vPos.y + WATERRING_HEIGHT_BIAS,
                                  vPos.z + fSin * fDist));
    pcSetNormal(vert.normal, PblVector3(0.0f, 1.0f, 0.0f));
    pcSetNormal(vert.binormal_s, PblVector3(-fSin, 0.0f, fCos));
    pcSetNormal(vert.binormal_t, PblVector3(-fCos, 0.0f, -fSin));
    vert.color = color;
    pcSetTexCoord(vert.uv, u, v);
}

static float pcRingAlphaScale(float fLifetime, float fAge)
{
    float fTime = fAge / fLifetime;
    if (fTime < WATERRING_MAX_ALPHA_AT_TIME)
    {
        return fTime * WATERRING_MAX_ALPHASCALE_BEFORE_TIME;
    }
    return (1.0f - fTime) * WATERRING_MAX_ALPHASCALE_AFTER_TIME;
}

// TEMPLATE: SWBF2 0x0071ae80
// RedWater::pcRenderDistortionRing<WaterDistortionRingVertex,738>

// FUNCTION: SWBF2 0x0071a9c0
template <class T, unsigned int FORMAT>
void RedWater::pcRenderDistortionRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                                      const PblVector3& vWorldMax, const PblVector3& vPos,
                                      float fRadius, float fLifetime, float fAge)
{
    float fAlphaScale = pcRingAlphaScale(fLifetime, fAge);
    float fU = 0.0f;
    if (fAlphaScale < 0.0f)
    {
        return;
    }
    float fAlpha = fLifetime * WATERRING_MAX_ALPHA / WATERRING_MAX_ALPHA_AT_LIFETIME * fAlphaScale;
    if (fAlpha > WATERRING_MAX_ALPHA)
    {
        fAlpha = WATERRING_MAX_ALPHA;
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, FORMAT, sizeof(T),
                                                   82, 0, false);
    pcSetRingBoundingBox(pPrim, vWorldMin, vWorldMax);
    T* pVert = (T*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }

    float fInner = fRadius - WATERDISTORTIONRING_INNERWIDTH;
    float fOuter = fRadius + WATERDISTORTIONRING_OUTERWIDTH;
    float fInnerV = 0.0f;
    if (fInner < 0.0f)
    {
        fInnerV = fInner / -(WATERDISTORTIONRING_INNERWIDTH + WATERDISTORTIONRING_OUTERWIDTH);
        fInner = 0.0f;
    }
    RedColor color = RedColor::WHITE;
    color.a = (unsigned char)(fAlpha * 255.0f);
    for (int i = 0; i <= 40; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 40.0f);
        pcSetDistortionRingVertex(pVert[0], vPos, fSin, fCos, fInner, color, fU, 1.0f - fInnerV);
        pcSetDistortionRingVertex(pVert[1], vPos, fSin, fCos, fOuter, color, fU, 0.0f);
        pVert += 2;
        fU += WATERRING_TEX_STEP;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
    pPrim->Release();
}

// TEMPLATE: SWBF2 0x0071bfb0
// RedWater::pcRenderDistortionWakeRing<WaterDistortionRingVertex,738>

// FUNCTION: SWBF2 0x0071b270
template <class T, unsigned int FORMAT>
void RedWater::pcRenderDistortionWakeRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                                          const PblVector3& vWorldMax, const PblVector3& vPos,
                                          float fRadius, const PblVector3& vVelocity,
                                          float fLifetime, float fAge)
{
    float fAlphaScale = pcRingAlphaScale(fLifetime, fAge);
    if (fAlphaScale < 0.0f)
    {
        return;
    }
    float fAlpha = fLifetime * WATERRING_MAX_ALPHA / WATERRING_MAX_ALPHA_AT_LIFETIME * fAlphaScale;
    if (fAlpha > WATERRING_MAX_ALPHA)
    {
        fAlpha = WATERRING_MAX_ALPHA;
    }

    float fSpeed = (float)sqrt(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y +
                               vVelocity.z * vVelocity.z);
    if (fSpeed < WATERRING_SPEED)
    {
        if (fSpeed <= 0.0f)
        {
            return;
        }
        fAlpha = WATERRING_SPEEDINV * fSpeed * fAlpha;
    }

    float fGroundSpeed = (float)sqrt(vVelocity.z * vVelocity.z + vVelocity.x * vVelocity.x);
    float fStretch = WATERRING_SPEEDINV * fGroundSpeed;
    PblVector3 vDir = vVelocity;
    float fFalloff = 150.0f;
    if (fGroundSpeed > 0.01f)
    {
        float fInv = 1.0f / fGroundSpeed;
        vDir.x *= fInv;
        vDir.z *= fInv;
        fFalloff = fInv * WATERRING_SPEED;
    }
    else
    {
        vDir = PblVector3(1.0f, 0.0f, 0.0f);
    }

    float fK = (fStretch - 1.0f) * (fStretch - 1.0f) * 4.0f + 0.4f;
    float fWakeAlpha = fK / (fK + 1.0f) * fAlpha;
    if (fWakeAlpha <= kInv255)
    {
        return;
    }

    float afAlpha[40];
    float afSin[40];
    float afCos[40];
    int iStart = -1;
    bool bStartFound = false;
    for (int i = 0; i < 40; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 40.0f);
        float fDot = vDir.z * fSin + vDir.x * fCos;
        float fShape;
        if (fStretch > 1.0f)
        {
            float fBehind = 1.0f - fDot * fStretch;
            fShape = 1.0f - fBehind * fFalloff * fBehind;
        }
        else
        {
            float fBehind = (1.0f - fDot) * fStretch;
            fShape = (1.0f - fBehind * fBehind) * (2.0f - fStretch);
        }
        float fSegAlpha = fShape * fWakeAlpha;
        if (fSegAlpha > kInv255)
        {
            if (iStart != -1)
            {
                bStartFound = true;
            }
        }
        else
        {
            fSegAlpha = 0.0f;
            if (!bStartFound)
            {
                iStart = i;
            }
        }
        afSin[i] = fSin;
        afCos[i] = fCos;
        afAlpha[i] = fSegAlpha;
    }
    if (!bStartFound)
    {
        iStart = 39;
    }

    unsigned int uiNumVertices = 0;
    bool bLastVisible = true;
    for (int i = 0; i <= 40; ++i)
    {
        if (i == 0 || afAlpha[(i + iStart) % 40] != 0.0f)
        {
            if (!bLastVisible)
            {
                uiNumVertices += 4;
            }
            uiNumVertices += 2;
            bLastVisible = true;
        }
        else
        {
            if (bLastVisible)
            {
                uiNumVertices += 2;
            }
            bLastVisible = false;
        }
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, FORMAT, sizeof(T),
                                                   uiNumVertices, 0, false);
    pcSetRingBoundingBox(pPrim, vWorldMin, vWorldMax);
    T* pVert = (T*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }

    float fInner = fRadius - WATERDISTORTIONRING_INNERWIDTH;
    float fOuter = fRadius + WATERDISTORTIONRING_OUTERWIDTH;
    float fInnerV = 0.0f;
    if (fInner < 0.0f)
    {
        fInnerV = fInner / -(WATERDISTORTIONRING_INNERWIDTH + WATERDISTORTIONRING_OUTERWIDTH);
        fInner = 0.0f;
    }

    float fU = (float)(iStart - 20) * WATERRING_TEX_STEP;
    float fLastU = 0.0f;
    int iLast = 0;
    int iPrev = 0;
    bLastVisible = true;
    for (int i = 0; i <= 40; ++i)
    {
        int iSeg = (i + iStart) % 40;
        bool bVisible = i == 0 || afAlpha[iSeg] != 0.0f;
        if (bVisible || bLastVisible)
        {
            if (!bLastVisible)
            {
                RedColor lastColor = RedColor::WHITE;
                lastColor.a = (unsigned char)(afAlpha[iLast] * 255.0f);
                RedColor prevColor = RedColor::WHITE;
                prevColor.a = (unsigned char)(afAlpha[iPrev] * 255.0f);
                pcSetDistortionRingVertex(pVert[0], vPos, afSin[iLast], afCos[iLast], fOuter,
                                          lastColor, fLastU, 0.0f);
                pcSetDistortionRingVertex(pVert[1], vPos, afSin[iPrev], afCos[iPrev], fInner,
                                          prevColor, fU - WATERRING_TEX_STEP, 1.0f - fInnerV);
                pcSetDistortionRingVertex(pVert[2], vPos, afSin[iPrev], afCos[iPrev], fInner,
                                          prevColor, fU - WATERRING_TEX_STEP, 1.0f - fInnerV);
                pcSetDistortionRingVertex(pVert[3], vPos, afSin[iPrev], afCos[iPrev], fOuter,
                                          prevColor, fU - WATERRING_TEX_STEP, 0.0f);
                pVert += 4;
            }

            RedColor color = RedColor::WHITE;
            color.a = (unsigned char)(afAlpha[iSeg] * 255.0f);
            pcSetDistortionRingVertex(pVert[0], vPos, afSin[iSeg], afCos[iSeg], fInner, color, fU,
                                      1.0f - fInnerV);
            pcSetDistortionRingVertex(pVert[1], vPos, afSin[iSeg], afCos[iSeg], fOuter, color, fU,
                                      0.0f);
            pVert += 2;
            iLast = iSeg;
            fLastU = fU;
            fU += WATERRING_TEX_STEP;
        }
        iPrev = iSeg;
        bLastVisible = bVisible;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
    pPrim->Release();
}

static void pcSetRingVertex(CompressedWaterRingVertex& vert, const PblVector3& vPos,
                            RedColor color, float u, float v)
{
    pcSetPos(vert.pos, vPos);
    vert.color = color;
    pcSetTexCoord(vert.uv, u, v);
}
static void pcSetRingVertex(WaterRingVertex& vert, const PblVector3& vPos, RedColor color,
                            float u, float v)
{
    vert.pos = vPos;
    vert.color = color;
    vert.u = u;
    vert.v = v;
}

template <class T>
static void pcSetWakeRingVertex(T& vert, const PblVector3& vPos, float fSin, float fCos,
                                float fDist, RedColor color, float u, float v)
{
    pcSetRingVertex(vert,
                    PblVector3(vPos.x + fCos * fDist, vPos.y + WATERRING_HEIGHT_BIAS,
                               vPos.z + fSin * fDist),
                    color, u, v);
}

// TEMPLATE: SWBF2 0x0071e9f0
// RedWater::pcRenderWakeRing<WaterRingVertex,642>

// FUNCTION: SWBF2 0x0071df70
template <class T, unsigned int FORMAT>
void RedWater::pcRenderWakeRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                                const PblVector3& vWorldMax, const PblVector3& vPos,
                                float fRadius, const PblVector3& vVelocity, float fLifetime,
                                float fAge)
{
    float fAlphaScale = pcRingAlphaScale(fLifetime, fAge);
    if (fAlphaScale < 0.0f)
    {
        return;
    }
    float fAlpha = fLifetime * WATERRING_MAX_ALPHA / WATERRING_MAX_ALPHA_AT_LIFETIME * fAlphaScale;
    if (fAlpha > WATERRING_MAX_ALPHA)
    {
        fAlpha = WATERRING_MAX_ALPHA;
    }

    float fSpeed = (float)sqrt(vVelocity.y * vVelocity.y + vVelocity.x * vVelocity.x +
                               vVelocity.z * vVelocity.z);
    if (fSpeed < WATERRING_SPEED)
    {
        if (fSpeed <= 0.0f)
        {
            return;
        }
        fAlpha = WATERRING_SPEEDINV * fSpeed * fAlpha;
    }

    float fGroundSpeed = (float)sqrt(vVelocity.z * vVelocity.z + vVelocity.x * vVelocity.x);
    float fStretch = WATERRING_SPEEDINV * fGroundSpeed;
    PblVector3 vDir = vVelocity;
    float fFalloff = 150.0f;
    if (fGroundSpeed > 0.01f)
    {
        float fInv = 1.0f / fGroundSpeed;
        vDir.x *= fInv;
        vDir.z *= fInv;
        fFalloff = fInv * WATERRING_SPEED;
    }
    else
    {
        vDir = PblVector3(1.0f, 0.0f, 0.0f);
    }

    float fK = (fStretch - 1.0f) * (fStretch - 1.0f) * 4.0f + 0.4f;
    float fWakeAlpha = fK / (fK + 1.0f) * fAlpha;
    if (fWakeAlpha <= kInv255)
    {
        return;
    }

    float afAlpha[40];
    float afSin[40];
    float afCos[40];
    int iStart = -1;
    bool bStartFound = false;
    for (int i = 0; i < 40; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 40.0f);
        float fDot = vDir.z * fSin + vDir.x * fCos;
        float fShape;
        if (fStretch > 1.0f)
        {
            float fBehind = 1.0f - fDot * fStretch;
            fShape = 1.0f - fBehind * fFalloff * fBehind;
        }
        else
        {
            float fBehind = (1.0f - fDot) * fStretch;
            fShape = (1.0f - fBehind * fBehind) * (2.0f - fStretch);
        }
        float fSegAlpha = fShape * fWakeAlpha;
        if (fSegAlpha > kInv255)
        {
            if (iStart != -1)
            {
                bStartFound = true;
            }
        }
        else
        {
            fSegAlpha = 0.0f;
            if (!bStartFound)
            {
                iStart = i;
            }
        }
        afSin[i] = fSin;
        afCos[i] = fCos;
        afAlpha[i] = fSegAlpha;
    }
    if (!bStartFound)
    {
        iStart = 39;
    }

    unsigned int uiNumVertices = 0;
    bool bLastVisible = true;
    for (int i = 0; i <= 40; ++i)
    {
        if (i == 0 || afAlpha[(i + iStart) % 40] != 0.0f)
        {
            if (!bLastVisible)
            {
                uiNumVertices += 4;
            }
            uiNumVertices += 2;
            bLastVisible = true;
        }
        else
        {
            if (bLastVisible)
            {
                uiNumVertices += 2;
            }
            bLastVisible = false;
        }
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, FORMAT, sizeof(T),
                                                   uiNumVertices, 0, false);
    pcSetRingBoundingBox(pPrim, vWorldMin, vWorldMax);
    T* pVert = (T*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }

    float fInner = fRadius - WATERRING_INNERWIDTH;
    float fOuter = fRadius + WATERRING_OUTERWIDTH;
    float fInnerV = 0.0f;
    if (fInner < 0.0f)
    {
        fInnerV = fInner / -(WATERRING_INNERWIDTH + WATERRING_OUTERWIDTH);
        fInner = 0.0f;
    }

    float fU = (float)(iStart - 20) * WATERRING_TEX_STEP;
    float fLastU = 0.0f;
    int iLast = 0;
    int iPrev = 0;
    bLastVisible = true;
    for (int i = 0; i <= 40; ++i)
    {
        int iSeg = (i + iStart) % 40;
        bool bVisible = i == 0 || afAlpha[iSeg] != 0.0f;
        if (bVisible || bLastVisible)
        {
            if (!bLastVisible)
            {
                RedColor lastColor = RedColor::WHITE;
                lastColor.a = (unsigned char)(afAlpha[iLast] * 255.0f);
                RedColor prevColor = RedColor::WHITE;
                prevColor.a = (unsigned char)(afAlpha[iPrev] * 255.0f);
                pcSetWakeRingVertex(pVert[0], vPos, afSin[iLast], afCos[iLast], fOuter,
                                    lastColor, fLastU, 0.0f);
                pcSetWakeRingVertex(pVert[1], vPos, afSin[iPrev], afCos[iPrev], fInner,
                                    prevColor, fU - WATERRING_TEX_STEP, 1.0f - fInnerV);
                pcSetWakeRingVertex(pVert[2], vPos, afSin[iPrev], afCos[iPrev], fInner,
                                    prevColor, fU - WATERRING_TEX_STEP, 1.0f - fInnerV);
                pcSetWakeRingVertex(pVert[3], vPos, afSin[iPrev], afCos[iPrev], fOuter,
                                    prevColor, fU - WATERRING_TEX_STEP, 0.0f);
                pVert += 4;
            }

            RedColor color = RedColor::WHITE;
            color.a = (unsigned char)(afAlpha[iSeg] * 255.0f);
            pcSetWakeRingVertex(pVert[0], vPos, afSin[iSeg], afCos[iSeg], fInner, color, fU,
                                1.0f - fInnerV);
            pcSetWakeRingVertex(pVert[1], vPos, afSin[iSeg], afCos[iSeg], fOuter, color, fU,
                                0.0f);
            pVert += 2;
            iLast = iSeg;
            fLastU = fU;
            fU += WATERRING_TEX_STEP;
        }
        iPrev = iSeg;
        bLastVisible = bVisible;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0x200, 0, 0, 0);
    pPrim->Release();
}

// FUNCTION: SWBF2 0x0071d1d0
template <>
void RedWater::pcRenderSplash<CompressedWaterSplashVertex, 37506>(
    pcRedShader* pShader, const PblVector3& vWorldMin, const PblVector3& vWorldMax,
    const PblVector3& vPos, float fRadius, float fAngle, float fHeight,
    const PblVector3& vVelocity, unsigned int uiSeed, float fAge)
{
    PblRandom rand(uiSeed ^ 0xC71C2830u);

    PblVector3 vDir(vVelocity.x, 0.0f, vVelocity.z);
    float fSpeed = (float)sqrt(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
    if (fSpeed > 2.0f)
    {
        float fInv = 1.0f / fSpeed;
        vDir.x = fInv * vVelocity.x;
        vDir.z = vVelocity.z * fInv;
        fSpeed -= 2.0f;
    }
    else
    {
        fSpeed = 0.0f;
        vDir = PblVector3(0.0f, 0.0f, 0.0f);
    }
    float fSpeedScale = fSpeed / (fSpeed + 5.0f);
    int iNumSegments = 6 - (int)(WATERSPLASH_MAX_SEGMENTS_RADIUSINV * fRadius * -14.0f);
    if (iNumSegments > 20)
    {
        iNumSegments = 20;
    }

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    float fRise = (fAge + 0.05f) * fHeight;
    float fReach = fRadius * 4.0f + 1.0f;
    float fTexStart = rand.GetFloat();
    if (fAngle > 1.0471976f)
    {
        fReach = fRadius * 4.0f * 4.0f + 1.0f;
    }
    float fFade = (fReach - fRise) / fReach;
    if (fFade <= 0.0f)
    {
        return;
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 37506,
                                                   sizeof(CompressedWaterSplashVertex),
                                                   2 * iNumSegments + 2, 0, false);
    pcSetRingBoundingBox(pPrim, vWorldMin, vWorldMax);
    CompressedWaterSplashVertex* pVert =
        (CompressedWaterSplashVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }

    float fBaseAlpha = fFade - 0.5f;
    if (fBaseAlpha > 0.6f)
    {
        fBaseAlpha = 0.6f;
    }
    else if (fBaseAlpha < 0.0f)
    {
        fBaseAlpha = 0.0f;
    }
    float fTopAlpha = fFade;
    if (fTopAlpha > 0.6f)
    {
        fTopAlpha = 0.6f;
    }
    RedColor baseColor = WATERSPLASH_COLOR;
    baseColor.a = (unsigned char)(fBaseAlpha * 255.0f);
    RedColor topColor = WATERSPLASH_COLOR;
    topColor.a = (unsigned char)(fTopAlpha * 255.0f);

    float fU = fTexStart;
    float fDrop = m_fWaterHeightOffset - fAge * 5.0f * fAge;
    float fDirScale = fSpeedScale * 2.0f;
    float fDirBias = 1.0f - fSpeedScale;
    float fBaseOut = fRise * 0.4f;
    short iBaseV = (short)((0.98f - fRise * 0.3f / fReach) * 2048.0f);
    for (int i = 0; i <= iNumSegments; ++i)
    {
        float fSegSin, fSegCos;
        PblMath::SinCos(&fSegSin, &fSegCos, (float)i * PblMath::fTwoPi / (float)iNumSegments);
        float fLength = ((vDir.z * fSegSin + vDir.x * fSegCos) * fDirScale + fDirBias) *
                        (rand.GetFloat() * fRise);
        if (fLength < 0.0f)
        {
            fLength = 0.0f;
        }
        float fBase = fBaseOut * fCos + fRadius;
        float fTop = fLength * fCos + fRadius;
        float fTopY = fLength * fSin + fDrop;
        if (fTopY < 0.0f)
        {
            fTopY = 0.0f;
        }

        pcSetPos(pVert[0].pos, PblVector3(vPos.x + fSegCos * fBase, vPos.y + 0.0f,
                                          vPos.z + fSegSin * fBase));
        pVert[0].uv.u = (short)(fU * 2048.0f);
        pVert[0].uv.v = iBaseV;
        pVert[0].color = baseColor;
        pcSetPos(pVert[1].pos, PblVector3(vPos.x + fSegCos * fTop, vPos.y + fTopY,
                                          vPos.z + fSegSin * fTop));
        pVert[1].uv.v = 40;
        pVert[1].uv.u = (short)(fU * 2048.0f);
        pVert[1].color = topColor;
        pVert += 2;
        if (fLength == 0.0f)
        {
            fU = fTexStart - (float)(iNumSegments - i) * 0.1f;
        }
        fU += 0.1f;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0, 0, 0, 0);
    pPrim->Release();
}

// FUNCTION: SWBF2 0x0071d900
template <>
void RedWater::pcRenderSplash<WaterSplashVertex, 642>(
    pcRedShader* pShader, const PblVector3& vWorldMin, const PblVector3& vWorldMax,
    const PblVector3& vPos, float fRadius, float fAngle, float fHeight,
    const PblVector3& vVelocity, unsigned int uiSeed, float fAge)
{
    PblRandom rand(uiSeed ^ 0xC71C2830u);

    PblVector3 vDir(vVelocity.x, 0.0f, vVelocity.z);
    float fSpeed = (float)sqrt(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
    if (fSpeed > 2.0f)
    {
        float fInv = 1.0f / fSpeed;
        vDir.x = fInv * vVelocity.x;
        vDir.z = vVelocity.z * fInv;
        fSpeed -= 2.0f;
    }
    else
    {
        fSpeed = 0.0f;
        vDir = PblVector3(0.0f, 0.0f, 0.0f);
    }
    float fSpeedScale = fSpeed / (fSpeed + 5.0f);
    int iNumSegments = 6 - (int)(WATERSPLASH_MAX_SEGMENTS_RADIUSINV * fRadius * -14.0f);
    if (iNumSegments > 20)
    {
        iNumSegments = 20;
    }

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    float fRise = (fAge + 0.05f) * fHeight;
    float fReach = fRadius * 4.0f + 1.0f;
    float fTexStart = rand.GetFloat();
    if (fAngle > 1.0471976f)
    {
        fReach = fRadius * 4.0f * 4.0f + 1.0f;
    }
    float fFade = (fReach - fRise) / fReach;
    if (fFade <= 0.0f)
    {
        return;
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 642,
                                                   sizeof(WaterSplashVertex),
                                                   2 * iNumSegments + 2, 0, false);
    pcSetRingBoundingBox(pPrim, vWorldMin, vWorldMax);
    WaterSplashVertex* pVert = (WaterSplashVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }

    float fBaseAlpha = fFade - 0.5f;
    if (fBaseAlpha > 0.6f)
    {
        fBaseAlpha = 0.6f;
    }
    else if (fBaseAlpha < 0.0f)
    {
        fBaseAlpha = 0.0f;
    }
    float fTopAlpha = fFade;
    if (fTopAlpha > 0.6f)
    {
        fTopAlpha = 0.6f;
    }
    RedColor baseColor = WATERSPLASH_COLOR;
    baseColor.a = (unsigned char)(fBaseAlpha * 255.0f);
    RedColor topColor = WATERSPLASH_COLOR;
    topColor.a = (unsigned char)(fTopAlpha * 255.0f);

    float fU = fTexStart;
    float fDrop = m_fWaterHeightOffset - fAge * 5.0f * fAge;
    float fDirScale = fSpeedScale * 2.0f;
    float fDirBias = 1.0f - fSpeedScale;
    float fBaseOut = fRise * 0.4f;
    float fBaseV = 0.98f - fRise * 0.3f / fReach;
    for (int i = 0; i <= iNumSegments; ++i)
    {
        float fSegSin, fSegCos;
        PblMath::SinCos(&fSegSin, &fSegCos, (float)i * PblMath::fTwoPi / (float)iNumSegments);
        float fLength = ((vDir.z * fSegSin + vDir.x * fSegCos) * fDirScale + fDirBias) *
                        (rand.GetFloat() * fRise);
        if (fLength < 0.0f)
        {
            fLength = 0.0f;
        }
        float fBase = fBaseOut * fCos + fRadius;
        float fTop = fLength * fCos + fRadius;
        float fTopY = fLength * fSin + fDrop;
        if (fTopY < 0.0f)
        {
            fTopY = 0.0f;
        }

        pVert[0].pos = PblVector3(vPos.x + fSegCos * fBase, vPos.y + 0.0f,
                                  vPos.z + fSegSin * fBase);
        pVert[0].color = baseColor;
        pVert[0].uv.u = fU;
        pVert[0].uv.v = fBaseV;
        pVert[1].pos = PblVector3(vPos.x + fSegCos * fTop, vPos.y + fTopY,
                                  vPos.z + fSegSin * fTop);
        pVert[1].color = topColor;
        pVert[1].uv.u = fU;
        pVert[1].uv.v = 0.01f;
        pVert += 2;
        if (fLength == 0.0f)
        {
            fU = fTexStart - (float)(iNumSegments - i) * 0.1f;
        }
        fU += 0.1f;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim, pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE, 0, 0, 0, 0);
    pPrim->Release();
}

// FUNCTION: SWBF2 0x00720c40
void RedWater::pcRegisterReflection()
{
    if (!bEnable)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    RedCamera camera(*pCamera);
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    fFar = (m_farSceneRange == 0.0f) ? FLRenderer::GetFarSceneRange() : m_farSceneRange;
    if (fFar > FLRenderer::GetNearSceneFadeStart())
    {
        camera.SetNearFarPlane(fNear, fFar);
    }
    BuildPatchCache(&camera, false, m_bEnableVisibilityTest ? VISIBILITYTEST_GETIDS : VISIBILITYTEST_NONE);

    if (m_patchCacheSize <= 0)
    {
        return;
    }
    if (RedRenderer::pcGetRenderOptions().waterQuality != QUALITY_LOW)
    {
        PblMatrix mPatch = gMatrixIdentity;
        PblMatrix mMesh = gMatrixIdentity;
        mMesh.m[3][1] = m_fWaterHeightOffset;

        PblPlane plane;
        plane.x = 0.0f;
        plane.y = 1.0f;
        plane.z = 0.0f;
        plane.w = -(m_waterHeight + m_fWaterHeightOffset);

        m_reflectionID = FLRenderer::RegisterReflection(
            reinterpret_cast<Renderable*>(&_WaterRenderable), plane,
            RedRenderer::pcGetRenderOptions().waterQuality == QUALITY_MEDIUM ? 0x400000 : 0);
        s_pPreReflectionShader->SetReflectionID(m_reflectionID);

        for (int i = 0; i < m_patchCacheSize; ++i)
        {
            WaterPatch& rPatch = m_patchCache[i];
            if (m_bEnableVisibilityTest)
            {

                bool bVisible = rPatch.uiVisibilityFarID && RedWater::pcGetVisibilityTestResult(rPatch.uiVisibilityFarID) != 0;
                if (!bVisible && rPatch.uiVisibilityID)
                {
                    RedWater::pcGetVisibilityTestResult(rPatch.uiVisibilityID);
                }
            }
            FLRenderer::AddReflectionObject(m_reflectionID, rPatch.bbox, true);
            if (rPatch.pCustomMesh)
            {
                RedRenderer::pcRenderPrimitive((pcRedPrimitive*)rPatch.pCustomMesh,
                                               s_pPreReflectionShader,
                                               mMesh,
                                               RedColor::WHITE, 0, 0, 0, 0);
            }
            else
            {
                mPatch.m[3][0] = rPatch.bbox._vMin.x;
                mPatch.m[3][1] = rPatch.bbox._vMin.y;
                mPatch.m[3][2] = rPatch.bbox._vMin.z;
                mPatch.m[3][3] = 1.0f;
                RedRenderer::pcRenderPrimitive(m_pLowResPrim,
                                               s_pPreReflectionShader,
                                               mPatch,
                                               RedColor::WHITE, 0, 0, 0, 0);
            }
        }
    }
    m_uiVisibilityCacheSize = 0;
    m_uiVisibilityCacheSize_FarScene = 0;
}

// FUNCTION: SWBF2 0x0071f270
void RedWater::PlatformCleanup()
{
    RedRenderer::pcUnregisterResetFunction(pcReset);
    if (m_aVisibilityCache)
    {
        for (int i = 0; i < 1024; ++i)
        {
            if (m_aVisibilityCache[i].pVisibilityQuery)
            {
                m_aVisibilityCache[i].pVisibilityQuery->Release();
            }
        }
        delete[] m_aVisibilityCache;
        m_aVisibilityCache = 0;
    }
    if (m_pHighResPrim)
    {
        m_pHighResPrim->Release();
        m_pHighResPrim = 0;
    }
    if (m_pLowResPrim)
    {
        m_pLowResPrim->Release();
        m_pLowResPrim = 0;
    }
    if (s_pPreReflectionShader)
    {
        s_pPreReflectionShader->Release();
        s_pPreReflectionShader = 0;
    }
    if (s_pNormalWaterShader)
    {
        s_pNormalWaterShader->Release();
        s_pNormalWaterShader = 0;
    }
    if (s_pOceanWaterShader)
    {
        s_pOceanWaterShader->Release();
        s_pOceanWaterShader = 0;
    }
    if (s_pNormalMapAdderShader)
    {
        s_pNormalMapAdderShader->Release();
        s_pNormalMapAdderShader = 0;
    }
    if (s_pWaterSplashShader)
    {
        s_pWaterSplashShader->Release();
        s_pWaterSplashShader = 0;
    }
    if (s_pWaterRingShader)
    {
        s_pWaterRingShader->Release();
        s_pWaterRingShader = 0;
    }
    if (s_pWaterDistortionRingShader)
    {
        s_pWaterDistortionRingShader->Release();
        s_pWaterDistortionRingShader = 0;
    }
    if (s_pUnderWaterShader)
    {
        s_pUnderWaterShader->Release();
        s_pUnderWaterShader = 0;
    }
}

void pcWaterShader::SetReflectionColor(RedColor color)
{
    s_reflectionColor.r = color.r * kInv255;
    s_reflectionColor.g = color.g * kInv255;
    s_reflectionColor.b = color.b * kInv255;
    s_reflectionColor.a = color.a * kInv255;
}

void pcWaterShader::SetRefractionColor(RedColor color)
{
    s_refractionColor.r = color.r * kInv255;
    s_refractionColor.g = color.g * kInv255;
    s_refractionColor.b = color.b * kInv255;
    s_refractionColor.a = color.a * kInv255;
}

void pcWaterShader::SetSpecularEnable(bool bEnable)
{
    s_specularEnable = bEnable;
}

// FUNCTION: SWBF2 0x00720500
void pcWaterShader::SetFresnelMinMax(float fMin, float fMax)
{
    if (fMax < 0.0f)
    {
        fMax = 0.0f;
    }
    else if (fMax > 1.0f)
    {
        fMax = 1.0f;
    }
    if (fMin < 0.0f)
    {
        fMin = 0.0f;
    }
    else if (fMin > 1.0f)
    {
        fMin = 1.0f;
    }
    s_fresnelMinMax.r = 0.0f;
    s_fresnelMinMax.g = 0.0f;
    s_fresnelMinMax.b = fMin;
    s_fresnelMinMax.a = fMax;
}

// FUNCTION: SWBF2 0x0071f520
void RedWater::PlatformPostReadInit()
{
    if (!m_waterMap)
    {
        return;
    }

    GeometryModeT geometry = RedRenderer::pcGetGeometryMode();
    m_aVisibilityCache = 0;
    m_uiVisibilityCacheSize = 0;
    m_uiVisibilityCacheSize_FarScene = 0;
    m_bEnableVisibilityTest = false;
    RedRenderer::pcRegisterResetFunction(pcReset);

    if (m_waterMode == WATER_MODE_NORMAL)
    {
        unsigned int uiRingFormat;
        unsigned int uiPatchFormat;
        unsigned int uiDistortionRingFormat;
        if (geometry == GEOMETRYMODE_COMPRESSED)
        {
            uiRingFormat = 0x9282;
            uiPatchFormat = 0xD222;
            uiDistortionRingFormat = 0xD2E2;
            m_pHighResPrim = pcCreateStaticPatchPrimitive<CompressedWaterVertex, 53794>(
                m_pointsPerPatchX, m_pointsPerPatchZ);
            m_pLowResPrim = pcCreateStaticPatchPrimitive<CompressedWaterVertex, 53794>(
                m_divisionsPerPatchX / RedWater::m_highResLowResRatio + 1, m_divisionsPerPatchZ / RedWater::m_highResLowResRatio + 1);
        }
        else
        {
            uiRingFormat = 0x282;
            uiPatchFormat = 0x222;
            uiDistortionRingFormat = 0x2E2;
            m_pHighResPrim = pcCreateStaticPatchPrimitive<WaterVertex, 546>(
                m_pointsPerPatchX, m_pointsPerPatchZ);
            m_pLowResPrim = pcCreateStaticPatchPrimitive<WaterVertex, 546>(
                m_divisionsPerPatchX / RedWater::m_highResLowResRatio + 1, m_divisionsPerPatchZ / RedWater::m_highResLowResRatio + 1);
        }

        s_pUnderWaterShader = (pcClearShader*)pcRedShader::Create(
            kuiUnderWaterShader, 3, 0, 0, 4, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pPreReflectionShader = (pcPreReflectionShader*)pcRedShader::Create(
            kuiPreReflectionShader, uiPatchFormat, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pNormalMapAdderShader = (pcNormalMapAdderShader*)pcRedShader::Create(
            kuiNormalMapAdderShader, uiPatchFormat, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pWaterSplashShader = pcRedShader::Create(kuiWaterSplashShader, uiRingFormat,
                                                   &kuiWaterSplashTextureHash, 1, 0x50004,
                                                   RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pWaterRingShader = pcRedShader::Create(kuiWaterSplashShader, uiRingFormat,
                                                 &kuiWaterRingTextureHash, 1, 0x10004,
                                                 RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pWaterDistortionRingShader = (pcNormalMapAdderShader*)pcRedShader::Create(
            kuiNormalMapAdderShader, uiDistortionRingFormat, 0, 0, 0, RedColor::WHITE,
            RedColor::BLACK, 32.0f);
        s_pOceanWaterShader = 0;
        s_pWaterDistortionRingShader->m_iShaderState = 1;
        s_pNormalWaterShader = (pcWaterShader*)pcRedShader::Create(
            kuiWaterShader, uiPatchFormat, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);

        pcWaterShader::SetRefractionColor(m_refractionColor);
        pcWaterShader::SetReflectionColor(m_reflectionColor);
        pcWaterShader::SetFresnelMinMax(m_fresnelMin, m_fresnelMax);

        if (RedRenderer::pcGetRenderCaps().pipeline != PIPELINE_FIXED &&
            pcDeviceSupportsVisibilityTesting())
        {
            m_aVisibilityCache = new VisibilityCacheEntryT[1024];
            memset(m_aVisibilityCache, 0, 1024 * sizeof(VisibilityCacheEntryT));
            m_bEnableVisibilityTest = true;
        }
    }
    else
    {
        s_pOceanWaterShader = (pcOceanShader*)pcRedShader::Create(
            kuiOceanShader, geometry == GEOMETRYMODE_COMPRESSED ? 0xD2A2 : 0x2A2,
            &m_uiTextureHash, 1, 0x10000, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pPreReflectionShader = 0;
        s_pNormalMapAdderShader = 0;
        s_pWaterSplashShader = 0;
        s_pWaterDistortionRingShader = 0;
        s_pWaterRingShader = 0;
    }

    float fScale = (float)m_reductionFactor;
    m_fSpecularMaskTileU = fScale * m_fSpecularMaskTileU;
    m_fSpecularMaskTileV = fScale * m_fSpecularMaskTileV;
    m_fSpecularMaskScrollU = m_fSpecularMaskScrollU / fScale;
    m_fSpecularMaskScrollV = m_fSpecularMaskScrollV / fScale;
}

// FUNCTION: SWBF2 0x0071f9b0
void RedWater::PlatformPreReadInit()
{
    m_reflectionColor.b = 255;
    m_reflectionColor.g = 255;
    m_reflectionColor.r = 255;
    m_reflectionColor.a = 255;
    m_refractionColor = m_reflectionColor;
    m_pHighResPrim = 0;
    m_pLowResPrim = 0;
    m_underwaterColor = m_reflectionColor;
    m_fresnelMin = 0.2f;
    m_fresnelMax = 0.9f;
    m_uiNumBumpMapTextures = 0;
    m_uiNumSpecularMaskTextures = 0;
    m_uiNumNormalMapTextures = 0;
    m_fSpecularMaskTileU = 1.0f;
    m_fSpecularMaskTileV = 1.0f;
    m_fSpecularMaskScrollU = 0.0f;
    m_fSpecularMaskScrollV = 0.0f;
    m_farSceneRange = 0.0f;
}

// FUNCTION: SWBF2 0x00720450
static RedColor ReadColor(PblConfig::Data& data)
{
    RedColor color;
    float f = data.GetFloatArg(0);
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    color.r = (unsigned char)(int)f;
    f = data.GetFloatArg(1);
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    color.g = (unsigned char)(int)f;
    f = data.GetFloatArg(2);
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    color.b = (unsigned char)(int)f;
    if (data.GetNumArgs() > 3)
    {
        f = data.GetFloatArg(3);
        if (f < 0.0f)
        {
            f = 0.0f;
        }
        else if (f > 255.0f)
        {
            f = 255.0f;
        }
        color.a = (unsigned char)(int)f;
    }
    else
    {
        color.a = 255;
    }
    return color;
}

static void ReadTextureList(PblConfig::Data& data, unsigned int* puiTextures,
                            unsigned int& ruiNumTextures, float& rfFrameRate)
{
    char buffer[256];
    ruiNumTextures = (int)data.GetFloatArg(1);
    for (unsigned int i = 0; i < ruiNumTextures; ++i)
    {
        sprintf(buffer, "%s%d", data.GetStringArg(0), i);
        puiTextures[i] = PblHash(buffer);
    }
    rfFrameRate = data.GetFloatArg(2);
}

// FUNCTION: SWBF2 0x0071fa60
void RedWater::PlatformReadConfigData(PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case 0x8A81DE8B:
        m_reflectionColor = ReadColor(data);
        break;
    case 0x5AA5A3BD:
    case 0x72E43B50:
        m_refractionColor = ReadColor(data);
        break;
    case 0x362C5FFF:
        m_underwaterColor = ReadColor(data);
        break;
    case 0x8F7CFA82:
        m_fresnelMin = data.GetFloatArg(0);
        m_fresnelMax = data.GetFloatArg(1);
        break;
    case 0x01192B7B:
        m_farSceneRange = data.GetFloatArg(0);
        break;
    case 0xE938342C:
        m_fSpecularMaskTileU = data.GetFloatArg(0);
        m_fSpecularMaskTileV = data.GetFloatArg(1);
        break;
    case 0xDD976C44:
        m_fSpecularMaskScrollU = data.GetFloatArg(0);
        m_fSpecularMaskScrollV = data.GetFloatArg(1);
        break;
    case 0xC0311180:
        ReadTextureList(data, m_uiSpecularMaskTextures, m_uiNumSpecularMaskTextures,
                        m_fSpecularMaskTexturesFrameRate);
        break;
    case 0x11D65039:
        ReadTextureList(data, m_uiBumpMapTextures, m_uiNumBumpMapTextures,
                        m_fBumpMapTexturesFrameRate);
        break;
    case 0xEAC587AC:
        ReadTextureList(data, m_uiNormalMapTextures, m_uiNumNormalMapTextures,
                        m_fNormalMapTexturesFrameRate);
        break;
    }
}

// FUNCTION: SWBF2 0x0071fd70
void* RedWater::PlatformReadCustomPatch(PblFileChunk* pChunk)
{
    if (!pChunk->HasMoreChildren())
    {
        return 0;
    }

    PblFileChunk patch = pChunk->ReadNextChild();
    if (patch._iId != 0x4354504C)
    {
        return 0;
    }

    PblBox bbox;
    bbox._vMin.x = bbox._vMin.y = bbox._vMin.z = 0.0f;
    bbox._vMax.x = bbox._vMax.y = bbox._vMax.z = 0.0f;
    pcRedVertexBuffer* pVertexBuffer = 0;
    pcRedIndexBuffer* pIndexBuffer = 0;
    unsigned int uiVertexFormat = 0;
    unsigned int uiNumVertices = 0;
    unsigned int uiStartVertex = 0;
    unsigned int uiStartIndex = 0;
    unsigned int uiNumIndices = 0;

    while (patch.HasMoreChildren())
    {
        PblFileChunk child = patch.ReadNextChild();
        switch (child._iId)
        {
        case 0x46554249:
            uiNumIndices = child._pFile->Read32();
            if (uiNumIndices)
            {
                pIndexBuffer = pcRedIndexBuffer::Create(uiNumIndices, true, true);
                unsigned short* pIndices = pIndexBuffer->Lock(uiNumIndices, uiStartIndex);
                child._pFile->ReadData(pIndices, uiNumIndices * 2);
                pIndexBuffer->Unlock();
            }
            break;
        case 0x46554256:
        {
            unsigned int uiCount = child._pFile->Read32();
            unsigned int uiStride = child._pFile->Read32();
            unsigned int uiFormat = child._pFile->Read32();
            if (!pVertexBuffer &&
                (RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED ||
                 !(uiFormat & 0xF000)))
            {
                uiNumVertices = uiCount;
                uiVertexFormat = uiFormat;
                if (uiCount)
                {
                    pVertexBuffer = pcRedVertexBuffer::Create(uiCount, uiStride, true, true);
                    void* pVertices = pVertexBuffer->Lock(uiCount, uiStartVertex);
                    child._pFile->ReadData(pVertices, uiStride * uiCount);
                    pVertexBuffer->Unlock();
                }
            }
            break;
        }
        case 0x4F464E49:
        {
            float fMinX = child._pFile->ReadFloat();
            float fMaxX = child._pFile->ReadFloat();
            float fMinY = child._pFile->ReadFloat();
            float fMaxY = child._pFile->ReadFloat();
            float fMinZ = child._pFile->ReadFloat();
            float fMaxZ = child._pFile->ReadFloat();
            bbox._vMin.x = fMinX;
            bbox._vMin.y = fMinY;
            bbox._vMin.z = fMinZ;
            bbox._vMax.x = fMaxX;
            bbox._vMax.y = fMaxY;
            bbox._vMax.z = fMaxZ;
            break;
        }
        }
    }

    if (pVertexBuffer && pIndexBuffer && (uiVertexFormat & 0xFFFF0FFF) == 0x222)
    {
        pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, uiNumIndices - 2, uiVertexFormat, uiNumVertices,
            uiStartVertex, pVertexBuffer, uiStartIndex, pIndexBuffer);
        pPrimitive->m_bbox = bbox;
        pIndexBuffer->Release();
        pVertexBuffer->Release();
        return pPrimitive;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00720020
void RedWater::PlatformRender(unsigned int uiFlags)
{
    if (!bEnable || !m_waterMap)
    {
        return;
    }

    if (m_waterMode != WATER_MODE_NORMAL)
    {
        if (uiFlags & 0x20000)
        {
            pcRenderFarOceanWaterPatches();
        }
        else
        {
            pcRenderOceanWaterPatches();
        }
        return;
    }

    if (RedRenderer::pcGetRenderOptions().waterQuality == QUALITY_HIGH &&
        RedRenderer::GetCamera()->_Matrix.m[3][1] -
                (m_fWaterHeightOffset + m_waterHeight) <= 0.0f)
    {
        pcWaterShader::SetFresnelMinMax(1.0f, 1.0f);
    }
    else
    {
        pcWaterShader::SetFresnelMinMax(m_fresnelMin, m_fresnelMax);
    }
    pcWaterShader::SetSpecularEnable(RedWater::m_specularEnable);

    if (uiFlags & 0x20000)
    {
        if (!(uiFlags & 0x100000))
        {
            pcRenderFarWaterPatches();
        }
        return;
    }
    if (uiFlags & 0x100000)
    {
        pcRegisterReflection();
        return;
    }

    BuildPatchCache(RedRenderer::GetCamera(), false,
                    m_bEnableVisibilityTest ? VISIBILITYTEST_ASSIGNID : VISIBILITYTEST_NONE);
    if (m_patchCacheSize <= 0)
    {
        return;
    }
    UpdateEventCache();
    if (RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
    {
        pcRenderWaterPatches();
        if (RedRenderer::pcGetRenderOptions().waterQuality == QUALITY_LOW)
        {
            return;
        }
        pcRenderWaterRings();
    }
    else
    {
        if (RedRenderer::pcGetRenderOptions().waterQuality != QUALITY_LOW)
        {
            pcAccumulateNormalMap();
        }
        pcRenderWaterPatches();
        pcRenderWaterRings();
    }
    pcRenderWaterSplashes();
}

// FUNCTION: SWBF2 0x00720170
void RedWater::PlatformRenderUnderWaterView(PblVector3* pVerts, int iNumVerts)
{
    if (!bEnable)
    {
        return;
    }

    pcRedPrimitive* pPrimitive =
        pcRedPrimitive::Create(D3DPT_TRIANGLEFAN, 3, 16, iNumVerts, 0, false);
    float* pOut = (float*)pPrimitive->BeginVertexData();
    if (!pOut)
    {
        return;
    }

    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);
    for (int i = 0; i < iNumVerts; ++i)
    {
        pOut[0] = (pVerts[i].x * 0.5f + 0.5f) * (fMaxX - fMinX);
        pOut[1] = (0.5f - pVerts[i].y * 0.5f) * (fMaxY - fMinY);
        pOut[2] = 0.0f;
        pOut[3] = 1.0f;
        pOut += 4;
    }
    pPrimitive->EndVertexData();

    float fR = m_underwaterColor.r * kInv255;
    float fG = m_underwaterColor.g * kInv255;
    float fB = m_underwaterColor.b * kInv255;
    float fScale = RedRenderer::GetLightingScale();
    RedColor color;
    color.r = (unsigned char)(int)(fR * fScale * 255.0f);
    color.a = m_underwaterColor.a;
    color.b = (unsigned char)(int)(fB * fScale * 255.0f);
    color.g = (unsigned char)(int)(fG * fScale * 255.0f);
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pUnderWaterShader, gMatrixIdentity,
                                   color, 0x200, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x00720340
void RedWater::PlatformUpdatePatchGeometry(PblVector3* pMin, PblVector3* pMax)
{
    if (!bEnable)
    {
        return;
    }

    if (m_pHighResPrim)
    {
        m_pHighResPrim->Release();
        m_pHighResPrim = 0;
    }
    if (m_pLowResPrim)
    {
        m_pLowResPrim->Release();
        m_pLowResPrim = 0;
    }
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        m_pHighResPrim = pcCreateDynamicPatchPrimitive<CompressedOceanVertex, 53922>(
            pMin, pMax, m_pointsPerPatchX, m_pointsPerPatchZ, 1);
        m_pLowResPrim = pcCreateDynamicPatchPrimitive<CompressedOceanVertex, 53922>(
            pMin, pMax, m_pointsPerPatchX, m_pointsPerPatchZ, RedWater::m_highResLowResRatio);
    }
    else
    {
        m_pHighResPrim = pcCreateDynamicPatchPrimitive<OceanVertex, 674>(
            pMin, pMax, m_pointsPerPatchX, m_pointsPerPatchZ, 1);
        m_pLowResPrim = pcCreateDynamicPatchPrimitive<OceanVertex, 674>(
            pMin, pMax, m_pointsPerPatchX, m_pointsPerPatchZ, RedWater::m_highResLowResRatio);
    }
}

namespace FLRenderer
{

RedTexture* pcGetReflectionTexture(char id);
}

void pcWaterShader::SetSpecularDirection(const PblVector3& vDirection)
{
    s_vSpecularDirection = vDirection;
}

void pcWaterShader::SetReflectionMap(RedTexture* pTexture)
{
    m_reflectionMap = pTexture;
}

void pcWaterShader::SetProjectedNormalMap(RedTexture* pTexture)
{
    m_projectedNormalMap = pTexture;
}

void pcNormalMapAdderShader::SetNormalMap(RedTexture* pTex1, RedTexture* pTex2, float fBlend)
{
    m_normalMap1 = pTex1;
    m_normalMap2 = pTex2;
    m_blend = fBlend;
}

void pcNormalMapAdderShader::SetTextureTransform(RedTextureTransform* pTransform1,
                                                 RedTextureTransform* pTransform2)
{
    if (pTransform1)
    {
        m_textureTransform1.m_bTransform = pTransform1->m_bTransform;
        if (m_textureTransform1.m_bTransform)
        {
            memcpy(m_textureTransform1.m_Transform, pTransform1->m_Transform,
                   sizeof(m_textureTransform1.m_Transform));
        }
    }
    else
    {
        m_textureTransform1.m_bTransform = 0;
    }
    if (pTransform2)
    {
        m_textureTransform2.m_bTransform = pTransform2->m_bTransform;
        if (m_textureTransform2.m_bTransform)
        {
            memcpy(m_textureTransform2.m_Transform, pTransform2->m_Transform,
                   sizeof(m_textureTransform2.m_Transform));
        }
    }
    else
    {
        m_textureTransform2.m_bTransform = 0;
    }
}

// FUNCTION: SWBF2 0x00720570
void RedWater::pcAccumulateNormalMap()
{
    PblMatrix mCustomPatch = gMatrixIdentity;
    PblMatrix mPatch = mCustomPatch;
    mCustomPatch.m[3][1] = m_fWaterHeightOffset;
    float fTime = m_waterTimeStamp.GetElapsed();

    RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();
    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    RedRenderer::pcSetRenderTarget(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP), 0);
    RedRenderer::pcSetViewport(viewport);
    RedRenderer::pcClear(D3DCLEAR_TARGET, RedColor::BLACK, 1.0f, 0);

    RedTextureTransform transform;
    transform.m_bTransform = 1;
    transform.m_Transform[0][0] = m_tilesU;
    transform.m_Transform[0][1] = 0.0f;
    transform.m_Transform[1][0] = 0.0f;
    transform.m_Transform[1][1] = m_tilesV;
    transform.m_Transform[2][0] = m_tilesU * 0.0f;
    transform.m_Transform[2][1] = m_tilesV * 0.0f;
    float fFrame = m_fNormalMapTexturesFrameRate * fTime;
    int iFrame = (int)fFrame;
    s_pNormalMapAdderShader->SetNormalMap(
        RedTexture::Find(m_uiNormalMapTextures[iFrame % m_uiNumNormalMapTextures]),
        RedTexture::Find(m_uiNormalMapTextures[(iFrame + 1) % m_uiNumNormalMapTextures]),
        fFrame - (float)iFrame);
    s_pNormalMapAdderShader->SetTextureTransform(&transform, &transform);

    for (int i = 0; i < m_patchCacheSize; ++i)
    {
        WaterPatch& patch = m_patchCache[i];
        if (patch.pCustomMesh)
        {
            RedRenderer::pcRenderPrimitive((pcRedPrimitive*)patch.pCustomMesh, s_pNormalMapAdderShader,
                                           mCustomPatch, RedColor::WHITE, 0x200, 0, 0, 0);
        }
        else
        {
            mPatch.m[3][0] = patch.bbox._vMin.x;
            mPatch.m[3][1] = patch.bbox._vMin.y;
            mPatch.m[3][2] = patch.bbox._vMin.z;
            mPatch.m[3][3] = 1.0f;
            RedRenderer::pcRenderPrimitive(m_pLowResPrim, s_pNormalMapAdderShader, mPatch,
                                           RedColor::WHITE, 0x200, 0, 0, 0);
        }
    }

    GeometryModeT geometry = RedRenderer::pcGetRenderCaps().geometry;
    const RedCamera* pCamera = RedRenderer::GetCamera();
    s_pWaterDistortionRingShader->SetNormalMap(RedTexture::Find(kuiWaterDistortionRingTextureHash),
                                               0, 0.5f);
    for (int i = 0; i < m_numWaterEvents; ++i)
    {
        WaterEvent& event = m_eventCache[i];
        if (event.type == WATEREVENT_SPLASH)
        {
            continue;
        }

        float fAge = m_waterTimeStamp.GetElapsed() - event.startTime;
        float fRadius = fAge * 1.5f + event.radius;
        float fDX = pCamera->_Matrix.m[3][0] - event.worldPos.x;
        float fDY = pCamera->_Matrix.m[3][1] - event.worldPos.y;
        float fDZ = pCamera->_Matrix.m[3][2] - event.worldPos.z;
        if (fDY * fDY + fDX * fDX + fDZ * fDZ < WATEREVENT_MAX_VISIBLE_DISTANCESQR)
        {
            ++event.in_range;
        }
        if (pCamera->ClassifySphere(event.worldPos, fRadius, -1.0f) == RedCamera::VISIBILITY_NONE)
        {
            continue;
        }

        PblVector3 vPos(event.worldPos.x, m_waterHeight + m_fWaterHeightOffset, event.worldPos.z);
        float fLifetime = event.expireTime - event.startTime;
        if (geometry == GEOMETRYMODE_COMPRESSED)
        {
            if (event.type != WATEREVENT_RING)
            {
                pcRenderDistortionWakeRing<CompressedWaterDistortionRingVertex, 53986>(
                    s_pWaterDistortionRingShader, m_worldMin, m_worldMax, vPos, fRadius,
                    event.worldVelocity, fLifetime, fAge);
            }
            else
            {
                pcRenderDistortionRing<CompressedWaterDistortionRingVertex, 53986>(
                    s_pWaterDistortionRingShader, m_worldMin, m_worldMax, vPos, fRadius,
                    fLifetime, fAge);
            }
        }
        else
        {
            if (event.type != WATEREVENT_RING)
            {
                pcRenderDistortionWakeRing<WaterDistortionRingVertex, 738>(
                    s_pWaterDistortionRingShader, m_worldMin, m_worldMax, vPos, fRadius,
                    event.worldVelocity, fLifetime, fAge);
            }
            else
            {
                pcRenderDistortionRing<WaterDistortionRingVertex, 738>(
                    s_pWaterDistortionRingShader, m_worldMin, m_worldMax, vPos, fRadius,
                    fLifetime, fAge);
            }
        }
    }

    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::pcSetViewport(viewport);
}

void pcOceanShader::SetBaseMap(RedTexture* pTexture, RedTextureTransform* pTransform)
{
    m_baseMap = pTexture;
    if (pTransform)
    {
        m_textureTransform.m_bTransform = pTransform->m_bTransform;
        if (m_textureTransform.m_bTransform)
        {
            memcpy(m_textureTransform.m_Transform, pTransform->m_Transform,
                   sizeof(m_textureTransform.m_Transform));
        }
    }
    else
    {
        m_textureTransform.m_bTransform = 0;
    }
}

void pcOceanShader::SetFoamMap(RedTexture* pTexture, RedTextureTransform* pTransform)
{
    m_foamMap = pTexture;
    if (pTransform)
    {
        m_foamMapTransform.m_bTransform = pTransform->m_bTransform;
        if (m_foamMapTransform.m_bTransform)
        {
            memcpy(m_foamMapTransform.m_Transform, pTransform->m_Transform,
                   sizeof(m_foamMapTransform.m_Transform));
        }
    }
    else
    {
        m_foamMapTransform.m_bTransform = 0;
    }
}

// FUNCTION: SWBF2 0x00720f80
void RedWater::pcRenderFarOceanWaterPatches()
{
    if (!bEnable)
    {
        return;
    }

    PblMatrix mPatch = gMatrixIdentity;
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fTime = m_waterTimeStamp.GetElapsed();
    float fScrollU = m_velocityU * fTime;
    float fScrollV = m_velocityV * fTime;
    fScrollU -= (float)(int)fScrollU;
    fScrollV -= (float)(int)fScrollV;

    RedTextureTransform baseTransform;
    baseTransform.m_bTransform = 1;
    baseTransform.m_Transform[0][0] = m_tilesU;
    baseTransform.m_Transform[0][1] = 0.0f;
    baseTransform.m_Transform[1][0] = 0.0f;
    baseTransform.m_Transform[1][1] = m_tilesV;
    baseTransform.m_Transform[2][0] = m_tilesU * fScrollU;
    baseTransform.m_Transform[2][1] = m_tilesV * fScrollV;
    RedTextureTransform foamTransform;
    foamTransform.m_bTransform = 1;
    foamTransform.m_Transform[0][0] = RedWater::m_foamTilesU;
    foamTransform.m_Transform[0][1] = 0.0f;
    foamTransform.m_Transform[1][0] = 0.0f;
    foamTransform.m_Transform[1][1] = RedWater::m_foamTilesV;
    foamTransform.m_Transform[2][0] = RedWater::m_foamTilesU * fScrollU;
    foamTransform.m_Transform[2][1] = RedWater::m_foamTilesV * fScrollV;
    s_pOceanWaterShader->SetBaseMap(RedTexture::Find(m_uiTextureHash), &baseTransform);
    s_pOceanWaterShader->SetFoamMap(RedTexture::Find(RedWater::m_uiFoamTextureHash), &foamTransform);

    float fNear = FLRenderer::GetNearSceneFadeStart();
    float fFar = m_farSceneRange;
    if (fFar == 0.0f)
    {
        fFar = FLRenderer::GetFarSceneRange();
    }
    RedCamera camera(*pCamera);
    if (fFar > FLRenderer::GetNearSceneFadeStart())
    {
        camera.SetNearFarPlane(fNear, fFar);
    }
    BuildPatchCache(&camera, true, VISIBILITYTEST_NONE);

    if (m_patchCacheSize > 0)
    {
        float fHeight = m_minPatchHeight + (m_maxPatchHeight - m_minPatchHeight) * 0.5f;
        for (int i = 0; i < m_patchCacheSize; ++i)
        {
            mPatch.m[3][0] = m_patchCache[i].bbox._vMin.x;
            mPatch.m[3][1] = fHeight;
            mPatch.m[3][2] = m_patchCache[i].bbox._vMin.z;
            mPatch.m[3][3] = 1.0f;
            RedRenderer::pcRenderPrimitive(m_pLowResPrim,
                                           s_pOceanWaterShader,
                                           mPatch,
                                           RedColor::WHITE, 0, 0, 0,
                                           0);
        }
    }
}

// FUNCTION: SWBF2 0x00721c10
void RedWater::pcRenderWaterSplashes()
{
    if (m_numWaterEvents <= 0)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    GeometryModeT geometry = RedRenderer::pcGetGeometryMode();
    for (int i = 0; i < m_numWaterEvents; ++i)
    {
        WaterEvent& event = m_eventCache[i];
        if (event.type != WATEREVENT_SPLASH)
        {
            continue;
        }

        float fAge = m_waterTimeStamp.GetElapsed() - event.startTime;
        float fRadius = event.radius * 4.0f + 1.0f;
        float fDX = pCamera->_Matrix.m[3][0] - event.worldPos.x;
        float fDY = pCamera->_Matrix.m[3][1] - event.worldPos.y;
        float fDZ = pCamera->_Matrix.m[3][2] - event.worldPos.z;
        if (fDX * fDX + fDY * fDY + fDZ * fDZ < WATEREVENT_MAX_VISIBLE_DISTANCESQR)
        {
            ++event.in_range;
        }
        if (pCamera->ClassifySphere(event.worldPos, fRadius, -1.0f) == RedCamera::VISIBILITY_NONE)
        {
            continue;
        }

        unsigned int uiSeed = (unsigned int)(event.startTime * 4096.0f);
        PblVector3 vPos(event.worldPos.x, m_fWaterHeightOffset + m_waterHeight, event.worldPos.z);
        if (geometry == GEOMETRYMODE_COMPRESSED)
        {
            pcRenderSplash<CompressedWaterSplashVertex, 37506>(
                s_pWaterSplashShader, m_worldMin, m_worldMax, vPos, event.radius,
                WATERSPLASH_OUTER_ANGLE, event.radius * 2.0f + 5.0f, event.worldVelocity, uiSeed,
                fAge);
            if (event.radius >= 1.0f)
            {
                pcRenderSplash<CompressedWaterSplashVertex, 37506>(
                    s_pWaterSplashShader, m_worldMin, m_worldMax, vPos, event.radius * 0.3f,
                    WATERSPLASH_INNER_ANGLE, event.radius * 3.0f + 5.0f, event.worldVelocity,
                    uiSeed + 0x81234567, fAge);
            }
        }
        else
        {
            pcRenderSplash<WaterSplashVertex, 642>(
                s_pWaterSplashShader, m_worldMin, m_worldMax, vPos, event.radius,
                WATERSPLASH_OUTER_ANGLE, event.radius * 2.0f + 5.0f, event.worldVelocity, uiSeed,
                fAge);
            if (event.radius >= 1.0f)
            {
                pcRenderSplash<WaterSplashVertex, 642>(
                    s_pWaterSplashShader, m_worldMin, m_worldMax, vPos, event.radius * 0.3f,
                    WATERSPLASH_INNER_ANGLE, event.radius * 3.0f + 5.0f, event.worldVelocity,
                    uiSeed + 0x81234567, fAge);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00721f00
void RedWater::pcReset(ResetFlagsT eFlags)
{
    if (!(eFlags & RESETFLAGS_FREE_RESOURCES))
    {
        return;
    }
    if (!m_aVisibilityCache)
    {
        return;
    }
    for (int i = 0; i < 1024; ++i)
    {
        if (m_aVisibilityCache[i].pVisibilityQuery)
        {
            m_aVisibilityCache[i].pVisibilityQuery->Release();
            m_aVisibilityCache[i].pVisibilityQuery = 0;
        }
    }
}

// FUNCTION: SWBF2 0x00721f60
void RedWater::pcSetupShaders()
{
    pcWaterShader::SetSpecularDirection(m_vSpecularDirection);
    float fTime = m_waterTimeStamp.GetElapsed();
    s_bWaterShaderSetupPending = false;

    float fScrollU = m_velocityU * fTime;
    float fScrollV = m_velocityV * fTime;
    RedTextureTransform transform1;
    transform1.m_bTransform = 1;
    transform1.m_Transform[0][0] = m_tilesU;
    transform1.m_Transform[0][1] = 0.0f;
    transform1.m_Transform[1][0] = 0.0f;
    transform1.m_Transform[1][1] = m_tilesV;
    transform1.m_Transform[2][0] = m_tilesU * (fScrollU - (float)(int)fScrollU);
    transform1.m_Transform[2][1] = (fScrollV - (float)(int)fScrollV) * m_tilesV;
    RedTextureTransform transform2;
    transform2.m_bTransform = 1;
    transform2.m_Transform[0][0] = m_tilesU;
    transform2.m_Transform[0][1] = 0.0f;
    transform2.m_Transform[1][0] = 0.0f;
    transform2.m_Transform[1][1] = m_tilesV;
    transform2.m_Transform[2][0] = m_tilesU * (-fScrollU - (float)(int)-fScrollU);
    transform2.m_Transform[2][1] = (-fScrollV - (float)(int)-fScrollV) * m_tilesV;

    if (!s_pNormalWaterShader)
    {
        return;
    }

    s_pNormalWaterShader->SetDiffuseMap(RedTexture::Find(m_uiTextureHash), &transform1,
                                        RedTexture::Find(m_uiTextureHash), &transform2, 0.5f);

    float fMaskU = m_fSpecularMaskScrollU * fTime;
    float fMaskV = m_fSpecularMaskScrollV * fTime;
    transform1.m_bTransform = 1;
    transform1.m_Transform[0][0] = m_fSpecularMaskTileU;
    transform1.m_Transform[0][1] = 0.0f;
    transform1.m_Transform[1][0] = 0.0f;
    transform1.m_Transform[1][1] = m_fSpecularMaskTileV;
    transform1.m_Transform[2][0] = m_fSpecularMaskTileU * (fMaskU - (float)(int)fMaskU);
    transform1.m_Transform[2][1] = (fMaskV - (float)(int)fMaskV) * m_fSpecularMaskTileV;
    if (m_uiNumSpecularMaskTextures)
    {
        float fFrame = m_fSpecularMaskTexturesFrameRate * fTime;
        int iFrame = (int)fFrame;
        s_pNormalWaterShader->SetSpecularMask(
            RedTexture::Find(m_uiSpecularMaskTextures[iFrame % m_uiNumSpecularMaskTextures]),
            &transform1,
            RedTexture::Find(m_uiSpecularMaskTextures[(iFrame + 1) % m_uiNumSpecularMaskTextures]),
            &transform1, fFrame - (float)iFrame);
    }

    transform1.m_bTransform = 1;
    transform1.m_Transform[0][0] = m_tilesU;
    transform1.m_Transform[0][1] = 0.0f;
    transform1.m_Transform[1][0] = 0.0f;
    transform1.m_Transform[1][1] = m_tilesV;
    transform1.m_Transform[2][0] = m_tilesU * 0.0f;
    transform1.m_Transform[2][1] = m_tilesV * 0.0f;
    if (m_uiNumBumpMapTextures)
    {
        float fFrame = m_fBumpMapTexturesFrameRate * fTime;
        int iFrame = (int)fFrame;
        s_pNormalWaterShader->SetBumpMap(
            RedTexture::Find(m_uiBumpMapTextures[iFrame % m_uiNumBumpMapTextures]), &transform1,
            RedTexture::Find(m_uiBumpMapTextures[(iFrame + 1) % m_uiNumBumpMapTextures]),
            &transform1, fFrame - (float)iFrame);
    }
    if (m_uiNumNormalMapTextures)
    {
        float fFrame = m_fNormalMapTexturesFrameRate * fTime;
        int iFrame = (int)fFrame;
        s_pNormalWaterShader->SetNormalMap(
            RedTexture::Find(m_uiNormalMapTextures[iFrame % m_uiNumNormalMapTextures]),
            &transform1,
            RedTexture::Find(m_uiNormalMapTextures[(iFrame + 1) % m_uiNumNormalMapTextures]),
            &transform1, fFrame - (float)iFrame);
    }

    s_pNormalWaterShader->SetReflectionMap(FLRenderer::pcGetReflectionTexture(m_reflectionID));
    s_pNormalWaterShader->SetProjectedNormalMap(RedRenderer::pcGetBuffer(PC_BUFFER_TEMP));
}

// FUNCTION: SWBF2 0x00721810
void RedWater::pcRenderWaterPatches()
{
    if (!bEnable)
    {
        return;
    }
    PblMatrix mWorld = gMatrixIdentity;
    mWorld.m[3][1] = m_fWaterHeightOffset;
    pcSetupShaders();
    unsigned short uiLastVisibilityID = 0;
    for (int i = 0; i < m_patchCacheSize; ++i)
    {
        WaterPatch& patch = m_patchCache[i];
        if (m_bEnableVisibilityTest && uiLastVisibilityID != patch.uiVisibilityID)
        {
            if (uiLastVisibilityID)
            {
                RedWater::pcEndVisibilityTest(uiLastVisibilityID);
            }
            uiLastVisibilityID = patch.uiVisibilityID;
            if (uiLastVisibilityID)
            {
                RedWater::pcBeginVisibilityTest(uiLastVisibilityID);
            }
        }
        if (patch.pCustomMesh)
        {
            RedRenderer::pcRenderPrimitive((pcRedPrimitive*)patch.pCustomMesh,
                                           s_pNormalWaterShader,
                                           mWorld,
                                           RedColor::WHITE,
                                           0x200, 0, 0, 0);
        }
        else
        {
            PblMatrix mPatch = mWorld;
            mPatch.m[3][0] = patch.bbox._vMin.x;
            mPatch.m[3][1] = patch.bbox._vMin.y;
            mPatch.m[3][2] = patch.bbox._vMin.z;
            mPatch.m[3][3] = 1.0f;
            RedRenderer::pcRenderPrimitive(m_pHighResPrim,
                                           s_pNormalWaterShader,
                                           mPatch,
                                           RedColor::WHITE,
                                           0x200, 0, 0, 0);
        }
    }
    if (uiLastVisibilityID)
    {
        RedWater::pcEndVisibilityTest(uiLastVisibilityID);
    }
}

struct CompressedWaterRingVertex;
struct WaterRingVertex;
template <>
void RedWater::pcRenderRing<CompressedWaterRingVertex, 37506>(pcRedShader*, const PblVector3&,
                                                              const PblVector3&, const PblVector3&,
                                                              float, float, float);
template <>
void RedWater::pcRenderRing<WaterRingVertex, 642>(pcRedShader*, const PblVector3&, const PblVector3&,
                                                  const PblVector3&, float, float, float);

// FUNCTION: SWBF2 0x007219a0
void RedWater::pcRenderWaterRings()
{
    const RedCamera* pCamera = RedRenderer::GetCamera();
    GeometryModeT geometry = RedRenderer::pcGetRenderCaps().geometry;
    for (int i = 0; i < m_numWaterEvents; ++i)
    {
        WaterEvent& event = m_eventCache[i];
        if (event.type == WATEREVENT_SPLASH)
        {
            continue;
        }
        float fAge = m_waterTimeStamp.GetElapsed() - event.startTime;
        float fRadius = fAge * 1.5f + event.radius;
        float fDX = pCamera->_Matrix.m[3][0] - event.worldPos.x;
        float fDY = pCamera->_Matrix.m[3][1] - event.worldPos.y;
        float fDZ = pCamera->_Matrix.m[3][2] - event.worldPos.z;
        if (fDX * fDX + fDY * fDY + fDZ * fDZ < WATEREVENT_MAX_VISIBLE_DISTANCESQR)
        {
            ++event.in_range;
        }
        if (pCamera->ClassifySphere(event.worldPos, fRadius, -1.0f) == RedCamera::VISIBILITY_NONE)
        {
            continue;
        }
        PblVector3 vPos(event.worldPos.x, m_waterHeight + m_fWaterHeightOffset,
                        event.worldPos.z);
        float fLifetime = event.expireTime - event.startTime;
        if (geometry == GEOMETRYMODE_COMPRESSED)
        {
            if (event.type != WATEREVENT_RING)
            {
                pcRenderWakeRing<CompressedWaterRingVertex, 37506>(
                    s_pWaterRingShader, m_worldMin, m_worldMax, vPos, fRadius,
                    event.worldVelocity, fLifetime, fAge);
            }
            else
            {
                pcRenderRing<CompressedWaterRingVertex, 37506>(s_pWaterRingShader, m_worldMin, m_worldMax, vPos,
                            fRadius, fLifetime, fAge);
            }
        }
        else
        {
            if (event.type != WATEREVENT_RING)
            {
                pcRenderWakeRing<WaterRingVertex, 642>(
                    s_pWaterRingShader, m_worldMin, m_worldMax, vPos, fRadius,
                    event.worldVelocity, fLifetime, fAge);
            }
            else
            {
                pcRenderRing<WaterRingVertex, 642>(s_pWaterRingShader, m_worldMin, m_worldMax, vPos,
                            fRadius, fLifetime, fAge);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0071ca70
template <>
void RedWater::pcRenderRing<CompressedWaterRingVertex, 37506>(pcRedShader* pShader, const PblVector3& vWorldMin,
                        const PblVector3& vWorldMax, const PblVector3& vPos,
                        float fRadius, float fLifetime, float fAge)
{
    float fRatio = fAge / fLifetime;
    float fFade = fRatio < 0.15f ? fRatio * 6.6666665f
                                 : (1.0f - fRatio) * 1.1764705f;
    if (fFade < 0.0f)
    {
        return;
    }
    float fAlpha = (fLifetime * 0.75f / 3.0f) * fFade;
    if (fAlpha > 0.75f)
    {
        fAlpha = 0.75f;
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 37506,
                                                   sizeof(CompressedWaterRingVertex),
                                                   82, 0, false);
    pPrim->m_bbox._vMin = vWorldMin;
    pPrim->m_bbox._vMax = vWorldMax;
    PblVector3 vBoxMin = {vWorldMin.x, vWorldMin.y, vWorldMin.z};
    PblVector3 vExtent = {vWorldMax.x - vWorldMin.x,
                               vWorldMax.y - vWorldMin.y,
                               vWorldMax.z - vWorldMin.z};
    PblVector3 vScale = vExtent * (1.0f / 65535.0f);
    PblVector3 vScaleInv = {1.0f / vScale.x, 1.0f / vScale.y,
                                 1.0f / vScale.z};
    CompressedPos::s_vMin = vBoxMin;
    CompressedPos::s_vScale = vScale;
    CompressedPos::s_vScaleInv = vScaleInv;

    CompressedWaterRingVertex* pVert =
        (CompressedWaterRingVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }
    float fInner = fRadius - 0.35f;
    float fOuter = fRadius + 0.35f;
    float fInnerV = 1.0f;
    if (fInner < 0.0f)
    {
        fInnerV += fInner / 0.7f;
        fInner = 0.0f;
    }
    RedColor color = RedColor::WHITE;
    color.a = (unsigned char)(fAlpha * 255.0f);
    float fU = 0.0f;
    for (int i = 0; i <= 40; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * 6.2831855f / 40.0f);
        PblVector3 inner(vPos.x + fCos * fInner, vPos.y + 0.01f,
                         vPos.z + fSin * fInner);
        PblVector3 outer(vPos.x + fCos * fOuter, vPos.y + 0.01f,
                         vPos.z + fSin * fOuter);
        pVert[0].pos.x = (short)((inner.x - vWorldMin.x) * vScaleInv.x - 32767.5f);
        pVert[0].pos.y = (short)((inner.y - vWorldMin.y) * vScaleInv.y - 32767.5f);
        pVert[0].pos.z = (short)((inner.z - vWorldMin.z) * vScaleInv.z - 32767.5f);
        pVert[0].color = color;
        pVert[0].uv.u = (short)(fU * 2048.0f);
        pVert[0].uv.v = (short)(fInnerV * 2048.0f);
        pVert[1].pos.x = (short)((outer.x - vWorldMin.x) * vScaleInv.x - 32767.5f);
        pVert[1].pos.y = (short)((outer.y - vWorldMin.y) * vScaleInv.y - 32767.5f);
        pVert[1].pos.z = (short)((outer.z - vWorldMin.z) * vScaleInv.z - 32767.5f);
        pVert[1].color = color;
        pVert[1].uv.u = (short)(fU * 2048.0f);
        pVert[1].uv.v = 0;
        pVert += 2;
        fU += 0.25f;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim,
                                   pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE,
                                   0x200, 0, 0, 0);
    pPrim->Release();
}

// FUNCTION: SWBF2 0x0071ce70
template <>
void RedWater::pcRenderRing<WaterRingVertex, 642>(pcRedShader* pShader, const PblVector3& vWorldMin,
                        const PblVector3& vWorldMax, const PblVector3& vPos,
                        float fRadius, float fLifetime, float fAge)
{
    float fRatio = fAge / fLifetime;
    float fFade = fRatio < 0.15f ? fRatio * 6.6666665f
                                 : (1.0f - fRatio) * 1.1764705f;
    if (fFade < 0.0f)
    {
        return;
    }
    float fAlpha = (fLifetime * 0.75f / 3.0f) * fFade;
    if (fAlpha > 0.75f)
    {
        fAlpha = 0.75f;
    }

    pcRedPrimitive* pPrim = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 642,
                                                   sizeof(WaterRingVertex), 82, 0, false);
    pPrim->m_bbox._vMin = vWorldMin;
    pPrim->m_bbox._vMax = vWorldMax;
    PblVector3 vBoxMin = {vWorldMin.x, vWorldMin.y, vWorldMin.z};
    PblVector3 vExtent = {vWorldMax.x - vWorldMin.x,
                               vWorldMax.y - vWorldMin.y,
                               vWorldMax.z - vWorldMin.z};
    PblVector3 vScale = vExtent * (1.0f / 65535.0f);
    PblVector3 vScaleInv = {1.0f / vScale.x, 1.0f / vScale.y,
                                 1.0f / vScale.z};
    CompressedPos::s_vMin = vBoxMin;
    CompressedPos::s_vScale = vScale;
    CompressedPos::s_vScaleInv = vScaleInv;

    WaterRingVertex* pVert = (WaterRingVertex*)pPrim->BeginVertexData();
    if (!pVert)
    {
        return;
    }
    float fInner = fRadius - 0.35f;
    float fOuter = fRadius + 0.35f;
    float fInnerV = 1.0f;
    if (fInner < 0.0f)
    {
        fInnerV += fInner / 0.7f;
        fInner = 0.0f;
    }
    RedColor color = RedColor::WHITE;
    color.a = (unsigned char)(fAlpha * 255.0f);
    float fU = 0.0f;
    for (int i = 0; i <= 40; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * 6.2831855f / 40.0f);
        pVert[0].pos.x = vPos.x + fCos * fInner;
        pVert[0].pos.y = vPos.y + 0.01f;
        pVert[0].pos.z = vPos.z + fSin * fInner;
        pVert[0].color = color;
        pVert[0].u = fU;
        pVert[0].v = fInnerV;
        pVert[1].pos.x = vPos.x + fCos * fOuter;
        pVert[1].pos.y = vPos.y + 0.01f;
        pVert[1].pos.z = vPos.z + fSin * fOuter;
        pVert[1].color = color;
        pVert[1].u = fU;
        pVert[1].v = 0.0f;
        pVert += 2;
        fU += 0.25f;
    }
    pPrim->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrim,
                                   pShader,
                                   gMatrixIdentity,
                                   RedColor::WHITE,
                                   0x200, 0, 0, 0);
    pPrim->Release();
}

// FUNCTION: SWBF2 0x00721300
void RedWater::pcRenderFarWaterPatches()
{
    if (!bEnable)
    {
        return;
    }

    PblMatrix mWorld = gMatrixIdentity;
    PblMatrix mPatch = mWorld;
    mWorld.m[3][1] = m_fWaterHeightOffset;
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fNear = FLRenderer::GetNearSceneFadeStart();
    float fFar = m_farSceneRange;
    if (fFar == 0.0f)
    {
        fFar = FLRenderer::GetFarSceneRange();
    }
    RedCamera camera(*pCamera);
    camera.SetNearFarPlane(fNear, fFar);
    m_uiVisibilityCacheSize = 0;
    m_uiVisibilityCacheSize_FarScene = 0;
    BuildPatchCache(&camera, false,
                    m_bEnableVisibilityTest ? VISIBILITYTEST_ASSIGNID : VISIBILITYTEST_NONE);
    m_uiVisibilityCacheSize_FarScene = m_uiVisibilityCacheSize;

    if (m_patchCacheSize > 0)
    {
        pcSetupShaders();
        unsigned short uiLastVisibilityID = 0;
        for (int i = 0; i < m_patchCacheSize; ++i)
        {
            WaterPatch& patch = m_patchCache[i];
            if (m_bEnableVisibilityTest && uiLastVisibilityID != patch.uiVisibilityID)
            {
                if (uiLastVisibilityID)
                {
                    RedWater::pcEndVisibilityTest(uiLastVisibilityID);
                }
                uiLastVisibilityID = patch.uiVisibilityID;
                if (uiLastVisibilityID)
                {
                    RedWater::pcBeginVisibilityTest(uiLastVisibilityID);
                }
            }
            if (patch.pCustomMesh)
            {
                RedRenderer::pcRenderPrimitive((pcRedPrimitive*)patch.pCustomMesh,
                                               s_pNormalWaterShader,
                                               mWorld,
                                               RedColor::WHITE,
                                               0x200, 0, 0, 0);
            }
            else
            {
                mPatch.m[3][0] = patch.bbox._vMin.x;
                mPatch.m[3][1] = patch.bbox._vMin.y;
                mPatch.m[3][2] = patch.bbox._vMin.z;
                mPatch.m[3][3] = 1.0f;
                RedRenderer::pcRenderPrimitive(m_pLowResPrim,
                                               s_pNormalWaterShader,
                                               mPatch,
                                               RedColor::WHITE,
                                               0x200, 0, 0, 0);
            }
        }
        if (uiLastVisibilityID)
        {
            RedWater::pcEndVisibilityTest(uiLastVisibilityID);
        }
    }
}

// FUNCTION: SWBF2 0x00721590
void RedWater::pcRenderOceanWaterPatches()
{
    if (!bEnable)
    {
        return;
    }

    PblMatrix mPatch = gMatrixIdentity;
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fTime = m_waterTimeStamp.GetElapsed();
    float fScrollU = m_velocityU * fTime;
    float fScrollV = m_velocityV * fTime;
    fScrollU -= (float)(int)fScrollU;
    fScrollV -= (float)(int)fScrollV;

    RedTextureTransform baseTransform;
    baseTransform.m_bTransform = 1;
    baseTransform.m_Transform[0][0] = m_tilesU;
    baseTransform.m_Transform[0][1] = 0.0f;
    baseTransform.m_Transform[1][0] = 0.0f;
    baseTransform.m_Transform[1][1] = m_tilesV;
    baseTransform.m_Transform[2][0] = m_tilesU * fScrollU;
    baseTransform.m_Transform[2][1] = m_tilesV * fScrollV;
    RedTextureTransform foamTransform;
    foamTransform.m_bTransform = 1;
    foamTransform.m_Transform[0][0] = RedWater::m_foamTilesU;
    foamTransform.m_Transform[0][1] = 0.0f;
    foamTransform.m_Transform[1][0] = 0.0f;
    foamTransform.m_Transform[1][1] = RedWater::m_foamTilesV;
    foamTransform.m_Transform[2][0] = RedWater::m_foamTilesU * fScrollU;
    foamTransform.m_Transform[2][1] = RedWater::m_foamTilesV * fScrollV;
    s_pOceanWaterShader->SetBaseMap(RedTexture::Find(m_uiTextureHash), &baseTransform);
    s_pOceanWaterShader->SetFoamMap(RedTexture::Find(RedWater::m_uiFoamTextureHash), &foamTransform);
    BuildPatchCache(pCamera, true, VISIBILITYTEST_NONE);

    if (m_patchCacheSize > 0)
    {
        float fHeight = m_minPatchHeight + (m_maxPatchHeight - m_minPatchHeight) * 0.5f;
        for (int i = 0; i < m_patchCacheSize; ++i)
        {
            mPatch.m[3][0] = m_patchCache[i].bbox._vMin.x;
            mPatch.m[3][1] = fHeight;
            mPatch.m[3][2] = m_patchCache[i].bbox._vMin.z;
            mPatch.m[3][3] = 1.0f;
            RedRenderer::pcRenderPrimitive(m_pHighResPrim,
                                           s_pOceanWaterShader,
                                           mPatch,
                                           RedColor::WHITE,
                                           0x200, 0, 0, 0);
        }
    }
}
