#include <float.h>
#include <math.h>
#include <stdio.h>

#include "Ordnance.h"
#include "BonusManager.h"
#include "Character.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "Explosion.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "NetGame.h"
#include "UnitController.h"
#include "VisionManager.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Application/RedWater.h"

class PlayerStats
{
public:
    void AddShot();
};

namespace
{

    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    Controllable* GetControllable(GameObject* pObject)
    {
        typedef Controllable*(__thiscall * GetControllableFn)(GameObject*);
        return pObject->GetControllable();
    }

    void SignalFire(Weapon* pWeapon)
    {
        typedef void(__thiscall * SignalFireFn)(Weapon*);
        pWeapon->SignalFire();
    }
}

// GLOBAL: SWBF2 0x01ef2578
// SYNTHETIC: SWBF2 0x004059f0
// `dynamic initializer for 'Ordnance::rttiHashOrdnance''
PblHash Ordnance::rttiHashOrdnance("Ordnance");
char Ordnance::rttiNameOrdnance[] = "Ordnance";

// GLOBAL: SWBF2 0x01ef2574
// SYNTHETIC: SWBF2 0x00405a00
// `dynamic initializer for 'OrdnanceClass::rttiHashOrdnanceClass''
PblHash OrdnanceClass::rttiHashOrdnanceClass("OrdnanceClass");
char OrdnanceClass::rttiNameOrdnanceClass[] = "OrdnanceClass";

// GLOBAL: SWBF2 0x01ef25c0
// SYNTHETIC: SWBF2 0x004059d0
// `dynamic initializer for 'Ordnance::sMemoryPool''
// SYNTHETIC: SWBF2 0x007691f0
// `dynamic atexit destructor for 'Ordnance::sMemoryPool''
MemoryPool Ordnance::sMemoryPool("Ordnance", 0x98);

// FUNCTION: SWBF2 0x005f7e70
OrdnanceClass::OrdnanceClass(unsigned int uHash)
    : Factory<Ordnance, OrdnanceClass, OrdnanceDesc>(uHash)
{
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    mDamage.mDamageOwner = owner;
    mDamage.mBaseValue = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mDamage.mHealthScale[i] = 1.0f;
    }
    mDamage.mShieldScale = 1.0f;

    mStrengthLimit = 0.0f;
    mNextCharge = this;
    mLifeSpan = FLT_MAX;
    mDamageTransitionDelay = 0.0f;
    mDamageTransitionPeriod = 0.0f;
    mDamageFinalDamage = -1.0f;
    mBonusAmplification = 0.0f;
    mBonusColor = RedColor::WHITE;
    mExplosionExpire = 0;
    mExplosionImpact = 0;
    mTrailEffect = 0;
    mImpactExplosionEffect = 0;
}

// FUNCTION: SWBF2 0x005f71a0
bool Ordnance::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnance;
}

// FUNCTION: SWBF2 0x005f71c0
unsigned int Ordnance::GetDerivedRtti() const
{
    return rttiHashOrdnance;
}

// FUNCTION: SWBF2 0x005f71d0
const char* Ordnance::GetDerivedRttiName() const
{
    return rttiNameOrdnance;
}

// FUNCTION: SWBF2 0x005f71f0
const PblVector3& Ordnance::GetPosition() const
{
    return mPosition;
}

// FUNCTION: SWBF2 0x005f7200
const PblVector3& Ordnance::GetOldPosition() const
{
    return mPosition;
}

// FUNCTION: SWBF2 0x005f7210
const PblVector3& Ordnance::GetVelocity() const
{
    static PblVector3 vZero(0.0f, 0.0f, 0.0f);
    return vZero;
}

// FUNCTION: SWBF2 0x005f7220
void Ordnance::KillOrdnance()
{
    if (this)
    {
        delete this;
    }
}

// FUNCTION: SWBF2 0x005f7230
void Ordnance::CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                              GameObject* pObject) const
{
}

// FUNCTION: SWBF2 0x005f7240
GameObject* Ordnance::GetOwner() const
{
    PblHandle<GameObject>& owner = const_cast<PblHandle<GameObject>&>(mOwner);
    if (owner.mObject && GameObjectHandleId(owner.mObject) != owner.mSavedHandleId)
    {
        owner.mObject = 0;
        owner.mSavedHandleId = 0;
    }
    return owner.mObject;
}

// SYNTHETIC: SWBF2 0x005f7270
// Ordnance::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f72a0
bool OrdnanceClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceClass;
}

