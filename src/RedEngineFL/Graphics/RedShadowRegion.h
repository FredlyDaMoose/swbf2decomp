#pragma once

#include <stddef.h>
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedAmbientLight.h"

// VTABLE: SWBF2 0x00772d4c
class RedShadowRegionFactory : public RedRegionFactory
{
public:
    RedShadowRegionFactory() : RedRegionFactory("shadow") {}
    virtual ~RedShadowRegionFactory() {}

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
    virtual void ReadParameter(const char* pcRegionName, const char* pcParameter,
                               unsigned int uiIndex, unsigned int uiCount,
                               const char* pcValue, unsigned int uiValueIndex,
                               void* pDesc) const;
};
 

class RedShadowRegion : public RedRegion
{
    friend class RedShadowRegionFactory;

public:

    struct Info
    {
        float fDirectionalFactor[2];
        RedAmbientLight ambient;
        unsigned int envMapHash;
    };

    static void Init();

    static void Cleanup();

    static void UpdateAllVisibility(const RedCamera* pCamera);

    static bool GetShadowValues(const PblVector3& vPos, float fRadius,
                                float* const pfDirectional, RedAmbientLight& ambient,
                                unsigned int& uEnvMapHash, float& fEnvMapFactor);

    static PblList<RedShadowRegion> s_List;
    static PblList<RedShadowRegion> s_VisibleList;

protected:

    RedShadowRegion(RedRegionDesc& desc, const Info& info);

public:
    Info m_info;
    PblList<RedShadowRegion>::Node m_Node;
    PblList<RedShadowRegion>::Node m_VisibleNode;
};
 
 
 
 
 
