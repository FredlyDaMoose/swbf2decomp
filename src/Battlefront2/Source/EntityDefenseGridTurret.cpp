#include "EntityDefenseGridTurret.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "CollisionManager.h"
#include "NetGame.h"

#include <stdio.h>

// GLOBAL: SWBF2 0x01ebb990
// SYNTHETIC: SWBF2 0x004029b0
// `dynamic initializer for 'EntityDefenseGridTurretClass::rttiHashEntityDefenseGridTurretClass''
PblHash EntityDefenseGridTurretClass::rttiHashEntityDefenseGridTurretClass("EntityDefenseGridTurretClass");
char EntityDefenseGridTurretClass::rttiNameEntityDefenseGridTurretClass[] = "EntityDefenseGridTurretClass";

// GLOBAL: SWBF2 0x01ebb994
// SYNTHETIC: SWBF2 0x004029a0
// `dynamic initializer for 'EntityDefenseGridTurret::rttiHashEntityDefenseGridTurret''
PblHash EntityDefenseGridTurret::rttiHashEntityDefenseGridTurret("EntityDefenseGridTurret");
char EntityDefenseGridTurret::rttiNameEntityDefenseGridTurret[] = "EntityDefenseGridTurret";

// GLOBAL: SWBF2 0x01ebb940
// SYNTHETIC: SWBF2 0x00402980
// `dynamic initializer for 'EntityDefenseGridTurret::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f20
// `dynamic atexit destructor for 'EntityDefenseGridTurret::sMemoryPool''
MemoryPool EntityDefenseGridTurret::sMemoryPool("EntityDefenseGridTurret", 0x6c0);

void* EntityDefenseGridTurret::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

void EntityDefenseGridTurret::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

// FUNCTION: SWBF2 0x00497ad0
EntityDefenseGridTurret::EntityDefenseGridTurret(EntityDefenseGridTurretClass* pClass,
                                                   const EntityDesc& desc)
    : EntityBuildingArmedDynamic(pClass, desc)
{
    mClass = pClass;
    mRespawnTimer = pClass->mRespawnTime;
}

EntityDefenseGridTurret::~EntityDefenseGridTurret()
{
}

// FUNCTION: SWBF2 0x00497b40
bool EntityDefenseGridTurret::Update(float fDeltaTime)
{
    const bool bClient = (netInShell ? netEnabled : netEnabledNext) &&
                         netEnabledNext && netClientActive;
    if (!bClient)
    {
        if (!mIsAlive)
        {
            mRespawnTimer -= fDeltaTime;
            if (mRespawnTimer < 0.0f)
            {
                mRespawnTimer = mClass->mRespawnTime;
                Respawn();
            }
        }
        if (mMatrix.m[3][1] < -500.0f)
        {
            Kill();
            static_cast<RedSceneObject*>(this)->Deactivate();
        }
    }
    return true;
}

// SYNTHETIC: SWBF2 0x00497a30
// EntityDefenseGridTurret::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00497dd2
// [thunk]:EntityDefenseGridTurret::`vector deleting destructor'`adjustor{148}'
// SYNTHETIC: SWBF2 0x00497ddd
// [thunk]:EntityDefenseGridTurret::`vector deleting destructor'`adjustor{320}'
// SYNTHETIC: SWBF2 0x00497de8
// [thunk]:EntityDefenseGridTurret::`vector deleting destructor'`adjustor{12}'
// SYNTHETIC: SWBF2 0x00497df0
// [thunk]:EntityDefenseGridTurret::`vector deleting destructor'`adjustor{512}'
// SYNTHETIC: SWBF2 0x00497dfb
// [thunk]:EntityDefenseGridTurret::`vector deleting destructor'`adjustor{608}'

// FUNCTION: SWBF2 0x004979a0
bool EntityDefenseGridTurret::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityDefenseGridTurret ||
           EntityBuildingArmedDynamic::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00497a00
unsigned int EntityDefenseGridTurret::GetDerivedRtti() const
{
    return rttiHashEntityDefenseGridTurret;
}

// FUNCTION: SWBF2 0x00497a10
const char* EntityDefenseGridTurret::GetDerivedRttiName() const
{
    return rttiNameEntityDefenseGridTurret;
}

// FUNCTION: SWBF2 0x00497d00
Entity* EntityDefenseGridTurretClass::Build(const EntityDesc& desc)
{
    if (EntityDefenseGridTurret::sMemoryPool.mUsed >= EntityDefenseGridTurret::sMemoryPool.mCount)
    {
        return 0;
    }
    EntityDefenseGridTurret* pTurret = new EntityDefenseGridTurret(this, desc);
    if (pTurret)
    {
        CollisionManager::AddItemBody(static_cast<CollisionObject*>(pTurret));
        pTurret->ActivatePhysics();
    }
    return pTurret;
}

// FUNCTION: SWBF2 0x00497bf0
EntityDefenseGridTurretClass::EntityDefenseGridTurretClass(unsigned int uHash)
    : EntityBuildingArmedDynamicClass(uHash)
{
    mRespawnTime = 30.0f;
}

// FUNCTION: SWBF2 0x00497c50
EntityDefenseGridTurretClass::EntityDefenseGridTurretClass(EntityDefenseGridTurretClass* pBase,
                                                           unsigned int uHash)
    : EntityBuildingArmedDynamicClass(pBase, uHash)
{
    mRespawnTime = 30.0f;
}

// FUNCTION: SWBF2 0x00497c80
EntityDefenseGridTurretClass::~EntityDefenseGridTurretClass()
{
}

// SYNTHETIC: SWBF2 0x00497c20
// EntityDefenseGridTurretClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00497c90
EntityClass* EntityDefenseGridTurretClass::Derive(unsigned int uHash)
{
    return new EntityDefenseGridTurretClass(this, uHash);
}

// FUNCTION: SWBF2 0x00497a60
bool EntityDefenseGridTurretClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityDefenseGridTurretClass || EntityBuildingArmedDynamicClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00497ab0
unsigned int EntityDefenseGridTurretClass::GetDerivedRtti()
{
    return rttiHashEntityDefenseGridTurretClass;
}

// FUNCTION: SWBF2 0x00497ac0
const char* EntityDefenseGridTurretClass::GetDerivedRttiName()
{
    return rttiNameEntityDefenseGridTurretClass;
}

// FUNCTION: SWBF2 0x00497da0
void EntityDefenseGridTurretClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xBD94B44E)
    {
        sscanf(pszValue, "%f", &mRespawnTime);
    }
    else
    {
        EntityBuildingArmedDynamicClass::SetProperty(uProperty, pszValue);
    }
}
