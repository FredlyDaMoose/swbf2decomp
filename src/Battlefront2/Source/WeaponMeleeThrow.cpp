#include <stdio.h>
#include <stdlib.h>
#include <new>

#include "WeaponMeleeThrow.h"
#include "WeaponMeleeInstance.h"
#include "Aimer.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "SoldierAnimatorClass.h"
#include "RedEngineFL/Graphics/RedPose.h"

extern bool netIsLocalTurn;

namespace
{
    EntityClass* GetEntityClass(GameObject* pObject)
    {
        typedef EntityClass* (__thiscall *Fn)(GameObject*);
        return pObject->GetEntityClass();
    }
}

// GLOBAL: SWBF2 0x01f9a8fc
// SYNTHETIC: SWBF2 0x00407240
// `dynamic initializer for 'WeaponMeleeThrow::rttiHashWeaponMeleeThrow''
PblHash WeaponMeleeThrow::rttiHashWeaponMeleeThrow("WeaponMeleeThrow");
char WeaponMeleeThrow::rttiNameWeaponMeleeThrow[] = "WeaponMeleeThrow";

// FUNCTION: SWBF2 0x0068f610
WeaponMeleeThrow::WeaponMeleeThrow(const WeaponMeleeThrowClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    m_bFired = 0;
    m_bReturnedThisFrame = 0;
    m_pPrimaryWeapon = 0;

    EntitySoldier* pSoldier = static_cast<EntitySoldier*>(mOwner->GetGameObject());
    Controllable* pWeapons = static_cast<Controllable*>(pSoldier);
    int iMeleeIndex = pClass->m_iPrimaryWeaponIndex;
    for (int i = 0; i < pWeapons->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = pWeapons->GetWeaponPtr(i);
        if (pWeapon && pWeapon->IsMelee())
        {
            m_pPrimaryWeapon = static_cast<WeaponMelee*>(pWeapons->GetWeaponPtr(i));
            if (iMeleeIndex-- == 0)
            {
                break;
            }
        }
    }

    const WeaponMeleeClass* pPrimaryClass =
        static_cast<const WeaponMeleeClass*>(m_pPrimaryWeapon->mClass);

    const unsigned char uiThrowAnim = static_cast<unsigned char>(pPrimaryClass->m_pCombo->m_uiThrowAnimIndex);
    SoldierAnimatorClass::sInstance->GetAnimationBoneMatrix(
        static_cast<SoldierAnimationBank::MAP>(m_pPrimaryWeapon->mSoldierAnimationMap),
        uiThrowAnim, 0x2B960099, pClass->m_fInitialSalvoDelay, m_mThrow0Local);
}

// FUNCTION: SWBF2 0x0068f520
bool WeaponMeleeThrow::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponMeleeThrow || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0068f550
unsigned int WeaponMeleeThrow::GetDerivedRtti() const
{
    return rttiHashWeaponMeleeThrow;
}

// FUNCTION: SWBF2 0x0068f560
const char* WeaponMeleeThrow::GetDerivedRttiName() const
{
    return rttiNameWeaponMeleeThrow;
}

// FUNCTION: SWBF2 0x0068f570
bool WeaponMeleeThrow::IsMeleeThrow() const
{
    return true;
}

// FUNCTION: SWBF2 0x0068f580
void WeaponMeleeThrow::SetOrdnance(Ordnance*  )
{
}

// FUNCTION: SWBF2 0x0068fca0
void WeaponMeleeThrow::NotifyThrow()
{
    m_bThrow = 1;
}

// FUNCTION: SWBF2 0x0068fcb0
void WeaponMeleeThrow::NotifyReturned()
{
    m_bThrow = 0;
    m_bReturnedThisFrame = 1;
}

// FUNCTION: SWBF2 0x0068f810
bool WeaponMeleeThrow::IsBusy() const
{
    return (mState == FIRE || mState == FIRE2) && (!m_bFired || m_bThrow);
}

