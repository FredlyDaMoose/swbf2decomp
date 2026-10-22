#pragma once

#include <stddef.h>
#include "RedEngineFL/Util/RedRegion.h"
#include "PebbleFL/Common/PblList.h"
#include "Thread.h"

class DamageRegion : public RedRegion
{
public:

    struct Damage
    {
        float damageRate;
        float healthScale[8];
        float shieldScale;
    };

    class UpdateThread : public Thread
    {
    public:
        virtual bool Update(float fDeltaTime);
    };

    DamageRegion(RedRegionDesc& desc, const Damage& damage);

    void Update(float fDeltaTime) const;

    static void Create();
    static void Destroy();
    static void ClearList();
    static void UpdateAll(float fDeltaTime);

    Damage mDamage;
    PblList<DamageRegion>::Node mDamageListNode;

    static PblList<DamageRegion> sList;
    static UpdateThread* mUpdateThread;
};
 
 
 
 
 

// VTABLE: SWBF2 0x0079fb48
class DamageRegionFactory : public RedRegionFactory
{
public:
    DamageRegionFactory() : RedRegionFactory("damage") {}

    // SYNTHETIC: SWBF2 0x00539240
    // DamageRegionFactory::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
    virtual void ReadParameter(const char* pcRegionName, const char* pcParameter,
                               unsigned int uiIndex, unsigned int uiCount,
                               const char* pcValue, unsigned int uiValueIndex,
                               void* pDesc) const;
};
  
