#include "RedEngineFL/Util/RedCommandConsole.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcStencilShadowShader.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "pcRedParticle.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include <string.h>
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Util/RedLodManager.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"
#include "pcShadowQuadRenderable.h"
#include <float.h>
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "pcFLRender.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "Battlefront2/Source/GameMovie.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include <math.h>

// GLOBAL: SWBF2 0x007de9c0
bool enableReflection = true;

// GLOBAL: SWBF2 0x007de9c1
bool bReflectionsEnable = true;

// GLOBAL: SWBF2 0x007de9c2
bool gEnable = true;

namespace FLRenderer
{
extern bool m_bFarSceneEnable[1];
}

// GLOBAL: SWBF2 0x007de9c3
bool FLRenderer::m_bFarSceneEnable[1] = { true };

// GLOBAL: SWBF2 0x007de9c4
int FAR_SCENE_IMMEDIATE_MODE = 32;
// GLOBAL: SWBF2 0x007de9c8
int REFLECTION_SCENE_IMMEDIATE_MODE = 32;

// GLOBAL: SWBF2 0x008f6d98
bool bReflectionsDraw;

// GLOBAL: SWBF2 0x008f6ded
bool NearScenePushInf;

// SYNTHETIC: SWBF2 0x00407870
// `dynamic initializer for 'g_Enable''
static RedCommandConsole g_Enable("Interface.Enable", MultiVar(&gEnable));

// SYNTHETIC: SWBF2 0x00407840
// `dynamic initializer for 'cvReflectionsEnable''
static RedCommandConsole cvReflectionsEnable("Reflections.Enable", MultiVar(&bReflectionsEnable));

// SYNTHETIC: SWBF2 0x00407810
// `dynamic initializer for 'cvReflectionsDraw''
static RedCommandConsole cvReflectionsDraw("Reflections.Draw", MultiVar(&bReflectionsDraw));

// SYNTHETIC: SWBF2 0x004078a0
// `dynamic initializer for 'cvarNearScenePushInf''
static RedCommandConsole cvarNearScenePushInf("nearScene.PushInfinite", MultiVar(&NearScenePushInf));

// SYNTHETIC: SWBF2 0x004077e0
// `dynamic initializer for 's_cvRenderFarScene''
static RedCommandConsole s_cvRenderFarScene("render.FarScene", MultiVar(&FLRenderer::m_bFarSceneEnable[0]));

// SYNTHETIC: SWBF2 0x004077b0
// `dynamic initializer for '_enableReflection''
static RedCommandConsole _enableReflection("render.Reflections", MultiVar(&enableReflection));

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// FUNCTION: SWBF2 0x006bb350
void FLRenderer::AddInterfaceElement(RedScreenGroupElement* pElement, float fX,
                                     float fY, bool bUseSafeZone)
{
    pElement->m_fScreenRelativeX = fX;
    pElement->m_fScreenRelativeY = fY;
    pElement->m_bUseSafeZone = bUseSafeZone;

    RedInterfaceScreen::s_screenFull.AddElement(pElement);
}

extern bool g_bNoRender;

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

// GLOBAL: SWBF2 0x007de9dc
PblList<ReflectionRegion> ReflectionRegion::s_List;

// GLOBAL: SWBF2 0x007dea30
PblList<ReflectionRegion> ReflectionRegion::s_VisibleList;

// GLOBAL: SWBF2 0x007dea48
FLRenderer::RenderableList FLRenderer::_RenderableLists[10];

// GLOBAL: SWBF2 0x007deb38

FLRenderer::TransitionT sViewTransitions[2] = {
    { { 0, 0, 0, 0 }, { 0 }, 0.0f, { 0, 0, 0, 0 }, 0.0f, 0.0f, 1.0f, 1.0f },
    { { 0, 0, 0, 0 }, { 0 }, 0.0f, { 0, 0, 0, 0 }, 0.0f, 0.0f, 1.0f, 1.0f },
};

// GLOBAL: SWBF2 0x008f6d99
bool FLRenderer::m_bEnableFadeAdjustWithZoom;
// GLOBAL: SWBF2 0x008f6d9a
bool FLRenderer::m_bSceneEnable[1];

// GLOBAL: SWBF2 0x008f6d9b
namespace FLRenderer { bool m_bInterfaceEnable[1]; }
namespace FLRenderer { extern float m_fWorldFogStart; }
namespace FLRenderer { extern float m_fWorldFogEnd; }
namespace FLRenderer { extern RedColor m_colorReflectionFog; }

// FUNCTION: SWBF2 0x006beac0
void FLRenderer::RenderableList::RenderReflections(unsigned int uiFlags) const
{
    for (const Node* pNode = _head._pNext; pNode->_pObject; pNode = pNode->_pNext)
    {
        if (pNode->_pObject->IsReflected())
        {
            pNode->_pObject->Render(uiFlags | 0x200000);
        }
    }
}

// FUNCTION: SWBF2 0x006bf020
void FLRenderer::SetFadeAdjustWithZoom(bool bEnable)
{
    m_bEnableFadeAdjustWithZoom = bEnable;
}

// FUNCTION: SWBF2 0x006bf110
void FLRenderer::SetInterfaceEnable(unsigned int uiView, bool bEnable)

{
    FLRenderer::m_bInterfaceEnable[uiView] = bEnable;
}

// FUNCTION: SWBF2 0x006bf220
void FLRenderer::SetReflectionFogColor(const RedColor& color)
{
    FLRenderer::m_colorReflectionFog = color;
}

// FUNCTION: SWBF2 0x006bf300
void FLRenderer::SetWorldFogRange(float fStart, float fEnd)
{
    FLRenderer::m_fWorldFogStart = fStart;
    FLRenderer::m_fWorldFogEnd = fEnd;
}

// FUNCTION: SWBF2 0x006bf840
RedTexture* FLRenderer::pcGetReflectionTexture(char)
{
    return m_reflectionRenderTarget;
}

// FUNCTION: SWBF2 0x006bf690
void ReflectionRegion::_UpdateReflectionMatrix()
{
    const float x = m_reflectionPlane.x;
    const float y = m_reflectionPlane.y;
    const float z = m_reflectionPlane.z;
    const float w = m_reflectionPlane.w;
    const float xx = x * x, yy = y * y, zz = z * z;
    const float inverse = 1.0f / ((xx + yy) + zz);
    const float twice = -2.0f * inverse;
    PblMatrix& m = m_reflectionMat;
    m.m[0][0] = ((yy - xx) + zz) * inverse;
    m.m[0][1] = (twice * x) * y;
    m.m[0][2] = (twice * x) * z;
    m.m[0][3] = 0.0f;
    m.m[1][0] = (twice * x) * y;
    m.m[1][1] = ((xx - yy) + zz) * inverse;
    m.m[1][2] = (twice * y) * z;
    m.m[1][3] = 0.0f;
    m.m[2][0] = (twice * x) * z;
    m.m[2][1] = (twice * y) * z;
    m.m[2][2] = ((xx + yy) - zz) * inverse;
    m.m[2][3] = 0.0f;
    m.m[3][0] = (twice * x) * w;
    m.m[3][1] = (twice * y) * w;
    m.m[3][2] = (twice * z) * w;
    m.m[3][3] = 1.0f;
}

// GLOBAL: SWBF2 0x008f6da0
RedViewport FLRenderer::m_Viewport[1];
// GLOBAL: SWBF2 0x008f6db0
RedCamera* FLRenderer::m_pViewCamera[1];
// GLOBAL: SWBF2 0x008f6db4
unsigned int FLRenderer::mView;
// GLOBAL: SWBF2 0x008f6db8
unsigned int FLRenderer::mViewCount;
// GLOBAL: SWBF2 0x008f6dbc
float fl_fog_start;
// GLOBAL: SWBF2 0x008f6dc0
float fl_fog_end;
// GLOBAL: SWBF2 0x008f6dc4
namespace FLRenderer { float m_fWorldFogStart; }
// GLOBAL: SWBF2 0x008f6dc8
namespace FLRenderer { float m_fWorldFogEnd; }
// GLOBAL: SWBF2 0x008f6dcc
float FLRenderer::m_fViewNearPlane;
// GLOBAL: SWBF2 0x008f6dd0
float FLRenderer::m_fViewNearSceneFadeStart[2];
// GLOBAL: SWBF2 0x008f6dd8
float FLRenderer::m_fViewNearSceneFadeEnd[2];
// GLOBAL: SWBF2 0x008f6de0
float FLRenderer::m_fViewFarPlane[2];
// GLOBAL: SWBF2 0x008f6de8
namespace FLRenderer { float m_fNearSceneRangeFactor; }
// GLOBAL: SWBF2 0x008f6dec

bool sFarSceneRangeEnabled[1];
// GLOBAL: SWBF2 0x008f6df0
RedInterfaceScreen* FLRenderer::m_pViewInterfaceScreen[1];
// GLOBAL: SWBF2 0x008f6df4
ReflectionRegionFactory* FLRenderer::m_pReflectionRegionFactory;
// GLOBAL: SWBF2 0x008f6df8
RedTexture* FLRenderer::m_reflectionRenderTarget;
// GLOBAL: SWBF2 0x008f6dfc
namespace FLRenderer { IDirect3DSurface9* m_reflectionDepthBuffer; }
// GLOBAL: SWBF2 0x008f6e00
namespace FLRenderer { IDirect3DSurface9* m_FarSceneDepthBuffer; }
// GLOBAL: SWBF2 0x008f6ed0
unsigned int FLRenderer::m_numCurFrameReflections;
// GLOBAL: SWBF2 0x008f6ed4
RedTexture* FLRenderer::m_previousFrameTex;
// GLOBAL: SWBF2 0x008f6ed8
_D3DFORMAT FLRenderer::m_formatReflectionTarget_Medium;
// GLOBAL: SWBF2 0x008f6edc
_D3DFORMAT FLRenderer::m_formatReflectionDepth_Medium;
// GLOBAL: SWBF2 0x008f6ef0
// SYNTHETIC: SWBF2 0x00407930
// `dynamic initializer for 'FLRenderer::mCursorImage''
// SYNTHETIC: SWBF2 0x00769880
// `dynamic atexit destructor for 'FLRenderer::mCursorImage''
RedBitmapElement FLRenderer::mCursorImage;

