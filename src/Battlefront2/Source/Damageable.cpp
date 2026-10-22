#include <float.h>
#include <string.h>

#include "Damageable.h"
#include "AIDifficulty.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Controller.h"
#include "EventInfo.h"
#include "EventManager_Object.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "MedalsMgr.h"
#include "NetGame.h"
#include "NetComm.h"
#include "NetSetup.h"
#include "ParticleEmitterObject.h"
#include "PlayerStats.h"
#include "PlayerProfile.h"
#include "ScreenFilterEffect.h"
#include "Team.h"
#include "UnitController.h"
#include "Weapon.h"

#include "ParticleEmitter.h"
#include "PebbleFL/Common/PblHandle.h"
#include "Adrenaline.h"

// FUNCTION: SWBF2 0x004884f0
void Damageable::Kill()
{
}

// FUNCTION: SWBF2 0x00488500
void Damageable::Respawn()
{
}

// FUNCTION: SWBF2 0x00488510
GameObject* Damageable::GetGameObject()
{
    return 0;
}

// FUNCTION: SWBF2 0x00488520
const GameObject* Damageable::GetGameObject() const
{
    return 0;
}

extern bool netTeamDamage;

// FUNCTION: SWBF2 0x00488570
DamageOwner::DamageOwner(Character* pCharacter, GameObject* pGameObject, int iTeam,
                         EntityClass* pClass, WeaponClass* pWeapon, float fDamageMultiplier)
{
    mCharacter = pCharacter;
    mGameObject.mObject = pGameObject;
    mGameObject.mSavedHandleId =
        pGameObject
            ? pGameObject->mHandleId
            : 0;
    mTeam = iTeam;
    mClass = pClass;
    mWeapon = pWeapon;
    mDamageMultiplier = fDamageMultiplier;
}

// FUNCTION: SWBF2 0x004885c0
Damageable::Damageable()
    : mCurHealth(FLT_MAX),
      mMaxHealth(FLT_MAX),
      mAddHealth(0.0f),
      mCurShield(0.0f),
      mMaxShield(0.0f),
      mAddShield(0.0f),
      mDisableTime(0.0f),
      mAIDamageThreshold(0.0f),
      mDamageOwner(0, 0, 0, 0, 0, 1.0f)
{
    mDamageRegionTime = 0.0f;
    mHealthType = 0;
    mIsAlive = true;
    mVanishing = false;
    mIsShielded = true;
    memset(mDamageEffect, 0, sizeof(mDamageEffect));
}

// FUNCTION: SWBF2 0x004886e0
Damageable::~Damageable()
{
    for (int i = 0; i < 10; ++i)
    {
        if (!(mDamageEffect[i] == 0))
        {
            mDamageEffect[i]->DeactivateEffect();
            mDamageEffect[i] = 0;
            mDamageEffectSound[i].Stop(true);
        }
    }
}

// SYNTHETIC: SWBF2 0x004886b0
// Damageable::`scalar deleting destructor'

// GLOBAL: SWBF2 0x007e6328
float Damageable::sSameTeamDamageScale = 1.0f;

// FUNCTION: SWBF2 0x004887b0
void Damageable::StaticInit()
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
    if (pProfile && !NetGame::IsEnabled())
    {
        SetTeamDamageScale(pProfile->mGameOptions.mFriendlyFire / 100.0f);
    }
    else
    {
        SetTeamDamageScale(netTeamDamage ? 1.0f : 0.0f);
    }
}

// FUNCTION: SWBF2 0x00488810
void Damageable::SetTeamDamageScale(float fScale)
{
    sSameTeamDamageScale = fScale;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

extern PblVector3 gVectorZero;
class PblMatrix;
extern PblMatrix gMatrixIdentity;
extern int gSpawnInvicibility;

// GLOBAL: SWBF2 0x01e3075d
bool gInvincibleAll;

namespace EventManager
{
template <>
void Event<GameObject, float>::Callback::Trigger(GameObject& item, float& param);
template <>
void Event<GameObject, DamageOwner>::Callback::Trigger(GameObject& item, DamageOwner& param);
}

static PblList<Controllable>& GameObject_GetControllableList(GameObject* pObject)
{
    return pObject->mControllableList;
}

static int GameObject_GetTeam(const GameObject* pObject)
{
    return pObject->GetTeam();
}

static int Damageable_GetTeam(const Damageable* pThis)
{
    return static_cast<const GameObject*>(pThis)->GetTeam();
}

static GameObjectClass* GameObject_GetClass(GameObject* pObject)
{
    return pObject->GetGameObjectClass();
}

static Damageable* GameObject_GetDamageable(GameObject* pObject)
{
    return static_cast<Damageable*>(pObject);
}

static CollisionObject* GameObject_GetCollisionObject(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject);
}

