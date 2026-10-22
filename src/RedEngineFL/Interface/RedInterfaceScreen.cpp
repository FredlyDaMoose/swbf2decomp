#include <d3dx9math.h>
#include <math.h>

#include <string.h>

#include "RedInterfaceScreen.h"
#include "Red3DModelElementLite.h"
#include "RedHotSpot.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x0093e4d4
// SYNTHETIC: SWBF2 0x00407a70
// `dynamic initializer for 'RedInterfaceScreen::sSQRT3''
float RedInterfaceScreen::sSQRT3 = (float)sqrt(3.0);

// GLOBAL: SWBF2 0x0093e4f0
// SYNTHETIC: SWBF2 0x00407aa0
// `dynamic initializer for 'RedInterfaceScreen::s_directionalLight''
// SYNTHETIC: SWBF2 0x00769890
// `dynamic atexit destructor for 'RedInterfaceScreen::s_directionalLight''
RedDirectionalLight RedInterfaceScreen::s_directionalLight[2];

// SYNTHETIC: SWBF2 0x006c0a90
// RedDirectionalLight::`default constructor closure'

// GLOBAL: SWBF2 0x007decb8
// SYNTHETIC: SWBF2 0x00407ad0
// `dynamic initializer for 'RedInterfaceScreen::s_lightingState''
// SYNTHETIC: SWBF2 0x007698b0
// `dynamic atexit destructor for 'RedInterfaceScreen::s_lightingState''
RedLightingState RedInterfaceScreen::s_lightingState;

// GLOBAL: SWBF2 0x0093e4a4
// SYNTHETIC: SWBF2 0x00407b60
// `dynamic initializer for 'RedInterfaceScreen::s_screenFull''
// SYNTHETIC: SWBF2 0x007698d0
// `dynamic atexit destructor for 'RedInterfaceScreen::s_screenFull''
RedInterfaceScreen RedInterfaceScreen::s_screenFull;

const float RedInterfaceScreen::sfNear = 1.0f;
const float RedInterfaceScreen::sfFar = 10000.0f;
// GLOBAL: SWBF2 0x0093e4d0
// SYNTHETIC: SWBF2 0x00407b80
// `dynamic initializer for 'RedInterfaceScreen::sfHeight''
float RedInterfaceScreen::sfHeight = RedInterfaceScreen::sSQRT3 * 0.5f;
// GLOBAL: SWBF2 0x0093e4e0
// SYNTHETIC: SWBF2 0x00407ba0
// `dynamic initializer for 'RedInterfaceScreen::sfWidth''
float RedInterfaceScreen::sfWidth = RedInterfaceScreen::sSQRT3 * (2.0f / 3.0f);

// GLOBAL: SWBF2 0x007dec90
float gRedInterfaceTranslateX = 1.0f;
// GLOBAL: SWBF2 0x007dec94
float gRedInterfaceTranslateY = 1.0f;
// GLOBAL: SWBF2 0x007dec98
float gRedInterfaceTranslateZ = 1.0f;
// GLOBAL: SWBF2 0x007dec9c
float gRedInterfaceScaleX = 1.0f;
// GLOBAL: SWBF2 0x007deca0
float gRedInterfaceScaleY = 1.0f;

// FUNCTION: SWBF2 0x006c0910
RedInterfaceScreen::RedInterfaceScreen()
    : m_uiWidthInPixels(0),
      m_uiHeightInPixels(0),
      m_fSafeZoneOffsetX(0.0f),
      m_fSafeZoneOffsetY(0.0f),
      m_fSafeZoneWidth(1.0f),
      m_fSafeZoneHeight(1.0f)
{
    s_directionalLight[0].SetShadowCaster(false);
    s_directionalLight[0].SetSpecularCaster(true);
    s_directionalLight[1].SetShadowCaster(false);
    s_directionalLight[1].SetSpecularCaster(false);

    memset(&s_lightingState, 0, sizeof(s_lightingState));
    s_lightingState.pDirectionalFactor[0] = 1.0f;
    s_lightingState.pDirectionalFactor[1] = 1.0f;
    s_lightingState.pDirectionalLights[0] = &s_directionalLight[0];
    s_lightingState.pDirectionalLights[1] = &s_directionalLight[1];
    s_lightingState.numDirectionalLights = 2;

    if (s_directionalLight[0].IsSpecularCaster())
    {
        ++s_lightingState.numSpecularLights;
    }
    if (s_directionalLight[1].IsSpecularCaster())
    {
        ++s_lightingState.numSpecularLights;
    }
}

