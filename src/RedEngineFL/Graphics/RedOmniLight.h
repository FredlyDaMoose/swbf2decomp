#pragma once

#include <stddef.h>

#include "RedLight.h"
#include "RedDirectionalLight.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class RedOmniLight : public RedLight
{
public:
    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

    // FUNCTION: SWBF2 0x004bd970
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    RedOmniLight(const PblVector3& pos, float fRadius, const RedColorValue& color);
    void SetColor(const RedColorValue& color);
    void SetPosition(const PblVector3& pos);
    void SetRadius(float fRadius);

    virtual ~RedOmniLight();

    virtual void DebugDraw() const;
    virtual bool IsVisible(const RedCamera* pCamera) const;
    // FUNCTION: SWBF2 0x00570a00
    virtual bool SphereIntersect(const PblVector3& vCenter, float fRadius) const { return true; }
    // FUNCTION: SWBF2 0x00570a10
    virtual bool AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const { return true; }
    // FUNCTION: SWBF2 0x00570a20
    virtual const PblMatrix& GetProjectionMatrix() const
    {
        if (m_bProjectionDirty)
        {
            _RecalculateProjectionMatrix();
        }
        return m_mProjection;
    }

protected:
    void _RecalculateProjectionMatrix() const;

public:
    PblVector3 m_pos;
    float m_radius;
    D3DLIGHT9 m_d3dLight;
    float m_fRadiusInv;
    float m_fRadiusInvSq;
    mutable PblMatrix m_mProjection;
    mutable bool m_bProjectionDirty;

    static MemoryPool sMemoryPool;

    static RedOmniLight BLACK;
};
 
 
 
 
 
 