// GLOBAL: SWBF2 0x008f6ff0
RedColor FLRenderer::m_colorFog;
// GLOBAL: SWBF2 0x008f6ff4
namespace FLRenderer { RedColor m_colorReflectionFog; }
// GLOBAL: SWBF2 0x008f7000
// SYNTHETIC: SWBF2 0x00407910
// `dynamic initializer for 'FLRenderer::mCursorGroup''
// SYNTHETIC: SWBF2 0x00769870
// `dynamic atexit destructor for 'FLRenderer::mCursorGroup''
RedScreenGroupElement FLRenderer::mCursorGroup;
// GLOBAL: SWBF2 0x008f70a0
// SYNTHETIC: SWBF2 0x00407950
// `dynamic initializer for 'FLRenderer::m_curFrameReflections''
FLRenderer::ReflectionT FLRenderer::m_curFrameReflections[255];

// FUNCTION: SWBF2 0x006bb240
FLRenderer::ReflectionT::ReflectionT()
{
}

// FUNCTION: SWBF2 0x006bb280
template void PblOrderedList<Renderable, int>::Add(PblOrderedList<Renderable, int>::Node*, Renderable*, int);

// FUNCTION: SWBF2 0x006bb2e0
void FLRenderer::RenderableList::Add(Renderable* pRenderable, int iOrder)
{
    PblOrderedList<Renderable, int>::Add(&pRenderable->_RenderableNode, pRenderable, iOrder);
}

// FUNCTION: SWBF2 0x006bb8c0
unsigned int FLRenderer::AddView(const RedViewport& rViewport, RedCamera* pCamera)
{
    unsigned int uiView;
    for (uiView = 0; uiView < mViewCount; uiView++)
    {
        if (!m_pViewCamera[uiView])
        {
            break;
        }
    }
    if (uiView >= 1)
    {
        return (unsigned int)-1;
    }
    if (uiView >= mViewCount)
    {
        mViewCount = uiView + 1;
    }

    m_Viewport[uiView] = rViewport;
    m_pViewCamera[uiView] = pCamera;
    m_pViewCamera[uiView]->SetNearFarPlane(GetNearPlane(), GetNearSceneFadeEnd());
    FLRenderer::m_bInterfaceEnable[uiView] = true;
    m_bSceneEnable[uiView] = true;

    m_pViewInterfaceScreen[uiView]->SetDimensions(
        (unsigned int)((float)RedInterfaceScreen::GetScreenWidth() * rViewport.fWidth),
        (unsigned int)((float)RedInterfaceScreen::GetScreenHeight() * rViewport.fHeight));

    float fMargin = 0.5f - RedInterfaceScreen::s_screenFull.m_fSafeZoneWidth * 0.5f;
    float fLeft = (fMargin >= rViewport.fOffsetX) ? fMargin : rViewport.fOffsetX + 0.01f;
    float fTop = (fMargin >= rViewport.fOffsetY) ? fMargin : rViewport.fOffsetY + 0.01f;
    float fLimit = 1.0f - fMargin;
    float fRight = rViewport.fOffsetX + rViewport.fWidth;
    fRight = (fRight >= fLimit) ? fLimit : fRight - 0.01f;
    float fBottom = rViewport.fOffsetY + rViewport.fHeight;
    fBottom = (fBottom >= fLimit) ? fLimit : fBottom - 0.01f;

    m_pViewInterfaceScreen[uiView]->SetSafeZone((fRight - fLeft) / rViewport.fWidth,
                                                (fBottom - fTop) / rViewport.fHeight,
                                                (fLeft - rViewport.fOffsetX) / rViewport.fWidth,
                                                (fTop - rViewport.fOffsetY) / rViewport.fHeight);
    return uiView;
}

// FUNCTION: SWBF2 0x006bc6b0
float FLRenderer::GetNearPlane()
{
    return m_fViewNearPlane;
}

// FUNCTION: SWBF2 0x006bc6c0
float FLRenderer::GetNearSceneFadeEnd()
{
    float fFactor = RedRenderer::pcGetRenderOptions().mNearSceneFadeFactor;
    if (0.0f > fFactor)
    {
        fFactor = 0.0f;
    }
    else if (fFactor > 1.0f)
    {
        fFactor = 1.0f;
    }
    return (m_fViewNearSceneFadeEnd[1] - m_fViewNearSceneFadeEnd[0]) * fFactor +
           m_fViewNearSceneFadeEnd[0];
}

// FUNCTION: SWBF2 0x006bc7a0
unsigned int FLRenderer::GetViewportCount()
{
    return mViewCount;
}

// FUNCTION: SWBF2 0x006bc7b0
void FLRenderer::Init(bool bUberMode)
{
    RedSceneManager::Init();

    RedLodManager::Init(4);
    RedLodManager::SetClassMaxCost(2, 100, 18000, 0, 1000, 40000);
    RedLodManager::SetFadeSpeed(2, 10.0f);
    RedLodManager::SetClassLodHint(2, 100, 0, 0);
    RedLodManager::SetClassMaxCost(1, 600, 100000, 22000, 22000, 40000);
    RedLodManager::SetFadeSpeed(1, 5.0f);
    RedLodManager::SetClassLodHint(1, 12000, 2000, 1000);
    if (bUberMode)
    {
        RedLodManager::SetClassMaxCost(3, 1500, 10000, 10000, 10000, 40000);
    }
    else
    {
        RedLodManager::SetClassMaxCost(3, 300, 1000, 1000, 1000, 40000);
    }
    RedLodManager::SetFadeSpeed(3, 10.0f);
    RedLodManager::SetClassMaxCost(0, 200, 50000, 3000, 3000, 40000);
    RedLodManager::SetFadeSpeed(0, 5.0f);
    RedLodManager::SetClassLodHint(0, 5000, 200, 100);

    RedRenderer::pcSetClearColor(RedColor::BLACK);
    RedLight::InitSys();

    m_fViewNearSceneFadeStart[0] = 72.0f;
    m_fViewNearSceneFadeEnd[0] = 120.0f;
    m_fViewNearSceneFadeStart[1] = 300.0f;
    m_fViewNearSceneFadeEnd[1] = 500.0f;
    m_fViewNearPlane = 0.69901854f;
    m_fViewFarPlane[0] = 1500.0f;
    m_fViewFarPlane[1] = 3000.0f;
    m_colorFog = RedColor::BLACK;
    FLRenderer::m_colorReflectionFog = RedColor::BLACK;
    fl_fog_start = 0.0f;
    fl_fog_end = 0.0f;
    FLRenderer::m_fWorldFogStart = 0.0f;
    FLRenderer::m_fWorldFogEnd = 0.0f;
    FLRenderer::m_fNearSceneRangeFactor = 1.0f;
    m_bEnableFadeAdjustWithZoom = true;
    m_Viewport[0].fOffsetY = 0.0f;
    m_Viewport[0].fOffsetX = 0.0f;
    m_Viewport[0].fWidth = 1.0f;
    m_Viewport[0].fHeight = 1.0f;
    m_pViewCamera[0] = 0;
    m_bSceneEnable[0] = true;
    FLRenderer::m_bInterfaceEnable[0] = true;
    sFarSceneRangeEnabled[0] = true;

    m_pViewInterfaceScreen[0] = new RedInterfaceScreen;
    m_reflectionRenderTarget = 0;
    FLRenderer::m_reflectionDepthBuffer = 0;
    FLRenderer::m_FarSceneDepthBuffer = 0;

    if (!g_bNoRender)
    {
        m_previousFrameTex = new RedTexture(512, 256, D3DFMT_A8R8G8B8, 1,
                                            D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, 0);

        if (RedRenderer::pcCheckDepthStencilMatch(D3DFMT_R5G6B5, D3DFMT_D24S8))
        {
            m_formatReflectionTarget_Medium = D3DFMT_R5G6B5;
            m_formatReflectionDepth_Medium = D3DFMT_D24S8;
        }
        else if (RedRenderer::pcCheckDepthStencilMatch(D3DFMT_R5G6B5, D3DFMT_D16))
        {
            m_formatReflectionTarget_Medium = D3DFMT_R5G6B5;
            m_formatReflectionDepth_Medium = D3DFMT_D16;
        }
        else
        {
            m_formatReflectionTarget_Medium = D3DFMT_A8R8G8B8;
            m_formatReflectionDepth_Medium = D3DFMT_D16;
        }
    }
    else
    {
        m_previousFrameTex = 0;
    }

    memset(FLRenderer::m_potentialReflections, 0, sizeof(FLRenderer::m_potentialReflections));
    memset(m_curFrameReflections, 0, sizeof(m_curFrameReflections));
    FLRenderer::m_numPotentialReflections = 0;
    m_numCurFrameReflections = 0;

    if (!g_bNoRender)
    {
        pcShadowQuadRenderable::Create();
    }

    mView = 0;
    mViewCount = 0;

    ReflectionRegion::Init();
    m_pReflectionRegionFactory = new ReflectionRegionFactory;

    RedRenderer::pcRegisterResetFunction(pcReset);
}

void pcShadowQuadRenderable::Create()
{
    s_pInstance = new pcShadowQuadRenderable;
    s_pInstance->Init();
}

void ReflectionRegion::Init()
{
    s_VisibleList.RemoveAll();
    s_List.RemoveAll();
}

void pcShadowQuadRenderable::Destroy()
{
    s_pInstance->Cleanup();
    delete s_pInstance;
    s_pInstance = 0;
}

void ReflectionRegion::Cleanup()
{
    s_VisibleList.RemoveAll();
    s_List.RemoveAll();
}

// FUNCTION: SWBF2 0x006bbc20
void FLRenderer::Cleanup()
{
    if (!g_bNoRender)
    {
        pcShadowQuadRenderable::Destroy();
    }

    RedSceneManager::Cleanup();
    RedLodManager::Cleanup();
    RedLight::DeinitSys();

    for (int i = 0; i < 10; i++)
    {
        _RenderableLists[i].Clear();
    }

    for (unsigned int i = 0; i < mViewCount; i++)
    {
        delete m_pViewInterfaceScreen[i];
        m_pViewInterfaceScreen[i] = 0;
        m_pViewCamera[i] = 0;
    }

    RedRenderer::pcUnregisterResetFunction(pcReset);

    delete m_reflectionRenderTarget;
    m_reflectionRenderTarget = 0;
    if (FLRenderer::m_reflectionDepthBuffer)
    {
        FLRenderer::m_reflectionDepthBuffer->Release();
        FLRenderer::m_reflectionDepthBuffer = 0;
    }
    if (FLRenderer::m_FarSceneDepthBuffer)
    {
        FLRenderer::m_FarSceneDepthBuffer->Release();
        FLRenderer::m_FarSceneDepthBuffer = 0;
    }
    delete m_previousFrameTex;
    m_previousFrameTex = 0;

    RemoveInterfaceElement(&mCursorGroup);
    mCursorGroup.RemoveElement(&mCursorImage);
    mCursorImage.Cleanup();

    ReflectionRegion::Cleanup();
    delete m_pReflectionRegionFactory;
    m_pReflectionRegionFactory = 0;
}

