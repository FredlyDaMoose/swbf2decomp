#pragma once

#ifndef SWBF2_GRAPHICS_PC_PCREDDEVICEENUMERATOR_H
#define SWBF2_GRAPHICS_PC_PCREDDEVICEENUMERATOR_H

#include <d3d9.h>
#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

class D3DAdapterInfo;
class D3DDeviceCombo;
class D3DDeviceInfo;
struct D3DDSMSConflict;
struct D3DDisplayMode;
struct D3DFormat;
struct D3DMultisampleQuality;
struct D3DMultisampleType;
struct D3DPresentInterval;
struct D3DVertexProcessingType;

enum VertexProcessingType
{
    SOFTWARE_VP = 0,
    MIXED_VP = 1,
    HARDWARE_VP = 2,
    PURE_HARDWARE_VP = 3
};

struct D3DDisplayMode
{
    PblList<D3DDisplayMode>::Node mNode;
    D3DDISPLAYMODE displayMode;
};

 
 

struct D3DFormat
{
    PblList<D3DFormat>::Node mNode;
    D3DFORMAT format;
};

 
 

struct D3DMultisampleType
{
    PblList<D3DMultisampleType>::Node mNode;
    D3DMULTISAMPLE_TYPE MSType;
};
 

struct D3DMultisampleQuality
{
    PblList<D3DMultisampleQuality>::Node mNode;
    DWORD MSQuality;
};
 

struct D3DDSMSConflict
{
    PblList<D3DDSMSConflict>::Node mNode;
    D3DFORMAT DSFormat;
    D3DMULTISAMPLE_TYPE MSType;
};

 
 

struct D3DPresentInterval
{
    PblList<D3DPresentInterval>::Node mNode;
    UINT presentInterval;
};
 

struct D3DVertexProcessingType
{
    PblList<D3DVertexProcessingType>::Node mNode;
    VertexProcessingType vpType;
};
 

class D3DDeviceCombo
{
public:
    // FUNCTION: SWBF2 0x00715f00 -- out of line, because Steam calls it

    D3DDeviceCombo();
    ~D3DDeviceCombo();

    PblList<D3DDeviceCombo>::Node mNode;
    int AdapterOrdinal;
    D3DDEVTYPE DevType;
    D3DFORMAT AdapterFormat;
    D3DFORMAT BackBufferFormat;
    bool IsWindowed;
    PblList<D3DFormat> DepthStencilFormatList;
    PblList<D3DFormat> TextureFormatList;
    PblList<D3DMultisampleType> MultiSampleTypeList;
    PblList<D3DMultisampleQuality> MultiSampleQualityList;
    PblList<D3DDSMSConflict> DSMSConflictList;
    PblList<D3DVertexProcessingType> VertexProcessingTypeList;
    PblList<D3DPresentInterval> PresentIntervalList;
};
 
 
 
 
 
 
 
 
 
 
 
 
 

class D3DDeviceInfo
{
public:
    ~D3DDeviceInfo();

    PblList<D3DDeviceInfo>::Node mNode;
    int AdapterOrdinal;
    D3DDEVTYPE DevType;
    D3DCAPS9 Caps;
    PblList<D3DDeviceCombo> DeviceComboList;
};
 

 
 
 

class D3DAdapterInfo
{
public:
    ~D3DAdapterInfo();

    PblList<D3DAdapterInfo>::Node mNode;
    int AdapterOrdinal;
    D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
    PblList<D3DDisplayMode> DisplayModeList;
    PblList<D3DDeviceInfo> DeviceInfoList;
};
 
 
 

unsigned int ColorChannelBits(D3DFORMAT fmt);
unsigned int AlphaChannelBits(D3DFORMAT fmt);
unsigned int DepthBits(D3DFORMAT fmt);
unsigned int StencilBits(D3DFORMAT fmt);

namespace pcRedDeviceEnumerator
{

// GLOBAL: SWBF2 0x007e2318
extern unsigned int AppMinFullscreenWidth;
// GLOBAL: SWBF2 0x007e231c
extern unsigned int AppMinFullscreenHeight;
// GLOBAL: SWBF2 0x007e2320
extern unsigned int AppMinColorChannelBits;
// GLOBAL: SWBF2 0x007e2324
extern unsigned int AppMinAlphaChannelBits;
// GLOBAL: SWBF2 0x007e2328
extern unsigned int AppMinDepthBits;
// GLOBAL: SWBF2 0x007e232c
extern unsigned int AppMinStencilBits;
// GLOBAL: SWBF2 0x007e2330
extern bool AppUsesDepthBuffer;

// GLOBAL: SWBF2 0x007e2334
extern PblList<D3DAdapterInfo> m_AdapterInfoList;

// GLOBAL: SWBF2 0x009cab00
extern bool AppUsesMixedVP;
// GLOBAL: SWBF2 0x009cab01
extern bool AppRequiresWindowed;
// GLOBAL: SWBF2 0x009cab02
extern bool AppRequiresFullscreen;
// GLOBAL: SWBF2 0x009cab04
extern bool (*ConfirmDeviceCallback)(D3DCAPS9* pCaps, VertexProcessingType vpType,
                                     D3DFORMAT backBufferFormat);

// GLOBAL: SWBF2 0x009cab08
extern IDirect3D9* m_pD3D;

void BuildDepthStencilFormatList(D3DDeviceCombo* pDeviceCombo);
void BuildMultiSampleTypeList(D3DDeviceCombo* pDeviceCombo);
void BuildDSMSConflictList(D3DDeviceCombo* pDeviceCombo);
void BuildTextureFormatList(D3DDeviceCombo* pDeviceCombo);
void BuildPresentIntervalList(D3DDeviceInfo* pDeviceInfo,
                              D3DDeviceCombo* pDeviceCombo);

void Init(IDirect3D9* pD3D);

void Enumerate(unsigned int uiMinColorChannelBits, unsigned int uiMinAlphaChannelBits,
               unsigned int uiMinDepthBits, unsigned int uiMinStencilBits);

void EnumerateDevices(D3DAdapterInfo* pAdapterInfo,
                      PblList<D3DFormat>* pAdapterFormatList);
void EnumerateDeviceCombos(D3DDeviceInfo* pDeviceInfo,
                           PblList<D3DFormat>* pAdapterFormatList);
void BuildVertexProcessingTypeList(D3DDeviceInfo* pDeviceInfo,
                                   D3DDeviceCombo* pDeviceCombo);

PblList<D3DAdapterInfo>& GetDeviceList();

}

#endif
