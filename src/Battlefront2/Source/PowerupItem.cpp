#include "PowerupItem.h"

#include <stdio.h>

#include "Damageable.h"
#include "EntitySoldier.h"
#include "NetGame.h"
#include "PowerupUtility.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// FUNCTION: SWBF2 0x00623cc0
PowerupItemClass::PowerupItemClass(unsigned int uHash)
    : EntityItemClass(uHash)
{
    for (int i = 0; i < 8; i++)
    {
        mAddHealth[i] = 0.0f;
        mAddAmmo[i] = 0.0f;
        mAddEnergy[i] = 0.0f;
    }
    mBuffOffenseTimer = 0.0f;
    mBuffOffenseMult = 0.0f;
    mBuffDefenseTimer = 0.0f;
    mBuffDefenseMult = 0.0f;
    mBuffHealthTimer = 0.0f;
    mBuffHealthRate = 0.0f;
    mDebuffDamageTimer = 0.0f;
    mDebuffDamageRate = 0.0f;
}

// GLOBAL: SWBF2 0x01efa1d0
// SYNTHETIC: SWBF2 0x00405f90
// `dynamic initializer for 'PowerupItemClass::rttiHashPowerupItemClass''
PblHash PowerupItemClass::rttiHashPowerupItemClass("PowerupItemClass");
char PowerupItemClass::rttiNamePowerupItemClass[] = "PowerupItemClass";

// FUNCTION: SWBF2 0x00623890
bool PowerupItemClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashPowerupItemClass || EntityItemClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006238d0
unsigned int PowerupItemClass::GetDerivedRtti()
{
    return rttiHashPowerupItemClass;
}

// FUNCTION: SWBF2 0x006238e0
const char* PowerupItemClass::GetDerivedRttiName()
{
    return rttiNamePowerupItemClass;
}

// SYNTHETIC: SWBF2 0x00623d80
// PowerupItemClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00623db0
PowerupItemClass::PowerupItemClass(PowerupItemClass* pBase, unsigned int uHash)
    : EntityItemClass(pBase, uHash)
{
    for (int i = 0; i < 8; i++)
    {
        mAddHealth[i] = pBase->mAddHealth[i];
        mAddAmmo[i] = pBase->mAddAmmo[i];
        mAddEnergy[i] = pBase->mAddEnergy[i];
    }
    mBuffOffenseTimer = pBase->mBuffOffenseTimer;
    mBuffOffenseMult = pBase->mBuffOffenseMult;
    mBuffDefenseTimer = pBase->mBuffDefenseTimer;
    mBuffDefenseMult = pBase->mBuffDefenseMult;
    mBuffHealthTimer = pBase->mBuffHealthTimer;
    mBuffHealthRate = pBase->mBuffHealthRate;
    mDebuffDamageTimer = pBase->mDebuffDamageTimer;
    mDebuffDamageRate = pBase->mDebuffDamageRate;
    mPowerupSound = pBase->mPowerupSound;
    mAmbientSound = pBase->mAmbientSound;
}

// FUNCTION: SWBF2 0x00623f00
PowerupItemClass::~PowerupItemClass()
{
}

// FUNCTION: SWBF2 0x00623f10
void PowerupItemClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float f;
    switch (uProperty)
    {
    case 0xf33b41df:
        sscanf(pszValue, "%f", &f);
        mAddHealth[HEALTH_TYPE_PERSON] = f;
        mAddHealth[HEALTH_TYPE_DROID] = f;
        break;
    case 0xf51ad206:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_PERSON]);
        break;
    case 0x60d746d7:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x9622c73b:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_DROID]);
        break;
    case 0x14e381c9:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xa6c59e9f:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_BUILDING]);
        break;
    case 0xc3da529f:
        sscanf(pszValue, "%f", &f);
        mAddAmmo[HEALTH_TYPE_PERSON] = f;
        mAddAmmo[HEALTH_TYPE_DROID] = f;
        break;
    case 0x738f412e:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_PERSON]);
        break;
    case 0xbfe8e397:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x0fda4dbb:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_DROID]);
        break;
    case 0xe728d955:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xa25f865f:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_BUILDING]);
        break;
    case 0x0adc5145:
        sscanf(pszValue, "%f", &f);
        mAddEnergy[HEALTH_TYPE_PERSON] = f;
        mAddEnergy[HEALTH_TYPE_DROID] = f;
        break;
    case 0x84622134:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_PERSON]);
        break;
    case 0xfb0507ad:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x3fe43479:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_DROID]);
        break;
    case 0x56238a9b:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xda2c3905:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_BUILDING]);
        break;
    case 0x70b204af:
        sscanf(pszValue, "%f", &mBuffOffenseTimer);
        break;
    case 0xce532e64:
        sscanf(pszValue, "%f", &mBuffOffenseMult);
        break;
    case 0x7705fcc1:
        sscanf(pszValue, "%f", &mBuffDefenseTimer);
        break;
    case 0xb25fb62e:
        sscanf(pszValue, "%f", &mBuffDefenseMult);
        break;
    case 0x460b143b:
        sscanf(pszValue, "%f", &mBuffHealthTimer);
        break;
    case 0xb6639618:
        sscanf(pszValue, "%f", &mBuffHealthRate);
        break;
    case 0x805b51e7:
        sscanf(pszValue, "%f", &mDebuffDamageTimer);
        break;
    case 0x9834a7b4:
        sscanf(pszValue, "%f", &mDebuffDamageRate);
        break;
    case 0xa2a034ea:
        mPowerupSound.SetID(pszValue, false, false);
        break;
    case 0x65a920ba:
        mAmbientSound.SetID(pszValue, false, false);
        break;
    default:
        EntityItemClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x006243a0
