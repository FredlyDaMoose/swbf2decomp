#pragma once

#include <d3d9.h>

#include "PebbleFL/Common/PblList.h"

class PblFile;

enum QualityT
{
    QUALITY_LOW = 0,
    QUALITY_MEDIUM = 1,
    QUALITY_HIGH = 2
};

enum Quality2T
{
    QUALITY2_OFF = 0,
    QUALITY2_LOW = 1,
    QUALITY2_HIGH = 2
};

enum TextureQualityT
{
    TEXTURE_QUALITY_16BITS = 0,
    TEXTURE_QUALITY_32BITS = 1,
    TEXTURE_QUALITY_COMPRESSED = 2
};

enum GeometryModeT
{
    GEOMETRYMODE_UNCOMPRESSED = 0,
    GEOMETRYMODE_COMPRESSED = 1
};

enum PipelineT
{
    PIPELINE_FIXED = 0,
    PIPELINE_PROG1 = 1,
    PIPELINE_PROG2 = 2,
    MAX_PIPELINES = 3
};

enum TextureCapsT
{
    TEXTURE_CAPS_UNCOMPRESSED = 0,
    TEXTURE_CAPS_COMPRESSED = 1
};

enum SkinningModeT
{
    SKINNING_SOFTWARE = 0,
    SKINNING_HARDWARE = 1
};

enum ShadowVolumeModeT
{
    SHADOWVOLUME_SOFTWARE = 0,
    SHADOWVOLUME_SOFTWARE_SKINNED = 1,
    SHADOWVOLUME_HARDWARE = 2
};

enum BackBufferCopyT
{
    BACKBUFFERCOPY_CAN_STRETCH = 0,
    BACKBUFFERCOPY_CAN_COPY = 1,
    BACKBUFFERCOPY_NONE = 2
};

enum TextureNonPow2SupportT
{
    TEXTURE_NONPOW2_NEVER = 0,
    TEXTURE_NONPOW2_CONDITIONAL = 1,
    TEXTURE_NONPOW2_ALWAYS = 2
};

struct RenderOptionsT
{

    // FUNCTION: SWBF2 0x005990f0
    RenderOptionsT()
        : textureQuality(TEXTURE_QUALITY_COMPRESSED)
        , textureDetail(QUALITY_MEDIUM)
        , waterQuality(QUALITY_MEDIUM)
        , particleQuality(QUALITY_MEDIUM)
        , shadowQuality(QUALITY2_LOW)
        , motionBlur(false)
        , distortion(false)
        , mGammaBrightness(0.5f)
        , mGammaContrast(0.5f)
        , mNearSceneFadeFactor(0.5f)
        , mFarSceneRangeFactor(0.5f)
        , mEnableHDR(false)
        , defaultWidth(800)
        , defaultHeight(600)
        , masterQuality(1)
        , mGraphicsTier(1)
        , bumpMapping(true)
        , specularQuality(QUALITY2_LOW)
        , terrainQuality(QUALITY_MEDIUM)
    {
    }

    TextureQualityT textureQuality;
    QualityT textureDetail;
    QualityT waterQuality;
    QualityT particleQuality;
    Quality2T shadowQuality;
    bool motionBlur;
    bool distortion;

    unsigned char mReservedAfterDistortion[2];

    float mGammaBrightness;
    float mGammaContrast;

    float mNearSceneFadeFactor;
    float mFarSceneRangeFactor;

    bool mEnableHDR;

    unsigned char mReservedAfterHDR[3];
    int defaultWidth;
    int defaultHeight;
    int masterQuality;

    int mGraphicsTier;
    bool bumpMapping;

    unsigned char mReservedAfterBumpMapping[3];
    Quality2T specularQuality;
    QualityT terrainQuality;
};
 

struct RenderCapsT
{
    GeometryModeT geometry;
    PipelineT pipeline;
    TextureCapsT textureCaps;
    SkinningModeT skinning;
    ShadowVolumeModeT shadowVolume;
    BackBufferCopyT backBufferCopy;
    bool cubemaps;
    bool volumemaps;
    TextureNonPow2SupportT nonpow2textures;
};
 

