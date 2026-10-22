#ifndef SWBF2_GRAPHICS_PC_FAKED3D_H
#define SWBF2_GRAPHICS_PC_FAKED3D_H

#include <stddef.h>
#include <d3d9.h>


// VTABLE: SWBF2 0x00772700
class FakeD3D : public IDirect3D9
{
public:

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(RegisterSoftwareDevice)(void* pInitializeFunction);
    STDMETHOD_(UINT, GetAdapterCount)();
    STDMETHOD(GetAdapterIdentifier)(UINT Adapter, DWORD Flags,
                                    D3DADAPTER_IDENTIFIER9* pIdentifier);
    STDMETHOD_(UINT, GetAdapterModeCount)(UINT Adapter, D3DFORMAT Format);
    STDMETHOD(EnumAdapterModes)(UINT Adapter, D3DFORMAT Format, UINT Mode,
                                D3DDISPLAYMODE* pMode);
    STDMETHOD(GetAdapterDisplayMode)(UINT Adapter, D3DDISPLAYMODE* pMode);
    STDMETHOD(CheckDeviceType)(UINT Adapter, D3DDEVTYPE DevType,
                               D3DFORMAT AdapterFormat,
                               D3DFORMAT BackBufferFormat, BOOL bWindowed);
    STDMETHOD(CheckDeviceFormat)(UINT Adapter, D3DDEVTYPE DeviceType,
                                 D3DFORMAT AdapterFormat, DWORD Usage,
                                 D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
    STDMETHOD(CheckDeviceMultiSampleType)(UINT Adapter, D3DDEVTYPE DeviceType,
                                          D3DFORMAT SurfaceFormat,
                                          BOOL Windowed,
                                          D3DMULTISAMPLE_TYPE MultiSampleType,
                                          DWORD* pQualityLevels);
    STDMETHOD(CheckDepthStencilMatch)(UINT Adapter, D3DDEVTYPE DeviceType,
                                      D3DFORMAT AdapterFormat,
                                      D3DFORMAT RenderTargetFormat,
                                      D3DFORMAT DepthStencilFormat);
    STDMETHOD(CheckDeviceFormatConversion)(UINT Adapter, D3DDEVTYPE DeviceType,
                                           D3DFORMAT SourceFormat,
                                           D3DFORMAT TargetFormat);
    STDMETHOD(GetDeviceCaps)(UINT Adapter, D3DDEVTYPE DeviceType,
                             D3DCAPS9* pCaps);
    STDMETHOD_(HMONITOR, GetAdapterMonitor)(UINT Adapter);
    STDMETHOD(CreateDevice)(UINT Adapter, D3DDEVTYPE DeviceType,
                            HWND hFocusWindow, DWORD BehaviorFlags,
                            D3DPRESENT_PARAMETERS* pPresentationParameters,
                            IDirect3DDevice9** ppReturnedDeviceInterface);
};

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DCUBETEXTURE_H
#define SWBF2_GRAPHICS_PC_FAKED3DCUBETEXTURE_H

#include <d3d9.h>


// VTABLE: SWBF2 0x007723ec
class FakeD3DCubeTexture : public IDirect3DCubeTexture9
{
public:

    FakeD3DCubeTexture(int edgeLength, int levels)
        : refCount(1),
          lockCount(0),
          length(edgeLength),
          levels(levels),
          bits(0)
    {
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void* pData,
                              DWORD SizeOfData, DWORD Flags);
    STDMETHOD(GetPrivateData)(REFGUID refguid, void* pData,
                              DWORD* pSizeOfData);
    STDMETHOD(FreePrivateData)(REFGUID refguid);
    STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
    STDMETHOD_(DWORD, GetPriority)();
    STDMETHOD_(void, PreLoad)();
    STDMETHOD_(D3DRESOURCETYPE, GetType)();

    STDMETHOD_(DWORD, SetLOD)(DWORD LODNew);
    STDMETHOD_(DWORD, GetLOD)();
    STDMETHOD_(DWORD, GetLevelCount)();
    STDMETHOD(SetAutoGenFilterType)(D3DTEXTUREFILTERTYPE FilterType);
    STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)();

    STDMETHOD_(void, GenerateMipSubLevels)();

    STDMETHOD(GetLevelDesc)(UINT Level, D3DSURFACE_DESC* pDesc);
    STDMETHOD(GetCubeMapSurface)(D3DCUBEMAP_FACES FaceType, UINT Level,
                                 IDirect3DSurface9** ppCubeMapSurface);
    STDMETHOD(LockRect)(D3DCUBEMAP_FACES FaceType, UINT Level,
                        D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect,
                        DWORD Flags);
    STDMETHOD(UnlockRect)(D3DCUBEMAP_FACES FaceType, UINT Level);
    STDMETHOD(AddDirtyRect)(D3DCUBEMAP_FACES FaceType,
                            CONST RECT* pDirtyRect);

    int refCount;
    int lockCount;
    int length;
    int levels;
    unsigned char* bits;
};

 
 
 
 
 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DDEVICE_H
#define SWBF2_GRAPHICS_PC_FAKED3DDEVICE_H

#include <d3d9.h>


