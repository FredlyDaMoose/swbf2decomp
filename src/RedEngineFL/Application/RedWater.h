#pragma once

#include <stddef.h>
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblFourier.h"
#include "RedEngineFL/Graphics/RedColor.h"

class RedCamera;
class WaterRenderable;
class UnderWaterRenderable;
class pcRedPrimitive;
class pcRedShader;

struct IDirect3DQuery9;
enum ResetFlagsT;

struct WaterVertex;
struct CompressedWaterVertex;
struct OceanVertex;
struct CompressedOceanVertex;

struct WaterDistortionRingVertex;
struct CompressedWaterDistortionRingVertex;
struct WaterSplashVertex;
struct CompressedWaterSplashVertex;

class RedWater
{
public:

    enum WaterModeT
    {
        WATER_MODE_NORMAL = 0,
        WATER_MODE_OCEAN = 1,
        WATER_MODE_ICE = 2
    };
    enum VISIBILITYTEST_MODE
    {
        VISIBILITYTEST_NONE = 0,
        VISIBILITYTEST_ASSIGNID = 1,
        VISIBILITYTEST_GETIDS = 2
    };

    struct VisibilityCacheEntryT
    {
        short x;
        short z;
        unsigned short uiVisibilityID;
        IDirect3DQuery9* pVisibilityQuery;
    };

    struct WaterCustomPatch
    {
        int x;
        int z;
        void* pCustomMesh;
        PblList<WaterCustomPatch>::Node Node;
    };

    struct WaterEvent
    {
        WaterEvent();

        PblVector3 worldPos;
        PblVector3 worldVelocity;
        float expireTime;
        float startTime;
        float radius;
        unsigned char type;
        unsigned char in_range;
    };

    struct WaterPatch
    {
        PblBox bbox;
        float fDistanceSq;
        void* pCustomMesh;
        bool bClipsNearPlane;
        bool bClipped;
        unsigned short uiVisibilityID;
        unsigned short uiVisibilityFarID;
    };

    enum WATEREVENT_TYPE
    {
        WATEREVENT_RING = 0,
        WATEREVENT_WAKE = 1,
        WATEREVENT_SPLASH = 2
    };

    static void Init();

    static void __cdecl pcBeginVisibilityTest(unsigned int uiID);
    static void __cdecl pcEndVisibilityTest(unsigned int uiID);
    static unsigned int __cdecl pcGetVisibilityTestResult(unsigned int uiID);

    static void AddWaterRing(const PblVector3& vPos, float fLifetime, float fRadius);
    static void AddWaterWake(const PblVector3& vPos, const PblVector3& vVelocity, float fLifetime, float fRadius);
    static void Cleanup();

    static void Render(unsigned int uiView);
    static void RenderUnderWaterView(unsigned int uiView);

    static bool GetWaterHeight(const PblVector3& vPos, float& fHeight);

    static bool GetWaterHeightAndVelocity(const PblVector3& vPos, float& fHeight, PblVector3& vVelocity);

    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        PblVector3* pNormal);
    static void Read(PblFileChunk& chunk);
    static void ReadConfig(PblConfig& config);

    static bool Update(float fElapsed);

    static PblList<WaterCustomPatch> m_CustomPatches;

    static bool mRenderLowResFlag;

    static PblTimeBase::Stamp m_waterTimeStamp;

    static WaterEvent m_eventCache[100];

    static unsigned char* m_waterMap;
    static unsigned int m_waterMapSize;
    static unsigned int m_patchesPerCell;
    static float m_metersPerPatch;
    static float m_patchesPerMeter;
    static float m_worldCenterX;
    static float m_worldCenterZ;
    static float m_waterHeight;
    static float m_velocityU;
    static float m_velocityV;
    static float m_fWaterHeightOffset;
    static float m_minPatchHeight;
    static float m_maxPatchHeight;
    static WaterModeT m_waterMode;
    static int m_numWaterEvents;
    static WaterPatch* m_patchCache;
    static int m_patchCacheSize;
    static int m_pointsPerPatchX;
    static int m_pointsPerPatchZ;
    static int m_divisionsPerPatchX;
    static int m_divisionsPerPatchZ;
    static PblComplex* m_initialOceanPatchFreq;
    static PblComplex* m_oceanPatchFreq;
    static unsigned char* m_foamIntensity;
    static float* m_oceanDispersion;

    static float m_windDirectionX;
    static float m_windDirectionZ;
    static float m_windSpeed;
    static float m_phillipsConstant;
    static unsigned int m_uiTextureHash;
    static float m_cellsPerPatch;
    static int m_reductionFactor;
    static float m_tilesU;
    static float m_tilesV;
    static PblVector3 m_worldMin;
    static PblVector3 m_worldMax;
    static UnderWaterRenderable _UnderWaterRenderable;
    static WaterRenderable _WaterRenderable;

    static PblVector3 m_vSpecularDirection;

    static bool IsSpecularDirectionSet() { return RedWater::m_bSetSpecularDirection; }
    static void SetSpecularDirection(const PblVector3& vDirection)
    {
        if (!IsSpecularDirectionSet())
        {
            RedWater::m_bSetSpecularDirection = true;
            m_vSpecularDirection = vDirection;
        }
    }

    static float m_fresnelMin;
    static float m_fresnelMax;
    static unsigned int m_uiSpecularMaskTextures[50];
    static unsigned int m_uiNumSpecularMaskTextures;
    static float m_fSpecularMaskTexturesFrameRate;
    static unsigned int m_uiBumpMapTextures[50];
    static unsigned int m_uiNumBumpMapTextures;
    static float m_fBumpMapTexturesFrameRate;
    static unsigned int m_uiNormalMapTextures[50];
    static unsigned int m_uiNumNormalMapTextures;
    static float m_fNormalMapTexturesFrameRate;
    static float m_fSpecularMaskTileU;
    static float m_fSpecularMaskTileV;
    static float m_fSpecularMaskScrollU;
    static float m_fSpecularMaskScrollV;
    static float m_farSceneRange;
    static bool m_bEnableVisibilityTest;
    static pcRedPrimitive* m_pLowResPrim;
    static pcRedPrimitive* m_pHighResPrim;
    static VisibilityCacheEntryT* m_aVisibilityCache;
    static unsigned int m_uiVisibilityCacheSize;
    static unsigned int m_uiVisibilityCacheSize_FarScene;
    static RedColor m_refractionColor;
    static RedColor m_reflectionColor;
    static RedColor m_underwaterColor;

    static char m_reflectionID;

    static void pcReset(ResetFlagsT eFlags);
    static void pcSetupShaders();

    static void PlatformReadConfigData(PblConfig::Data& data);

