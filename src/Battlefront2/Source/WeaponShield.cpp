#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdio.h>

#include <d3dx9math.h>

#include "WeaponShield.h"
#include "CollisionBody.h"
#include "CollisionPrimitive.h"
#include "Controllable.h"
#include "EntityDroideka.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "NetGame.h"
#include "EnergyBar.h"
#include "Trigger.h"
#include "SoundFL/source/sndvoicevirtual.h"

namespace GameLoop { extern float sClientDeltaTime; }

class AmmoCounter
{
public:

    bool HasInfiniteAmmo() const { return mMaxClips == FLT_MAX; }
    void ExpendRounds(float fRounds);

    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
    unsigned int m_uiRefCount;
};

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01f9a914
// SYNTHETIC: SWBF2 0x004072a0
// `dynamic initializer for 'WeaponShield::rttiHashWeaponShield''
PblHash WeaponShield::rttiHashWeaponShield("WeaponShield");

// GLOBAL: SWBF2 0x01f9a918
// SYNTHETIC: SWBF2 0x004072b0
// `dynamic initializer for 'WeaponShieldClass::rttiHashWeaponShieldClass''
PblHash WeaponShieldClass::rttiHashWeaponShieldClass("WeaponShieldClass");
char WeaponShieldClass::rttiNameWeaponShieldClass[] = "WeaponShieldClass";

// GLOBAL: SWBF2 0x009cef54
PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x006920c0
WeaponShieldClass::WeaponShieldClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mLockOnAngle = 0.0f;
    mTargetSides = 0;
    mTargetTypes = 0;
    mAITargetTypes = 0;

    mMaxShield = 0.0f;
    mAddShield = 0.0f;
    mAddShieldOff = 0.0f;
    m_vShieldOffset = gVectorZero;
    m_vShieldSize = gVectorZero;
    mCollisionBody = 0;
    mShieldEffect = 0;
}

// FUNCTION: SWBF2 0x006921c0
WeaponShieldClass::WeaponShieldClass(WeaponShieldClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mMaxShield = pBase->mMaxShield;
    mAddShield = pBase->mAddShield;
    mAddShieldOff = pBase->mAddShieldOff;
    m_vShieldOffset = pBase->m_vShieldOffset;
    m_vShieldSize = pBase->m_vShieldSize;
    mCollisionBody = pBase->mCollisionBody;
    mCollisionModel = pBase->mCollisionModel;
    mShieldEffect = pBase->mShieldEffect;
    mSoundShield = pBase->mSoundShield;
    mSoundShieldOff = pBase->mSoundShieldOff;
}

// FUNCTION: SWBF2 0x006922f0
WeaponShieldClass::~WeaponShieldClass()
{
}

// SYNTHETIC: SWBF2 0x00692190
// WeaponShieldClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00691770
bool WeaponShieldClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponShieldClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006917a0
unsigned int WeaponShieldClass::GetDerivedRtti()
{
    return rttiHashWeaponShieldClass;
}

// FUNCTION: SWBF2 0x006917b0
const char* WeaponShieldClass::GetDerivedRttiName()
{
    return rttiNameWeaponShieldClass;
}

// FUNCTION: SWBF2 0x00692350
void WeaponShieldClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xda3491bc:
        sscanf(pszValue, "%f", &mMaxShield);
        break;
    case 0xbad94c15:
        sscanf(pszValue, "%f", &mAddShield);
        break;
    case 0x729dfa7a:
        sscanf(pszValue, "%f", &mAddShieldOff);
        break;
    case 0x1d726313:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        m_vShieldOffset.x = x;
        m_vShieldOffset.y = y;
        m_vShieldOffset.z = z;
        break;
    }
    case 0x5a9201b6:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        int iCount = sscanf(pszValue, "%f %f %f", &x, &y, &z);
        if (iCount < 3)
        {
            z = x;
        }
        if (iCount < 2)
        {
            y = x;
        }
        m_vShieldSize.x = x;
        m_vShieldSize.y = y;
        m_vShieldSize.z = z;
        break;
    }
    case 0xddb924a1:
        mShieldEffect = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
        break;
    case 0xd85503ff:
        mSoundShield.SetID(pszValue, false, false);
        break;
    case 0x5df6c1a4:
        mSoundShieldOff.SetID(pszValue, false, false);
        break;
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00692520
void WeaponShieldClass::PostReadInit()
{
    CollisionPrimitive primitive;
    primitive.SetBubble(m_vShieldSize);
    PblMatrix matrix = _IdentityMatrix;
    matrix.m[3][0] = m_vShieldOffset.x;
    matrix.m[3][1] = m_vShieldOffset.y;
    matrix.m[3][2] = m_vShieldOffset.z;
    matrix.m[3][3] = 1.0f;
    mCollisionBody = new CollisionBodyPrimitive(primitive, "shield", matrix, 0);
    mCollisionModel.AddBody(mCollisionBody);
}

