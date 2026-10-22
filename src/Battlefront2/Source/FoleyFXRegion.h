#pragma once

#include <stddef.h>
#include "RedEngineFL/Util/RedRegion.h"

// VTABLE: SWBF2 0x0079f260
class FoleyFXRegionFactory : public RedRegionFactory
{
public:

    FoleyFXRegionFactory();

    // SYNTHETIC: SWBF2 0x0052cb40
    // FoleyFXRegionFactory::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
};

class FoleyFXGroup;

class FoleyFXRegion : public RedRegion
{
public:
    FoleyFXRegion(RedRegionDesc& desc, FoleyFXGroup* const pGroup);

    static PblListSingle smList;

    FoleyFXGroup* mGroup;
    PblListSingle::Node mNode;
};
  
 
 
 
