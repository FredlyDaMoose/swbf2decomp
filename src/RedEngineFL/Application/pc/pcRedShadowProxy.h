#pragma once

#include <stddef.h>

#include "RedEngineFL/Util/RedSceneObject.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class PblMatrix;
class RedModel;

// VTABLE: SWBF2 0x00783cb0
class RedShadowProxy : public RedSceneObject
{
public:
    RedShadowProxy();
    virtual ~RedShadowProxy();

    // SYNTHETIC: SWBF2 0x006ed720
    // RedShadowProxy::`scalar deleting destructor'

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    void BuildData(const RedModel* pModel, const PblMatrix* pMatrix, float fLength,
                   const PblVector3& vPos);
    bool HasShadow();

    static void AddToScene2();
    static void RemoveFromScene2();
    static void CleanUp();
    static void Reset();

    static PblList<RedShadowProxy> _sShadowItemsList;

    PblMatrix* mMatrix;
    RedModel* mModel;
    PblList<RedShadowProxy>::Node mObjectNode;
    float m_fShadowLength;
    PblVector3 m_vCasterCenter;
    float m_fCasterRadius;
};

 
 
 
 