// FUNCTION: SWBF2 0x006bcf50
void FLRenderer::RenderableList::Remove(Renderable* pRenderable)
{
    PblOrderedList<Renderable, int>::Remove(&pRenderable->_RenderableNode);
}

// FUNCTION: SWBF2 0x006bf030
void FLRenderer::SetFarSceneRange(float fStart, float fEnd)
{
    m_fViewFarPlane[0] = fStart;
    m_fViewFarPlane[1] = fEnd;
    for (unsigned int i = 0; i < mViewCount; ++i)
    {
        if (m_pViewCamera[i])
        {
            sFarSceneRangeEnabled[i] = fStart > 0.0f;
        }
    }
}

// FUNCTION: SWBF2 0x006bf0e0
void FLRenderer::SetFogColor(const RedColor& color)
{
    m_colorFog = color;
}

// FUNCTION: SWBF2 0x006bf0f0
void FLRenderer::SetFogRange(float fStart, float fEnd)
{
    fl_fog_start = fStart;
    fl_fog_end = fEnd;
}

// FUNCTION: SWBF2 0x006bf130
void FLRenderer::SetNearSceneRange(float fNearPlane, float fFadeStartLow, float fFadeStartHigh,
                                   float fFadeEndLow, float fFadeEndHigh)
{
    if (NearScenePushInf)
    {
        fFadeEndHigh = 100000.0f;
        fFadeEndLow = 100000.0f;
    }
    m_fViewNearPlane = fNearPlane;
    m_fViewNearSceneFadeStart[0] = fFadeStartLow > 0.0f ? fFadeStartLow : 0.0f;
    m_fViewNearSceneFadeStart[1] = fFadeStartHigh > 0.0f ? fFadeStartHigh : 0.0f;
    m_fViewNearSceneFadeEnd[0] = fFadeEndLow > 0.0f ? fFadeEndLow : 0.0f;
    m_fViewNearSceneFadeEnd[1] = fFadeEndHigh > 0.0f ? fFadeEndHigh : 0.0f;
    for (unsigned int i = 0; i < mViewCount; ++i)
    {
        if (m_pViewCamera[i])
        {
            m_pViewCamera[i]->SetNearFarPlane(GetNearPlane(), GetNearSceneFadeEnd());
        }
    }
    RedRenderer::pcSetNearSceneStart(GetNearSceneFadeStart());
    RedRenderer::pcSetNearSceneEnd(GetNearSceneFadeEnd());
}

// FUNCTION: SWBF2 0x006bf230
void FLRenderer::SetSceneEnable(unsigned int uiView, bool bEnable)
{
    m_bSceneEnable[uiView] = bEnable;
}

// FUNCTION: SWBF2 0x006bf250
void FLRenderer::SetTransitionBlurColor(int iView, RedColor const& rColor)
{
    sViewTransitions[iView].blurColor = rColor;
}

// FUNCTION: SWBF2 0x006bf270
void FLRenderer::SetTransitionBlurZoom(int iView, float fZoom)
{
    sViewTransitions[iView].blurZoom = fZoom;
}

// FUNCTION: SWBF2 0x006bf290
void FLRenderer::SetTransitionOverlayColor(int iView, RedColor const& rColor)
{
    sViewTransitions[iView].overlayColor = rColor;
}

// FUNCTION: SWBF2 0x006bf2b0
void FLRenderer::SetTransitionOverlayRect(int iView, float fX0, float fY0, float fX1, float fY1)
{
    sViewTransitions[iView].overlayX0 = fX0;
    sViewTransitions[iView].overlayY0 = fY0;
    sViewTransitions[iView].overlayX1 = fX1;
    sViewTransitions[iView].overlayY1 = fY1;
}

// FUNCTION: SWBF2 0x006bf320
void ReflectionRegion::UpdateAllVisibility(const RedCamera* pCamera)
{
    s_VisibleList.RemoveAll();
    if (!pCamera)
    {
        return;
    }
    for (PblList<ReflectionRegion>::Itor it = s_List.Begin(); it != s_List.End(); ++it)
    {
        if (it->_IsVisible(pCamera))
        {
            s_VisibleList.AddLast(&it->m_VisibleNode, it);
        }
    }
}

// FUNCTION: SWBF2 0x006bf3a0
bool ReflectionRegion::_IsVisible(const RedCamera* pCamera) const
{
    const float fMargin = 25.0f;
    switch (mType)
    {
    case type_sphere:
        return pCamera->ClassifySphere(*(const PblVector3*)&mMatrix.m[3][0],
                                       mSize.w + fMargin, -1.0f) != RedCamera::VISIBILITY_NONE;
    case type_box:
    case type_cylinder:
        return pCamera->ClassifyBox(mMatrix,
                     PblVector3(mSize.x + fMargin, mSize.y + fMargin, mSize.z + fMargin))
               != RedCamera::VISIBILITY_NONE;
    case type_plane:
        return pCamera->ClassifyPlane(*(const PblVector3*)&mMatrix.m[2][0], mSize.w + fMargin)
               != RedCamera::VISIBILITY_NONE;
    case type_box_aligned:
        return pCamera->ClassifyAABox(*(const PblVector3*)&mMatrix.m[3][0],
                     PblVector3(mSize.x + fMargin, mSize.y + fMargin, mSize.z + fMargin))
               != RedCamera::VISIBILITY_NONE;
    case type_cylinder_xaligned:
        return pCamera->ClassifyAABox(*(const PblVector3*)&mMatrix.m[3][0],
                     PblVector3(mSize.y + fMargin, mSize.x + fMargin, mSize.x + fMargin))
               != RedCamera::VISIBILITY_NONE;
    case type_cylinder_yaligned:
        return pCamera->ClassifyAABox(*(const PblVector3*)&mMatrix.m[3][0],
                     PblVector3(mSize.x + fMargin, mSize.y + fMargin, mSize.z + fMargin))
               != RedCamera::VISIBILITY_NONE;
    case type_cylinder_zaligned:
        return pCamera->ClassifyAABox(*(const PblVector3*)&mMatrix.m[3][0],
                     PblVector3(mSize.x + fMargin, mSize.x + fMargin, mSize.y + fMargin))
               != RedCamera::VISIBILITY_NONE;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x006bf850
void FLRenderer::pcReset(ResetFlagsT flags)
{
    if (flags & RESETFLAGS_FREE_RESOURCES)
    {
        if (FLRenderer::m_reflectionDepthBuffer)
        {
            FLRenderer::m_reflectionDepthBuffer->Release();
            FLRenderer::m_reflectionDepthBuffer = 0;
        }
        if (FLRenderer::m_FarSceneDepthBuffer)
        {
            FLRenderer::m_FarSceneDepthBuffer->Release();
            FLRenderer::m_FarSceneDepthBuffer = 0;
        }
    }
}

// FUNCTION: SWBF2 0x006bb0f0
ReflectionRegion::ReflectionRegion(RedRegionDesc& desc) : RedRegion(desc)
{
    switch (mType)
    {
    case type_box:
    case type_cylinder:
    case type_box_aligned:
    case type_cylinder_xaligned:
    case type_cylinder_yaligned:
    case type_cylinder_zaligned:
    {
        const float fHalfHeight = mSize.y;
        const float fX = mMatrix.m[3][0] - mMatrix.m[1][0] * fHalfHeight;
        const float fY = mMatrix.m[3][1] - mMatrix.m[1][1] * fHalfHeight;
        const float fZ = mMatrix.m[3][2] - mMatrix.m[1][2] * fHalfHeight;
        m_reflectionPlane.x = mMatrix.m[1][0];
        m_reflectionPlane.y = mMatrix.m[1][1];
        m_reflectionPlane.z = mMatrix.m[1][2];
        m_reflectionPlane.w = -(m_reflectionPlane.x * fX + m_reflectionPlane.y * fY +
                                m_reflectionPlane.z * fZ);
        break;
    }
    default:
        break;
    }

    _UpdateReflectionMatrix();
    s_List.AddLast(&m_Node, this);
}

// SYNTHETIC: SWBF2 0x006bb250
// ReflectionRegionFactory::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006bb300
void FLRenderer::AddInterfaceElement(unsigned int uiView, RedScreenGroupElement* pElement,
                                     float fX, float fY, bool bUseSafeZone)
{
    pElement->m_fScreenRelativeX = fX;
    pElement->m_fScreenRelativeY = fY;
    pElement->m_bUseSafeZone = bUseSafeZone;

    m_pViewInterfaceScreen[uiView]->AddElement(pElement);
}

// FUNCTION: SWBF2 0x006bb390
void FLRenderer::AddReflectionObject(char reflectionID, const PblBox& box, bool bVisible)
{
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const PblMatrix& mView = RedRenderer::GetCamera()->GetMatrixInverse();

    PblVector3 aPoints[32];
    if (!bVisible)
    {
        return;
    }

    PblVector3 vCorner;
    D3DXVec3TransformCoord(&aPoints[0], &box._vMin, &mView);
    vCorner = PblVector3(box._vMax.x, box._vMin.y, box._vMin.z);
    D3DXVec3TransformCoord(&aPoints[1], &vCorner, &mView);
    vCorner = PblVector3(box._vMax.x, box._vMin.y, box._vMax.z);
    D3DXVec3TransformCoord(&aPoints[2], &vCorner, &mView);
    vCorner = PblVector3(box._vMin.x, box._vMin.y, box._vMax.z);
    D3DXVec3TransformCoord(&aPoints[3], &vCorner, &mView);
    aPoints[0].z = -aPoints[0].z;
    aPoints[1].z = -aPoints[1].z;
    aPoints[2].z = -aPoints[2].z;
    aPoints[3].z = -aPoints[3].z;

    int nPoints = 4;
    PblVector3* pPoints = aPoints;
    PblVector3* pClipped;
    int nClipped;

    pClipped = pPoints + nPoints;
    nClipped = 0;
    if (ClipLoopToZPlanes(pClipped, nClipped, pPoints, nPoints, fNear, fFar))
    {
        nPoints = nClipped;
        if (!nPoints)
        {
            return;
        }
        pPoints = pClipped;
    }

    const float fInvNear = 1.0f / fNear;

    pClipped = pPoints + nPoints;
    nClipped = 0;
    if (ClipLoopToFrustumPlanes(pClipped, nClipped, pPoints, nPoints, COMPONENT_X,
                                -(fInvNear * fWidth * 0.5f), fInvNear * fWidth * 0.5f))
    {
        nPoints = nClipped;
        if (!nPoints)
        {
            return;
        }
        pPoints = pClipped;
    }

    pClipped = pPoints + nPoints;
    nClipped = 0;
    if (ClipLoopToFrustumPlanes(pClipped, nClipped, pPoints, nPoints, COMPONENT_Y,
                                -(fInvNear * fHeight * 0.5f), fInvNear * fHeight * 0.5f))
    {
        nPoints = nClipped;
        if (!nPoints)
        {
            return;
        }
        pPoints = pClipped;
    }

    const float fScaleX = (fNear * 2.0f) / fWidth;
    const float fScaleY = (fNear * 2.0f) / fHeight;
    PblBox& rScreenBox = m_curFrameReflections[reflectionID - 1].bbox;
    for (int i = 0; i < nPoints; i++)
    {
        const float fInvZ = 1.0f / pPoints[i].z;
        pPoints[i].x = fInvZ * fScaleX * pPoints[i].x;
        pPoints[i].y = fInvZ * fScaleY * pPoints[i].y;
        rScreenBox.AddPoint(pPoints[i]);
    }
}

// FUNCTION: SWBF2 0x006bba80
void FLRenderer::CaptureFrame(unsigned int uiView)
{
    if (!(RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES))
    {
        return;
    }
    if (!RedRenderer::pcGetRenderOptions().motionBlur)
    {
        return;
    }

    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);

    RECT rcSource;
    rcSource.left = (LONG)fMinX;
    rcSource.top = (LONG)fMinY;
    rcSource.right = (LONG)fMaxX;
    rcSource.bottom = (LONG)fMaxY;

    const RedViewport& rViewport = m_Viewport[uiView];
    RECT rcDest;
    rcDest.left = (LONG)((float)m_previousFrameTex->m_uiWidth * rViewport.fOffsetX);
    rcDest.top = (LONG)((float)m_previousFrameTex->m_uiHeight * rViewport.fOffsetY);
    rcDest.right = (LONG)((rViewport.fWidth + rViewport.fOffsetX) *
                          (float)m_previousFrameTex->m_uiWidth);
    rcDest.bottom = (LONG)((rViewport.fHeight + rViewport.fOffsetY) *
                           (float)m_previousFrameTex->m_uiHeight);

    IDirect3DSurface9* pBackBuffer;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    IDirect3DSurface9* pTarget;
    ((IDirect3DTexture9*)m_previousFrameTex->m_pTexture)->GetSurfaceLevel(0, &pTarget);
    RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &rcSource, pTarget, &rcDest,
                                            D3DTEXF_NONE);
    pBackBuffer->Release();
    pTarget->Release();
}

