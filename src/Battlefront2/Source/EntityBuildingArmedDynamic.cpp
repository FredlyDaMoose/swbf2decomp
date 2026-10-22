#include "EntityBuildingArmedDynamic.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// GLOBAL: SWBF2 0x01ebb894
// SYNTHETIC: SWBF2 0x00402930
// `dynamic initializer for 'EntityBuildingArmedDynamicClass::rttiHashEntityBuildingArmedDynamicClass''
PblHash EntityBuildingArmedDynamicClass::rttiHashEntityBuildingArmedDynamicClass("EntityBuildingArmedDynamicClass");
char EntityBuildingArmedDynamicClass::rttiNameEntityBuildingArmedDynamicClass[] = "EntityBuildingArmedDynamicClass";

// FUNCTION: SWBF2 0x00496c50
EntityBuildingArmedDynamicClass::EntityBuildingArmedDynamicClass(unsigned int uHash)
    : EntityBuildingArmedClass(uHash)
{
}

// FUNCTION: SWBF2 0x00496ca0
EntityBuildingArmedDynamicClass::EntityBuildingArmedDynamicClass(EntityBuildingArmedDynamicClass* pBase,
                                                                 unsigned int uHash)
    : EntityBuildingArmedClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x00496cc0
EntityBuildingArmedDynamicClass::~EntityBuildingArmedDynamicClass()
{
}

// SYNTHETIC: SWBF2 0x00496c70
// EntityBuildingArmedDynamicClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00496cd0
EntityClass* EntityBuildingArmedDynamicClass::Derive(unsigned int uHash)
{
    return new EntityBuildingArmedDynamicClass(this, uHash);
}

// FUNCTION: SWBF2 0x00496d40
Entity* EntityBuildingArmedDynamicClass::Build(const EntityDesc& desc)
{
    if (EntityBuildingArmedDynamic::sMemoryPool.mUsed >=
        EntityBuildingArmedDynamic::sMemoryPool.GetCount())
    {
        return 0;
    }

    EntityBuildingArmedDynamic* pBuilding = new EntityBuildingArmedDynamic(this, desc);
    if (pBuilding)
    {
        pBuilding->AddStaticToCollisionManager();
        pBuilding->ActivatePhysics();
    }
    return pBuilding;
}

// FUNCTION: SWBF2 0x00496b90
bool EntityBuildingArmedDynamicClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityBuildingArmedDynamicClass || EntityBuildingArmedClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00496be0
unsigned int EntityBuildingArmedDynamicClass::GetDerivedRtti()
{
    return rttiHashEntityBuildingArmedDynamicClass;
}

// FUNCTION: SWBF2 0x00496bf0
const char* EntityBuildingArmedDynamicClass::GetDerivedRttiName()
{
    return rttiNameEntityBuildingArmedDynamicClass;
}

// GLOBAL: SWBF2 0x01ebb890
// SYNTHETIC: SWBF2 0x00402920
// `dynamic initializer for 'EntityBuildingArmedDynamic::rttiHashEntityBuildingArmedDynamic''
PblHash EntityBuildingArmedDynamic::rttiHashEntityBuildingArmedDynamic("EntityBuildingArmedDynamic");
char EntityBuildingArmedDynamic::rttiNameEntityBuildingArmedDynamic[] = "EntityBuildingArmedDynamic";

// GLOBAL: SWBF2 0x01ebb898
// SYNTHETIC: SWBF2 0x00402900
// `dynamic initializer for 'EntityBuildingArmedDynamic::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f00
// `dynamic atexit destructor for 'EntityBuildingArmedDynamic::sMemoryPool''
MemoryPool EntityBuildingArmedDynamic::sMemoryPool("EntityBuildingArmedDynamic", 0x6b0);

// FUNCTION: SWBF2 0x00496b10
unsigned int EntityBuildingArmedDynamic::GetDerivedRtti() const
{
    return rttiHashEntityBuildingArmedDynamic;
}

// FUNCTION: SWBF2 0x00496b20
const char* EntityBuildingArmedDynamic::GetDerivedRttiName() const
{
    return rttiNameEntityBuildingArmedDynamic;
}

// FUNCTION: SWBF2 0x00496b40
EntityType EntityBuildingArmedDynamic::GetEntityType() const
{
    return ENT_ARMED_DYNAMIC;
}

// FUNCTION: SWBF2 0x00496ac0
bool EntityBuildingArmedDynamic::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityBuildingArmedDynamic || EntityBuildingArmed::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00496c00
EntityBuildingArmedDynamic::EntityBuildingArmedDynamic(EntityBuildingArmedDynamicClass* pClass,
                                                       const EntityDesc& desc)
    : EntityBuildingArmed(pClass, desc)
{
}

// SYNTHETIC: SWBF2 0x00496b50
// EntityBuildingArmedDynamic::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00496dda
// [thunk]:EntityBuildingArmedDynamic::`scalar deleting destructor' adjustor{148}
// SYNTHETIC: SWBF2 0x00496de5
// [thunk]:EntityBuildingArmedDynamic::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x00496df0
// [thunk]:EntityBuildingArmedDynamic::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x00496df8
// [thunk]:EntityBuildingArmedDynamic::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x00496e03
// [thunk]:EntityBuildingArmedDynamic::`scalar deleting destructor' adjustor{608}

void* EntityBuildingArmedDynamic::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

void EntityBuildingArmedDynamic::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}
