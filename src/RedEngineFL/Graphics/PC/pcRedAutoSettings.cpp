#include <windows.h>
#include <mmsystem.h>

#include <ctype.h>
#include <intrin.h>
#include <stdio.h>
#include <string.h>

#include "pcRedAutoSettings.h"
#include "pcRedDeviceEnumerator.h"
#include "pcRedRenderer.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblMath.h"

#include "RedEngineFL/Memory/engineAlloc.h"
#include <stdlib.h>
#include "PebbleFL/Common/PblHashTable.h"

extern HWND hGlobalWnd;

namespace RedAutoSettings
{

RenderOptionsT _RenderSettings;

// GLOBAL: SWBF2 0x007e1098

RenderOptionsT sCustomRenderPresets[3];

// GLOBAL: SWBF2 0x009aba49
bool _bAthlon;
// GLOBAL: SWBF2 0x009aba50
int _ProcessorSpeed;
// GLOBAL: SWBF2 0x009aba54
unsigned int _NonMaskableCount;
// GLOBAL: SWBF2 0x009aba58
char* _NonMaskableStr[8];
// GLOBAL: SWBF2 0x009aba78

int sHardwareGeneration;
// GLOBAL: SWBF2 0x009abaa3
bool optionsFounds_;
// GLOBAL: SWBF2 0x009abaac

int sLastTextureFourCC;

// FUNCTION: SWBF2 0x006ea1b0
RenderOptionsT const& GetRenderOptions()
{
    return _RenderSettings;
}

// FUNCTION: SWBF2 0x006e9bd0
bool DeviceOptionsFound()
{
    return optionsFounds_;
}

// FUNCTION: SWBF2 0x006ea290
int GetEffectiveProcessorSpeed()
{
    if (_bAthlon)
    {
        return (int)(_ProcessorSpeed * 1.3f);
    }
    return _ProcessorSpeed;
}

// FUNCTION: SWBF2 0x006ea2c0
void GetIdealResolution(int& iWidth, int& iHeight)
{
    iWidth = _RenderSettings.defaultWidth;
    iHeight = _RenderSettings.defaultHeight;
}

// FUNCTION: SWBF2 0x006ea2e0
RenderOptionsT* GetCustomPreset(int iPreset)
{
    return &sCustomRenderPresets[iPreset];
}

// FUNCTION: SWBF2 0x006ea300
void GetCustomPresetResolution(int iPreset, int& iWidth, int& iHeight)
{
    iWidth = sCustomRenderPresets[iPreset].defaultWidth;
    iHeight = sCustomRenderPresets[iPreset].defaultHeight;
}

// FUNCTION: SWBF2 0x006ea420
int GetProcessorSpeed()
{
    return _ProcessorSpeed;
}

// FUNCTION: SWBF2 0x006ea430
bool IsAthlonProcessor()
{
    return _bAthlon;
}

}

// FUNCTION: SWBF2 0x006ec700
int axtoi(char const* pszHex, int iMaxDigits)
{
    int digit[8];
    int n;

    for (n = 0; n < iMaxDigits; ++n)
    {
        if (pszHex[n] == '\0')
        {
            break;
        }

        if (pszHex[n] > 0x29 && pszHex[n] < 0x40)
        {
            digit[n] = pszHex[n] & 0x0f;
        }
        else if (pszHex[n] >= 'a' && pszHex[n] <= 'f')
        {
            digit[n] = (pszHex[n] & 0x0f) + 9;
        }
        else if (pszHex[n] >= 'A' && pszHex[n] <= 'F')
        {
            digit[n] = (pszHex[n] & 0x0f) + 9;
        }
        else if (pszHex[n] == 'x' && pszHex[n - 1] == '0')
        {
            n = -1;
        }
        else
        {
            break;
        }
    }

    int iCount = n;
    int m = n - 1;
    int iValue = 0;

    for (n = 0; n < iCount; ++n)
    {
        iValue |= digit[n] << (m << 2);
        --m;
    }

    return iValue;
}

PblList<VidConfig> VidConfig::sConfigs;

// FUNCTION: SWBF2 0x006e9a80
VidConfig::VidConfig(unsigned long ulVendorId, unsigned long ulDeviceId, unsigned long ulHint)
{
    mVendorId = ulVendorId;
    mDeviceId = ulDeviceId;
    mHint = ulHint;

    sConfigs.AddLast(&mNode, this);
}

// FUNCTION: SWBF2 0x006e9b00
void VidConfig::AddVidMode(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                           unsigned long ulMultiSampleQuality, bool bSupported)
{
    VidMode* pVidMode = new VidMode;

    pVidMode->Width = iWidth;
    pVidMode->Height = iHeight;
    pVidMode->MultiSampleType = eMultiSampleType;
    pVidMode->MultiSampleQuality = ulMultiSampleQuality;
    pVidMode->bSupported = bSupported;

    VidModeList.AddLast(&pVidMode->mNode, pVidMode);
}

namespace RedAutoSettings
{

// FUNCTION: SWBF2 0x006e9b80
void _SetGeneration(RenderOptionsT* pOptions)
{
    switch (pOptions->mGraphicsTier)
    {
    case 0:
        pOptions->specularQuality = QUALITY2_OFF;
        pOptions->terrainQuality = QUALITY_LOW;
        pOptions->bumpMapping = false;
        break;

    case 1:
        pOptions->specularQuality = QUALITY2_HIGH;
        pOptions->terrainQuality = QUALITY_MEDIUM;
        pOptions->bumpMapping = false;
        break;

    case 2:
        pOptions->bumpMapping = true;
        pOptions->specularQuality = QUALITY2_HIGH;
        pOptions->terrainQuality = QUALITY_HIGH;
        break;
    }
}

// FUNCTION: SWBF2 0x006e9be0
void DoAutoDetection()
{
    D3DCAPS9 caps = RedRenderer::pcGetDeviceCaps();

    if (caps.VertexShaderVersion >= D3DVS_VERSION(3, 0)
        && caps.PixelShaderVersion >= D3DPS_VERSION(3, 0)
        && caps.MaxVertexShaderConst >= 0x60)
    {
        sHardwareGeneration = 4;
    }
    else if (caps.VertexShaderVersion >= D3DVS_VERSION(2, 0)
             && caps.PixelShaderVersion >= D3DPS_VERSION(2, 0)
             && caps.MaxVertexShaderConst >= 0x60)
    {
        sHardwareGeneration = 3;
    }
    else if (caps.VertexShaderVersion >= D3DVS_VERSION(1, 1)
             && caps.PixelShaderVersion >= D3DPS_VERSION(1, 1)
             && caps.MaxVertexShaderConst >= 0x60)
    {
        sHardwareGeneration = 2;
    }
    else
    {
        sHardwareGeneration = 1;
    }

    SetMasterQuality(&_RenderSettings, sHardwareGeneration - 2);

    _NonMaskableCount = 0;

    if ((caps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP) && sHardwareGeneration > 1)
    {
        _RenderSettings.bumpMapping = true;
    }
    else
    {
        _RenderSettings.bumpMapping = false;
    }

    bool bAllCombosHaveDXT = true;
    D3DAdapterInfo* pAdapter = 0;

    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    for (PblList<D3DAdapterInfo>::Node* pNode
             = pcRedDeviceEnumerator::GetDeviceList().GetFirst();
         pNode->_pObject; pNode = pNode->_pNext)
    {
        D3DAdapterInfo* pInfo = pNode->_pObject;

        if (pInfo->AdapterIdentifier.VendorId == identifier.VendorId
            && pInfo->AdapterIdentifier.DeviceId == identifier.DeviceId)
        {
            pAdapter = pInfo;
        }
    }

    for (PblList<D3DDeviceInfo>::Node* pDevNode = pAdapter->DeviceInfoList.GetFirst();
         pDevNode->_pObject; pDevNode = pDevNode->_pNext)
    {
        D3DDeviceInfo* pDevice = pDevNode->_pObject;

        if (pDevice->AdapterOrdinal != caps.AdapterOrdinal)
        {
            continue;
        }

        for (PblList<D3DDeviceCombo>::Node* pComboNode
                 = pDevice->DeviceComboList.GetFirst();
             pComboNode->_pObject; pComboNode = pComboNode->_pNext)
        {
            D3DDeviceCombo* pCombo = pComboNode->_pObject;
            bool bFound = false;

            for (PblList<D3DFormat>::Node* pFormatNode
                     = pCombo->TextureFormatList.GetFirst();
                 pFormatNode->_pObject; pFormatNode = pFormatNode->_pNext)
            {
                sLastTextureFourCC = (int)pFormatNode->_pObject->format;

                if (sLastTextureFourCC == (int)MAKEFOURCC('D', 'X', 'T', '1')
                    || sLastTextureFourCC == (int)MAKEFOURCC('D', 'X', 'T', '2')
                    || sLastTextureFourCC == (int)MAKEFOURCC('D', 'X', 'T', '3')
                    || sLastTextureFourCC == (int)MAKEFOURCC('D', 'X', 'T', '4')
                    || sLastTextureFourCC == (int)MAKEFOURCC('D', 'X', 'T', '5'))
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                bAllCombosHaveDXT = false;
            }
        }
    }

    if (bAllCombosHaveDXT)
    {
        _RenderSettings.textureQuality = TEXTURE_QUALITY_COMPRESSED;
    }

    HKEY hKey;

    _ProcessorSpeed = 800;
    _bAthlon = false;

    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        "Hardware\\Description\\System\\CentralProcessor\\0",
                        0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0)
        == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSize = 4;

