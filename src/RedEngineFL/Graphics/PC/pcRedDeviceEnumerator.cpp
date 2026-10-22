#include "pcRedDeviceEnumerator.h"

namespace pcRedDeviceEnumerator
{

// GLOBAL: SWBF2 0x007e2318
unsigned int AppMinFullscreenWidth = 640;
// GLOBAL: SWBF2 0x007e231c
unsigned int AppMinFullscreenHeight = 480;
// GLOBAL: SWBF2 0x007e2320
unsigned int AppMinColorChannelBits = 5;
// GLOBAL: SWBF2 0x007e2324
unsigned int AppMinAlphaChannelBits = 1;
// GLOBAL: SWBF2 0x007e2328
unsigned int AppMinDepthBits = 15;
// GLOBAL: SWBF2 0x007e232c
unsigned int AppMinStencilBits = 1;
// GLOBAL: SWBF2 0x007e2330
bool AppUsesDepthBuffer = true;

// GLOBAL: SWBF2 0x007e2334

PblList<D3DAdapterInfo> m_AdapterInfoList;

// GLOBAL: SWBF2 0x009cab00
bool AppUsesMixedVP = false;
// GLOBAL: SWBF2 0x009cab01
bool AppRequiresWindowed = false;
// GLOBAL: SWBF2 0x009cab02
bool AppRequiresFullscreen = false;
// GLOBAL: SWBF2 0x009cab04
bool (*ConfirmDeviceCallback)(D3DCAPS9* pCaps, VertexProcessingType vpType,
                              D3DFORMAT backBufferFormat) = 0;

// GLOBAL: SWBF2 0x009cab08

IDirect3D9* m_pD3D = 0;

}

