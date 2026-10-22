#include <stdio.h>

#include "WeaponRepair.h"

#include "Aimer.h"
#include "Character.h"
#include "CommandFlyer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntityItem.h"
#include "EntitySoldier.h"
#include "EventManager_Object.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "NetGame.h"
#include "PowerupUtility.h"
#include "StatusDisplay.h"
#include "Trigger.h"

namespace GameLoop { extern float sClientDeltaTime; }

class AmmoCounter
{
public:
    void ExpendRounds(float fRounds);
};
class EntityWalker { public: static PblHash rttiHashEntityWalker; };
class EntityHover { public: static PblHash rttiHashEntityHover; };
class EntityMine { public: static PblHash rttiHashEntityMine; };
class CommandWalker { public: static PblHash rttiHashCommandWalker; };

namespace EventManager
{
namespace ObjectEvent
{
extern Event<GameObject, DamageOwner> mRepair;
extern Event<GameObject, DamageOwner> mHack;
}
}

// GLOBAL: SWBF2 0x01f9a90c
// SYNTHETIC: SWBF2 0x00407290
// `dynamic initializer for 'WeaponRepairClass::rttiHashWeaponRepairClass''
PblHash WeaponRepairClass::rttiHashWeaponRepairClass("WeaponRepairClass");
char WeaponRepairClass::rttiNameWeaponRepairClass[] = "WeaponRepairClass";

// FUNCTION: SWBF2 0x006910c0
WeaponRepairClass::WeaponRepairClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mTriggerSingle = false;
    mTargetSides = 6;
    mTargetTypes = 0;
    mAITargetTypes = 0;

    mShotDelay = 0.125f;
    for (int i = 0; i < 8; ++i)
    {
        mAddHealth[i] = 0.0f;
        mAddAmmo[i] = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00691160
WeaponRepairClass::WeaponRepairClass(WeaponRepairClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mShotDelay = pBase->mShotDelay;
    for (int i = 0; i < 8; ++i)
    {
        mAddHealth[i] = pBase->mAddHealth[i];
        mAddAmmo[i] = pBase->mAddAmmo[i];
    }
}

// FUNCTION: SWBF2 0x006911d0
WeaponRepairClass::~WeaponRepairClass()
{
}

// SYNTHETIC: SWBF2 0x00691130
// WeaponRepairClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006905f0
bool WeaponRepairClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponRepairClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00690620
unsigned int WeaponRepairClass::GetDerivedRtti()
{
    return rttiHashWeaponRepairClass;
}

// FUNCTION: SWBF2 0x00690630
const char* WeaponRepairClass::GetDerivedRttiName()
{
    return rttiNameWeaponRepairClass;
}

// FUNCTION: SWBF2 0x006911e0
void WeaponRepairClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float fValue;
    int iMask;

    switch (uProperty)
    {
    case 0xd9356908:
        sscanf(pszValue, "%f", &mShotDelay);
        return;
    case 0xf51ad206:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[0] = fValue;
        iMask = 1;
        break;
    case 0x60d746d7:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[1] = fValue;
        iMask = 2;
        break;
    case 0x9622c73b:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[2] = fValue;
        iMask = 4;
        break;
    case 0xf33b41df:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[0] = fValue;
        mAddHealth[2] = fValue;
        iMask = 5;
        break;
    case 0x14e381c9:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[3] = fValue;
        iMask = 8;
        break;
    case 0xa6c59e9f:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[4] = fValue;
        iMask = 0x10;
        break;
    case 0x03ec6220:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[5] = fValue;
        iMask = 0x20;
        break;
    case 0x09cc7f53:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[6] = fValue;
        iMask = 0x40;
        break;
    case 0xf5ed2064:
        sscanf(pszValue, "%f", &fValue);
        mAddHealth[7] = fValue;
        iMask = 0x80;
        break;
    case 0x738f412e:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[0] = fValue;
        iMask = 1;
        break;
    case 0xbfe8e397:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[1] = fValue;
        iMask = 2;
        break;
    case 0x0fda4dbb:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[2] = fValue;
        iMask = 4;
        break;
    case 0xc3da529f:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[0] = fValue;
        mAddAmmo[2] = fValue;
        iMask = 5;
        break;
    case 0xe728d955:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[3] = fValue;
        iMask = 8;
        break;
    case 0xa25f865f:
        sscanf(pszValue, "%f", &fValue);
        mAddAmmo[4] = fValue;
        iMask = 0x10;
        break;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        return;
    }

    if (fValue != 0.0f)
    {
        mTargetTypes |= iMask;
        mAITargetTypes |= iMask;
    }
}

// FUNCTION: SWBF2 0x00691620
WeaponClass* WeaponRepairClass::Derive(unsigned int uHash)
{
    return new WeaponRepairClass(this, uHash);
}