        if (RegQueryValueExA(hKey, "~MHz", 0, &dwType, (BYTE*)&_ProcessorSpeed, &dwSize)
            != ERROR_SUCCESS)
        {
            _ProcessorSpeed = (int)GetCpuSpeed();
        }

        RegCloseKey(hKey);
    }

    float fSpeed = (float)_ProcessorSpeed;

    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        "Hardware\\Description\\System\\CentralProcessor\\0",
                        0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0)
        == ERROR_SUCCESS)
    {
        char acProcessorName[256];
        DWORD dwType;
        DWORD dwSize = 256;

        if (RegQueryValueExA(hKey, "ProcessorNameString", 0, &dwType,
                             (BYTE*)acProcessorName, &dwSize) == ERROR_SUCCESS)
        {

            if (strstr(acProcessorName, "Athlon"))
            {
                fSpeed *= 1.3f;
                _bAthlon = true;
            }
        }

        RegCloseKey(hKey);
    }

    int iTier;
    bool bTail;

    if (1500.0f > fSpeed)
    {
        float t = (fSpeed - 800.0f) / 700.0f;
        float u = t * (0.25f - 0.0f);
        float fClamped = (0.0f > u) ? 0.0f : u;

        if (_RenderSettings.mFarSceneRangeFactor > fClamped)
        {
            _RenderSettings.mFarSceneRangeFactor = fClamped;
        }

        if (_RenderSettings.shadowQuality == QUALITY2_HIGH)
        {
            _RenderSettings.shadowQuality = QUALITY2_LOW;
        }
        _RenderSettings.bumpMapping = false;
        if (_RenderSettings.waterQuality == QUALITY_HIGH)
        {
            _RenderSettings.waterQuality = QUALITY_MEDIUM;
        }
        if (_RenderSettings.particleQuality == QUALITY_HIGH)
        {
            _RenderSettings.particleQuality = QUALITY_MEDIUM;
        }

        if (sHardwareGeneration == 1)
        {
            float v = 0.0f + u;
            if (_RenderSettings.mNearSceneFadeFactor > v)
            {
                _RenderSettings.mNearSceneFadeFactor = v;
            }
            iTier = sHardwareGeneration;
            bTail = false;
        }
        else if (sHardwareGeneration == 2)
        {
            float v = 0.2f + t * (0.4f - 0.2f);
            if (_RenderSettings.mNearSceneFadeFactor > v)
            {
                _RenderSettings.mNearSceneFadeFactor = v;
            }
            iTier = sHardwareGeneration;
            bTail = true;
        }
        else
        {
            float v = 0.25f + u;
            if (_RenderSettings.mNearSceneFadeFactor > v)
            {
                _RenderSettings.mNearSceneFadeFactor = v;
            }
            iTier = (sHardwareGeneration > 2) ? 2 : sHardwareGeneration;
            bTail = sHardwareGeneration > 1;
        }
    }
    else if (2500.0f > fSpeed)
    {
        float t = (fSpeed - 1500.0f) / 1000.0f;
        float v = 0.25f + t * (0.7f - 0.25f);

        if (_RenderSettings.mFarSceneRangeFactor > v)
        {
            _RenderSettings.mFarSceneRangeFactor = v;
        }

        if (sHardwareGeneration == 1)
        {
            float w = 0.25f + t * (0.5f - 0.25f);
            if (_RenderSettings.mNearSceneFadeFactor > w)
            {
                _RenderSettings.mNearSceneFadeFactor = w;
            }

            if (_RenderSettings.waterQuality == QUALITY_HIGH)
            {
                _RenderSettings.waterQuality = QUALITY_MEDIUM;
            }
            _RenderSettings.bumpMapping = false;
            if (_RenderSettings.shadowQuality == QUALITY2_HIGH)
            {
                _RenderSettings.shadowQuality = QUALITY2_LOW;
            }

            iTier = sHardwareGeneration;
            bTail = false;
        }
        else if (sHardwareGeneration == 2)
        {
            float w = 0.4f + t * (0.6f - 0.4f);
            if (_RenderSettings.mNearSceneFadeFactor > w)
            {
                _RenderSettings.mNearSceneFadeFactor = w;
            }

            if (_RenderSettings.shadowQuality == QUALITY2_HIGH)
            {
                _RenderSettings.shadowQuality = QUALITY2_LOW;
            }
            _RenderSettings.bumpMapping = false;
            if (_RenderSettings.waterQuality == QUALITY_HIGH)
            {
                _RenderSettings.waterQuality = QUALITY_MEDIUM;
            }

            iTier = sHardwareGeneration;
            bTail = true;
        }
        else
        {
            float w = 0.5f + t * (0.7f - 0.5f);
            if (_RenderSettings.mNearSceneFadeFactor > w)
            {
                _RenderSettings.mNearSceneFadeFactor = w;
            }

            iTier = (sHardwareGeneration > 2) ? 2 : sHardwareGeneration;
            bTail = sHardwareGeneration > 1;
        }
    }
    else
    {

        iTier = (sHardwareGeneration > 2) ? 2 : sHardwareGeneration;
        bTail = sHardwareGeneration > 1;
    }

    _RenderSettings.masterQuality = 3;
    _RenderSettings.mGraphicsTier = iTier;
    _RenderSettings.mEnableHDR = bTail;

    _SetGeneration(&_RenderSettings);

    for (int i = 0; i < 3; ++i)
    {
        SetMasterQuality(&sCustomRenderPresets[i], i);
    }

    _LoadVendorDeviceOverrides();
}

}

// FUNCTION: SWBF2 0x006ea1c0
VidConfig* VidConfig::GetConfig(unsigned long ulVendorId, unsigned long ulDeviceId,
                                unsigned long ulHint)
{
    for (PblList<VidConfig>::Node* pNode = sConfigs.GetFirst(); pNode->_pObject;
         pNode = pNode->_pNext)
    {
        VidConfig* pConfig = pNode->_pObject;

        if (pConfig->mVendorId == ulVendorId && pConfig->mDeviceId == ulDeviceId
            && pConfig->mHint == ulHint)
        {
            return pConfig;
        }
    }

    return 0;
}