// FUNCTION: SWBF2 0x005f72c0
unsigned int OrdnanceClass::GetDerivedRtti()
{
    return rttiHashOrdnanceClass;
}

// FUNCTION: SWBF2 0x005f72d0
const char* OrdnanceClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceClass;
}

// FUNCTION: SWBF2 0x005f72e0
float OrdnanceClass::GetVelocity()
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x005f72f0
float OrdnanceClass::GetGravity()
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x005f7300
float OrdnanceClass::GetRange()
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x005f7310
DamageDesc OrdnanceClass::GetDamage() const
{
    return mDamage;
}

// FUNCTION: SWBF2 0x005f7350
int CollisionManager::GetLastRayHitBodyId()
{
    return sInstance ? sInstance->mLastRayHitBodyId : -1;
}

// FUNCTION: SWBF2 0x005f7370
Ordnance::Ordnance(OrdnanceClass* pClass, const OrdnanceDesc& desc)
    : FoleyFXCollider(pClass),
      FoleyFXCollidee(pClass),
      mDamageOwner(0, 0, 0, 0, 0, 1.0f)
{
    mCheckStaticCollision = true;
    mDecrementLifeTime = true;
    mClass = pClass;
    while (desc.mStrength > mClass->mNextCharge->mStrengthLimit && mClass->mNextCharge != mClass)
    {
        mClass = mClass->mNextCharge;
    }

    const float fLow = mClass->mStrengthLimit;
    const float fHigh = mClass->mNextCharge->mStrengthLimit;
    if (fLow > desc.mStrength || fLow >= fHigh)
    {
        mRatio = 0.0f;
    }
    else
    {
        mRatio = (desc.mStrength - fLow) / (fHigh - fLow);
    }

    mPosition = desc.mPosition;
    mOwner.mObject = desc.mOwner;
    mOwner.mSavedHandleId = desc.mOwner ? GameObjectHandleId(desc.mOwner) : 0;
    mDamageOwner = desc.mDamageOwner;
    mLifeTime = (mClass->mNextCharge->mLifeSpan - mClass->mLifeSpan) * mRatio + mClass->mLifeSpan;
    mLifeElapsed = 0.0f;

    if (pClass->mTrailEffect &&
        (!NetGame::IsEnabled() || NetGame::IsClient() || (NetGame::IsOneWorld() && NetGame::IsHost())))
    {
        mTrailEmitterObj = pClass->mTrailEffect->BuildEffect();
    }
    else
    {
        mTrailEmitterObj = 0;
    }

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->AttachEffectToVector(&mPosition, true);
        mTrailEmitterObj->SetOrientation(desc.mDirection);
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        if (desc.mDamageOwner.mCharacter)
        {
            desc.mDamageOwner.mCharacter->GetPlayerStats().AddShot();
        }
    }

    mSoundOrdnance.Play(&mClass->mSoundOrdnance, 0, 0, true);
    if (desc.mFireWeapon)
    {
        SignalFire(desc.mFireWeapon);
    }

    mInterpIndex = -1;
    mRenderColor = RedColor::WHITE;
    if (GetOwner() && GetControllable(GetOwner()))
    {
        Character* pCharacter = GetControllable(GetOwner())->mCharacter;
        if (pCharacter &&
            BonusManager::sInstance.IsTeamBonusActive(pCharacter->mTeamNumber, 0xBB03442E))
        {
            mRenderColor = mClass->mBonusColor;
        }
    }
}

// FUNCTION: SWBF2 0x005f7790
Ordnance::~Ordnance()
{
    CollisionManager::RemoveOrdnance(this);
    DeactivateThread();
    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->DeactivateEffect();
    }
    mSoundOrdnance.Stop(true);
}

// SYNTHETIC: SWBF2 0x005f8ace
// [thunk]:Ordnance::`vector deleting destructor' adjustor{24}

// FUNCTION: SWBF2 0x005f7880
void Ordnance::AddOrdnanceToCollisionManager()
{
    CollisionManager::AddOrdnance(this);
}

