#pragma once

#include <stddef.h>

#include "RedGroupElement.h"

// VTABLE: SWBF2 0x00772a0c

class RedInterfaceScreen;

class RedScreenGroupElement : public RedGroupElement
{
public:

    // FUNCTION: SWBF2 0x006c0a40
    RedScreenGroupElement();

    // FUNCTION: SWBF2 0x006c0a00
    RedScreenGroupElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x005656e0
    // RedScreenGroupElement::`vector deleting destructor'

    // FUNCTION: SWBF2 0x004297b0
    virtual ~RedScreenGroupElement() {}

    // FUNCTION: SWBF2 0x00543ec0
    virtual bool IsScreenGroup() const { return true; }

    // FUNCTION: SWBF2 0x006c0b90
    void AddToRedScreen();

    void RemoveFromRedScreen();

    bool GetUseSafezone() const { return m_bUseSafeZone; }
    void SetUseSafezone(bool bUseSafeZone) { m_bUseSafeZone = bUseSafeZone; }

    void SetUsePerspectiveHack(bool bUse) { m_bUsePerspectiveHack = bUse; }

    void SetScreenRelativePos(float fX, float fY)
    {
        m_fScreenRelativeX = fX;
        m_fScreenRelativeY = fY;
    }

    float m_fScreenRelativeX;
    float m_fScreenRelativeY;
    bool m_bUseSafeZone : 1;
    bool m_bUsePerspectiveHack : 1;
};

 
 
 
 

class RedDirectionalLight;
struct RedLightingState;

class RedInterfaceScreen
{
public:

    // FUNCTION: SWBF2 0x006c0910
    RedInterfaceScreen();

    ~RedInterfaceScreen();

    // FUNCTION: SWBF2 0x006c0b20
    bool AddElement(RedScreenGroupElement* pElement);

    bool RemoveElement(RedScreenGroupElement* pElement);
    void RemoveAllElements();

    void GetSafeZone(float& fWidth, float& fHeight, float& fOffsetX, float& fOffsetY) const;
    void GetSafeArea(float& fLeft, float& fTop, float& fRight, float& fBottom);
    void GetScale(float& fScaleX, float& fScaleY, float& fUnitX, float& fUnitY) const;

    void Render() const;

    // FUNCTION: SWBF2 0x006c14c0
    void SetDimensions(unsigned int uiWidth, unsigned int uiHeight);

    // FUNCTION: SWBF2 0x006c14e0
    void SetSafeZone(float fPercent);

    // FUNCTION: SWBF2 0x006c1530
    void SetSafeZone(float fWidth, float fHeight, float fOffsetX, float fOffsetY);

    // FUNCTION: SWBF2 0x00429240
    static unsigned int GetScreenWidth() { return s_screenFull.m_uiWidthInPixels; }
    // FUNCTION: SWBF2 0x0044d270
    static unsigned int GetScreenHeight() { return s_screenFull.m_uiHeightInPixels; }

    static float GetScreenSafeZone() { return s_screenFull.m_fSafeZoneWidth; }

    static float GetScreenBottomV() { return s_fScreenBottomV; }

    unsigned int m_uiWidthInPixels;
    unsigned int m_uiHeightInPixels;
    float m_fSafeZoneOffsetX;
    float m_fSafeZoneOffsetY;
    float m_fSafeZoneWidth;
    float m_fSafeZoneHeight;
    PblList<RedScreenGroupElement> m_children;

    static RedInterfaceScreen s_screenFull;

    static RedLightingState s_lightingState;
    static RedDirectionalLight s_directionalLight[2];

    static void GetDirectionalLight(int iIndex, PblVector3& vDirection, class RedColorValue& color);

    static const float LEFT;
    static const float TOP;

    static const float CENTER;

    static const float RIGHT;
    static const float BOTTOM;

    static float s_fScreenBottomV;

    static float sSQRT3;
    static const float sfNear;
    static const float sfFar;
    static float sfWidth;
    static float sfHeight;
};

extern float gRedInterfaceTranslateX;
extern float gRedInterfaceTranslateY;
extern float gRedInterfaceTranslateZ;
extern float gRedInterfaceScaleX;
extern float gRedInterfaceScaleY;

 
 