// FUNCTION: SWBF2 0x006ea210
unsigned long GetCpuSpeed()
{
    unsigned int uiStart = 0;
    unsigned int uiEnd = 0;

    int aCpuInfo[4];

    DWORD dwT0 = timeGetTime();
    DWORD dwT1;

    do
    {
        dwT1 = timeGetTime();
    }
    while ((int)(dwT1 - dwT0) <= 1);

    __cpuid(aCpuInfo, 0);
    uiStart = (unsigned int)__rdtsc();

    while ((int)(timeGetTime() - dwT1) <= 1000)
        ;

    __cpuid(aCpuInfo, 0);
    uiEnd = (unsigned int)__rdtsc();

    return (uiEnd - uiStart) / 1000000;
}

bool VidConfig::VidModeTested(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                              unsigned long ulMultiSampleQuality)
{
    for (PblList<VidMode>::Node* pNode = VidModeList.GetFirst(); pNode->_pObject;
         pNode = pNode->_pNext)
    {
        VidMode* pVidMode = pNode->_pObject;

        if (pVidMode->Width == iWidth && pVidMode->Height == iHeight
            && pVidMode->MultiSampleType == eMultiSampleType
            && pVidMode->MultiSampleQuality == ulMultiSampleQuality)
        {
            return true;
        }
    }

    return false;
}

// FUNCTION: SWBF2 0x006ea790
bool VidConfig::VidModeAvailable(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                                 unsigned long ulMultiSampleQuality)
{
    for (PblList<VidMode>::Node* pNode = VidModeList.GetFirst(); pNode->_pObject;
         pNode = pNode->_pNext)
    {
        VidMode* pVidMode = pNode->_pObject;

        if (pVidMode->Width == iWidth && pVidMode->Height == iHeight
            && pVidMode->MultiSampleType == eMultiSampleType
            && pVidMode->MultiSampleQuality == ulMultiSampleQuality)
        {
            return pVidMode->bSupported;
        }
    }

    return false;
}

// FUNCTION: SWBF2 0x006ea330
bool VidConfig::GetMaxMultisamplingModeForScreenSize(int iWidth, int iHeight,
                                                     D3DMULTISAMPLE_TYPE& rMultiSampleType,
                                                     unsigned long& rMultiSampleQuality)
{
    bool bFound = false;
    rMultiSampleType = D3DMULTISAMPLE_NONE;
    rMultiSampleQuality = 0;

    for (PblList<VidMode>::Node* pNode = VidModeList.GetFirst(); pNode->_pObject;
         pNode = pNode->_pNext)
    {
        VidMode* pVidMode = pNode->_pObject;

        if (pVidMode->Width == iWidth && pVidMode->Height == iHeight && pVidMode->bSupported
            && ((unsigned long)rMultiSampleType < (unsigned long)pVidMode->MultiSampleType
                || (rMultiSampleType == pVidMode->MultiSampleType
                    && rMultiSampleQuality < pVidMode->MultiSampleQuality)))
        {
            rMultiSampleType = pVidMode->MultiSampleType;
            rMultiSampleQuality = pVidMode->MultiSampleQuality;
            bFound = true;
        }
    }

    return bFound;
}

// FUNCTION: SWBF2 0x006ea3b0
bool VidConfig::GetMaxScreenSizeForMultisamplingMode(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                                     unsigned long ulMultiSampleQuality,
                                                     int& rWidth, int& rHeight)
{
    rHeight = 0;
    rWidth = 0;

    for (PblList<VidMode>::Node* pNode = VidModeList.GetFirst(); pNode->_pObject;
         pNode = pNode->_pNext)
    {
        VidMode* pVidMode = pNode->_pObject;

        if (pVidMode->MultiSampleType == eMultiSampleType
            && pVidMode->MultiSampleQuality == ulMultiSampleQuality && pVidMode->bSupported
            && (rWidth < pVidMode->Width
                || (rWidth == pVidMode->Width && rHeight < pVidMode->Height)))
        {
            rWidth = pVidMode->Width;
            rHeight = pVidMode->Height;
        }
    }

    return rWidth > 0;
}

// FUNCTION: SWBF2 0x006ea440
void VidConfig::LoadConfigFile()
{
    PblFile file;
    int iLen = 0;

    if (file.Open("Data\\_LVL_PC\\vidmode.ini", true, false, true)
        || file.Open("vidmode.ini", true, false, true))
    {
        VidConfig* pConfig = 0;
        int iSize = file.GetSize();

        if (iSize > 0)
        {
            char* pcLine = new char[iSize + 1];

            while (file.Tell() != iSize)
            {
                char cByte = (char)tolower(file.Read8());

                if (cByte == '\n')
                {
                    if (iLen)
                    {
                        if (pcLine[0] == '[' && pcLine[iLen - 1] == ']')
                        {
                            char const* pcVendor = strstr(pcLine, "0x") + 2;
                            char const* pcDevice = strstr(pcVendor, "0x") + 2;
                            char const* pcHint = strstr(pcDevice, "0x");

                            pConfig = new VidConfig(axtoi(pcVendor, 4), axtoi(pcDevice, 4),
                                                    axtoi(pcHint + 2, 8));
                        }
                        else
                        {
                            int iWidth;
                            int iHeight;
                            int iMultiSampleType;
                            int iMultiSampleQuality;
                            int iTested;

                            sscanf(pcLine, "%d %d %d %d %d", &iWidth, &iHeight,
                                   &iMultiSampleType, &iMultiSampleQuality, &iTested);

                            pConfig->AddVidMode(iWidth, iHeight,
                                                (D3DMULTISAMPLE_TYPE)iMultiSampleType,
                                                iMultiSampleQuality, iTested == 1);
                        }

                        iLen = 0;
                    }
                }
                else if (cByte != '\r')
                {
                    pcLine[iLen++] = cByte;
                    pcLine[iLen] = '\0';
                }
            }

            file.Close();
            delete[] pcLine;
        }
    }
}

// FUNCTION: SWBF2 0x006ea620
void VidConfig::SaveConfigFile()
{
    PblFile file;
    char acBuffer[256];

    if (file.OpenCreate("Data\\_LVL_PC\\vidmode.ini", false, true, false)
        || file.OpenCreate("vidmode.ini", false, true, false))
    {
        for (PblList<VidConfig>::Node* pNode = sConfigs.GetFirst(); pNode->_pObject;
             pNode = pNode->_pNext)
        {
            VidConfig* pConfig = pNode->_pObject;

            sprintf(acBuffer, "[Vendor 0x%04X Device 0x%04X Hint 0x%08X]\r\n",
                    pConfig->mVendorId, pConfig->mDeviceId, pConfig->mHint);
            file.WriteString(acBuffer);

            for (PblList<VidMode>::Node* pModeNode = pConfig->VidModeList.GetFirst();
                 pModeNode->_pObject; pModeNode = pModeNode->_pNext)
            {
                VidMode* pVidMode = pModeNode->_pObject;

                sprintf(acBuffer, "%d %d %d %d %d\r\n", pVidMode->Width, pVidMode->Height,
                        pVidMode->MultiSampleType, pVidMode->MultiSampleQuality,
                        pVidMode->bSupported);
                file.WriteString(acBuffer);
            }

            file.WriteString("\r\n");
        }

        file.Close();
    }
}

