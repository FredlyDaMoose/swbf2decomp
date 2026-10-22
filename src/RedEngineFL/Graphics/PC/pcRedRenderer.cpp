#include "pcRedRenderer.h"

#include "RedEngineFL/Memory/engineAlloc.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "PebbleFL/Common/PblMatrix.h"
#include <d3dx9math.h>
#include "pcRedDeviceEnumerator.h"
#include "pcRedShader.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "pcRedPrimitive.h"
#include "pcRedVertexFormat.h"
#include "RedEngineFL/Graphics/RedShadingState.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include <stdlib.h>
#include <string.h>
#include "RedEngineFL/Util/RedWarning.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "StateManager_DX.h"
#include "pcRedAutoSettings.h"
#include "pcRedIndexBuffer.h"
#include "pcRedTexture.h"
#include "pcRedVertexBuffer.h"

#include "RedEngineFL/Graphics/RedAmbientLight.h"

#include "RedEngineFL/Graphics/RedDirectionalLight.h"

#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "PebbleFL/Common/PblChunk.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "PebbleFL/Common/PblMath.h"
#include <math.h>
#include "RedEngineFL/Util/ext/Globals.h"

#include "RedEngineFL/Graphics/RedSpotLight.h"
#include "pcRedPixelShader.h"
#include "pcRedVertexShader.h"

// FUNCTION: SWBF2 0x006b01e0
STDMETHODIMP_(ULONG) FakeD3D::AddRef()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b05b0
STDMETHODIMP FakeD3D::CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT,
                                             D3DFORMAT, D3DFORMAT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b05c0
STDMETHODIMP FakeD3D::CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD,
                                        D3DRESOURCETYPE, D3DFORMAT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b05d0