static GameObject* GetHandleObject(const PblHandle<GameObject>& handle)
{
    PblHandle<GameObject>& h = const_cast<PblHandle<GameObject>&>(handle);
    if (h.mObject &&
        h.mObject->mHandleId !=
            h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

static inline void FlashLocalPlayers(Damageable* pThis, GameObject* pObject, float fHealthLost)
{
    PblList<Controllable>& list = GameObject_GetControllableList(pObject);
    for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
    {
        if (it->mPlayerId < 0)
        {
            continue;
        }
        int iViewport = NetGame::GetJoystickIndex(it->mPlayerId);
        if (iViewport >= 0 && pThis->mIsAlive)
        {
            float fAmount;
            if (pThis->mCurHealth > 0.0f)
            {
                fAmount = fHealthLost / (pThis->mCurHealth * 0.2f);
                if (fAmount > 1.0f)
                {
                    fAmount = 1.0f;
                }
            }
            else
            {
                fAmount = 1.0f;
            }
            RedColor color = RedColor::RED;
            color.a = (unsigned char)(int)(fAmount * 96.0f + 32.0f);
            float fTime = fAmount * 0.15f + 0.15f;
            ScreenFilterEffect::Instance()->TimedFilter(iViewport, fTime, 0.0f, fTime, color,
                                                        0xDE020766);
        }
    }
}

// FUNCTION: SWBF2 0x00438a40
DamageDesc::DamageDesc(DamageOwner owner, float fBaseValue)
    : mDamageOwner(owner), mBaseValue(fBaseValue)
{
    for (int i = 0; i < 8; ++i)
    {
        mHealthScale[i] = 1.0f;
    }
    mShieldScale = 1.0f;
}

// FUNCTION: SWBF2 0x00488820
void Damageable::SetCurHealth(float fHealth, bool bNotify, DamageOwner* pOwner)
{
    if (NetGame::IsEnabled() && NetGame::IsClient() && fHealth <= 0.0f)
    {
        fHealth = 0.0001f;
    }
    float fOld = mCurHealth;
    if (fOld != fHealth)
    {
        if (pOwner)
        {
            mDamageOwner = *pOwner;
        }
        mCurHealth = fHealth;
        if (bNotify)
        {
            HealthChangeCallback(false, fOld - fHealth);
        }
        else
        {
            Damageable::HealthChangeCallback(false, 0.0f);
        }
    }
}

// FUNCTION: SWBF2 0x004888e0
void Damageable::SetCurShield(float fShield, bool, DamageOwner*)
{
    float fOld = mCurShield;
    if (fOld != fShield)
    {
        mCurShield = fShield;
        ShieldChangeCallback(fOld - fShield);
    }
}

// FUNCTION: SWBF2 0x00488910
void Damageable::UpdateStatus(float fDeltaTime)
{
    if (mAddHealth != 0.0f && mMaxHealth > mCurHealth)
    {
        float fHealth = mCurHealth + mAddHealth * fDeltaTime;
        SetCurHealth(mMaxHealth > fHealth ? fHealth : mMaxHealth, true, 0);
    }
    if (mAddShield != 0.0f && mMaxShield > mCurShield)
    {
        float fShield = mCurShield + mAddShield * fDeltaTime;
        float fNew = mMaxShield;
        if (0.0f > fShield)
        {
            fNew = 0.0f;
        }
        else if (!(fShield > fNew))
        {
            fNew = fShield;
        }
        SetCurShield(fNew, true, 0);
    }
    float fTime = mDisableTime - fDeltaTime;
    mDisableTime = fTime > 0.0f ? fTime : 0.0f;
}

// FUNCTION: SWBF2 0x004889d0
void Damageable::InstantDeath(const DamageOwner& owner, bool bCheckProtection)
{
    if (mIsAlive)
    {
        mDamageOwner = owner;
    }
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (bCheckProtection)
    {
        if (gInvincibleAll)
        {
            return;
        }
        if (GameLoop::gInvincible && !NetGame::IsEnabled())
        {
            PblList<Controllable>& list = GameObject_GetControllableList(GetGameObject());
            for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
            {
                if (it->mPlayerId >= 0)
                {
                    return;
                }
            }
        }
    }
    if (mCurHealth > 0.0f)
    {
        GameObject* pObject = GetGameObject();
        if (pObject)
        {
            pObject->PlayDeathSound();
        }
        mCurHealth = 0.0f;
    }
    HealthChangeCallback(true, mMaxHealth);
}

// FUNCTION: SWBF2 0x00488af0
void Damageable::ApplyNetClientDamage(const DamageDesc& damage, unsigned int uFlags,
                                      float fMultiplier, bool, const PblVector3&)
{
    GameObject* pObject = GetGameObject();
    float fDamage = damage.mBaseValue * fMultiplier * damage.mDamageOwner.mDamageMultiplier;
    if (damage.mDamageOwner.mTeam == Damageable_GetTeam(this))
    {
        Character* pAttacker = damage.mDamageOwner.mCharacter;
        if (pAttacker && pAttacker->mPlayerId < 0 && pObject->GetControllable() &&
            pObject->GetControllable()->mPlayerId >= 0)
        {
            return;
        }
        fDamage = sSameTeamDamageScale * fDamage;
    }
    if ((uFlags & 2) && mMaxHealth < FLT_MAX)
    {
        float fHealthLost = damage.mHealthScale[mHealthType] * fDamage;
        if (fHealthLost > 0.0f)
        {
            bool bBigHit = fHealthLost > mCurHealth * 0.02f;
            if (bBigHit)
            {
                FlashLocalPlayers(this, pObject, fHealthLost);
            }
            if (mIsAlive)
            {
                if (bBigHit && (uFlags & 4))
                {
                    pObject->PlayHurtSound(damage.mDamageOwner.mCharacter != 0);
                }
            }
            else
            {
                pObject->PlayDeathSound();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00488d20
void Damageable::ApplyDamageCommon(const DamageDesc& damage, unsigned int uFlags,
                                   float fMultiplier, bool bIsShot, const PblVector3& vPos)
{
    GameObject* pObject = GetGameObject();
    if (GameLoop::IsGameOver())
    {
        return;
    }
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        ApplyNetClientDamage(damage, uFlags, fMultiplier, bIsShot, vPos);
        return;
    }

    bool bWasAlive = mIsAlive;
    if (damage.mDamageOwner.mCharacter && mMaxHealth < FLT_MAX)
    {
        Character::GetPlayerStats(damage.mDamageOwner.mCharacter).AddHit();
    }
    if (gInvincibleAll)
    {
        return;
    }
    if (GameLoop::gInvincible && !NetGame::IsEnabled())
    {
        PblList<Controllable>& list = GameObject_GetControllableList(pObject);
        for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
        {
            if (it->mPlayerId >= 0)
            {
                return;
            }
        }
    }
    if (NetSetup::InNetGame() && pObject->GetControllable() &&
        pObject->GetControllable()->mPlayerId >= 0)
    {
        Character* pCharacter = pObject->GetControllable()->mCharacter;
        if (pCharacter && (int)(NetComm::GetTime() - pCharacter->GetSpawnTime()) < gSpawnInvicibility)
        {
            return;
        }
    }

    if (bWasAlive)
    {
        mDamageOwner = damage.mDamageOwner;
    }

    float fDamage = damage.mBaseValue * fMultiplier * damage.mDamageOwner.mDamageMultiplier;
    if (Team::sTeams[Damageable_GetTeam(this)]->mAffiliation[damage.mDamageOwner.mTeam] > 0)
    {
        Character* pAttacker = damage.mDamageOwner.mCharacter;
        if (pAttacker && pAttacker->mPlayerId < 0 && pObject->GetControllable() &&
            pObject->GetControllable()->mPlayerId >= 0)
        {
            return;
        }
        fDamage = sSameTeamDamageScale * fDamage;
    }

    bool bDamaged = false;
    float fTotalLost = 0.0f;
    if ((uFlags & 1) && mCurShield > 0.0f && mIsShielded)
    {
        float fShieldLost = damage.mShieldScale * fDamage;
        fTotalLost = fShieldLost + 0.0f;
        if (fShieldLost > 0.0f)
        {
            bDamaged = true;
            if (fShieldLost > mCurShield)
            {
                fShieldLost = mCurShield;
                fDamage -= mCurShield / damage.mShieldScale;
            }
            else
            {
                fDamage = 0.0f;
            }
            float fShield = mCurShield - fShieldLost;
            float fFloor = mAIDamageThreshold * mMaxShield;
            mCurShield = fShield;
            Character* pAttacker = damage.mDamageOwner.mCharacter;
            if ((!pAttacker || pAttacker->mPlayerId < 0) && fFloor > 0.0f && fFloor > fShield)
            {
                mCurShield = (fShield + fShieldLost < fFloor) ? fShield + fShieldLost : fShieldLost;
            }
            ShieldChangeCallback(fShieldLost);
            SpreadDamageEvent(damage, bIsShot, vPos, 0.0f, fShieldLost);
        }
    }

    if ((uFlags & 2) && mMaxHealth < FLT_MAX)
    {
        float fHealthLost = damage.mHealthScale[mHealthType] * fDamage;
        fTotalLost = fHealthLost + fTotalLost;
        if (fHealthLost > 0.0f)
        {
            float fHealth = mCurHealth - fHealthLost;
            bDamaged = true;
            float fFloor = mAIDamageThreshold * mMaxHealth;
            mCurHealth = fHealth;
            Character* pAttacker = damage.mDamageOwner.mCharacter;
            if ((!pAttacker || pAttacker->mPlayerId < 0) && fFloor > 0.0f && fFloor > fHealth)
            {
                mCurHealth = (fHealth + fHealthLost < fFloor) ? fHealth + fHealthLost : fFloor;
            }
            HealthChangeCallback(false, fHealthLost);
            SpreadDamageEvent(damage, bIsShot, vPos, fHealthLost, 0.0f);

            bool bBigHit = fHealthLost > mCurHealth * 0.02f;
            if (bBigHit)
            {
                FlashLocalPlayers(this, pObject, fHealthLost);
            }
            if (mIsAlive)
            {
                if (bBigHit && (uFlags & 4))
                {
                    pObject->PlayHurtSound(damage.mDamageOwner.mCharacter != 0);
                }
            }
            else
            {
                pObject->PlayDeathSound();
            }
        }
    }

    if (damage.mDamageOwner.mCharacter &&
        GameObject_GetClass(GetGameObject())->GetVehicleType() != 9 &&
        GameObject_GetTeam(GetGameObject()) != damage.mDamageOwner.mTeam)
    {
        Character* pAttacker = damage.mDamageOwner.mCharacter;
        if (GetGameObject() == pAttacker->GetDamagedVehicle())
        {
            pAttacker->AddTotalVehicleDamage(fTotalLost);
        }
        else
        {
            pAttacker->SetDamagedVehicle(GetGameObject());
        }
    }

    if (bWasAlive && bDamaged)
    {
        if (mDamageOwner.mCharacter)
        {
            mDamageOwner.mCharacter->RegisterHit(GameObject_GetTeam(pObject), fMultiplier);
        }
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            EventManager::ObjectEvent::mDamage.Trigger(*pObject, mDamageOwner);
        }
    }
}

// FUNCTION: SWBF2 0x00489340
bool Damageable::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir,
                             const PblVector3& vPos, int iBodyPart, unsigned int uFlags)
{
    float fMultiplier = GetDamageMultiplier(iBodyPart);
    if (fMultiplier > 1.0f && AIDifficulty::GetProfileDifficulty() < 3)
    {
        Character* pAttacker = damage.mDamageOwner.mCharacter;
        if (!pAttacker || pAttacker->mPlayerId < 0)
        {
            fMultiplier = 1.0f;
        }
    }
    GameObject* pAttackerObject = GetHandleObject(damage.mDamageOwner.mGameObject);
    bool bWasAlive = mIsAlive;
    if (fMultiplier > 1.0f && pAttackerObject &&
        pAttackerObject->GetAffiliationToMe(GetGameObject()) < 0 &&
        damage.mHealthScale[mHealthType] >= 1.0f)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) && damage.mDamageOwner.mCharacter)
        {
            Character::GetPlayerStats(damage.mDamageOwner.mCharacter).AddCriticalShot();
        }
        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(0xD7A77563);
        if (pClass)
        {
            FLEffectObject* pEffect = pClass->BuildEffect();
            if (pEffect)
            {
                pEffect->AttachEffectToVector(&vPos, false);
            }
        }
    }

    PblVector3 vFlat;
    vFlat.x = vDir.x;
    vFlat.y = 0.0001f;
    vFlat.z = vDir.z;
    PblVector3 vNormal;
    D3DXVec3Normalize(&vNormal, &vFlat);
    vFlat = vNormal;
    const PblMatrix& mMatrix = GameObject_GetCollisionObject(GetGameObject())->GetMatrix();
    PblVector3 vHit;
    vHit.x = mMatrix.m[3][0] - vFlat.x * 30.0f;
    vHit.y = mMatrix.m[3][1] - vFlat.y * 30.0f;
    vHit.z = mMatrix.m[3][2] - vFlat.z * 30.0f;
    ApplyDamageCommon(damage, uFlags, fMultiplier, true, vHit);

    bool bAlive = mIsAlive;
    if (bWasAlive && damage.mDamageOwner.mWeapon && damage.mDamageOwner.mCharacter &&
        pAttackerObject && pAttackerObject->GetAffiliationToMe(GetGameObject()) < 0)
    {
        short sMedalsType = damage.mDamageOwner.mWeapon->mScoreForMedalsType;
        MedalsMgr* pMedals = damage.mDamageOwner.mCharacter->mMedalsMgr;
        if (sMedalsType >= 0 && pMedals)
        {
            pMedals->AddMedalPoints(sMedalsType, !bAlive, fMultiplier > 1.0f, (HealthType)mHealthType, false,
                                    false, 1);
        }
    }
    return bAlive;
}

