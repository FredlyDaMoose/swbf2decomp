#pragma once

#include <stddef.h>
#include <d3d9.h>
#include "RedLight.h"
#include "PebbleFL/Common/PblMatrix.h"

class RedRegion;

 

// VTABLE: SWBF2 0x007729e8
class RedDirectionalLight : public RedLight
{
public:
    RedDirectionalLight(const PblVector3& vDirection = PblVector3(0.0f, -1.0f, 0.0f),
                        const RedColorValue& color = RedColorValue::WHITE);

    // FUNCTION: SWBF2 0x006c0a70
    virtual ~RedDirectionalLight() {}

    virtual void DebugDraw() const;
    virtual bool IsVisible(const RedCamera* pCamera) const;
    virtual bool SphereIntersect(const PblVector3& vCenter, float fRadius) const;
    virtual bool AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const;
    virtual const PblMatrix& GetProjectionMatrix() const;

    static RedDirectionalLight BLACK;

    void SetColor(const RedColorValue& color);
    void SetDirection(const PblVector3& vDirection);

    void SetProjectionTextureOffset(float fU, float fV)
    {
        m_fProjTexOffsetU = fU;
        m_fProjTexOffsetV = fV;
        m_bProjectionDirty = true;
    }
    void SetProjectionTextureTile(float fU, float fV)
    {
        m_fProjTexTileU = fU;
        m_fProjTexTileV = fV;
        m_bProjectionDirty = true;
    }

private:
    void _RecalculateProjectionMatrix() const;

public:
    PblVector3 m_vDirection;
    unsigned int m_uiRegionName;
    RedRegion* m_boundingRegion;
    D3DLIGHT9 m_d3dLight;
    mutable PblMatrix m_mProjection;
    mutable bool m_bProjectionDirty;
    float m_fProjTexOffsetU;
    float m_fProjTexOffsetV;
    float m_fProjTexTileU;
    float m_fProjTexTileV;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