namespace RedVideoConfig
{

IDirect3D9* m_pD3D;
bool m_MultiSampleDriverControlled;
unsigned int m_MemoryHint;

// FUNCTION: SWBF2 0x006ea7f0
void VidMode_ApplyRenderCapOverrides(RenderCapsT& rCaps)
{
    RedRenderCapOverrides::_LoadVendorDeviceRenderCapOverrides();
    RedRenderCapOverrides::_ApplyVendorDeviceRenderCapOverrides(rCaps);
}

// FUNCTION: SWBF2 0x006ea800
bool VidMode_GetMaxMultisamplingModeForScreenSize(int iWidth, int iHeight,
                                                  D3DMULTISAMPLE_TYPE& rMultiSampleType,
                                                  unsigned long& rMultiSampleQuality)
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    VidConfig* pConfig = VidConfig::GetConfig(identifier.VendorId, identifier.DeviceId,
                                              m_MemoryHint);
    if (!pConfig)
    {
        return true;
    }

    return pConfig->GetMaxMultisamplingModeForScreenSize(iWidth, iHeight, rMultiSampleType,
                                                         rMultiSampleQuality);
}

// FUNCTION: SWBF2 0x006ea860
bool VidMode_GetMaxScreenSizeForMultisamplingMode(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                                  unsigned long ulMultiSampleQuality,
                                                  int& rWidth, int& rHeight)
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    VidConfig* pConfig = VidConfig::GetConfig(identifier.VendorId, identifier.DeviceId,
                                              m_MemoryHint);
    if (!pConfig)
    {
        return true;
    }

    return pConfig->GetMaxScreenSizeForMultisamplingMode(eMultiSampleType, ulMultiSampleQuality,
                                                         rWidth, rHeight);
}

// FUNCTION: SWBF2 0x006ea8c0
char const* VidMode_GetMultiSampleDescription(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                              unsigned long ulMultiSampleQuality)
{
    switch (eMultiSampleType)
    {
    case D3DMULTISAMPLE_NONE:
        return "None";
    case D3DMULTISAMPLE_NONMASKABLE:
        if (ulMultiSampleQuality < RedAutoSettings::_NonMaskableCount)
        {
            return RedAutoSettings::_NonMaskableStr[ulMultiSampleQuality];
        }
        break;
    case D3DMULTISAMPLE_2_SAMPLES:  return "2x";
    case D3DMULTISAMPLE_3_SAMPLES:  return "3x";
    case D3DMULTISAMPLE_4_SAMPLES:  return "4x";
    case D3DMULTISAMPLE_5_SAMPLES:  return "5x";
    case D3DMULTISAMPLE_6_SAMPLES:  return "6x";
    case D3DMULTISAMPLE_7_SAMPLES:  return "7x";
    case D3DMULTISAMPLE_8_SAMPLES:  return "8x";
    case D3DMULTISAMPLE_9_SAMPLES:  return "9x";
    case D3DMULTISAMPLE_10_SAMPLES: return "10x";
    case D3DMULTISAMPLE_11_SAMPLES: return "11x";
    case D3DMULTISAMPLE_12_SAMPLES: return "12x";
    case D3DMULTISAMPLE_13_SAMPLES: return "13x";
    case D3DMULTISAMPLE_14_SAMPLES: return "14x";
    case D3DMULTISAMPLE_15_SAMPLES: return "15x";
    case D3DMULTISAMPLE_16_SAMPLES: return "16x";
    }

    return "Unsupported";
}

// FUNCTION: SWBF2 0x006ea9b0
int VidMode_GetNumMultiSampleQualityModes()
{
    return RedAutoSettings::_NonMaskableCount;
}

// FUNCTION: SWBF2 0x006ea9c0
void VidMode_Init(IDirect3D9* pD3D)
{

    D3DCAPS9 caps;
    pD3D->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);

    m_pD3D = pD3D;

    DWORD dwBehaviorFlags = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
                                ? D3DCREATE_HARDWARE_VERTEXPROCESSING
                                : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    VidMode_StartupVoodoo(pD3D);
    VidConfig::LoadConfigFile();

    D3DADAPTER_IDENTIFIER9 identifier;
    m_pD3D->GetAdapterIdentifier(0, 0, &identifier);

    if (VidConfig::GetConfig(identifier.VendorId, identifier.DeviceId, m_MemoryHint))
    {
        return;
    }

    VidConfig* pConfig = new VidConfig(identifier.VendorId, identifier.DeviceId, m_MemoryHint);

    for (int iMultiSampleType = 0; iMultiSampleType <= 16; ++iMultiSampleType)
    {
        DWORD dwQualityLevels = 0;
        if (m_pD3D->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL,
                                               D3DFMT_A8R8G8B8, TRUE,
                                               (D3DMULTISAMPLE_TYPE)iMultiSampleType,
                                               &dwQualityLevels) != D3D_OK)
        {
            continue;
        }

        for (unsigned int uiQuality = 0; uiQuality < dwQualityLevels; ++uiQuality)
        {
            UINT uiModeCount = m_pD3D->GetAdapterModeCount(0, D3DFMT_X8R8G8B8);

            unsigned int uiBestArea = 0;

            for (int iMode = (int)uiModeCount - 1; iMode >= 0; --iMode)
            {
                D3DDISPLAYMODE mode;
                m_pD3D->EnumAdapterModes(0, D3DFMT_X8R8G8B8, iMode, &mode);

                if (pConfig->VidModeTested((int)mode.Width, (int)mode.Height,
                                           (D3DMULTISAMPLE_TYPE)iMultiSampleType, uiQuality))
                {
                    continue;
                }

                if (mode.Width * mode.Height < uiBestArea)
                {

                    pConfig->AddVidMode((int)mode.Width, (int)mode.Height,
                                        (D3DMULTISAMPLE_TYPE)iMultiSampleType, uiQuality, true);
                    continue;
                }

                D3DPRESENT_PARAMETERS pp;
                memset(&pp, 0, sizeof(pp));
                pp.BackBufferWidth = mode.Width;
                pp.BackBufferHeight = mode.Height;
                pp.BackBufferFormat = D3DFMT_A8R8G8B8;
                pp.BackBufferCount = 1;
                pp.MultiSampleType = (D3DMULTISAMPLE_TYPE)iMultiSampleType;
                pp.MultiSampleQuality = uiQuality;
                pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                pp.Windowed = TRUE;
                pp.EnableAutoDepthStencil = TRUE;
                pp.AutoDepthStencilFormat = D3DFMT_D24S8;
                pp.FullScreen_RefreshRateInHz = 0;
                pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

                IDirect3DDevice9* pDevice = 0;
                if (m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hGlobalWnd, dwBehaviorFlags,
                                         &pp, &pDevice) != D3D_OK)
                {
                    pConfig->AddVidMode((int)mode.Width, (int)mode.Height,
                                        (D3DMULTISAMPLE_TYPE)iMultiSampleType, uiQuality, false);
                    continue;
                }

                D3DCAPS9 deviceCaps;
                pDevice->GetDeviceCaps(&deviceCaps);

                unsigned int uiTexWidth;
                unsigned int uiTexHeight;
                bool bNonPow2 = true;

                if ((deviceCaps.TextureCaps & D3DPTEXTURECAPS_POW2)
                    && !(deviceCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
                {

                    bNonPow2 = false;

                    unsigned int uiShift = 31;
                    for (;;)
                    {
                        uiTexWidth = 1u << uiShift;
                        if (uiTexWidth <= mode.Width && uiTexWidth <= deviceCaps.MaxTextureWidth)
                        {
                            break;
                        }
                        --uiShift;
                    }

                    uiShift = 31;
                    for (;;)
                    {
                        uiTexHeight = 1u << uiShift;
                        if (uiTexHeight <= mode.Height && uiTexHeight <= deviceCaps.MaxTextureHeight)
                        {
                            break;
                        }
                        --uiShift;
                    }
                }
                else
                {
                    uiTexWidth = mode.Width;
                    uiTexHeight = mode.Height;
                }

                unsigned int auiTexWidth[12] = {
                    uiTexWidth, uiTexWidth, uiTexWidth, uiTexWidth,
                    512, 256, 16, 2,
                    256, 64, 4, uiTexWidth
                };
                unsigned int auiTexHeight[12] = {
                    uiTexHeight, uiTexHeight, uiTexHeight, uiTexHeight,
                    256, 256, 16, 2,
                    256, 64, 4, uiTexHeight
                };
                IDirect3DTexture9* apTextures[12] = { 0 };

                bool bAllSurfaces = true;

                for (int i = 0; i < 12; ++i)
                {
                    if (pDevice->CreateTexture(auiTexWidth[i], auiTexHeight[i], 1,
                                               D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                                               D3DPOOL_DEFAULT, &apTextures[i], 0) != D3D_OK)
                    {
                        apTextures[i] = 0;
                        bAllSurfaces = false;
                        break;
                    }
                }

                unsigned int auiDepthWidth[2] = { uiTexWidth, 512 };
                unsigned int auiDepthHeight[2] = { uiTexHeight, 256 };
                IDirect3DSurface9* apDepthStencil[2] = { 0, 0 };

                for (int i = 0; i < 2; ++i)
                {
                    if (i == 0)
                    {

                        D3DCAPS9 renderCaps = RedRenderer::pcGetDeviceCaps();
                        if (renderCaps.VertexShaderVersion < D3DVS_VERSION(1, 1))
                        {
                            continue;
                        }
                        if (renderCaps.PixelShaderVersion < D3DPS_VERSION(1, 1))
                        {
                            continue;
                        }
                        if (renderCaps.MaxVertexShaderConst < 0x60)
                        {
                            continue;
                        }
                    }
                    else if (i == 1)
                    {

                        if (iMultiSampleType == 0)
                        {
                            continue;
                        }
                        if (!bNonPow2)
                        {
                            continue;
                        }
                    }

                    if (pDevice->CreateDepthStencilSurface(
                            auiDepthWidth[i], auiDepthHeight[i], D3DFMT_D24S8,
                            D3DMULTISAMPLE_NONE, 0, TRUE, &apDepthStencil[i], 0) != D3D_OK)
                    {
                        apDepthStencil[i] = 0;
                        bAllSurfaces = false;
                        break;
                    }
                }

                IDirect3DTexture9* pMultiSampleTexture = 0;
                bool bSupported = bAllSurfaces;

                if (iMultiSampleType != 0)
                {
                    if (pDevice->CreateTexture(uiTexWidth, uiTexHeight, 1,
                                               D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                                               D3DPOOL_DEFAULT, &pMultiSampleTexture, 0) != D3D_OK)
                    {

                        pMultiSampleTexture = 0;
                        bSupported = false;
                    }
                }

                pConfig->AddVidMode((int)mode.Width, (int)mode.Height,
                                    (D3DMULTISAMPLE_TYPE)iMultiSampleType, uiQuality, bSupported);
                if (bSupported)
                {
                    uiBestArea = mode.Width * mode.Height;
                }

                for (int j = 0; j < 12; ++j)
                {
                    if (apTextures[j])
                    {
                        apTextures[j]->Release();
                    }
                }
                for (int j = 0; j < 2; ++j)
                {
                    if (apDepthStencil[j])
                    {
                        apDepthStencil[j]->Release();
                    }
                }
                if (pMultiSampleTexture)
                {
                    pMultiSampleTexture->Release();
                }
                pDevice->Release();
            }
        }
    }

    VidConfig::SaveConfigFile();
}

