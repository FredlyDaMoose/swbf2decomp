#include <stdio.h>

#include "WeaponInvisibility.h"

#include "Character.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "EntitySoldier.h"
#include "GameObject.h"
#include "Trigger.h"
#include "PebbleFL/Common/PblRandom.h"

// GLOBAL: SWBF2 0x01f9a8d4
// SYNTHETIC: SWBF2 0x004071a0
// `dynamic initializer for 'WeaponInvisibility::rttiHashWeaponInvisibility''
PblHash WeaponInvisibility::rttiHashWeaponInvisibility("WeaponInvisibility");
char WeaponInvisibility::rttiNameWeaponInvisibility[] = "WeaponInvisibility";

// FUNCTION: SWBF2 0x00685cc0
bool WeaponInvisibility::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponInvisibility || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00685cf0
unsigned int WeaponInvisibility::GetDerivedRtti() const
{
    return rttiHashWeaponInvisibility;
}

// FUNCTION: SWBF2 0x00685d00
const char* WeaponInvisibility::GetDerivedRttiName() const
{
    return rttiNameWeaponInvisibility;
}

// GLOBAL: SWBF2 0x01f9a8d8
// SYNTHETIC: SWBF2 0x004071b0
// `dynamic initializer for 'WeaponInvisibilityClass::rttiHashWeaponInvisibilityClass''
PblHash WeaponInvisibilityClass::rttiHashWeaponInvisibilityClass("WeaponInvisibilityClass");
char WeaponInvisibilityClass::rttiNameWeaponInvisibilityClass[] = "WeaponInvisibilityClass";

// FUNCTION: SWBF2 0x006865f0
WeaponInvisibilityClass::WeaponInvisibilityClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mTargetSides = 0;
    mTargetTypes = 0;
    mAITargetTypes = 0;

    m_eInvisibilityTrigger = TRIGGER_TOGGLE;
    m_fInvisibilityMin = 0.4f;
    m_fInvisibilityMax = 0.8f;
    m_fSpeedForInvisibilityMin = 8.0f;
    m_fSpeedForInvisibilityMax = 2.0f;
    m_fInvisibilityIncRate = 0.4f;
    m_fInvisibilityDecRate = 0.8f;
    m_fInvisibilityPersistTime = 0.0f;
    m_fFlickerAmplitude = 0.0f;
    m_fFlickerTimeMin = 10.0f;
    m_fFlickerTimeMax = 10.0f;
    mInitialSalvoDelay = 0.6f;
    mInitialEnergyDrain = 0.0f;
}

// FUNCTION: SWBF2 0x006866e0
WeaponInvisibilityClass::WeaponInvisibilityClass(WeaponInvisibilityClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    m_eInvisibilityTrigger = pBase->m_eInvisibilityTrigger;
    m_fInvisibilityMin = pBase->m_fInvisibilityMin;
    m_fInvisibilityMax = pBase->m_fInvisibilityMax;
    m_fSpeedForInvisibilityMin = pBase->m_fSpeedForInvisibilityMin;
    m_fSpeedForInvisibilityMax = pBase->m_fSpeedForInvisibilityMax;
    m_fInvisibilityIncRate = pBase->m_fInvisibilityIncRate;
    m_fInvisibilityDecRate = pBase->m_fInvisibilityDecRate;
    m_fInvisibilityPersistTime = pBase->m_fInvisibilityPersistTime;
    m_fFlickerAmplitude = pBase->m_fFlickerAmplitude;
    m_fFlickerTimeMin = pBase->m_fFlickerTimeMin;
    m_fFlickerTimeMax = pBase->m_fFlickerTimeMax;
    mInitialSalvoDelay = pBase->mInitialSalvoDelay;
    mInitialEnergyDrain = pBase->mInitialEnergyDrain;
}

// FUNCTION: SWBF2 0x006867a0
WeaponInvisibilityClass::~WeaponInvisibilityClass()
{
}

// SYNTHETIC: SWBF2 0x006866b0
// WeaponInvisibilityClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00685d10
bool WeaponInvisibilityClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponInvisibilityClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00685d40
unsigned int WeaponInvisibilityClass::GetDerivedRtti()
{
    return rttiHashWeaponInvisibilityClass;
}

// FUNCTION: SWBF2 0x00685d50
const char* WeaponInvisibilityClass::GetDerivedRttiName()
{
    return rttiNameWeaponInvisibilityClass;
}