// GLOBAL: SWBF2 0x007729b4
const float RedInterfaceScreen::LEFT = 0.0f;
// GLOBAL: SWBF2 0x007729bc
const float RedInterfaceScreen::TOP = 0.0f;
// GLOBAL: SWBF2 0x007729b8
const float RedInterfaceScreen::RIGHT = 1.0f;
// GLOBAL: SWBF2 0x007729c0
const float RedInterfaceScreen::BOTTOM = 1.0f;
// GLOBAL: SWBF2 0x007729c4
const float RedInterfaceScreen::CENTER = 0.5f;

// GLOBAL: SWBF2 0x007deca8 -- see the header for the two routes and why the

float RedInterfaceScreen::s_fScreenBottomV = 1.0f;

// FUNCTION: SWBF2 0x006c14c0
void RedInterfaceScreen::SetDimensions(unsigned int uiWidth, unsigned int uiHeight)
{
    m_uiWidthInPixels = uiWidth;
    m_uiHeightInPixels = uiHeight;
}

// FUNCTION: SWBF2 0x006c14e0
void RedInterfaceScreen::SetSafeZone(float fPercent)
{
    if (0.0f > fPercent)
    {
        fPercent = 0.0f;
    }
    else if (fPercent > 1.0f)
    {
        fPercent = 1.0f;
    }

    m_fSafeZoneHeight = fPercent;
    m_fSafeZoneWidth = fPercent;

    float fMargin = (1.0f - fPercent) * 0.5f;
    m_fSafeZoneOffsetY = fMargin;
    m_fSafeZoneOffsetX = fMargin;
}

// FUNCTION: SWBF2 0x006c1530
void RedInterfaceScreen::SetSafeZone(float fWidth, float fHeight, float fOffsetX, float fOffsetY)
{
    float fRight = fWidth + fOffsetX;
    if (0.0f > fRight)
    {
        fRight = 0.0f;
    }
    else if (fRight > 1.0f)
    {
        fRight = 1.0f;
    }

    float fBottom = fHeight + fOffsetY;
    if (0.0f > fBottom)
    {
        fBottom = 0.0f;
    }
    else if (fBottom > 1.0f)
    {
        fBottom = 1.0f;
    }

    if (0.0f > fOffsetX)
    {
        fOffsetX = 0.0f;
    }
    else if (fOffsetX > 1.0f)
    {
        fOffsetX = 1.0f;
    }
    m_fSafeZoneOffsetX = fOffsetX;

    if (0.0f > fOffsetY)
    {
        fOffsetY = 0.0f;
    }
    else if (fOffsetY > 1.0f)
    {
        fOffsetY = 1.0f;
    }
    m_fSafeZoneOffsetY = fOffsetY;

    float fW = fRight - fOffsetX;
    m_fSafeZoneWidth = (fW > 0.0f) ? fW : 0.0f;

    float fH = fBottom - fOffsetY;
    m_fSafeZoneHeight = (fH > 0.0f) ? fH : 0.0f;
}

static const float kInv255 = 1.0f / 255.0f;

// FUNCTION: SWBF2 0x006c0a00
RedScreenGroupElement::RedScreenGroupElement(const char* pcName)
    : RedGroupElement(pcName)
{
    m_bUseSafeZone = false;
    m_bUsePerspectiveHack = false;

    m_fScreenRelativeY = 0.0f;
    m_fScreenRelativeX = 0.0f;
}

// FUNCTION: SWBF2 0x006c0a40
RedScreenGroupElement::RedScreenGroupElement()
{
    m_bUseSafeZone = false;
    m_bUsePerspectiveHack = false;

    m_fScreenRelativeY = 0.0f;
    m_fScreenRelativeX = 0.0f;
}

// FUNCTION: SWBF2 0x006c0b90
void RedScreenGroupElement::AddToRedScreen()
{
    RedInterfaceScreen::s_screenFull.AddElement(this);
}

// FUNCTION: SWBF2 0x006c0b20
bool RedInterfaceScreen::AddElement(RedScreenGroupElement* pElement)
{
    if (pElement->m_groupNode._pList)
    {
        return false;
    }

    PblList<RedScreenGroupElement>::Node* pAt = m_children.GetFirst();
    while (pAt->_pObject != m_children._head._pObject &&
           pElement->m_zorder <= pAt->_pObject->m_zorder)
    {
        pAt = pAt->_pNext;
    }

    m_children.AddBefore(pAt,
                         (PblList<RedScreenGroupElement>::Node*)&pElement->m_groupNode,
                         pElement);
    return true;
}