// FUNCTION: SWBF2 0x006eb190
bool VidMode_IsDriverMultisampling()
{
    return m_MultiSampleDriverControlled;
}

// FUNCTION: SWBF2 0x006eb1a0
bool VidMode_MultisamplingModeUseable(D3DMULTISAMPLE_TYPE eMultiSampleType,
                                      unsigned long ulMultiSampleQuality)
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    VidConfig* pConfig = VidConfig::GetConfig(identifier.VendorId, identifier.DeviceId,
                                              m_MemoryHint);
    if (!pConfig)
    {
        return true;
    }

    return pConfig->MultisamplingModeAvailable(eMultiSampleType, ulMultiSampleQuality);
}

// FUNCTION: SWBF2 0x006eb220
void VidMode_StartupVoodoo(IDirect3D9* pD3D)
{
    D3DCAPS9 caps;
    pD3D->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);

    DWORD dwBehaviorFlags = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
                                ? D3DCREATE_HARDWARE_VERTEXPROCESSING
                                : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    m_MultiSampleDriverControlled = false;
    m_MemoryHint = 0;

    D3DMULTISAMPLE_TYPE msType = D3DMULTISAMPLE_2_SAMPLES;
    DWORD dwMsQuality = 0;

    if (m_pD3D->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL,
                                           D3DFMT_A8R8G8B8, TRUE,
                                           D3DMULTISAMPLE_2_SAMPLES, 0) != D3D_OK)
    {
        DWORD dwQualityLevels = 0;
        msType = D3DMULTISAMPLE_NONMASKABLE;
        if (m_pD3D->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL,
                                               D3DFMT_A8R8G8B8, TRUE,
                                               D3DMULTISAMPLE_NONMASKABLE,
                                               &dwQualityLevels) != D3D_OK)
        {
            return;
        }
        if (dwQualityLevels == 0)
        {
            return;
        }
        dwMsQuality = dwQualityLevels - 1;
    }

    D3DPRESENT_PARAMETERS pp;
    memset(&pp, 0, sizeof(pp));
    pp.BackBufferWidth = 640;
    pp.BackBufferHeight = 480;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.Windowed = TRUE;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D24S8;
    pp.FullScreen_RefreshRateInHz = 0;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    IDirect3DDevice9* pDevice = 0;
    if (m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hGlobalWnd, dwBehaviorFlags,
                             &pp, &pDevice) != D3D_OK)
    {
        return;
    }

    m_MemoryHint = pDevice->GetAvailableTextureMem();
    pDevice->Release();

    pp.MultiSampleType = msType;
    pp.MultiSampleQuality = dwMsQuality;

    if (m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hGlobalWnd, dwBehaviorFlags,
                             &pp, &pDevice) != D3D_OK)
    {
        return;
    }

    UINT uiAfter = pDevice->GetAvailableTextureMem();
    pDevice->Release();

    if (m_MemoryHint == uiAfter)
    {
        m_MultiSampleDriverControlled = true;
    }
}

// FUNCTION: SWBF2 0x006eb430
bool VidMode_Usable(int iWidth, int iHeight, D3DMULTISAMPLE_TYPE eMultiSampleType,
                    unsigned long ulMultiSampleQuality)
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    VidConfig* pConfig = VidConfig::GetConfig(identifier.VendorId, identifier.DeviceId,
                                              m_MemoryHint);
    if (!pConfig)
    {
        return true;
    }

    return pConfig->VidModeAvailable(iWidth, iHeight, eMultiSampleType, ulMultiSampleQuality);
}

}