// FUNCTION: SWBF2 0x006867b0
void WeaponInvisibilityClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x2f19df96:
        switch (PblHash(pszValue))
        {
        case 0x01fcf9b4:
        case 0x350ca8af:
            m_eInvisibilityTrigger = TRIGGER_PRESS;
            break;
        case 0x40296205:
        case 0x340ca71c:
            m_eInvisibilityTrigger = TRIGGER_TOGGLE;
            break;
        case 0xadc649b8:
        case 0x370cabd5:
            m_eInvisibilityTrigger = TRIGGER_HOLD;
            break;
        }
        break;
    case 0xfb2e6bba:
        sscanf(pszValue, "%f", &m_fInvisibilityMin);
        break;
    case 0xe54363d0:
        sscanf(pszValue, "%f", &m_fInvisibilityMax);
        break;
    case 0xe0716444:
        sscanf(pszValue, "%f", &m_fSpeedForInvisibilityMin);
        break;
    case 0xee850206:
        sscanf(pszValue, "%f", &m_fSpeedForInvisibilityMax);
        break;
    case 0xb6243814:
        sscanf(pszValue, "%f", &m_fInvisibilityIncRate);
        break;
    case 0x5d77dec4:
        sscanf(pszValue, "%f", &m_fInvisibilityDecRate);
        break;
    case 0xa591fd27:
        sscanf(pszValue, "%f", &m_fInvisibilityPersistTime);
        break;
    case 0xe91ab0b8:
        sscanf(pszValue, "%f", &m_fFlickerAmplitude);
        break;
    case 0xc2298dea:
        sscanf(pszValue, "%f", &m_fFlickerTimeMin);
        break;
    case 0xcc3d2560:
        sscanf(pszValue, "%f", &m_fFlickerTimeMax);
        break;
    case 0xc8388d9d:
        sscanf(pszValue, "%f", &mInitialSalvoDelay);
        break;
    case 0xb4932ba7:
        sscanf(pszValue, "%f", &mInitialEnergyDrain);
        break;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00686930
WeaponClass* WeaponInvisibilityClass::Derive(unsigned int uHash)
{
    return new WeaponInvisibilityClass(this, uHash);
}

inline float Controllable::GetEnergy() const
{
    EnergyBar* pEnergyBar = const_cast<Controllable*>(this)->GetEnergyBar();
    return pEnergyBar ? pEnergyBar->GetEnergy() : 0.0f;
}

// FUNCTION: SWBF2 0x006869a0
Weapon* WeaponInvisibilityClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponInvisibility(this, desc);
}

// FUNCTION: SWBF2 0x00685d60
WeaponInvisibility::WeaponInvisibility(const WeaponInvisibilityClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    m_fInvisibilityFactor = 0.0f;
    m_fInvisibilityPersistTimer = 0.0f;
    m_fInvisibilityFlickerTimer = 0.0f;
    m_bInvisibilityOn = false;
    m_bInvisibilityActive = false;
    mFired = false;
    mOwnerHealth = 0.0f;
}

// SYNTHETIC: SWBF2 0x00685dc0
// WeaponInvisibility::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00685df0
WeaponInvisibility::~WeaponInvisibility()
{
}

// FUNCTION: SWBF2 0x00685e00
void WeaponInvisibility::Select(unsigned int uFlags, bool bOffhand)
{
    Weapon::Select(uFlags, bOffhand);
}

// FUNCTION: SWBF2 0x00685e10
void WeaponInvisibility::Deselect(unsigned int uFlags, bool bOffhand)
{
    InputInvisibilityOff();
    Weapon::Deselect(uFlags, bOffhand);
}

// FUNCTION: SWBF2 0x00685e20
void WeaponInvisibility::EnterIdle()
{
    Weapon::EnterIdle();
}

// FUNCTION: SWBF2 0x00685e30
bool WeaponInvisibility::UpdateIdle(float fDeltaTime)
{
    UpdateInvisibility(fDeltaTime);
    return Weapon::UpdateIdle(fDeltaTime);
}

// FUNCTION: SWBF2 0x00685e60
void WeaponInvisibility::ExitIdle()
{
}

// FUNCTION: SWBF2 0x00685e70
void WeaponInvisibility::EnterFire()
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    mStateTimer = 0.0f;

    if (!m_bInvisibilityActive)
    {
        float fDrain = pClass->mInitialEnergyDrain;
        if (EnergyBar* pEnergyBar = mOwner->GetEnergyBar())
        {
            pEnergyBar->SpendEnergy(fDrain);
        }
    }
    if (pClass->m_eInvisibilityTrigger == WeaponInvisibilityClass::TRIGGER_PRESS)
    {
        float fDrain = pClass->mEnergyDrain;
        if (EnergyBar* pEnergyBar = mOwner->GetEnergyBar())
        {
            pEnergyBar->SpendEnergy(fDrain);
        }
    }

    if (pClass->m_eInvisibilityTrigger != WeaponInvisibilityClass::TRIGGER_HOLD && m_bInvisibilityActive)
    {
        if (mOwner->mPlayerId >= 0)
        {
            StopInvisibility();
            mFired = true;
            mTrigger->Clear();
            mStateLimit = 0.0f;
        }
    }
    else
    {
        mFiredFlag = 1;
        mStateLimit = pClass->mInitialSalvoDelay;
        Weapon::EnterFire();
    }
}