// FUNCTION: SWBF2 0x004895e0
bool Damageable::ApplyDamage(const DamageDesc& damage, unsigned int uFlags)
{
    float fMultiplier = GetDamageMultiplier(-1);
    ApplyDamageCommon(damage, uFlags, fMultiplier, false, gVectorZero);
    return mIsAlive;
}

// FUNCTION: SWBF2 0x00489620
void Damageable::SpreadDamageEvent(const DamageDesc& damage, bool bIsShot, const PblVector3& vPos,
                                   float fHealthLost, float)
{
    if (!damage.mDamageOwner.mCharacter)
    {
        return;
    }
    GameObject* pObject = GetGameObject();
    float fShieldHit = damage.mShieldScale * damage.mBaseValue;
    float fHealthHit =
        damage.mHealthScale[GameObject_GetDamageable(pObject)->mHealthType] * damage.mBaseValue;
    if (!(fShieldHit > 1.0f) && !(fHealthHit > 1.0f))
    {
        return;
    }

    if (GetGameObject()->GetControllable() &&
        GetGameObject()->GetControllable()->mCharacter)
    {
        float fMaxHealth = mMaxHealth;
        GetGameObject()->GetControllable()->mCharacter->GetAdrenaline().GetHit(fHealthHit / fMaxHealth);
    }
    damage.mDamageOwner.mCharacter->GetAdrenaline().AddHit(fHealthHit / mMaxHealth);

    if (pObject->GetControllable() && pObject->GetControllable()->mCharacter &&
        pObject->GetControllable()->mCharacter->mVehicle)
    {
        pObject = pObject->GetControllable()->mCharacter->mVehicle->GetGameObject();
    }

    PblList<Controllable>& list = GameObject_GetControllableList(pObject);
    for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
    {
        Controller* pController = it->mCtrl;
        if (pController && pController->IsRtti(UnitController::GetRtti()))
        {
            UnitController* pUnit = static_cast<UnitController*>(pController);
            AI::EventInfo_Damaged info(damage.mDamageOwner.mCharacter, bIsShot, vPos, fHealthLost, false);
            pUnit->SendEvent(&info);
        }
    }
}

