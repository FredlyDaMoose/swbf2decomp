#pragma once

#include <stddef.h>
#include "RedEngineFL/Util/RedRegion.h"
#include "ListPool.h"

// VTABLE: SWBF2 0x0079f200
class FlyRegionManager : public RedRegionFactory
{
public:

    FlyRegionManager();

    // SYNTHETIC: SWBF2 0x0052bd30
    // FlyRegionManager::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;

    static void Init() {}
    static void Reset();
    static bool IsInside(const PblVector3& rPoint, float fRadius);
    static bool RayTest(const PblVector3& rStart, const PblVector3& rEnd,
                        float fRadius);

    static ListPool<RedRegion*, int> sRegions;
};
  