// FUNCTION: SWBF2 0x00685f50
bool WeaponInvisibility::UpdateFire(float fDeltaTime)
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    mStateTimer += fDeltaTime;
    if (!mSelectedFlag && pClass->m_eInvisibilityTrigger != WeaponInvisibilityClass::TRIGGER_PRESS)
    {
        mFired = true;
    }
    if (!mFired && mStateTimer >= pClass->mInitialSalvoDelay)
    {
        Fire();
        mFired = true;
    }
    UpdateInvisibility(fDeltaTime);
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00685fd0
void WeaponInvisibility::ExitFire()
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    if (pClass->m_eInvisibilityTrigger == WeaponInvisibilityClass::TRIGGER_HOLD && m_bInvisibilityActive)
    {
        InputInvisibilityOff();
    }
    mFired = false;
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00686000
void WeaponInvisibility::EnterEmpty()
{
    StopInvisibility();
    Weapon::EnterEmpty();
}

// FUNCTION: SWBF2 0x00686010
bool WeaponInvisibility::UpdateEmpty(float fDeltaTime)
{
    return Weapon::UpdateEmpty(fDeltaTime);
}

// FUNCTION: SWBF2 0x00686030
void WeaponInvisibility::ExitEmpty()
{
}

// FUNCTION: SWBF2 0x00686040
bool WeaponInvisibility::CheckFire() const
{
    if (!mTrigger || CheckEmpty() || mFired)
    {
        return false;
    }
    return mTrigger->IsPressed();
}