// FUNCTION: SWBF2 0x0068f7f0
bool WeaponMeleeThrow::Update(float fDeltaTime)
{
    return Weapon::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0068f940
void WeaponMeleeThrow::EnterEmpty()
{
    Weapon::EnterEmpty();
}

// FUNCTION: SWBF2 0x0068fa20
bool WeaponMeleeThrow::ReloadDone() const
{
    return Weapon::ReloadDone();
}

// FUNCTION: SWBF2 0x0068f9b0
bool WeaponMeleeThrow::FireDone() const
{
    return m_bFired && !m_bThrow && Weapon::FireDone();
}

// FUNCTION: SWBF2 0x0068f720
WeaponMeleeThrow::~WeaponMeleeThrow()
{
}

// SYNTHETIC: SWBF2 0x0068f590
// WeaponMeleeThrow::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068f730
void WeaponMeleeThrow::Render(const PblMatrix& matrix, const RedPose* pPose,
    const RedColor& color, unsigned int uiFlags, bool bRenderModel) const
{
    uiFlags |= 1;
    if (m_bThrow && pPose)
    {
        const PblMatrix* pThrowMatrix = pPose->Find(0x2B960099);
        if (pThrowMatrix)
        {
            RedPose* pMutablePose = const_cast<RedPose*>(pPose);
            pMutablePose->Remove(0x2B960099);
            m_pPrimaryWeapon->Render(matrix, pPose, color, uiFlags, bRenderModel);
            pMutablePose->Store(0x2B960099, pThrowMatrix);
            return;
        }
    }
    m_pPrimaryWeapon->Render(matrix, pPose, color, uiFlags, bRenderModel);
}

// FUNCTION: SWBF2 0x0068f840
void WeaponMeleeThrow::EnterFire()
{
    Weapon::EnterFire();
    mStateTimer = 0.0f;
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy)
    {
        pEnergy->SpendEnergy(mClass->mEnergyDrain);
    }
    m_bFired = 0;
    if (mClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 1;
    }
}

// FUNCTION: SWBF2 0x0068f920
void WeaponMeleeThrow::ExitFire()
{
    Weapon::ExitFire();
    m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
}

// FUNCTION: SWBF2 0x0068f950
bool WeaponMeleeThrow::CheckFire() const
{
    if (!mTrigger)
    {
        return false;
    }
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy && pEnergy->IsOverheating() && mClass->mEnergyDrain > 0.0f)
    {
        return false;
    }
    return !Weapon::CheckOverheat() && mTrigger->IsPressed();
}

// FUNCTION: SWBF2 0x0068f9d0
bool WeaponMeleeThrow::CheckEmpty() const
{
    EnergyBar* pEnergy = mOwner->GetEnergyBar();
    if (pEnergy && pEnergy->IsOverheating() && mClass->mEnergyDrain > 0.0f)
    {
        return true;
    }
    return Weapon::CheckEmpty();
}