EntityClass* PowerupItemClass::Derive(unsigned int uHash)
{
    return new PowerupItemClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x00623990
// PowerupItem::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x006244ba
// [thunk]:PowerupItem::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x006244c5
// [thunk]:PowerupItem::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x006244cd
// [thunk]:PowerupItem::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x006244e3
// [thunk]:PowerupItem::`scalar deleting destructor' adjustor{148}

// SYNTHETIC: SWBF2 0x006244d8
// [thunk]:PowerupItem::`scalar deleting destructor'`adjustor{576}'

// GLOBAL: SWBF2 0x01efa1d8
// SYNTHETIC: SWBF2 0x00405f60
// `dynamic initializer for 'PowerupItem::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769340
// `dynamic atexit destructor for 'PowerupItem::sMemoryPool''
MemoryPool PowerupItem::sMemoryPool("PowerupItem", 0x290);

// GLOBAL: SWBF2 0x01efa1d4
// SYNTHETIC: SWBF2 0x00405f80
// `dynamic initializer for 'PowerupItem::rttiHashPowerupItem''
PblHash PowerupItem::rttiHashPowerupItem("PowerupItem");
char PowerupItem::rttiNamePowerupItem[] = "PowerupItem";

class CollisionManager
{
public:
    static bool AddItemBody(CollisionObject* pObject);
};

// FUNCTION: SWBF2 0x00624410
Entity* PowerupItemClass::Build(const EntityDesc& desc)
{
    if (NetGame::IsPreGame())
    {
        return 0;
    }
    if (PowerupItem::sMemoryPool.mUsed >= PowerupItem::sMemoryPool.mCount)
    {
        return 0;
    }

    PowerupItem* pItem = new PowerupItem(this, desc);
    if (pItem)
    {
        CollisionManager::AddItemBody(pItem);
        pItem->ActivatePhysics();
        pItem->BuildEffects();
    }
    return pItem;
}

// FUNCTION: SWBF2 0x006238f0
PowerupItem::PowerupItem(PowerupItemClass* pClass, const EntityDesc& desc)
    : EntityItem(pClass, desc)
{
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x00623870
EntityType PowerupItem::GetEntityType() const
{
    return ENT_POWERUP;
}

// FUNCTION: SWBF2 0x006237f0
bool PowerupItem::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashPowerupItem || uRtti == EntityItem::rttiHashEntityItem || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00623840
unsigned int PowerupItem::GetDerivedRtti() const
{
    return rttiHashPowerupItem;
}

// FUNCTION: SWBF2 0x00623850
const char* PowerupItem::GetDerivedRttiName() const
{
    return rttiNamePowerupItem;
}

// FUNCTION: SWBF2 0x00623880
bool PowerupItem::AddToTeamManager() const
{
    return false;
}

// FUNCTION: SWBF2 0x006239c0
PowerupItem::~PowerupItem()
{
    NetGame::Destroyed(this);
    mAmbientSound.Stop(true);
}

// FUNCTION: SWBF2 0x00623a60
bool PowerupItem::Update(float fDeltaTime)
{
    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mMatrix.m[3][1] < -500.0f)
    {
        Kill();
        return false;
    }

    const PowerupItemClass* pClass = static_cast<const PowerupItemClass*>(mClass);
    const PblVector3* pPos = (const PblVector3*)mMatrix.m[3];
    if (mAmbientSound.GetPlaying())
    {
        mAmbientSound.Update(&pClass->mAmbientSound, pPos, &mVelocity);
    }
    else
    {
        mAmbientSound.Play(&pClass->mAmbientSound, pPos, &mVelocity, true);
    }
    return EntityItem::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00623b10
bool PowerupItem::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                    Restrictor* pRestrictor)
{
    if (IsAlive() && (mStationary || mClass->mLifeSpan - mLifeTimer > 0.5f))
    {
        GameObject* pObject = pOther->GetGameObject();
        if (pObject && pObject->IsAlive())
        {
            const PowerupItemClass* pClass = static_cast<const PowerupItemClass*>(mClass);
            int iHealthType = pObject->mHealthType;
            float fHealth = pClass->mAddHealth[iHealthType];
            float fAmmo = pClass->mAddAmmo[iHealthType];
            float fEnergy = pClass->mAddEnergy[iHealthType];

            bool bUsed = PowerupUtility::ApplyBuff(pObject, pClass->mBuffOffenseTimer, pClass->mBuffOffenseMult,
                                                   pClass->mBuffDefenseTimer, pClass->mBuffDefenseMult,
                                                   pClass->mBuffHealthTimer, pClass->mBuffHealthRate,
                                                   pClass->mDebuffDamageTimer, pClass->mDebuffDamageRate);
            bUsed |= PowerupUtility::ApplyHealth(pObject, fHealth, 0, false);
            bUsed |= PowerupUtility::ApplyAmmo(pObject, fAmmo, true);
            bUsed |= PowerupUtility::ApplyEnergy(pObject, fEnergy);
            if (bUsed)
            {
                const PblVector3& vPos = GetPosition();
                if (pObject->GetEntityType() != ENT_SOLDIER ||
                    !static_cast<EntitySoldier*>(pObject)->mMarkedForDeath)
                {
                    pClass->mPowerupSound.Play(&vPos, 0, false, true);
                }
                InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
                mIsAlive = false;
                return true;
            }
        }
    }
    return EntityItem::CollisionCallback(pResult, pOther, pRestrictor);
}