// FUNCTION: SWBF2 0x00691690
Weapon* WeaponRepairClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponRepair(this, desc);
}

// FUNCTION: SWBF2 0x00690640
WeaponRepair::WeaponRepair(const WeaponRepairClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
}

// GLOBAL: SWBF2 0x01f9a910
// SYNTHETIC: SWBF2 0x00407280
// `dynamic initializer for 'WeaponRepair::rttiHashWeaponRepair''
PblHash WeaponRepair::rttiHashWeaponRepair("WeaponRepair");
char WeaponRepair::rttiNameWeaponRepair[] = "WeaponRepair";

// FUNCTION: SWBF2 0x006905a0
bool WeaponRepair::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponRepair || reinterpret_cast<const Weapon*>(this)->Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006905d0
unsigned int WeaponRepair::GetDerivedRtti() const
{
    return rttiHashWeaponRepair;
}

// FUNCTION: SWBF2 0x006905e0
const char* WeaponRepair::GetDerivedRttiName() const
{
    return rttiNameWeaponRepair;
}

// FUNCTION: SWBF2 0x00690830
bool WeaponRepair::IsBusy() const
{
    if (mState != FIRE && mState != FIRE2)
    {
        return false;
    }
    return !mFired || mStateLimit > mStateTimer + GameLoop::sClientDeltaTime;
}

