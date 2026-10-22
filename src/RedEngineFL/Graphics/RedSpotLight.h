#pragma once

#include <stddef.h>

#include "RedLight.h"
#include "RedDirectionalLight.h"
#include "PebbleFL/Common/PblMatrix.h"

// VTABLE: SWBF2 0x00783dc8
class RedSpotLight : public RedLight
{
public:

    RedSpotLight(const RedColorValue& color, const PblVector3& vDirection,
                 const PblVector3& vPosition, float fRadius, float fFalloff);
    virtual ~RedSpotLight();

    virtual void DebugDraw() const;
    virtual bool IsVisible(const RedCamera* pCamera) const;
    virtual bool SphereIntersect(const PblVector3& vCenter, float fRadius) const;
    virtual bool AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const;
    virtual const PblMatrix& GetProjectionMatrix() const;

    void SetColor(const RedColorValue& color);
    void SetRadius(float fRadius);
    void SetPosition(const PblVector3& vPosition);
    void SetDirection(const PblVector3& vDirection);
    void SetInnerConeAngle(float fAngle);
    void SetOuterConeAngle(float fAngle);
    void SetFalloff(float fFalloff);

    void SetBidirectional(bool bBidirectional) { m_bBidirectional = bBidirectional; }

private:
    void _RecalculateProjectionMatrix() const;

public:
    float m_fTanHalfInnerConeAngle;
    float m_fTanHalfOuterConeAngle;
    float m_fCosHalfInnerConeAngle;
    float m_fCosHalfOuterConeAngle;
    float m_fFalloff;
    float m_fRadius;
    PblVector3 m_vDirection;
    PblVector3 m_vPos;
    mutable PblMatrix m_mProjection;
    mutable bool m_bProjectionDirty;
    bool m_bBidirectional;
    D3DLIGHT9 m_d3dLight;
    float m_fRadiusInv;
    float m_fRadiusInvSq;
    float m_fShadowRadius;

    float GetShadowRadius() const { return m_fShadowRadius; }

    static RedSpotLight BLACK;
};
 
 
 
 
 
 
 
 
 
 
 