// FUNCTION: SWBF2 0x00686080
bool WeaponInvisibility::CheckReload() const
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    if (!m_bInvisibilityActive && pClass->mInitialEnergyDrain > 0.0f && mOwner &&
        pClass->mInitialEnergyDrain > mOwner->GetEnergy())
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006860d0
bool WeaponInvisibility::UpdateReload(float fDeltaTime)
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    mStateTimer = mOwner->GetEnergy() / 100.0f;
    mStateLimit = pClass->mInitialEnergyDrain / 100.0f;
    UpdateInvisibility(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00686130
bool WeaponInvisibility::CheckEmpty() const
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    float fDrain = pClass->mEnergyDrain;
    EnergyBar* pEnergyBar = mOwner->GetEnergyBar();
    if (pEnergyBar && pEnergyBar->IsOverheating() && fDrain > 0.0f)
    {
        return true;
    }
    if (!m_bInvisibilityActive && mOwner && pClass->mInitialEnergyDrain > 0.0f &&
        pClass->mInitialEnergyDrain > mOwner->GetEnergy() && mState == IDLE)
    {
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006861c0
bool WeaponInvisibility::FireDone() const
{
    if (!mFired)
    {
        return false;
    }
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    if (pClass->m_eInvisibilityTrigger == WeaponInvisibilityClass::TRIGGER_HOLD && !CheckEmpty())
    {
        return mTrigger->IsRelease();
    }
    return true;
}

// FUNCTION: SWBF2 0x00686200
void WeaponInvisibility::Fire()
{
    InputInvisibilityOn();
    if (static_cast<const WeaponInvisibilityClass*>(mClass)->m_eInvisibilityTrigger ==
        WeaponInvisibilityClass::TRIGGER_PRESS)
    {
        InputInvisibilityOff();
    }
}

// FUNCTION: SWBF2 0x00686220
void WeaponInvisibility::InputInvisibilityOn()
{
    StartInvisibility();
}

// FUNCTION: SWBF2 0x00686230
void WeaponInvisibility::InputInvisibilityOff()
{
    m_fInvisibilityPersistTimer = static_cast<const WeaponInvisibilityClass*>(mClass)->m_fInvisibilityPersistTime;
    m_bInvisibilityOn = false;
}

// FUNCTION: SWBF2 0x00686250
void WeaponInvisibility::StartInvisibility()
{
    m_fInvisibilityPersistTimer = static_cast<const WeaponInvisibilityClass*>(mClass)->m_fInvisibilityPersistTime;
    m_bInvisibilityOn = true;
    m_bInvisibilityActive = true;
}

// FUNCTION: SWBF2 0x00686270
void WeaponInvisibility::StopInvisibility()
{
    m_bInvisibilityOn = false;
    m_bInvisibilityActive = false;
    m_fInvisibilityPersistTimer = 0.0f;
}

// FUNCTION: SWBF2 0x00686290
void WeaponInvisibility::StartFlicker()
{
    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    m_fInvisibilityFactor -= pClass->m_fFlickerAmplitude;
    if (m_fInvisibilityFactor < 0.0f)
    {
        m_fInvisibilityFactor = 0.0f;
    }
    m_fInvisibilityFlickerTimer = pClass->m_fFlickerTimeMin;
    if (pClass->m_fFlickerTimeMax > pClass->m_fFlickerTimeMin)
    {
        m_fInvisibilityFlickerTimer +=
            (pClass->m_fFlickerTimeMax - pClass->m_fFlickerTimeMin) * PblRandom::_Global.GetFloat();
    }
}

// FUNCTION: SWBF2 0x00686350
void WeaponInvisibility::UpdateInvisibility(float fDeltaTime)
{
    if (mOwner->mCharacter && mOwner->mCharacter->mCarriedFlagPtr)
    {
        StopInvisibility();
    }

    float fHealth = mOwner->GetGameObject()->mCurHealth;
    if (fHealth != mOwnerHealth)
    {
        mOwnerHealth = fHealth;
        m_fInvisibilityFactor = 0.0f;
    }

    const WeaponInvisibilityClass* pClass = static_cast<const WeaponInvisibilityClass*>(mClass);
    if (m_bInvisibilityActive)
    {
        if (!m_bInvisibilityOn)
        {
            m_fInvisibilityPersistTimer -= fDeltaTime;
            if (m_fInvisibilityPersistTimer < 0.0f)
            {
                StopInvisibility();
            }
        }
        if (pClass->m_eInvisibilityTrigger != WeaponInvisibilityClass::TRIGGER_PRESS)
        {
            float fDrain = pClass->mEnergyDrain * fDeltaTime;
            EnergyBar* pEnergyBar = mOwner->GetEnergyBar();
            if (pEnergyBar && !pEnergyBar->SpendEnergy(fDrain))
            {
                StopInvisibility();
            }
        }
    }

    if (!m_bInvisibilityActive)
    {
        if (m_fInvisibilityFactor > 0.0f)
        {
            float fDec = pClass->m_fInvisibilityDecRate * fDeltaTime;
            if (fDec >= m_fInvisibilityFactor)
            {
                m_fInvisibilityFactor = 0.0f;
            }
            else
            {
                m_fInvisibilityFactor -= fDec;
            }
        }
    }
    else
    {
        const PblVector3& vVelocity = mOwner->GetGameObject()->GetVelocity();
        float fSpeedSq = vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z;
        float fMinSpeedSq = pClass->m_fSpeedForInvisibilityMin * pClass->m_fSpeedForInvisibilityMin;
        float fTarget = pClass->m_fInvisibilityMax;
        if (fSpeedSq > fMinSpeedSq)
        {
            fTarget = pClass->m_fInvisibilityMin;
        }
        else
        {
            float fMaxSpeedSq = pClass->m_fSpeedForInvisibilityMax * pClass->m_fSpeedForInvisibilityMax;
            if (fSpeedSq > fMaxSpeedSq)
            {
                float fBlend = (fMinSpeedSq - fSpeedSq) / (fMinSpeedSq - fMaxSpeedSq);
                fBlend *= fBlend;
                fTarget = (1.0f - fBlend) * pClass->m_fInvisibilityMin + fTarget * fBlend;
            }
        }

        if (fTarget > m_fInvisibilityFactor)
        {
            float fInc = pClass->m_fInvisibilityIncRate * fDeltaTime;
            if (fInc >= fTarget - m_fInvisibilityFactor)
            {
                m_fInvisibilityFactor = fTarget;
            }
            else
            {
                m_fInvisibilityFactor += fInc;
            }
        }
        else if (m_fInvisibilityFactor > fTarget)
        {
            float fDec = pClass->m_fInvisibilityDecRate * fDeltaTime;
            if (fDec >= m_fInvisibilityFactor - fTarget)
            {
                m_fInvisibilityFactor = fTarget;
            }
            else
            {
                m_fInvisibilityFactor -= fDec;
            }
        }

        if (pClass->m_fFlickerAmplitude > 0.0f)
        {
            m_fInvisibilityFlickerTimer -= fDeltaTime;
            if (m_fInvisibilityFlickerTimer <= 0.0f)
            {
                StartFlicker();
            }
        }
    }

    if (mOwner->GetGameObject()->GetEntityType() == ENT_SOLDIER)
    {
        static_cast<EntitySoldier*>(mOwner->GetGameObject()->GetEntity())
            ->SetInvisibilityFactor(m_fInvisibilityFactor);
    }
}
