#pragma once

#include <stddef.h>
#include "RedLight.h"

// VTABLE: SWBF2 0x00772170
class RedAmbientLight : public RedLight
{
public:

    // FUNCTION: SWBF2 0x006afca0
    RedAmbientLight(const RedColorValue& color = RedColorValue::WHITE,
                    const RedColorValue& topColor = RedColorValue::WHITE,
                    const RedColorValue& bottomColor = RedColorValue::WHITE)
        : RedLight(TYPE_AMBIENT)
    {
        m_topColor = topColor;
        m_bottomColor = bottomColor;
        m_color = color;
    }

    // FUNCTION: SWBF2 0x006afed0
    virtual ~RedAmbientLight() {}

    // FUNCTION: SWBF2 0x006aff00
    RedAmbientLight& operator=(const RedAmbientLight& rOther)
    {
        RedLight::operator=(rOther);
        m_topColor = rOther.m_topColor;
        m_bottomColor = rOther.m_bottomColor;
        return *this;
    }

    virtual void DebugDraw() const;
    virtual bool IsVisible(const RedCamera* pCamera) const;
    virtual bool SphereIntersect(const PblVector3& vCenter, float fRadius) const;
    virtual bool AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const;

    void SetColor(const RedColorValue& color) { m_color = color; }
    void SetTopColor(const RedColorValue& color) { m_topColor = color; }
    void SetBottomColor(const RedColorValue& color) { m_bottomColor = color; }

    const RedColorValue& GetTopColor() const { return m_topColor; }
    const RedColorValue& GetBottomColor() const { return m_bottomColor; }

    RedColorValue m_topColor;
    RedColorValue m_bottomColor;

    // GLOBAL: SWBF2 0x007e23a0
    static RedAmbientLight WHITE;
    // GLOBAL: SWBF2 0x007e2410
    static RedAmbientLight BLACK;
};
 
 
 

struct RedLightingState
{

    RedLightingState& operator=(const RedLightingState& rOther);

    RedAmbientLight ambientLight;
    RedDirectionalLight* pDirectionalLights[2];
    RedOmniLight* pOmniLights[4];
    RedSpotLight* pSpotLights[1];
    float pDirectionalFactor[2];
    RedLight* pProjectedLight;
    RedTexture* pEnvMap;
    float envMapFactor;
    unsigned char numDirectionalLights;
    unsigned char numOmniLights;
    unsigned char numSpotLights;
    unsigned char bStaticLighting;
    unsigned char numSpecularLights;
};
 
 
 
 
 
 
 