// FUNCTION: SWBF2 0x004897d0
void Damageable::HealthChangeCallback(bool, float)
{
    if (!(0.0f < mCurHealth))
    {
        mIsAlive = false;
    }
    GameObject* pObject = GetGameObject();
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        EventManager::ObjectEvent::mHealthChange.Trigger(*pObject, mCurHealth);
    }
    if (!GetGameObject())
    {
        return;
    }
    GameObjectClass* pClass = GameObject_GetClass(GetGameObject());
    if (!pClass)
    {
        return;
    }

    float fRatio = 0.0f;
    if (mIsAlive && mMaxHealth > 0.0f)
    {
        fRatio = mCurHealth / mMaxHealth;
    }
    bool bUnbuilt = GetGameObject()->IsUnbuilt();

    if (0.0001f > mCurHealth || !mIsAlive)
    {
        for (int i = 0; i < 10; ++i)
        {
            if (!(mDamageEffect[i] == 0))
            {
                mDamageEffect[i]->DeactivateEffect();
                mDamageEffect[i] = 0;
                mDamageEffectSound[i].Stop(true);
            }
        }
        return;
    }

    for (int i = 0; i < 10; ++i)
    {
        const DamageEffectClass* pStage = pClass->GetDamageEffectClass(i);
        if (!pStage)
        {
            return;
        }
        bool bShow = false;
        if (!bUnbuilt && pStage->mPercent + 0.0001f > fRatio &&
            (!(0.0f < pStage->mPercentStop) || !(fRatio < pStage->mPercentStop - 0.0001f)))
        {
            bShow = true;
        }

        if (!(mDamageEffect[i] == 0))
        {
            GameObject* pOwner = GetGameObject();
            mDamageEffectSound[i].Update(&pStage->mSound,
                                         reinterpret_cast<const PblVector3*>(&mDamageEffect[i]->m_mWorldMatrix.m[3][0]),
                                         &pOwner->GetVelocity());
            if (!bShow)
            {
                mDamageEffect[i]->DeactivateEffect();
                mDamageEffect[i] = 0;
                mDamageEffectSound[i].Stop(true);
            }
        }
        else if (bShow)
        {
            FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(pStage->mNameHash);
            if (!pEffectClass)
            {
                continue;
            }
            mDamageEffect[i] = pEffectClass->BuildEffect();
            if (mDamageEffect[i] == 0)
            {
                continue;
            }
            if (pEffectClass->mType == 1)
            {
                ParticleEmitterObject* pEmitter = static_cast<ParticleEmitterObject*>(static_cast<FLEffectObject*>(mDamageEffect[i]));
                pEmitter->ScaleMultiply(pStage->mScale);
                pEmitter->SetInheritVelocity(pStage->mInheritVelocity);
            }
            PblMatrix mOffset = gMatrixIdentity;
            mOffset.m[3][0] = pStage->mAttachOffset.x;
            mOffset.m[3][1] = pStage->mAttachOffset.y;
            mOffset.m[3][2] = pStage->mAttachOffset.z;
            mOffset.m[3][3] = 1.0f;
            const EntityGeometry* pGeometry =
                reinterpret_cast<const EntityGeometry*>(GetGameObject()->GetEntity());
            if (pStage->mHardPoint == 0)
            {
                mDamageEffect[i]->AttachEffectToObject(pGeometry, mOffset, true);
            }
            else
            {
                mDamageEffect[i]->AttachEffectToObject(pGeometry, pStage->mHardPoint, mOffset, true);
            }
            GameObject* pOwner = GetGameObject();
            mDamageEffectSound[i].Play(&pStage->mSound,
                                       reinterpret_cast<const PblVector3*>(&mDamageEffect[i]->m_mWorldMatrix.m[3][0]),
                                       &pOwner->GetVelocity(), true);
        }
    }
}