// FUNCTION: SWBF2 0x006c0de0
void RedInterfaceElement::Render(const PblMatrix& mParent, const RedColor& color)
{
    if (!m_bEnabled)
    {
        return;
    }

    PblMatrix mContext;
    D3DXMatrixMultiply((D3DXMATRIX*)&mContext,
                       (const D3DXMATRIX*)&m_mLocal,
                       (const D3DXMATRIX*)&mParent);

    mContext.m[3][0] = (float)floor(mContext.m[3][0] + 0.5f);
    mContext.m[3][1] = (float)floor(mContext.m[3][1] + 0.5f);

    RedColor modulated;
    modulated.r = (unsigned char)((float)m_color.r * kInv255 * (float)color.r + 0.5f);
    modulated.g = (unsigned char)((float)m_color.g * kInv255 * (float)color.g + 0.5f);
    modulated.b = (unsigned char)((float)m_color.b * kInv255 * (float)color.b + 0.5f);
    modulated.a = (unsigned char)((float)m_color.a * kInv255 * (float)color.a + 0.5f);

    mContext.m[3][3] = 1.0f;

    RenderUsingContext(mContext, modulated);

    if (m_pHotSpot)
    {
        m_pHotSpot->SetContext(mContext);
    }
}

// FUNCTION: SWBF2 0x006c0f70
void RedInterfaceScreen::Render() const
{
    const bool bShadowMap = RedShadowMap::IsShadowMapEnabled();
    if (bShadowMap)
    {
        RedShadowMap::_bShadowMapEnable = false;
    }
    const bool bFog = RedRenderer::GetFogEnable();
    RedRenderer::SetFogEnable(false);
    const RedCamera* pOldCamera = RedRenderer::GetCamera();

    static RedCamera camera;
    camera.SetMatrix(gMatrixIdentity);
    camera.SetFrustum(sfNear, sfFar, sfWidth,
                      (float)m_uiHeightInPixels * sfWidth * sfHeight /
                          ((float)m_uiWidthInPixels * sfHeight));
    RedRenderer::SetCamera(&camera);

    const float fNearSceneStart = RedRenderer::pcGetNearSceneStart();
    const float fNearSceneEnd = RedRenderer::pcGetNearSceneEnd();
    RedRenderer::pcSetNearSceneStart(0.0f);
    RedRenderer::pcSetNearSceneEnd(0.0f);
    RedRenderer::BeginScene();
    RedHotSpot::UpdateCamera();

    gRedInterfaceTranslateZ = gRedInterfaceScaleX = 640.0f / (float)GetScreenWidth();
    gRedInterfaceScaleY = 480.0f / (float)GetScreenHeight();
    const float fDepth = (float)m_uiWidthInPixels / sfWidth;

    float fScaleX, fScaleY, fUnitX, fUnitY;
    GetScale(fScaleX, fScaleY, fUnitX, fUnitY);

    PblMatrix mScreen;
    mScreen.m[0][0] = fUnitX;
    mScreen.m[0][1] = 0.0f;
    mScreen.m[0][2] = 0.0f;
    mScreen.m[0][3] = 0.0f;
    mScreen.m[1][0] = 0.0f;
    mScreen.m[1][1] = fUnitY;
    mScreen.m[1][2] = 0.0f;
    mScreen.m[1][3] = 0.0f;
    mScreen.m[2][0] = 0.0f;
    mScreen.m[2][1] = 0.0f;
    mScreen.m[2][2] = 1.0f;
    mScreen.m[2][3] = 0.0f;
    mScreen.m[3][0] = 0.0f;
    mScreen.m[3][1] = 0.0f;
    mScreen.m[3][2] = 0.0f;
    mScreen.m[3][3] = 1.0f;

    PblMatrix mPerspective = mScreen;
    mPerspective.m[0][0] = gRedInterfaceScaleX * fUnitX;
    mPerspective.m[1][1] = gRedInterfaceScaleY * fUnitY;

    for (PblList<RedScreenGroupElement>::Itor it = m_children.Begin(); it != m_children.End(); ++it)
    {
        RedScreenGroupElement* pGroup = it;
        float fX, fY;
        if (pGroup->m_bUseSafeZone)
        {
            fX = m_fSafeZoneWidth * pGroup->m_fScreenRelativeX + m_fSafeZoneOffsetX;
            fY = m_fSafeZoneHeight * pGroup->m_fScreenRelativeY + m_fSafeZoneOffsetY;
        }
        else
        {
            fX = pGroup->m_fScreenRelativeX;
            fY = pGroup->m_fScreenRelativeY;
        }

        const bool bPerspective = pGroup->m_bUsePerspectiveHack;
        Red3DModelElementLite::SetPerspectiveHack(bPerspective);
        if (bPerspective)
        {
            mPerspective.m[3][0] = gRedInterfaceScaleX * fScaleX * (fX - 0.5f) * gRedInterfaceTranslateX;
            mPerspective.m[3][1] = gRedInterfaceScaleY * fScaleY * (fY - 0.5f) * gRedInterfaceTranslateY;
            mPerspective.m[3][2] = -(gRedInterfaceTranslateZ * fDepth);
            mPerspective.m[3][3] = 1.0f;
        }
        else
        {
            mScreen.m[3][0] = (fX - 0.5f) * fScaleX;
            mScreen.m[3][1] = (fY - 0.5f) * fScaleY;
            mScreen.m[3][2] = -fDepth;
            mScreen.m[3][3] = 1.0f;
        }
        it->Render(it->m_bUsePerspectiveHack ? mPerspective : mScreen, RedColor::WHITE);
    }

    RedRenderer::EndScene();
    RedRenderer::SetCamera(pOldCamera);
    RedRenderer::pcSetNearSceneStart(fNearSceneStart);
    RedRenderer::pcSetNearSceneEnd(fNearSceneEnd);
    RedRenderer::SetFogEnable(bFog);
    if (bShadowMap)
    {
        RedShadowMap::_bShadowMapEnable = true;
    }
}