// FUNCTION: SWBF2 0x006bbe40
template void PblList<ReflectionRegion>::RemoveAll();

// FUNCTION: SWBF2 0x006bbe80
bool ClipLoopToFrustumPlanes(PblVector3* pOut, int& nOut, const PblVector3* pIn,
                             const int nIn, const COMPONENT eComponent,
                             const float fMin, const float fMax)
{
    int aiCode[32];
    int iAll = 0;
    for (int i = 0; i < nIn; i++)
    {
        const float fC = (&pIn[i].x)[eComponent];
        if (pIn[i].z * fMin > fC)
        {
            aiCode[i] = 1;
            iAll |= 1;
        }
        else if (fC > pIn[i].z * fMax)
        {
            aiCode[i] = 2;
            iAll |= 2;
        }
        else
        {
            aiCode[i] = 0;
        }
    }
    if (!iAll)
    {
        return false;
    }

    const int iOther = 1 - eComponent;
    nOut = 0;
    for (int i = 0; i < nIn; i++)
    {
        const int iNext = (i + 1) % nIn;
        if (!aiCode[i])
        {
            pOut[nOut++] = pIn[i];
        }

        const int iCross = aiCode[i] ^ aiCode[iNext];
        if (!iCross)
        {
            continue;
        }

        const float* pCur = &pIn[i].x;
        const float* pNext = &pIn[iNext].x;
        const float fDC = pCur[eComponent] - pNext[eComponent];
        const float fDZ = pIn[i].z - pIn[iNext].z;
        float fT;
        float* pDst;
        if (fDC >= 0.0f)
        {
            if (iCross & 2)
            {
                fT = (pCur[eComponent] - pIn[i].z * fMax) / (fDC - fDZ * fMax);
                pDst = &pOut[nOut].x;
                pOut[nOut].z = pIn[iNext].z * fT + (1.0f - fT) * pIn[i].z;
                pDst[eComponent] = pOut[nOut].z * fMax;
                pDst[iOther] = pNext[iOther] * fT + (1.0f - fT) * pCur[iOther];
                nOut++;
            }
            if (iCross & 1)
            {
                fT = (pCur[eComponent] - pIn[i].z * fMin) / (fDC - fDZ * fMin);
                pDst = &pOut[nOut].x;
                pOut[nOut].z = (1.0f - fT) * pIn[i].z + pIn[iNext].z * fT;
                pDst[eComponent] = pOut[nOut].z * fMin;
                pDst[iOther] = pNext[iOther] * fT + (1.0f - fT) * pCur[iOther];
                nOut++;
            }
        }
        else
        {
            if (iCross & 1)
            {
                fT = (pCur[eComponent] - pIn[i].z * fMin) / (fDC - fDZ * fMin);
                pDst = &pOut[nOut].x;
                pOut[nOut].z = pIn[iNext].z * fT + (1.0f - fT) * pIn[i].z;
                pDst[eComponent] = pOut[nOut].z * fMin;
                pDst[iOther] = pNext[iOther] * fT + (1.0f - fT) * pCur[iOther];
                nOut++;
            }
            if (iCross & 2)
            {
                fT = (pCur[eComponent] - pIn[i].z * fMax) / (fDC - fDZ * fMax);
                pDst = &pOut[nOut].x;
                pOut[nOut].z = pIn[iNext].z * fT + (1.0f - fT) * pIn[i].z;
                pDst[eComponent] = pOut[nOut].z * fMax;
                pDst[iOther] = pNext[iOther] * fT + (1.0f - fT) * pCur[iOther];
                nOut++;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006bc2f0
bool ClipLoopToZPlanes(PblVector3* pOut, int& nOut, const PblVector3* pIn,
                       const int nIn, const float fNear, const float fFar)
{
    int aiCode[32];
    int iAll = 0;
    for (int i = 0; i < nIn; i++)
    {
        if (fNear > pIn[i].z)
        {
            aiCode[i] = 1;
            iAll |= 1;
        }
        else if (pIn[i].z > fFar)
        {
            aiCode[i] = 2;
            iAll |= 2;
        }
        else
        {
            aiCode[i] = 0;
        }
    }
    if (!iAll)
    {
        return false;
    }

    nOut = 0;
    for (int i = 0; i < nIn; i++)
    {
        const int iNext = (i + 1) % nIn;
        if (!aiCode[i])
        {
            pOut[nOut++] = pIn[i];
        }

        const int iCross = aiCode[i] ^ aiCode[iNext];
        if (!iCross)
        {
            continue;
        }

        const float fInvDZ = 1.0f / (pIn[i].z - pIn[iNext].z);
        float fT;
        if (0.0f > fInvDZ)
        {
            if (iCross & 1)
            {
                pOut[nOut].z = fNear;
                fT = (pIn[i].z - fNear) * fInvDZ;
                pOut[nOut].x = pIn[i].x * (1.0f - fT) + pIn[iNext].x * fT;
                pOut[nOut].y = pIn[i].y * (1.0f - fT) + pIn[iNext].y * fT;
                nOut++;
            }
            if (iCross & 2)
            {
                pOut[nOut].z = fFar;
                fT = (pIn[i].z - fFar) * fInvDZ;
                pOut[nOut].x = pIn[i].x * (1.0f - fT) + pIn[iNext].x * fT;
                pOut[nOut].y = pIn[i].y * (1.0f - fT) + pIn[iNext].y * fT;
                nOut++;
            }
        }
        else
        {
            if (iCross & 2)
            {
                pOut[nOut].z = fFar;
                fT = (pIn[i].z - fFar) * fInvDZ;
                pOut[nOut].x = pIn[i].x * (1.0f - fT) + pIn[iNext].x * fT;
                pOut[nOut].y = pIn[i].y * (1.0f - fT) + pIn[iNext].y * fT;
                nOut++;
            }
            if (iCross & 1)
            {
                pOut[nOut].z = fNear;
                fT = (pIn[i].z - fNear) * fInvDZ;
                pOut[nOut].x = pIn[i].x * (1.0f - fT) + pIn[iNext].x * fT;
                pOut[nOut].y = pIn[i].y * (1.0f - fT) + pIn[iNext].y * fT;
                nOut++;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006bc5a0
void* ReflectionRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    return new ReflectionRegion(desc);
}

// FUNCTION: SWBF2 0x006bc610
void FLRenderer::EnableCursor(bool bEnable)
{
    mCursorGroup.Enable(bEnable);
}

// FUNCTION: SWBF2 0x006bc630
unsigned int FLRenderer::GetCurrentViewport()
{
    return mView;
}

// FUNCTION: SWBF2 0x006bc640
bool FLRenderer::GetFadeAdjustWithZoom()
{
    return m_bEnableFadeAdjustWithZoom;
}

// FUNCTION: SWBF2 0x006bc650
float FLRenderer::GetFarSceneRange()
{
    float fFactor = RedRenderer::pcGetRenderOptions().mFarSceneRangeFactor;
    if (0.0f > fFactor)
    {
        fFactor = 0.0f;
    }
    else if (fFactor > 1.0f)
    {
        fFactor = 1.0f;
    }
    return (m_fViewFarPlane[1] - m_fViewFarPlane[0]) * fFactor + m_fViewFarPlane[0];
}

// FUNCTION: SWBF2 0x006bc6a0
RedInterfaceScreen* FLRenderer::GetInterfaceView(int iView)
{
    return m_pViewInterfaceScreen[iView];
}

// FUNCTION: SWBF2 0x006bc710
float FLRenderer::GetNearSceneFadeStart()
{
    float fFactor = RedRenderer::pcGetRenderOptions().mNearSceneFadeFactor;
    if (0.0f > fFactor)
    {
        fFactor = 0.0f;
    }
    else if (fFactor > 1.0f)
    {
        fFactor = 1.0f;
    }
    return (m_fViewNearSceneFadeStart[1] - m_fViewNearSceneFadeStart[0]) * fFactor +
           m_fViewNearSceneFadeStart[0];
}

// FUNCTION: SWBF2 0x006bc760
unsigned int FLRenderer::GetViewHeight(unsigned int uiView)
{
    return m_pViewInterfaceScreen[uiView]->m_uiHeightInPixels;
}

// FUNCTION: SWBF2 0x006bc780
unsigned int FLRenderer::GetViewWidth(unsigned int uiView)
{
    return m_pViewInterfaceScreen[uiView]->m_uiWidthInPixels;
}

// FUNCTION: SWBF2 0x006bcc40
void FLRenderer::InitCursor(unsigned int uiTextureHash, bool bInit)
{
    if (bInit)
    {
        mCursorImage.Init();
    }

    mCursorImage.SetBlendMode(RedInterfaceElement::BLEND_ALPHA);
    mCursorImage.m_color.a = 0xff;
    mCursorImage.SetTexture(uiTextureHash);
    mCursorImage.SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);

    const RedTexture* pTexture = RedTexture::Find(uiTextureHash);
    if (pTexture)
    {
        mCursorImage.SetRect(0.0f, 0.0f, (float)pTexture->m_uiWidth, (float)pTexture->m_uiHeight);
    }
    else
    {
        mCursorImage.SetRect(0.0f, 0.0f, 32.0f, 32.0f);
    }
    mCursorImage.Enable(true);

    mCursorGroup.AddElement(&mCursorImage);
    mCursorGroup.BringToFront();
    AddInterfaceElement(&mCursorGroup, 0.0f, 0.0f, false);
}

// FUNCTION: SWBF2 0x006befd0
void FLRenderer::SetCursorPos(float fX, float fY)
{
    mCursorImage.SetPosition(fX, fY);
    RedColor white;
    white.b = white.g = white.r = white.a = 0xff;
    mCursorImage.SetColor(white);
    mCursorGroup.SetColor(white);
}

// FUNCTION: SWBF2 0x006bcd70
bool FLRenderer::IsReflected(const PblVector3& vPoint, float fRadius, PblMatrix& mReflection,
                             bool bAllRegions)
{
    return ReflectionRegion::IsReflected(vPoint, fRadius, mReflection, bAllRegions);
}

// FUNCTION: SWBF2 0x006bcda0
bool ReflectionRegion::IsReflected(const PblVector3& vPoint, float fRadius,
                                   PblMatrix& mReflection, bool bAllRegions)
{
    if (!bReflectionsEnable)
    {
        return false;
    }

    PblList<ReflectionRegion>& rList = bAllRegions ? s_List : s_VisibleList;
    for (PblList<ReflectionRegion>::Node* pNode = rList._head._pNext;
         pNode->_pObject != rList._head._pObject; pNode = pNode->_pNext)
    {
        if (pNode->_pObject->IsPointInside(vPoint))
        {
            mReflection = pNode->_pObject->m_reflectionMat;
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006bce90
void InitMouseCursor()
{
    FLRenderer::InitCursor(PblHash("mousecursor"), true);
}

// FUNCTION: SWBF2 0x006bcec0
char FLRenderer::RegisterReflection(Renderable* pRenderable, const PblPlane& plane,
                                    unsigned int uiRenderFlags)
{
    if (m_numCurFrameReflections >= 255)
    {
        return -1;
    }

    ReflectionT& rReflection = m_curFrameReflections[m_numCurFrameReflections];
    rReflection.renderable = pRenderable;
    rReflection.reflectionPlane = plane;
    rReflection.reflectionID = (char)(m_numCurFrameReflections + 1);
    rReflection.bbox._vMin.x = FLT_MAX;
    rReflection.bbox._vMin.y = FLT_MAX;
    rReflection.bbox._vMin.z = FLT_MAX;
    rReflection.bbox._vMax.x = -FLT_MAX;
    rReflection.bbox._vMax.y = -FLT_MAX;
    rReflection.bbox._vMax.z = -FLT_MAX;
    rReflection.renderFlags = uiRenderFlags;
    m_numCurFrameReflections++;
    return m_curFrameReflections[m_numCurFrameReflections - 1].reflectionID;
}

// FUNCTION: SWBF2 0x006bcf80
void FLRenderer::RemoveInterfaceElement(unsigned int uiView, RedScreenGroupElement* pElement)
{
    m_pViewInterfaceScreen[uiView]->RemoveElement(pElement);
}

// FUNCTION: SWBF2 0x006bcfa0
void FLRenderer::RemoveInterfaceElement(RedScreenGroupElement* pElement)
{
    RedInterfaceScreen::s_screenFull.RemoveElement(pElement);
}

// FUNCTION: SWBF2 0x006bd040
void FLRenderer::RemoveView(unsigned int uiView)
{
    m_pViewCamera[uiView] = 0;
    while (mViewCount && !m_pViewCamera[mViewCount - 1])
    {
        mViewCount--;
    }
}

// GLOBAL: SWBF2 0x008f6d90
int ZPREPASS_SCENE_IMMEDIATE_MODE;
// GLOBAL: SWBF2 0x008f6d94
int NEAR_SCENE_IMMEDIATE_MODE;

// GLOBAL: SWBF2 0x00772890

static const RedViewport sFullScreenViewport = { 0.0f, 0.0f, 1.0f, 1.0f };

// GLOBAL: SWBF2 0x00784cb8
const unsigned int pcStencilShadowShader::s_stencilRef = 0;

// GLOBAL: SWBF2 0x01e56137

bool g_InGameMoviePlayerUpdateEnable;

// FUNCTION: SWBF2 0x006bd080
void FLRenderer::Render()
{
    RedRenderer::BeginFrame();
    if (!g_bNoRender)
    {
        RedColor clearColor = RedRenderer::pcGetClearColor();
        clearColor.a = 0;
        RedRenderer::pcClear(D3DCLEAR_TARGET, clearColor, 1.0f, 0);
        RedRenderer::SetFogColor(m_colorFog);
        RedRenderer::SetFogRange(fl_fog_start, fl_fog_end);
        RedRenderer::SetWorldFogRange(FLRenderer::m_fWorldFogStart, FLRenderer::m_fWorldFogEnd);
        if (RedRenderer::pcGetRenderCaps().pipeline != PIPELINE_FIXED)
        {
            RedRenderer::SetFogEnable(FLRenderer::m_fWorldFogStart > FLRenderer::m_fWorldFogEnd || fl_fog_end > fl_fog_start);
        }
        else
        {
            RedRenderer::SetFogEnable(fl_fog_end > fl_fog_start);
        }

        for (mView = 0; mView < mViewCount; mView++)
        {
            RedCamera* pCamera = m_pViewCamera[mView];
            if (!pCamera)
            {
                continue;
            }

            if (m_bSceneEnable[mView])
            {
                float fZoom = 1.0f;
                if (GetFadeAdjustWithZoom())
                {
                    fZoom = pCamera->GetZoom();
                }

                PblMatrix mSaved;
                if (fZoom != 1.0f)
                {
                    mSaved = pCamera->GetMatrix();
                    PblMatrix mPushed = mSaved;
                    float fPush = GetNearPlane() * (fZoom - 1.0f);
                    mPushed.m[3][0] = mSaved.m[3][0] + mSaved.m[2][0] * fPush;
                    mPushed.m[3][1] = mSaved.m[3][1] + mSaved.m[2][1] * fPush;
                    mPushed.m[3][2] = mSaved.m[3][2] + mSaved.m[2][2] * fPush;
                    mPushed.m[3][3] = 1.0f;
                    pCamera->SetMatrix(mPushed);
                    m_pViewCamera[mView]->SetNearFarPlane(GetNearPlane() * fZoom,
                                                          GetNearSceneFadeEnd() * fZoom);
                    RedRenderer::pcSetNearSceneStart(GetNearSceneFadeStart() * fZoom);
                    RedRenderer::pcSetNearSceneEnd(GetNearSceneFadeEnd() * fZoom);
                }

                RedRenderer::pcSetViewport(m_Viewport[mView]);
                RedRenderer::SetCamera(m_pViewCamera[mView]);
                RedSceneManager::GetInstance().Update(*m_pViewCamera[mView]);
                RedLight::UpdateAllVisibility(0, mView);
                RenderReflections();
                RenderFarScene(mView);
                _RenderableLists[2].Render();
                RedRenderer::pcClear(D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, RedColor::BLACK, 1.0f,
                                     pcStencilShadowShader::s_stencilRef);
                RedRenderer::BeginScene();
                RedRenderer::pcSetNearScene(true);
                RedLight::UpdateAllVisibility(m_pViewCamera[mView], mView);
                ReflectionRegion::UpdateAllVisibility(m_pViewCamera[mView]);
                RedStencilShadow::SetVisibilityTestCamera(m_pViewCamera[mView]);

                bool bShadowMap;
                if (RedShadowMap::IsShadowMapEnabled() && RedStencilShadow::GetShadowEnable())
                {
                    bShadowMap = true;
                    RedRenderer::pcEnableScreenSpaceShadow(true);
                    RedRenderer::pcSetImmediateMode(ZPREPASS_SCENE_IMMEDIATE_MODE);
                }
                else
                {
                    bShadowMap = false;
                    RedRenderer::pcSetImmediateMode(NEAR_SCENE_IMMEDIATE_MODE);
                }
                RedTerrain::Render(0);
                if (!bShadowMap)
                {
                    RedRenderer::pcFlush(1);
                }
                RedSceneManager::Render(0, 0);
                _RenderableLists[3].Render();
                RedRenderer::pcEnableScreenSpaceShadow(false);
                RedRenderer::pcSetImmediateMode(0);
                RedRenderer::pcFlush(7);
                _RenderableLists[4].Render();
                RedRenderer::pcFlush(8);
                _RenderableLists[5].Render();
                pcUpdateRefractionBuffer();
                RedRenderer::FlushOcclusionQueries();
                RedRenderer::pcFlush(0x10);
                RedRenderer::pcFlush(0x20);
                _RenderableLists[6].Render();
                _RenderableLists[7].Render();
                RedRenderer::Flush();
                RedRenderer::pcClear(D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, RedColor::BLACK, 1.0f,
                                     pcStencilShadowShader::s_stencilRef);

                if (fZoom != 1.0f)
                {
                    m_pViewCamera[mView]->SetMatrix(mSaved);
                }
                m_pViewCamera[mView]->SetNearFarPlane(0.3f, GetNearSceneFadeEnd());
                RedRenderer::SetCamera(m_pViewCamera[mView]);

                bool bShadowMapEnabled = RedShadowMap::IsShadowMapEnabled();
                if (bShadowMap)
                {
                    RedRenderer::pcEnableScreenSpaceShadow(false);
                    if (bShadowMapEnabled)
                    {
                        RedShadowMap::_bShadowMapEnable = false;
                    }
                }
                _RenderableLists[8].Render();
                RedRenderer::Flush();
                if (bShadowMap)
                {
                    RedRenderer::pcEnableScreenSpaceShadow(false);
                    if (bShadowMapEnabled)
                    {
                        RedShadowMap::_bShadowMapEnable = true;
                    }
                }

                m_pViewCamera[mView]->SetNearFarPlane(GetNearPlane(), GetNearSceneFadeEnd());
                RedRenderer::pcSetNearSceneStart(GetNearSceneFadeStart());
                RedRenderer::pcSetNearSceneEnd(GetNearSceneFadeEnd());
                _RenderableLists[9].Render();
                RedRenderer::Flush();
                RedRenderer::pcSetNearScene(false);
                CaptureFrame(mView);
                RedRenderer::Flush();
                RedRenderer::EndScene();
            }
            else
            {
                RedRenderer::pcSetViewport(m_Viewport[mView]);
                RedRenderer::SetCamera(m_pViewCamera[mView]);
                RedRenderer::pcClear(D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, RedColor::BLACK, 1.0f,
                                     pcStencilShadowShader::s_stencilRef);
            }

            float fLightingScale = RedRenderer::GetLightingScale();
            bool bFogEnable = RedRenderer::GetFogEnable();
            RedRenderer::SetLightingScale(1.0f);
            RedRenderer::SetFogEnable(false);
            RedRenderer::pcSetNearSceneStart(0.0f);
            RedRenderer::pcSetNearSceneEnd(0.0f);
            RedRenderer::BeginScene();
            if (g_InGameMoviePlayerUpdateEnable)
            {
                UpdatInGameMovie();
            }
            GameMovie::Render();
            GameMovie::EndFrame();
            RedRenderer::EndScene();
            if (gEnable && FLRenderer::m_bInterfaceEnable[mView])
            {
                RedRenderer::pcSetNearScene(true);
                m_pViewInterfaceScreen[mView]->Render();
                RedRenderer::pcSetNearScene(false);
            }
            RedRenderer::BeginScene();
            RenderViewTransition(mView);
            RedRenderer::EndScene();
            RedRenderer::SetLightingScale(fLightingScale);
            RedRenderer::SetFogEnable(bFogEnable);
            RedRenderer::pcSetNearSceneStart(GetNearSceneFadeStart());
            RedRenderer::pcSetNearSceneEnd(GetNearSceneFadeEnd());
        }

        RedRenderer::pcSetViewport(sFullScreenViewport);
        mCursorGroup.BringToFront();
        RedRenderer::pcSetNearScene(true);
        float fLightingScale = RedRenderer::GetLightingScale();
        bool bFogEnable = RedRenderer::GetFogEnable();
        RedRenderer::SetLightingScale(1.0f);
        RedRenderer::SetFogEnable(false);
        RedRenderer::pcSetNearSceneStart(0.0f);
        RedRenderer::pcSetNearSceneEnd(0.0f);
        if (gEnable)
        {
            RedInterfaceScreen::s_screenFull.Render();
        }
        RedRenderer::Flush();
        RenderViewTransition(1);
        RedRenderer::SetLightingScale(fLightingScale);
        RedRenderer::SetFogEnable(bFogEnable);
        RedRenderer::pcSetNearSceneStart(GetNearSceneFadeStart());
        RedRenderer::pcSetNearSceneEnd(GetNearSceneFadeEnd());
        RedRenderer::pcSetNearScene(false);
    }
    RedRenderer::EndFrame();
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

static inline void Vec3Cross_(PblVector3& rOut, const PblVector3& a, const PblVector3& b)
{
    rOut.x = a.y * b.z - a.z * b.y;
    rOut.y = a.z * b.x - a.x * b.z;
    rOut.z = a.x * b.y - a.y * b.x;
}

// FUNCTION: SWBF2 0x006bd910
void FLRenderer::RenderableList::Render() const
{
    for (const Node* pNode = _head._pNext; pNode->_pObject; pNode = pNode->_pNext)
    {
        pNode->_pObject->Render(0);
    }
}

// FUNCTION: SWBF2 0x006bd940
void FLRenderer::RenderFarScene(unsigned int uiView)
{
    if (!_RenderableLists[0]._iCount)
    {
        return;
    }

    RedViewport oldViewport;
    RedRenderer::pcGetViewport(oldViewport);
    RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();

    RedTexture* pSky = RedRenderer::pcGetBuffer(PC_BUFFER_SKY);
    if (!FLRenderer::m_FarSceneDepthBuffer)
    {
        RedRenderer::pcGetDevice()->CreateDepthStencilSurface(
            pSky->m_uiWidth, pSky->m_uiHeight, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE,
            &FLRenderer::m_FarSceneDepthBuffer, 0);
    }
    RedRenderer::pcSetRenderTarget(pSky, FLRenderer::m_FarSceneDepthBuffer);
    RedRenderer::pcSetViewport(oldViewport);
    RedRenderer::pcClear(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, RedColor::BLACK,
                         1.0f, pcStencilShadowShader::s_stencilRef);

    float fFogStart, fFogEnd;
    RedRenderer::GetFogRange(fFogStart, fFogEnd);
    float fWorldFogStart, fWorldFogEnd;
    RedRenderer::GetWorldFogRange(fWorldFogStart, fWorldFogEnd);
    bool bFogEnable = RedRenderer::GetFogEnable();
    RedColor fogColor = RedRenderer::GetFogColor();
    bool bDistort = RedParticleRenderer::s_blurEnable;
    RedParticleRenderer::s_blurEnable = false;

    RedRenderer::SetFogColor(m_colorFog);
    RedRenderer::SetFogRange(fl_fog_start, fl_fog_end);
    RedRenderer::SetWorldFogRange(FLRenderer::m_fWorldFogStart, FLRenderer::m_fWorldFogEnd);
    if (RedRenderer::pcGetRenderCaps().pipeline != PIPELINE_FIXED)
    {
        RedRenderer::SetFogEnable(FLRenderer::m_fWorldFogStart > FLRenderer::m_fWorldFogEnd || fl_fog_end > fl_fog_start);
    }
    else
    {
        RedRenderer::SetFogEnable(fl_fog_end > fl_fog_start);
    }

    float fNearSceneStart = RedRenderer::pcGetNearSceneStart();
    float fNearSceneEnd = RedRenderer::pcGetNearSceneEnd();
    RedRenderer::pcSetNearSceneStart(0.0f);
    RedRenderer::pcSetNearSceneEnd(0.0f);

    float fZoom = 1.0f;
    if (GetFadeAdjustWithZoom())
    {
        fZoom = m_pViewCamera[mView]->GetZoom();
    }
    if (sFarSceneRangeEnabled[uiView])
    {
        m_pViewCamera[uiView]->SetNearFarPlane(2.0f, GetFarSceneRange());
    }

    RedRenderer::SetCamera(m_pViewCamera[uiView]);
    RedRenderer::pcSetImmediateMode(FAR_SCENE_IMMEDIATE_MODE);
    RedRenderer::BeginScene();
    _RenderableLists[0].Render();
    RedRenderer::Flush();
    if (m_bFarSceneEnable[0] && sFarSceneRangeEnabled[uiView])
    {
        RedSceneManager::Render(0x60000, 0);
    }
    RedRenderer::Flush();
    _RenderableLists[1].Render();
    RedRenderer::EndScene();
    RedRenderer::pcSetImmediateMode(0);

    RedRenderer::SetFogEnable(bFogEnable);
    RedParticleRenderer::s_blurEnable = bDistort;
    RedRenderer::SetFogColor(fogColor);
    RedRenderer::SetFogRange(fFogStart, fFogEnd);
    RedRenderer::SetWorldFogRange(fWorldFogStart, fWorldFogEnd);
    RedRenderer::pcSetNearSceneStart(fNearSceneStart);
    RedRenderer::pcSetNearSceneEnd(fNearSceneEnd);
    m_pViewCamera[uiView]->SetNearFarPlane(GetNearPlane() * fZoom, GetNearSceneFadeEnd() * fZoom);
    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::pcSetViewport(oldViewport);
}

// FUNCTION: SWBF2 0x006bdc90
void FLRenderer::RenderReflections()
{
    if (!enableReflection || !FLRenderer::m_numPotentialReflections)
    {
        return;
    }

    if (m_reflectionRenderTarget && m_reflectionRenderTarget->m_format != D3DFMT_A8R8G8B8)
    {
        delete m_reflectionRenderTarget;
        m_reflectionRenderTarget = 0;
        if (FLRenderer::m_reflectionDepthBuffer)
        {
            FLRenderer::m_reflectionDepthBuffer->Release();
            FLRenderer::m_reflectionDepthBuffer = 0;
        }
    }
    if (!m_reflectionRenderTarget)
    {
        m_reflectionRenderTarget = new RedTexture(512, 256, D3DFMT_A8R8G8B8, 1,
                                                  D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, 0);
    }
    if (!FLRenderer::m_reflectionDepthBuffer)
    {
        RedRenderer::pcGetDevice()->CreateDepthStencilSurface(
            512, 256, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &FLRenderer::m_reflectionDepthBuffer, 0);
    }

    bool bShadowMap = RedShadowMap::IsShadowMapEnabled();
    bool bDistort = RedParticleRenderer::s_blurEnable;
    if (bShadowMap)
    {
        RedShadowMap::_bShadowMapEnable = false;
    }
    RedParticleRenderer::s_blurEnable = false;

    RedViewport oldViewport;
    RedRenderer::pcGetViewport(oldViewport);
    RedTexture* pOldTarget = RedRenderer::pcGetRenderTarget();
    IDirect3DSurface9* pOldDepth = RedRenderer::pcGetDepthTarget();
    RedRenderer::pcSetRenderTarget(m_reflectionRenderTarget, FLRenderer::m_reflectionDepthBuffer);
    RedRenderer::pcClear(D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, RedColor::BLACK, 0.0f,
                         pcStencilShadowShader::s_stencilRef);

    const RedCamera* pCamera = RedRenderer::GetCamera();
    RedCamera reflectCamera(*pCamera);

    float fZoom = 1.0f;
    if (GetFadeAdjustWithZoom())
    {
        fZoom = pCamera->GetZoom();
    }

    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    float fTanX = fWidth / (fNear * 2.0f);
    float fTanY = fHeight / (fNear * 2.0f);
    if (GetFarSceneRange() > fFar)
    {
        fFar = GetFarSceneRange();
    }
    reflectCamera.SetNearFarPlane(fNear, fFar);
    RedRenderer::SetCamera(&reflectCamera);

    m_numCurFrameReflections = 0;
    RedRenderer::pcSetImmediateMode(1);
    RedRenderer::BeginScene();
    for (unsigned int i = 0; i < FLRenderer::m_numPotentialReflections; ++i)
    {
        FLRenderer::m_potentialReflections[i]->Render(0x100000);
    }
    RedRenderer::EndScene();
    RedRenderer::pcSetImmediateMode(0);

    bool bAnyVisible = false;
    for (unsigned int i = 0; i < m_numCurFrameReflections; ++i)
    {
        ReflectionT& r = m_curFrameReflections[i];
        if (r.bbox._vMin.x < 1.0f && r.bbox._vMax.x > -1.0f &&
            r.bbox._vMin.y < 1.0f && r.bbox._vMax.y > -1.0f &&
            r.bbox._vMin.z < fFar && r.bbox._vMax.z > fNear)
        {
            bAnyVisible = true;
        }
        else
        {
            r.reflectionID = 0;
        }
    }

    if (!bAnyVisible)
    {
        RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
        RedRenderer::pcSetViewport(oldViewport);
        if (bShadowMap)
        {
            RedShadowMap::_bShadowMapEnable = true;
        }
        RedParticleRenderer::s_blurEnable = bDistort;
        return;
    }

    RedCamera clipCamera;
    for (unsigned int i = 0; i < m_numCurFrameReflections; ++i)
    {
        ReflectionT& r = m_curFrameReflections[i];
        if (!r.reflectionID)
        {
            continue;
        }

        const PblPlane& plane = r.reflectionPlane;
        const PblMatrix& mCamera = pCamera->GetMatrix();
        const RedCamera* pSceneCamera = 0;
        bool bClipPlane;
        if (mCamera.m[3][1] * plane.y + plane.x * mCamera.m[3][0] + mCamera.m[3][2] * plane.z +
                plane.w > 0.0f)
        {
            RedRenderer::pcSetFrontFace(PCREDFRONT_CW);

            PblMatrix mReflect = gMatrixIdentity;
            float fInvLenSq = 1.0f / (plane.y * plane.y + plane.x * plane.x + plane.z * plane.z);
            float fScale = fInvLenSq * -2.0f;
            mReflect.m[0][0] = (plane.y * plane.y - plane.x * plane.x + plane.z * plane.z) * fInvLenSq;
            mReflect.m[0][1] = fScale * plane.x * plane.y;
            mReflect.m[0][2] = fScale * plane.x * plane.z;
            mReflect.m[1][0] = fScale * plane.x * plane.y;
            mReflect.m[1][1] = (plane.x * plane.x - plane.y * plane.y + plane.z * plane.z) * fInvLenSq;
            mReflect.m[1][2] = plane.y * fScale * plane.z;
            mReflect.m[2][0] = plane.x * fScale * plane.z;
            mReflect.m[2][1] = plane.y * fScale * plane.z;
            mReflect.m[2][2] = (plane.y * plane.y + plane.x * plane.x - plane.z * plane.z) * fInvLenSq;
            mReflect.m[3][0] = plane.x * fScale * plane.w;
            mReflect.m[3][1] = plane.y * fScale * plane.w;
            mReflect.m[3][2] = plane.z * fScale * plane.w;
            bClipPlane = true;

            PblMatrix mReflected;
            D3DXMatrixMultiply(&mReflected, &mCamera, &mReflect);
            reflectCamera.SetMatrix(mReflected);

            float fFadeEnd = GetNearSceneFadeEnd() * fZoom;
            const PblVector3& vMin = r.bbox._vMin;
            const PblVector3& vMax = r.bbox._vMax;
            if (vMin.x > -0.9f || 0.9f > vMax.x || vMin.y > -0.9f || 0.9f > vMax.y)
            {

                float fHalfX = (vMax.x - vMin.x) * 0.5f * fTanX;
                float fHalfY = (vMax.y - vMin.y) * 0.5f * fTanY;
                PblVector3 vCentre;
                vCentre.x = -((vMin.x + vMax.x) * 0.5f * fTanX);
                vCentre.y = -((vMin.y + vMax.y) * 0.5f * fTanY);
                vCentre.z = 1.0f;

                PblMatrix mBasis;
                mBasis.m[3][0] = 0.0f;
                mBasis.m[3][1] = 0.0f;
                mBasis.m[3][2] = 0.0f;
                mBasis.m[3][3] = 1.0f;
                PblVector3 vDir;
                D3DXVec3Normalize(&vDir, &vCentre);
                mBasis.m[2][0] = vDir.x;
                mBasis.m[2][1] = vDir.y;
                mBasis.m[2][2] = vDir.z;
                mBasis.m[2][3] = 0.0f;

                PblVector3 vRight, vUp, vCross;
                if (fHalfX > fHalfY)
                {
                    Vec3Cross_(vCross, vDir, gVectorUnitX);
                    D3DXVec3Normalize(&vUp, &vCross);
                    Vec3Cross_(vRight, vUp, vDir);
                }
                else
                {
                    Vec3Cross_(vCross, gVectorUnitY, vDir);
                    D3DXVec3Normalize(&vRight, &vCross);
                    Vec3Cross_(vUp, vDir, vRight);
                }
                mBasis.m[0][0] = vRight.x;
                mBasis.m[0][1] = vRight.y;
                mBasis.m[0][2] = vRight.z;
                mBasis.m[0][3] = 0.0f;
                mBasis.m[1][0] = vUp.x;
                mBasis.m[1][1] = vUp.y;
                mBasis.m[1][2] = vUp.z;
                mBasis.m[1][3] = 0.0f;

                float fEdgeY = fabs(vMax.y) > fabs(vMin.y) ? vMax.y : vMin.y;
                float fNearPlane = vMin.z;
                float fCos = (vDir.y * -0.0f - fEdgeY * fTanY * vDir.x) + vDir.z;

                PblMatrix mClip;
                D3DXMatrixMultiply(&mClip, &mBasis, &reflectCamera.GetMatrix());
                clipCamera.SetMatrix(mClip);
                clipCamera.SetFrustum(fNearPlane, fCos * fFadeEnd, fHalfX * 2.0f * fNearPlane,
                                      fHalfY * 2.0f * fNearPlane);
            }
            else
            {
                float fNearPlane = vMin.z;
                clipCamera.SetMatrix(reflectCamera.GetMatrix());
                clipCamera.SetFrustum(vMin.z, fFadeEnd, fWidth, fHeight);
                clipCamera.SetFrustum(fNearPlane, fFadeEnd, fTanX * 2.0f * fNearPlane,
                                      fTanY * 2.0f * fNearPlane);
            }
            pSceneCamera = &clipCamera;
        }
        else
        {
            RedRenderer::pcSetFrontFace(PCREDFRONT_CCW);
            bClipPlane = false;
            reflectCamera.SetMatrix(mCamera);
        }

        _RenderableLists[2].RenderReflections(0);
        RedRenderer::pcFlush(0);

        bool bStencilShadows = RedStencilShadow::GetShadowEnable();
        bool bShadowMapOff;
        if (bStencilShadows && RedShadowMap::IsShadowMapEnabled())
        {
            bShadowMapOff = true;
            RedShadowMap::_bShadowMapEnable = false;
        }
        else
        {
            bShadowMapOff = false;
        }
        if (bStencilShadows)
        {
            RedStencilShadow::s_bShadowsEnabled = false;
        }

        RedRenderer::pcSetUserNearClipPlane(r.reflectionPlane);
        RedRenderer::pcSetUserNearClipPlaneEnable(bClipPlane);

        bool bFarScene = GetFarSceneRange() > GetNearSceneFadeStart();
        bool bFogEnable = RedRenderer::GetFogEnable();
        float fFogStart, fFogEnd;
        RedRenderer::GetFogRange(fFogStart, fFogEnd);
        float fWorldFogStart, fWorldFogEnd;
        RedRenderer::GetWorldFogRange(fWorldFogStart, fWorldFogEnd);
        RedColor fogColor = RedRenderer::GetFogColor();
        if (!bFarScene)
        {
            RedRenderer::SetFogEnable(true);
            RedRenderer::SetFogRange(GetNearSceneFadeStart() * fZoom, GetNearSceneFadeEnd() * fZoom);
            RedRenderer::SetWorldFogRange(0.0f, 0.0f);
            RedRenderer::SetFogColor(FLRenderer::m_colorReflectionFog);
        }

        RedRenderer::pcSetImmediateMode(REFLECTION_SCENE_IMMEDIATE_MODE);
        RedRenderer::BeginScene();
        _RenderableLists[0].RenderReflections(r.renderFlags);
        if (!(r.renderFlags & 0x400000))
        {
            RedRenderer::pcSetNearScene(true);
            if (bFarScene)
            {
                RedSceneManager::Render(0x20200000, pSceneCamera);
            }
            else
            {
                RedTerrain::Render(0x200000);
                RedSceneManager::Render(0x200000, pSceneCamera);
            }
            _RenderableLists[3].RenderReflections(0);
        }
        RedRenderer::EndScene();
        RedRenderer::pcSetNearScene(false);
        RedRenderer::pcSetImmediateMode(0);

        if (!bFarScene)
        {
            RedRenderer::SetFogEnable(bFogEnable);
            RedRenderer::SetFogRange(fFogStart, fFogEnd);
            RedRenderer::SetWorldFogRange(fWorldFogStart, fWorldFogEnd);
            RedRenderer::SetFogColor(fogColor);
        }
        if (bShadowMapOff)
        {
            RedShadowMap::_bShadowMapEnable = true;
        }
        if (bStencilShadows)
        {
            RedStencilShadow::s_bShadowsEnabled = true;
        }
        RedRenderer::pcSetUserNearClipPlaneEnable(false);
    }

    RedRenderer::pcSetFrontFace(PCREDFRONT_CCW);
    RedRenderer::pcSetRenderTarget(pOldTarget, pOldDepth);
    RedRenderer::SetCamera(pCamera);
    RedRenderer::pcSetViewport(oldViewport);
    if (bShadowMap)
    {
        RedShadowMap::_bShadowMapEnable = true;
    }
    RedParticleRenderer::s_blurEnable = bDistort;
}

// FUNCTION: SWBF2 0x006beb00
void RenderViewTransition(int iView)
{
    RedViewport viewport;
    RedRenderer::pcGetViewport(viewport);
    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);
    float fWidth = fMaxX - fMinX;
    float fHeight = fMaxY - fMinY;
    FLRenderer::TransitionT& transition = sViewTransitions[iView];

    if (transition.overlayColor.a > 0)
    {
        pcRedPrimitive* pPrimitive =
            pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x203, 24, 4, 0, false);
        float* pVerts = (float*)pPrimitive->BeginVertexData();
        if (!pVerts)
        {
            return;
        }
        pVerts[0] = transition.overlayX0 * fWidth;
        pVerts[1] = transition.overlayY0 * fHeight;
        pVerts[2] = 0.0f;
        pVerts[3] = 1.0f;
        pVerts[4] = viewport.fOffsetX;
        pVerts[5] = viewport.fOffsetY;
        pVerts[6] = transition.overlayX0 * fWidth;
        pVerts[7] = transition.overlayY1 * fHeight;
        pVerts[8] = 0.0f;
        pVerts[9] = 1.0f;
        pVerts[10] = viewport.fOffsetX;
        pVerts[11] = viewport.fHeight + viewport.fOffsetY;
        pVerts[12] = transition.overlayX1 * fWidth;
        pVerts[13] = transition.overlayY0 * fHeight;
        pVerts[14] = 0.0f;
        pVerts[15] = 1.0f;
        pVerts[16] = viewport.fWidth + viewport.fOffsetX;
        pVerts[17] = viewport.fOffsetY;
        pVerts[18] = transition.overlayX1 * fWidth;
        pVerts[19] = transition.overlayY1 * fHeight;
        pVerts[20] = 0.0f;
        pVerts[21] = 1.0f;
        pVerts[22] = viewport.fWidth + viewport.fOffsetX;
        pVerts[23] = viewport.fHeight + viewport.fOffsetY;
        pPrimitive->EndVertexData();

        pcClearShader* pShader = (pcClearShader*)pcRedShader::Create(
            0x5C6E1222, 0x203, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        pShader->m_pTex = RedTexture::Find(0xEA24E62E);
        pShader->m_pAlphaMaskTex = RedTexture::Find(0xEA24E62E);
        pShader->m_blendMode = pcClearShader::BLENDMODE_BLEND;
        RedRenderer::pcRenderPrimitive(pPrimitive, pShader, gMatrixIdentity,
                                       transition.overlayColor, 0x200, 0, 0, 0);
        pPrimitive->Release();
        pShader->Release();
    }

    if (transition.blurColor.a > 0 &&
        (RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) &&
        RedRenderer::pcGetRenderOptions().motionBlur)
    {
        pcRedPrimitive* pPrimitive =
            pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 0x403, 32, 4, 0, false);
        float* pVerts = (float*)pPrimitive->BeginVertexData();
        if (!pVerts)
        {
            return;
        }
        float fZoom = transition.blurZoom;
        pVerts[0] = fMinX;
        pVerts[1] = fMinY;
        pVerts[2] = 0.0f;
        pVerts[3] = 1.0f;
        pVerts[4] = fZoom + viewport.fOffsetX;
        pVerts[5] = fZoom + viewport.fOffsetY;
        pVerts[6] = 0.0f;
        pVerts[7] = 0.0f;
        pVerts[8] = fMinX;
        pVerts[9] = fMaxY;
        pVerts[10] = 0.0f;
        pVerts[11] = 1.0f;
        pVerts[12] = fZoom + viewport.fOffsetX;
        pVerts[13] = (viewport.fHeight + viewport.fOffsetY) - fZoom;
        pVerts[14] = 0.0f;
        pVerts[15] = 1.0f;
        pVerts[16] = fMaxX;
        pVerts[17] = fMinY;
        pVerts[18] = 0.0f;
        pVerts[19] = 1.0f;
        pVerts[20] = (viewport.fWidth + viewport.fOffsetX) - fZoom;
        pVerts[21] = fZoom + viewport.fOffsetY;
        pVerts[22] = 1.0f;
        pVerts[23] = 0.0f;
        pVerts[24] = fMaxX;
        pVerts[25] = fMaxY;
        pVerts[26] = 0.0f;
        pVerts[27] = 1.0f;
        pVerts[28] = (viewport.fWidth + viewport.fOffsetX) - fZoom;
        pVerts[29] = (viewport.fHeight + viewport.fOffsetY) - fZoom;
        pVerts[30] = 1.0f;
        pVerts[31] = 1.0f;
        pPrimitive->EndVertexData();

        pcClearShader* pShader = (pcClearShader*)pcRedShader::Create(
            0x5C6E1222, 0x403, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        pShader->m_pTex = FLRenderer::pcGetPreviousFrameTexture();
        pShader->m_pAlphaMaskTex = RedTexture::Find(0xEA24E62E);
        pShader->m_blendMode = pcClearShader::BLENDMODE_BLEND;
        RedRenderer::pcRenderPrimitive(pPrimitive, pShader, gMatrixIdentity,
                                       transition.blurColor, 0x200, 0, 0, 0);
        pPrimitive->Release();
        pShader->Release();
    }
}

// FUNCTION: SWBF2 0x006bf830
RedTexture* FLRenderer::pcGetPreviousFrameTexture()
{
    return m_previousFrameTex;
}

// FUNCTION: SWBF2 0x006bf890
void FLRenderer::pcUpdateRefractionBuffer()
{
    if (!(RedRenderer::pcGetDeviceCaps().DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES))
    {
        return;
    }
    if (!RedRenderer::pcGetRenderOptions().distortion)
    {
        return;
    }

    float fMinX, fMinY, fMaxX, fMaxY;
    RedRenderer::pcGetViewportExtents(fMinX, fMinY, fMaxX, fMaxY);
    RECT srcRect;
    RECT destRect;
    destRect.left = 0;
    destRect.top = 0;
    srcRect.left = (int)fMinX;
    srcRect.top = (int)fMinY;
    srcRect.right = (int)fMaxX;
    srcRect.bottom = (int)fMaxY;
    destRect.right = RedRenderer::pcGetBuffer(PC_BUFFER_REFRACTION)->m_uiWidth;
    destRect.bottom = RedRenderer::pcGetBuffer(PC_BUFFER_REFRACTION)->m_uiHeight;

    IDirect3DSurface9* pBackBuffer;
    RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
    IDirect3DSurface9* pSurface;
    ((IDirect3DTexture9*)RedRenderer::pcGetBuffer(PC_BUFFER_REFRACTION)->m_pTexture)
        ->GetSurfaceLevel(0, &pSurface);
    RedRenderer::pcGetDevice()->StretchRect(pBackBuffer, &srcRect, pSurface, &destRect,
                                            D3DTEXF_NONE);
    pBackBuffer->Release();
    pSurface->Release();
}

namespace FLRenderer
{
unsigned int m_numPotentialReflections;
Renderable* m_potentialReflections[50];
}

// FUNCTION: SWBF2 0x006bb840
void FLRenderer::AddRenderable(Renderable* renderable, int index, int value)
{
    FLRenderer::RenderableList* bucket = &FLRenderer::_RenderableLists[index];
    PblOrderedList<Renderable, int>* list = renderable->_RenderableNode._pList;
    if (list == bucket)
    {
        if (value != renderable->_RenderableNode._iOrder)
        {
            bucket->Remove(renderable);
            bucket->Add(renderable, value);
        }
    }
    else if (!list)
    {
        bucket->Add(renderable, value);
        if (renderable->HasReflection() &&
            RedRenderer::pcGetRenderCaps().pipeline)
        {
            unsigned int count = FLRenderer::m_numPotentialReflections;
            FLRenderer::m_potentialReflections[count] = renderable;
            FLRenderer::m_numPotentialReflections = count + 1;
        }
    }
}

// FUNCTION: SWBF2 0x006bce60
bool FLRenderer::IsRenderableActive(const Renderable* renderable, int index)
{
    if (index < 10)
    {
        return renderable->_RenderableNode._pList == &FLRenderer::_RenderableLists[index];
    }
    return renderable->_RenderableNode._pList != 0;
}

// FUNCTION: SWBF2 0x006bcfc0
void FLRenderer::RemoveRenderable(Renderable* renderable)
{

    if (!renderable->_RenderableNode._pList)
    {
        return;
    }

    renderable->_RenderableNode._pList->Remove(&renderable->_RenderableNode);

    if (!renderable->HasReflection())
    {
        return;
    }
    if (!RedRenderer::pcGetRenderCaps().pipeline)
    {
        return;
    }

    unsigned int count = FLRenderer::m_numPotentialReflections;
    for (unsigned int i = 0; i < count; ++i)
    {
        if (FLRenderer::m_potentialReflections[i] == renderable)
        {
            FLRenderer::m_potentialReflections[i] =
                FLRenderer::m_potentialReflections[--count];
            FLRenderer::m_numPotentialReflections = count;
            return;
        }
    }
}