// FUNCTION: SWBF2 0x00690810
bool WeaponRepair::Update(float fDeltaTime)
{
    return Weapon::Update(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x00690660
// WeaponRepair::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00690690
WeaponRepair::~WeaponRepair()
{
    StatusDisplay::Instance(NetGame::GetJoystickIndex(mOwner->mInitialPlayerId))->SetRepairWeapon(0);
}

// FUNCTION: SWBF2 0x006906f0
bool WeaponRepair::IsLocked() const
{
    GameObject* pTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    if (!pTarget)
    {
        pTarget = mOwner->GetGameObject();
        if (!pTarget)
        {
            return false;
        }
    }

    const WeaponRepairClass* pClass = static_cast<const WeaponRepairClass*>(mClass);
    bool bHack = false;
    float fHealth = pClass->mAddHealth[pTarget->mHealthType];
    float fAmmo = pClass->mAddAmmo[pTarget->mHealthType];
    if (pTarget->GetControllable())
    {
        PilotType ePilot = pTarget->GetControllable()->mPilotType;
        if ((ePilot == PILOT_VEHICLE || ePilot == PILOT_VEHICLESELF ||
             pTarget->GetControllable()->mPilotType == PILOT_REMOTE) &&
            mOwner->GetGameObject() && mOwner->GetGameObject()->IsMyEnemy(pTarget))
        {
            bHack = true;
        }
    }

    bool bAmmo = PowerupUtility::CheckAmmo(pTarget, fAmmo, false);
    bool bHealth = PowerupUtility::CheckHealth(pTarget, fHealth);
    return (bAmmo | bHack | bHealth) != 0;
}

// FUNCTION: SWBF2 0x00690870
void WeaponRepair::EnterFire()
{
    Weapon::EnterFire();
    mStateTimer = 0.0f;
    mStateLimit = static_cast<const WeaponRepairClass*>(mClass)->mShotDelay;
    mFired = false;
    if (mClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x006908c0
bool WeaponRepair::UpdateFire(float fDeltaTime)
{
    if (mStateLimit <= 0.0f)
    {
        mFired = true;
        if (Fire(fDeltaTime))
        {
            m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot * fDeltaTime);
            m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot * fDeltaTime);
            mAimer->UpdateFire(mAimer, true);
        }
    }
    else
    {
        float fTimeLeft = fDeltaTime;
        while (fTimeLeft > 0.0f)
        {
            if (mFired)
            {
                float fStep = mStateLimit - mStateTimer;
                if (fStep > fTimeLeft)
                {
                    fStep = fTimeLeft;
                }
                fTimeLeft -= fStep;
                mStateTimer += fStep;
                if (mStateTimer < mStateLimit || mClass->mTriggerSingle || !CheckFire() ||
                    CheckReload() || CheckEmpty())
                {
                    break;
                }
                mStateTimer -= mStateLimit;
                Weapon::ExitFire();
                Weapon::EnterFire();
                mFired = false;
            }
            else
            {
                mFired = true;
                if (Fire(1.0f))
                {
                    m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot * fDeltaTime);
                    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot * fDeltaTime);
                    mAimer->UpdateFire(mAimer, true);
                }
            }
        }
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00690aa0
void WeaponRepair::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00690ab0
bool WeaponRepair::CheckFire() const
{
    if (NetGame::IsPreGame() || !mTrigger || CheckOverheat() || !IsLocked())
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00690af0
bool WeaponRepair::FireDone() const
{
    return mFired && mStateTimer >= mStateLimit &&
           (mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty());
}

// FUNCTION: SWBF2 0x00690b50
bool WeaponRepair::Fire(float fDeltaTime)
{
    GameObject* pTarget = const_cast<GameObject*>(static_cast<const GameObject*>(mTarget));
    if (!pTarget)
    {
        pTarget = mOwner->GetGameObject();
    }

    mClass->CreateMuzzleFlash(mFirePointMatrix);

    if (!pTarget)
    {
        return false;
    }

    const WeaponRepairClass* pClass = static_cast<const WeaponRepairClass*>(mClass);
    float fHealth = pClass->mAddHealth[pTarget->mHealthType] * fDeltaTime;
    float fAmmo = pClass->mAddAmmo[pTarget->mHealthType] * fDeltaTime;

    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    owner.mCharacter = mOwner->mCharacter;
    owner.mGameObject = mOwner->GetGameObject();
    owner.mTeam = mOwner->GetGameObject()->mTeam;
    owner.mClass = mOwner->GetGameObject()->GetEntityClass();
    owner.mWeapon = mClass;

    if (pTarget->IsRtti(EntityWalker::rttiHashEntityWalker) ||
        pTarget->IsRtti(EntityHover::rttiHashEntityHover) ||
        pTarget->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        if (mOwner->GetGameObject()->IsMyEnemy(pTarget) &&
            pTarget->GetEntityType() != ENT_TAUNTAUN &&
            !pTarget->IsRtti(CommandFlyer::rttiHashCommandFlyer) &&
            !pTarget->IsRtti(CommandWalker::rttiHashCommandWalker))
        {
            int iSoldiers = 0;
            const PblList<Controllable>& list = pTarget->GetControllableList();
            for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
            {
                if (it->GetPilot() && it->GetPilot()->GetGameObject() &&
                    it->GetPilot()->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
                {
                    ++iSoldiers;
                }
            }

            if (iSoldiers && pTarget->GetGameObjectClass()->GetTimeRequiredToEject() > 0.0f)
            {
                pTarget->SetEjectTimer(pClass->mShotDelay + pTarget->GetEjectTimer());
                pTarget->ResetTimeSinceLastHack();
                if (pTarget->GetEjectTimer() > pTarget->GetGameObjectClass()->GetTimeRequiredToEject())
                {
                    EventManager::ObjectEvent::mHack.Trigger(*pTarget, owner);

                    for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
                    {
                        if (it->GetPilot() && it->GetPilot()->GetGameObject() &&
                            it->GetPilot()->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
                        {
                            EntitySoldier* pSoldier = static_cast<EntitySoldier*>(it->GetPilot());
                            Character* pCharacter = pSoldier->mCharacter;
                            if (pCharacter)
                            {
                                if (pCharacter->mRemote)
                                {
                                    pSoldier->ExitRemote();
                                }
                                if (pCharacter->GetVehicle())
                                {
                                    pSoldier->ExitVehicle(false, true);
                                }
                            }
                        }
                    }

                    pTarget->SetEjectTimer(0.0f);
                    pTarget->mDisableTime = pTarget->GetGameObjectClass()->GetReboardTime();
                    if (mClass->mScoreForMedalsType >= 0 && mOwner->mCharacter &&
                        mOwner->mCharacter->mMedalsMgr)
                    {
                        mOwner->mCharacter->mMedalsMgr->AddMedalPoints(
                            mClass->mScoreForMedalsType, false, false, (HealthType)pTarget->mHealthType,
                            true, false, 1);
                    }
                }
                return true;
            }
        }
    }
    else if (pTarget->IsRtti(EntityMine::rttiHashEntityMine))
    {
        EntityItem* pMine = static_cast<EntityItem*>(pTarget);
        if (pMine->GetLifeTimer() > 0.0f)
        {
            pMine->mLifeTimer += fHealth;
            return true;
        }
        return false;
    }

    bool bHealth = PowerupUtility::ApplyHealth(pTarget, fHealth, &owner, false);
    if (PowerupUtility::ApplyAmmo(pTarget, fAmmo, false) | bHealth)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && pTarget->mCurHealth == pTarget->mMaxHealth)
        {
            EventManager::ObjectEvent::mRepair.Trigger(*pTarget, owner);
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00691060
void WeaponRepair::Select(unsigned int uFlags, bool bOffhand)
{
    StatusDisplay::Instance(NetGame::GetJoystickIndex(mOwner->mPlayerId))->SetRepairWeapon(this);
    Weapon::Select(uFlags, bOffhand);
}

// FUNCTION: SWBF2 0x00691090
void WeaponRepair::Deselect(unsigned int uFlags, bool bOffhand)
{
    StatusDisplay::Instance(NetGame::GetJoystickIndex(mOwner->mPlayerId))->SetRepairWeapon(0);
    Weapon::Deselect(uFlags, bOffhand);
}