// FUNCTION: SWBF2 0x00716aa0
unsigned int ColorChannelBits(D3DFORMAT fmt)
{
    switch (fmt)
    {
    case D3DFMT_R8G8B8:
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
        return 8;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
        return 5;
    case D3DFMT_A4R4G4B4:
    case D3DFMT_X4R4G4B4:
        return 4;
    case D3DFMT_R3G3B2:
    case D3DFMT_A8R3G3B2:
        return 2;
    case D3DFMT_A2B10G10R10:
    case D3DFMT_A2R10G10B10:
        return 10;
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x007162f0
unsigned int AlphaChannelBits(D3DFORMAT fmt)
{
    switch (fmt)
    {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_A8R3G3B2:
        return 8;
    case D3DFMT_A1R5G5B5:
        return 1;
    case D3DFMT_A4R4G4B4:
        return 4;
    case D3DFMT_A2B10G10R10:
    case D3DFMT_A2R10G10B10:
        return 2;
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x00716b20
unsigned int DepthBits(D3DFORMAT fmt)
{
    switch (fmt)
    {
    case D3DFMT_D16:
        return 16;
    case D3DFMT_D15S1:
        return 15;
    case D3DFMT_D24S8:
    case D3DFMT_D24X8:
    case D3DFMT_D24X4S4:
        return 24;
    case D3DFMT_D32:
        return 32;
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x00717210
unsigned int StencilBits(D3DFORMAT fmt)
{
    switch (fmt)
    {
    case D3DFMT_D15S1:
        return 1;
    case D3DFMT_D24S8:
        return 8;
    case D3DFMT_D24X4S4:
        return 4;
    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x00715f00
D3DDeviceCombo::D3DDeviceCombo()
{
}

// FUNCTION: SWBF2 0x00716090
D3DDeviceCombo::~D3DDeviceCombo()
{
    DepthStencilFormatList.Clear();
    TextureFormatList.Clear();
    MultiSampleTypeList.Clear();
    MultiSampleQualityList.Clear();
    DSMSConflictList.Clear();
    VertexProcessingTypeList.Clear();
    PresentIntervalList.Clear();
}

// FUNCTION: SWBF2 0x00716290
D3DDeviceInfo::~D3DDeviceInfo()
{
    DeviceComboList.Clear();
}

// FUNCTION: SWBF2 0x00715fe0
D3DAdapterInfo::~D3DAdapterInfo()
{
    DisplayModeList.Clear();
    DeviceInfoList.Clear();
}

namespace pcRedDeviceEnumerator
{

// FUNCTION: SWBF2 0x007167a0
void BuildTextureFormatList(D3DDeviceCombo* pDeviceCombo)
{
    D3DFORMAT afmtTextureFormats[28];
    afmtTextureFormats[0] = D3DFMT_R3G3B2;
    afmtTextureFormats[1] = D3DFMT_L8;
    afmtTextureFormats[2] = D3DFMT_A8;
    afmtTextureFormats[3] = D3DFMT_A4L4;
    afmtTextureFormats[4] = D3DFMT_P8;
    afmtTextureFormats[5] = D3DFMT_R5G6B5;
    afmtTextureFormats[6] = D3DFMT_X1R5G5B5;
    afmtTextureFormats[7] = D3DFMT_X4R4G4B4;
    afmtTextureFormats[8] = D3DFMT_A1R5G5B5;
    afmtTextureFormats[9] = D3DFMT_A4R4G4B4;
    afmtTextureFormats[10] = D3DFMT_A8R3G3B2;
    afmtTextureFormats[11] = D3DFMT_L16;
    afmtTextureFormats[12] = D3DFMT_A8L8;
    afmtTextureFormats[13] = D3DFMT_A8P8;
    afmtTextureFormats[14] = D3DFMT_R8G8B8;
    afmtTextureFormats[15] = D3DFMT_X8R8G8B8;
    afmtTextureFormats[16] = D3DFMT_X8B8G8R8;
    afmtTextureFormats[17] = D3DFMT_G16R16;
    afmtTextureFormats[18] = D3DFMT_A8R8G8B8;
    afmtTextureFormats[19] = D3DFMT_A8B8G8R8;
    afmtTextureFormats[20] = D3DFMT_A2R10G10B10;
    afmtTextureFormats[21] = D3DFMT_A2B10G10R10;
    afmtTextureFormats[22] = D3DFMT_DXT1;
    afmtTextureFormats[23] = D3DFMT_DXT2;
    afmtTextureFormats[24] = D3DFMT_DXT3;
    afmtTextureFormats[25] = D3DFMT_DXT4;
    afmtTextureFormats[26] = D3DFMT_DXT5;
    afmtTextureFormats[27] = D3DFMT_A16B16G16R16;

    for (unsigned int ifmt = 0; ifmt < 28; ++ifmt)
    {
        const D3DFORMAT fmt = afmtTextureFormats[ifmt];

        if (FAILED(m_pD3D->CheckDeviceFormat(pDeviceCombo->AdapterOrdinal,
                                             pDeviceCombo->DevType,
                                             pDeviceCombo->AdapterFormat,
                                             0, D3DRTYPE_TEXTURE, fmt)))
        {
            continue;
        }

        D3DFormat* pTextureFormat = new D3DFormat;
        pTextureFormat->format = fmt;
        pDeviceCombo->TextureFormatList.AddLast(&pTextureFormat->mNode, pTextureFormat);
    }
}

// FUNCTION: SWBF2 0x00716460
void BuildDepthStencilFormatList(D3DDeviceCombo* pDeviceCombo)
{
    D3DFORMAT afmtDepthStencilFormats[6];
    afmtDepthStencilFormats[0] = D3DFMT_D16;
    afmtDepthStencilFormats[1] = D3DFMT_D15S1;
    afmtDepthStencilFormats[2] = D3DFMT_D24X8;
    afmtDepthStencilFormats[3] = D3DFMT_D24S8;
    afmtDepthStencilFormats[4] = D3DFMT_D24X4S4;
    afmtDepthStencilFormats[5] = D3DFMT_D32;

    for (unsigned int ifmt = 0; ifmt < 6; ++ifmt)
    {
        const D3DFORMAT fmt = afmtDepthStencilFormats[ifmt];

        if (DepthBits(fmt) < AppMinDepthBits)
        {
            continue;
        }
        if (StencilBits(fmt) < AppMinStencilBits)
        {
            continue;
        }

        if (FAILED(m_pD3D->CheckDeviceFormat(pDeviceCombo->AdapterOrdinal,
                                             pDeviceCombo->DevType,
                                             pDeviceCombo->AdapterFormat,
                                             D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, fmt)))
        {
            continue;
        }
        if (FAILED(m_pD3D->CheckDepthStencilMatch(pDeviceCombo->AdapterOrdinal,
                                                  pDeviceCombo->DevType,
                                                  pDeviceCombo->AdapterFormat,
                                                  pDeviceCombo->BackBufferFormat, fmt)))
        {
            continue;
        }

        D3DFormat* pDepthStencilFormat = new D3DFormat;
        pDepthStencilFormat->format = fmt;
        pDeviceCombo->DepthStencilFormatList.AddLast(&pDepthStencilFormat->mNode,
                                                     pDepthStencilFormat);
    }
}

// FUNCTION: SWBF2 0x00716570
void BuildMultiSampleTypeList(D3DDeviceCombo* pDeviceCombo)
{
    D3DMULTISAMPLE_TYPE amsTypeArray[D3DMULTISAMPLE_16_SAMPLES + 1];
    for (int i = 0; i <= D3DMULTISAMPLE_16_SAMPLES; ++i)
    {
        amsTypeArray[i] = (D3DMULTISAMPLE_TYPE)i;
    }

    for (unsigned int ims = 0; ims <= (unsigned int)D3DMULTISAMPLE_16_SAMPLES; ++ims)
    {
        const D3DMULTISAMPLE_TYPE msType = amsTypeArray[ims];
        DWORD dwMSQuality = 0;

        if (FAILED(m_pD3D->CheckDeviceMultiSampleType(pDeviceCombo->AdapterOrdinal,
                                                      pDeviceCombo->DevType,
                                                      pDeviceCombo->BackBufferFormat,
                                                      pDeviceCombo->IsWindowed,
                                                      msType, &dwMSQuality)))
        {
            continue;
        }

        D3DMultisampleType* pMultiSampleType = new D3DMultisampleType;
        pMultiSampleType->MSType = msType;
        D3DMultisampleQuality* pMultiSampleQuality = new D3DMultisampleQuality;
        pMultiSampleQuality->MSQuality = dwMSQuality;

        pDeviceCombo->MultiSampleTypeList.AddLast(&pMultiSampleType->mNode, pMultiSampleType);
        pDeviceCombo->MultiSampleQualityList.AddLast(&pMultiSampleQuality->mNode,
                                                     pMultiSampleQuality);
    }
}

// FUNCTION: SWBF2 0x00716370
void BuildDSMSConflictList(D3DDeviceCombo* pDeviceCombo)
{
    PblList<D3DFormat>& dsList = pDeviceCombo->DepthStencilFormatList;
    for (PblList<D3DFormat>::Node* pDSNode = dsList.GetFirst();
         pDSNode->_pObject != dsList._head._pObject;
         pDSNode = pDSNode->_pNext)
    {
        const D3DFORMAT dsFmt = pDSNode->_pObject->format;

        PblList<D3DMultisampleType>& msList = pDeviceCombo->MultiSampleTypeList;
        for (PblList<D3DMultisampleType>::Node* pMSNode = msList.GetFirst();
             pMSNode->_pObject != msList._head._pObject;
             pMSNode = pMSNode->_pNext)
        {
            const D3DMULTISAMPLE_TYPE msType = pMSNode->_pObject->MSType;

            if (SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(pDeviceCombo->AdapterOrdinal,
                                                             pDeviceCombo->DevType,
                                                             dsFmt,
                                                             pDeviceCombo->IsWindowed,
                                                             msType, 0)))
            {
                continue;
            }

            D3DDSMSConflict* pDSMSConflict = new D3DDSMSConflict;
            pDSMSConflict->DSFormat = dsFmt;
            pDSMSConflict->MSType = msType;
            pDeviceCombo->DSMSConflictList.AddLast(&pDSMSConflict->mNode, pDSMSConflict);
        }
    }
}

// FUNCTION: SWBF2 0x007166d0
void BuildPresentIntervalList(D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo)
{
    UINT auiPresentIntervals[6];
    auiPresentIntervals[0] = D3DPRESENT_INTERVAL_IMMEDIATE;
    auiPresentIntervals[1] = D3DPRESENT_INTERVAL_DEFAULT;
    auiPresentIntervals[2] = D3DPRESENT_INTERVAL_ONE;
    auiPresentIntervals[3] = D3DPRESENT_INTERVAL_TWO;
    auiPresentIntervals[4] = D3DPRESENT_INTERVAL_THREE;
    auiPresentIntervals[5] = D3DPRESENT_INTERVAL_FOUR;

    for (unsigned int i = 0; i < 6; ++i)
    {
        const UINT uiInterval = auiPresentIntervals[i];

        if (pDeviceCombo->IsWindowed)
        {
            if (uiInterval == D3DPRESENT_INTERVAL_TWO ||
                uiInterval == D3DPRESENT_INTERVAL_THREE ||
                uiInterval == D3DPRESENT_INTERVAL_FOUR)
            {
                continue;
            }
        }

        if (uiInterval != D3DPRESENT_INTERVAL_DEFAULT &&
            !(pDeviceInfo->Caps.PresentationIntervals & uiInterval))
        {
            continue;
        }

        D3DPresentInterval* pPresentInterval = new D3DPresentInterval;
        pPresentInterval->presentInterval = uiInterval;
        pDeviceCombo->PresentIntervalList.AddLast(&pPresentInterval->mNode, pPresentInterval);
    }
}

// FUNCTION: SWBF2 0x007168a0
void BuildVertexProcessingTypeList(D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo)
{
    if (pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    {
        if (pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE)
        {
            if (ConfirmDeviceCallback == 0 ||
                ConfirmDeviceCallback(&pDeviceInfo->Caps, PURE_HARDWARE_VP,
                                      pDeviceCombo->BackBufferFormat))
            {
                D3DVertexProcessingType* pVertexProcessingType = new D3DVertexProcessingType;
                pVertexProcessingType->vpType = PURE_HARDWARE_VP;
                pDeviceCombo->VertexProcessingTypeList.AddLast(&pVertexProcessingType->mNode,
                                                               pVertexProcessingType);
            }
        }

        if (ConfirmDeviceCallback == 0 ||
            ConfirmDeviceCallback(&pDeviceInfo->Caps, HARDWARE_VP,
                                  pDeviceCombo->BackBufferFormat))
        {
            D3DVertexProcessingType* pVertexProcessingType = new D3DVertexProcessingType;
            pVertexProcessingType->vpType = HARDWARE_VP;
            pDeviceCombo->VertexProcessingTypeList.AddLast(&pVertexProcessingType->mNode,
                                                           pVertexProcessingType);
        }

        if (AppUsesMixedVP)
        {
            if (ConfirmDeviceCallback == 0 ||
                ConfirmDeviceCallback(&pDeviceInfo->Caps, MIXED_VP,
                                      pDeviceCombo->BackBufferFormat))
            {
                D3DVertexProcessingType* pVertexProcessingType = new D3DVertexProcessingType;
                pVertexProcessingType->vpType = MIXED_VP;
                pDeviceCombo->VertexProcessingTypeList.AddLast(&pVertexProcessingType->mNode,
                                                               pVertexProcessingType);
            }
        }
    }

    if (ConfirmDeviceCallback == 0 ||
        ConfirmDeviceCallback(&pDeviceInfo->Caps, SOFTWARE_VP,
                              pDeviceCombo->BackBufferFormat))
    {
        D3DVertexProcessingType* pVertexProcessingType = new D3DVertexProcessingType;
        pVertexProcessingType->vpType = SOFTWARE_VP;
        pDeviceCombo->VertexProcessingTypeList.AddLast(&pVertexProcessingType->mNode,
                                                       pVertexProcessingType);
    }
}

// FUNCTION: SWBF2 0x00716f20
void EnumerateDeviceCombos(D3DDeviceInfo* pDeviceInfo, PblList<D3DFormat>* pAdapterFormatList)
{
    D3DFORMAT afmtBackBufferFormats[6];
    afmtBackBufferFormats[0] = D3DFMT_A8R8G8B8;
    afmtBackBufferFormats[1] = D3DFMT_X8R8G8B8;
    afmtBackBufferFormats[2] = D3DFMT_A2R10G10B10;
    afmtBackBufferFormats[3] = D3DFMT_R5G6B5;
    afmtBackBufferFormats[4] = D3DFMT_A1R5G5B5;
    afmtBackBufferFormats[5] = D3DFMT_X1R5G5B5;

    bool abWindowed[2];
    abWindowed[0] = false;
    abWindowed[1] = true;

    for (PblList<D3DFormat>::Node* pNode = pAdapterFormatList->GetFirst();
         pNode->_pObject != pAdapterFormatList->_head._pObject;
         pNode = pNode->_pNext)
    {
        const D3DFORMAT adapterFormat = pNode->_pObject->format;

        for (unsigned int ifmt = 0; ifmt < 6; ++ifmt)
        {
            const D3DFORMAT backBufferFormat = afmtBackBufferFormats[ifmt];

            if (AlphaChannelBits(backBufferFormat) < AppMinAlphaChannelBits)
            {
                continue;
            }

            for (unsigned int iwindowed = 0; iwindowed < 2; ++iwindowed)
            {
                const bool bWindowed = abWindowed[iwindowed];

                if (!bWindowed && AppRequiresWindowed)
                {
                    continue;
                }
                if (bWindowed && AppRequiresFullscreen)
                {
                    continue;
                }

                if (FAILED(m_pD3D->CheckDeviceType(pDeviceInfo->AdapterOrdinal,
                                                   pDeviceInfo->DevType,
                                                   adapterFormat, backBufferFormat,
                                                   bWindowed)))
                {
                    continue;
                }

                D3DDeviceCombo* pDeviceCombo = new D3DDeviceCombo;
                if (pDeviceCombo == 0)
                {
                    return;
                }

                pDeviceCombo->AdapterOrdinal = pDeviceInfo->AdapterOrdinal;
                pDeviceCombo->DevType = pDeviceInfo->DevType;
                pDeviceCombo->AdapterFormat = adapterFormat;
                pDeviceCombo->BackBufferFormat = backBufferFormat;
                pDeviceCombo->IsWindowed = bWindowed;

                BuildTextureFormatList(pDeviceCombo);

                if (AppUsesDepthBuffer)
                {
                    BuildDepthStencilFormatList(pDeviceCombo);
                    if (pDeviceCombo->DepthStencilFormatList.GetCount() == 0)
                    {
                        delete pDeviceCombo;
                        continue;
                    }
                }

                BuildMultiSampleTypeList(pDeviceCombo);
                if (pDeviceCombo->MultiSampleTypeList.GetCount() == 0)
                {
                    delete pDeviceCombo;
                    continue;
                }

                BuildDSMSConflictList(pDeviceCombo);
                BuildVertexProcessingTypeList(pDeviceInfo, pDeviceCombo);
                if (pDeviceCombo->VertexProcessingTypeList.GetCount() == 0)
                {
                    delete pDeviceCombo;
                    continue;
                }

                BuildPresentIntervalList(pDeviceInfo, pDeviceCombo);
                pDeviceInfo->DeviceComboList.AddLast(&pDeviceCombo->mNode, pDeviceCombo);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00717100
void EnumerateDevices(D3DAdapterInfo* pAdapterInfo, PblList<D3DFormat>* pAdapterFormatList)
{
    D3DDEVTYPE adevTypeArray[1];
    adevTypeArray[0] = D3DDEVTYPE_HAL;
    const unsigned int uiDevTypeArrayCount = sizeof(adevTypeArray) / sizeof(adevTypeArray[0]);

    for (unsigned int iDeviceType = 0; iDeviceType < uiDevTypeArrayCount; ++iDeviceType)
    {
        D3DDeviceInfo* pDeviceInfo = new D3DDeviceInfo;

        if (pDeviceInfo == 0)
        {
            return;
        }

        pDeviceInfo->AdapterOrdinal = pAdapterInfo->AdapterOrdinal;
        pDeviceInfo->DevType = adevTypeArray[iDeviceType];

        if (FAILED(m_pD3D->GetDeviceCaps(pAdapterInfo->AdapterOrdinal, pDeviceInfo->DevType,
                                         &pDeviceInfo->Caps)))
        {
            delete pDeviceInfo;
            continue;
        }

        EnumerateDeviceCombos(pDeviceInfo, pAdapterFormatList);
        if (pDeviceInfo->DeviceComboList.GetCount() == 0)
        {
            delete pDeviceInfo;
            continue;
        }

        pAdapterInfo->DeviceInfoList.AddLast(&pDeviceInfo->mNode, pDeviceInfo);
    }
}

// FUNCTION: SWBF2 0x00716b80
void Enumerate(unsigned int uiMinColorChannelBits, unsigned int uiMinAlphaChannelBits,
               unsigned int uiMinDepthBits, unsigned int uiMinStencilBits)
{
    PblList<D3DFormat> adapterFormatList;

    AppMinColorChannelBits = uiMinColorChannelBits;
    AppMinAlphaChannelBits = uiMinAlphaChannelBits;
    AppMinDepthBits = uiMinDepthBits;
    AppMinStencilBits = uiMinStencilBits;

    const UINT uiNumAdapters = m_pD3D->GetAdapterCount();
    for (UINT uiAdapter = 0; uiAdapter < uiNumAdapters; ++uiAdapter)
    {
        D3DAdapterInfo* pAdapterInfo = new D3DAdapterInfo;
        pAdapterInfo->AdapterOrdinal = uiAdapter;
        m_pD3D->GetAdapterIdentifier(uiAdapter, 0, &pAdapterInfo->AdapterIdentifier);

        while (adapterFormatList.GetCount() > 0)
        {
            adapterFormatList.Remove(adapterFormatList.GetFirst());
        }

        const UINT uiNumAdapterModes =
            m_pD3D->GetAdapterModeCount(uiAdapter, D3DFMT_X8R8G8B8);
        for (UINT uiMode = 0; uiMode < uiNumAdapterModes; ++uiMode)
        {
            D3DDISPLAYMODE displayMode;
            m_pD3D->EnumAdapterModes(uiAdapter, D3DFMT_X8R8G8B8, uiMode, &displayMode);

            if (displayMode.Width < AppMinFullscreenWidth)
            {
                continue;
            }
            if (displayMode.Height < AppMinFullscreenHeight)
            {
                continue;
            }
            if (ColorChannelBits(displayMode.Format) < AppMinColorChannelBits)
            {
                continue;
            }

            D3DDisplayMode* pDisplayMode = new D3DDisplayMode;
            pDisplayMode->displayMode = displayMode;

            PblList<D3DDisplayMode>& modeList = pAdapterInfo->DisplayModeList;
            for (PblList<D3DDisplayMode>::Node* pAt = modeList.GetFirst();
                 pAt->_pObject != modeList._head._pObject;
                 pAt = pAt->_pNext)
            {
                const D3DDISPLAYMODE& mode = pAt->_pObject->displayMode;
                if (mode.Width != displayMode.Width)
                {
                    if (mode.Width > displayMode.Width)
                    {
                        modeList.AddBefore(pAt, &pDisplayMode->mNode, pDisplayMode);
                        break;
                    }
                    continue;
                }
                if (mode.Height != displayMode.Height)
                {
                    if (mode.Height > displayMode.Height)
                    {
                        modeList.AddBefore(pAt, &pDisplayMode->mNode, pDisplayMode);
                        break;
                    }
                    continue;
                }
                if (mode.RefreshRate >= displayMode.RefreshRate)
                {
                    modeList.AddBefore(pAt, &pDisplayMode->mNode, pDisplayMode);
                    break;
                }
            }
            if (pDisplayMode->mNode._pList == 0)
            {
                modeList.AddLast(&pDisplayMode->mNode, pDisplayMode);
            }

            bool bFound = false;
            for (PblList<D3DFormat>::Node* pFmt = adapterFormatList.GetFirst();
                 pFmt->_pObject != adapterFormatList._head._pObject;
                 pFmt = pFmt->_pNext)
            {
                if (pFmt->_pObject->format == displayMode.Format)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                D3DFormat* pAdapterFormat = new D3DFormat;
                pAdapterFormat->format = displayMode.Format;
                adapterFormatList.AddLast(&pAdapterFormat->mNode, pAdapterFormat);
            }
        }

        EnumerateDevices(pAdapterInfo, &adapterFormatList);

        if (pAdapterInfo->DeviceInfoList.GetCount() == 0)
        {
            delete pAdapterInfo;
        }
        else
        {
            m_AdapterInfoList.AddLast(&pAdapterInfo->mNode, pAdapterInfo);
        }
    }

    adapterFormatList.Clear();
}

// FUNCTION: SWBF2 0x007171f0
PblList<D3DAdapterInfo>& GetDeviceList()
{
    return m_AdapterInfoList;
}

// FUNCTION: SWBF2 0x00717200
void Init(IDirect3D9* pD3D)
{
    m_pD3D = pD3D;
}
}