static const unsigned int HASH_NONMASKABLE     = 0x05f14b14u;
static const unsigned int HASH_MASTERQUALITY   = 0x0ae6a228u;
static const unsigned int HASH_SHADOWVOLUMES   = 0x00b4b3e8u;
static const unsigned int HASH_NONPOW2TEXTURES = 0x08657a18u;
static const unsigned int HASH_TEXTURECAPS     = 0x1373c7f5u;
static const unsigned int HASH_GEOMETRYMODE    = 0x146f012eu;
static const unsigned int HASH_VOLUMEMAPS      = 0x18185ab4u;
static const unsigned int HASH_BUMPMAPPING     = 0x1cdb7895u;
static const unsigned int HASH_TERRAINQUALITY  = 0x1d6194abu;
static const unsigned int HASH_SPECULARQUALITY = 0x2b24b6d1u;
static const unsigned int HASH_SKINNING        = 0x496610feu;
static const unsigned int HASH_DEFAULTHEIGHT   = 0x4d51a215u;
static const unsigned int HASH_WATERQUALITY    = 0x577bb6a1u;
static const unsigned int HASH_SHADOWQUALITY   = 0x590f9cd2u;
static const unsigned int HASH_TEXTUREQUALITY  = 0x79f4f255u;
static const unsigned int HASH_GENERATION      = 0x708174b7u;

static const unsigned int HASH_ENABLE_HDR = 0x726f6d36u;
static const unsigned int HASH_PARTICLEQUALITY = 0x818751e4u;
static const unsigned int HASH_VENDOR          = 0x97b0c453u;
static const unsigned int HASH_CUBEMAPS        = 0x9c96013fu;
static const unsigned int HASH_TEXTUREDETAIL   = 0xa6af22e5u;
static const unsigned int HASH_DEFAULTWIDTH    = 0xb58502bau;
static const unsigned int HASH_DISTORTION      = 0xbc5efeaau;
static const unsigned int HASH_BACKBUFFERCOPY  = 0xbf962ad1u;
static const unsigned int HASH_DEVICE          = 0xd07076f3u;
static const unsigned int HASH_PIPELINE        = 0xd3442983u;

namespace RedRenderCapOverrides
{

// GLOBAL: SWBF2 0x009aba4a
bool _bOverride_geometry;
// GLOBAL: SWBF2 0x009aba4b
bool _bOverride_pipeline;
// GLOBAL: SWBF2 0x009aba7c
RenderCapsT _RenderCapOverrides;
// GLOBAL: SWBF2 0x009aba9c
bool _bOverride_textureCaps;
// GLOBAL: SWBF2 0x009aba9d
bool _bOverride_skinning;
// GLOBAL: SWBF2 0x009aba9e
bool _bOverride_shadowVolume;
// GLOBAL: SWBF2 0x009aba9f
bool _bOverride_backBufferCopy;
// GLOBAL: SWBF2 0x009abaa0
bool _bOverride_cubemaps;
// GLOBAL: SWBF2 0x009abaa1
bool _bOverride_volumemaps;
// GLOBAL: SWBF2 0x009abaa2
bool _bOverride_nonpow2textures;

// FUNCTION: SWBF2 0x006eb490
void _ApplyVendorDeviceRenderCapOverrides(RenderCapsT& rCaps)
{
    if (_bOverride_geometry)
    {
        rCaps.geometry = _RenderCapOverrides.geometry;
    }
    if (_bOverride_pipeline)
    {
        rCaps.pipeline = _RenderCapOverrides.pipeline;
    }
    if (_bOverride_textureCaps)
    {
        rCaps.textureCaps = _RenderCapOverrides.textureCaps;
    }
    if (_bOverride_skinning)
    {
        rCaps.skinning = _RenderCapOverrides.skinning;
    }
    if (_bOverride_shadowVolume)
    {
        rCaps.shadowVolume = _RenderCapOverrides.shadowVolume;
    }
    if (_bOverride_backBufferCopy)
    {
        rCaps.backBufferCopy = _RenderCapOverrides.backBufferCopy;
    }
    if (_bOverride_cubemaps)
    {
        rCaps.cubemaps = _RenderCapOverrides.cubemaps;
    }
    if (_bOverride_volumemaps)
    {
        rCaps.volumemaps = _RenderCapOverrides.volumemaps;
    }
    if (_bOverride_nonpow2textures)
    {
        rCaps.nonpow2textures = _RenderCapOverrides.nonpow2textures;
    }
}

}

namespace RedAutoSettings
{

// FUNCTION: SWBF2 0x006eb530
void _LoadVendorDeviceOverrides()
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    char szToken[256];
    bool bDeviceMatched = false;
    bool bVendorMatched = false;
    bool bDone = false;

    unsigned int uiKey = 0;
    int iLen = 0;

    PblFile file;

    if (!file.Open("Data\\_LVL_PC\\device.def", true, false, true))
    {
        return;
    }

    int iRenderWidth = 0;
    int iRenderHeight = 0;
    int iSize = file.GetSize();