// SYNTHETIC: SWBF2 0x007698e0
// RedInterfaceScreen::Render's `dynamic atexit destructor for 'camera'' (the

// SYNTHETIC: SWBF2 0x006c0ae0
// RedDirectionalLight::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006c0b10
bool RedDirectionalLight::AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const
{
    return true;
}

// FUNCTION: SWBF2 0x006c0ba0
const PblMatrix& RedDirectionalLight::GetProjectionMatrix() const
{
    if (m_bProjectionDirty)
    {
        _RecalculateProjectionMatrix();
    }
    return (const PblMatrix&)m_mProjection;
}

// FUNCTION: SWBF2 0x006c0bc0
void RedInterfaceScreen::GetSafeArea(float& fLeft, float& fTop, float& fRight, float& fBottom)
{
    float fWidth, fHeight, fOffsetX, fOffsetY;
    GetSafeZone(fWidth, fHeight, fOffsetX, fOffsetY);

    fLeft = (float)GetScreenWidth() * fOffsetX;
    fRight = (fWidth + fOffsetX) * (float)GetScreenWidth();
    fTop = (float)GetScreenHeight() * fOffsetY;
    fBottom = (fHeight + fOffsetY) * (float)GetScreenHeight();
}

// FUNCTION: SWBF2 0x006c0ca0
void RedInterfaceScreen::GetSafeZone(float& fWidth, float& fHeight, float& fOffsetX, float& fOffsetY) const
{
    fOffsetX = m_fSafeZoneOffsetX;
    fOffsetY = m_fSafeZoneOffsetY;
    fWidth = m_fSafeZoneWidth;
    fHeight = m_fSafeZoneHeight;
}

// FUNCTION: SWBF2 0x006c0cd0
void RedInterfaceScreen::GetScale(float& fScaleX, float& fScaleY, float& fUnitX, float& fUnitY) const
{
    fScaleX = (float)m_uiWidthInPixels;
    fScaleY = -(float)m_uiHeightInPixels;
    fUnitX = 1.0f;
    fUnitY = -1.0f;
}

// FUNCTION: SWBF2 0x006c0a80
RedInterfaceScreen::~RedInterfaceScreen()
{
    RemoveAllElements();
}

// FUNCTION: SWBF2 0x006c0d40
void RedInterfaceScreen::RemoveAllElements()
{
    while (m_children.GetCount())
    {
        m_children.GetFirst()->_pObject->Cleanup();
        m_children.Remove(m_children.GetFirst());
    }
}

// FUNCTION: SWBF2 0x006c0d90
bool RedInterfaceScreen::RemoveElement(RedScreenGroupElement* pElement)
{
    PblList<RedScreenGroupElement>::Node* pNode =
        (PblList<RedScreenGroupElement>::Node*)&pElement->m_groupNode;
    if (pNode->_pList != &m_children)
    {
        return false;
    }

    m_children.Remove(pNode);
    return true;
}

// FUNCTION: SWBF2 0x006c0dd0
void RedScreenGroupElement::RemoveFromRedScreen()
{
    RedInterfaceScreen::s_screenFull.RemoveElement(this);
}

// FUNCTION: SWBF2 0x006c15e0
bool RedDirectionalLight::SphereIntersect(const PblVector3& vCenter, float fRadius) const
{
    return true;
}