STDMETHODIMP FakeD3D::CheckDeviceFormatConversion(UINT, D3DDEVTYPE, D3DFORMAT,
                                                  D3DFORMAT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b05e0
STDMETHODIMP FakeD3D::CheckDeviceMultiSampleType(UINT, D3DDEVTYPE, D3DFORMAT,
                                                 BOOL, D3DMULTISAMPLE_TYPE,
                                                 DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b05f0
STDMETHODIMP FakeD3D::CheckDeviceType(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT,
                                      BOOL)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b09d0
STDMETHODIMP FakeD3D::CreateDevice(UINT, D3DDEVTYPE, HWND, DWORD,
                                   D3DPRESENT_PARAMETERS*, IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0d20
STDMETHODIMP FakeD3D::EnumAdapterModes(UINT, D3DFORMAT, UINT,
                                       D3DDISPLAYMODE* pMode)
{
    pMode->Width = 640;
    pMode->Height = 512;
    pMode->Format = D3DFMT_A8R8G8B8;
    return 0;
}

// FUNCTION: SWBF2 0x006b1060
STDMETHODIMP_(UINT) FakeD3D::GetAdapterCount()
{
    return 1;
}

// FUNCTION: SWBF2 0x006b1070
STDMETHODIMP FakeD3D::GetAdapterDisplayMode(UINT, D3DDISPLAYMODE*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1080
STDMETHODIMP FakeD3D::GetAdapterIdentifier(UINT, DWORD,
                                           D3DADAPTER_IDENTIFIER9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1090
STDMETHODIMP_(UINT) FakeD3D::GetAdapterModeCount(UINT, D3DFORMAT)
{
    return 1;
}

// FUNCTION: SWBF2 0x006b10a0
STDMETHODIMP_(HMONITOR) FakeD3D::GetAdapterMonitor(UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b12c0
STDMETHODIMP FakeD3D::GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b28e0
STDMETHODIMP FakeD3D::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2d00
STDMETHODIMP FakeD3D::RegisterSoftwareDevice(void*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2d10
STDMETHODIMP_(ULONG) FakeD3D::Release()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b01c0
STDMETHODIMP FakeD3DCubeTexture::AddDirtyRect(D3DCUBEMAP_FACES, CONST RECT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b01f0
STDMETHODIMP_(ULONG) FakeD3DCubeTexture::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b0fd0
STDMETHODIMP FakeD3DCubeTexture::FreePrivateData(REFGUID)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1030
STDMETHODIMP_(void) FakeD3DCubeTexture::GenerateMipSubLevels()
{
}

// FUNCTION: SWBF2 0x006b10b0
STDMETHODIMP_(D3DTEXTUREFILTERTYPE) FakeD3DCubeTexture::GetAutoGenFilterType()
{
    return D3DTEXF_LINEAR;
}

// FUNCTION: SWBF2 0x006b1180
STDMETHODIMP FakeD3DCubeTexture::GetCubeMapSurface(D3DCUBEMAP_FACES, UINT,
                                                   IDirect3DSurface9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1250
STDMETHODIMP FakeD3DCubeTexture::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b13f0
STDMETHODIMP_(DWORD) FakeD3DCubeTexture::GetLOD()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1420
STDMETHODIMP_(DWORD) FakeD3DCubeTexture::GetLevelCount()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1450
STDMETHODIMP FakeD3DCubeTexture::GetLevelDesc(UINT, D3DSURFACE_DESC*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1620
STDMETHODIMP_(DWORD) FakeD3DCubeTexture::GetPriority()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1680
STDMETHODIMP FakeD3DCubeTexture::GetPrivateData(REFGUID, void*, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1970
PblTimeBase* RedRenderer::GetTimeBase()
{
    return m_pTimeBase;
}

// FUNCTION: SWBF2 0x006b1990
STDMETHODIMP_(D3DRESOURCETYPE) FakeD3DCubeTexture::GetType()
{
    return D3DRTYPE_CUBETEXTURE;
}

// FUNCTION: SWBF2 0x006b2670
STDMETHODIMP FakeD3DCubeTexture::LockRect(D3DCUBEMAP_FACES, UINT,
                                          D3DLOCKED_RECT* pLockedRect,
                                          CONST RECT*, DWORD)
{
    if (lockCount++ == 0)
    {
        bits = new unsigned char[levels
                                              * length
                                              * length * 4];
    }
    pLockedRect->pBits = bits;
    return 0;
}

// FUNCTION: SWBF2 0x006b2760
STDMETHODIMP_(void) FakeD3DCubeTexture::PreLoad()
{
}

// FUNCTION: SWBF2 0x006b28f0
STDMETHODIMP FakeD3DCubeTexture::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2d20
STDMETHODIMP_(ULONG) FakeD3DCubeTexture::Release()
{
    if (--refCount == 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b33f0
STDMETHODIMP FakeD3DCubeTexture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b36f0
STDMETHODIMP_(DWORD) FakeD3DCubeTexture::SetLOD(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3af0
STDMETHODIMP_(DWORD) FakeD3DCubeTexture::SetPriority(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b50
STDMETHODIMP FakeD3DCubeTexture::SetPrivateData(REFGUID, CONST void*, DWORD,
                                                DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4390
STDMETHODIMP FakeD3DCubeTexture::UnlockRect(D3DCUBEMAP_FACES, UINT)
{
    if (--lockCount == 0)
    {
        delete[] bits;
        bits = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b0200
STDMETHODIMP_(ULONG) FakeD3DDevice::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b0550
STDMETHODIMP FakeD3DDevice::BeginScene()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b05a0
STDMETHODIMP FakeD3DDevice::BeginStateBlock()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0930
STDMETHODIMP FakeD3DDevice::Clear(DWORD, CONST D3DRECT*, DWORD, D3DCOLOR,
                                  float, DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0940
STDMETHODIMP FakeD3DDevice::ColorFill(IDirect3DSurface9*, CONST RECT*,
                                      D3DCOLOR)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0950
STDMETHODIMP FakeD3DDevice::CreateAdditionalSwapChain(
    D3DPRESENT_PARAMETERS*, IDirect3DSwapChain9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0960
STDMETHODIMP FakeD3DDevice::CreateCubeTexture(
    UINT EdgeLength, UINT Levels, DWORD, D3DFORMAT, D3DPOOL,
    IDirect3DCubeTexture9** ppCubeTexture, HANDLE*)
{
    *ppCubeTexture = new FakeD3DCubeTexture(EdgeLength, Levels);
    return 0;
}

// FUNCTION: SWBF2 0x006b09c0
STDMETHODIMP FakeD3DDevice::CreateDepthStencilSurface(
    UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL,
    IDirect3DSurface9**, HANDLE*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0a20
STDMETHODIMP FakeD3DDevice::CreateIndexBuffer(
    UINT Length, DWORD, D3DFORMAT, D3DPOOL,
    IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE*)
{
    *ppIndexBuffer = new FakeD3DIndexBuffer(Length);
    return 0;
}

// FUNCTION: SWBF2 0x006b0a70
STDMETHODIMP FakeD3DDevice::CreateOffscreenPlainSurface(
    UINT, UINT, D3DFORMAT, D3DPOOL, IDirect3DSurface9**, HANDLE*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0a80
STDMETHODIMP FakeD3DDevice::CreatePixelShader(CONST DWORD*,
                                              IDirect3DPixelShader9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0a90
STDMETHODIMP FakeD3DDevice::CreateQuery(D3DQUERYTYPE, IDirect3DQuery9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0aa0
STDMETHODIMP FakeD3DDevice::CreateRenderTarget(
    UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL,
    IDirect3DSurface9**, HANDLE*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0ab0
STDMETHODIMP FakeD3DDevice::CreateStateBlock(D3DSTATEBLOCKTYPE,
                                             IDirect3DStateBlock9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0ac0
STDMETHODIMP FakeD3DDevice::CreateTexture(UINT Width, UINT Height, UINT,
                                          DWORD, D3DFORMAT Format, D3DPOOL,
                                          IDirect3DTexture9** ppTexture,
                                          HANDLE*)
{
    *ppTexture = new FakeD3DTexture(Width, Height, Format);
    return 0;
}

// FUNCTION: SWBF2 0x006b0b20
STDMETHODIMP FakeD3DDevice::CreateVertexBuffer(
    UINT Length, DWORD, DWORD, D3DPOOL,
    IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE*)
{
    *ppVertexBuffer = new FakeD3DVertexBuffer(Length);
    return 0;
}

// FUNCTION: SWBF2 0x006b0b70
STDMETHODIMP FakeD3DDevice::CreateVertexDeclaration(
    CONST D3DVERTEXELEMENT9*, IDirect3DVertexDeclaration9** ppDecl)
{
    *ppDecl = new FakeD3DVertexDeclaration;
    return 0;
}

// FUNCTION: SWBF2 0x006b0bb0
STDMETHODIMP FakeD3DDevice::CreateVertexShader(CONST DWORD*,
                                               IDirect3DVertexShader9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0bc0
STDMETHODIMP FakeD3DDevice::CreateVolumeTexture(
    UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD, D3DFORMAT Format,
    D3DPOOL, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE*)
{
    *ppVolumeTexture =
        new FakeD3DVolumeTexture(Width, Height, Depth, Levels, Format);
    return 0;
}

// FUNCTION: SWBF2 0x006b0c30
STDMETHODIMP FakeD3DDevice::DeletePatch(UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0c40
STDMETHODIMP FakeD3DDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT, UINT,
                                                 UINT, UINT, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0c50
STDMETHODIMP FakeD3DDevice::DrawIndexedPrimitiveUP(
    D3DPRIMITIVETYPE, UINT, UINT, UINT, CONST void*, D3DFORMAT, CONST void*,
    UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0c60
STDMETHODIMP FakeD3DDevice::DrawPrimitive(D3DPRIMITIVETYPE, UINT, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0c70
STDMETHODIMP FakeD3DDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT,
                                            CONST void*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0c80
STDMETHODIMP FakeD3DDevice::DrawRectPatch(UINT, CONST float*,
                                          CONST D3DRECTPATCH_INFO*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0c90
STDMETHODIMP FakeD3DDevice::DrawTriPatch(UINT, CONST float*,
                                         CONST D3DTRIPATCH_INFO*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0ce0
STDMETHODIMP FakeD3DDevice::EndScene()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0d10
STDMETHODIMP FakeD3DDevice::EndStateBlock(IDirect3DStateBlock9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0d40
STDMETHODIMP FakeD3DDevice::EvictManagedResources()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b10e0
STDMETHODIMP_(UINT) FakeD3DDevice::GetAvailableTextureMem()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b10f0
STDMETHODIMP FakeD3DDevice::GetBackBuffer(UINT, UINT, D3DBACKBUFFER_TYPE,
                                          IDirect3DSurface9** ppBackBuffer)
{
    *ppBackBuffer = new FakeD3DSurface;
    return 0;
}

// FUNCTION: SWBF2 0x006b1140
STDMETHODIMP FakeD3DDevice::GetClipPlane(DWORD, float*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1150
STDMETHODIMP FakeD3DDevice::GetClipStatus(D3DCLIPSTATUS9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1170
STDMETHODIMP FakeD3DDevice::GetCreationParameters(
    D3DDEVICE_CREATION_PARAMETERS*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1190
STDMETHODIMP FakeD3DDevice::GetCurrentTexturePalette(UINT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b11d0
STDMETHODIMP FakeD3DDevice::GetDepthStencilSurface(
    IDirect3DSurface9** ppZStencilSurface)
{
    *ppZStencilSurface = new FakeD3DSurface;
    return 0;
}

// FUNCTION: SWBF2 0x006b12d0
STDMETHODIMP FakeD3DDevice::GetDeviceCaps(D3DCAPS9* pCaps)
{
    pCaps->VertexShaderVersion = D3DVS_VERSION(1, 1);
    pCaps->PixelShaderVersion = D3DPS_VERSION(1, 1);
    pCaps->MaxVertexShaderConst = 96;
    pCaps->MaxTextureWidth = 2048;
    pCaps->MaxTextureHeight = 2048;
    return 0;
}

// FUNCTION: SWBF2 0x006b1310
STDMETHODIMP FakeD3DDevice::GetDirect3D(IDirect3D9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1320
STDMETHODIMP FakeD3DDevice::GetDisplayMode(UINT, D3DDISPLAYMODE*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1330
STDMETHODIMP FakeD3DDevice::GetFVF(DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b13c0
STDMETHODIMP FakeD3DDevice::GetFrontBufferData(UINT, IDirect3DSurface9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b13d0
STDMETHODIMP_(void) FakeD3DDevice::GetGammaRamp(UINT, D3DGAMMARAMP*)
{
}

// FUNCTION: SWBF2 0x006b13e0
STDMETHODIMP FakeD3DDevice::GetIndices(IDirect3DIndexBuffer9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b14b0
STDMETHODIMP FakeD3DDevice::GetLight(DWORD, D3DLIGHT9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b14c0
STDMETHODIMP FakeD3DDevice::GetLightEnable(DWORD, BOOL*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b14f0
STDMETHODIMP FakeD3DDevice::GetMaterial(D3DMATERIAL9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1500
STDMETHODIMP_(float) FakeD3DDevice::GetNPatchMode()
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x006b1510
STDMETHODIMP_(UINT) FakeD3DDevice::GetNumberOfSwapChains()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1560
STDMETHODIMP FakeD3DDevice::GetPaletteEntries(UINT, PALETTEENTRY*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b15e0
STDMETHODIMP FakeD3DDevice::GetPixelShader(IDirect3DPixelShader9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b15f0
STDMETHODIMP FakeD3DDevice::GetPixelShaderConstantB(UINT, BOOL*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1600
STDMETHODIMP FakeD3DDevice::GetPixelShaderConstantF(UINT, float*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1610
STDMETHODIMP FakeD3DDevice::GetPixelShaderConstantI(UINT, int*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1800
STDMETHODIMP FakeD3DDevice::GetRasterStatus(UINT, D3DRASTER_STATUS*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1810
STDMETHODIMP FakeD3DDevice::GetRenderState(D3DRENDERSTATETYPE, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1820
STDMETHODIMP FakeD3DDevice::GetRenderTarget(DWORD,
                                            IDirect3DSurface9** ppRenderTarget)
{
    *ppRenderTarget = new FakeD3DSurface;
    return 0;
}

// FUNCTION: SWBF2 0x006b1860
STDMETHODIMP FakeD3DDevice::GetRenderTargetData(IDirect3DSurface9*,
                                                IDirect3DSurface9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1870
STDMETHODIMP FakeD3DDevice::GetSamplerState(DWORD, D3DSAMPLERSTATETYPE, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1880
STDMETHODIMP FakeD3DDevice::GetScissorRect(RECT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1890
float RedRenderer::GetScreenAspectRatio()
{
    return m_screenAspectRatio;
}

// FUNCTION: SWBF2 0x006b18e0
STDMETHODIMP_(BOOL) FakeD3DDevice::GetSoftwareVertexProcessing()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1900
STDMETHODIMP FakeD3DDevice::GetStreamSource(UINT, IDirect3DVertexBuffer9**,
                                            UINT*, UINT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1910
STDMETHODIMP FakeD3DDevice::GetStreamSourceFreq(UINT, UINT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1940
STDMETHODIMP FakeD3DDevice::GetSwapChain(UINT, IDirect3DSwapChain9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1950
STDMETHODIMP FakeD3DDevice::GetTexture(DWORD, IDirect3DBaseTexture9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1960
STDMETHODIMP FakeD3DDevice::GetTextureStageState(DWORD,
                                                 D3DTEXTURESTAGESTATETYPE,
                                                 DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1980
STDMETHODIMP FakeD3DDevice::GetTransform(D3DTRANSFORMSTATETYPE, D3DMATRIX*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b19f0
STDMETHODIMP FakeD3DDevice::GetVertexDeclaration(
    IDirect3DVertexDeclaration9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1a00
STDMETHODIMP FakeD3DDevice::GetVertexShader(IDirect3DVertexShader9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1a10
STDMETHODIMP FakeD3DDevice::GetVertexShaderConstantB(UINT, BOOL*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1a20
STDMETHODIMP FakeD3DDevice::GetVertexShaderConstantF(UINT, float*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1a30
STDMETHODIMP FakeD3DDevice::GetVertexShaderConstantI(UINT, int*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1a40
STDMETHODIMP FakeD3DDevice::GetViewport(D3DVIEWPORT9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2580
STDMETHODIMP FakeD3DDevice::LightEnable(DWORD, BOOL)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2710
STDMETHODIMP FakeD3DDevice::MultiplyTransform(D3DTRANSFORMSTATETYPE,
                                              CONST D3DMATRIX*)
{
    return 0;
}

extern bool g_bNoWait;
extern float g_fFixedRate;

// FUNCTION: SWBF2 0x006b27c0
STDMETHODIMP FakeD3DDevice::Present(CONST RECT*, CONST RECT*, HWND,
                                    CONST RGNDATA*)
{
    if (!g_bNoWait)
    {
        if (g_fFixedRate > 0.0f)
        {
            Sleep(static_cast<int>(1000.0f / g_fFixedRate));
        }
        else
        {
            Sleep(16);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b2810
STDMETHODIMP FakeD3DDevice::ProcessVertices(UINT, UINT, UINT,
                                            IDirect3DVertexBuffer9*,
                                            IDirect3DVertexDeclaration9*,
                                            DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2900
STDMETHODIMP FakeD3DDevice::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2d40
STDMETHODIMP_(ULONG) FakeD3DDevice::Release()
{
    if (--refCount <= 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b3100
STDMETHODIMP FakeD3DDevice::Reset(D3DPRESENT_PARAMETERS*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3440
STDMETHODIMP FakeD3DDevice::SetClipPlane(DWORD, CONST float*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3450
STDMETHODIMP FakeD3DDevice::SetClipStatus(CONST D3DCLIPSTATUS9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3470
STDMETHODIMP FakeD3DDevice::SetCurrentTexturePalette(UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3490
STDMETHODIMP_(void) FakeD3DDevice::SetCursorPosition(int, int, DWORD)
{
}

// FUNCTION: SWBF2 0x006b34a0
STDMETHODIMP FakeD3DDevice::SetCursorProperties(UINT, UINT, IDirect3DSurface9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b34d0
STDMETHODIMP FakeD3DDevice::SetDepthStencilSurface(IDirect3DSurface9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b34e0
STDMETHODIMP FakeD3DDevice::SetDialogBoxMode(BOOL)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b34f0
STDMETHODIMP FakeD3DDevice::SetFVF(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3660
STDMETHODIMP_(void) FakeD3DDevice::SetGammaRamp(UINT, DWORD,
                                                CONST D3DGAMMARAMP*)
{
}

// FUNCTION: SWBF2 0x006b3670
STDMETHODIMP FakeD3DDevice::SetIndices(IDirect3DIndexBuffer9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3720
STDMETHODIMP FakeD3DDevice::SetLight(DWORD, CONST D3DLIGHT9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b37b0
STDMETHODIMP FakeD3DDevice::SetMaterial(CONST D3DMATERIAL9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3a40
STDMETHODIMP FakeD3DDevice::SetNPatchMode(float)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3a50
STDMETHODIMP FakeD3DDevice::SetPaletteEntries(UINT, CONST PALETTEENTRY*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3a60
STDMETHODIMP FakeD3DDevice::SetPixelShader(IDirect3DPixelShader9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3ac0
STDMETHODIMP FakeD3DDevice::SetPixelShaderConstantB(UINT, CONST BOOL*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3ad0
STDMETHODIMP FakeD3DDevice::SetPixelShaderConstantF(UINT, CONST float*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3ae0
STDMETHODIMP FakeD3DDevice::SetPixelShaderConstantI(UINT, CONST int*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3bb0
STDMETHODIMP FakeD3DDevice::SetRenderState(D3DRENDERSTATETYPE, DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3bc0
STDMETHODIMP FakeD3DDevice::SetRenderTarget(DWORD, IDirect3DSurface9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3bd0
STDMETHODIMP FakeD3DDevice::SetSamplerState(DWORD, D3DSAMPLERSTATETYPE, DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3c50
STDMETHODIMP FakeD3DDevice::SetScissorRect(CONST RECT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3cd0
STDMETHODIMP FakeD3DDevice::SetSoftwareVertexProcessing(BOOL)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3d10
STDMETHODIMP FakeD3DDevice::SetStreamSource(UINT, IDirect3DVertexBuffer9*,
                                            UINT, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3da0
STDMETHODIMP FakeD3DDevice::SetStreamSourceFreq(UINT, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3db0
STDMETHODIMP FakeD3DDevice::SetTexture(DWORD, IDirect3DBaseTexture9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3e20
STDMETHODIMP FakeD3DDevice::SetTextureStageState(DWORD,
                                                 D3DTEXTURESTAGESTATETYPE,
                                                 DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3ea0
void RedRenderer::SetTimeBase(PblTimeBase* pTimeBase)
{
    m_pTimeBase = pTimeBase;
}

// FUNCTION: SWBF2 0x006b3eb0
STDMETHODIMP FakeD3DDevice::SetTransform(D3DTRANSFORMSTATETYPE,
                                         CONST D3DMATRIX*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3f40
STDMETHODIMP FakeD3DDevice::SetVertexDeclaration(IDirect3DVertexDeclaration9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3f50
STDMETHODIMP FakeD3DDevice::SetVertexShader(IDirect3DVertexShader9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3fb0
STDMETHODIMP FakeD3DDevice::SetVertexShaderConstantB(UINT, CONST BOOL*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3fc0
STDMETHODIMP FakeD3DDevice::SetVertexShaderConstantF(UINT, CONST float*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3fd0
STDMETHODIMP FakeD3DDevice::SetVertexShaderConstantI(UINT, CONST int*, UINT)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3fe0
STDMETHODIMP FakeD3DDevice::SetViewport(CONST D3DVIEWPORT9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4010
STDMETHODIMP_(BOOL) FakeD3DDevice::ShowCursor(BOOL)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4030
STDMETHODIMP FakeD3DDevice::StretchRect(IDirect3DSurface9*, CONST RECT*,
                                        IDirect3DSurface9*, CONST RECT*,
                                        D3DTEXTUREFILTERTYPE)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b41a0
STDMETHODIMP FakeD3DDevice::TestCooperativeLevel()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4400
STDMETHODIMP FakeD3DDevice::UpdateSurface(IDirect3DSurface9*, CONST RECT*,
                                          IDirect3DSurface9*, CONST POINT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4410
STDMETHODIMP FakeD3DDevice::UpdateTexture(IDirect3DBaseTexture9*,
                                          IDirect3DBaseTexture9*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4420
STDMETHODIMP FakeD3DDevice::ValidateDevice(DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0210
STDMETHODIMP_(ULONG) FakeD3DIndexBuffer::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b0fe0
STDMETHODIMP FakeD3DIndexBuffer::FreePrivateData(REFGUID)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1210
STDMETHODIMP FakeD3DIndexBuffer::GetDesc(D3DINDEXBUFFER_DESC*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1260
STDMETHODIMP FakeD3DIndexBuffer::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1630
STDMETHODIMP_(DWORD) FakeD3DIndexBuffer::GetPriority()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1690
STDMETHODIMP FakeD3DIndexBuffer::GetPrivateData(REFGUID, void*, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b19a0
STDMETHODIMP_(D3DRESOURCETYPE) FakeD3DIndexBuffer::GetType()
{
    return D3DRTYPE_INDEXBUFFER;
}

// FUNCTION: SWBF2 0x006b2590
STDMETHODIMP FakeD3DIndexBuffer::Lock(UINT, UINT, void** ppbData, DWORD)
{
    if (lockCount++ == 0)
    {
        bits = new unsigned char[len];
    }
    *ppbData = bits;
    return 0;
}

// FUNCTION: SWBF2 0x006b2770
STDMETHODIMP_(void) FakeD3DIndexBuffer::PreLoad()
{
}

// FUNCTION: SWBF2 0x006b2910
STDMETHODIMP FakeD3DIndexBuffer::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2d60
STDMETHODIMP_(ULONG) FakeD3DIndexBuffer::Release()
{
    if (--refCount == 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b3b00
STDMETHODIMP_(DWORD) FakeD3DIndexBuffer::SetPriority(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b60
STDMETHODIMP FakeD3DIndexBuffer::SetPrivateData(REFGUID, CONST void*, DWORD,
                                                DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4300
STDMETHODIMP FakeD3DIndexBuffer::Unlock()
{
    if (--lockCount == 0)
    {
        delete[] bits;
        bits = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b0220
STDMETHODIMP_(ULONG) FakeD3DSurface::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b0ff0
STDMETHODIMP FakeD3DSurface::FreePrivateData(REFGUID)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1160
STDMETHODIMP FakeD3DSurface::GetContainer(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b11a0
STDMETHODIMP FakeD3DSurface::GetDC(HDC*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1220
STDMETHODIMP FakeD3DSurface::GetDesc(D3DSURFACE_DESC* pDesc)
{
    pDesc->Width = 128;
    pDesc->Height = 128;
    pDesc->Format = D3DFMT_A8R8G8B8;
    return 0;
}

// FUNCTION: SWBF2 0x006b1270
STDMETHODIMP FakeD3DSurface::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1640
STDMETHODIMP_(DWORD) FakeD3DSurface::GetPriority()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b16a0
STDMETHODIMP FakeD3DSurface::GetPrivateData(REFGUID, void*, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b19b0
STDMETHODIMP_(D3DRESOURCETYPE) FakeD3DSurface::GetType()
{
    return D3DRTYPE_SURFACE;
}

// FUNCTION: SWBF2 0x006b26b0
STDMETHODIMP FakeD3DSurface::LockRect(D3DLOCKED_RECT*, CONST RECT*, DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2720
static IDirect3DSurface9* NewFakeD3DSurface()
{
    return new FakeD3DSurface;
}

// FUNCTION: SWBF2 0x006b2780
STDMETHODIMP_(void) FakeD3DSurface::PreLoad()
{
}

// FUNCTION: SWBF2 0x006b2920
STDMETHODIMP FakeD3DSurface::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2d80
STDMETHODIMP_(ULONG) FakeD3DSurface::Release()
{
    if (--refCount == 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b2e20
STDMETHODIMP FakeD3DSurface::ReleaseDC(HDC)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b10
STDMETHODIMP_(DWORD) FakeD3DSurface::SetPriority(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b70
STDMETHODIMP FakeD3DSurface::SetPrivateData(REFGUID, CONST void*, DWORD, DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b43c0
STDMETHODIMP FakeD3DSurface::UnlockRect()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b01d0
STDMETHODIMP FakeD3DTexture::AddDirtyRect(CONST RECT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0230
STDMETHODIMP_(ULONG) FakeD3DTexture::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b1000
STDMETHODIMP FakeD3DTexture::FreePrivateData(REFGUID)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1040
STDMETHODIMP_(void) FakeD3DTexture::GenerateMipSubLevels()
{
}

// FUNCTION: SWBF2 0x006b10c0
STDMETHODIMP_(D3DTEXTUREFILTERTYPE) FakeD3DTexture::GetAutoGenFilterType()
{
    return D3DTEXF_NONE;
}

// FUNCTION: SWBF2 0x006b1280
STDMETHODIMP FakeD3DTexture::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1400
STDMETHODIMP_(DWORD) FakeD3DTexture::GetLOD()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1430
STDMETHODIMP_(DWORD) FakeD3DTexture::GetLevelCount()
{
    return 1;
}

// FUNCTION: SWBF2 0x006b1460
STDMETHODIMP FakeD3DTexture::GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc)
{
    pDesc->Width = w >> Level;
    pDesc->Height = h >> Level;
    pDesc->Format = (D3DFORMAT)fmt;
    return 0;
}

// FUNCTION: SWBF2 0x006b1650
STDMETHODIMP_(DWORD) FakeD3DTexture::GetPriority()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b16b0
STDMETHODIMP FakeD3DTexture::GetPrivateData(REFGUID, void*, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1920
STDMETHODIMP FakeD3DTexture::GetSurfaceLevel(UINT,
                                             IDirect3DSurface9** ppSurfaceLevel)
{
    *ppSurfaceLevel = NewFakeD3DSurface();
    return 0;
}

// FUNCTION: SWBF2 0x006b19c0
STDMETHODIMP_(D3DRESOURCETYPE) FakeD3DTexture::GetType()
{
    return D3DRTYPE_TEXTURE;
}

// FUNCTION: SWBF2 0x006b26c0
STDMETHODIMP FakeD3DTexture::LockRect(UINT, D3DLOCKED_RECT* pLockedRect,
                                      CONST RECT*, DWORD)
{
    if (lockCount++ == 0)
    {
        bits =
            new unsigned char[h * w * 4];
    }
    pLockedRect->pBits = bits;
    pLockedRect->Pitch = w * 4;
    return 0;
}

// FUNCTION: SWBF2 0x006b2790
STDMETHODIMP_(void) FakeD3DTexture::PreLoad()
{
}

// FUNCTION: SWBF2 0x006b2930
STDMETHODIMP FakeD3DTexture::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2da0
STDMETHODIMP_(ULONG) FakeD3DTexture::Release()
{
    if (--refCount == 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b3400
STDMETHODIMP FakeD3DTexture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3700
STDMETHODIMP_(DWORD) FakeD3DTexture::SetLOD(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b20
STDMETHODIMP_(DWORD) FakeD3DTexture::SetPriority(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b80
STDMETHODIMP FakeD3DTexture::SetPrivateData(REFGUID, CONST void*, DWORD, DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b43d0
STDMETHODIMP FakeD3DTexture::UnlockRect(UINT)
{
    if (--lockCount == 0)
    {
        delete[] bits;
        bits = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b0240
STDMETHODIMP_(ULONG) FakeD3DVertexBuffer::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b1010
STDMETHODIMP FakeD3DVertexBuffer::FreePrivateData(REFGUID)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1240
STDMETHODIMP FakeD3DVertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1290
STDMETHODIMP FakeD3DVertexBuffer::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1660
STDMETHODIMP_(DWORD) FakeD3DVertexBuffer::GetPriority()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b16c0
STDMETHODIMP FakeD3DVertexBuffer::GetPrivateData(REFGUID, void*, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b19d0
STDMETHODIMP_(D3DRESOURCETYPE) FakeD3DVertexBuffer::GetType()
{
    return D3DRTYPE_VERTEXBUFFER;
}

// FUNCTION: SWBF2 0x006b25d0
STDMETHODIMP FakeD3DVertexBuffer::Lock(UINT, UINT, void** ppbData, DWORD)
{
    if (lockCount++ == 0)
    {
        bits = new unsigned char[len];
    }
    *ppbData = bits;
    return 0;
}

// FUNCTION: SWBF2 0x006b27a0
STDMETHODIMP_(void) FakeD3DVertexBuffer::PreLoad()
{
}

// FUNCTION: SWBF2 0x006b2940
STDMETHODIMP FakeD3DVertexBuffer::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2dc0
STDMETHODIMP_(ULONG) FakeD3DVertexBuffer::Release()
{
    if (--refCount == 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b3b30
STDMETHODIMP_(DWORD) FakeD3DVertexBuffer::SetPriority(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b90
STDMETHODIMP FakeD3DVertexBuffer::SetPrivateData(REFGUID, CONST void*, DWORD,
                                                 DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4330
STDMETHODIMP FakeD3DVertexBuffer::Unlock()
{
    if (--lockCount == 0)
    {
        delete[] bits;
        bits = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b0250
STDMETHODIMP_(ULONG) FakeD3DVertexDeclaration::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b11b0
STDMETHODIMP FakeD3DVertexDeclaration::GetDeclaration(D3DVERTEXELEMENT9*,
                                                      UINT*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b12a0
STDMETHODIMP FakeD3DVertexDeclaration::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2950
STDMETHODIMP FakeD3DVertexDeclaration::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2de0
STDMETHODIMP_(ULONG) FakeD3DVertexDeclaration::Release()
{
    if (--refCount <= 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b01b0
STDMETHODIMP FakeD3DVolumeTexture::AddDirtyBox(CONST D3DBOX*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b0260
STDMETHODIMP_(ULONG) FakeD3DVolumeTexture::AddRef()
{
    ++refCount;
    return 0;
}

// FUNCTION: SWBF2 0x006b1020
STDMETHODIMP FakeD3DVolumeTexture::FreePrivateData(REFGUID)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1050
STDMETHODIMP_(void) FakeD3DVolumeTexture::GenerateMipSubLevels()
{
}

// FUNCTION: SWBF2 0x006b10d0
STDMETHODIMP_(D3DTEXTUREFILTERTYPE) FakeD3DVolumeTexture::GetAutoGenFilterType()
{
    return D3DTEXF_LINEAR;
}

// FUNCTION: SWBF2 0x006b12b0
STDMETHODIMP FakeD3DVolumeTexture::GetDevice(IDirect3DDevice9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1410
STDMETHODIMP_(DWORD) FakeD3DVolumeTexture::GetLOD()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1440
STDMETHODIMP_(DWORD) FakeD3DVolumeTexture::GetLevelCount()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b1490
STDMETHODIMP FakeD3DVolumeTexture::GetLevelDesc(UINT, D3DVOLUME_DESC* pDesc)
{
    pDesc->Width = width;
    pDesc->Height = height;
    pDesc->Format = (D3DFORMAT)format;
    return 0;
}

// FUNCTION: SWBF2 0x006b1670
STDMETHODIMP_(DWORD) FakeD3DVolumeTexture::GetPriority()
{
    return 0;
}

// FUNCTION: SWBF2 0x006b16d0
STDMETHODIMP FakeD3DVolumeTexture::GetPrivateData(REFGUID, void*, DWORD*)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b19e0
STDMETHODIMP_(D3DRESOURCETYPE) FakeD3DVolumeTexture::GetType()
{
    return D3DRTYPE_VOLUMETEXTURE;
}

// FUNCTION: SWBF2 0x006b1a50
STDMETHODIMP FakeD3DVolumeTexture::GetVolumeLevel(UINT, IDirect3DVolume9**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2610
STDMETHODIMP FakeD3DVolumeTexture::LockBox(UINT, D3DLOCKED_BOX* pLockedVolume,
                                           CONST D3DBOX*, DWORD)
{
    if (lockCount++ == 0)
    {
        bits = new unsigned char[levels
                                              * depth
                                              * height
                                              * width * 4];
    }
    pLockedVolume->pBits = bits;
    pLockedVolume->RowPitch = width * 4;
    pLockedVolume->SlicePitch = height * width * 4;
    return 0;
}

// FUNCTION: SWBF2 0x006b27b0
STDMETHODIMP_(void) FakeD3DVolumeTexture::PreLoad()
{
}

// FUNCTION: SWBF2 0x006b2960
STDMETHODIMP FakeD3DVolumeTexture::QueryInterface(REFIID, void**)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b2e00
STDMETHODIMP_(ULONG) FakeD3DVolumeTexture::Release()
{
    if (--refCount == 0)
    {
        delete this;
    }
    return 0;
}

// FUNCTION: SWBF2 0x006b3410
STDMETHODIMP FakeD3DVolumeTexture::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3710
STDMETHODIMP_(DWORD) FakeD3DVolumeTexture::SetLOD(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3b40
STDMETHODIMP_(DWORD) FakeD3DVolumeTexture::SetPriority(DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b3ba0
STDMETHODIMP FakeD3DVolumeTexture::SetPrivateData(REFGUID, CONST void*, DWORD,
                                                  DWORD)
{
    return 0;
}

// FUNCTION: SWBF2 0x006b4360
STDMETHODIMP FakeD3DVolumeTexture::UnlockBox(UINT)
{
    if (--lockCount == 0)
    {
        delete[] bits;
        bits = 0;
    }
    return 0;
}

RedViewport RedRenderer::m_normalizedViewport;

// FUNCTION: SWBF2 0x006b8480
const D3DADAPTER_IDENTIFIER9& RedRenderer::pcGetAdaptorIdentifier()
{
    return m_adaptorIdentifier;
}

// FUNCTION: SWBF2 0x006b8490
RedTexture* RedRenderer::pcGetBuffer(pcBufferT eBuffer)
{
    return m_buffers[eBuffer];
}

// FUNCTION: SWBF2 0x006b84d0
IDirect3DSurface9* RedRenderer::pcGetDepthTarget()
{
    return m_curDepthTarget;
}

// FUNCTION: SWBF2 0x006b84e0
IDirect3DDevice9* RedRenderer::pcGetDevice()
{
    return m_pd3dDevice;
}

// FUNCTION: SWBF2 0x006b84f0
const D3DCAPS9& RedRenderer::pcGetDeviceCaps()
{
    return m_deviceCaps;
}

// FUNCTION: SWBF2 0x006b8500
GeometryModeT RedRenderer::pcGetGeometryMode()
{
    return m_renderCaps.geometry;
}

// FUNCTION: SWBF2 0x006b8510
float RedRenderer::pcGetNearSceneEnd()
{
    return m_fNearSceneEnd;
}

// FUNCTION: SWBF2 0x006b8530
float RedRenderer::pcGetNearSceneStart()
{
    return m_fNearSceneStart;
}

// FUNCTION: SWBF2 0x006b8550
const RenderCapsT& RedRenderer::pcGetRenderCaps()
{
    return m_renderCaps;
}

// FUNCTION: SWBF2 0x006b8560
const RenderOptionsT& RedRenderer::pcGetRenderOptions()
{
    return m_renderOptions;
}

// FUNCTION: SWBF2 0x006b8570
RedTexture* RedRenderer::pcGetRenderTarget()
{
    return m_curRenderTarget;
}

 

// FUNCTION: SWBF2 0x006b85a0
unsigned int RedRenderer::pcGetValidDisplayModes(D3DDISPLAYMODE* pModes,
                                                 unsigned int uiMaxModes,
                                                 unsigned int uiMinWidth,
                                                 unsigned int uiMinHeight)
{
    unsigned int uiCount = 0;

    PblList<D3DAdapterInfo>& adapterList = pcRedDeviceEnumerator::GetDeviceList();

    for (PblList<D3DAdapterInfo>::Node* pAdapterNode = adapterList.GetFirst();
         pAdapterNode->_pObject != adapterList._head._pObject;
         pAdapterNode = pAdapterNode->_pNext)
    {
        PblList<D3DDisplayMode>& modeList = pAdapterNode->_pObject->DisplayModeList;

        for (PblList<D3DDisplayMode>::Node* pModeNode = modeList.GetFirst();
             pModeNode->_pObject != modeList._head._pObject;
             pModeNode = pModeNode->_pNext)
        {
            const D3DDISPLAYMODE& mode = pModeNode->_pObject->displayMode;

            if (mode.Width < uiMinWidth || mode.Height < uiMinHeight)
            {
                continue;
            }

            bool bAlreadyListed = false;
            for (unsigned int i = 0; i < uiCount; ++i)
            {
                if (pModes[i].Width == mode.Width && pModes[i].Height == mode.Height)
                {
                    bAlreadyListed = true;
                    break;
                }
            }

            if (bAlreadyListed || uiCount >= uiMaxModes)
            {
                continue;
            }

            pModes[uiCount++] = mode;
        }
    }

    return uiCount;
}

// FUNCTION: SWBF2 0x006b8680
void RedRenderer::pcGetViewport(RedViewport& rViewport)
{
    rViewport = RedRenderer::m_normalizedViewport;
}

// FUNCTION: SWBF2 0x006b88b0
void RedRenderer::pcRegisterResetFunction(void(__cdecl* pFunction)(ResetFlagsT))
{
    unsigned int uiCount = m_numResetFunctions;

    for (unsigned int i = 0; i < uiCount; ++i)
    {
        if (m_resetFunctions[i] == pFunction)
        {
            return;
        }
    }

    if (uiCount >= 100)
    {
        return;
    }

    m_resetFunctions[uiCount] = pFunction;
    m_numResetFunctions = uiCount + 1;
}

// FUNCTION: SWBF2 0x006b9f50
void RedRenderer::pcUnregisterResetFunction(void(__cdecl* pFunction)(ResetFlagsT))
{
    unsigned int uiCount = m_numResetFunctions;

    for (unsigned int i = 0; i < uiCount; ++i)
    {
        if (m_resetFunctions[i] == pFunction)
        {
            m_numResetFunctions = --uiCount;
            m_resetFunctions[i] = m_resetFunctions[uiCount];
            return;
        }
    }
}

bool RedRenderer::sLightingEnable = true;
bool RedRenderer::m_bStencilShadowZFail;
bool RedRenderer::m_bScreenSpaceShadows;
float RedRenderer::m_fStencilShadowLength;
int RedRenderer::m_iImmediateMode;
int RedRenderer::m_uiCurMatrixPoolOffset;
unsigned int RedRenderer::m_curRenderItemOffset;
RedCamera* RedRenderer::m_camera;
RedLightingState* RedRenderer::m_pLightingState;
int RedRenderer::m_pmObjWorldPrev;
int RedRenderer::m_pmBoneMatricesPrev;
int RedRenderer::m_pPrimPrev;
int RedRenderer::m_pLightingStatePrev;

// GLOBAL: SWBF2 0x008bef50
// SYNTHETIC: SWBF2 0x00407590
// `dynamic initializer for 'RedRenderer::m_amMatrixPool''

PblMatrix RedRenderer::m_amMatrixPool[3062];

// SYNTHETIC: SWBF2 0x00407610
// `dynamic initializer for 'RedRenderer::m_mView''

__declspec(align(16)) PblMatrix RedRenderer::m_mView;

// GLOBAL: SWBF2 0x008174f0
// SYNTHETIC: SWBF2 0x004076e0
// `dynamic initializer for 'RedRenderer::m_renderItemPool''
// SYNTHETIC: SWBF2 0x00769840
// `dynamic atexit destructor for 'RedRenderer::m_renderItemPool''
RenderItem RedRenderer::m_renderItemPool[4748];

int GetPhasePass_(RenderPhaseT ePhase);
unsigned int GetShaderID_(const pcRedShader* pShader, const RenderItem* pItem, unsigned int uiPass);

 

// FUNCTION: SWBF2 0x006b88f0
void RedRenderer::pcRenderPrimitive(pcRedPrimitive* pPrim, pcRedShader* pShader,
                                    const PblMatrix& mWorld, const RedColor& tweakColor,
                                    unsigned int uiFlags, const PblMatrix* pBoneMatrices,
                                    unsigned int uiNumBoneMatrices,
                                    RedShadingState* pShadingState)
{

    pcRedShader* const pZPrepass = m_systemShaders[1];
    if (!sLightingEnable)
    {
        uiFlags |= 0x10;
    }

    unsigned int count = m_curRenderItemOffset;
    if (count >= 0x128c)
    {
        return;
    }

    unsigned int uiRVF = pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    RenderItem* pItem = &m_renderItemPool[count++];
    m_curRenderItemOffset = count;

    pItem->pVisibilityQuery = 0;
    pItem->pRootShader = pShader;
    pItem->pRootShader->AddRef();
    pItem->uiShaderState = 0;
    pItem->pPrim = pPrim;
    pItem->pPrim->AddRef();

    int iSavedMatrixOffset = m_uiCurMatrixPoolOffset;
    if (!(uiFlags & 0x1000000) &&
        (&mWorld < &m_amMatrixPool[0] || &mWorld >= &m_amMatrixPool[3062]))
    {

        pItem->pmObjWorld = (PblMatrix*)AllocMatrixInCache(mWorld);
    }
    else
    {
        pItem->pmObjWorld = (PblMatrix*)&mWorld;
    }

    pItem->tweakColor = tweakColor;
    pItem->pShadingState = pShadingState;
    if (pItem->pShadingState)
    {
        pItem->pShadingState->AddRef();
    }
    pItem->flags = uiFlags;
    if (m_bStencilShadowZFail)
    {
        pItem->flags = uiFlags | 0x400;
    }
    pItem->pBoneMatrices = (PblMatrix*)pBoneMatrices;
    pItem->numBoneMatrices = (unsigned short)uiNumBoneMatrices;

    unsigned int uiEnvState = 0;
    if (uiFlags & 0x10)
    {
        pItem->pLightingState = (uiRVF & 0x100) ? &m_aLightingStatePool[2048]
                                                : &m_aLightingStatePool[2049];
    }
    else
    {
        RedLightingState* pLightingState = m_pLightingState;
        pItem->pLightingState = pLightingState;
        if (uiRVF & 0x20)
        {
            uiEnvState = 0x10;
            if (pLightingState->pSpotLights[0])
            {
                uiEnvState = 0x14;
                if (pLightingState->pOmniLights[1])
                {
                    uiEnvState = 0x16;
                }
                else if (pLightingState->pOmniLights[0])
                {
                    uiEnvState = 0x15;
                }
            }
            else if (pLightingState->pOmniLights[0])
            {
                if (pLightingState->pOmniLights[2])
                {
                    uiEnvState = 0x13;
                }
                else
                {
                    uiEnvState = pLightingState->pOmniLights[1] ? 0x12 : 0x11;
                }
            }
        }
    }
    pItem->uiEnvState = uiEnvState;
    pItem->fStencilShadowLength = m_fStencilShadowLength;
    pItem->uiCurrentShader = 0;
    pItem->uiShaderChainSize = pItem->pRootShader->GetShaderChain(pItem->pShaderChain, 3, pItem);
    for (unsigned short i = 0; i < pItem->uiShaderChainSize; ++i)
    {
        pItem->pShaderChain[i]->AddRef();
    }

    pcRedShader* pCurShader = pItem->pShaderChain[pItem->uiCurrentShader];
    if (!pCurShader)
    {
        return;
    }

    unsigned int uiState = (pCurShader->*(pCurShader->GetStateFunc))(pItem);
    pItem->uiShaderState = uiState;
    if (uiFlags & 0x200)
    {
        internalRenderImmediate(pItem);
        --m_curRenderItemOffset;
        m_uiCurMatrixPoolOffset = iSavedMatrixOffset;
        m_pmBoneMatricesPrev = 0;
        m_pmObjWorldPrev = 0;
        m_pPrimPrev = 0;
        m_pLightingStatePrev = 0;
        return;
    }

    int phase = pCurShader->GetRenderPhase(pItem, 0);
    unsigned int uiPassCaps = pCurShader->GetShaderDef()->m_states[uiState].pass[0].uiPassCaps;
    RenderSortKey key;
    key.id = 0;

    switch (phase)
    {
    case 0x10:
    case 0x20:
    case 2:
    {
        unsigned char low =
            (unsigned char)internalGetTransformAndLightingID(uiRVF, uiEnvState, uiPassCaps);
        key.translucent.material =
            ((pCurShader->GetID(uiState) & 0xff) << 8) | low;
        key.translucent.translucent = 1;

        unsigned int hidden = uiFlags & 0x10000000;
        if (hidden)
        {
            phase = 2;
        }

        if (uiFlags & 0x8000000)
        {
            key.translucent.depth = 0x7ff;
            key.translucent.pass = 2;

            unsigned int uiZState = pItem->pRootShader->GetZPrepassState();
            unsigned int uiZPassCaps =
                pZPrepass->GetShaderDef()->m_states[uiZState].pass[0].uiPassCaps;
            unsigned int copyCount = m_curRenderItemOffset;
            RenderItem* pCopy = &m_renderItemPool[copyCount];
            m_curRenderItemOffset = copyCount + 1;
            *pCopy = *pItem;
            pCopy->pPrim->AddRef();
            if (pCopy->pShadingState)
            {
                pCopy->pShadingState->AddRef();
            }
            pCopy->pRootShader = pItem->pRootShader;
            pCopy->pRootShader->AddRef();
            pCopy->pShaderChain[0] = pZPrepass;
            pCopy->pShaderChain[0]->AddRef();
            pCopy->uiShaderChainSize = 1;
            pCopy->uiCurrentShader = 0;
            pCopy->uiShaderState = uiZState;
            pCopy->flags |= 0x10;
            pCopy->pLightingState = &m_aLightingStatePool[2049];

            unsigned char copyLow =
                (unsigned char)internalGetTransformAndLightingID(uiRVF, uiEnvState, uiZPassCaps);
            RenderSortKey copyKey;
            copyKey.id = 0;
            copyKey.translucent.material =
                ((GetShaderID_(pCopy->pShaderChain[pCopy->uiCurrentShader], pCopy,
                               pCopy->uiShaderState) &
                  0xff)
                 << 8) |
                copyLow;
            copyKey.translucent.depth = 0x7ff;
            copyKey.translucent.pass = 2;
            copyKey.translucent.translucent = 1;

            m_preShadowTransparentItems.Push(pCopy, ~copyKey.id);
        }
        else if ((uiFlags & 0x20000000) &&
                 pItem->pRootShader->SortAsZWritingTransparent())
        {
            if (hidden)
            {
                key.translucent.pass = 1;
            }
        }
        else if (hidden)
        {
            key.translucent.pass = 2;
        }
        else
        {
            key.translucent.pass = 3;
        }
        break;
    }
    default:
    {
        if (phase == 1)
        {
            unsigned int uiItemFlags = pItem->flags;
            if (uiItemFlags & 0x10000000)
            {
                phase = 1;
            }
            else if (uiItemFlags & 8)
            {
                phase = 8;
            }
            else if (m_bScreenSpaceShadows)
            {
                unsigned int uiZState = pItem->pRootShader->GetZPrepassState();
                if (uiZState != (unsigned int)-1)
                {
                    unsigned int uiZPassCaps = pZPrepass->GetPassCaps(uiZState, 0);
                    unsigned char low = (unsigned char)internalGetTransformAndLightingID(
                        uiRVF, uiEnvState, uiZPassCaps);
                    RenderSortKey queryKey;
                    queryKey.id = 0;
                    queryKey.opaque.material =
                        ((GetShaderID_(pZPrepass, pItem, uiZState) & 0xff) << 8) | low;
                    if (m_iImmediateMode == 1)
                    {

                        IDirect3DQuery9* pQuery = internalGetD3DVisibilityQuery();
                        pItem->pVisibilityQuery = pQuery;
                        if (!pQuery)
                        {
                            internalRenderZPrepassProg1(
                                pItem, queryKey.opaque.material);
                        }
                        else
                        {
                            pQuery->Issue(D3DISSUE_BEGIN);
                            internalRenderZPrepassProg1(
                                pItem, queryKey.opaque.material);
                            pItem->pVisibilityQuery->Issue(D3DISSUE_END);
                        }
                    }
                    else
                    {
                        SortHeap* bucket =
                            &m_opaqueQueues._elements[GetPhasePass_(RENDERPHASE_OPAQUE)];
                        if (bucket->_numElements > 0x2000000)
                        {
                            return;
                        }
                        pItem->flags |= 0x1000;
                        bucket->Push(pItem, ~queryKey.id);
                        if (m_iImmediateMode && bucket->_numElements >= m_iImmediateMode)
                        {
                            pcFlush(0x80000001);
                        }
                    }
                    pItem->flags |= 0x20;
                    phase = 8;
                }
            }
            else if (m_iImmediateMode == 1)
            {
                internalRenderImmediate(pItem);
                --m_curRenderItemOffset;
                m_pmBoneMatricesPrev = 0;
                m_pmObjWorldPrev = 0;
                m_pPrimPrev = 0;
                m_pLightingStatePrev = 0;
                m_uiCurMatrixPoolOffset = iSavedMatrixOffset;
                return;
            }
        }

        unsigned char low =
            (unsigned char)internalGetTransformAndLightingID(uiRVF, uiEnvState, uiPassCaps);
        key.opaque.material =
            ((GetShaderID_(pCurShader, pItem, uiState) & 0xff) << 8) | low;

        if (phase != 4 && m_iImmediateMode == 0 &&
            &mWorld != &gMatrixIdentity)
        {
            PblVector3 vCenter = (pPrim->m_bbox.GetMax() + pPrim->m_bbox.GetMin()) * 0.5f;
            PblVector3 vView;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vView, (const D3DXVECTOR3*)&vCenter,
                                   (const D3DXMATRIX*)&mWorld);

            const PblMatrix& mView = m_mView;
            float fDistance = vView.x * mView.m[0][2] + vView.y * mView.m[1][2] +
                              vView.z * mView.m[2][2] + mView.m[3][2];
            RedCamera* pCamera = m_camera;
            float fNear = pCamera->_fNear;
            float fFar = pCamera->_fFar;
            unsigned int uiDepth;
            if (fNear >= fDistance)
            {
                uiDepth = 0;
            }
            else if (fDistance >= fFar)
            {
                uiDepth = 0x1fff;
            }
            else
            {
                uiDepth = (unsigned int)((fDistance - fNear) * 8191.0f / (fFar - fNear));
            }
            key.opaque.depth = uiDepth;
        }
        break;
    }
    }

    int firstIndex = GetPhasePass_(RENDERPHASE_OPAQUE);
    SortHeap* buckets = m_opaqueQueues._elements;
    SortHeap* first = &buckets[firstIndex];
    SortHeap* fourth = &buckets[GetPhasePass_(RENDERPHASE_SHADOW)];
    SortHeap* eighth = &buckets[GetPhasePass_(RENDERPHASE_POSTSHADOW)];

    TransparentHeap<RenderItem*, unsigned int>* list;
    switch (phase)
    {
    case 1:
        if (first->_numElements > 0x2000000)
        {
            return;
        }
        first->Push(pItem, ~key.id);
        if (m_iImmediateMode && first->_numElements >= m_iImmediateMode)
        {
            pcFlush(0x80000001);
        }
        return;
    case 2:
        if (m_preShadowTransparentItems.m_iCount > m_preShadowTransparentItems.m_iMaxSize)
        {
            return;
        }
        list = &m_preShadowTransparentItems;
        break;
    case 4:
        if (fourth->_numElements > 0x2000000)
        {
            return;
        }
        fourth->Push(pItem, ~key.id);
        return;
    case 8:
        if (eighth->_numElements > 0x2000000)
        {
            return;
        }
        eighth->Push(pItem, ~key.id);
        return;
    case 32:
        if (m_posttransparentItems.m_iCount > m_posttransparentItems.m_iMaxSize)
        {
            return;
        }
        list = &m_posttransparentItems;
        break;
    default:
        if (m_transparentItems.m_iCount > m_transparentItems.m_iMaxSize)
        {
            return;
        }
        list = &m_transparentItems;
        break;
    }

    list->Push(pItem, ~key.id);
}

template <class ITEM, class KEY>
TransparentHeap<ITEM, KEY>::TransparentHeap(unsigned int uiMaxSize)
{
    InitNodePool(uiMaxSize);
    globalPass_ = 0;
}

template <class ITEM, class KEY>
TransparentHeap<ITEM, KEY>::~TransparentHeap()
{
    delete[] m_aNodePool;
}

// FUNCTION: SWBF2 0x006b21f0
template <class ITEM, class KEY>
void TransparentHeap<ITEM, KEY>::InitNodePool(unsigned int uiMaxSize)
{
    m_iMaxSize = uiMaxSize;
    delete[] m_aNodePool;
    m_aNodePool = new Node[m_iMaxSize];
    m_pFirstFreeNode = m_aNodePool;

    for (int i = 0; i < m_iMaxSize - 1; ++i)
    {
        m_aNodePool[i].pNext = &m_aNodePool[i + 1];
    }

    m_aNodePool[m_iMaxSize - 1].pNext = 0;
}

// GLOBAL: SWBF2 0x007fbec8
// SYNTHETIC: SWBF2 0x004076b0
// `dynamic initializer for 'RedRenderer::m_preShadowTransparentItems''
// SYNTHETIC: SWBF2 0x00769830
// `dynamic atexit destructor for 'RedRenderer::m_preShadowTransparentItems''
TransparentHeap<RenderItem*, unsigned int> RedRenderer::m_preShadowTransparentItems(100);
// GLOBAL: SWBF2 0x0086ac50
// SYNTHETIC: SWBF2 0x00407710
// `dynamic initializer for 'RedRenderer::m_transparentItems''
// SYNTHETIC: SWBF2 0x00769860
// `dynamic atexit destructor for 'RedRenderer::m_transparentItems''
TransparentHeap<RenderItem*, unsigned int> RedRenderer::m_transparentItems(800);
// GLOBAL: SWBF2 0x0086ac74
// SYNTHETIC: SWBF2 0x00407680
// `dynamic initializer for 'RedRenderer::m_posttransparentItems''
// SYNTHETIC: SWBF2 0x00769820
// `dynamic atexit destructor for 'RedRenderer::m_posttransparentItems''
TransparentHeap<RenderItem*, unsigned int> RedRenderer::m_posttransparentItems(512);

extern HWND hGlobalWnd;

extern bool g_bNoRender;

// GLOBAL: SWBF2 0x007f5948
IDirect3D9* RedRenderer::m_pD3D;
// GLOBAL: SWBF2 0x007f594c
IDirect3DDevice9* RedRenderer::m_pd3dDevice;
// GLOBAL: SWBF2 0x007f5ecc
D3DPRESENT_PARAMETERS RedRenderer::m_d3dpp;
// GLOBAL: SWBF2 0x007f5f28
void(__cdecl* RedRenderer::m_resetFunctions[100])(ResetFlagsT);
// GLOBAL: SWBF2 0x007f60b8
unsigned int RedRenderer::m_numResetFunctions;
// GLOBAL: SWBF2 0x007f5a80
D3DADAPTER_IDENTIFIER9 RedRenderer::m_adaptorIdentifier;
// GLOBAL: SWBF2 0x007f5950
D3DCAPS9 RedRenderer::m_deviceCaps;
// GLOBAL: SWBF2 0x007f5f08
RenderCapsT RedRenderer::m_renderCaps;

// GLOBAL: SWBF2 0x007f5934
PblTimeBase* RedRenderer::m_pTimeBase;

// GLOBAL: SWBF2 0x007de300

RenderOptionsT RedRenderer::m_renderOptions;

// GLOBAL: SWBF2 0x007fa034
RedTexture* RedRenderer::m_buffers[3];
// GLOBAL: SWBF2 0x007fbe78
pcRedShader* RedRenderer::m_systemShaders[6];
// GLOBAL: SWBF2 0x007f5f04
IDirect3DQuery9* RedRenderer::m_pd3dQuery;

// GLOBAL: SWBF2 0x007de2a4
float RedRenderer::m_lightingScale = 1.0f;
// GLOBAL: SWBF2 0x007de2a8
float RedRenderer::m_shadowIntensity = 1.0f;
// GLOBAL: SWBF2 0x007f592c
int RedRenderer::m_frameNumber;
// GLOBAL: SWBF2 0x007f58e7
bool RedRenderer::m_bFogEnable;
// GLOBAL: SWBF2 0x007fa054
int RedRenderer::m_uiCurLightingStatePoolOffset;

// GLOBAL: SWBF2 0x007fbec4
// SYNTHETIC: SWBF2 0x004075b0
// `dynamic initializer for 'RedRenderer::m_colorFog''
RedColor RedRenderer::m_colorFog = RedColor::BLACK;
// GLOBAL: SWBF2 0x0086ac70
RedColor RedRenderer::m_lightingScaleColor;

// SYNTHETIC: SWBF2 0x00408c60
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00408c80
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// FUNCTION: SWBF2 0x006b1a90
void RedRenderer::Init()
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(wndpl);
    GetWindowPlacement(hGlobalWnd, &wndpl);

    bool bMinimized;
    if (wndpl.showCmd == 6 || wndpl.showCmd == 2)
    {
        bMinimized = true;
    }
    else
    {
        bMinimized = false;
        if (wndpl.showCmd == 7)
        {
            bMinimized = true;
        }
    }

    dx::StateManager_DX::CreateInstance(m_pd3dDevice);

    RedShadingState::sMemoryPool.Setup(0x10, RedShadingState::sMemoryPool.mSize,
                                       RedShadingState::sMemoryPool.mGrow);
    RedShadingPose::Entry::sMemoryPool.Setup(0x100, RedShadingPose::Entry::sMemoryPool.mSize,
                                             RedShadingPose::Entry::sMemoryPool.mGrow);
    RedOmniLight::sMemoryPool.Setup(0x80, RedOmniLight::sMemoryPool.mSize,
                                    RedOmniLight::sMemoryPool.mGrow);

    const DWORD dwStyle = m_d3dpp.Windowed ? 0x00C40000 : 0x90000000;
    const UINT uFlags = (m_d3dpp.Windowed || bMinimized) ? SWP_NOZORDER
                                                         : SWP_NOMOVE;

    RECT rc;
    SetRect(&rc, 0, 0, m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
    AdjustWindowRect(&rc, dwStyle, FALSE);

    RECT desktop;
    GetClientRect(GetDesktopWindow(), &desktop);

    const int iWidth = rc.right - rc.left;
    const int iHeight = rc.bottom - rc.top;
    SetWindowPos(hGlobalWnd, 0,
                 (desktop.left - iWidth + desktop.right) / 2,
                 (desktop.top - iHeight + desktop.bottom) / 2,
                 iWidth, iHeight, uFlags);

    HRESULT hr = m_pd3dDevice->Reset(&m_d3dpp);

    while (hr == D3DERR_DEVICELOST)
    {
        Sleep(0);
        hr = m_pd3dDevice->Reset(&m_d3dpp);
    }

    if (hr == D3DERR_DRIVERINTERNALERROR)
    {
        RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL,
                               "Graphics\\PC\\pcRedRenderer.cpp", 1236,
                               "Mar 31 2017", "10:37:01");
        RedWarning::LogMessage("Internal driver error!");
        RedWarning::DialogBoxMessage("Internal driver error!");
        _exit(0);
    }

    if (!bMinimized)
    {
        ShowWindow(hGlobalWnd, SW_SHOWNORMAL);
    }

    if (!g_bNoRender)
    {
        m_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &m_pd3dQuery);
    }
    else
    {
        m_pd3dQuery = 0;
    }

    internalSetupCaps();

    D3DCAPS9 caps;
    m_pd3dDevice->GetDeviceCaps(&caps);

    unsigned int uiPow2Width;
    int iShift = 31;
    for (;;)
    {
        uiPow2Width = 1u << iShift;
        if (uiPow2Width <= m_d3dpp.BackBufferWidth &&
            uiPow2Width <= caps.MaxTextureWidth)
        {
            break;
        }
        --iShift;
    }

    unsigned int uiPow2Height;
    iShift = 31;
    for (;;)
    {
        uiPow2Height = 1u << iShift;
        if (uiPow2Height <= m_d3dpp.BackBufferHeight &&
            uiPow2Height <= caps.MaxTextureHeight)
        {
            break;
        }
        --iShift;
    }

    if (m_renderCaps.nonpow2textures)
    {
        m_buffers[1] = new RedTexture(m_d3dpp.BackBufferWidth,
                                      m_d3dpp.BackBufferHeight,
                                      D3DFMT_A8R8G8B8, 1,
                                      D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, 0);
    }
    else
    {
        m_buffers[1] = new RedTexture(uiPow2Width, uiPow2Height,
                                      D3DFMT_A8R8G8B8, 1,
                                      D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, 0);
    }

    m_buffers[0] = new RedTexture(
        uiPow2Width > 0x200 ? 0x200 : uiPow2Width,
        uiPow2Height > 0x200 ? 0x200 : uiPow2Height,
        D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, 0);

    const unsigned int uiSmallWidth =
        uiPow2Width > 0x100 ? 0x100 : uiPow2Width;
    const unsigned int uiSmallHeight =
        uiPow2Height > 0x100 ? 0x100 : uiPow2Height;
    m_buffers[2] = new RedTexture(uiSmallWidth, uiSmallHeight,
                                  D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                  D3DPOOL_DEFAULT, 0);

    RedRenderer::m_camera = 0;
    RedRenderer::m_frameNumber = 0;
    RedRenderer::m_fStencilShadowLength = 500.0f;
    RedRenderer::m_bStencilShadowZFail = true;
    m_systemShaders[0] = 0;
    m_systemShaders[1] = 0;
    m_systemShaders[2] = 0;
    m_systemShaders[3] = 0;
    RedRenderer::m_bScreenSpaceShadows = false;
    m_systemShaders[4] = 0;
    m_systemShaders[5] = 0;
    RedRenderer::m_iImmediateMode = 0;
    RedRenderer::m_bFogEnable = false;
    RedRenderer::m_colorFog = RedColor::BLACK;

    internalResetState();

    if (!g_bNoRender)
    {
        RedAutoSettings::DoAutoDetection();
    }

    pcRedVertexBuffer::Init();
    pcRedIndexBuffer::Init();
    pcRedShader::Init();

    memset(&RedRenderer::m_aLightingStatePool[2048], 0, sizeof(RedLightingState));
    memset(&RedRenderer::m_aLightingStatePool[2049], 0, sizeof(RedLightingState));
    memset(&RedRenderer::m_aLightingStatePool[2050], 0, sizeof(RedLightingState));

    RedRenderer::m_curRenderItemOffset = 0;
    RedRenderer::m_uiCurMatrixPoolOffset = 0;
    RedRenderer::m_uiCurLightingStatePoolOffset = 0;

    memcpy((unsigned char*)&RedRenderer::m_aLightingStatePool[2048] + 4,
           (const unsigned char*)&RedAmbientLight::BLACK + 4, 0x6C);
    *(void**)((unsigned char*)&RedRenderer::m_aLightingStatePool[2048] + 0x70) =
        &RedDirectionalLight::BLACK;

    memcpy((unsigned char*)&RedRenderer::m_aLightingStatePool[2049] + 4,
           (const unsigned char*)&RedAmbientLight::WHITE + 4, 0x6C);
    *(void**)((unsigned char*)&RedRenderer::m_aLightingStatePool[2049] + 0x70) =
        &RedDirectionalLight::BLACK;

    RedRenderer::m_pLightingState = &RedRenderer::m_aLightingStatePool[2048];

    m_lightingScale = 1.0f;
    m_lightingScaleColor = RedColor::WHITE;
    RedRenderer::m_shadowIntensity = 1.0f;

    m_systemShaders[0] = 0;

    m_systemShaders[1] = pcRedShader::Create(PblHash("zprepass"), 0x1002, 0, 0, 0,
                                             RedColor::WHITE, RedColor::BLACK,
                                             32.0f);
    m_systemShaders[2] = pcRedShader::Create(PblHash("pervertexdiffuselighting"),
                                             0, 0, 0, 0, RedColor::WHITE,
                                             RedColor::BLACK, 32.0f);
    m_systemShaders[3] = pcRedShader::Create(PblHash("perpixeldiffuselighting"),
                                             0, 0, 0, 0, RedColor::WHITE,
                                             RedColor::BLACK, 32.0f);
    m_systemShaders[4] = pcRedShader::Create(PblHash("specularlighting"),
                                             0, 0, 0, 0, RedColor::WHITE,
                                             RedColor::BLACK, 32.0f);
    m_systemShaders[5] = pcRedShader::Create(PblHash("normal"),
                                             0, 0, 0, 0, RedColor::WHITE,
                                             RedColor::BLACK, 32.0f);

    RedRenderer::internalInitOcclusionQuery();
}

// FUNCTION: SWBF2 0x006b2270
void RedRenderer::InitStatic()
{

    if (!g_bNoRender)
    {
        m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    }
    else
    {
        m_pD3D = new FakeD3D;
    }

    if (!m_pD3D)
    {
        RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL,
                               "Graphics\\PC\\pcRedRenderer.cpp", 1065,
                               "Mar 31 2017", "10:37:01");
        RedWarning::LogMessage("Cannot create Direct3D interface!");
        RedWarning::DialogBoxMessage("Cannot create Direct3D interface!");
        _exit(0);
    }

    pcRedDeviceEnumerator::Init(m_pD3D);
    pcRedDeviceEnumerator::Enumerate(8, 8, 24, 8);

    if (!g_bNoRender)
    {
        RedVideoConfig::VidMode_Init(m_pD3D);
    }

    m_renderOptions.textureQuality = TEXTURE_QUALITY_COMPRESSED;
    m_renderOptions.textureDetail = QUALITY_HIGH;
    m_renderOptions.terrainQuality = QUALITY_HIGH;
    m_renderOptions.waterQuality = QUALITY_HIGH;
    m_renderOptions.particleQuality = QUALITY_HIGH;
    m_renderOptions.shadowQuality = QUALITY2_HIGH;
    m_renderOptions.specularQuality = QUALITY2_HIGH;

    m_renderOptions.bumpMapping = true;
    m_renderOptions.motionBlur = true;
    m_renderOptions.distortion = true;
    m_renderOptions.mGammaBrightness = 0.5f;
    m_renderOptions.mGammaContrast = 0.5f;
    m_renderOptions.mNearSceneFadeFactor = 0.5f;
    m_renderOptions.mFarSceneRangeFactor = 0.5f;

    m_numResetFunctions = 0;
    memset(m_resetFunctions, 0, sizeof(m_resetFunctions));

    memset(&m_d3dpp, 0, sizeof(m_d3dpp));
    m_d3dpp.Windowed = TRUE;
    m_d3dpp.BackBufferCount = 1;
    m_d3dpp.BackBufferWidth = 800;
    m_d3dpp.BackBufferHeight = 600;
    m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

    m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    m_d3dpp.EnableAutoDepthStencil = TRUE;
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    m_d3dpp.FullScreen_RefreshRateInHz = 0;
    m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    m_d3dpp.MultiSampleQuality = 0;

    HRESULT hr = D3D_OK;
    if (!g_bNoRender)
    {

        if (m_pD3D->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8,
                                               TRUE, D3DMULTISAMPLE_NONE, 0)
            != D3D_OK)
        {
            m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
            m_d3dpp.MultiSampleQuality = 0;
        }

        D3DCAPS9 caps;
        m_pD3D->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
        DWORD dwBehaviorFlags = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
                                    ? D3DCREATE_HARDWARE_VERTEXPROCESSING
                                    : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

        hr = m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hGlobalWnd, dwBehaviorFlags,
                                  &m_d3dpp, &m_pd3dDevice);

        HRESULT hrIdentifier = m_pD3D->GetAdapterIdentifier(0, 0, &m_adaptorIdentifier);
        if (hrIdentifier != D3D_OK)
        {
            RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL,
                                   "Graphics\\PC\\pcRedRenderer.cpp", 1156,
                                   "Mar 31 2017", "10:37:01");
            RedWarning::LogMessage("Get Adapter Identifier failed! Error: 0x%x",
                                   hrIdentifier);
            RedWarning::DialogBoxMessage("Get Adapter Identifier failed! Error: 0x%x",
                                         hrIdentifier);
            _exit(0);
        }
    }
    else
    {

        m_pd3dDevice = 0;
        m_pd3dDevice = new FakeD3DDevice;
        memset(&m_adaptorIdentifier, 0, sizeof(m_adaptorIdentifier));
    }

    if (!m_pd3dDevice)
    {
        RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL,
                               "Graphics\\PC\\pcRedRenderer.cpp", 1166,
                               "Mar 31 2017", "10:37:01");
        RedWarning::LogMessage("Cannot create Direct3D device! Error: 0x%x", hr);
        RedWarning::DialogBoxMessage("Cannot create Direct3D device! Error: 0x%x", hr);

        _exit(0);
    }
}

// FUNCTION: SWBF2 0x006b2740
void RedRenderer::OpenDynamic()
{
}

// FUNCTION: SWBF2 0x006b2970
void RedRenderer::Read(PblFileChunk& chunk)
{
    if (!g_bNoRender && chunk._iId == 0x52444853)
    {
        pcRedShader::Read(chunk);
    }
}

// FUNCTION: SWBF2 0x006b3480
void RedRenderer::SetCursorImage(RedTexture const* pTexture, unsigned int uiX,
                                unsigned int uiY)
{

    pcSetCursorImage(pTexture, uiX, uiY);
}

extern bool pcForceFixedFunction;

// GLOBAL: SWBF2 0x007fbea8
void(__cdecl* RedRenderer::internalRenderFunc)(RenderItem*, unsigned int, unsigned int);

// FUNCTION: SWBF2 0x006b6f70
void RedRenderer::internalSetupCaps()
{
    m_pd3dDevice->GetDeviceCaps(&m_deviceCaps);

    if (!pcForceFixedFunction
        && m_deviceCaps.VertexShaderVersion >= 0xFFFE0101
        && m_deviceCaps.PixelShaderVersion >= 0xFFFF0101
        && m_deviceCaps.MaxVertexShaderConst >= 0x60)
    {
        internalRenderFunc = internalRenderProg1;
        m_renderCaps.pipeline = PIPELINE_PROG1;
        m_renderCaps.geometry = GEOMETRYMODE_COMPRESSED;
        m_renderCaps.skinning = SKINNING_HARDWARE;
        m_renderCaps.shadowVolume = SHADOWVOLUME_SOFTWARE_SKINNED;

        if (m_deviceCaps.PixelShaderVersion >= 0xFFFF0200)
        {
            m_renderCaps.pipeline = PIPELINE_PROG2;
        }
    }
    else
    {
        internalRenderFunc = internalRenderFixed;
        m_renderCaps.pipeline = PIPELINE_FIXED;
        m_renderCaps.geometry = GEOMETRYMODE_UNCOMPRESSED;
        m_renderCaps.skinning = SKINNING_SOFTWARE;
        m_renderCaps.shadowVolume = SHADOWVOLUME_SOFTWARE;
        RedRenderer::m_bScreenSpaceShadows = false;
    }

    m_renderCaps.textureCaps = TEXTURE_CAPS_COMPRESSED;
    m_renderCaps.cubemaps = ((m_deviceCaps.TextureCaps >> 11) & 1) != 0;
    m_renderCaps.volumemaps = ((m_deviceCaps.TextureCaps >> 13) & 1) != 0;

    if (m_deviceCaps.TextureCaps & D3DPTEXTURECAPS_POW2)
    {
        m_renderCaps.nonpow2textures =
            (TextureNonPow2SupportT)((m_deviceCaps.TextureCaps >> 8) & 1);
    }
    else
    {
        m_renderCaps.nonpow2textures = TEXTURE_NONPOW2_ALWAYS;
    }

    RedVideoConfig::VidMode_ApplyRenderCapOverrides(m_renderCaps);
}

// FUNCTION: SWBF2 0x006b92b0
void RedRenderer::pcSetCursorImage(RedTexture const* pTexture, unsigned int uiX,
                                  unsigned int uiY)
{
    if (!pTexture)
    {
        return;
    }

    IDirect3DSurface9* pSurface = 0;
    ((IDirect3DTexture9*)pTexture->m_pTexture)->GetSurfaceLevel(0, &pSurface);

    if (uiX >= pTexture->m_uiWidth)
    {
        uiX = pTexture->m_uiWidth;
    }
    if (uiY >= pTexture->m_uiHeight)
    {
        uiY = pTexture->m_uiHeight;
    }

    m_pd3dDevice->SetCursorProperties(uiX, uiY, pSurface);

    pTexture->m_pTexture->Release();
}

void __cdecl PblTraceP(const char* Format, ...);

// FUNCTION: SWBF2 0x006b9310
void RedRenderer::pcSetDisplayMode(unsigned int uiWidth, unsigned int uiHeight,
                                   bool bWindowed, bool bVSync,
                                   D3DMULTISAMPLE_TYPE msType, unsigned long ulMSQuality)
{
    m_d3dpp.BackBufferWidth = uiWidth;
    m_d3dpp.BackBufferHeight = uiHeight;
    m_d3dpp.MultiSampleType = msType;
    m_d3dpp.MultiSampleQuality = ulMSQuality;
    m_d3dpp.Windowed = bWindowed;
    m_d3dpp.PresentationInterval =
        bVSync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;

    PblTraceP("pcSetDisplayMode- width: %d hieght: %d windowed: %d mstype: %d "
              "msquality: %d vsync: %d \n",
              uiWidth, uiHeight, bWindowed, msType, ulMSQuality, bVSync);
}

// FUNCTION: SWBF2 0x006b9540
void RedRenderer::pcSetRenderOptions(const RenderOptionsT& rOptions)
{
    m_renderOptions = rOptions;
}

// GLOBAL: SWBF2 0x00771d40
extern const int IS_VALID_RENDERSTATE[210];
const int IS_VALID_RENDERSTATE[210] = {
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0,
    0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// GLOBAL: SWBF2 0x00772088
extern const int IS_VALID_TEXTURESTAGESTATE[33];
const int IS_VALID_TEXTURESTAGESTATE[33] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 0, 1
};

// GLOBAL: SWBF2 0x0077210c
extern const int IS_VALID_SAMPLERSTATE[14];
const int IS_VALID_SAMPLERSTATE[14] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// GLOBAL: SWBF2 0x007de2b1
bool stateManagerEnabled_ = true;

// GLOBAL: SWBF2 0x007fbe9c
int RedRenderer::m_PrevSortID;

// GLOBAL: SWBF2 0x007fbea0
pcRedShader* RedRenderer::m_pPrevShader;
// GLOBAL: SWBF2 0x007fbea4
int RedRenderer::m_uiPrevShaderState;
// GLOBAL: SWBF2 0x007f58e4
bool RedRenderer::m_bSetTransform_View;
// GLOBAL: SWBF2 0x007f58e5
bool RedRenderer::m_bSetTransform_Projection;

// GLOBAL: SWBF2 0x007f5910
float RedRenderer::m_screenOffsetX;
// GLOBAL: SWBF2 0x007f5914
float RedRenderer::m_screenOffsetY;

// GLOBAL: SWBF2 0x007fbd8c
int RedRenderer::m_bIsSetTransformViewUndo;
// GLOBAL: SWBF2 0x007fbe70
int RedRenderer::m_bIsSetTransformProjectionUndo;

// GLOBAL: SWBF2 0x007fbe90
const pcRedVertexShader* RedRenderer::m_curVertexShader;
// GLOBAL: SWBF2 0x007fbe94
const pcRedPixelShader* RedRenderer::m_curPixelShader;

// GLOBAL: SWBF2 0x007fbe98
const pcRedVertexFormat* RedRenderer::m_curFormat;

// GLOBAL: SWBF2 0x008174e0
// SYNTHETIC: SWBF2 0x00407670
// `dynamic initializer for 'RedRenderer::m_opaqueQueues''
// SYNTHETIC: SWBF2 0x00769810
// `dynamic atexit destructor for 'RedRenderer::m_opaqueQueues''
util::Vector<RedRenderer::SortHeap> RedRenderer::m_opaqueQueues;
// GLOBAL: SWBF2 0x007de348 -- .data, and the image's 1,544 bytes there are zero

pcRedVertexShaderConstants RedRenderer::m_VSConstants = { { { { 0.0f, 0.0f, 0.0f, 0.0f } } }, 0x60, 0 };
// GLOBAL: SWBF2 0x007f58f8
D3DVIEWPORT9 RedRenderer::m_viewport;
// GLOBAL: SWBF2 0x007f5928
float RedRenderer::m_fMaxZDepth;
// GLOBAL: SWBF2 0x007f5930
float RedRenderer::m_screenAspectRatio;
// GLOBAL: SWBF2 0x007fa02c
IDirect3DSurface9* RedRenderer::m_pBackBufferSurface;
// GLOBAL: SWBF2 0x007fa030
IDirect3DSurface9* RedRenderer::m_pDepthSurface;
// GLOBAL: SWBF2 0x007fa040
RedTexture* RedRenderer::m_curRenderTarget;
// GLOBAL: SWBF2 0x007fa044
IDirect3DSurface9* RedRenderer::m_curDepthTarget;
// GLOBAL: SWBF2 0x007fa058
unsigned long RedRenderer::m_curRenderStateMode[210];
// GLOBAL: SWBF2 0x007fa3a0
RedTexture* RedRenderer::m_curSetTextures[8];
// GLOBAL: SWBF2 0x007fa3c0
unsigned long RedRenderer::m_curTextureStageState[8][33];
// GLOBAL: SWBF2 0x007fa7e0
unsigned long RedRenderer::m_curSamplerState[16][14];
// GLOBAL: SWBF2 0x007fab60
IDirect3DVertexBuffer9* RedRenderer::m_curStream[16];
// GLOBAL: SWBF2 0x007faba0
int RedRenderer::m_iStateStackFrame;
// GLOBAL: SWBF2 0x007faba4
unsigned int RedRenderer::m_uiNumRenderStateUndo;
// GLOBAL: SWBF2 0x007faba8
RedRenderer::RenderStateUndo RedRenderer::m_RenderStateUndo[128];
// GLOBAL: SWBF2 0x007fafa8
bool RedRenderer::m_bIsRenderStateUndo[210];
// GLOBAL: SWBF2 0x007fb07c
unsigned int RedRenderer::m_uiNumTextureStageStateUndo;
// GLOBAL: SWBF2 0x007fb080
RedRenderer::TextureStageStateUndo RedRenderer::m_TextureStageStateUndo[128];
// GLOBAL: SWBF2 0x007fb680
bool RedRenderer::m_bIsTextureStageStateUndo[8][33];
// GLOBAL: SWBF2 0x007fb788
RedRenderer::SamplerStateUndo RedRenderer::m_SamplerStateUndo[128];
// GLOBAL: SWBF2 0x007fbd88
unsigned int RedRenderer::m_uiNumSamplerStateUndo;
// GLOBAL: SWBF2 0x007fbd90
bool RedRenderer::m_bIsSamplerStateUndo[16][14];
// GLOBAL: SWBF2 0x007f60bc
pcRedFrontT RedRenderer::m_FrontFace;
// GLOBAL: SWBF2 0x007f60c0
D3DCULL RedRenderer::m_frontCull;
// GLOBAL: SWBF2 0x007f60c4
D3DCULL RedRenderer::m_backCull;
// GLOBAL: SWBF2 0x0086ac6c
RedColor RedRenderer::m_clearColor;

// FUNCTION: SWBF2 0x006ad760
template dx::NVStateID* util::TypedArrayAllocator<dx::NVStateID>::Allocate_(int&);
// FUNCTION: SWBF2 0x006af030
template bool util::BaseArray<dx::NVStateID>::Reserve_(int, bool);
// FUNCTION: SWBF2 0x006b0140
template bool util::Vector<dx::NVStateID>::Add(dx::NVStateID const&);
// FUNCTION: SWBF2 0x006b0340
template util::TKeyPair_<RenderItem*, unsigned int>* util::TypedArrayAllocator<util::TKeyPair_<RenderItem*, unsigned int> >::Allocate_(int&);
// FUNCTION: SWBF2 0x006b0390
template RedRenderer::SortHeap* util::TypedArrayAllocator<RedRenderer::SortHeap>::Allocate_(int&);
// FUNCTION: SWBF2 0x006afbe0
template util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >::Vector(util::Vector<util::TKeyPair_<RenderItem*, unsigned int> > const&);
// FUNCTION: SWBF2 0x006b2e30
template bool util::BaseArray<util::TKeyPair_<RenderItem*, unsigned int> >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x006b2f20
template bool util::BaseArray<RedRenderer::SortHeap>::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x006afe10
// util::BaseArray<RedRenderer::SortHeap,util::TNewArrayAlloc<RedRenderer::SortHeap> >::~BaseArray<RedRenderer::SortHeap,util::TNewArrayAlloc<RedRenderer::SortHeap> >
template util::BaseArray<RedRenderer::SortHeap>::~BaseArray();

// GLOBAL: SWBF2 0x007fa008
unsigned int RedRenderer::m_curVisibilityIndex;
// GLOBAL: SWBF2 0x007fa00c

unsigned int sOcclusionQuerySubmitIndex;
// GLOBAL: SWBF2 0x007fa010
unsigned int RedRenderer::m_occlusionQueryFlushOffset;

// GLOBAL: SWBF2 0x007f60c8
IDirect3DQuery9* RedRenderer::m_pVisibility[4048];
// GLOBAL: SWBF2 0x007fa014
pcRedPrimitive* RedRenderer::m_occlusionQueryShapes[4];

// FUNCTION: SWBF2 0x006b4520
IDirect3DQuery9* RedRenderer::internalGetD3DVisibilityQuery()
{
    const unsigned int i = RedRenderer::m_curVisibilityIndex;
    if (i < 2024 ? i == 2023 : i - 2024 >= 2023)
    {
        return 0;
    }
    RedRenderer::m_curVisibilityIndex = i + 1;
    return m_pVisibility[i];
}

// FUNCTION: SWBF2 0x006b4460
void RedRenderer::internalCleanupOcclusionQuery()
{
    RedRenderer::m_curVisibilityIndex = 0;
    for (int i = 0; i < 4048; ++i)
    {
        if (m_pVisibility[i])
        {
            m_pVisibility[i]->Release();
        }
        m_pVisibility[i] = 0;
    }

    sOcclusionQuerySubmitIndex = 0;
    RedRenderer::m_occlusionQueryFlushOffset = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (m_occlusionQueryShapes[i])
        {
            m_occlusionQueryShapes[i]->Release();
        }
        m_occlusionQueryShapes[i] = 0;
    }
}

// FUNCTION: SWBF2 0x006b45c0
void RedRenderer::internalInitOcclusionQuery()
{
    memset(RedRenderer::m_pVisibility, 0, sizeof(RedRenderer::m_pVisibility));

    for (int iQuery = 0; iQuery < 4048; iQuery++)
    {
        RedRenderer::pcGetDevice()->CreateQuery(D3DQUERYTYPE_OCCLUSION,
                                                &RedRenderer::m_pVisibility[iQuery]);
    }

    PblBox bbox;
    bbox._vMin.x = -2.0f;
    bbox._vMin.y = -2.0f;
    bbox._vMin.z = -2.0f;
    bbox._vMax.x = 2.0f;
    bbox._vMax.y = 2.0f;
    bbox._vMax.z = 2.0f;

    RedRenderer::m_curVisibilityIndex = 0;
    sOcclusionQuerySubmitIndex = 0;
    RedRenderer::m_occlusionQueryFlushOffset = 0;

    float fSin;
    float fCos;
    float fSinTheta;
    float fCosTheta;
    unsigned int i;
    float* pVertex;

    RedRenderer::m_occlusionQueryShapes[3] =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1002, 8, 30, 0, true);
    RedRenderer::m_occlusionQueryShapes[3]->m_bbox = bbox;

    pVertex = (float*)RedRenderer::m_occlusionQueryShapes[3]->BeginVertexData();

    for (i = 0; i <= 6; i++)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 6.0f);

        const float fZero = 0.0f;
        pVertex[0] = fCos * fZero;
        pVertex[1] = 0.0f;
        pVertex[2] = fSin * fZero;
        pVertex[3] = fCos;
        pVertex[4] = 1.0f;
        pVertex[5] = fSin;
        pVertex += 6;
    }

    pVertex[0] = fCos;
    pVertex[1] = 1.0f;
    pVertex[2] = fSin;
    pVertex[3] = fCos;
    pVertex[4] = 1.0f;
    pVertex[5] = fSin;
    pVertex += 6;

    for (i = 0; i <= 6; i++)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 6.0f);
        pVertex[0] = fCos;
        pVertex[1] = 1.0f;
        pVertex[2] = fSin;
        pVertex[3] = 0.0f;
        pVertex[4] = 1.0f;
        pVertex[5] = 0.0f;
        pVertex += 6;
    }

    RedRenderer::m_occlusionQueryShapes[3]->EndVertexData();

    RedRenderer::m_occlusionQueryShapes[1] =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1002, 8, 46, 0, true);
    RedRenderer::m_occlusionQueryShapes[1]->m_bbox = bbox;

    pVertex = (float*)RedRenderer::m_occlusionQueryShapes[1]->BeginVertexData();

    for (i = 0; i <= 6; i++)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 6.0f);
        pVertex[0] = 0.0f;
        pVertex[1] = -1.0f;
        pVertex[2] = 0.0f;
        pVertex[3] = fCos;
        pVertex[4] = -1.0f;
        pVertex[5] = fSin;
        pVertex += 6;
    }

    pVertex[0] = fCos;
    pVertex[1] = -1.0f;
    pVertex[2] = fSin;
    pVertex[3] = fCos;
    pVertex[4] = -1.0f;
    pVertex[5] = fSin;
    pVertex += 6;

    for (i = 0; i <= 6; i++)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 6.0f);
        pVertex[0] = fCos;
        pVertex[1] = -1.0f;
        pVertex[2] = fSin;
        pVertex[3] = fCos;
        pVertex[4] = 1.0f;
        pVertex[5] = fSin;
        pVertex += 6;
    }

    pVertex[0] = fCos;
    pVertex[1] = 1.0f;
    pVertex[2] = fSin;
    pVertex[3] = fCos;
    pVertex[4] = 1.0f;
    pVertex[5] = fSin;
    pVertex += 6;

    for (i = 0; i <= 6; i++)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * PblMath::fTwoPi / 6.0f);
        pVertex[0] = fCos;
        pVertex[1] = 1.0f;
        pVertex[2] = fSin;
        pVertex[3] = 0.0f;
        pVertex[4] = 1.0f;
        pVertex[5] = 0.0f;
        pVertex += 6;
    }

    RedRenderer::m_occlusionQueryShapes[1]->EndVertexData();

    RedRenderer::m_occlusionQueryShapes[0] =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1002, 8, 160, 0, true);
    RedRenderer::m_occlusionQueryShapes[0]->m_bbox = bbox;

    pVertex = (float*)RedRenderer::m_occlusionQueryShapes[0]->BeginVertexData();

    for (unsigned int iStack = 0; iStack < 8; iStack++)
    {
        const float fTheta0 = (float)iStack * PblMath::fPi * 0.125f;
        const float fTheta1 = (float)(iStack + 1) * PblMath::fPi * 0.125f;

        float fSinTheta1 = 0.0f;
        float fCosTheta1 = 0.0f;

        for (i = 0; i <= 8; i++)
        {
            PblMath::SinCos(&fSin, &fCos,
                            -((float)i * PblMath::fTwoPi * 0.125f));

            PblMath::SinCos(&fSinTheta, &fCosTheta, fTheta0);
            pVertex[0] = fCos * fSinTheta;
            pVertex[1] = fCosTheta;
            pVertex[2] = fSin * fSinTheta;

            PblMath::SinCos(&fSinTheta, &fCosTheta, fTheta1);
            pVertex[3] = fCos * fSinTheta;
            pVertex[4] = fCosTheta;
            pVertex[5] = fSin * fSinTheta;
            pVertex += 6;

            fSinTheta1 = fSinTheta;
            fCosTheta1 = fCosTheta;
        }

        pVertex[0] = fCos * fSinTheta1;
        pVertex[1] = fCosTheta1;
        pVertex[2] = fSin * fSinTheta1;
        pVertex[3] = fCos * fSinTheta1;
        pVertex[4] = fCosTheta1;
        pVertex[5] = fSin * fSinTheta1;
        pVertex += 6;
    }

    RedRenderer::m_occlusionQueryShapes[0]->EndVertexData();

    static const unsigned int auiCorners[20] = {
        0, 1, 2, 3,
        6, 7, 7, 3,
        5, 1, 4, 0,
        6, 2, 2, 6,
        6, 7, 4, 5
    };

    RedRenderer::m_occlusionQueryShapes[2] =
        pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x1002, 8, 20, 0, true);
    RedRenderer::m_occlusionQueryShapes[2]->m_bbox = bbox;

    pVertex = (float*)RedRenderer::m_occlusionQueryShapes[2]->BeginVertexData();

    for (i = 0; i < 20; i++)
    {
        const unsigned int uiCorner = auiCorners[i];
        pVertex[0] = (uiCorner & 4) ? 1.0f : -1.0f;
        pVertex[1] = (uiCorner & 2) ? 1.0f : -1.0f;
        pVertex[2] = (uiCorner & 1) ? 1.0f : -1.0f;
        pVertex += 3;
    }

    RedRenderer::m_occlusionQueryShapes[2]->EndVertexData();
}

// FUNCTION: SWBF2 0x006b68a0
void RedRenderer::internalReset()
{
    m_pBackBufferSurface->Release();
    m_pDepthSurface->Release();
    if (m_pd3dQuery)
    {
        m_pd3dQuery->Release();
        m_pd3dQuery = 0;
    }
    internalCleanupOcclusionQuery();

    unsigned int i;
    for (i = 0; i < m_numResetFunctions; ++i)
    {
        m_resetFunctions[i](RESETFLAGS_FREE_RESOURCES);
    }

    if (m_pd3dDevice->Reset(&m_d3dpp) == D3DERR_DRIVERINTERNALERROR)
    {
        RedWarning::SetLogData(RedWarning::RED_SEVERITY_FATAL,
                               "Graphics\\PC\\pcRedRenderer.cpp", 1673,
                               "Mar 31 2017", "10:37:01");
        RedWarning::LogMessage("Internal driver error!");
        RedWarning::DialogBoxMessage("Internal driver error!");
        _exit(0);
    }

    if (!g_bNoRender)
    {
        m_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &m_pd3dQuery);
    }
    else
    {
        m_pd3dQuery = 0;
    }

    internalResetState();
    RedRenderer::internalInitOcclusionQuery();

    for (i = 0; i < m_numResetFunctions; ++i)
    {
        m_resetFunctions[i](RESETFLAGS_CREATE_RESOURCES);
    }
}

// FUNCTION: SWBF2 0x006b69b0
void RedRenderer::internalResetState()
{
    dx::StateManager_DX::GetInstance().ResetAndCaptureStates();

    m_opaqueQueues.SetSize(64);

    for (int i = 0; i < 64; i++)
    {

        if (m_opaqueQueues._elements[i].Reserve_(0, true))
        {
            m_opaqueQueues._elements[i]._numElements = 0;
        }
        m_opaqueQueues._elements[i].globalPass_ = i;
    }

    m_transparentItems.Flush();
    m_preShadowTransparentItems.Flush();
    m_posttransparentItems.Flush();

    RedRenderer::m_PrevSortID = 0;
    RedRenderer::m_pPrevShader = 0;
    RedRenderer::m_uiPrevShaderState = 0;
    RedRenderer::m_bSetTransform_Projection = true;
    RedRenderer::m_bSetTransform_View = true;

    RedInterfaceScreen::s_screenFull.SetDimensions(m_d3dpp.BackBufferWidth,
                                                   m_d3dpp.BackBufferHeight);
    m_screenAspectRatio = (float)((double)m_d3dpp.BackBufferHeight
                                  / (double)m_d3dpp.BackBufferWidth);
    RedInterfaceScreen::s_fScreenBottomV = 1.0f;

    RedInterfaceScreen::s_screenFull.SetSafeZone(0.9f);

    for (unsigned long i = 0; i < 0xD2; i++)
    {
        if (IS_VALID_RENDERSTATE[i])
        {
            m_pd3dDevice->GetRenderState((D3DRENDERSTATETYPE)i,
                                         &m_curRenderStateMode[i]);
        }
        else
        {
            m_curRenderStateMode[i] = 0;
        }
    }

    internalStateStackInit();

    if (m_renderCaps.pipeline == PIPELINE_FIXED)
    {
        pcSetRenderState(D3DRS_FOGVERTEXMODE, 3);
        pcSetRenderState(D3DRS_FOGTABLEMODE, 0);
        pcSetRenderState(D3DRS_RANGEFOGENABLE, 0);
        pcSetRenderState(D3DRS_SPECULARENABLE, 0);
    }
    else
    {
        pcSetRenderState(D3DRS_FOGVERTEXMODE, 0);
        pcSetRenderState(D3DRS_FOGTABLEMODE, 0);
        pcSetRenderState(D3DRS_RANGEFOGENABLE, 0);
        pcSetRenderState(D3DRS_SPECULARENABLE, 1);
    }

    pcSetRenderState(D3DRS_FOGENABLE, 0);
    pcSetRenderState(D3DRS_FOGSTART, 0);
    pcSetRenderState(D3DRS_FOGEND, 0);
    pcSetRenderState(D3DRS_LIGHTING, 1);
    pcSetRenderState(D3DRS_SPECULARMATERIALSOURCE, 0);

    m_FrontFace = PCREDFRONT_CW;
    pcSetFrontFace(PCREDFRONT_CCW);
    pcSetCullMode(PCREDCULL_BACK);

    for (int i = 0; i < 8; i++)
    {
        m_curSetTextures[i] = 0;
    }

    for (unsigned long uiStage = 0; uiStage < 8; uiStage++)
    {
        for (unsigned long uiType = 0; uiType < 0x21; uiType++)
        {
            if (IS_VALID_TEXTURESTAGESTATE[uiType])
            {
                m_pd3dDevice->GetTextureStageState(0, (D3DTEXTURESTAGESTATETYPE)uiType,
                                                   &m_curTextureStageState[uiStage][uiType]);
            }
            else
            {
                m_curTextureStageState[uiStage][uiType] = 0;
            }
        }
    }

    for (unsigned long uiSampler = 0; uiSampler < 0x10; uiSampler++)
    {
        for (unsigned long uiType = 0; uiType < 0x0E; uiType++)
        {
            if (IS_VALID_SAMPLERSTATE[uiType])
            {
                m_pd3dDevice->GetSamplerState(uiSampler, (D3DSAMPLERSTATETYPE)uiType,
                                              &m_curSamplerState[uiSampler][uiType]);
            }
            else
            {
                m_curSamplerState[uiSampler][uiType] = 0;
            }
        }
    }

    memset(m_curStream, 0, sizeof(m_curStream));
    memset(m_VSConstants.m_Registers, 0, sizeof(m_VSConstants.m_Registers));

    m_clearColor.b = 0;
    m_clearColor.g = 0;
    m_clearColor.r = 0;
    m_clearColor.a = 0;
    D3DCOLOR dwClear = (D3DCOLOR)(((((unsigned int)m_clearColor.a << 8)
                                    | m_clearColor.r) << 8 | m_clearColor.g) << 8);

    m_VSConstants.m_iMinConstantSet = 0x60;
    m_VSConstants.m_iMaxConstantSet = 0;

    m_pd3dDevice->Clear(0, 0,
                        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
                        dwClear, 1.0f, 0);

    m_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBufferSurface);
    m_pd3dDevice->GetDepthStencilSurface(&m_pDepthSurface);
    m_curDepthTarget = m_pDepthSurface;

    m_viewport.Width = m_d3dpp.BackBufferWidth;
    m_curRenderTarget = 0;
    m_fMaxZDepth = 16777215.0f;
    RedRenderer::m_screenOffsetX = 0.0f;
    RedRenderer::m_screenOffsetY = 0.0f;
    m_viewport.Y = 0;
    m_viewport.X = 0;
    m_viewport.Height = m_d3dpp.BackBufferHeight;
    m_viewport.MinZ = 0.0f;
    m_viewport.MaxZ = 1.0f;

    RedViewport& rNormalized = RedRenderer::m_normalizedViewport;
    rNormalized.fOffsetY = 0.0f;
    rNormalized.fOffsetX = 0.0f;
    rNormalized.fHeight = 1.0f;
    rNormalized.fWidth = 1.0f;
    pcSetViewport(rNormalized);

    RedRenderer::m_pLightingState = &RedRenderer::m_aLightingStatePool[2048];
    RedRenderer::m_curVertexShader = 0;
    RedRenderer::m_curPixelShader = 0;
    RedRenderer::m_curFormat = 0;

    if (m_renderCaps.pipeline != PIPELINE_FIXED)
    {

        pcRedVertexShaderConstantRegister* pRegs = pcLockVertexShaderConstants(0, 2);
        pRegs[0].component[0] = 0.0f;
        pRegs[0].component[1] = 0.5f;
        pRegs[0].component[2] = 1.0f;
        pRegs[0].component[3] = -1.0f;
        pRegs[1].component[0] = 2.0f;
        pRegs[1].component[1] = 0.25f;
        pRegs[1].component[2] = 0.5f;
        pRegs[1].component[3] = 765.0009765625f;
        pcUnlockVertexShaderConstants(pRegs);
    }
}

// FUNCTION: SWBF2 0x006b7230
void RedRenderer::internalStateStackInit()
{
    m_iStateStackFrame = 0;
    m_uiNumRenderStateUndo = 0;
    m_uiNumTextureStageStateUndo = 0;
    m_uiNumSamplerStateUndo = 0;

    memset(m_bIsRenderStateUndo, 0, sizeof(m_bIsRenderStateUndo));
    memset(m_bIsTextureStageStateUndo, 0, sizeof(m_bIsTextureStageStateUndo));
    memset(m_bIsSamplerStateUndo, 0, sizeof(m_bIsSamplerStateUndo));

    RedRenderer::m_bIsSetTransformProjectionUndo = 0;
    RedRenderer::m_bIsSetTransformViewUndo = 0;
}

// FUNCTION: SWBF2 0x006b8870
pcRedVertexShaderConstantRegister* RedRenderer::pcLockVertexShaderConstants(int iFirst,
                                                                            int iCount)
{
    return m_VSConstants.GetRegisterArray(iFirst, iCount);
}

// FUNCTION: SWBF2 0x006b9800
void RedRenderer::pcSetShadowMapTransform(const PblVector4& u, const PblVector4& v,
                                          const PblVector4& w)
{
    pcRedVertexShaderConstantRegister* regs = m_VSConstants.GetRegisterArray(9, 3);
    regs[9].component[0] = u.x;
    regs[9].component[1] = u.y;
    regs[9].component[2] = u.z;
    regs[9].component[3] = u.w;
    regs[10].component[0] = v.x;
    regs[10].component[1] = v.y;
    regs[10].component[2] = v.z;
    regs[10].component[3] = v.w;
    regs[11].component[0] = w.x;
    regs[11].component[1] = w.y;
    regs[11].component[2] = w.z;
    regs[11].component[3] = w.w;
    m_VSConstants.Update();
}

// FUNCTION: SWBF2 0x006b9260
void RedRenderer::pcSetCullMode(pcRedCullT eCull)
{
    if (eCull == PCREDCULL_FRONT)
    {
        pcSetRenderState(D3DRS_CULLMODE, m_frontCull);
    }
    else if (eCull == PCREDCULL_BACK)
    {
        pcSetRenderState(D3DRS_CULLMODE, m_backCull);
    }
    else
    {
        pcSetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    }
}

// FUNCTION: SWBF2 0x006b9420
void RedRenderer::pcSetFrontFace(pcRedFrontT eFront)
{
    if (m_FrontFace == eFront)
    {
        return;
    }

    if (eFront == PCREDFRONT_CW)
    {
        m_frontCull = D3DCULL_CW;
        m_backCull = D3DCULL_CCW;
    }
    else
    {
        m_frontCull = D3DCULL_CCW;
        m_backCull = D3DCULL_CW;
    }

    if (m_curRenderStateMode[D3DRS_CULLMODE] == D3DCULL_CW)
    {
        pcSetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    }
    else if (m_curRenderStateMode[D3DRS_CULLMODE] == D3DCULL_CCW)
    {
        pcSetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    }

    m_FrontFace = eFront;
}

// FUNCTION: SWBF2 0x006b3620
void RedRenderer::SetFogEnable(bool bEnable)
{
    RedRenderer::m_bFogEnable = bEnable;
    pcSetRenderState(D3DRS_FOGENABLE, bEnable);
}

// FUNCTION: SWBF2 0x006b3730
void RedRenderer::SetLightingScale(float fScale)
{
    m_lightingScale = fScale;
    unsigned char uc = (unsigned char)(int)(fScale * 255.0f);
    m_lightingScaleColor.b = uc;
    m_lightingScaleColor.g = uc;
    m_lightingScaleColor.r = uc;
    m_lightingScaleColor.a = 0xFF;
}

// FUNCTION: SWBF2 0x006b9590
void RedRenderer::pcSetRenderState(D3DRENDERSTATETYPE eState, unsigned long ulValue)
{
    unsigned long ulOld = m_curRenderStateMode[eState];
    if (ulValue == ulOld)
    {
        return;
    }

    if (m_iStateStackFrame != 0 && !m_bIsRenderStateUndo[eState])
    {
        unsigned int uiSlot = m_uiNumRenderStateUndo;
        m_bIsRenderStateUndo[eState] = true;
        m_RenderStateUndo[uiSlot].state = eState;
        m_RenderStateUndo[uiSlot].mode = ulOld;
        m_uiNumRenderStateUndo = uiSlot + 1;
    }

    m_curRenderStateMode[eState] = ulValue;

    if (!stateManagerEnabled_)
    {
        m_pd3dDevice->SetRenderState(eState, ulValue);
        return;
    }

    dx::StateManager_DX& rSM = dx::StateManager_DX::GetInstance();
    if (rSM.m_bForce || rSM.m_CurrentState.m_RenderStates[eState] != ulValue)
    {

        rSM.renderStates_.Add(dx::NVStateID(1, eState, 0));
        rSM.m_CurrentState.m_RenderStates[eState] = ulValue;
    }
}

// FUNCTION: SWBF2 0x006b9e00
void RedRenderer::pcSetViewport(const RedViewport& rViewport)
{
    IDirect3DSurface9* pTarget = 0;
    m_pd3dDevice->GetRenderTarget(0, &pTarget);

    D3DSURFACE_DESC desc;
    pTarget->GetDesc(&desc);
    pTarget->Release();

    float fWidth = (float)(double)desc.Width;
    float fHeight = (float)(double)desc.Height;

    m_viewport.X = (unsigned int)(fWidth * rViewport.fOffsetX);
    m_viewport.Y = (unsigned int)(fHeight * rViewport.fOffsetY);
    m_viewport.Width = (unsigned int)(fWidth * rViewport.fWidth);
    m_viewport.Height = (unsigned int)(fHeight * rViewport.fHeight);

    *(RedViewport*)&RedRenderer::m_normalizedViewport = rViewport;

    m_pd3dDevice->SetViewport(&m_viewport);
}

// FUNCTION: SWBF2 0x006b9f00
void RedRenderer::pcUnlockVertexShaderConstants(pcRedVertexShaderConstantRegister* pRegs)
{
    (void)pRegs;
    m_VSConstants.Update();
}

void pcRedVertexShaderConstants::Update()
{
    if (m_iMaxConstantSet >= m_iMinConstantSet)
    {
        RedRenderer::pcGetDevice()->SetVertexShaderConstantF(
            m_iMinConstantSet,
            m_Registers[m_iMinConstantSet].component,
            m_iMaxConstantSet - m_iMinConstantSet + 1);
    }

    m_iMaxConstantSet = 0;
    m_iMinConstantSet = 0x60;
}

// FUNCTION: SWBF2 0x006b8120
void RedRenderer::pcClear(unsigned int uiFlags, RedColor color, float fZ, unsigned int uiStencil)
{
    m_pd3dDevice->Clear(0, 0, uiFlags, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), fZ,
                        uiStencil);
}

// FUNCTION: SWBF2 0x006b86a0
void RedRenderer::pcGetViewportExtents(float& fMinX, float& fMinY, float& fMaxX, float& fMaxY)
{
    fMinX = (float)m_viewport.X;
    fMinY = (float)m_viewport.Y;
    fMaxX = (float)(m_viewport.X + m_viewport.Width);
    fMaxY = (float)(m_viewport.Y + m_viewport.Height);
}

// FUNCTION: SWBF2 0x006b94f0
void RedRenderer::pcSetNearScene(bool bNearScene)
{
    m_bIsNearScene = bNearScene;
}

// FUNCTION: SWBF2 0x006b9500
void RedRenderer::pcSetNearSceneEnd(float fEnd)
{
    m_fNearSceneEnd = fEnd;
}

// FUNCTION: SWBF2 0x006b9520
void RedRenderer::pcSetNearSceneStart(float fStart)
{
    m_fNearSceneStart = fStart;
}

// FUNCTION: SWBF2 0x006b80f0
bool RedRenderer::pcCheckDepthStencilMatch(_D3DFORMAT renderTargetFormat,
                                           _D3DFORMAT depthStencilFormat)
{
    return m_pD3D->CheckDepthStencilMatch(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8,
                                          renderTargetFormat,
                                          depthStencilFormat) == D3D_OK;
}

// FUNCTION: SWBF2 0x006b9250
void RedRenderer::pcSetClearColor(const RedColor& color)
{
    m_clearColor = color;
}

// FUNCTION: SWBF2 0x006b84a0
RedColor RedRenderer::pcGetClearColor()
{
    return m_clearColor;
}

// FUNCTION: SWBF2 0x006afd00
RedLight::RedLight(TypeT eType)
{
    m_uiHideInView = 0;
    m_uiFlags = 0;
    m_eType = eType;
    m_uiProjTexHash = 0;
    m_uiProjTexWrapMode = 3;
    m_uiProjTexBlendMode = 0;
}

// GLOBAL: SWBF2 0x0086ac90
// SYNTHETIC: SWBF2 0x004075e0
// `dynamic initializer for 'RedRenderer::m_mProjection''

PblMatrix RedRenderer::m_mProjection;
// GLOBAL: SWBF2 0x008eed20
// SYNTHETIC: SWBF2 0x00407620
// `dynamic initializer for 'RedRenderer::m_mViewProjection''

PblMatrix RedRenderer::m_mViewProjection;
// GLOBAL: SWBF2 0x008bef10
// SYNTHETIC: SWBF2 0x00407640
// `dynamic initializer for 'RedRenderer::m_nearSceneFadeTexTransform''

PblMatrix RedRenderer::m_nearSceneFadeTexTransform;

// GLOBAL: SWBF2 0x008eed10
// SYNTHETIC: SWBF2 0x00407740
// `dynamic initializer for 'RedRenderer::m_userNearClipPlane''

PblVector4 RedRenderer::m_userNearClipPlane;

// FUNCTION: SWBF2 0x006b81a0
void RedRenderer::pcFFSetTextureTransform(unsigned int uiStage, const PblMatrix& rMatrix)
{
    pcSetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + uiStage), rMatrix);
}

// FUNCTION: SWBF2 0x006b8520
const PblMatrix& RedRenderer::pcGetNearSceneFadeTextureTransform()
{
    return RedRenderer::m_nearSceneFadeTexTransform;
}

// FUNCTION: SWBF2 0x006b9b60
void RedRenderer::pcSetTransform(D3DTRANSFORMSTATETYPE eState, const PblMatrix& rMatrix)
{
    if (eState == D3DTS_VIEW)
    {
        PblMatrix& rView = *(PblMatrix*)&RedRenderer::m_mView;
        if (m_iStateStackFrame > 0)
        {
            if (!RedRenderer::m_bIsSetTransformViewUndo)
            {
                RedRenderer::m_mViewUndo = rView;
                RedRenderer::m_bIsSetTransformViewUndo = 1;
            }
        }
        else
        {
            RedRenderer::m_bSetTransform_View = true;
        }
        rView = rMatrix;
        for (int row = 0; row < 4; ++row)
        {
            rView.m[row][2] = -rView.m[row][2];
        }
    }
    else if (eState == D3DTS_PROJECTION)
    {
        if (m_iStateStackFrame > 0)
        {
            if (!RedRenderer::m_bIsSetTransformProjectionUndo)
            {
                RedRenderer::m_mProjectionUndo = RedRenderer::m_mProjection;
                RedRenderer::m_bIsSetTransformProjectionUndo = 1;
            }
        }
        else
        {
            RedRenderer::m_bSetTransform_Projection = true;
        }
        RedRenderer::m_mProjection = rMatrix;
    }
    if (eState == D3DTS_VIEW || eState == D3DTS_PROJECTION)
    {
        PblMatrix product;
        D3DXMatrixMultiply((D3DXMATRIX*)&product,
                           (const D3DXMATRIX*)&RedRenderer::m_mView,
                           (const D3DXMATRIX*)&RedRenderer::m_mProjection);
        RedRenderer::m_mViewProjection = product;
    }
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetTransform(eState, (const D3DMATRIX*)&rMatrix);
    }
    else
    {
        m_pd3dDevice->SetTransform(eState, (const D3DMATRIX*)&rMatrix);
    }
}

// FUNCTION: SWBF2 0x006b9db0
void RedRenderer::pcSetUserNearClipPlane(const PblPlane& plane)
{
    RedRenderer::m_userNearClipPlane = *(const PblVector4*)&plane;
    if (RedRenderer::m_userNearClipPlaneEnable)
    {
        internalUpdateSceneConstants();
    }
}

// GLOBAL: SWBF2 0x007fa024
bool RedRenderer::m_userNearClipPlaneEnable;

// GLOBAL: SWBF2 0x007f5938
float RedRenderer::m_fNearSceneStart;
// GLOBAL: SWBF2 0x007f593c
float RedRenderer::m_fNearSceneEnd;
// GLOBAL: SWBF2 0x007f58e6
bool RedRenderer::m_bIsNearScene;
// GLOBAL: SWBF2 0x007f5940
float RedRenderer::m_fWorldFogStart;
// GLOBAL: SWBF2 0x007f5944
float RedRenderer::m_fWorldFogEnd;

// FUNCTION: SWBF2 0x006b1380
void RedRenderer::GetFogRange(float& fStart, float& fEnd)
{
    fStart = *(const float*)&m_curRenderStateMode[D3DRS_FOGSTART];
    fEnd = *(const float*)&m_curRenderStateMode[D3DRS_FOGEND];
}

// FUNCTION: SWBF2 0x006b3420
void RedRenderer::SetCamera(const RedCamera* pCamera)
{
    RedRenderer::m_camera = (RedCamera*)pCamera;
    internalUpdateSceneConstants();
}

// FUNCTION: SWBF2 0x006b3810
void pcRedVertexShaderConstantRegister::SetMatrix4x4(const PblMatrix& rMatrix)
{
    for (int i = 0; i < 4; i++)
    {
        this[0].component[i] = rMatrix.m[i][0];
        this[1].component[i] = rMatrix.m[i][1];
        this[2].component[i] = rMatrix.m[i][2];
        this[3].component[i] = rMatrix.m[i][3];
    }
}

// FUNCTION: SWBF2 0x006b3ec0
void dx::StateManager_DX::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix)
{
    if (pMatrix)
    {
        m_DirtyStates.Add(NVStateID(9, State, 0));
        memcpy(m_CurrentState.m_Matrices[State], pMatrix, sizeof(D3DMATRIX));
    }
}

// FUNCTION: SWBF2 0x006b4430
void RedRenderer::internalBeginConstants()
{
    RedRenderer::m_pmBoneMatricesPrev = 0;
    RedRenderer::m_pmObjWorldPrev = 0;
    RedRenderer::m_pPrimPrev = 0;
    RedRenderer::m_pLightingStatePrev = 0;
}

// FUNCTION: SWBF2 0x006b44f0
void RedRenderer::internalEndConstants()
{
    RedRenderer::m_pmBoneMatricesPrev = 0;
    RedRenderer::m_pmObjWorldPrev = 0;
    RedRenderer::m_pPrimPrev = 0;
    RedRenderer::m_pLightingStatePrev = 0;
}

// FUNCTION: SWBF2 0x006b71b0
void RedRenderer::internalShaderBeginEnd()
{
    if (RedRenderer::m_pPrevShader)
    {
        (RedRenderer::m_pPrevShader->*RedRenderer::m_pPrevShader->EndFunc)(RedRenderer::m_uiPrevShaderState, (RedRenderer::m_PrevSortID >> 16) & 3);
        internalStateStackPop();
        RedRenderer::m_pPrevShader->Release();
    }
    RedRenderer::m_pPrevShader = 0;
    RedRenderer::m_uiPrevShaderState = 0;
    RedRenderer::m_PrevSortID = 0;
}

int GetPhasePass_(RenderPhaseT ePhase);

// FUNCTION: SWBF2 0x006b81e0
void RedRenderer::pcFlush(unsigned int uiPhases)
{
    internalShaderBeginEnd();
    if (uiPhases)
    {
        RedRenderer::internalBeginConstants();
    }

    for (int i = 0; i < m_opaqueQueues._numElements; ++i)
    {
        m_opaqueQueues._elements[i].lastkey_ = 0;
    }

    if (uiPhases & RENDERPHASE_OPAQUE)
    {
        int iFirst = GetPhasePass_(RENDERPHASE_OPAQUE);
        if (m_opaqueQueues._elements[iFirst]._numElements)
        {
            int iEnd = GetPhasePass_(RENDERPHASE_SHADOW);
            for (int i = iFirst; i != iEnd; ++i)
            {
                while (m_opaqueQueues._elements[i]._numElements)
                {
                    internalRenderFromHeap(m_opaqueQueues._elements[i]);
                }
            }
            internalShaderBeginEnd();
        }
    }
    if ((uiPhases & RENDERPHASE_PRESHADOW_TRANSPARENT) && !m_preShadowTransparentItems.IsEmpty())
    {
        do
            internalRenderFromHeap(m_preShadowTransparentItems);
        while (!m_preShadowTransparentItems.IsEmpty());
        internalShaderBeginEnd();
    }
    if (uiPhases & RENDERPHASE_SHADOW)
    {
        int iFirst = GetPhasePass_(RENDERPHASE_SHADOW);
        if (m_opaqueQueues._elements[iFirst]._numElements)
        {
            int iEnd = GetPhasePass_(RENDERPHASE_POSTSHADOW);
            for (int i = iFirst; i != iEnd; ++i)
            {
                while (m_opaqueQueues._elements[i]._numElements)
                {
                    internalRenderFromHeap(m_opaqueQueues._elements[i]);
                }
            }
            internalShaderBeginEnd();
        }
    }
    if (uiPhases & RENDERPHASE_POSTSHADOW)
    {
        int iFirst = GetPhasePass_(RENDERPHASE_POSTSHADOW);
        if (m_opaqueQueues._elements[iFirst]._numElements)
        {
            int iEnd = GetPhasePass_(RENDERPHASE_TRANSPARENT);
            for (int i = iFirst; i != iEnd; ++i)
            {
                while (m_opaqueQueues._elements[i]._numElements)
                {
                    internalRenderFromHeap(m_opaqueQueues._elements[i]);
                }
            }
            internalShaderBeginEnd();
        }
    }
    if ((uiPhases & RENDERPHASE_TRANSPARENT) && !m_transparentItems.IsEmpty())
    {
        do
            internalRenderFromHeap(m_transparentItems);
        while (!m_transparentItems.IsEmpty());
        internalShaderBeginEnd();
    }
    if ((uiPhases & RENDERPHASE_POSTTRANSPARENT) && !m_posttransparentItems.IsEmpty())
    {
        do
            internalRenderFromHeap(m_posttransparentItems);
        while (!m_posttransparentItems.IsEmpty());
        internalShaderBeginEnd();
    }

    RedRenderer::internalEndConstants();

    bool bEmpty = m_transparentItems.IsEmpty() && m_posttransparentItems.IsEmpty()
                  && m_preShadowTransparentItems.IsEmpty();
    for (int i = 0; i < m_opaqueQueues._numElements; ++i)
    {
        if (m_opaqueQueues._elements[i]._numElements > 0)
        {
            return;
        }
    }
    if (bEmpty)
    {
        RedRenderer::m_curRenderItemOffset = 0;
        if (!(uiPhases & 0x80000000))
        {
            RedRenderer::m_uiCurMatrixPoolOffset = 0;
            RedRenderer::m_uiCurLightingStatePoolOffset = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006b76e0
void RedRenderer::internalUpdateSceneConstants()
{
    D3DXMATRIX mWorld;
    D3DXMatrixIdentity(&mWorld);
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetTransform(D3DTS_WORLD, &mWorld);
    }
    else
    {
        m_pd3dDevice->SetTransform(D3DTS_WORLD, &mWorld);
    }

    const RedCamera* pCamera = (const RedCamera*)RedRenderer::m_camera;
    PblMatrix& rView = *(PblMatrix*)&RedRenderer::m_mView;
    PblMatrix& rProj = RedRenderer::m_mProjection;
    const PblMatrix& rIdentity = gMatrixIdentity;

    rView = pCamera->GetMatrixInverse();
    RedRenderer::m_bSetTransform_View = false;
    rView.m[0][2] *= -1.0f;
    rView.m[1][2] *= -1.0f;
    rView.m[2][2] *= -1.0f;
    rView.m[3][2] *= -1.0f;

    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    fFar = PROJECTION_FAR_PLANE;
    rProj = rIdentity;
    rProj.m[2][3] = 1.0f;
    rProj.m[3][3] = 0.0f;
    rProj.m[0][0] = fNear * 2.0f / fWidth;
    RedRenderer::m_bSetTransform_Projection = false;
    rProj.m[1][1] = fNear * 2.0f / fHeight;
    rProj.m[2][2] = fFar / (fFar - fNear);
    rProj.m[3][2] = fNear * -fFar / (fFar - fNear);

    if (RedRenderer::m_userNearClipPlaneEnable)
    {
        PblMatrix mViewProj, mClip;
        D3DXMatrixMultiply((D3DXMATRIX*)&mViewProj, (const D3DXMATRIX*)&rView,
                           (const D3DXMATRIX*)&rProj);
        D3DXMatrixInverse((D3DXMATRIX*)&mClip, 0, (const D3DXMATRIX*)&mViewProj);
        mClip.Transpose4x4();

        PblVector4 vPlane = RedRenderer::m_userNearClipPlane;
        D3DXVECTOR4 vIn(vPlane.x, vPlane.y, vPlane.z, vPlane.w);
        D3DXVECTOR4 vClip;
        D3DXVec4Transform(&vClip, &vIn, (const D3DXMATRIX*)&mClip);
        if (vClip.w != 0.0f)
        {
            if (vClip.w > 0.0f)
            {
                vIn = D3DXVECTOR4(-vPlane.x, -vPlane.y, -vPlane.z, -vPlane.w);
                D3DXVECTOR4 vFlipped;
                D3DXVec4Transform(&vFlipped, &vIn, (const D3DXMATRIX*)&mClip);
                vClip = vFlipped;
            }
            PblMatrix mOblique = rIdentity;
            mOblique.m[0][2] = vClip.x;
            mOblique.m[1][2] = vClip.y;
            mOblique.m[2][2] = vClip.z;
            mOblique.m[3][2] = vClip.w;
            D3DXMatrixMultiply((D3DXMATRIX*)&rProj, (const D3DXMATRIX*)&rProj,
                               (const D3DXMATRIX*)&mOblique);
        }
    }

    PblMatrix mOffset = rIdentity;
    mOffset.m[3][0] = 2.0f / (float)m_d3dpp.BackBufferWidth * RedRenderer::m_screenOffsetX;
    mOffset.m[3][1] = -(2.0f / (float)m_d3dpp.BackBufferHeight * RedRenderer::m_screenOffsetY);
    D3DXMatrixMultiply((D3DXMATRIX*)&rProj, (const D3DXMATRIX*)&rProj,
                       (const D3DXMATRIX*)&mOffset);

    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetTransform(D3DTS_VIEW, (const D3DMATRIX*)&rView);
    }
    else
    {
        m_pd3dDevice->SetTransform(D3DTS_VIEW, (const D3DMATRIX*)&rView);
    }
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)&rProj);
    }
    else
    {
        m_pd3dDevice->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)&rProj);
    }

    PblMatrix mViewProj;
    D3DXMatrixMultiply((D3DXMATRIX*)&mViewProj, (const D3DXMATRIX*)&rView,
                       (const D3DXMATRIX*)&rProj);
    RedRenderer::m_mViewProjection = mViewProj;

    if (m_renderCaps.pipeline != PIPELINE_FIXED)
    {
        pcRedVertexShaderConstantRegister* pRegs = pcLockVertexShaderConstants(2, 7);
        pRegs[2].SetMatrix4x4(RedRenderer::m_mViewProjection);

        const PblMatrix& rCamera = ((const RedCamera*)RedRenderer::m_camera)->GetMatrix();
        pRegs[6].component[0] = rCamera.m[3][0];
        pRegs[6].component[1] = rCamera.m[3][1];
        pRegs[6].component[2] = rCamera.m[3][2];
        pRegs[6].component[3] = 1.0f;

        float fFogStart, fFogEnd;
        GetFogRange(fFogStart, fFogEnd);
        if (fFogEnd > fFogStart)
        {
            float fScale = 1.0f / (fFogEnd - fFogStart);
            pRegs[7].component[0] = -fScale;
            pRegs[7].component[1] = fScale * fFogEnd;
        }
        else
        {
            pRegs[7].component[0] = 0.0f;
            pRegs[7].component[1] = 1.0f;
        }

        if (RedRenderer::m_fWorldFogStart > RedRenderer::m_fWorldFogEnd)
        {
            float fScale = 1.0f / (RedRenderer::m_fWorldFogEnd - RedRenderer::m_fWorldFogStart);
            pRegs[7].component[2] = -fScale;
            pRegs[7].component[3] = RedRenderer::m_fWorldFogEnd * fScale;
        }
        else
        {
            pRegs[7].component[2] = 0.0f;
            pRegs[7].component[3] = 1.0f;
        }

        if (m_fNearSceneStart > 0.0f && m_fNearSceneEnd > m_fNearSceneStart)
        {
            float fScale = 1.0f / (m_fNearSceneEnd - m_fNearSceneStart);
            pRegs[8].component[0] = -fScale;
            pRegs[8].component[1] = m_fNearSceneEnd * fScale;
        }
        else
        {
            pRegs[8].component[0] = 0.0f;
            pRegs[8].component[1] = 1.0f;
        }
        pRegs[8].component[2] = RedRenderer::m_lightingScale;
        pRegs[8].component[3] = 1.0f;

        pcUnlockVertexShaderConstants(pRegs);
    }
    else
    {

        float fScale = 0.0f;
        if (m_fNearSceneStart > 0.0f && m_fNearSceneEnd > m_fNearSceneStart)
        {
            fScale = 1.0f / (m_fNearSceneEnd - m_fNearSceneStart);
        }
        PblMatrix& rFade = RedRenderer::m_nearSceneFadeTexTransform;
        memset(&rFade, 0, 2 * sizeof(rFade.m[0]));
        rFade.m[2][0] = fScale;
        rFade.m[2][1] = 0.0f;
        rFade.m[2][2] = 0.0f;
        rFade.m[2][3] = 0.0f;
        rFade.m[3][0] = -(m_fNearSceneStart * fScale);
        rFade.m[3][1] = 0.0f;
        rFade.m[3][2] = 0.0f;
        rFade.m[3][3] = 1.0f;
    }

    RedRenderer::internalBeginConstants();
}

// SYNTHETIC: SWBF2 0x006b0060
// RedAmbientLight::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006afee0
// RedLightingState::~RedLightingState

// SYNTHETIC: SWBF2 0x006b0090
// RedLight::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006b00c0
bool RedAmbientLight::AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const
{
    return true;
}

// FUNCTION: SWBF2 0x006b0c20
void RedAmbientLight::DebugDraw() const
{
}

// FUNCTION: SWBF2 0x006b2570
bool RedAmbientLight::IsVisible(const RedCamera* pCamera) const
{
    return true;
}

// FUNCTION: SWBF2 0x006b4020
bool RedAmbientLight::SphereIntersect(const PblVector3& vCenter, float fRadius) const
{
    return true;
}

// FUNCTION: SWBF2 0x006b16e0
const PblMatrix& RedLight::GetProjectionMatrix() const
{
    return gMatrixIdentity;
}

// FUNCTION: SWBF2 0x006b00d0
template bool util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >::Add(util::TKeyPair_<RenderItem*, unsigned int> const&);

template <class TABLE>
static void ZeroAllVariables_(TABLE& rTable)
{
    for (int i = 0; i < rTable.hashes_.GetSize(); ++i)
    {
        if (rTable.hashes_[i] != 0)
        {
            *rTable.elements_[i] = 0;
        }
    }
}

template <class ALLOC>
void util::VariableSet<ALLOC>::ResetAll()
{
    ZeroAllVariables_(int32Map_);
    ZeroAllVariables_(floatMap_);
    ZeroAllVariables_(boolMap_);
}

// FUNCTION: SWBF2 0x006b3110
template void util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >::ResetAll();

// GLOBAL: SWBF2 0x0086ad10
// SYNTHETIC: SWBF2 0x00407560
// `dynamic initializer for 'RedRenderer::m_aLightingStatePool''
// SYNTHETIC: SWBF2 0x007697f0
// `dynamic atexit destructor for 'RedRenderer::m_aLightingStatePool''
// SYNTHETIC: SWBF2 0x006afd70
// RedLightingState::RedLightingState
RedLightingState RedRenderer::m_aLightingStatePool[2051];

// FUNCTION: SWBF2 0x006b0270
PblMatrix* RedRenderer::AllocBoneMatricesInCache(unsigned int uiCount)
{
    unsigned int uiOffset = (unsigned int)RedRenderer::m_uiCurMatrixPoolOffset;
    PblMatrix* pMatrices = &RedRenderer::m_amMatrixPool[uiOffset];
    if (uiOffset + uiCount >= 3062)
    {
        uiOffset = 0;
    }
    RedRenderer::m_uiCurMatrixPoolOffset = (int)(uiOffset + uiCount);
    return pMatrices;
}

// FUNCTION: SWBF2 0x006b02a0
RedLightingState* RedRenderer::AllocLightingStateInCache()
{
    RedLightingState* pState = &m_aLightingStatePool[RedRenderer::m_uiCurLightingStatePoolOffset];
    RedRenderer::m_uiCurLightingStatePoolOffset = (unsigned int)(RedRenderer::m_uiCurLightingStatePoolOffset + 1) < 2048 ? RedRenderer::m_uiCurLightingStatePoolOffset + 1 : 1;
    memset(pState, 0, sizeof(RedLightingState));
    return pState;
}

// FUNCTION: SWBF2 0x006b02e0
const PblMatrix* RedRenderer::AllocMatrixInCache(const PblMatrix& mat)
{
    PblMatrix* pMatrix = &RedRenderer::m_amMatrixPool[RedRenderer::m_uiCurMatrixPoolOffset];
    if ((unsigned int)(RedRenderer::m_uiCurMatrixPoolOffset + 1) >= 3062)
    {
        RedRenderer::m_uiCurMatrixPoolOffset = 0;
    }
    RedRenderer::m_uiCurMatrixPoolOffset = RedRenderer::m_uiCurMatrixPoolOffset + 1;
    *pMatrix = mat;
    return pMatrix;
}

// GLOBAL: SWBF2 0x007de2b0

bool sWaitForFrameQuery = true;
// GLOBAL: SWBF2 0x007de2b2

bool sEnableOcclusionQueries = true;

// FUNCTION: SWBF2 0x006b03e0
void RedRenderer::BeginFrame()
{
    if (sWaitForFrameQuery && m_pd3dQuery)
    {
        BOOL bData;
        while (m_pd3dQuery->GetData(&bData, sizeof(bData), D3DGETDATA_FLUSH) == S_FALSE)
        {
            Sleep(0);
        }
    }

    if (hGlobalWnd == GetFocus() && m_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
    {
        internalReset();
    }

    D3DCOLOR clearColor = (((((unsigned int)m_clearColor.a << 8) | m_clearColor.r) << 8
                            | m_clearColor.g) << 8) | m_clearColor.b;
    m_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
                        clearColor, 1.0f, 0);

    ++RedRenderer::m_frameNumber;
    RedRenderer::m_occlusionQueryFlushOffset = 0;
    RedRenderer::m_curVisibilityIndex = (RedRenderer::m_frameNumber & 1) ? 0x7E8 : 0;
    sOcclusionQuerySubmitIndex = (RedRenderer::m_frameNumber & 1) ? 700 : 0;

    static util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >& s_perFrame =
        util::Globals::GetInstance().GetSet("perFrame");
    s_perFrame.ResetAll();
}

// FUNCTION: SWBF2 0x006b0560
void RedRenderer::BeginScene()
{
    m_pd3dDevice->BeginScene();
    RedRenderer::m_curRenderItemOffset = 0;
    RedRenderer::m_uiCurMatrixPoolOffset = 0;
    RedRenderer::m_uiCurLightingStatePoolOffset = 0;
    internalUpdateSceneConstants();
}

// FUNCTION: SWBF2 0x006b0600
void RedRenderer::Cleanup()
{
    internalCleanupOcclusionQuery();

    for (int i = 0; i < MAX_SYSTEM_SHADER; ++i)
    {
        if (m_systemShaders[i])
        {
            m_systemShaders[i]->Release();
        }
        m_systemShaders[i] = 0;
    }

    if (m_pd3dQuery)
    {
        m_pd3dQuery->Release();
    }
    m_pd3dQuery = 0;

    pcRedShader::Cleanup();
    pcRedVertexBuffer::Cleanup();
    pcRedIndexBuffer::Cleanup();
    pcRedVertexFormat::Cleanup();

    RedRenderer::m_curPixelShader = 0;
    RedRenderer::m_curVertexShader = 0;
    RedRenderer::m_curFormat = 0;

    if (m_pBackBufferSurface)
    {
        m_pBackBufferSurface->Release();
    }
    m_pBackBufferSurface = 0;
    if (m_pDepthSurface)
    {
        m_pDepthSurface->Release();
    }
    m_pDepthSurface = 0;

    for (int i = 0; i < 3; ++i)
    {
        delete m_buffers[i];
        m_buffers[i] = 0;
    }

    RedRenderer::m_camera = 0;
    RedRenderer::m_frameNumber = 0;

    for (int i = 0; i < 64; ++i)
    {
        if (m_opaqueQueues._elements[i].Reserve_(0, true))
        {
            m_opaqueQueues._elements[i]._numElements = 0;
        }
    }
    if (m_opaqueQueues.Reserve_(0, true))
    {
        m_opaqueQueues._numElements = 0;
    }

    m_transparentItems.Flush();
    m_preShadowTransparentItems.Flush();
    m_posttransparentItems.Flush();

    memset(m_VSConstants.m_Registers, 0, sizeof(m_VSConstants.m_Registers));
    m_VSConstants.m_iMinConstantSet = 96;
    m_VSConstants.m_iMaxConstantSet = 0;

    RedRenderer::m_PrevSortID = 0;
    RedRenderer::m_pPrevShader = 0;
    RedRenderer::m_uiPrevShaderState = 0;
    RedRenderer::m_iImmediateMode = 0;

    memset(&RedRenderer::m_aLightingStatePool[2048], 0, sizeof(RedLightingState));
    memset(&RedRenderer::m_aLightingStatePool[2049], 0, sizeof(RedLightingState));
    memset(&RedRenderer::m_aLightingStatePool[2050], 0, sizeof(RedLightingState));
    RedRenderer::m_pLightingState = &RedRenderer::m_aLightingStatePool[2048];

    if (dx::localInstance_)
    {
        delete dx::localInstance_;
    }
    dx::localInstance_ = 0;
}

// FUNCTION: SWBF2 0x006b08f0
void RedRenderer::CleanupStatic()
{
    if (m_pd3dDevice)
    {
        m_pd3dDevice->Release();
    }
    m_pd3dDevice = 0;
    if (m_pD3D)
    {
        m_pD3D->Release();
    }
    m_pD3D = 0;
}

// FUNCTION: SWBF2 0x006b09e0
IDirect3D9* RedRenderer::CreateFakeD3D()
{
    return new FakeD3D;
}

// FUNCTION: SWBF2 0x006b0a00
IDirect3DDevice9* RedRenderer::CreateFakeDevice()
{
    return new FakeD3DDevice;
}

// FUNCTION: SWBF2 0x006b0ca0
void RedRenderer::EndFrame()
{
    pcRedVertexBuffer::FlushAllDynamicVertexBuffers();
    pcRedIndexBuffer::FlushAllDynamicIndexBuffers();
    FlushOcclusionQueries();
    HRESULT hr = m_pd3dDevice->Present(0, 0, 0, 0);
    if (m_pd3dQuery && hr != D3DERR_DEVICELOST)
    {
        m_pd3dQuery->Issue(D3DISSUE_END);
    }
}

// FUNCTION: SWBF2 0x006b0cf0
void RedRenderer::EndScene()
{
    Flush();
    pcRedVertexBuffer::FlushAllDynamicVertexBuffers();
    m_pd3dDevice->EndScene();
}

// FUNCTION: SWBF2 0x006b0d50
void RedRenderer::Flush()
{
    pcFlush(RENDERPHASE_ALL);
    RedRenderer::m_pLightingState = &RedRenderer::m_aLightingStatePool[2048];
}

// GLOBAL: SWBF2 0x007fb07a
bool bSortMaterial;

// FUNCTION: SWBF2 0x006b18a0
unsigned int GetShaderID_(const pcRedShader* pShader, const RenderItem* pItem, unsigned int uiPass)
{
    if (pShader->m_material && bSortMaterial)
    {
        return (unsigned int)const_cast<pcRedShader*>(pShader)->GetDiffuseMapHash(
            (int)pItem, (int)uiPass);
    }
    return pShader->GetID(uiPass);
}

struct OcclusionQueryT
{
    PblMatrix worldMat;
    OcclusionQueryShapeT shape;
    IDirect3DQuery9* pVisibilityQuery;
};
 

// SYNTHETIC: SWBF2 0x006afc80
// OcclusionQueryT::OcclusionQueryT
// GLOBAL: SWBF2 0x007fbf60
// SYNTHETIC: SWBF2 0x00407650
// `dynamic initializer for 'RedRenderer::m_occlusionQueries''

OcclusionQueryT RedRenderer::m_occlusionQueries[1400];

// FUNCTION: SWBF2 0x006b0d70
void RedRenderer::FlushOcclusionQueries()
{
    if (g_bNoRender)
    {
        return;
    }

    pcRedShader* pShader = m_systemShaders[SYSTEM_SHADER_ZPREPASS];
    unsigned int uiCaps = pShader->GetPassCaps(0, 0);

    RenderItem* pItem = &m_renderItemPool[RedRenderer::m_curRenderItemOffset++];
    pItem->pBoneMatrices = 0;
    pItem->numBoneMatrices = 0;
    pItem->pShadingState = 0;
    pItem->pRootShader = m_systemShaders[SYSTEM_SHADER_NORMAL];
    pItem->pShaderChain[0] = pShader;
    pItem->uiShaderChainSize = 1;
    pItem->uiCurrentShader = 0;
    pItem->uiShaderState = 0;
    pItem->flags = 48;
    pItem->pLightingState = &RedRenderer::m_aLightingStatePool[2049];
    pItem->uiEnvState = 0;
    pItem->pVisibilityQuery = 0;

    unsigned char ucTL = (unsigned char)internalGetTransformAndLightingID(0x1002, 0, uiCaps);
    unsigned short usID = (unsigned short)(((GetShaderID_(pShader, pItem, 0) & 0xff) << 8) | ucTL);

    unsigned int uiBase = sOcclusionQuerySubmitIndex < 700 ? 0 : 700;
    for (unsigned int i = uiBase + RedRenderer::m_occlusionQueryFlushOffset; i < sOcclusionQuerySubmitIndex; ++i)
    {
        OcclusionQueryT& rQuery = m_occlusionQueries[i];
        pItem->pmObjWorld = &rQuery.worldMat;
        pItem->pPrim = m_occlusionQueryShapes[rQuery.shape];
        rQuery.pVisibilityQuery->Issue(D3DISSUE_BEGIN);
        internalRenderZPrepassProg1(pItem, usID);
        rQuery.pVisibilityQuery->Issue(D3DISSUE_END);
    }
    --RedRenderer::m_curRenderItemOffset;
    RedRenderer::m_occlusionQueryFlushOffset = sOcclusionQuerySubmitIndex - uiBase;
}

// FUNCTION: SWBF2 0x006b0ed0
void RenderItem::Free()
{
    if (pPrim)
    {
        pPrim->Release();
    }
    pPrim = 0;
    if (pShadingState)
    {
        pShadingState->Release();
    }
    pShadingState = 0;
    if (pRootShader)
    {
        pRootShader->Release();
    }
    pRootShader = 0;
    for (unsigned short i = 0; i < uiShaderChainSize; ++i)
    {
        if (pShaderChain[i])
        {
            pShaderChain[i]->Release();
        }
        pShaderChain[i] = 0;
    }
}

// FUNCTION: SWBF2 0x006afef0
RenderItem::~RenderItem()
{
    Free();
}

// FUNCTION: SWBF2 0x006afe00
RenderItem::RenderItem()
{
}

// FUNCTION: SWBF2 0x006b1130
const RedCamera* RedRenderer::GetCamera()
{
    return (const RedCamera*)RedRenderer::m_camera;
}

// FUNCTION: SWBF2 0x006b11c0
const RedLightingState& RedRenderer::GetDefaultLightingState()
{
    return RedRenderer::m_aLightingStatePool[2050];
}

// FUNCTION: SWBF2 0x006b1340
RedColor RedRenderer::GetFogColor()
{
    return RedRenderer::m_colorFog;
}

// FUNCTION: SWBF2 0x006b1370
bool RedRenderer::GetFogEnable()
{
    return RedRenderer::m_bFogEnable;
}

// FUNCTION: SWBF2 0x006b13b0
int RedRenderer::GetFrameNumber()
{
    return RedRenderer::m_frameNumber;
}

// FUNCTION: SWBF2 0x006b14d0
bool RedRenderer::GetLightingEnable()
{
    return RedRenderer::sLightingEnable;
}

// FUNCTION: SWBF2 0x006b14e0
float RedRenderer::GetLightingScale()
{
    return m_lightingScale;
}

// FUNCTION: SWBF2 0x006b1520
bool RedRenderer::GetOcclusionQueryPixelCount(unsigned int uiQuery, unsigned int& uiPixelCount)
{
    if (!g_bNoRender)
    {
        return false;
    }
    return m_occlusionQueries[uiQuery].pVisibilityQuery->GetData(&uiPixelCount, sizeof(uiPixelCount), 0)
           == S_OK;
}

// FUNCTION: SWBF2 0x006b1570
unsigned int pcRedShader::GetPassCaps(unsigned int uiState, unsigned int uiPass) const
{
    return GetShaderDef()->m_states[uiState].pass[uiPass].uiPassCaps;
}

// FUNCTION: SWBF2 0x006b15a0
int GetPhasePass_(RenderPhaseT ePhase)
{
    switch (ePhase)
    {
    case RENDERPHASE_OPAQUE:
        return 0;
    case RENDERPHASE_SHADOW:
        return 4;
    case RENDERPHASE_POSTSHADOW:
        return 6;
    }
    return ePhase == RENDERPHASE_TRANSPARENT ? 20 : 0;
}

// GLOBAL: SWBF2 0x0086acd0
// SYNTHETIC: SWBF2 0x004075f0
// `dynamic initializer for 'RedRenderer::m_mProjectionTemp''

PblMatrix RedRenderer::m_mProjectionTemp;

// FUNCTION: SWBF2 0x006b16f0
const PblMatrix& RedRenderer::GetProjectionMatrix()
{
    if (RedRenderer::m_bSetTransform_Projection || RedRenderer::m_bIsSetTransformProjectionUndo)
    {
        return RedRenderer::m_mProjection;
    }

    float fNear, fFar, fWidth, fHeight;
    ((const RedCamera*)RedRenderer::m_camera)->GetFrustum(fNear, fFar, fWidth, fHeight);

    PblMatrix& rProj = RedRenderer::m_mProjectionTemp;
    rProj = gMatrixIdentity;
    rProj.m[2][3] = -1.0f;
    rProj.m[3][3] = 0.0f;
    rProj.m[0][0] = fNear * 2.0f / fWidth;
    rProj.m[1][1] = fNear * -2.0f / fHeight;
    rProj.m[3][2] = fNear * -PROJECTION_FAR_PLANE / (PROJECTION_FAR_PLANE - fNear);
    rProj.m[2][2] = -PROJECTION_FAR_PLANE / (PROJECTION_FAR_PLANE - fNear);
    return rProj;
}

// FUNCTION: SWBF2 0x006b18d0
float RedRenderer::GetShadowIntensity()
{
    return RedRenderer::m_shadowIntensity;
}

// FUNCTION: SWBF2 0x006b18f0
float RedRenderer::GetStencilShadowLength()
{
    return RedRenderer::m_fStencilShadowLength;
}

// FUNCTION: SWBF2 0x006b1a60
void RedRenderer::GetWorldFogRange(float& fStart, float& fEnd)
{
    fStart = RedRenderer::m_fWorldFogStart;
    fEnd = RedRenderer::m_fWorldFogEnd;
}

// FUNCTION: SWBF2 0x006b2750
void RedRenderer::PostLoad()
{
    RedLight::PostLoad();
}

template <class ITEM, class KEY>
void TransparentHeap<ITEM, KEY>::Push(ITEM& rItem, KEY idSort)
{
    Node* pPrev = 0;
    Node* pNext;
    for (pNext = m_pFirstNode; pNext; pNext = pNext->pNext)
    {
        if ((pNext->idSort & 0xffff0000) < (idSort & 0xffff0000))
        {
            break;
        }
        pPrev = pNext;
    }

    Node* pNode = AllocNode();
    if (pNode)
    {
        pNode->idSort = idSort;
        pNode->Item = rItem;
        pNode->pNext = pNext;
        if (pPrev)
        {
            pPrev->pNext = pNode;
            ++m_iCount;
        }
        else
        {
            ++m_iCount;
            m_pFirstNode = pNode;
        }
    }
}

// FUNCTION: SWBF2 0x006b2820
template void TransparentHeap<RenderItem*, unsigned int>::Push(RenderItem*&, unsigned int);

// FUNCTION: SWBF2 0x006b33c0
void RedRenderer::SetAmbientFilter(bool bEnable)
{
}

// FUNCTION: SWBF2 0x006b33d0
void RedRenderer::SetAmbientIntensity(int iIntensity)
{
}

// FUNCTION: SWBF2 0x006b33e0
void RedRenderer::SetAmbientThreshold(int iThreshold)
{
}

// FUNCTION: SWBF2 0x006b3460
void RedRenderer::SetColorFilter(bool bEnable)
{
}

// FUNCTION: SWBF2 0x006b3500
void RedRenderer::SetFilterColor(RedColor color, int iAmount)
{
}

// FUNCTION: SWBF2 0x006b3510
void RedRenderer::SetFogColor(const RedColor& color)
{
    RedRenderer::m_colorFog = color;
    RedColor c = m_lightingScaleColor * RedRenderer::m_colorFog;
    pcSetRenderState(D3DRS_FOGCOLOR,
                     (((((unsigned int)c.a << 8) | c.r) << 8 | c.g) << 8) | c.b);
}

// FUNCTION: SWBF2 0x006b3640
void RedRenderer::SetFogRange(float fStart, float fEnd)
{
    pcSetRenderState(D3DRS_FOGSTART, *(unsigned long*)&fStart);
    pcSetRenderState(D3DRS_FOGEND, *(unsigned long*)&fEnd);
}

// FUNCTION: SWBF2 0x006b3850
void RedRenderer::SetMultiplicativeFog(int iPower)
{
    unsigned char ucGrey = (unsigned char)(int)(m_lightingScale * 255.0f);
    RedColor grey;
    grey.b = ucGrey;
    grey.g = ucGrey;
    grey.r = ucGrey;
    grey.a = ucGrey;
    RedColor c = grey * RedRenderer::m_colorFog;

    float fExp = 1.0f / (float)iPower;
    float fR = (float)pow((double)(c.r * (1.0f / 255.0f)), (double)fExp);
    float fG = (float)pow((double)(c.g * (1.0f / 255.0f)), (double)fExp);
    float fB = (float)pow((double)(c.b * (1.0f / 255.0f)), (double)fExp);
    float fA = (float)pow((double)(c.a * (1.0f / 255.0f)), (double)fExp);

    pcSetRenderState(D3DRS_FOGCOLOR,
                     ((((((unsigned int)(unsigned char)(int)(fA * 255.0f) << 8)
                         | (unsigned char)(int)(fR * 255.0f)) << 8)
                       | (unsigned char)(int)(fG * 255.0f)) << 8)
                         | (unsigned char)(int)(fB * 255.0f));
}

// FUNCTION: SWBF2 0x006b3c60
void RedRenderer::SetScreenOffset(float fX, float fY)
{
    RedRenderer::m_screenOffsetX = fX;
    RedRenderer::m_screenOffsetY = fY;
}

// FUNCTION: SWBF2 0x006b3c80
void RedRenderer::SetShadowIntensity(float fIntensity)
{
    RedRenderer::m_shadowIntensity = fIntensity;
}

// FUNCTION: SWBF2 0x006b3cb0
void RedRenderer::SetSoftnessEnable(bool bEnable)
{
}

// FUNCTION: SWBF2 0x006b3cc0
void RedRenderer::SetSoftnessValue(int iValue)
{
}

// FUNCTION: SWBF2 0x006b3ce0
void RedRenderer::SetStencilShadowLength(float fLength)
{
    RedRenderer::m_fStencilShadowLength = fLength;
}

// FUNCTION: SWBF2 0x006b3d00
void RedRenderer::SetStencilShadowZFail(bool bZFail)
{
    RedRenderer::m_bStencilShadowZFail = bZFail;
}

// FUNCTION: SWBF2 0x006b3ff0
void RedRenderer::SetWorldFogRange(float fStart, float fEnd)
{
    RedRenderer::m_fWorldFogStart = fStart;
    RedRenderer::m_fWorldFogEnd = fEnd;
}

// FUNCTION: SWBF2 0x006b37c0
void pcRedVertexShaderConstantRegister::SetMatrix4x3(const PblMatrix& rMatrix)
{
    for (int i = 0; i < 4; i++)
    {
        this[0].component[i] = rMatrix.m[i][0];
        this[1].component[i] = rMatrix.m[i][1];
        this[2].component[i] = rMatrix.m[i][2];
    }
}

// FUNCTION: SWBF2 0x006b3680
void dx::StateManager_DX::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
    if (m_bForce || m_CurrentState.m_IndexData.m_pIndexData != pIndexData
        || m_CurrentState.m_IndexData.m_BaseVertexIndex != 0)
    {
        m_DirtyStates.Add(NVStateID(13, 0, 0));
        m_CurrentState.m_IndexData.m_pIndexData = pIndexData;
        m_CurrentState.m_IndexData.m_BaseVertexIndex = 0;
    }
}

// FUNCTION: SWBF2 0x006b3a70
void dx::StateManager_DX::SetPixelShader(IDirect3DPixelShader9* pShader)
{
    if (m_bForce || m_CurrentState.m_PixelShader != pShader)
    {
        m_DirtyStates.Add(NVStateID(8, 0, 0));
        m_CurrentState.m_PixelShader = pShader;
    }
}

// FUNCTION: SWBF2 0x006b3be0
void dx::StateManager_DX::SetSamplerState(unsigned long ulSampler, D3DSAMPLERSTATETYPE eType,
                                          unsigned long ulValue)
{
    if (m_bForce || m_CurrentState.m_SamplerStates[ulSampler][eType] != ulValue)
    {
        samplerStates_.Add(NVStateID(4, ulSampler, eType));
        m_CurrentState.m_SamplerStates[ulSampler][eType] = ulValue;
    }
}

// FUNCTION: SWBF2 0x006b3d20
void dx::StateManager_DX::SetStreamSource(unsigned int uiStream, IDirect3DVertexBuffer9* pStreamData,
                                          unsigned int uiOffset, unsigned int uiStride)
{
    NVStreamData& rData = m_CurrentState.m_StreamData[uiStream];
    if (m_bForce || rData.m_pStreamData != pStreamData || rData.m_Stride != 0)
    {
        m_DirtyStates.Add(NVStateID(12, uiStream, 0));
        rData.m_pStreamData = pStreamData;
        rData.m_Offset = uiOffset;
        rData.m_Stride = uiStride;
    }
}

// FUNCTION: SWBF2 0x006b3dc0
bool dx::StateManager_DX::SetTexture(unsigned long ulStage, IDirect3DBaseTexture9* pTexture)
{
    if (!m_bForce && m_CurrentState.m_Textures[ulStage] == pTexture)
    {
        return false;
    }
    textureStates_.Add(NVStateID(2, ulStage, 0));
    m_CurrentState.m_Textures[ulStage] = pTexture;
    return true;
}

// FUNCTION: SWBF2 0x006b3e30
void dx::StateManager_DX::SetTextureStageState(unsigned long ulStage, D3DTEXTURESTAGESTATETYPE eType,
                                               unsigned long ulValue)
{
    if (m_bForce || m_CurrentState.m_TextureStates[ulStage][eType] != ulValue)
    {
        textureStageStates_.Add(NVStateID(3, ulStage, eType));
        m_CurrentState.m_TextureStates[ulStage][eType] = ulValue;
    }
}

// FUNCTION: SWBF2 0x006b3f60
void dx::StateManager_DX::SetVertexShader(IDirect3DVertexShader9* pShader)
{
    if (m_bForce || m_CurrentState.m_VertexShader != pShader)
    {
        m_DirtyStates.Add(NVStateID(5, 0, 0));
        m_CurrentState.m_VertexShader = pShader;
    }
}

// FUNCTION: SWBF2 0x006b4040
unsigned int RedRenderer::SubmitOcclusionQuery(const PblMatrix& mWorld, OcclusionQueryShapeT eShape,
                                               const PblVector3& vScale, bool)
{
    if (!g_bNoRender || !sEnableOcclusionQueries)
    {
        return (unsigned int)-1;
    }

    unsigned int uiIndex = sOcclusionQuerySubmitIndex;
    if (uiIndex < 700)
    {
        if (uiIndex == 699)
        {
            return (unsigned int)-1;
        }
    }
    else if (uiIndex - 700 >= 699)
    {
        return (unsigned int)-1;
    }

    OcclusionQueryT& rQuery = m_occlusionQueries[uiIndex];
    rQuery.pVisibilityQuery = internalGetD3DVisibilityQuery();
    if (!rQuery.pVisibilityQuery)
    {
        return (unsigned int)-1;
    }

    rQuery.worldMat = mWorld;
    rQuery.worldMat.m[0][0] = vScale.x * rQuery.worldMat.m[0][0];
    rQuery.worldMat.m[1][1] = vScale.y * rQuery.worldMat.m[1][1];
    rQuery.worldMat.m[2][2] = vScale.z * rQuery.worldMat.m[2][2];
    sOcclusionQuerySubmitIndex = uiIndex + 1;
    rQuery.shape = eShape;
    return uiIndex;
}

// FUNCTION: SWBF2 0x006b41b0
const PblVector3 RedRenderer::TransformCameraPointToProjectionSpace(const PblVector3& vPoint)
{
    return ((const RedCamera*)RedRenderer::m_camera)->TransformCameraPointToProjectionSpace(vPoint);
}

// FUNCTION: SWBF2 0x006b41d0
const PblVector3 RedRenderer::TransformProjectionPointToScreenSpace(const PblVector3& vPoint)
{
    return PblVector3((vPoint.x + 1.0f) * 0.5f * (float)m_viewport.Width + (float)m_viewport.X,
                      (vPoint.y + 1.0f) * 0.5f * (float)m_viewport.Height + (float)m_viewport.Y,
                      vPoint.z * m_viewport.MaxZ);
}

// FUNCTION: SWBF2 0x006b42b0
const PblVector3 RedRenderer::TransformWorldPointToProjectionSpace(const PblVector3& vPoint)
{
    const RedCamera* pCamera = (const RedCamera*)RedRenderer::m_camera;
    PblVector3 vCamera;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCamera, (const D3DXVECTOR3*)&vPoint,
                           (const D3DXMATRIX*)&pCamera->_MatrixInverse);
    return pCamera->TransformCameraPointToProjectionSpace(vCamera);
}

// GLOBAL: SWBF2 0x007de2a3

bool sFlushVertexConstantsOnShaderChange = true;

// GLOBAL: SWBF2 0x007fa027

bool sCurrentFormatHasNormals;

// GLOBAL: SWBF2 0x007fbebc

unsigned int sAccumulatedLightingBits;

// FUNCTION: SWBF2 0x006b4560
unsigned int RedRenderer::internalGetTransformAndLightingID(unsigned int uiRVF,
                                                            unsigned int uiEnvState,
                                                            unsigned int uiPassCaps)
{
    unsigned int uiID = 0;
    if (uiPassCaps & 0x10)
    {
        return uiID;
    }

    unsigned int uiWeights = (uiRVF >> 2) & 7;
    if (uiWeights > 1)
    {
        uiID = 0x40;
    }
    else if (uiWeights)
    {
        uiID = 0x40;
    }

    if ((uiPassCaps & 0x100) && (uiRVF & 0x180))
    {
        uiID |= 0x80;
    }
    if (uiPassCaps & 6)
    {
        uiID |= uiEnvState & 0x1f;
    }
    return uiID;
}

// FUNCTION: SWBF2 0x006b5560
void RedRenderer::internalRenderImmediate(RenderItem* pItem)
{
    unsigned int uiRVF = pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    for (; pItem->uiCurrentShader < pItem->uiShaderChainSize; ++pItem->uiCurrentShader)
    {
        if (pItem->uiCurrentShader)
        {
            pcRedShader* pNext = pItem->pShaderChain[pItem->uiCurrentShader];
            pItem->uiShaderState = (pNext->*pNext->GetStateFunc)(pItem);
        }

        unsigned int uiState = pItem->uiShaderState;
        unsigned int uiCaps = pItem->pShaderChain[pItem->uiCurrentShader]
                                  ->GetShaderDef()->m_states[uiState].pass[0].uiPassCaps;
        SortIdT sortId;
        sortId.id = 0;
        sortId.o_customID = GetShaderID_(pItem->pShaderChain[pItem->uiCurrentShader], pItem,
                                         pItem->uiShaderState);
        sortId.o_tnl = internalGetTransformAndLightingID(uiRVF, pItem->uiEnvState, uiCaps);
        internalRenderFunc(pItem, sortId.o_shaderID, sortId.o_pass);

        while (sortId.o_pass + 1 < pItem->pShaderChain[pItem->uiCurrentShader]
                                       ->GetShaderDef()->m_states[pItem->uiShaderState].uiNumPasses)
        {
            ++sortId.o_pass;
            const pcRedShaderDef::StateT& rState =
                pItem->pShaderChain[pItem->uiCurrentShader]
                    ->GetShaderDef()->m_states[pItem->uiShaderState];
            sortId.o_tnl = internalGetTransformAndLightingID(
                uiRVF, pItem->uiEnvState, rState.pass[sortId.o_pass].uiPassCaps);
            internalRenderFunc(pItem, sortId.o_shaderID, sortId.o_pass);
        }
    }
    pItem->Free();
}

namespace
{

inline void SetRGBW_(pcRedVertexShaderConstantRegister& rReg, const RedColorValue& rColor, float fW)
{
    rReg.component[0] = rColor.r;
    rReg.component[1] = rColor.g;
    rReg.component[2] = rColor.b;
    rReg.component[3] = fW;
}

inline void SetColor_(pcRedVertexShaderConstantRegister& rReg, const RedColorValue& rColor)
{
    rReg.component[0] = rColor.r;
    rReg.component[1] = rColor.g;
    rReg.component[2] = rColor.b;
    rReg.component[3] = rColor.a;
}

inline void SetXYZ_(pcRedVertexShaderConstantRegister& rReg, const PblVector3& rV)
{
    rReg.component[0] = rV.x;
    rReg.component[1] = rV.y;
    rReg.component[2] = rV.z;
}

inline RedColorValue ScaledColor_(const RedLight* pLight, float fFactor)
{
    RedColorValue c;
    c.r = pLight->m_color.r * fFactor;
    c.g = pLight->m_color.g * fFactor;
    c.b = pLight->m_color.b * fFactor;
    c.a = pLight->m_color.a * fFactor;
    return c;
}

inline bool LightFlag0_(const RedLight* pLight) { return (pLight->m_uiFlags & 1) != 0; }
inline float LightFlag1_(const RedLight* pLight) { return (float)(int)((pLight->m_uiFlags >> 1) & 1); }
}

static unsigned int ToD3DColor_(const RedColorValue& c)
{
    return ((unsigned int)(unsigned char)(int)(c.a * 255.0f) << 24) |
           ((unsigned int)(unsigned char)(int)(c.r * 255.0f) << 16) |
           ((unsigned int)(unsigned char)(int)(c.g * 255.0f) << 8) |
           (unsigned int)(unsigned char)(int)(c.b * 255.0f);
}

static float Saturate_(float f)
{
    if (f < 0.0f)
    {
        return 0.0f;
    }
    if (f > 1.0f)
    {
        return 1.0f;
    }
    return f;
}

static void SetScaledLight_(unsigned int uiIndex, const D3DLIGHT9& rSource, float fScale,
                            bool bSaturate)
{
    D3DLIGHT9 light = rSource;
    RedColorValue color;
    color.r = light.Diffuse.r * fScale;
    color.g = light.Diffuse.g * fScale;
    color.b = light.Diffuse.b * fScale;
    if (bSaturate)
    {
        color.r = Saturate_(color.r);
        color.g = Saturate_(color.g);
        color.b = Saturate_(color.b);
    }
    color.a = 1.0f;
    light.Specular = *(const D3DCOLORVALUE*)&color;
    light.Diffuse = *(const D3DCOLORVALUE*)&color;
    RedRenderer::pcGetDevice()->SetLight(uiIndex, &light);
    RedRenderer::pcGetDevice()->LightEnable(uiIndex, TRUE);
}

// FUNCTION: SWBF2 0x006b4d90
void __cdecl RedRenderer::internalRenderFixed(RenderItem* pItem, unsigned int uiShaderID,
                                              unsigned int uiPass)
{
    const unsigned int uiRVF = pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    const unsigned int uiPassCaps = pItem->pShaderChain[pItem->uiCurrentShader]
                                        ->GetShaderDef()
                                        ->m_states[pItem->uiShaderState]
                                        .pass[uiPass]
                                        .uiPassCaps;
    const unsigned int uiTnl = uiShaderID & 0xff;
    unsigned int uiNumLights = 0;
    internalShaderBeginEnd(pItem, uiShaderID, uiPass);

    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetTransform(D3DTS_WORLD,
                                                         (const D3DMATRIX*)pItem->pmObjWorld);
    }
    else
    {
        m_pd3dDevice->SetTransform(D3DTS_WORLD, (const D3DMATRIX*)pItem->pmObjWorld);
    }
    pcSetRenderState(D3DRS_LIGHTING, TRUE);

    if ((uiTnl & 0x1f) && !(pItem->flags & 0x10) && (uiPassCaps & 6))
    {
        const RedLightingState* pLS = pItem->pLightingState;
        const bool bNormals = (uiRVF & 0x100) != 0;
        const bool bAmbient = !bNormals && (uiPassCaps & 2);
        const bool bDirectional = pLS->pDirectionalLights[0] && (!bNormals || (uiPassCaps & 4));
        if (bDirectional || pLS->pOmniLights[0])
        {
            const PblMatrix mWorldInverse = pItem->pmObjWorld->GetInverseFast();
        }

        if (bAmbient)
        {
            pcSetRenderState(D3DRS_AMBIENT,
                             ToD3DColor_(pLS->ambientLight.m_color));
        }
        else
        {
            pcSetRenderState(D3DRS_AMBIENT, *(const unsigned int*)&RedColor::BLACK);
        }

        if (bDirectional)
        {
            float fFactor = pLS->pDirectionalFactor[0];
            if (pItem->flags & 0x800)
            {
                fFactor *= 0.5f;
            }
            SetScaledLight_(0, pLS->pDirectionalLights[0]->m_d3dLight, fFactor, true);
            uiNumLights = 1;
        }

        if (pLS->pOmniLights[0])
        {
            if (pItem->flags & 0x800)
            {
                SetScaledLight_(uiNumLights++, pLS->pOmniLights[0]->m_d3dLight, 0.5f, false);
                if (pLS->pOmniLights[1])
                {
                    SetScaledLight_(uiNumLights++, pLS->pOmniLights[1]->m_d3dLight, 0.5f, false);
                    if (pLS->pOmniLights[2])
                    {
                        SetScaledLight_(uiNumLights++, pLS->pOmniLights[2]->m_d3dLight, 0.5f, false);

                        if (pLS->pOmniLights[3])
                        {
                            SetScaledLight_(uiNumLights++, pLS->pOmniLights[0]->m_d3dLight, 0.5f,
                                            false);
                        }
                    }
                }
            }
            else
            {
                for (unsigned int i = 0; i < 4 && pLS->pOmniLights[i]; ++i)
                {
                    m_pd3dDevice->SetLight(uiNumLights,
                                           &pLS->pOmniLights[i]->m_d3dLight);
                    m_pd3dDevice->LightEnable(uiNumLights, TRUE);
                    ++uiNumLights;
                }
            }
        }
    }
    else
    {
        pcSetRenderState(D3DRS_AMBIENT, *(const unsigned int*)&RedColor::WHITE);
    }

    pcRedShader* pShader;
    if (pItem->flags & 0x100)
    {
        pcRedFrontT eFront = m_FrontFace;
        if (eFront == PCREDFRONT_CCW)
        {
            pcSetFrontFace(PCREDFRONT_CW);
        }
        else if (eFront == PCREDFRONT_CW)
        {
            pcSetFrontFace(PCREDFRONT_CCW);
        }
        pShader = pItem->pShaderChain[pItem->uiCurrentShader];
        (pShader->*pShader->RenderFunc)(pItem, uiPass);
        pcSetFrontFace(eFront);
    }
    else
    {
        pShader = pItem->pShaderChain[pItem->uiCurrentShader];
        (pShader->*pShader->RenderFunc)(pItem, uiPass);
    }

    for (unsigned int i = 0; i < uiNumLights; ++i)
    {
        m_pd3dDevice->LightEnable(i, FALSE);
    }
}

// FUNCTION: SWBF2 0x006b5700
void __cdecl RedRenderer::internalRenderProg1(RenderItem* pItem, unsigned int uiShaderID,
                                              unsigned int uiPass)
{
    unsigned int uiRVF = pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    unsigned int uiTnl = uiShaderID & 0xff;
    internalShaderBeginEnd(pItem, uiShaderID, uiPass);

    pcRedVertexShaderConstantRegister* pRegs;
    if (pItem->pBoneMatrices && (int)pItem->pBoneMatrices != RedRenderer::m_pmBoneMatricesPrev)
    {
        RedRenderer::m_pmBoneMatricesPrev = (int)pItem->pBoneMatrices;
        pRegs = m_VSConstants.GetRegisterArray(51, 3 * pItem->numBoneMatrices);
        for (unsigned short i = 0; i < pItem->numBoneMatrices; ++i)
        {
            pRegs[51 + 3 * i].SetMatrix4x3(pItem->pBoneMatrices[i]);
        }
        m_VSConstants.Update();
    }

    float fSpotW = 2.0f;
    float fSpotProjected = 0.0f;
    if ((int)pItem->pPrim != RedRenderer::m_pPrimPrev)
    {
        RedRenderer::m_pPrimPrev = (int)pItem->pPrim;
        pRegs = m_VSConstants.GetRegisterArray(12, 4);
        if (uiRVF & 0x4000)
        {
            pRegs[12].component[0] = 2.0f;
            pRegs[12].component[1] = -1.0f;
        }
        else
        {
            pRegs[12].component[0] = 1.0f;
            pRegs[12].component[1] = 0.0f;
        }
        pRegs[12].component[2] = (uiRVF & 0x8000) ? (1.0f / 2048.0f) : 1.0f;
        pRegs[12].component[3] = 1.0f;

        if (uiRVF & 0x1000)
        {
            const PblBox& rBox = pItem->pPrim->m_bbox;
            const float fScale = 1.0f / 65534.0f;
            pRegs[13].component[3] = 0.0f;
            pRegs[13].component[0] = (rBox._vMax.x - rBox._vMin.x) * fScale;
            pRegs[13].component[1] = (rBox._vMax.y - rBox._vMin.y) * fScale;
            pRegs[13].component[2] = (rBox._vMax.z - rBox._vMin.z) * fScale;
            pRegs[14].component[3] = 1.0f;
            pRegs[14].component[0] = (rBox._vMax.x + rBox._vMin.x) * 0.5f;
            pRegs[14].component[1] = (rBox._vMax.y + rBox._vMin.y) * 0.5f;
            pRegs[14].component[2] = (rBox._vMax.z + rBox._vMin.z) * 0.5f;
        }
        else
        {
            pRegs[13].component[0] = 1.0f;
            pRegs[13].component[1] = 1.0f;
            pRegs[13].component[2] = 1.0f;
            pRegs[13].component[3] = 0.0f;
            pRegs[14].component[0] = 0.0f;
            pRegs[14].component[1] = 0.0f;
            pRegs[14].component[2] = 0.0f;
            pRegs[14].component[3] = 1.0f;
        }

        if (uiRVF & 0x80)
        {
            pRegs[15].component[0] = 0.0f;
            pRegs[15].component[1] = 1.0f;
        }
        else
        {
            pRegs[15].component[0] = 1.0f;
            pRegs[15].component[1] = 0.0f;
        }
        pRegs[15].component[2] = 0.0f;
        pRegs[15].component[3] = 1.0f;
    }

    if ((int)pItem->pmObjWorld != RedRenderer::m_pmObjWorldPrev)
    {
        RedRenderer::m_pmObjWorldPrev = (int)pItem->pmObjWorld;
        pRegs = m_VSConstants.GetRegisterArray(16, 3);
        PblMatrix mWorld = *pItem->pmObjWorld;
        pRegs[16].SetMatrix4x3(mWorld);
    }

    bool bNormals = ((uiRVF >> 8) & 1) != 0;
    unsigned int uiLights = uiTnl & 0x1f;
    if (uiLights && !(pItem->flags & 0x10))
    {
        const RedLightingState* pLS = pItem->pLightingState;
        sAccumulatedLightingBits |= uiLights;
        pRegs = m_VSConstants.GetRegisterArray(19, 6);
        RedRenderer::m_pLightingStatePrev = (int)pLS;
        sCurrentFormatHasNormals = bNormals;
        float fDirProjected = 0.0f;
        float fOmniProjected = 0.0f;
        const RedColorValue black = RedColorValue::BLACK;

        const RedDirectionalLight* pDir0 = pLS->pDirectionalLights[0];
        const RedDirectionalLight* pDir1 = 0;
        if (!bNormals)
        {
            const RedAmbientLight& rAmbient = pLS->ambientLight;
            SetColor_(pRegs[19], rAmbient.m_topColor);
            SetColor_(pRegs[20], rAmbient.m_bottomColor);
            if (pDir0)
            {
                const float fFactor = pLS->pDirectionalFactor[0];
                SetRGBW_(pRegs[21], ScaledColor_(pDir0, fFactor), LightFlag1_(pDir0));
                SetXYZ_(pRegs[22], pDir0->m_vDirection);
                fDirProjected = (float)(int)((const RedLight*)pDir0 ==
                                             pItem->pLightingState->pProjectedLight);
                pDir1 = pLS->pDirectionalLights[1];
                if (pDir1)
                {
                    SetRGBW_(pRegs[23], ScaledColor_(pDir1, pLS->pDirectionalFactor[1]),
                             LightFlag1_(pDir1));
                }
            }
        }
        else
        {
            SetColor_(pRegs[19], black);
            SetColor_(pRegs[20], black);
            if (pDir0)
            {
                RedColorValue color = LightFlag0_(pDir0)
                                          ? black
                                          : ScaledColor_(pDir0, pLS->pDirectionalFactor[0]);
                SetRGBW_(pRegs[21], color, LightFlag1_(pDir0) * pLS->pDirectionalFactor[0]);
                SetXYZ_(pRegs[22], pDir0->m_vDirection);
                fDirProjected = (float)(int)((const RedLight*)pDir0 ==
                                             pItem->pLightingState->pProjectedLight);
                pDir1 = pLS->pDirectionalLights[1];
                if (pDir1)
                {

                    color = LightFlag0_(pDir0) ? black
                                               : ScaledColor_(pDir1, pLS->pDirectionalFactor[1]);
                    SetRGBW_(pRegs[23], color, LightFlag1_(pDir1) * pLS->pDirectionalFactor[1]);
                }
            }
        }
        if (!pDir0)
        {
            SetRGBW_(pRegs[21], black, 0.0f);
        }
        if (pDir1)
        {
            SetXYZ_(pRegs[24], pDir1->m_vDirection);
        }
        else
        {
            SetRGBW_(pRegs[23], black, 0.0f);
        }

        unsigned int uiOmni = uiTnl & 3;
        if (uiOmni)
        {
            if (uiOmni == 3)
            {
                pRegs = m_VSConstants.GetRegisterArray(25, 8);
                const RedOmniLight* pOmni3 = pLS->pOmniLights[3];
                if (pOmni3)
                {
                    SetRGBW_(pRegs[31], pOmni3->m_color, LightFlag1_(pOmni3));
                    SetXYZ_(pRegs[32], pOmni3->m_pos);
                    pRegs[32].component[3] = pOmni3->m_fRadiusInvSq;
                }
                else
                {
                    SetRGBW_(pRegs[31], black, 0.0f);
                    pRegs[32].component[0] = 0.0f;
                    pRegs[32].component[1] = 0.0f;
                    pRegs[32].component[2] = 0.0f;
                    pRegs[32].component[3] = 1.0f;
                }
            }
            else
            {
                pRegs = m_VSConstants.GetRegisterArray(25, 2 * uiOmni);
            }

            switch (uiOmni)
            {
            case 3:
            {
                const RedOmniLight* pOmni = pLS->pOmniLights[2];
                SetRGBW_(pRegs[29], pOmni->m_color, LightFlag1_(pOmni));
                SetXYZ_(pRegs[30], pOmni->m_pos);
                pRegs[30].component[3] = pOmni->m_fRadiusInvSq;
            }

            case 2:
            {
                const RedOmniLight* pOmni = pLS->pOmniLights[1];
                SetRGBW_(pRegs[27], pOmni->m_color, LightFlag1_(pOmni));
                SetXYZ_(pRegs[28], pOmni->m_pos);
                pRegs[28].component[3] = pOmni->m_fRadiusInvSq;
            }

            case 1:
            {
                const RedOmniLight* pOmni = pLS->pOmniLights[0];
                SetRGBW_(pRegs[25], pOmni->m_color, LightFlag1_(pOmni));
                SetXYZ_(pRegs[26], pOmni->m_pos);
                pRegs[26].component[3] = pOmni->m_fRadiusInvSq;
                fOmniProjected = (float)(int)((const RedLight*)pOmni ==
                                              pItem->pLightingState->pProjectedLight);
            }
            }
        }

        unsigned int uiSpot = uiTnl & 0xc;
        if (uiSpot)
        {
            if (uiSpot == 4)
            {
                pRegs = m_VSConstants.GetRegisterArray(29, 4);
                const RedSpotLight* pSpot = pLS->pSpotLights[0];
                if (pSpot)
                {
                    SetRGBW_(pRegs[29], pSpot->m_color, LightFlag1_(pSpot));
                    SetXYZ_(pRegs[30], pSpot->m_vPos);
                    pRegs[30].component[3] = pSpot->m_fRadiusInvSq;
                    if (!pSpot->m_bBidirectional)
                    {
                        fSpotW = 0.0f;
                    }
                    SetXYZ_(pRegs[31], pSpot->m_vDirection);
                    pRegs[31].component[3] = fSpotW;
                    pRegs[32].component[1] = pSpot->m_fCosHalfInnerConeAngle;
                    pRegs[32].component[0] = pSpot->m_fCosHalfOuterConeAngle;
                    pRegs[32].component[3] = pSpot->m_fFalloff;
                    pRegs[32].component[2] = 1.0f / (pSpot->m_fCosHalfInnerConeAngle -
                                                     pSpot->m_fCosHalfOuterConeAngle);
                    fSpotProjected = (float)(int)((const RedLight*)pSpot ==
                                                  pItem->pLightingState->pProjectedLight);
                }
                else
                {
                    SetRGBW_(pRegs[29], black, 0.0f);
                }
            }
        }

        if (pItem->pLightingState->pProjectedLight)
        {
            pRegs = m_VSConstants.GetRegisterArray(33, 6);
            pRegs[35].SetMatrix4x4(pItem->pLightingState->pProjectedLight->GetProjectionMatrix());
            pRegs[34].component[0] = fDirProjected;
            pRegs[34].component[1] = fOmniProjected;
            pRegs[34].component[2] = fSpotProjected;
            pRegs[34].component[3] = 0.0f;
            const RedLightingState* pState = pItem->pLightingState;
            const RedLight* pProjected = pState->pProjectedLight;
            if (pProjected->m_eType == RedLight::TYPE_DIRECTIONAL)
            {
                SetColor_(pRegs[33], ScaledColor_(pProjected, pState->pDirectionalFactor[0]));
            }
            else
            {
                SetColor_(pRegs[33], pProjected->m_color);
            }
        }
        else
        {
            SetColor_(pRegs[33], black);
            pRegs = m_VSConstants.GetRegisterArray(33, 1);
        }
    }
    else
    {
        const RedLightingState* pDefault =
            bNormals ? &RedRenderer::m_aLightingStatePool[2048]
                     : &RedRenderer::m_aLightingStatePool[2049];
        if ((int)pDefault != RedRenderer::m_pLightingStatePrev)
        {
            RedRenderer::m_pLightingStatePrev = (int)pDefault;
            pRegs = m_VSConstants.GetRegisterArray(19, 2);
            const RedAmbientLight& rAmbient = pDefault->ambientLight;
            SetColor_(pRegs[19], rAmbient.m_topColor);
            SetColor_(pRegs[20], rAmbient.m_bottomColor);
        }
        sCurrentFormatHasNormals = bNormals;
    }

    if (sFlushVertexConstantsOnShaderChange)
    {
        m_VSConstants.Update();
    }

    pcRedShader* pShader;
    if (pItem->flags & 0x100)
    {
        pcRedFrontT eFront = m_FrontFace;
        if (eFront == PCREDFRONT_CCW)
        {
            pcSetFrontFace(PCREDFRONT_CW);
        }
        else if (eFront == PCREDFRONT_CW)
        {
            pcSetFrontFace(PCREDFRONT_CCW);
        }
        pShader = pItem->pShaderChain[pItem->uiCurrentShader];
        (pShader->*pShader->RenderFunc)(pItem, uiPass);
        pcSetFrontFace(eFront);
    }
    else
    {
        pShader = pItem->pShaderChain[pItem->uiCurrentShader];
        (pShader->*pShader->RenderFunc)(pItem, uiPass);
    }
}

// FUNCTION: SWBF2 0x006b6500
void RedRenderer::internalRenderZPrepassProg1(RenderItem* pItem, unsigned int uiShaderID)
{
    const unsigned int uiRVF = pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF;
    const unsigned short iShader = pItem->uiCurrentShader;
    pcRedShader* pShader = pItem->pShaderChain[iShader];
    const unsigned int uiState = pItem->uiShaderState;

    pItem->pShaderChain[iShader] = m_systemShaders[SYSTEM_SHADER_ZPREPASS];
    pItem->uiShaderState = pItem->pRootShader->GetZPrepassState();
    internalShaderBeginEnd(pItem, uiShaderID, 0);
    pItem->pShaderChain[iShader] = pShader;
    pItem->uiShaderState = uiState;

    pcRedVertexShaderConstantRegister* pRegs;
    if (pItem->pBoneMatrices &&
        (int)pItem->pBoneMatrices != RedRenderer::m_pmBoneMatricesPrev)
    {
        RedRenderer::m_pmBoneMatricesPrev = (int)pItem->pBoneMatrices;
        pRegs = m_VSConstants.GetRegisterArray(51, 3 * pItem->numBoneMatrices);
        for (unsigned short i = 0; i < pItem->numBoneMatrices; ++i)
        {
            pRegs[51 + 3 * i].SetMatrix4x3(pItem->pBoneMatrices[i]);
        }
        m_VSConstants.Update();
    }

    if ((int)pItem->pPrim != RedRenderer::m_pPrimPrev)
    {
        RedRenderer::m_pPrimPrev = (int)pItem->pPrim;
        pRegs = m_VSConstants.GetRegisterArray(12, 4);
        pRegs[12].component[0] = (uiRVF & 0x4000) ? 2.0f : 1.0f;
        pRegs[12].component[1] = (uiRVF & 0x4000) ? -1.0f : 0.0f;
        pRegs[12].component[2] = (uiRVF & 0x8000) ? 1.0f / 2048.0f : 1.0f;
        pRegs[12].component[3] = 1.0f;

        if (uiRVF & 0x1000)
        {
            const PblBox& box = pItem->pPrim->m_bbox;
            const float scale = 1.0f / 65534.0f;
            pRegs[13].component[0] = (box._vMax.x - box._vMin.x) * scale;
            pRegs[13].component[1] = (box._vMax.y - box._vMin.y) * scale;
            pRegs[13].component[2] = (box._vMax.z - box._vMin.z) * scale;
            pRegs[13].component[3] = 0.0f;
            pRegs[14].component[0] = (box._vMax.x + box._vMin.x) * 0.5f;
            pRegs[14].component[1] = (box._vMax.y + box._vMin.y) * 0.5f;
            pRegs[14].component[2] = (box._vMax.z + box._vMin.z) * 0.5f;
            pRegs[14].component[3] = 1.0f;
        }
        else
        {
            pRegs[13].component[0] = 1.0f;
            pRegs[13].component[1] = 1.0f;
            pRegs[13].component[2] = 1.0f;
            pRegs[13].component[3] = 0.0f;
            pRegs[14].component[0] = 0.0f;
            pRegs[14].component[1] = 0.0f;
            pRegs[14].component[2] = 0.0f;
            pRegs[14].component[3] = 1.0f;
        }
        pRegs[15].component[0] = (uiRVF & 0x80) ? 0.0f : 1.0f;
        pRegs[15].component[1] = (uiRVF & 0x80) ? 1.0f : 0.0f;
        pRegs[15].component[2] = 0.0f;
        pRegs[15].component[3] = 1.0f;
    }

    if ((int)pItem->pmObjWorld != RedRenderer::m_pmObjWorldPrev)
    {
        RedRenderer::m_pmObjWorldPrev = (int)pItem->pmObjWorld;
        pRegs = m_VSConstants.GetRegisterArray(16, 3);
        PblMatrix world = *pItem->pmObjWorld;
        pRegs[16].SetMatrix4x3(world);
    }

    m_VSConstants.Update();
    pItem->pRootShader->RenderZPrepass(pItem);
}

// FUNCTION: SWBF2 0x006b7090
void RedRenderer::internalShaderBeginEnd(RenderItem* pItem, unsigned int uiShaderID,
                                         unsigned int uiPass)
{
    SortIdT sortId;
    sortId.id = 0;
    sortId.t_shaderID = uiShaderID;
    sortId.t_pass = uiPass;
    sortId.transparent = 1;

    SortIdT prevId;
    prevId.id = (unsigned int)RedRenderer::m_PrevSortID;
    bool bNewID = prevId.id != sortId.id;
    bool bNewShader;
    if (prevId.t_pass == sortId.t_pass && prevId.t_customID == sortId.t_customID)
    {
        bNewShader = false;
    }
    else
    {
        bNewShader = true;
        if (RedRenderer::m_pPrevShader)
        {
            (RedRenderer::m_pPrevShader->*RedRenderer::m_pPrevShader->EndFunc)(RedRenderer::m_uiPrevShaderState, prevId.t_pass);
            internalStateStackPop();
            RedRenderer::m_pPrevShader->Release();
        }
    }

    if (bNewID)
    {
        pItem->pShaderChain[pItem->uiCurrentShader]->GetShaderDef()->LoadVertexAndPixelShader(
            pItem->uiShaderState, uiPass, sortId.t_tnl);
    }

    if (bNewShader)
    {
        internalStateStackPush();
        pcRedShader* pShader = pItem->pShaderChain[pItem->uiCurrentShader];
        (pShader->*pShader->BeginFunc)(pItem->uiShaderState, uiPass);
        RedRenderer::m_pPrevShader = pItem->pShaderChain[pItem->uiCurrentShader];
        RedRenderer::m_pPrevShader->AddRef();
        RedRenderer::m_uiPrevShaderState = pItem->uiShaderState;
    }
    RedRenderer::m_PrevSortID = (int)sortId.id;
}

// FUNCTION: SWBF2 0x006b76c0
void RedRenderer::internalStateStackPush()
{
    m_iStateStackFrame = 1;
}

// FUNCTION: SWBF2 0x006b8800
void RedRenderer::pcLoadVertexShader(const pcRedVertexShader* pShader)
{
    if (RedRenderer::m_curVertexShader == pShader)
    {
        return;
    }
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetVertexShader(pShader ? pShader->m_pD3DVertexShader : 0);
    }
    else
    {
        m_pd3dDevice->SetVertexShader(pShader ? pShader->m_pD3DVertexShader : 0);
    }
    RedRenderer::m_curVertexShader = pShader;
}

// FUNCTION: SWBF2 0x006b8790
void RedRenderer::pcLoadPixelShader(const pcRedPixelShader* pShader)
{
    if (RedRenderer::m_curPixelShader == pShader)
    {
        return;
    }
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetPixelShader(pShader ? pShader->m_pD3DPixelShader : 0);
    }
    else
    {
        m_pd3dDevice->SetPixelShader(pShader ? pShader->m_pD3DPixelShader : 0);
    }
    RedRenderer::m_curPixelShader = pShader;
}

// FUNCTION: SWBF2 0x006b9750
void RedRenderer::pcSetSamplerState(unsigned int uiSampler, D3DSAMPLERSTATETYPE eType,
                                    unsigned int uiValue)
{
    unsigned long ulOld = m_curSamplerState[uiSampler][eType];
    if (uiValue == ulOld)
    {
        return;
    }

    if (m_iStateStackFrame != 0 && !m_bIsSamplerStateUndo[uiSampler][eType])
    {
        unsigned int uiSlot = m_uiNumSamplerStateUndo;
        m_bIsSamplerStateUndo[uiSampler][eType] = true;
        m_SamplerStateUndo[uiSlot].sampler = uiSampler;
        m_SamplerStateUndo[uiSlot].type = eType;
        m_SamplerStateUndo[uiSlot].value = ulOld;
        m_uiNumSamplerStateUndo = uiSlot + 1;
    }

    m_curSamplerState[uiSampler][eType] = uiValue;
    if (!stateManagerEnabled_)
    {
        m_pd3dDevice->SetSamplerState(uiSampler, eType, uiValue);
        return;
    }
    dx::StateManager_DX::GetInstance().SetSamplerState(uiSampler, eType, uiValue);
}

// FUNCTION: SWBF2 0x006b9ac0
void RedRenderer::pcSetTextureStageState(unsigned int uiStage, D3DTEXTURESTAGESTATETYPE eType,
                                         unsigned long ulValue)
{
    unsigned long ulOld = m_curTextureStageState[uiStage][eType];
    if (ulValue == ulOld)
    {
        return;
    }

    if (m_iStateStackFrame != 0 && !m_bIsTextureStageStateUndo[uiStage][eType])
    {
        unsigned int uiSlot = m_uiNumTextureStageStateUndo;
        m_bIsTextureStageStateUndo[uiStage][eType] = true;
        m_TextureStageStateUndo[uiSlot].stage = uiStage;
        m_TextureStageStateUndo[uiSlot].type = eType;
        m_TextureStageStateUndo[uiSlot].value = ulOld;
        m_uiNumTextureStageStateUndo = uiSlot + 1;
    }

    m_curTextureStageState[uiStage][eType] = ulValue;
    if (!stateManagerEnabled_)
    {
        m_pd3dDevice->SetTextureStageState(uiStage, eType, ulValue);
        return;
    }
    dx::StateManager_DX::GetInstance().SetTextureStageState(uiStage, eType, ulValue);
}

// FUNCTION: SWBF2 0x006b8760
void RedRenderer::pcLoadFormat(const pcRedVertexFormat* pFormat)
{
    if (RedRenderer::m_curFormat == pFormat)
    {
        return;
    }
    m_pd3dDevice->SetVertexDeclaration(pFormat->m_pVertexDeclaration);
    RedRenderer::m_curFormat = pFormat;
}

// FUNCTION: SWBF2 0x006b94b0
void RedRenderer::pcSetIndices(const pcRedIndexBuffer* pIndexBuffer)
{
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetIndices(pIndexBuffer->m_buffer);
    }
    else
    {
        m_pd3dDevice->SetIndices(pIndexBuffer->m_buffer);
    }
}

// FUNCTION: SWBF2 0x006b9910
void RedRenderer::pcSetStreamSource(unsigned int uiStream, const pcRedVertexBuffer* pVertexBuffer)
{
    if (m_curStream[uiStream] == pVertexBuffer->m_buffer)
    {
        return;
    }
    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetStreamSource(uiStream, pVertexBuffer->m_buffer, 0,
                                                           pVertexBuffer->m_vertexSize);
    }
    else
    {
        m_pd3dDevice->SetStreamSource(uiStream, pVertexBuffer->m_buffer, 0,
                                      pVertexBuffer->m_vertexSize);
    }
    m_curStream[uiStream] = pVertexBuffer->m_buffer;
}

// FUNCTION: SWBF2 0x006b9970
void RedRenderer::pcSetTexture(unsigned int uiStage, const RedTexture* pTexture)
{
    if (m_curSetTextures[uiStage] == pTexture)
    {
        return;
    }

    if (stateManagerEnabled_)
    {
        dx::StateManager_DX::GetInstance().SetTexture(uiStage, pTexture ? pTexture->m_pTexture : 0);
    }
    else
    {
        m_pd3dDevice->SetTexture(uiStage, pTexture ? pTexture->m_pTexture : 0);
    }
    m_curSetTextures[uiStage] = const_cast<RedTexture*>(pTexture);

    if (pTexture && pTexture->m_uiLastFrameUsed < (unsigned int)RedRenderer::m_frameNumber)
    {
        const_cast<RedTexture*>(pTexture)->m_uiLastFrameUsed = RedRenderer::m_frameNumber;
        static util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >& s_perFrame =
            util::Globals::GetInstance().GetSet("perFrame");
        static int& s_numTexUsed = s_perFrame.GetInt("numTexUsed");
        ++s_numTexUsed;
    }
}

#define COUNT_PERFRAME_VALUE(NAME)                                                              \
    {                                                                                            \
        static util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >&       \
            s_perFrame = util::Globals::GetInstance().GetSet("perFrame");                        \
        static int& s_count = s_perFrame.GetInt(NAME);                                           \
        ++s_count;                                                                               \
    }

// FUNCTION: SWBF2 0x006adb60
void dx::StateManager_DX::FlushState()
{
    FlushStateEx_();

    for (NVStateID* pState = m_DirtyStates.Begin(); pState != m_DirtyStates.End(); ++pState)
    {
        switch (pState->m_Type)
        {
        case 0:
            m_pD3DDev->SetMaterial(&m_CurrentState.m_D3DMaterial);
            m_ChipState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
            break;
        case 5:
            if (m_bForce || m_ChipState.m_VertexShader != m_CurrentState.m_VertexShader)
            {
                COUNT_PERFRAME_VALUE("numSetVShader");
                m_pD3DDev->SetVertexShader(m_CurrentState.m_VertexShader);
                m_ChipState.m_VertexShader = m_CurrentState.m_VertexShader;
            }
            break;
        case 6:
            m_pD3DDev->SetVertexDeclaration(m_CurrentState.m_VertexShaderDecl);
            m_ChipState.m_VertexShaderDecl = m_CurrentState.m_VertexShaderDecl;
            break;
        case 7:
            m_pD3DDev->SetFVF(m_CurrentState.m_FVF);
            m_ChipState.m_FVF = m_CurrentState.m_FVF;
            break;
        case 8:
            if (m_bForce || m_ChipState.m_PixelShader != m_CurrentState.m_PixelShader)
            {
                COUNT_PERFRAME_VALUE("numSetPShader");
                m_pD3DDev->SetPixelShader(m_CurrentState.m_PixelShader);
                m_ChipState.m_PixelShader = m_CurrentState.m_PixelShader;
            }
            break;
        case 9:
        {
            const unsigned long ulState = pState->m_dwValue0;
            m_pD3DDev->SetTransform((D3DTRANSFORMSTATETYPE)ulState,
                                    (const D3DMATRIX*)m_CurrentState.m_Matrices[ulState]);
            memcpy(m_ChipState.m_Matrices[ulState], m_CurrentState.m_Matrices[ulState],
                   sizeof(m_ChipState.m_Matrices[ulState]));
            break;
        }
        case 10:
        {
            const unsigned long ulReg = pState->m_dwValue0;
            const float* pChip = m_ChipState.m_VertexShaderConstants[ulReg];
            const float* pCur = m_CurrentState.m_VertexShaderConstants[ulReg];
            if (m_bForce || pChip[0] != pCur[0] || pChip[1] != pCur[1] || pChip[2] != pCur[2]
                || pChip[3] != pCur[3])
            {
                m_pD3DDev->SetVertexShaderConstantF(ulReg, pCur, 1);
                memcpy(m_ChipState.m_VertexShaderConstants[ulReg], pCur, 16);
            }
            break;
        }
        case 11:
        {
            const unsigned long ulReg = pState->m_dwValue0;
            const float* pChip = m_ChipState.m_PixelShaderConstants[ulReg];
            const float* pCur = m_CurrentState.m_PixelShaderConstants[ulReg];
            if (m_bForce || pChip[0] != pCur[0] || pChip[1] != pCur[1] || pChip[2] != pCur[2]
                || pChip[3] != pCur[3])
            {
                m_pD3DDev->SetPixelShaderConstantF(ulReg, pCur, 1);
                memcpy(m_ChipState.m_PixelShaderConstants[ulReg], pCur, 16);
            }
            break;
        }
        case 12:
        {
            const unsigned long ulStream = pState->m_dwStage;
            NVStreamData& rChip = m_ChipState.m_StreamData[ulStream];
            const NVStreamData& rCur = m_CurrentState.m_StreamData[ulStream];
            if (m_bForce || rChip.m_pStreamData != rCur.m_pStreamData || rChip.m_Stride != rCur.m_Stride)
            {
                COUNT_PERFRAME_VALUE("numSetStream");
                m_pD3DDev->SetStreamSource(ulStream, rCur.m_pStreamData, rCur.m_Offset, rCur.m_Stride);
                rChip = rCur;
            }
            break;
        }
        case 13:
            if (m_bForce
                || m_ChipState.m_IndexData.m_pIndexData != m_CurrentState.m_IndexData.m_pIndexData
                || m_ChipState.m_IndexData.m_BaseVertexIndex != m_CurrentState.m_IndexData.m_BaseVertexIndex)
            {
                COUNT_PERFRAME_VALUE("numSetIndices");
                m_pD3DDev->SetIndices((IDirect3DIndexBuffer9*)m_CurrentState.m_IndexData.m_pIndexData);
                m_ChipState.m_IndexData = m_CurrentState.m_IndexData;
            }
            break;
        default:
            break;
        }
    }

    m_DirtyStates.SetSize(0);
    renderStates_.SetSize(0);
    textureStageStates_.SetSize(0);
    samplerStates_.SetSize(0);
    textureStates_.SetSize(0);
}

// FUNCTION: SWBF2 0x006ae270
void dx::StateManager_DX::FlushStateEx_()
{
    if (m_bForce)
    {
        for (int i = 0; i < renderStates_.GetSize(); ++i)
        {
            const D3DRENDERSTATETYPE eState = renderStates_[i].m_RenderStateType;
            m_pD3DDev->SetRenderState(eState, m_CurrentState.m_RenderStates[eState]);
            m_ChipState.m_RenderStates[eState] = m_CurrentState.m_RenderStates[eState];
        }
        for (int i = 0; i < textureStageStates_.GetSize(); ++i)
        {
            const NVStateID& rId = textureStageStates_[i];
            m_pD3DDev->SetTextureStageState(rId.m_dwStage, rId.m_TextureStageStateType,
                                            m_CurrentState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType]);
            m_ChipState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType] =
                m_CurrentState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType];
        }
        for (int i = 0; i < samplerStates_.GetSize(); ++i)
        {
            const NVStateID& rId = samplerStates_[i];
            m_pD3DDev->SetSamplerState(rId.m_dwStage, rId.m_SamplerStateType,
                                       m_CurrentState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType]);
            m_ChipState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType] =
                m_CurrentState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType];
        }
        for (int i = 0; i < textureStates_.GetSize(); ++i)
        {
            COUNT_PERFRAME_VALUE("numSetTex");
            const unsigned long ulStage = textureStates_[i].m_dwStage;
            m_pD3DDev->SetTexture(ulStage, m_CurrentState.m_Textures[ulStage]);
            m_ChipState.m_Textures[ulStage] = m_CurrentState.m_Textures[ulStage];
        }
    }
    else
    {
        for (int i = 0; i < renderStates_.GetSize(); ++i)
        {
            const D3DRENDERSTATETYPE eState = renderStates_[i].m_RenderStateType;
            if (m_ChipState.m_RenderStates[eState] != m_CurrentState.m_RenderStates[eState])
            {
                m_pD3DDev->SetRenderState(eState, m_CurrentState.m_RenderStates[eState]);
                m_ChipState.m_RenderStates[eState] = m_CurrentState.m_RenderStates[eState];
            }
        }
        for (int i = 0; i < textureStageStates_.GetSize(); ++i)
        {
            const NVStateID& rId = textureStageStates_[i];
            const unsigned long ulValue =
                m_CurrentState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType];
            if (m_ChipState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType] != ulValue)
            {
                m_pD3DDev->SetTextureStageState(rId.m_dwStage, rId.m_TextureStageStateType, ulValue);
                m_ChipState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType] =
                    m_CurrentState.m_TextureStates[rId.m_dwStage][rId.m_TextureStageStateType];
            }
        }
        for (int i = 0; i < samplerStates_.GetSize(); ++i)
        {
            const NVStateID& rId = samplerStates_[i];
            const unsigned long ulValue =
                m_CurrentState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType];
            if (m_ChipState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType] != ulValue)
            {
                m_pD3DDev->SetSamplerState(rId.m_dwStage, rId.m_SamplerStateType, ulValue);
                m_ChipState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType] =
                    m_CurrentState.m_SamplerStates[rId.m_dwStage][rId.m_SamplerStateType];
            }
        }
        for (int i = 0; i < textureStates_.GetSize(); ++i)
        {
            const unsigned long ulStage = textureStates_[i].m_dwStage;
            if (m_ChipState.m_Textures[ulStage] != m_CurrentState.m_Textures[ulStage])
            {
                COUNT_PERFRAME_VALUE("numSetTex");
                m_pD3DDev->SetTexture(ulStage, m_CurrentState.m_Textures[ulStage]);
                m_ChipState.m_Textures[ulStage] = m_CurrentState.m_Textures[ulStage];
            }
        }
    }
}

// GLOBAL: SWBF2 0x007fbee0
// SYNTHETIC: SWBF2 0x00407630
// `dynamic initializer for 'RedRenderer::m_mViewUndo''

PblMatrix RedRenderer::m_mViewUndo;
// GLOBAL: SWBF2 0x007fbf20
// SYNTHETIC: SWBF2 0x00407600
// `dynamic initializer for 'RedRenderer::m_mProjectionUndo''

PblMatrix RedRenderer::m_mProjectionUndo;
// GLOBAL: SWBF2 0x007fb07b
bool bSetNullTexture;

// FUNCTION: SWBF2 0x006b72b0
void RedRenderer::internalStateStackPop()
{
    m_iStateStackFrame = 0;

    for (unsigned int i = 0; i < m_uiNumRenderStateUndo; ++i)
    {
        pcSetRenderState(m_RenderStateUndo[i].state, m_RenderStateUndo[i].mode);
    }
    m_uiNumRenderStateUndo = 0;
    memset(m_bIsRenderStateUndo, 0, sizeof(m_bIsRenderStateUndo));

    if (bSetNullTexture)
    {
        for (unsigned int i = 0; i < 8; ++i)
        {
            if (m_curSetTextures[i])
            {
                pcSetTexture(i, 0);
            }
        }
    }

    for (unsigned int i = 0; i < m_uiNumTextureStageStateUndo; ++i)
    {
        pcSetTextureStageState(m_TextureStageStateUndo[i].stage, m_TextureStageStateUndo[i].type,
                               m_TextureStageStateUndo[i].value);
    }
    m_uiNumTextureStageStateUndo = 0;
    memset(m_bIsTextureStageStateUndo, 0, sizeof(m_bIsTextureStageStateUndo));

    for (unsigned int i = 0; i < m_uiNumSamplerStateUndo; ++i)
    {
        pcSetSamplerState(m_SamplerStateUndo[i].sampler, m_SamplerStateUndo[i].type,
                          m_SamplerStateUndo[i].value);
    }
    m_uiNumSamplerStateUndo = 0;
    memset(m_bIsSamplerStateUndo, 0, sizeof(m_bIsSamplerStateUndo));

    PblMatrix& rView = *(PblMatrix*)&RedRenderer::m_mView;
    PblMatrix& rProj = RedRenderer::m_mProjection;
    if (!RedRenderer::m_bIsSetTransformViewUndo && !RedRenderer::m_bIsSetTransformProjectionUndo)
    {
        return;
    }

    if (RedRenderer::m_bIsSetTransformViewUndo)
    {
        rView = RedRenderer::m_mViewUndo;
        RedRenderer::m_bIsSetTransformViewUndo = 0;
        if (stateManagerEnabled_)
        {
            dx::StateManager_DX::GetInstance().SetTransform(D3DTS_VIEW, (const D3DMATRIX*)&rView);
        }
        else
        {
            m_pd3dDevice->SetTransform(D3DTS_VIEW, (const D3DMATRIX*)&rView);
        }
    }
    if (RedRenderer::m_bIsSetTransformProjectionUndo)
    {
        rProj = RedRenderer::m_mProjectionUndo;
        RedRenderer::m_bIsSetTransformProjectionUndo = 0;
        if (stateManagerEnabled_)
        {
            dx::StateManager_DX::GetInstance().SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)&rProj);
        }
        else
        {
            m_pd3dDevice->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)&rProj);
        }
    }

    PblMatrix mViewProj;
    D3DXMatrixMultiply((D3DXMATRIX*)&mViewProj, (const D3DXMATRIX*)&rView, (const D3DXMATRIX*)&rProj);
    RedRenderer::m_mViewProjection = mViewProj;
}

// FUNCTION: SWBF2 0x006af340
template void util::HeapInsert_<util::TKeyPair_<RenderItem*, unsigned int>, util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >,
                                util::GreaterEqOp<util::TKeyPair_<RenderItem*, unsigned int> > >(
    util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >&, int, util::TKeyPair_<RenderItem*, unsigned int> const&);

// TEMPLATE: SWBF2 0x006af390
// util::HeapPop_<util::TKeyPair_<RenderItem *,unsigned int>,util::Vector<util::TKeyPair_<RenderItem *,unsigned int>,util::BaseArray<util::TKeyPair_<RenderItem *,unsigned int>,util::TNewArrayAlloc<util::TKeyPair_<RenderItem *,unsigned int> > > >,util::GreaterEqOp<util::TKeyPair_<RenderItem *,unsigned int> > >
template void util::HeapPop_<util::TKeyPair_<RenderItem*, unsigned int>, util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >,
                             util::GreaterEqOp<util::TKeyPair_<RenderItem*, unsigned int> > >(
    util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >&, int);

// GLOBAL: SWBF2 0x007de2a2

bool sUseSecondOpaqueQueue = true;

template <class HEAP>
void RedRenderer::internalRenderFromHeap(HEAP& heap)
{
    SortIdT sortId;
    sortId.id = ~heap.Value();
    RenderItem* pItem = heap.Get();
    heap.Pop();

    if (pItem->pVisibilityQuery)
    {
        unsigned int uiPixels;
        if (pItem->pVisibilityQuery->GetData(&uiPixels, sizeof(uiPixels), 0) == S_OK && uiPixels == 0)
        {
            pItem->Free();
            return;
        }
    }

    if (pItem->flags & 0x1000)
    {
        pItem->pVisibilityQuery = internalGetD3DVisibilityQuery();
        unsigned int uiFlags = pItem->flags;
        pItem->flags = uiFlags & ~0x20;
        if (pItem->pVisibilityQuery)
        {
            pItem->pVisibilityQuery->Issue(D3DISSUE_BEGIN);
            internalRenderZPrepassProg1(pItem, sortId.o_shaderID);
            pItem->pVisibilityQuery->Issue(D3DISSUE_END);
        }
        else
        {
            internalRenderZPrepassProg1(pItem, sortId.o_shaderID);
        }
        pItem->flags = uiFlags & ~0x1000;
        return;
    }

    if (sortId.transparent)
    {
        internalRenderFunc(pItem, sortId.t_shaderID, sortId.t_pass);
        if (sortId.t_pass + 1 < pItem->pShaderChain[pItem->uiCurrentShader]
                                    ->GetShaderDef()->m_states[pItem->uiShaderState].uiNumPasses)
        {
            ++sortId.t_pass;
            const pcRedShaderDef::StateT& rState = pItem->pShaderChain[pItem->uiCurrentShader]
                                                       ->GetShaderDef()->m_states[pItem->uiShaderState];
            sortId.t_tnl = internalGetTransformAndLightingID(
                pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF, pItem->uiEnvState,
                rState.pass[sortId.t_pass].uiPassCaps);
            heap.Push(pItem, ~sortId.id);
            return;
        }
        if (pItem->uiCurrentShader + 1 < pItem->uiShaderChainSize)
        {
            ++pItem->uiCurrentShader;
            pcRedShader* pShader = pItem->pShaderChain[pItem->uiCurrentShader];
            pItem->uiShaderState = (pShader->*pShader->GetStateFunc)(pItem);
            sortId.t_pass = 0;
            sortId.t_customID = pItem->pShaderChain[pItem->uiCurrentShader]->GetID(pItem->uiShaderState);
            sortId.t_tnl = internalGetTransformAndLightingID(
                pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF, pItem->uiEnvState,
                pItem->pShaderChain[pItem->uiCurrentShader]->GetPassCaps(pItem->uiShaderState,
                                                                         sortId.t_pass));
            heap.Push(pItem, ~sortId.id);
            return;
        }
    }
    else
    {
        internalRenderFunc(pItem, sortId.o_shaderID, sortId.o_pass);
        SortHeap& rQueue = m_opaqueQueues._elements[heap.globalPass_ + (sUseSecondOpaqueQueue ? 1 : 0)];
        if (sortId.o_pass + 1 < pItem->pShaderChain[pItem->uiCurrentShader]
                                    ->GetShaderDef()->m_states[pItem->uiShaderState].uiNumPasses)
        {
            ++sortId.o_pass;
            const pcRedShaderDef::StateT& rState = pItem->pShaderChain[pItem->uiCurrentShader]
                                                       ->GetShaderDef()->m_states[pItem->uiShaderState];
            sortId.o_tnl = internalGetTransformAndLightingID(
                pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF, pItem->uiEnvState,
                rState.pass[sortId.o_pass].uiPassCaps);
            rQueue.Push(pItem, ~sortId.id);
            return;
        }
        if (pItem->uiCurrentShader + 1 < pItem->uiShaderChainSize)
        {
            ++pItem->uiCurrentShader;
            pcRedShader* pShader = pItem->pShaderChain[pItem->uiCurrentShader];
            pItem->uiShaderState = (pShader->*pShader->GetStateFunc)(pItem);
            sortId.o_pass = 0;
            sortId.o_customID = GetShaderID_(pItem->pShaderChain[pItem->uiCurrentShader], pItem,
                                             pItem->uiShaderState);
            sortId.o_tnl = internalGetTransformAndLightingID(
                pItem->pPrim->m_vertexFormat->m_uiVertexBufferRVF, pItem->uiEnvState,
                pItem->pShaderChain[pItem->uiCurrentShader]->GetPassCaps(pItem->uiShaderState,
                                                                         sortId.o_pass));
            rQueue.Push(pItem, ~sortId.id);
            return;
        }
    }
    pItem->Free();
}

// FUNCTION: SWBF2 0x006af430
template void RedRenderer::internalRenderFromHeap<TransparentHeap<RenderItem*, unsigned int> >(
    TransparentHeap<RenderItem*, unsigned int>&);
// FUNCTION: SWBF2 0x006af820
template void RedRenderer::internalRenderFromHeap<RedRenderer::SortHeap>(RedRenderer::SortHeap&);

// FUNCTION: SWBF2 0x006aff80
RedLightingState& RedLightingState::operator=(const RedLightingState& rOther)
{
    ambientLight = rOther.ambientLight;
    pDirectionalLights[0] = rOther.pDirectionalLights[0];
    pDirectionalLights[1] = rOther.pDirectionalLights[1];
    pOmniLights[0] = rOther.pOmniLights[0];
    pOmniLights[1] = rOther.pOmniLights[1];
    pOmniLights[2] = rOther.pOmniLights[2];
    pOmniLights[3] = rOther.pOmniLights[3];
    pSpotLights[0] = rOther.pSpotLights[0];
    pDirectionalFactor[0] = rOther.pDirectionalFactor[0];
    pDirectionalFactor[1] = rOther.pDirectionalFactor[1];
    pProjectedLight = rOther.pProjectedLight;
    pEnvMap = rOther.pEnvMap;
    envMapFactor = rOther.envMapFactor;
    numDirectionalLights = rOther.numDirectionalLights;
    numOmniLights = rOther.numOmniLights;
    numSpotLights = rOther.numSpotLights;
    bStaticLighting = rOther.bStaticLighting;
    numSpecularLights = rOther.numSpecularLights;
    return *this;
}

// FUNCTION: SWBF2 0x006b28a0
void RedRenderer::SortHeap::Push(RenderItem* pItem, unsigned int uiKey)
{
    util::TKeyPair_<RenderItem*, unsigned int> pair(pItem, uiKey);
    int iIndex = GetSize();
    Add(pair);
    util::HeapInsert_<util::TKeyPair_<RenderItem*, unsigned int>, util::Vector<util::TKeyPair_<RenderItem*, unsigned int> >,
                      util::GreaterEqOp<util::TKeyPair_<RenderItem*, unsigned int> > >(*this, iIndex, pair);
}

// FUNCTION: SWBF2 0x006b34b0
void RedRenderer::SetDefaultLightingState(const RedLightingState& state)
{
    RedRenderer::m_aLightingStatePool[2050] = state;
}

// FUNCTION: SWBF2 0x006b3770
void RedRenderer::SetLightingState(const RedLightingState& state)
{
    if (&state >= m_aLightingStatePool && &state < m_aLightingStatePool + 2051)
    {
        RedRenderer::m_pLightingState = (RedLightingState*)&state;
        return;
    }
    RedLightingState* pState = AllocLightingStateInCache();
    *pState = state;
    RedRenderer::m_pLightingState = pState;
}

// FUNCTION: SWBF2 0x006b8180
void RedRenderer::pcEnableScreenSpaceShadow(bool bEnable)
{
    if (RedRenderer::m_renderCaps.pipeline == PIPELINE_FIXED)
    {
        bEnable = false;
    }
    RedRenderer::m_bScreenSpaceShadows = bEnable;
}

// FUNCTION: SWBF2 0x006b8540
const D3DPRESENT_PARAMETERS& RedRenderer::pcGetPresentParameters()
{
    return m_d3dpp;
}

// FUNCTION: SWBF2 0x006b8580
pcRedShader* RedRenderer::pcGetSystemShader(SystemShaderT eShader)
{
    return m_systemShaders[eShader];
}

// FUNCTION: SWBF2 0x006b8590
bool RedRenderer::pcGetUserNearClipPlaneEnable()
{
    return RedRenderer::m_userNearClipPlaneEnable;
}

// FUNCTION: SWBF2 0x006b8750
bool RedRenderer::pcIsNearScene()
{
    return m_bIsNearScene;
}

// FUNCTION: SWBF2 0x006b9390
void RedRenderer::pcSetFogForBlendMode(FOGFORBLEND_MODE eMode)
{
    if (!RedRenderer::m_bFogEnable)
    {
        return;
    }

    RedColor color;
    switch (eMode)
    {
    case FOGFORBLEND_ADD:
        pcSetRenderState(D3DRS_FOGCOLOR, 0xFF000000);
        return;
    case FOGFORBLEND_MODULATE:
        color = RedColor::WHITE * m_lightingScaleColor;
        break;
    case FOGFORBLEND_MODULATE2X:
    {
        RedColor grey;
        grey.b = 0x80;
        grey.g = 0x80;
        grey.r = 0x80;
        grey.a = 0xFF;
        color = grey * m_lightingScaleColor;
        break;
    }
    default:
        color = RedRenderer::m_colorFog * m_lightingScaleColor;
        break;
    }
    pcSetRenderState(D3DRS_FOGCOLOR,
                     (((((unsigned int)color.a << 8) | color.r) << 8 | color.g) << 8) | color.b);
}

// FUNCTION: SWBF2 0x006b94a0
void RedRenderer::pcSetImmediateMode(int iValue)
{
    RedRenderer::m_iImmediateMode = iValue;
}

// FUNCTION: SWBF2 0x006b9650
void RedRenderer::pcSetRenderTarget(RedTexture* pTarget, IDirect3DSurface9* pDepth)
{
    static const RedViewport s_fullViewport = { 0.0f, 0.0f, 1.0f, 1.0f };

    for (int i = 0; i < 4; ++i)
    {
        if (m_curSetTextures[i] && m_curSetTextures[i] == m_curRenderTarget)
        {
        }
    }

    if (pDepth)
    {
        m_curDepthTarget = pDepth;
        D3DSURFACE_DESC desc;
        pDepth->GetDesc(&desc);
        if (desc.Format == D3DFMT_D16 || desc.Format == D3DFMT_D16_LOCKABLE)
        {
            m_fMaxZDepth = 65535.0f;
        }
        else
        {
            m_fMaxZDepth = 16777215.0f;
        }
    }
    else
    {
        m_curDepthTarget = 0;
        m_fMaxZDepth = 16777215.0f;
    }

    IDirect3DSurface9* pSurface;
    if (pTarget)
    {
        ((IDirect3DTexture9*)pTarget->m_pTexture)->GetSurfaceLevel(0, &pSurface);
        m_curRenderTarget = pTarget;
    }
    else
    {
        m_pBackBufferSurface->AddRef();
        pSurface = m_pBackBufferSurface;
        m_curRenderTarget = 0;
    }
    m_pd3dDevice->SetRenderTarget(0, pSurface);
    m_pd3dDevice->SetDepthStencilSurface(pDepth);
    pSurface->Release();
    pcSetViewport(s_fullViewport);
}

// FUNCTION: SWBF2 0x006b9de0
void RedRenderer::pcSetUserNearClipPlaneEnable(bool bEnable)
{
    if (RedRenderer::m_userNearClipPlaneEnable == bEnable)
    {
        return;
    }
    RedRenderer::m_userNearClipPlaneEnable = bEnable;
    internalUpdateSceneConstants();
}

// LIBRARY: SWBF2 0x006ac57c
// Direct3DCreate9