    while (file.Tell() != iSize)
    {
        if (bDone)
        {
            break;
        }

        int c = tolower(file.Read8());

        switch (c)
        {
        case '\t':
        case '\r':
            break;

        case '/':
            if (iLen > 0 && szToken[iLen - 1] == '/')
            {
                while (file.Read8() != '\n')
                    ;
                iLen = 0;
                break;
            }
            szToken[iLen++] = (char)c;
            break;

        case ':':
        case '=':
            szToken[iLen] = '\0';
            uiKey = PblHash(szToken);
            iLen = 0;
            break;

        case -1:
        case '\n':
            szToken[iLen] = '\0';

            if (!uiKey)
            {
                break;
            }

            switch (uiKey)
            {
            case HASH_VENDOR:
                if (axtoi(szToken, 4) == (int)identifier.VendorId)
                {
                    iLen = 0;
                    bVendorMatched = true;
                    uiKey = 0;
                }
                else if (bVendorMatched)
                {
                    iLen = 0;
                    bDone = true;
                    uiKey = 0;
                }
                else
                {
                    fskip(file, "vendor:");
                    uiKey = HASH_VENDOR;
                    iLen = 0;
                }
                break;

            case HASH_DEVICE:
                if (bDeviceMatched)
                {
                    iLen = 0;
                    bDone = true;
                    uiKey = 0;
                    break;
                }

                strtok(szToken, " ");
                for (;;)
                {
                    char* pcId = strtok(0, " ");

                    if (!pcId)
                    {
                        fskip(file, "device:");
                        uiKey = HASH_DEVICE;
                        iLen = 0;
                        break;
                    }

                    if ((int)identifier.DeviceId == axtoi(pcId, 4))
                    {
                        iLen = 0;
                        bDeviceMatched = true;
                        optionsFounds_ = true;
                        uiKey = 0;
                        break;
                    }
                }
                break;

            case HASH_MASTERQUALITY:
                _RenderSettings.masterQuality = atoi(szToken);
                SetMasterQuality(&_RenderSettings, _RenderSettings.masterQuality);

                _RenderSettings.masterQuality = 3;
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_GENERATION:
                sHardwareGeneration = atoi(szToken);
                SetMasterQuality(&_RenderSettings, sHardwareGeneration - 2);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_NONMASKABLE:
                _NonMaskableCount = 0;
                for (char* pcMode = strtok(szToken, ","); pcMode;
                     pcMode = strtok(0, ","))
                {
                    _NonMaskableStr[_NonMaskableCount] = _strdup(pcMode);
                    ++_NonMaskableCount;

                    if (_NonMaskableCount == 8)
                    {
                        break;
                    }
                }
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_DEFAULTWIDTH:
                iRenderWidth = atoi(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_DEFAULTHEIGHT:
                iRenderHeight = atoi(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_DISTORTION:
                {
                    bool bDistortion = atob(szToken);

                    _RenderSettings.distortion = bDistortion;
                    sCustomRenderPresets[0].distortion = bDistortion;
                    sCustomRenderPresets[1].distortion = bDistortion;
                    sCustomRenderPresets[2].distortion = bDistortion;
                }
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TEXTUREQUALITY:
                _RenderSettings.textureQuality = atotexq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TEXTUREDETAIL:
                _RenderSettings.textureDetail = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_WATERQUALITY:
                _RenderSettings.waterQuality = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_PARTICLEQUALITY:
                _RenderSettings.particleQuality = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_SHADOWQUALITY:
                _RenderSettings.shadowQuality = atoq2(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_SPECULARQUALITY:
                _RenderSettings.specularQuality = atoq2(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TERRAINQUALITY:
                _RenderSettings.terrainQuality = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_BUMPMAPPING:
                _RenderSettings.bumpMapping = atob(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_ENABLE_HDR:
                _RenderSettings.mEnableHDR = atob(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            default:
                iLen = 0;
                uiKey = 0;
                break;
            }
            break;

        default:
            szToken[iLen++] = (char)c;
            break;
        }
    }

    if (iRenderWidth > 0 && iRenderHeight > 0
        && RedVideoConfig::VidMode_Usable(iRenderWidth, iRenderHeight,
                                          D3DMULTISAMPLE_NONE, 0))
    {
        _RenderSettings.defaultWidth = iRenderWidth;
        _RenderSettings.defaultHeight = iRenderHeight;
    }

    file.Close();
}

}

namespace RedRenderCapOverrides
{

// FUNCTION: SWBF2 0x006ebb70
void _LoadVendorDeviceRenderCapOverrides()
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();

    char szToken[256];
    bool bVendorMatched = false;
    bool bDeviceMatched = false;
    bool bDone = false;

    _bOverride_nonpow2textures = false;
    _bOverride_volumemaps = false;
    _bOverride_cubemaps = false;
    _bOverride_backBufferCopy = false;
    _bOverride_shadowVolume = false;
    _bOverride_skinning = false;
    _bOverride_textureCaps = false;
    _bOverride_pipeline = false;
    _bOverride_geometry = false;

    PblFile file;

    if (!file.Open("Data\\_LVL_PC\\device.def", true, false, true))
    {
        return;
    }

    int iSize = file.GetSize();
    unsigned int uiKey = 0;
    int iLen = 0;

    while (file.Tell() != iSize)
    {
        if (bDone)
        {
            break;
        }

        int c = tolower(file.Read8());

        switch (c)
        {
        case '\t':
        case '\r':

            break;

        case '/':
            if (iLen > 0 && szToken[iLen - 1] == '/')
            {
                while (file.Read8() != '\n')
                    ;
                iLen = 0;
                break;
            }
            szToken[iLen++] = (char)c;
            break;

        case ':':
        case '=':
            szToken[iLen] = '\0';
            uiKey = PblHash(szToken);
            iLen = 0;
            break;

        case -1:
        case '\n':
            szToken[iLen] = '\0';

            if (!uiKey)
            {
                break;
            }

            switch (uiKey)
            {
            case HASH_VENDOR:
                if (axtoi(szToken, 4) == (int)identifier.VendorId)
                {
                    iLen = 0;
                    bVendorMatched = true;
                    uiKey = 0;
                }
                else if (bVendorMatched)
                {
                    iLen = 0;
                    bDone = true;
                    uiKey = 0;
                }
                else
                {
                    fskip(file, "vendor:");
                    uiKey = HASH_VENDOR;
                    iLen = 0;
                }
                break;

            case HASH_DEVICE:
                if (bDeviceMatched)
                {
                    iLen = 0;
                    bDone = true;
                    uiKey = 0;
                    break;
                }

                strtok(szToken, " ");
                for (;;)
                {
                    char* pcId = strtok(0, " ");

                    if (!pcId)
                    {
                        fskip(file, "device:");
                        uiKey = HASH_DEVICE;
                        iLen = 0;
                        break;
                    }

                    if ((int)identifier.DeviceId == axtoi(pcId, 4))
                    {
                        iLen = 0;
                        bDeviceMatched = true;
                        uiKey = 0;
                        break;
                    }
                }
                break;

            case HASH_GEOMETRYMODE:
                _bOverride_geometry = true;
                _RenderCapOverrides.geometry = atogeom(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_PIPELINE:

                _bOverride_pipeline = true;
                _RenderCapOverrides.pipeline = atopipe(szToken);
                _bOverride_geometry = true;
                _RenderCapOverrides.geometry = GEOMETRYMODE_UNCOMPRESSED;
                _bOverride_shadowVolume = true;
                _RenderCapOverrides.shadowVolume = SHADOWVOLUME_SOFTWARE;
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TEXTURECAPS:
                _bOverride_textureCaps = true;
                _RenderCapOverrides.textureCaps = atotexc(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_SKINNING:
                _bOverride_skinning = true;
                _RenderCapOverrides.skinning = atoskin(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_SHADOWVOLUMES:
                _bOverride_shadowVolume = true;
                _RenderCapOverrides.shadowVolume = atosv(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_BACKBUFFERCOPY:
                _bOverride_backBufferCopy = true;
                _RenderCapOverrides.backBufferCopy = atobbc(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_CUBEMAPS:
                _bOverride_cubemaps = true;
                _RenderCapOverrides.cubemaps = (atoi(szToken) != 0);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_VOLUMEMAPS:
                _bOverride_volumemaps = true;
                _RenderCapOverrides.volumemaps = (atoi(szToken) != 0);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_NONPOW2TEXTURES:
                _bOverride_nonpow2textures = true;
                _RenderCapOverrides.nonpow2textures = atotexpow2(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TEXTUREQUALITY:
                RedAutoSettings::_RenderSettings.textureQuality = atotexq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TEXTUREDETAIL:
                RedAutoSettings::_RenderSettings.textureDetail = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_WATERQUALITY:
                RedAutoSettings::_RenderSettings.waterQuality = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_PARTICLEQUALITY:
                RedAutoSettings::_RenderSettings.particleQuality = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_SHADOWQUALITY:
                RedAutoSettings::_RenderSettings.shadowQuality = atoq2(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_SPECULARQUALITY:
                RedAutoSettings::_RenderSettings.specularQuality = atoq2(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_TERRAINQUALITY:
                RedAutoSettings::_RenderSettings.terrainQuality = atoq(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            case HASH_BUMPMAPPING:
                RedAutoSettings::_RenderSettings.bumpMapping = atob(szToken);
                iLen = 0;
                uiKey = 0;
                break;

            default:
                iLen = 0;
                uiKey = 0;
                break;
            }
            break;

        default:
            szToken[iLen++] = (char)c;
            break;
        }
    }

    file.Close();
}

}

namespace RedAutoSettings
{

// FUNCTION: SWBF2 0x006ec1e0
void SetMasterQuality(RenderOptionsT* pOptions, int iLevel)
{
    pOptions->masterQuality = iLevel;
    pOptions->mGraphicsTier = iLevel;
    pOptions->textureQuality = TEXTURE_QUALITY_COMPRESSED;

    if (iLevel == 0)
    {
        pOptions->textureDetail = QUALITY_LOW;
        pOptions->terrainQuality = QUALITY_LOW;
        pOptions->waterQuality = QUALITY_LOW;
        pOptions->particleQuality = QUALITY_LOW;
        pOptions->shadowQuality = QUALITY2_OFF;
        pOptions->specularQuality = QUALITY2_LOW;
        pOptions->bumpMapping = false;
        pOptions->motionBlur = false;
        pOptions->distortion = true;
        pOptions->mEnableHDR = false;
        pOptions->mNearSceneFadeFactor = 0.2f;
        pOptions->mFarSceneRangeFactor = 0.2f;
    }
    else
    {
        pOptions->specularQuality = QUALITY2_HIGH;
        pOptions->motionBlur = true;
        pOptions->distortion = true;

        if (iLevel == 1)
        {
            pOptions->textureDetail = QUALITY_MEDIUM;
            pOptions->terrainQuality = QUALITY_MEDIUM;
            pOptions->waterQuality = QUALITY_MEDIUM;
            pOptions->particleQuality = QUALITY_MEDIUM;
            pOptions->shadowQuality = QUALITY2_LOW;
            pOptions->bumpMapping = false;
            pOptions->mEnableHDR = false;
            pOptions->mNearSceneFadeFactor = 0.5f;
            pOptions->mFarSceneRangeFactor = 0.5f;

            if (RedVideoConfig::VidMode_Usable(1024, 768, D3DMULTISAMPLE_NONE, 0))
            {
                pOptions->defaultWidth = 1024;
                pOptions->defaultHeight = 768;
                _SetGeneration(pOptions);
                return;
            }
        }
        else
        {
            pOptions->textureDetail = QUALITY_HIGH;
            pOptions->terrainQuality = QUALITY_HIGH;
            pOptions->waterQuality = QUALITY_HIGH;
            pOptions->particleQuality = QUALITY_HIGH;
            pOptions->shadowQuality = QUALITY2_HIGH;
            pOptions->bumpMapping = true;
            pOptions->mEnableHDR = true;
            pOptions->mNearSceneFadeFactor = 0.7f;
            pOptions->mFarSceneRangeFactor = 0.7f;

            if (RedVideoConfig::VidMode_Usable(1280, 960, D3DMULTISAMPLE_NONE, 0))
            {
                pOptions->defaultWidth = 1280;
                pOptions->defaultHeight = 960;
                _SetGeneration(pOptions);
                return;
            }

            if (RedVideoConfig::VidMode_Usable(1024, 768, D3DMULTISAMPLE_NONE, 0))
            {
                pOptions->defaultWidth = 1024;
                pOptions->defaultHeight = 768;
                _SetGeneration(pOptions);
                return;
            }
        }
    }

    pOptions->defaultWidth = 800;
    pOptions->defaultHeight = 600;
    _SetGeneration(pOptions);
}

}

// FUNCTION: SWBF2 0x006ec390
bool atob(char const* pszValue)
{
    if (!_stricmp(pszValue, "on"))
    {
        return true;
    }
    if (!_stricmp(pszValue, "off"))
    {
        return false;
    }
    return false;
}

// FUNCTION: SWBF2 0x006ec3d0
BackBufferCopyT atobbc(char const* pszValue)
{
    if (!_stricmp(pszValue, "stretch"))
    {
        return BACKBUFFERCOPY_CAN_STRETCH;
    }
    if (!_stricmp(pszValue, "copy"))
    {
        return BACKBUFFERCOPY_CAN_COPY;
    }
    if (!_stricmp(pszValue, "none"))
    {
        return BACKBUFFERCOPY_NONE;
    }
    return BACKBUFFERCOPY_NONE;
}

// FUNCTION: SWBF2 0x006ec420
GeometryModeT atogeom(char const* pszValue)
{
    if (!_stricmp(pszValue, "uncompressed"))
    {
        return GEOMETRYMODE_UNCOMPRESSED;
    }
    if (!_stricmp(pszValue, "compressed"))
    {
        return GEOMETRYMODE_COMPRESSED;
    }
    return GEOMETRYMODE_UNCOMPRESSED;
}

// FUNCTION: SWBF2 0x006ec460
PipelineT atopipe(char const* pszValue)
{
    if (!_stricmp(pszValue, "fixed"))
    {
        return PIPELINE_FIXED;
    }
    if (!_stricmp(pszValue, "1.1"))
    {
        return PIPELINE_PROG1;
    }
    if (!_stricmp(pszValue, "2.0"))
    {
        return PIPELINE_PROG2;
    }
    return PIPELINE_FIXED;
}

// FUNCTION: SWBF2 0x006ec4c0
Quality2T atoq2(char const* pszValue)
{
    if (!_stricmp(pszValue, "high"))
    {
        return QUALITY2_HIGH;
    }
    if (!_stricmp(pszValue, "medium"))
    {
        return QUALITY2_HIGH;
    }
    if (!_stricmp(pszValue, "low"))
    {
        return QUALITY2_LOW;
    }
    if (!_stricmp(pszValue, "off"))
    {
        return QUALITY2_OFF;
    }
    return QUALITY2_OFF;
}

// FUNCTION: SWBF2 0x006ec520
QualityT atoq(char const* pszValue)
{
    if (!_stricmp(pszValue, "high"))
    {
        return QUALITY_HIGH;
    }
    if (!_stricmp(pszValue, "medium"))
    {
        return QUALITY_MEDIUM;
    }
    if (!_stricmp(pszValue, "low"))
    {
        return QUALITY_LOW;
    }
    return QUALITY_LOW;
}

// FUNCTION: SWBF2 0x006ec570
SkinningModeT atoskin(char const* pszValue)
{
    if (!_stricmp(pszValue, "software"))
    {
        return SKINNING_SOFTWARE;
    }
    if (!_stricmp(pszValue, "hardware"))
    {
        return SKINNING_HARDWARE;
    }
    return SKINNING_SOFTWARE;
}

// FUNCTION: SWBF2 0x006ec5b0
ShadowVolumeModeT atosv(char const* pszValue)
{
    if (!_stricmp(pszValue, "software"))
    {
        return SHADOWVOLUME_SOFTWARE;
    }
    if (!_stricmp(pszValue, "mixed"))
    {
        return SHADOWVOLUME_SOFTWARE_SKINNED;
    }
    if (!_stricmp(pszValue, "hardware"))
    {
        return SHADOWVOLUME_HARDWARE;
    }
    return SHADOWVOLUME_SOFTWARE;
}

// FUNCTION: SWBF2 0x006ec610
TextureCapsT atotexc(char const* pszValue)
{
    if (!_stricmp(pszValue, "uncompressed"))
    {
        return TEXTURE_CAPS_UNCOMPRESSED;
    }
    if (!_stricmp(pszValue, "compressed"))
    {
        return TEXTURE_CAPS_COMPRESSED;
    }
    return TEXTURE_CAPS_UNCOMPRESSED;
}

// FUNCTION: SWBF2 0x006ec650
TextureNonPow2SupportT atotexpow2(char const* pszValue)
{
    if (!_stricmp(pszValue, "always"))
    {
        return TEXTURE_NONPOW2_ALWAYS;
    }
    if (!_stricmp(pszValue, "conditional"))
    {
        return TEXTURE_NONPOW2_CONDITIONAL;
    }
    if (!_stricmp(pszValue, "never"))
    {
        return TEXTURE_NONPOW2_NEVER;
    }
    return TEXTURE_NONPOW2_NEVER;
}

// FUNCTION: SWBF2 0x006ec6a0
TextureQualityT atotexq(char const* pszValue)
{
    if (!_stricmp(pszValue, "32"))
    {
        return TEXTURE_QUALITY_32BITS;
    }
    if (!_stricmp(pszValue, "16"))
    {
        return TEXTURE_QUALITY_16BITS;
    }
    if (!_stricmp(pszValue, "compressed"))
    {
        return TEXTURE_QUALITY_COMPRESSED;
    }
    return TEXTURE_QUALITY_16BITS;
}

// FUNCTION: SWBF2 0x006ec7c0
int fskip(PblFile& rFile, char const* pcToken)
{
    int iLen = (int)strlen(pcToken);
    int iSize = rFile.GetSize();
    int iMatched = 0;

    while (rFile.Tell() != iSize)
    {
        if ((char)tolower(rFile.Read8()) == pcToken[iMatched])
        {
            ++iMatched;
        }
        else
        {
            iMatched = 0;
        }

        if (iMatched == iLen)
        {
            return PblHash(pcToken);
        }
    }

    return 0;
}

// SYNTHETIC: SWBF2 0x00408b60
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00408b80
// `dynamic initializer for 'PblMath::fOneOverTwoPi''