// FUNCTION: SWBF2 0x0068f8a0
bool WeaponMeleeThrow::UpdateFire(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (!m_bFired)
    {
        if (static_cast<const WeaponMeleeThrowClass*>(mClass)->m_fInitialSalvoDelay > mStateTimer)
        {
            return true;
        }
        if (!mClass->mInstantPlayFireAnim)
        {
            mFiredFlag = 1;
        }
        Fire();
        m_bFired = 1;
        mStateTimer = 0.0f;
    }
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x0068fa30
void WeaponMeleeThrow::Fire()
{
    OrdnanceDesc desc;
    desc.mPosition = mAimer->mFirePos;
    desc.mDirection = mAimer->mDirection;
    desc.mStrength = 1.0f;
    GameObject* pOwnerObject = mOwner->GetGameObject();
    desc.mOwner = pOwnerObject;
    desc.mDamageOwner = DamageOwner(mOwner->mCharacter, pOwnerObject,
        pOwnerObject ? pOwnerObject->GetTeam() : 0,
        GetEntityClass(pOwnerObject), mClass, mOwner->GetOffenseMultiplier());
    desc.mTarget = IsLocked() ? const_cast<GameObject*>(mTarget.GetValidObject()) : 0;
    desc.mBodyID = IsLocked() ? mTargetBodyID : -1;
    desc.mFireWeapon = this;

    bool bFlash = true;
    if (NetGame::IsEnabled())
    {
        if (NetGame::IsClient())
        {
            if (!netIsLocalTurn || NetGame::GetJoystickIndex(mOwner->mPlayerId) < 0)
            {
                return;
            }
        }
        else
        {
            bFlash = NetGame::IsOneWorld() && !NetGame::IsDedicated();
            NetGame::CreateEvent_CreateOrdnance(mClass->mOrdnanceClass, desc);
        }
    }
    if (bFlash)
    {
        mClass->CreateMuzzleFlash(mAimer->GetFireMatrix());
    }
    Ordnance* pOrdnance = CreateOrdnance(desc);
    if (mClass->mInstantPlayFireAnim)
    {
        mFiredFlag = 0;
    }
    SetOrdnance(pOrdnance);
}

// FUNCTION: SWBF2 0x0068fc90
Ordnance* WeaponMeleeThrow::CreateOrdnance(OrdnanceDesc& desc)
{
    return mClass->mOrdnanceClass->Build(desc);
}

// GLOBAL: SWBF2 0x01f9a900
// SYNTHETIC: SWBF2 0x00407250
// `dynamic initializer for 'WeaponMeleeThrowClass::rttiHashWeaponMeleeThrowClass''
PblHash WeaponMeleeThrowClass::rttiHashWeaponMeleeThrowClass("WeaponMeleeThrowClass");
char WeaponMeleeThrowClass::rttiNameWeaponMeleeThrowClass[] = "WeaponMeleeThrowClass";

// FUNCTION: SWBF2 0x0068fcd0
WeaponMeleeThrowClass::WeaponMeleeThrowClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    m_iPrimaryWeaponIndex = 0;
    m_fInitialSalvoDelay = 0.0f;
}

// FUNCTION: SWBF2 0x0068f5c0
bool WeaponMeleeThrowClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponMeleeThrowClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0068f5f0
unsigned int WeaponMeleeThrowClass::GetDerivedRtti()
{
    return rttiHashWeaponMeleeThrowClass;
}

// FUNCTION: SWBF2 0x0068f600
const char* WeaponMeleeThrowClass::GetDerivedRttiName()
{
    return rttiNameWeaponMeleeThrowClass;
}

// SYNTHETIC: SWBF2 0x0068fd00
// WeaponMeleeThrowClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0068fd30
WeaponMeleeThrowClass::WeaponMeleeThrowClass(WeaponMeleeThrowClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    m_iPrimaryWeaponIndex = pBase->m_iPrimaryWeaponIndex;
    m_fInitialSalvoDelay = pBase->m_fInitialSalvoDelay;
}

// FUNCTION: SWBF2 0x0068fd70
WeaponMeleeThrowClass::~WeaponMeleeThrowClass()
{
}

// FUNCTION: SWBF2 0x0068fd80
void WeaponMeleeThrowClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xC8388D9D)
    {
        sscanf(pszValue, "%f", &m_fInitialSalvoDelay);
    }
    else if (uProperty == 0xF83F717B)
    {
        m_iPrimaryWeaponIndex = atoi(pszValue);
    }
    else
    {
        WeaponClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x0068fde0
WeaponClass* WeaponMeleeThrowClass::Derive(unsigned int uHash)
{
    return new WeaponMeleeThrowClass(this, uHash);
}

// FUNCTION: SWBF2 0x0068fe50
Weapon* WeaponMeleeThrowClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    void* pMemory = Weapon::operator new(sizeof(WeaponMeleeThrow));
    return pMemory ? ::new (pMemory) WeaponMeleeThrow(this, desc) : 0;
}