// FUNCTION: SWBF2 0x00692640
WeaponClass* WeaponShieldClass::Derive(unsigned int uHash)
{
    return new WeaponShieldClass(this, uHash);
}

// FUNCTION: SWBF2 0x006917c0
WeaponShield::WeaponShield(const WeaponShieldClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc),
      CollisionObject(COLL_SHIELD, &pClass->mCollisionModel, 0, 0)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        ActivatePhysics();
    }

    if (pClass->m_vShieldSize.x > 0.0f)
    {
        float fRadius = pClass->m_vShieldSize.x;
        if (pClass->m_vShieldSize.y > fRadius)
        {
            fRadius = pClass->m_vShieldSize.y;
        }
        if (pClass->m_vShieldSize.z > fRadius)
        {
            fRadius = pClass->m_vShieldSize.z;
        }
        SetCollisionRadius(fRadius);
        Set2DCollisionRadius(fRadius);
    }

    mParent = mOwner->GetGameObject()->GetCollisionObject();
    mParent->mParent = this;
    mShieldEffect = 0;

    mOwner->GetGameObject()->SetMaxShield(static_cast<const WeaponShieldClass*>(mClass)->mMaxShield);
    mOwner->GetGameObject()->SetCurShield(
        m_pAmmoCounter->HasInfiniteAmmo() ? static_cast<const WeaponShieldClass*>(mClass)->mMaxShield : 0.0f,
        false, 0);
    mOwner->GetGameObject()->SetAddShield(static_cast<const WeaponShieldClass*>(mClass)->mAddShieldOff);
    mOwner->GetGameObject()->SetIsShielded(false);
}

// SYNTHETIC: SWBF2 0x00691990
// WeaponShield::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0069271d
// [thunk]:WeaponShield::`scalar deleting destructor' adjustor{272}

// FUNCTION: SWBF2 0x006919c0
WeaponShield::~WeaponShield()
{
    if (mShieldEffect)
    {
        mShieldEffect->StopAndFinish();
        mSoundControl.Stop(true);
    }
}

// FUNCTION: SWBF2 0x00691700
bool WeaponShield::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponShield || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00691730
unsigned int WeaponShield::GetDerivedRtti() const
{
    return rttiHashWeaponShield;
}

// FUNCTION: SWBF2 0x00691740
const char* WeaponShield::GetDerivedRttiName() const
{
    return "WeaponShield";
}

// FUNCTION: SWBF2 0x00691750
const CollisionObject* WeaponShield::GetCollisionObject() const
{
    return this ? static_cast<const CollisionObject*>(this) : 0;
}

// FUNCTION: SWBF2 0x00691760
CollisionObject* WeaponShield::GetCollisionObject()
{
    return this ? static_cast<CollisionObject*>(this) : 0;
}

// FUNCTION: SWBF2 0x00691a40
bool WeaponShield::IsBusy() const
{
    return (mState == FIRE || mState == FIRE2) &&
           mStateLimit > mStateTimer + GameLoop::sClientDeltaTime;
}

