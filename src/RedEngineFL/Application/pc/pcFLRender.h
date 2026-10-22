#pragma once

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblPlane.h"
#include "PebbleFL/Common/PblVector.h"

class Renderable;
class RedCamera;
class RedInterfaceScreen;
class RedScreenGroupElement;
class RedBitmapElement;
class PblMatrix;
class ReflectionRegionFactory;

namespace FLRenderer
{

struct ReflectionT
{
    ReflectionT();

    Renderable* renderable;
    PblPlane reflectionPlane;
    PblBox bbox;
    char reflectionID;
    unsigned int renderFlags;
};

void Init(bool bUberMode);
void Cleanup();
void pcReset(ResetFlagsT flags);

unsigned int AddView(const RedViewport& rViewport, RedCamera* pCamera);
void RemoveView(unsigned int uiView);
float GetNearPlane();
float GetNearSceneFadeStart();
float GetNearSceneFadeEnd();
float GetFarSceneRange();
bool GetFadeAdjustWithZoom();
unsigned int GetCurrentViewport();

unsigned int GetViewportCount();
unsigned int GetViewWidth(unsigned int uiView);
unsigned int GetViewHeight(unsigned int uiView);
RedInterfaceScreen* GetInterfaceView(int iView);
void SetSceneEnable(unsigned int uiView, bool bEnable);
void SetInterfaceEnable(unsigned int uiView, bool bEnable);

void SetFarSceneRange(float fStart, float fEnd);
void SetFogColor(const RedColor& color);
void SetFogRange(float fStart, float fEnd);
void SetNearSceneRange(float fNearPlane, float fFadeStartLow, float fFadeStartHigh,
                       float fFadeEndLow, float fFadeEndHigh);

void SetFadeAdjustWithZoom(bool bEnable);
void SetReflectionFogColor(const RedColor& color);
void SetWorldFogRange(float fStart, float fEnd);
RedTexture* pcGetReflectionTexture(char reflectionID);

void SetTransitionBlurColor(int iView, RedColor const& rColor);
void SetTransitionBlurZoom(int iView, float fZoom);
void SetTransitionOverlayColor(int iView, RedColor const& rColor);
void SetTransitionOverlayRect(int iView, float fX0, float fY0, float fX1, float fY1);

struct TransitionT
{
    RedColor blurColor;
    char unknown04[20];
    float blurZoom;
    RedColor overlayColor;
    float overlayX0;
    float overlayY0;
    float overlayX1;
    float overlayY1;
};

void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
void AddInterfaceElement(unsigned int uiView, RedScreenGroupElement* pElement,
                         float fX, float fY, bool bUseSafeZone);
void RemoveInterfaceElement(RedScreenGroupElement* pElement);
void RemoveInterfaceElement(unsigned int uiView, RedScreenGroupElement* pElement);

void InitCursor(unsigned int uiTextureHash, bool bInit);
void EnableCursor(bool bEnable);

void SetCursorPos(float fX, float fY);

void CaptureFrame(unsigned int uiView);

void Render();
void RenderReflections();
void RenderFarScene(unsigned int uiView);
void pcUpdateRefractionBuffer();
RedTexture* pcGetPreviousFrameTexture();

char RegisterReflection(Renderable* pRenderable, const PblPlane& plane,
                        unsigned int uiRenderFlags);
void AddReflectionObject(char reflectionID, const PblBox& box, bool bVisible);
bool IsReflected(const PblVector3& vPoint, float fRadius, PblMatrix& mReflection,
                 bool bAllRegions);

extern bool m_bFarSceneEnable[1];
extern bool m_bEnableFadeAdjustWithZoom;
extern bool m_bSceneEnable[1];
extern RedViewport m_Viewport[1];
extern RedCamera* m_pViewCamera[1];
extern unsigned int mView;
extern unsigned int mViewCount;
extern float m_fViewNearPlane;
extern float m_fViewNearSceneFadeStart[2];
extern float m_fViewNearSceneFadeEnd[2];
extern float m_fViewFarPlane[2];
extern RedInterfaceScreen* m_pViewInterfaceScreen[1];
extern ReflectionRegionFactory* m_pReflectionRegionFactory;
extern RedTexture* m_reflectionRenderTarget;
extern unsigned int m_numCurFrameReflections;
extern RedTexture* m_previousFrameTex;
extern _D3DFORMAT m_formatReflectionTarget_Medium;
extern _D3DFORMAT m_formatReflectionDepth_Medium;
extern RedBitmapElement mCursorImage;
extern RedColor m_colorFog;
extern RedScreenGroupElement mCursorGroup;
extern ReflectionT m_curFrameReflections[255];
}
 
 
extern FLRenderer::TransitionT sViewTransitions[2];

void RenderViewTransition(int iView);

enum COMPONENT
{
    COMPONENT_X = 0,
    COMPONENT_Y = 1,
    COMPONENT_Z = 2
};

bool ClipLoopToZPlanes(PblVector3* pOut, int& nOut, const PblVector3* pIn,
                       const int nIn, const float fNear, const float fFar);
bool ClipLoopToFrustumPlanes(PblVector3* pOut, int& nOut, const PblVector3* pIn,
                             const int nIn, const COMPONENT eComponent,
                             const float fMin, const float fMax);

// VTABLE: SWBF2 0x0077286c
class ReflectionRegionFactory : public RedRegionFactory
{
public:
    ReflectionRegionFactory() : RedRegionFactory("reflection") {}
    virtual ~ReflectionRegionFactory() {}

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
};
 

class ReflectionRegion : public RedRegion
{
public:
    ReflectionRegion(RedRegionDesc& desc);

    static void Init();

    static void Cleanup();

    static void UpdateAllVisibility(const RedCamera* pCamera);
    static bool IsReflected(const PblVector3& vPoint, float fRadius,
                            PblMatrix& mReflection, bool bAllRegions);

private:

    bool _IsVisible(const RedCamera* pCamera) const;

    void _UpdateReflectionMatrix();

public:
    PblPlane m_reflectionPlane;
    PblMatrix m_reflectionMat;
    PblList<ReflectionRegion>::Node m_Node;
    PblList<ReflectionRegion>::Node m_VisibleNode;

    static PblList<ReflectionRegion> s_List;
    static PblList<ReflectionRegion> s_VisibleList;
};
 
 