// FUNCTION: SWBF2 0x005f7890
bool Ordnance::Collide(CollisionObject* pObject, const PblVector3& vPos,
                       const PblVector3& vNormal, int iBodyId)
{
    const CollisionObjectType eType = (CollisionObjectType)pObject->GetObjectData(0);
    if (eType == COLL_LEAFPATCH)
    {
        return false;
    }

    mPosition = vPos;

    if (mClass->mExplosionImpact)
    {
        PblVector3 vExplosionPos(mPosition.x + vNormal.x * 0.2f,
                                 mPosition.y + vNormal.y * 0.2f,
                                 mPosition.z + vNormal.z * 0.2f);
        mClass->mExplosionImpact->CreateExplosion(vExplosionPos, mDamageOwner, false, eType);
    }

    if (mClass->mImpactExplosionEffect)
    {
        FLEffectObject* pEffect = mClass->mImpactExplosionEffect->BuildEffect();
        if (pEffect)
        {
            PblVector3 vEffectPos(mPosition.x + vNormal.x * 0.2f,
                                  mPosition.y + vNormal.y * 0.2f,
                                  mPosition.z + vNormal.z * 0.2f);
            pEffect->AttachEffectToVector(&vEffectPos, false);
        }
    }

    GameObject* pOwner = GetOwner();
    if (GetOwner())
    {
        UnitController* pOwnerController = pOwner ? UnitControllerPtr(GetControllable(pOwner)) : 0;
        if (eType == COLL_SOFT && mLifeElapsed < 0.4f)
        {
            GameObject* pVictim = pObject->GetGameObject();
            UnitController* pVictimController = pVictim ? UnitControllerPtr(GetControllable(pVictim)) : 0;
            if (pOwner && pVictim)
            {
                if (pOwnerController)
                {
                    VisionManager::AddFakeRayhitResult(pOwnerController, pVictim);
                }
                if (pVictimController)
                {
                    VisionManager::AddFakeRayhitResult(pVictimController, pOwner);
                }
            }
        }
        if (pOwnerController)
        {
            const PblVector3& vVelocity = GetVelocity();
            const float fX = vVelocity.x * mLifeElapsed;
            const float fY = vVelocity.y * mLifeElapsed;
            const float fZ = vVelocity.z * mLifeElapsed;
            const float fDistSq = fY * fY + fX * fX + fZ * fZ;
            if (eType != COLL_SOFT && fDistSq > 0.0f)
            {
                pOwnerController->SetLastShotDist(fDistSq);
            }
        }
    }

    if (eType == COLL_SHIELD && mClass->mSoundCollisionShield.GetProperties())
    {
        mClass->mSoundCollisionShield.Play(&mPosition, 0, false, true);
    }
    else
    {
        if (eType == COLL_WATER)
        {
            mClass->mSoundCollisionWater.Play(&mPosition, 0, false, true);
        }
        float fWaterHeight;
        RedWater::GetWaterHeight(mPosition, fWaterHeight);
        if (mPosition.y > fWaterHeight)
        {
            mClass->mSoundCollision.Play(&mPosition, 0, false, true);
        }
    }

    CollisionCallback((CollisionObjectType)pObject->GetObjectData(0), pObject, &vPos);
    pObject->FoleyFXCollider::CollisionCallback(COLL_ORDNANCE, this, &vPos);
    return true;
}

// FUNCTION: SWBF2 0x005f7bd0
bool Ordnance::Update(float fDeltaTime)
{
    mSoundOrdnance.Update(&mClass->mSoundOrdnance, &mPosition, &GetVelocity());
    if (mDecrementLifeTime)
    {
        mLifeTime -= fDeltaTime;
        if (mLifeTime <= 0.0f)
        {
            if (mClass->mExplosionExpire)
            {
                mClass->mExplosionExpire->CreateExplosion(mPosition, mDamageOwner, false, COLL_UNKNOWN);
            }
            mSoundOrdnance.Stop(true);
            return false;
        }
    }
    mLifeElapsed += fDeltaTime;
    return mPosition.y >= -2000.0f;
}

// FUNCTION: SWBF2 0x005f7c70
DamageDesc Ordnance::GetDamage()
{
    return mClass->GetDamage(mDamageOwner, 0.0f, mRatio, mLifeElapsed);
}