// FUNCTION: SWBF2 0x00489bb0
void Damageable::ShieldChangeCallback(float)
{
    GameObject* pObject = GetGameObject();
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        EventManager::ObjectEvent::mShieldChange.Trigger(*pObject, mCurShield);
    }
}

// FUNCTION: SWBF2 0x00489c00
void Damageable::SetDamageRegionTime()
{
    mDamageRegionTime = GameLoop::GetMissionTime();
}

namespace EventManager
{

// FUNCTION: SWBF2 0x00489c50
template <>
void Event<GameObject, DamageOwner>::Trigger(GameObject& object, DamageOwner& owner)
{
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        mRoot.Trigger(object, owner);
    }
}

// FUNCTION: SWBF2 0x00489c90
template <>
void Event<GameObject, DamageOwner>::Callback::Trigger(GameObject& item, DamageOwner& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}

// FUNCTION: SWBF2 0x00489db0
template <>
void Event<GameObject, float>::Callback::Trigger(GameObject& item, float& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

// FUNCTION: SWBF2 0x00488530
DamageOwner& DamageOwner::operator=(const DamageOwner& other)
{
    mCharacter = other.mCharacter;
    mGameObject = other.mGameObject;
    mTeam = other.mTeam;
    mClass = other.mClass;
    mWeapon = other.mWeapon;
    mDamageMultiplier = other.mDamageMultiplier;
    return *this;
}

// TEMPLATE: SWBF2 0x004479e0
// PblHandle<FLEffectObject>::operator=

// TEMPLATE: SWBF2 0x00447a40
// PblHandle<FLEffectObject>::operator->

// TEMPLATE: SWBF2 0x00447a60
// PblHandle<FLEffectObject>::operator FLEffectObject *

// GLOBAL: SWBF2 0x007e6324
int gSpawnInvicibility = 2;