protected:

    static void PlatformPreReadInit();

    static void PlatformCleanup();
    static void PlatformPostReadInit();
    static void* PlatformReadCustomPatch(PblFileChunk* pChunk);
    static void PlatformRender(unsigned int uiFlags);
    static void PlatformRenderUnderWaterView(PblVector3* pVerts, int iNumVerts);
    static void PlatformUpdatePatchGeometry(PblVector3* pMin, PblVector3* pMax);

    static void BuildPatchCache(const RedCamera* pCamera, bool bFarScene,
                                VISIBILITYTEST_MODE mode);
    static void UpdateEventCache();

    static bool WaterExists(int x, int z);
    static void GetVisibleExtents(const RedCamera* pCamera, int& iMinX, int& iMinZ, int& iMaxX, int& iMaxZ);
    static void OceanCleanup();
    static void OceanCreatePatch(PblComplex* pFreq);
    static float OceanDispersion(float kx, float kz);

    static void OceanInit();
    static float OceanPhillipsSpectrum(float kx, float kz);
    static void OceanUpdate(float fTime);

private:

    static bool pcDeviceSupportsVisibilityTesting();
    static void pcAccumulateNormalMap();
    static void pcRegisterReflection();
    static void pcRenderFarOceanWaterPatches();
    static void pcRenderFarWaterPatches();
    static void pcRenderOceanWaterPatches();
    static void pcRenderWaterPatches();
    static void pcRenderWaterRings();
    static void pcRenderWaterSplashes();

    template <class T, unsigned int FORMAT>
    static void pcRenderDistortionRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                                       const PblVector3& vWorldMax, const PblVector3& vPos,
                                       float fRadius, float fLifetime, float fAge);

    template <class T, unsigned int FORMAT>
    static void pcRenderRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                             const PblVector3& vWorldMax, const PblVector3& vPos,
                             float fRadius, float fLifetime, float fAge);
    template <class T, unsigned int FORMAT>
    static void pcRenderWakeRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                                 const PblVector3& vWorldMax, const PblVector3& vPos,
                                 float fRadius, const PblVector3& vVelocity,
                                 float fLifetime, float fAge);
    template <class T, unsigned int FORMAT>
    static void pcRenderDistortionWakeRing(pcRedShader* pShader, const PblVector3& vWorldMin,
                                           const PblVector3& vWorldMax, const PblVector3& vPos,
                                           float fRadius, const PblVector3& vVelocity,
                                           float fLifetime, float fAge);
    template <class T, unsigned int FORMAT>
    static void pcRenderSplash(pcRedShader* pShader, const PblVector3& vWorldMin,
                               const PblVector3& vWorldMax, const PblVector3& vPos,
                               float fRadius, float fAngle, float fHeight,
                               const PblVector3& vVelocity, unsigned int uiSeed, float fAge);

    template <class T, unsigned int FORMAT>
    static pcRedPrimitive* pcCreateStaticPatchPrimitive(unsigned int uiPointsX,
                                                        unsigned int uiPointsZ);
    template <class T, unsigned int FORMAT>
    static pcRedPrimitive* pcCreateDynamicPatchPrimitive(PblVector3* pMin, PblVector3* pMax,
                                                         unsigned int uiPointsX,
                                                         unsigned int uiPointsZ,
                                                         unsigned int uiStep);
public:
    static int m_highResLowResRatio;
    static unsigned int m_uiFoamTextureHash;
    static float m_foamTilesU;
    static float m_foamTilesV;
    static bool m_oscillationEnable;
    static bool m_specularEnable;
    static bool m_bSetSpecularDirection;
};
 
 
 
 
 
 
 
 