// VTABLE: SWBF2 0x00772520
class FakeD3DDevice : public IDirect3DDevice9
{
public:

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(TestCooperativeLevel)();
    STDMETHOD_(UINT, GetAvailableTextureMem)();
    STDMETHOD(EvictManagedResources)();
    STDMETHOD(GetDirect3D)(IDirect3D9** ppD3D9);
    STDMETHOD(GetDeviceCaps)(D3DCAPS9* pCaps);
    STDMETHOD(GetDisplayMode)(UINT iSwapChain, D3DDISPLAYMODE* pMode);
    STDMETHOD(GetCreationParameters)(
        D3DDEVICE_CREATION_PARAMETERS* pParameters);
    STDMETHOD(SetCursorProperties)(UINT XHotSpot, UINT YHotSpot,
                                   IDirect3DSurface9* pCursorBitmap);
    STDMETHOD_(void, SetCursorPosition)(int X, int Y, DWORD Flags);
    STDMETHOD_(BOOL, ShowCursor)(BOOL bShow);
    STDMETHOD(CreateAdditionalSwapChain)(
        D3DPRESENT_PARAMETERS* pPresentationParameters,
        IDirect3DSwapChain9** pSwapChain);
    STDMETHOD(GetSwapChain)(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
    STDMETHOD_(UINT, GetNumberOfSwapChains)();
    STDMETHOD(Reset)(D3DPRESENT_PARAMETERS* pPresentationParameters);
    STDMETHOD(Present)(CONST RECT* pSourceRect, CONST RECT* pDestRect,
                       HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
    STDMETHOD(GetBackBuffer)(UINT iSwapChain, UINT iBackBuffer,
                             D3DBACKBUFFER_TYPE Type,
                             IDirect3DSurface9** ppBackBuffer);
    STDMETHOD(GetRasterStatus)(UINT iSwapChain,
                               D3DRASTER_STATUS* pRasterStatus);
    STDMETHOD(SetDialogBoxMode)(BOOL bEnableDialogs);
    STDMETHOD_(void, SetGammaRamp)(UINT iSwapChain, DWORD Flags,
                                   CONST D3DGAMMARAMP* pRamp);
    STDMETHOD_(void, GetGammaRamp)(UINT iSwapChain, D3DGAMMARAMP* pRamp);
    STDMETHOD(CreateTexture)(UINT Width, UINT Height, UINT Levels, DWORD Usage,
                             D3DFORMAT Format, D3DPOOL Pool,
                             IDirect3DTexture9** ppTexture,
                             HANDLE* pSharedHandle);
    STDMETHOD(CreateVolumeTexture)(UINT Width, UINT Height, UINT Depth,
                                   UINT Levels, DWORD Usage, D3DFORMAT Format,
                                   D3DPOOL Pool,
                                   IDirect3DVolumeTexture9** ppVolumeTexture,
                                   HANDLE* pSharedHandle);
    STDMETHOD(CreateCubeTexture)(UINT EdgeLength, UINT Levels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 IDirect3DCubeTexture9** ppCubeTexture,
                                 HANDLE* pSharedHandle);
    STDMETHOD(CreateVertexBuffer)(UINT Length, DWORD Usage, DWORD FVF,
                                  D3DPOOL Pool,
                                  IDirect3DVertexBuffer9** ppVertexBuffer,
                                  HANDLE* pSharedHandle);
    STDMETHOD(CreateIndexBuffer)(UINT Length, DWORD Usage, D3DFORMAT Format,
                                 D3DPOOL Pool,
                                 IDirect3DIndexBuffer9** ppIndexBuffer,
                                 HANDLE* pSharedHandle);
    STDMETHOD(CreateRenderTarget)(UINT Width, UINT Height, D3DFORMAT Format,
                                  D3DMULTISAMPLE_TYPE MultiSample,
                                  DWORD MultisampleQuality, BOOL Lockable,
                                  IDirect3DSurface9** ppSurface,
                                  HANDLE* pSharedHandle);
    STDMETHOD(CreateDepthStencilSurface)(UINT Width, UINT Height,
                                         D3DFORMAT Format,
                                         D3DMULTISAMPLE_TYPE MultiSample,
                                         DWORD MultisampleQuality,
                                         BOOL Discard,
                                         IDirect3DSurface9** ppSurface,
                                         HANDLE* pSharedHandle);
    STDMETHOD(UpdateSurface)(IDirect3DSurface9* pSourceSurface,
                             CONST RECT* pSourceRect,
                             IDirect3DSurface9* pDestinationSurface,
                             CONST POINT* pDestPoint);
    STDMETHOD(UpdateTexture)(IDirect3DBaseTexture9* pSourceTexture,
                             IDirect3DBaseTexture9* pDestinationTexture);
    STDMETHOD(GetRenderTargetData)(IDirect3DSurface9* pRenderTarget,
                                   IDirect3DSurface9* pDestSurface);
    STDMETHOD(GetFrontBufferData)(UINT iSwapChain,
                                  IDirect3DSurface9* pDestSurface);
    STDMETHOD(StretchRect)(IDirect3DSurface9* pSourceSurface,
                           CONST RECT* pSourceRect,
                           IDirect3DSurface9* pDestSurface,
                           CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
    STDMETHOD(ColorFill)(IDirect3DSurface9* pSurface, CONST RECT* pRect,
                         D3DCOLOR color);
    STDMETHOD(CreateOffscreenPlainSurface)(UINT Width, UINT Height,
                                           D3DFORMAT Format, D3DPOOL Pool,
                                           IDirect3DSurface9** ppSurface,
                                           HANDLE* pSharedHandle);
    STDMETHOD(SetRenderTarget)(DWORD RenderTargetIndex,
                               IDirect3DSurface9* pRenderTarget);
    STDMETHOD(GetRenderTarget)(DWORD RenderTargetIndex,
                               IDirect3DSurface9** ppRenderTarget);
    STDMETHOD(SetDepthStencilSurface)(IDirect3DSurface9* pNewZStencil);
    STDMETHOD(GetDepthStencilSurface)(IDirect3DSurface9** ppZStencilSurface);
    STDMETHOD(BeginScene)();
    STDMETHOD(EndScene)();
    STDMETHOD(Clear)(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
                     D3DCOLOR Color, float Z, DWORD Stencil);
    STDMETHOD(SetTransform)(D3DTRANSFORMSTATETYPE State,
                            CONST D3DMATRIX* pMatrix);
    STDMETHOD(GetTransform)(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
    STDMETHOD(MultiplyTransform)(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
    STDMETHOD(SetViewport)(CONST D3DVIEWPORT9* pViewport);
    STDMETHOD(GetViewport)(D3DVIEWPORT9* pViewport);
    STDMETHOD(SetMaterial)(CONST D3DMATERIAL9* pMaterial);
    STDMETHOD(GetMaterial)(D3DMATERIAL9* pMaterial);
    STDMETHOD(SetLight)(DWORD Index, CONST D3DLIGHT9*);
    STDMETHOD(GetLight)(DWORD Index, D3DLIGHT9*);
    STDMETHOD(LightEnable)(DWORD Index, BOOL Enable);
    STDMETHOD(GetLightEnable)(DWORD Index, BOOL* pEnable);
    STDMETHOD(SetClipPlane)(DWORD Index, CONST float* pPlane);
    STDMETHOD(GetClipPlane)(DWORD Index, float* pPlane);
    STDMETHOD(SetRenderState)(D3DRENDERSTATETYPE State, DWORD Value);
    STDMETHOD(GetRenderState)(D3DRENDERSTATETYPE State, DWORD* pValue);
    STDMETHOD(CreateStateBlock)(D3DSTATEBLOCKTYPE Type,
                                IDirect3DStateBlock9** ppSB);
    STDMETHOD(BeginStateBlock)();
    STDMETHOD(EndStateBlock)(IDirect3DStateBlock9** ppSB);
    STDMETHOD(SetClipStatus)(CONST D3DCLIPSTATUS9* pClipStatus);
    STDMETHOD(GetClipStatus)(D3DCLIPSTATUS9* pClipStatus);
    STDMETHOD(GetTexture)(DWORD Stage, IDirect3DBaseTexture9** ppTexture);
    STDMETHOD(SetTexture)(DWORD Stage, IDirect3DBaseTexture9* pTexture);
    STDMETHOD(GetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                                    DWORD* pValue);
    STDMETHOD(SetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                                    DWORD Value);
    STDMETHOD(GetSamplerState)(DWORD Sampler, D3DSAMPLERSTATETYPE Type,
                               DWORD* pValue);
    STDMETHOD(SetSamplerState)(DWORD Sampler, D3DSAMPLERSTATETYPE Type,
                               DWORD Value);
    STDMETHOD(ValidateDevice)(DWORD* pNumPasses);
    STDMETHOD(SetPaletteEntries)(UINT PaletteNumber,
                                 CONST PALETTEENTRY* pEntries);
    STDMETHOD(GetPaletteEntries)(UINT PaletteNumber, PALETTEENTRY* pEntries);
    STDMETHOD(SetCurrentTexturePalette)(UINT PaletteNumber);
    STDMETHOD(GetCurrentTexturePalette)(UINT* PaletteNumber);
    STDMETHOD(SetScissorRect)(CONST RECT* pRect);
    STDMETHOD(GetScissorRect)(RECT* pRect);
    STDMETHOD(SetSoftwareVertexProcessing)(BOOL bSoftware);
    STDMETHOD_(BOOL, GetSoftwareVertexProcessing)();
    STDMETHOD(SetNPatchMode)(float nSegments);
    STDMETHOD_(float, GetNPatchMode)();
    STDMETHOD(DrawPrimitive)(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
                             UINT PrimitiveCount);
    STDMETHOD(DrawIndexedPrimitive)(D3DPRIMITIVETYPE, INT BaseVertexIndex,
                                    UINT MinVertexIndex, UINT NumVertices,
                                    UINT startIndex, UINT primCount);
    STDMETHOD(DrawPrimitiveUP)(D3DPRIMITIVETYPE PrimitiveType,
                               UINT PrimitiveCount,
                               CONST void* pVertexStreamZeroData,
                               UINT VertexStreamZeroStride);
    STDMETHOD(DrawIndexedPrimitiveUP)(D3DPRIMITIVETYPE PrimitiveType,
                                      UINT MinVertexIndex, UINT NumVertices,
                                      UINT PrimitiveCount,
                                      CONST void* pIndexData,
                                      D3DFORMAT IndexDataFormat,
                                      CONST void* pVertexStreamZeroData,
                                      UINT VertexStreamZeroStride);
    STDMETHOD(ProcessVertices)(UINT SrcStartIndex, UINT DestIndex,
                               UINT VertexCount,
                               IDirect3DVertexBuffer9* pDestBuffer,
                               IDirect3DVertexDeclaration9* pVertexDecl,
                               DWORD Flags);
    STDMETHOD(CreateVertexDeclaration)(
        CONST D3DVERTEXELEMENT9* pVertexElements,
        IDirect3DVertexDeclaration9** ppDecl);
    STDMETHOD(SetVertexDeclaration)(IDirect3DVertexDeclaration9* pDecl);
    STDMETHOD(GetVertexDeclaration)(IDirect3DVertexDeclaration9** ppDecl);
    STDMETHOD(SetFVF)(DWORD FVF);
    STDMETHOD(GetFVF)(DWORD* pFVF);
    STDMETHOD(CreateVertexShader)(CONST DWORD* pFunction,
                                  IDirect3DVertexShader9** ppShader);
    STDMETHOD(SetVertexShader)(IDirect3DVertexShader9* pShader);
    STDMETHOD(GetVertexShader)(IDirect3DVertexShader9** ppShader);
    STDMETHOD(SetVertexShaderConstantF)(UINT StartRegister,
                                        CONST float* pConstantData,
                                        UINT Vector4fCount);
    STDMETHOD(GetVertexShaderConstantF)(UINT StartRegister,
                                        float* pConstantData,
                                        UINT Vector4fCount);
    STDMETHOD(SetVertexShaderConstantI)(UINT StartRegister,
                                        CONST int* pConstantData,
                                        UINT Vector4iCount);
    STDMETHOD(GetVertexShaderConstantI)(UINT StartRegister, int* pConstantData,
                                        UINT Vector4iCount);
    STDMETHOD(SetVertexShaderConstantB)(UINT StartRegister,
                                        CONST BOOL* pConstantData,
                                        UINT BoolCount);
    STDMETHOD(GetVertexShaderConstantB)(UINT StartRegister,
                                        BOOL* pConstantData, UINT BoolCount);
    STDMETHOD(SetStreamSource)(UINT StreamNumber,
                               IDirect3DVertexBuffer9* pStreamData,
                               UINT OffsetInBytes, UINT Stride);
    STDMETHOD(GetStreamSource)(UINT StreamNumber,
                               IDirect3DVertexBuffer9** ppStreamData,
                               UINT* pOffsetInBytes, UINT* pStride);
    STDMETHOD(SetStreamSourceFreq)(UINT StreamNumber, UINT Setting);
    STDMETHOD(GetStreamSourceFreq)(UINT StreamNumber, UINT* pSetting);
    STDMETHOD(SetIndices)(IDirect3DIndexBuffer9* pIndexData);
    STDMETHOD(GetIndices)(IDirect3DIndexBuffer9** ppIndexData);
    STDMETHOD(CreatePixelShader)(CONST DWORD* pFunction,
                                 IDirect3DPixelShader9** ppShader);
    STDMETHOD(SetPixelShader)(IDirect3DPixelShader9* pShader);
    STDMETHOD(GetPixelShader)(IDirect3DPixelShader9** ppShader);
    STDMETHOD(SetPixelShaderConstantF)(UINT StartRegister,
                                       CONST float* pConstantData,
                                       UINT Vector4fCount);
    STDMETHOD(GetPixelShaderConstantF)(UINT StartRegister,
                                       float* pConstantData,
                                       UINT Vector4fCount);
    STDMETHOD(SetPixelShaderConstantI)(UINT StartRegister,
                                       CONST int* pConstantData,
                                       UINT Vector4iCount);
    STDMETHOD(GetPixelShaderConstantI)(UINT StartRegister, int* pConstantData,
                                       UINT Vector4iCount);
    STDMETHOD(SetPixelShaderConstantB)(UINT StartRegister,
                                       CONST BOOL* pConstantData,
                                       UINT BoolCount);
    STDMETHOD(GetPixelShaderConstantB)(UINT StartRegister, BOOL* pConstantData,
                                       UINT BoolCount);
    STDMETHOD(DrawRectPatch)(UINT Handle, CONST float* pNumSegs,
                             CONST D3DRECTPATCH_INFO* pRectPatchInfo);
    STDMETHOD(DrawTriPatch)(UINT Handle, CONST float* pNumSegs,
                            CONST D3DTRIPATCH_INFO* pTriPatchInfo);
    STDMETHOD(DeletePatch)(UINT Handle);
    STDMETHOD(CreateQuery)(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);

    int refCount;
};

 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DINDEXBUFFER_H
#define SWBF2_GRAPHICS_PC_FAKED3DINDEXBUFFER_H

#include <d3d9.h>


// VTABLE: SWBF2 0x007724cc
class FakeD3DIndexBuffer : public IDirect3DIndexBuffer9
{
public:

    FakeD3DIndexBuffer(unsigned int length)
        : refCount(1),
          lockCount(0),
          len(length),
          bits(0)
    {
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void* pData,
                              DWORD SizeOfData, DWORD Flags);
    STDMETHOD(GetPrivateData)(REFGUID refguid, void* pData,
                              DWORD* pSizeOfData);
    STDMETHOD(FreePrivateData)(REFGUID refguid);
    STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
    STDMETHOD_(DWORD, GetPriority)();
    STDMETHOD_(void, PreLoad)();
    STDMETHOD_(D3DRESOURCETYPE, GetType)();

    STDMETHOD(Lock)(UINT OffsetToLock, UINT SizeToLock, void** ppbData,
                    DWORD Flags);
    STDMETHOD(Unlock)();
    STDMETHOD(GetDesc)(D3DINDEXBUFFER_DESC* pDesc);

    int refCount;
    int lockCount;
    unsigned int len;
    unsigned char* bits;
};

 
 
 
 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DSURFACE_H
#define SWBF2_GRAPHICS_PC_FAKED3DSURFACE_H

#include <d3d9.h>


// VTABLE: SWBF2 0x00772448
class FakeD3DSurface : public IDirect3DSurface9
{
public:

    FakeD3DSurface() : refCount(1) {}

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void* pData,
                              DWORD SizeOfData, DWORD Flags);
    STDMETHOD(GetPrivateData)(REFGUID refguid, void* pData,
                              DWORD* pSizeOfData);
    STDMETHOD(FreePrivateData)(REFGUID refguid);
    STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
    STDMETHOD_(DWORD, GetPriority)();
    STDMETHOD_(void, PreLoad)();
    STDMETHOD_(D3DRESOURCETYPE, GetType)();

    STDMETHOD(GetContainer)(REFIID riid, void** ppContainer);
    STDMETHOD(GetDesc)(D3DSURFACE_DESC* pDesc);
    STDMETHOD(LockRect)(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect,
                        DWORD Flags);
    STDMETHOD(UnlockRect)();
    STDMETHOD(GetDC)(HDC* phdc);
    STDMETHOD(ReleaseDC)(HDC hdc);

    int refCount;
};

 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DTEXTURE_H
#define SWBF2_GRAPHICS_PC_FAKED3DTEXTURE_H

#include <d3d9.h>


// VTABLE: SWBF2 0x00772334
class FakeD3DTexture : public IDirect3DTexture9
{
public:

    FakeD3DTexture(unsigned int width, unsigned int height,
                   unsigned int format)
        : refCount(1),
          lockCount(0),
          w(width),
          h(height),
          fmt(format),
          bits(0)
    {
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void* pData,
                              DWORD SizeOfData, DWORD Flags);
    STDMETHOD(GetPrivateData)(REFGUID refguid, void* pData,
                              DWORD* pSizeOfData);
    STDMETHOD(FreePrivateData)(REFGUID refguid);
    STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
    STDMETHOD_(DWORD, GetPriority)();
    STDMETHOD_(void, PreLoad)();
    STDMETHOD_(D3DRESOURCETYPE, GetType)();

    STDMETHOD_(DWORD, SetLOD)(DWORD LODNew);
    STDMETHOD_(DWORD, GetLOD)();
    STDMETHOD_(DWORD, GetLevelCount)();
    STDMETHOD(SetAutoGenFilterType)(D3DTEXTUREFILTERTYPE FilterType);
    STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)();

    STDMETHOD_(void, GenerateMipSubLevels)();

    STDMETHOD(GetLevelDesc)(UINT Level, D3DSURFACE_DESC* pDesc);
    STDMETHOD(GetSurfaceLevel)(UINT Level,
                               IDirect3DSurface9** ppSurfaceLevel);
    STDMETHOD(LockRect)(UINT Level, D3DLOCKED_RECT* pLockedRect,
                        CONST RECT* pRect, DWORD Flags);
    STDMETHOD(UnlockRect)(UINT Level);
    STDMETHOD(AddDirtyRect)(CONST RECT* pDirtyRect);

    int refCount;
    int lockCount;
    unsigned int w;
    unsigned int h;
    unsigned int fmt;
    unsigned char* bits;
};

 
 
 
 
 
 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DVERTEXBUFFER_H
#define SWBF2_GRAPHICS_PC_FAKED3DVERTEXBUFFER_H

#include <d3d9.h>


// VTABLE: SWBF2 0x00772490
class FakeD3DVertexBuffer : public IDirect3DVertexBuffer9
{
public:

    FakeD3DVertexBuffer(unsigned int length)
        : refCount(1),
          lockCount(0),
          len(length),
          bits(0)
    {
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void* pData,
                              DWORD SizeOfData, DWORD Flags);
    STDMETHOD(GetPrivateData)(REFGUID refguid, void* pData,
                              DWORD* pSizeOfData);
    STDMETHOD(FreePrivateData)(REFGUID refguid);
    STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
    STDMETHOD_(DWORD, GetPriority)();
    STDMETHOD_(void, PreLoad)();
    STDMETHOD_(D3DRESOURCETYPE, GetType)();

    STDMETHOD(Lock)(UINT OffsetToLock, UINT SizeToLock, void** ppbData,
                    DWORD Flags);
    STDMETHOD(Unlock)();
    STDMETHOD(GetDesc)(D3DVERTEXBUFFER_DESC* pDesc);

    int refCount;
    int lockCount;
    unsigned int len;
    unsigned char* bits;
};

 
 
 
 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DVERTEXDECLARATION_H
#define SWBF2_GRAPHICS_PC_FAKED3DVERTEXDECLARATION_H

#include <d3d9.h>


// VTABLE: SWBF2 0x00772508
class FakeD3DVertexDeclaration : public IDirect3DVertexDeclaration9
{
public:

    FakeD3DVertexDeclaration() : refCount(1) {}

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(GetDeclaration)(D3DVERTEXELEMENT9* pElement, UINT* pNumElements);

    int refCount;
};

 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_FAKED3DVOLUMETEXTURE_H
#define SWBF2_GRAPHICS_PC_FAKED3DVOLUMETEXTURE_H

#include <d3d9.h>


// VTABLE: SWBF2 0x00772390
class FakeD3DVolumeTexture : public IDirect3DVolumeTexture9
{
public:

    FakeD3DVolumeTexture(int width, int height, int depth, int levels,
                         unsigned int format)
        : refCount(1),
          lockCount(0),
          width(width),
          height(height),
          depth(depth),
          levels(levels),
          format(format)
    {
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(GetDevice)(IDirect3DDevice9** ppDevice);
    STDMETHOD(SetPrivateData)(REFGUID refguid, CONST void* pData,
                              DWORD SizeOfData, DWORD Flags);
    STDMETHOD(GetPrivateData)(REFGUID refguid, void* pData,
                              DWORD* pSizeOfData);
    STDMETHOD(FreePrivateData)(REFGUID refguid);
    STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew);
    STDMETHOD_(DWORD, GetPriority)();
    STDMETHOD_(void, PreLoad)();
    STDMETHOD_(D3DRESOURCETYPE, GetType)();

    STDMETHOD_(DWORD, SetLOD)(DWORD LODNew);
    STDMETHOD_(DWORD, GetLOD)();
    STDMETHOD_(DWORD, GetLevelCount)();
    STDMETHOD(SetAutoGenFilterType)(D3DTEXTUREFILTERTYPE FilterType);
    STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)();

    STDMETHOD_(void, GenerateMipSubLevels)();

    STDMETHOD(GetLevelDesc)(UINT Level, D3DVOLUME_DESC* pDesc);
    STDMETHOD(GetVolumeLevel)(UINT Level, IDirect3DVolume9** ppVolumeLevel);
    STDMETHOD(LockBox)(UINT Level, D3DLOCKED_BOX* pLockedVolume,
                       CONST D3DBOX* pBox, DWORD Flags);
    STDMETHOD(UnlockBox)(UINT Level);
    STDMETHOD(AddDirtyBox)(CONST D3DBOX* pDirtyBox);

    int refCount;
    int lockCount;
    unsigned char* bits;
    int width;
    int height;
    int depth;
    int levels;
    unsigned int format;
};

 
 
 
 
 
 
 
 
 

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_PCREDRENDERER_H
#define SWBF2_GRAPHICS_PC_PCREDRENDERER_H

#include "pcRedAutoSettings.h"

#include "StateManager_DX.h"

#include "RedEngineFL/Util/ext/heap.h"

#include "RedEngineFL/Graphics/RedColor.h"

struct RenderItem;

union SortIdT
{
    unsigned int id;
    struct
    {
        unsigned int t_tnl : 8;
        unsigned int t_customID : 8;
        unsigned int t_pass : 2;
        unsigned int t_zval : 13;
        unsigned int transparent : 1;
    };
    struct
    {
        unsigned int t_shaderID : 16;
        unsigned int t_sortval : 16;
    };
    struct
    {
        unsigned int o_zval : 13;
        unsigned int o_tnl : 8;
        unsigned int o_customID : 8;
        unsigned int o_pass : 2;
        unsigned int zz_reserved1 : 1;
    };
    struct
    {
        unsigned int zz_reserved2 : 13;
        unsigned int o_shaderID : 16;
    };
};
 

class RedTexture;

class PblTimeBase;

class pcRedPrimitive;
class pcRedShader;
class RedShadingState;

enum pcBufferT
{
    PC_BUFFER_TEMP = 0,
    PC_BUFFER_SKY = 1,
    PC_BUFFER_REFRACTION = 2,
    PC_MAX_BUFFERS = 3,
    PC_BUFFER_FORCE_DWORD = 0x7fffffff
};

enum ResetFlagsT
{
    RESETFLAGS_NONE = 0,
    RESETFLAGS_FREE_RESOURCES = 1,
    RESETFLAGS_CREATE_RESOURCES = 2,
    RESETFLAGS_FORCE_DWORD = 2147483647
};

template <class ITEM, class KEY>
class TransparentHeap
{
public:
    struct Node
    {
        ITEM Item;
        KEY idSort;
        Node* pNext;
    };

    TransparentHeap(unsigned int uiMaxSize);
    ~TransparentHeap();

    void Push(ITEM& rItem, KEY idSort);

    bool IsEmpty() const { return m_pFirstNode == 0; }

    KEY Value() const { return m_pFirstNode->idSort; }
    ITEM& Get() const { return m_pFirstNode->Item; }
    void Pop()
    {
        Node* pNode = m_pFirstNode;
        if (pNode)
        {
            m_pFirstNode = pNode->pNext;
            FreeNode(pNode);
            --m_iCount;
        }
    }

    void Flush()
    {
        Node* pNode = m_pFirstNode;
        if (pNode)
        {
            Node* pFree = m_pFirstFreeNode;
            do
            {
                m_pFirstNode = pNode->pNext;
                pNode->pNext = pFree;
                pFree = pNode;
                pNode = m_pFirstNode;
                m_pFirstFreeNode = pFree;
            } while (pNode);
        }
        m_iCount = 0;
    }

    int globalPass_;
    Node* m_aNodePool;
    Node* m_pFirstFreeNode;
    Node* m_pFirstNode;
    int m_iCount;
    int m_iMaxSize;

private:
    void InitNodePool(unsigned int uiMaxSize);

    Node* AllocNode()
    {
        Node* pNode = m_pFirstFreeNode;
        if (pNode)
        {
            m_pFirstFreeNode = pNode->pNext;
            pNode->pNext = 0;
        }
        return pNode;
    }

    void FreeNode(Node* pNode)
    {
        pNode->pNext = m_pFirstFreeNode;
        m_pFirstFreeNode = pNode;
    }
};

 
 

class RedViewport
{
public:
    float fOffsetX;
    float fOffsetY;
    float fWidth;
    float fHeight;
};
 
 
 
 
 

class PblMatrix;
class PblVector3;
class PblVector4;
class RedCamera;

struct pcRedVertexShaderConstantRegister
{

    void SetMatrix4x4(const PblMatrix& rMatrix);

    void SetMatrix4x3(const PblMatrix& rMatrix);

    void SetFloats(float fX, float fY, float fZ)
    {
        component[0] = fX;
        component[1] = fY;
        component[2] = fZ;
    }

    float component[4];
};
 

class pcRedVertexShaderConstants
{
public:

    pcRedVertexShaderConstantRegister* GetRegisterArray(int iFirst, int iCount)
    {
        if (m_iMinConstantSet > iFirst)
        {
            m_iMinConstantSet = iFirst;
        }
        if (m_iMaxConstantSet < iFirst + iCount - 1)
        {
            m_iMaxConstantSet = iFirst + iCount - 1;
        }
        return m_Registers;
    }

    void Update();

    pcRedVertexShaderConstantRegister m_Registers[96];
    int m_iMinConstantSet;
    int m_iMaxConstantSet;
};
 
 

enum SystemShaderT
{
    SYSTEM_SHADER_DEBUG = 0,
    SYSTEM_SHADER_ZPREPASS = 1,
    SYSTEM_SHADER_PERVERTEXDIFFUSELIGHTING = 2,
    SYSTEM_SHADER_PERPIXELDIFFUSELIGHTING = 3,
    SYSTEM_SHADER_SPECULARLIGHTING = 4,
    SYSTEM_SHADER_NORMAL = 5,
    MAX_SYSTEM_SHADER = 6
};

enum OcclusionQueryShapeT
{
    OCCLUSION_QUERY_SPHERE = 0,
    OCCLUSION_QUERY_CYLINDER = 1,
    OCCLUSION_QUERY_BOX = 2,
    OCCLUSION_QUERY_CONE = 3,
    OCCLUSION_QUERY_NUM_SHAPES = 4
};

enum pcRedCullT
{
    PCREDCULL_NONE = 0,
    PCREDCULL_FRONT = 1,
    PCREDCULL_BACK = 2,
    PCREDCULL_FORCE_DWORD = 3
};

enum pcRedFrontT
{
    PCREDFRONT_CW = 0,
    PCREDFRONT_CCW = 1,
    PCREDFRONT_FORCE_DWORD = 2
};

class PblFileChunk;

class RedCamera;
class RedViewport;
struct RedLightingState;
class PblMatrix;

class RedRenderer
{
public:

    static void Read(PblFileChunk& chunk);

    static void SetSky(bool bEnable);

    static void SetStencilShadowLength(float fLength);
    static void SetStencilShadowZFail(bool bZFail);

    static int GetFrameNumber();

    static const RedCamera* GetCamera();
    static bool GetOcclusionQueryPixelCount(unsigned int uiQuery,
                                            unsigned int& uiPixelCount);

    static void InitStatic();

    static void CleanupStatic();

    static void Init();

    static void OpenDynamic();
    static void internalBeginConstants();
    static void internalEndConstants();
    static void internalInitOcclusionQuery();
    static void pcEnableScreenSpaceShadow(bool bEnable);
    static void pcSetImmediateMode(int iValue);

    static void SetFogEnable(bool bEnable);

    static bool GetFogEnable();

    static bool GetLightingEnable();
    static const struct RedLightingState& GetDefaultLightingState();
    static struct RedLightingState* AllocLightingStateInCache();

    static void SetDefaultLightingState(const struct RedLightingState& state);

    static void pcSetUserNearClipPlaneEnable(bool bEnable);

    static void pcSetUserNearClipPlane(const class PblPlane& plane);

    static const PblMatrix* AllocMatrixInCache(const PblMatrix& mat);
    static PblMatrix* AllocBoneMatricesInCache(unsigned int uiCount);
    static void SetLightingState(const struct RedLightingState& state);
    static float GetStencilShadowLength();

    static void SetLightingScale(float fScale);

    static float GetLightingScale();

    static void SetShadowIntensity(float fIntensity);

    static void BeginFrame();
    static void BeginScene();
    static void Cleanup();
    static IDirect3D9* CreateFakeD3D();
    static IDirect3DDevice9* CreateFakeDevice();
    static void EndFrame();
    static void EndScene();
    static void Flush();
    static void FlushOcclusionQueries();
    static RedColor GetFogColor();
    static const PblMatrix& GetProjectionMatrix();
    static float GetShadowIntensity();
    static void GetWorldFogRange(float& fStart, float& fEnd);
    static void PostLoad();
    static void SetAmbientFilter(bool bEnable);
    static void SetAmbientIntensity(int iIntensity);
    static void SetAmbientThreshold(int iThreshold);
    static void SetColorFilter(bool bEnable);
    static void SetFilterColor(RedColor color, int iAmount);
    static void SetFogColor(const RedColor& color);
    static void SetFogRange(float fStart, float fEnd);
    static void SetMultiplicativeFog(int iPower);
    static void SetScreenOffset(float fX, float fY);
    static void SetSoftnessEnable(bool bEnable);
    static void SetSoftnessValue(int iValue);
    static void SetWorldFogRange(float fStart, float fEnd);
    static unsigned int SubmitOcclusionQuery(const PblMatrix& mWorld,
                                             enum OcclusionQueryShapeT eShape,
                                             const PblVector3& vScale, bool);
    static const PblVector3 TransformCameraPointToProjectionSpace(const PblVector3& vPoint);
    static const PblVector3 TransformProjectionPointToScreenSpace(const PblVector3& vPoint);

    static const PblVector3 TransformCameraPointToScreenSpace(const PblVector3& vPoint);
    static const PblVector3 TransformWorldPointToProjectionSpace(const PblVector3& vPoint);

    static void internalCleanupOcclusionQuery();
    static IDirect3DQuery9* internalGetD3DVisibilityQuery();
    static unsigned int internalGetTransformAndLightingID(unsigned int uiRVF,
                                                          unsigned int uiEnvState,
                                                          unsigned int uiPassCaps);
    static void internalRenderZPrepassProg1(RenderItem* pItem, unsigned int uiShaderID);
    static void internalReset();

    static void pcFlush(unsigned int uiPhases);
    static void internalShaderBeginEnd();
    static void internalStateStackPop();

    static void __cdecl pcRenderPrimitive(pcRedPrimitive* pPrim, pcRedShader* pShader,
                                          const PblMatrix& mWorld, const RedColor& tweakColor,
                                          unsigned int uiFlags, const PblMatrix* pBoneMatrices,
                                          unsigned int uiNumBoneMatrices,
                                          RedShadingState* pShadingState);

    static void internalRenderImmediate(RenderItem* pItem);
    static void internalShaderBeginEnd(RenderItem* pItem, unsigned int uiShaderID,
                                       unsigned int uiPass);
    static void internalStateStackPush();

    template <class HEAP>
    static void internalRenderFromHeap(HEAP& heap);

    // GLOBAL: SWBF2 0x0086ad10 -- pdb_globals.tsv `RedLightingState[2051]`.

    static struct RedLightingState m_aLightingStatePool[2051];

    static struct RenderItem m_renderItemPool[4748];

    // GLOBAL: SWBF2 0x008bef50 -- pdb_globals.tsv `PblMatrix[3062]` (22, TU 42;

    static class PblMatrix m_amMatrixPool[3062];

    // GLOBAL: SWBF2 0x007fbf60 -- pdb_globals.tsv `OcclusionQueryT[1400]`,

    static struct OcclusionQueryT m_occlusionQueries[1400];

    // GLOBAL: SWBF2 0x007de2a4 -- .data, 1.0f. SetLightingScale's first store.
    static float m_lightingScale;
    // GLOBAL: SWBF2 0x0086ac70 -- SetLightingScale's grey colour.
    static RedColor m_lightingScaleColor;

    static float GetScreenAspectRatio();

    static void pcRegisterResetFunction(void(__cdecl* pFunction)(ResetFlagsT));

    static void pcUnregisterResetFunction(void(__cdecl* pFunction)(ResetFlagsT));

    static bool pcCheckDepthStencilMatch(_D3DFORMAT renderTargetFormat,
                                         _D3DFORMAT depthStencilFormat);
    static void pcSetClearColor(const RedColor& color);

    static RedColor pcGetClearColor();

    static void SetCursorImage(RedTexture const* pTexture, unsigned int uiX,
                               unsigned int uiY);

    static void pcSetCursorImage(RedTexture const* pTexture, unsigned int uiX,
                                 unsigned int uiY);

    static const D3DADAPTER_IDENTIFIER9& pcGetAdaptorIdentifier();

    static IDirect3DDevice9* pcGetDevice();

    static const D3DCAPS9& pcGetDeviceCaps();

    static const RenderCapsT& pcGetRenderCaps();

    static GeometryModeT pcGetGeometryMode();

    static const RenderOptionsT& pcGetRenderOptions();

    static bool pcIsNearScene();

    static float pcGetNearSceneStart();
    static float pcGetNearSceneEnd();
    static void pcSetNearScene(bool bNearScene);
    static void pcSetNearSceneStart(float fStart);
    static void pcSetNearSceneEnd(float fEnd);
    static pcRedShader* pcGetSystemShader(SystemShaderT eShader);

    static void pcLoadVertexShader(const class pcRedVertexShader* pShader);
    static void pcLoadPixelShader(const class pcRedPixelShader* pShader);

    // FUNCTION: SWBF2 0x006b1970
    static PblTimeBase* GetTimeBase();

    static unsigned int pcGetValidDisplayModes(D3DDISPLAYMODE* pModes,
                                               unsigned int uiMaxModes,
                                               unsigned int uiMinWidth,
                                               unsigned int uiMinHeight);

    static void pcSetDisplayMode(unsigned int uiWidth, unsigned int uiHeight,
                                 bool bWindowed, bool bVSync,
                                 D3DMULTISAMPLE_TYPE msType, unsigned long ulMSQuality);

    static void pcSetRenderOptions(const RenderOptionsT& rOptions);

    static void SetTimeBase(PblTimeBase* pTimeBase);

    static void internalSetupCaps();

    static void __cdecl internalRenderProg1(RenderItem* pItem, unsigned int uiFlags,
                                            unsigned int uiPass);
    static void __cdecl internalRenderFixed(RenderItem* pItem, unsigned int uiFlags,
                                            unsigned int uiPass);

    // GLOBAL: SWBF2 0x007fbea8
    static void(__cdecl* internalRenderFunc)(RenderItem*, unsigned int, unsigned int);

    static TransparentHeap<RenderItem*, unsigned int> m_preShadowTransparentItems;
    static TransparentHeap<RenderItem*, unsigned int> m_transparentItems;
    static TransparentHeap<RenderItem*, unsigned int> m_posttransparentItems;

    // GLOBAL: SWBF2 0x007f5948
    static IDirect3D9* m_pD3D;
    // GLOBAL: SWBF2 0x007f594c
    static IDirect3DDevice9* m_pd3dDevice;
    // GLOBAL: SWBF2 0x007f5ecc -- 56 B, and the field/offset pairing is proved

    static D3DPRESENT_PARAMETERS m_d3dpp;

    // GLOBAL: SWBF2 0x007de300 -- the one INITIALISED datum of the seven, and the

    static RenderOptionsT m_renderOptions;

    // GLOBAL: SWBF2 0x007f5f28 -- 400 B, `void ()*[100]` in pdb_globals.tsv, and

    static void(__cdecl* m_resetFunctions[100])(ResetFlagsT);
    // GLOBAL: SWBF2 0x007f60b8
    static unsigned int m_numResetFunctions;
    // GLOBAL: SWBF2 0x007f5a80 -- 1100 B, an exact size match with

    static D3DADAPTER_IDENTIFIER9 m_adaptorIdentifier;

    // GLOBAL: SWBF2 0x007f5950 -- 304 B, D3DCAPS9, an exact size match with

    static D3DCAPS9 m_deviceCaps;
    // GLOBAL: SWBF2 0x007f5f08 -- 32 B, RenderCapsT. Located by the getter's

    static RenderCapsT m_renderCaps;

    // GLOBAL: SWBF2 0x007f5934 -- the ONE store 0x006b3ea0 makes, and its only

    static PblTimeBase* m_pTimeBase;

    class SortHeap : public util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >
    {
    public:
        SortHeap() : lastkey_(0) {}

        SortHeap(SortHeap const& rOther)
            : util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >(rOther),
              lastkey_(rOther.lastkey_), globalPass_(rOther.globalPass_)
        {
        }

        void Push(RenderItem* pItem, unsigned int uiKey);

        unsigned int Value() { return _numElements > 0 ? _elements[0].key_ : 0; }
        RenderItem* Get() { return _numElements > 0 ? _elements[0].elem_ : 0; }
        void Pop()
        {
            util::HeapPop_<util::TKeyPair_<RenderItem*, unsigned int>,
                           util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >,
                           util::GreaterEqOp<util::TKeyPair_<RenderItem*, unsigned int> > >(*this, 0);
        }

        unsigned int lastkey_;
        int globalPass_;
    };

    // GLOBAL: SWBF2 0x008174e0
    static util::Vector<SortHeap> m_opaqueQueues;

    static void internalResetState();

    static void internalStateStackInit();

    static void pcSetRenderState(D3DRENDERSTATETYPE eState, unsigned long ulValue);

    enum FOGFORBLEND_MODE
    {
        FOGFORBLEND_NORMAL = 0,
        FOGFORBLEND_ADD = 1,
        FOGFORBLEND_MODULATE = 2,
        FOGFORBLEND_MODULATE2X = 3
    };

    static void pcSetFogForBlendMode(FOGFORBLEND_MODE eMode);

    static void pcSetCullMode(pcRedCullT eCull);

    static void pcLoadFormat(const class pcRedVertexFormat* pFormat);

    static void pcSetStreamSource(unsigned int uiStream, const class pcRedVertexBuffer* pVertexBuffer);
    static void pcSetIndices(const class pcRedIndexBuffer* pIndexBuffer);

    static void pcSetSamplerState(unsigned int uiSampler, D3DSAMPLERSTATETYPE eType,
                                  unsigned int uiValue);

    static void pcSetTexture(unsigned int uiStage, const class RedTexture* pTexture);

    static void pcSetTextureStageState(unsigned int uiStage, D3DTEXTURESTAGESTATETYPE eType,
                                       unsigned long ulValue);

    static void pcFFSetTextureTransform(unsigned int uiStage, const class PblMatrix& rMatrix);

    static const class PblMatrix& pcGetNearSceneFadeTextureTransform();

    static void pcSetTransform(D3DTRANSFORMSTATETYPE eState, const class PblMatrix& rMatrix);

    static void pcGetViewportExtents(float& fMinX, float& fMinY, float& fMaxX, float& fMaxY);

    static void pcClear(unsigned int uiFlags, RedColor color, float fZ, unsigned int uiStencil);

    static const D3DPRESENT_PARAMETERS& pcGetPresentParameters();
    static RedTexture* pcGetRenderTarget();
    static IDirect3DSurface9* pcGetDepthTarget();
    static void pcGetViewport(RedViewport& rViewport);
    static void pcSetRenderTarget(RedTexture* pTarget, IDirect3DSurface9* pDepth);

    static RedTexture* pcGetBuffer(pcBufferT eBuffer);

    static bool pcGetUserNearClipPlaneEnable();

    static void pcSetFrontFace(pcRedFrontT eFront);

    static void pcSetViewport(const RedViewport& rViewport);

    static pcRedVertexShaderConstantRegister* pcLockVertexShaderConstants(int iFirst,
                                                                          int iCount);
    static void pcSetShadowMapTransform(const PblVector4& u, const PblVector4& v,
                                        const PblVector4& w);

    static void pcUnlockVertexShaderConstants(pcRedVertexShaderConstantRegister* pRegs);

    static void SetCamera(const RedCamera* pCamera);

    static void internalUpdateSceneConstants();

    static void GetFogRange(float& fStart, float& fEnd);

    // GLOBAL: SWBF2 0x007de348 -- 1,544 B, .data, and the one INITIALISED

    static pcRedVertexShaderConstants m_VSConstants;
    // GLOBAL: SWBF2 0x007f58f8 -- 24 B. The six stores internalResetState makes

    static D3DVIEWPORT9 m_viewport;
    // GLOBAL: SWBF2 0x007f5928 -- 0x4B7FFFFF = 16777215.0f = 2^24 - 1, which is

    static float m_fMaxZDepth;
    // GLOBAL: SWBF2 0x007f5930 -- internalResetState's only writer, and the

    static float m_screenAspectRatio;

    static float m_fNearSceneStart;
    static float m_fNearSceneEnd;
    static bool m_bIsNearScene;
    // GLOBAL: SWBF2 0x007fa02c -- IDirect3DDevice9::GetBackBuffer's out slot

    static IDirect3DSurface9* m_pBackBufferSurface;
    // GLOBAL: SWBF2 0x007fa030 -- GetDepthStencilSurface's out slot

    static IDirect3DSurface9* m_pDepthSurface;
    // GLOBAL: SWBF2 0x007fa040 -- set to 0, i.e. "the back buffer", right after

    static RedTexture* m_curRenderTarget;
    // GLOBAL: SWBF2 0x007fa044 -- a copy of m_pDepthSurface (0x006b6e47).
    static IDirect3DSurface9* m_curDepthTarget;
    // GLOBAL: SWBF2 0x007fa058 -- 840 B = 210 dwords, and 0x007fa3a0 is exactly

    static unsigned long m_curRenderStateMode[210];
    // GLOBAL: SWBF2 0x007fa3a0 -- 32 B, the eight RedTexture* the xorps/movdqu

    static RedTexture* m_curSetTextures[8];
    // GLOBAL: SWBF2 0x007fa3c0 -- 1,056 B. The dims are the TRAVERSAL's (eight

    static unsigned long m_curTextureStageState[8][33];
    // GLOBAL: SWBF2 0x007fa7e0 -- 896 B, sixteen samplers of fourteen types.
    static unsigned long m_curSamplerState[16][14];
    // GLOBAL: SWBF2 0x007fab60 -- 64 B, the memset at 0x006b6d94.
    static IDirect3DVertexBuffer9* m_curStream[16];

    // GLOBAL: SWBF2 0x007f60c8
    static IDirect3DQuery9* m_pVisibility[4048];
    // GLOBAL: SWBF2 0x007fa014
    static pcRedPrimitive* m_occlusionQueryShapes[4];

    // GLOBAL: SWBF2 0x007fa034
    static RedTexture* m_buffers[3];
    // GLOBAL: SWBF2 0x007fbe78
    static pcRedShader* m_systemShaders[6];
    // GLOBAL: SWBF2 0x007f5f04
    static IDirect3DQuery9* m_pd3dQuery;

    struct RenderStateUndo
    {
        D3DRENDERSTATETYPE state;
        unsigned long mode;
    };
    struct TextureStageStateUndo
    {
        unsigned int stage;
        D3DTEXTURESTAGESTATETYPE type;
        unsigned long value;
    };
    struct SamplerStateUndo
    {
        unsigned int sampler;
        D3DSAMPLERSTATETYPE type;
        unsigned int value;
    };

    // GLOBAL: SWBF2 0x007faba0
    static int m_iStateStackFrame;
    // GLOBAL: SWBF2 0x007faba4
    static unsigned int m_uiNumRenderStateUndo;
    // GLOBAL: SWBF2 0x007faba8 -- 1,024 B = 128 * 8, and pcSetRenderState

    static RenderStateUndo m_RenderStateUndo[128];
    // GLOBAL: SWBF2 0x007fafa8 -- 210 B, internalStateStackInit's first memset.
    static bool m_bIsRenderStateUndo[210];
    // GLOBAL: SWBF2 0x007fb07c
    static unsigned int m_uiNumTextureStageStateUndo;
    // GLOBAL: SWBF2 0x007fb080 -- 1,536 B = 128 * 12.
    static TextureStageStateUndo m_TextureStageStateUndo[128];
    // GLOBAL: SWBF2 0x007fb680 -- 264 B, the second memset (0x108).
    static bool m_bIsTextureStageStateUndo[8][33];
    // GLOBAL: SWBF2 0x007fb788 -- 1,536 B, and it is the 1,536 bytes between

    static SamplerStateUndo m_SamplerStateUndo[128];
    // GLOBAL: SWBF2 0x007fbd88
    static unsigned int m_uiNumSamplerStateUndo;
    // GLOBAL: SWBF2 0x007fbd90 -- 224 B, the third memset (0xE0).
    static bool m_bIsSamplerStateUndo[16][14];

    // GLOBAL: SWBF2 0x007f60bc -- pcSetFrontFace's redundant-set guard and its

    static pcRedFrontT m_FrontFace;
    // GLOBAL: SWBF2 0x007f60c0 / 0x007f60c4 -- the D3DCULL pair pcSetFrontFace

    static D3DCULL m_frontCull;
    static D3DCULL m_backCull;
    // GLOBAL: SWBF2 0x0086ac6c -- pdb_globals.tsv `RedRenderer::m_clearColor`,

    static RedColor m_clearColor;
public:
    static bool sLightingEnable;
    static float m_shadowIntensity;
    static RedCamera* m_camera;
    static bool m_bSetTransform_View;
    static bool m_bSetTransform_Projection;
    static bool m_bFogEnable;
    static RedViewport m_normalizedViewport;
    static float m_screenOffsetX;
    static float m_screenOffsetY;
    static int m_frameNumber;
    static float m_fWorldFogStart;
    static float m_fWorldFogEnd;
    static unsigned int m_curVisibilityIndex;
    static unsigned int m_occlusionQueryFlushOffset;
    static bool m_userNearClipPlaneEnable;
    static bool m_bStencilShadowZFail;
    static bool m_bScreenSpaceShadows;
    static float m_fStencilShadowLength;
    static int m_iImmediateMode;
    static int m_uiCurMatrixPoolOffset;
    static unsigned int m_curRenderItemOffset;
    static int m_uiCurLightingStatePoolOffset;
    static int m_bIsSetTransformViewUndo;
    static int m_bIsSetTransformProjectionUndo;
    static RedLightingState* m_pLightingState;
    static const pcRedVertexShader* m_curVertexShader;
    static const pcRedPixelShader* m_curPixelShader;
    static const pcRedVertexFormat* m_curFormat;
    static int m_PrevSortID;
    static pcRedShader* m_pPrevShader;
    static int m_uiPrevShaderState;
    static int m_pmObjWorldPrev;
    static int m_pmBoneMatricesPrev;
    static int m_pPrimPrev;
    static int m_pLightingStatePrev;
    static RedColor m_colorFog;
    static PblMatrix m_mViewUndo;
    static PblMatrix m_mProjectionUndo;
    static PblMatrix m_mProjection;
    static PblMatrix m_mProjectionTemp;
    static PblMatrix m_nearSceneFadeTexTransform;
    static PblVector4 m_userNearClipPlane;
    static PblMatrix m_mViewProjection;
    __declspec(align(16)) static PblMatrix m_mView;
};

#endif