// FUNCTION: SWBF2 0x00691a80
bool WeaponShield::Update(float fDeltaTime)
{
    const WeaponShieldClass* pClass = static_cast<const WeaponShieldClass*>(mClass);
    float fCurShield = mOwner->GetGameObject()->GetCurShield();
    bool bShielded = mOwner->GetGameObject()->IsShielded();

    if (fCurShield > 0.0f && mOwner->GetGameObject()->IsAlive() &&
        !(mOwner->GetGameObject()->IsRtti(EntityDroideka::rttiHashEntityDroideka) &&
          reinterpret_cast<const EntityDroideka*>(mOwner->GetGameObject())->IsRollingOrTransition()))
    {
        if (m_pAmmoCounter->HasInfiniteAmmo())
        {
            if (mTrigger->IsPressed())
            {
                bShielded = !bShielded;
            }
        }
        else if (fCurShield > 0.0f)
        {
            bShielded = true;
        }
    }
    else
    {
        bShielded = false;
    }

    if (bShielded)
    {
        mOwner->GetGameObject()->SetAddShield(pClass->mAddShield);
        FLEffectClass* pEffectClass = pClass->mShieldEffect;
        if (!mShieldEffect && pEffectClass)
        {
            mSoundControl.Play(&pClass->mSoundShield,
                               &mOwner->GetGameObject()->GetMatrix().GetTranslation(),
                               &mOwner->GetGameObject()->GetVelocity(), true);
            PlayFireSound(false);
            mShieldEffect = pEffectClass->BuildEffect();
            if (mShieldEffect)
            {
                mShieldEffect->AttachEffectToObject(mOwner->GetGameObject(),
                                                    pClass->mCollisionBody->mMatrix, true);
                mShieldEffect->ActivateEffect();
            }
        }
        if (mSoundControl.GetPlaying())
        {
            mSoundControl.Update(&pClass->mSoundShield,
                                 &mOwner->GetGameObject()->GetMatrix().GetTranslation(),
                                 &mOwner->GetGameObject()->GetVelocity());
        }

        float fSpent = 1.0f - fCurShield / mOwner->GetGameObject()->GetMaxShield();
        if (Snd::VoiceVirtual* pVoice = mSoundControl.GetVoiceVirtual())
        {
            pVoice->SetGain(pClass->mSoundShield.GetGain() * (1.0f - fSpent * fSpent));
        }

        if (pClass->m_vShieldSize.x > 0.0f)
        {
            PblVector3 vCenter;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                                   (const D3DXVECTOR3*)&pClass->mCollisionBody->mMatrix.GetTranslation(),
                                   (const D3DXMATRIX*)&GetMatrix());
            SetCollisionCenter(vCenter);
            RecalculateAABB();
            if (!IsInCollisionList())
            {
                AddShieldToCollisionManager();
            }
        }
    }
    else
    {
        mOwner->GetGameObject()->SetAddShield(pClass->mAddShieldOff);
        if (mShieldEffect)
        {
            mShieldEffect->StopAndFinish();
            mShieldEffect = 0;
        }
        if (mSoundControl.GetPlaying())
        {
            mSoundControl.Stop(true);
            pClass->mSoundShieldOff.Play(&mOwner->GetGameObject()->GetMatrix().GetTranslation(),
                                         &mOwner->GetGameObject()->GetVelocity(), false, true);
        }
        if (IsInCollisionList())
        {
            RemoveFromCollisionManager();
        }
    }

    mOwner->GetGameObject()->SetIsShielded(bShielded);
    return Weapon::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00692020
bool WeaponShield::CheckFire() const
{
    if (!mTrigger || CheckOverheat() || m_pAmmoCounter->HasInfiniteAmmo())
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00691f00
bool WeaponShield::CollisionCallback(const CollisionResult* pResult,
                                     CollisionObject* pOther, Restrictor* pRestrictor)
{
    return mParent->CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x00691f10
void WeaponShield::EnterFire()
{
    Weapon::EnterFire();
    mStateTimer = 0.0f;
    mStateLimit = 0.0001f;
    if (mClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x00691f50
bool WeaponShield::UpdateFire(float fDeltaTime)
{
    if (mStateTimer == 0.0f)
    {
        GameObject* pObject = mOwner->GetGameObject();
        pObject->SetMaxShield(static_cast<const WeaponShieldClass*>(mClass)->mMaxShield);
        pObject->SetCurShield(static_cast<const WeaponShieldClass*>(mClass)->mMaxShield, false, 0);
        pObject->SetAddShield(static_cast<const WeaponShieldClass*>(mClass)->mAddShield);
        pObject->SetIsShielded(true);
        m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
        m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
    }
    mStateTimer += fDeltaTime;
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00692010
void WeaponShield::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00692060
bool WeaponShield::FireDone() const
{
    return mStateTimer >= mStateLimit &&
           (mClass->mTriggerSingle || !CheckFire() || CheckReload() || CheckEmpty());
}

// FUNCTION: SWBF2 0x006926b0
Weapon* WeaponShieldClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponShield(this, desc);
}

// FUNCTION: SWBF2 0x00691ee0
const PblVector3& WeaponShield::GetPosition() const
{
    return mParent->GetPosition();
}

// FUNCTION: SWBF2 0x00691ef0
const PblMatrix& WeaponShield::GetMatrix() const
{
    return mParent->GetMatrix();
}
