#pragma once

#include <stddef.h>
#include "RedEngineFL/Util/RedRegion.h"
#include "ListPool.h"

class AIVisRegion : public RedRegion
{
public:

    AIVisRegion(RedRegionDesc& desc, float fVisStand, float fVisCrouch)
        : RedRegion(desc), mVisStand(fVisStand), mVisCrouch(fVisCrouch), mEnabled(true)
    {
    }

    float GetVisStand();
    float GetVisCrouch();

    float mVisStand;
    float mVisCrouch;
    bool mEnabled;
};
 
 
 
 

// VTABLE: SWBF2 0x00796be0
class AIVisRegionManager : public RedRegionFactory
{
public:

    AIVisRegionManager();

    // SYNTHETIC: SWBF2 0x00443450
    // AIVisRegionManager::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
    virtual void ReadParameter(const char* pcRegionName, const char* pcParameter,
                               unsigned int uiIndex, unsigned int uiCount,
                               const char* pcValue, unsigned int uiValueIndex,
                               void* pDesc) const;

    static void Init() {}
    static void Reset();
    static void UpdateNextRegion();

    static ListPool<AIVisRegion*, int> sRegions;
    static int mNextUpdateRegion;
};
  