// FUNCTION: SWBF2 0x005f7cb0
bool Ordnance::CheckCollision()
{
    const PblVector3 vStart = GetOldPosition();
    const PblVector3& vEnd = GetPosition();
    PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const float fDist = sqrtf(vDir.y * vDir.y + vDir.x * vDir.x + vDir.z * vDir.z);
    if (fDist > 0.0f)
    {
        const float fInv = 1.0f / fDist;
        vDir.x *= fInv;
        vDir.y *= fInv;
        vDir.z *= fInv;

        CollisionObject* pHitObject = 0;
        PblVector3 vNormal;
        const GameObject* pIgnore = GetOwner();
        const float fHit = CollisionManager::RayHit(vStart, vDir, fDist, pHitObject, &vNormal, &pIgnore, 1,
                                                    mCheckStaticCollision ? 0x13BE : 0x132E, true);
        if (fHit < 1.0f)
        {
            const float fAlong = fHit * fDist;
            PblVector3 vHit(vDir.x * fAlong + vStart.x, vDir.y * fAlong + vStart.y,
                            vDir.z * fAlong + vStart.z);
            return !Collide(pHitObject, vHit, vNormal, CollisionManager::GetLastRayHitBodyId());
        }
    }
    return true;
}

// SYNTHETIC: SWBF2 0x005f7fb0
// OrdnanceClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f7fe0
OrdnanceClass::OrdnanceClass(OrdnanceClass* pBase, unsigned int uHash)
    : Factory<Ordnance, OrdnanceClass, OrdnanceDesc>(pBase, uHash)
{
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    mDamage.mDamageOwner = owner;
    mDamage.mBaseValue = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mDamage.mHealthScale[i] = 1.0f;
    }
    mDamage.mShieldScale = 1.0f;

    mStrengthLimit = pBase->mStrengthLimit;
    mNextCharge = this;
    mLifeSpan = pBase->mLifeSpan;
    mDamageTransitionDelay = pBase->mDamageTransitionDelay;
    mDamageTransitionPeriod = pBase->mDamageTransitionPeriod;
    mDamageFinalDamage = pBase->mDamageFinalDamage;
    mBonusAmplification = pBase->mBonusAmplification;
    mBonusColor = pBase->mBonusColor;
    mExplosionExpire = pBase->mExplosionExpire;
    mExplosionImpact = pBase->mExplosionImpact;
    mDamage = pBase->mDamage;
    mTrailEffect = pBase->mTrailEffect;
    mImpactExplosionEffect = pBase->mImpactExplosionEffect;
    mSoundOrdnance = pBase->mSoundOrdnance;
    mSoundCollision = pBase->mSoundCollision;
}

// SYNTHETIC: SWBF2 0x005f8180
// OrdnanceClass::~OrdnanceClass

// FUNCTION: SWBF2 0x005f81c0
DamageDesc OrdnanceClass::GetDamage(const DamageOwner& owner, float fUnused, float fRatio,
                                    float fElapsed) const
{
    DamageDesc damage = GetDamage();
    damage.mDamageOwner = owner;

    if (fRatio > 0.0f)
    {
        const DamageDesc& next = mNextCharge->mDamage;
        damage.mBaseValue = (next.mBaseValue - damage.mBaseValue) * fRatio + damage.mBaseValue;
        for (int i = 0; i < 8; ++i)
        {
            damage.mHealthScale[i] = (next.mHealthScale[i] - damage.mHealthScale[i]) * fRatio + damage.mHealthScale[i];
        }
        damage.mShieldScale = (next.mShieldScale - damage.mShieldScale) * fRatio + damage.mShieldScale;
    }

    const float fTime = fElapsed - mDamageTransitionDelay;
    if (fTime > 0.0f)
    {
        float fT = 1.0f;
        if (mDamageTransitionPeriod > 0.0f)
        {
            const float fScaled = fTime / mDamageTransitionPeriod;
            if (fScaled < 0.0f)
            {
                fT = 0.0f;
            }
            else if (!(fScaled > 1.0f))
            {
                fT = fScaled;
            }
        }
        float fFinal = mDamageFinalDamage;
        if (fFinal < 0.0f)
        {
            fFinal = damage.mBaseValue;
        }
        damage.mBaseValue = (fFinal - damage.mBaseValue) * fT + damage.mBaseValue;
    }

    if (BonusManager::sInstance.IsTeamBonusActive(owner.mTeam, 0xBB03442E))
    {
        damage.mBaseValue = mBonusAmplification * damage.mBaseValue + damage.mBaseValue;
    }
    return damage;
}