namespace RedAutoSettings
{

extern RenderOptionsT _RenderSettings;

RenderOptionsT const& GetRenderOptions();

bool DeviceOptionsFound();
int GetEffectiveProcessorSpeed();
void GetIdealResolution(int& iWidth, int& iHeight);
int GetProcessorSpeed();
bool IsAthlonProcessor();

RenderOptionsT* GetCustomPreset(int iPreset);
void GetCustomPresetResolution(int iPreset, int& iWidth, int& iHeight);
extern RenderOptionsT sCustomRenderPresets[3];
extern bool _bAthlon;
extern int _ProcessorSpeed;
extern unsigned int _NonMaskableCount;
extern char* _NonMaskableStr[8];
extern bool optionsFounds_;

void SetMasterQuality(RenderOptionsT* pOptions, int iLevel);
void _SetGeneration(RenderOptionsT* pOptions);

void _LoadVendorDeviceOverrides();

void DoAutoDetection();

}

unsigned long GetCpuSpeed();

int axtoi(char const* pszHex, int iMaxDigits);

bool atob(char const* pszValue);
BackBufferCopyT atobbc(char const* pszValue);
GeometryModeT atogeom(char const* pszValue);
PipelineT atopipe(char const* pszValue);
Quality2T atoq2(char const* pszValue);
QualityT atoq(char const* pszValue);
SkinningModeT atoskin(char const* pszValue);
ShadowVolumeModeT atosv(char const* pszValue);
TextureCapsT atotexc(char const* pszValue);
TextureNonPow2SupportT atotexpow2(char const* pszValue);
TextureQualityT atotexq(char const* pszValue);
int fskip(PblFile& rFile, char const* pcToken);

namespace RedRenderCapOverrides
{

void _LoadVendorDeviceRenderCapOverrides();

void _ApplyVendorDeviceRenderCapOverrides(RenderCapsT& rCaps);

}

class VidMode
{
public:

    VidMode() {}

    int Width;
    int Height;
    bool bSupported;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    unsigned long MultiSampleQuality;
    int VidMemUsed;

    PblList<VidMode>::Node mNode;
};
 

class VidConfig
{
public:
    VidConfig(unsigned long ulVendorId, unsigned long ulDeviceId, unsigned long ulHint);

    static VidConfig* GetConfig(unsigned long ulVendorId, unsigned long ulDeviceId,
                                unsigned long ulHint);
    static void LoadConfigFile();
    static void SaveConfigFile();

    void AddVidMode(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                    unsigned long ulMultiSampleQuality, bool bSupported);
    bool VidModeTested(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                       unsigned long ulMultiSampleQuality);

    bool VidModeAvailable(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                          unsigned long ulMultiSampleQuality);

    bool GetMaxMultisamplingModeForScreenSize(int iWidth, int iHeight,
                                              D3DMULTISAMPLE_TYPE& rMultiSampleType,
                                              unsigned long& rMultiSampleQuality);
    bool GetMaxScreenSizeForMultisamplingMode(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                              unsigned long ulMultiSampleQuality,
                                              int& rWidth, int& rHeight);

    bool MultisamplingModeAvailable(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                    unsigned long ulMultiSampleQuality)
    {
        for (PblList<VidMode>::Node* pNode = VidModeList.GetFirst(); pNode->_pObject;
             pNode = pNode->_pNext)
        {
            VidMode* pVidMode = pNode->_pObject;
            if (pVidMode->MultiSampleType == eMultiSampleType
                && pVidMode->MultiSampleQuality == ulMultiSampleQuality && pVidMode->bSupported)
            {
                return true;
            }
        }
        return false;
    }

    unsigned long mVendorId;
    unsigned long mDeviceId;
    unsigned long mHint;
    PblList<VidMode> VidModeList;
    PblList<VidConfig>::Node mNode;

    static PblList<VidConfig> sConfigs;
};
 

namespace RedVideoConfig
{

extern IDirect3D9* m_pD3D;
extern bool m_MultiSampleDriverControlled;
extern unsigned int m_MemoryHint;

void VidMode_Init(IDirect3D9* pD3D);
void VidMode_StartupVoodoo(IDirect3D9* pD3D);

bool VidMode_IsDriverMultisampling();

bool VidMode_GetMaxMultisamplingModeForScreenSize(int iWidth, int iHeight,
                                                  D3DMULTISAMPLE_TYPE& rMultiSampleType,
                                                  unsigned long& rMultiSampleQuality);
bool VidMode_GetMaxScreenSizeForMultisamplingMode(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                                  unsigned long ulMultiSampleQuality,
                                                  int& rWidth, int& rHeight);
char const* VidMode_GetMultiSampleDescription(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                              unsigned long ulMultiSampleQuality);
int VidMode_GetNumMultiSampleQualityModes();
bool VidMode_MultisamplingModeUseable(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                      unsigned long ulMultiSampleQuality);

bool VidMode_Usable(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                    unsigned long ulMultiSampleQuality);

void VidMode_ApplyRenderCapOverrides(RenderCapsT& rCaps);

}