// FUNCTION: SWBF2 0x005f83c0
void OrdnanceClass::Read(PblFileChunk& chunk)
{
    char acBuffer[128];
    OrdnanceClass* pClass = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x45505954:
        {
            const unsigned int uType = PblHash(child.ReadString(acBuffer, sizeof(acBuffer)));
            if (Find(uType))
            {
                return;
            }
            pClass = pClass->Derive(uType);
            break;
        }
        case 0x45534142:
            pClass = Find(PblHash(child.ReadString(acBuffer, sizeof(acBuffer))));
            break;
        case 0x504F5250:
        {
            const unsigned int uProperty = child._pFile->Read32();
            pClass->SetProperty(uProperty, child.ReadString(acBuffer, sizeof(acBuffer)));
            if (pClass != pClass->mNextCharge)
            {
                pClass = pClass->mNextCharge;
            }
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x005f8540
void OrdnanceClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x7C7C544B:
        sscanf(pszValue, "%f", &mLifeSpan);
        break;
    case 0x338A07CD:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[2]);
        break;
    case 0x21DE3FAB:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        RedColor color;
        color.b = (unsigned char)b;
        color.g = (unsigned char)g;
        color.r = (unsigned char)r;
        color.a = (unsigned char)a;
        mBonusColor = color;
        break;
    }
    case 0x02168F61:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        mDamage.mHealthScale[4] = fScale;
        mDamage.mHealthScale[5] = fScale;
        mDamage.mHealthScale[6] = fScale;
        break;
    }
    case 0x0BB94602:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[0]);
        break;
    case 0x02BB1FE0:
    case 0x61A8BB12:
    case 0x9B30E763:
        if (*pszValue)
        {
            mExplosionImpact = ExplosionClass::Find(PblHash(pszValue));
        }
        else
        {
            mExplosionImpact = 0;
        }
        break;
    case 0x2297E47D:
        mSoundCollisionWater.SetID(pszValue, false, false);
        break;
    case 0x2E51081F:
        if (*pszValue)
        {
            mExplosionExpire = ExplosionClass::Find(PblHash(pszValue));
        }
        else
        {
            mExplosionExpire = 0;
        }
        break;
    case 0x6CCDC5EF:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[3]);
        break;
    case 0x756BCB2D:
        mSoundCollisionShield.SetID(pszValue, false, false);
        break;
    case 0x47D0E794:
    case 0x59E94C40:
        sscanf(pszValue, "%f", &mDamage.mBaseValue);
        break;
    case 0xD0545C72:
        sscanf(pszValue, "%f", &mDamage.mShieldScale);
        break;
    case 0xC28F0C96:
        mSoundCollision.SetID(pszValue, false, false);
        break;
    case 0xC37970CF:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        mDamage.mHealthScale[0] = fScale;
        mDamage.mHealthScale[1] = fScale;
        mDamage.mHealthScale[2] = fScale;
        break;
    }
    case 0xCCF9BE5C:
        mSoundOrdnance.SetID(pszValue, false, false);
        break;
    case 0xD4E84271:
        if (*pszValue)
        {
            mImpactExplosionEffect = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
        }
        else
        {
            mImpactExplosionEffect = 0;
        }
        break;
    case 0xEE08D1BA:
        sscanf(pszValue, "%f", &mDamageTransitionPeriod);
        break;
    case 0xA74561B2:
        sscanf(pszValue, "%f", &mDamageTransitionDelay);
        break;
    case 0x9AA17A49:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[1]);
        break;
    case 0xA0BB60CE:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        mDamage.mHealthScale[3] = fScale;
        mDamage.mHealthScale[4] = fScale;
        mDamage.mHealthScale[5] = fScale;
        mDamage.mHealthScale[6] = fScale;
        break;
    }
    case 0x97481656:
        sscanf(pszValue, "%f", &mBonusAmplification);
        break;
    case 0x7E71E2C6:
        if (*pszValue)
        {
            mTrailEffect = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
        }
        else
        {
            mTrailEffect = 0;
        }
        break;
    case 0x8316F444:
        mNextCharge = Derive(mId);
        if (mNextCharge)
        {
            sscanf(pszValue, "%f", &mNextCharge->mStrengthLimit);
        }
        break;
    case 0x955F71E9:
        sscanf(pszValue, "%f", &mDamageFinalDamage);
        break;
    default:
        if (!FoleyFXColliderClass::SetProperty(uProperty, pszValue))
        {
            FoleyFXCollideeClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x005f89c0
template Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Factory(OrdnanceClass*, unsigned int);

// SYNTHETIC: SWBF2 0x005f8a80
// Factory<Ordnance,OrdnanceClass,OrdnanceDesc>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005fa200
// OrdnanceBeamClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005fa2d0
// OrdnanceBeamClass::~OrdnanceBeamClass
