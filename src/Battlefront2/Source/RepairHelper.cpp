#include <float.h>
#include <math.h>

#include "RepairHelper.h"

#include "AILowLevel.h"
#include "AIUtil.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Entity.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "PowerupItem.h"
#include "Team.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "WeaponDispenser.h"
#include "WeaponRepair.h"
#include "PebbleFL/Common/PblMatrix.h"

namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};

enum VOType
{
    VOT_RepairStart = 16,
    VOT_RepairEnd = 17,
    VOT_GivePowerup = 18
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}
}

namespace TeamManager
{

int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);
}

namespace AIUtil
{

bool DirectLineToTarget(const GameObject* pObj, const PblVector3& vPos);
void Slide(GameObject* pMe, const GameObject* pTarget, bool bUseAttackHelper,
           bool* pbIsPathing, float* pfPathTime, float fMinDist, float fOffset);
}

namespace CombatUtil
{

float GetWeaponMaxRange(const Controllable* pMe);

bool IsTargetLinedUpObj(const Controllable* pMe, const GameObject* pTarget);
}

namespace ReserveManager
{

int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);

void Reserve(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
}

class AmmoCounter
{
public:
    float GetMaxClips() const { return mMaxClips; }
    float GetNumClips() const { return mNumClips; }
    int GetNumRounds() const;

    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
};

// GLOBAL: SWBF2 0x007ec54c

int gOutOfAmmoWeaponCount;

// GLOBAL: SWBF2 0x007ec550

int gRepairSearchTimeoutCount;

namespace
{

    const CollisionObject* AsCollisionObject(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj);
    }

    HealthType GetHealthType(const GameObject* pObj)
    {
        return (HealthType)pObj->mHealthType;
    }

    float GetCurHealth(const GameObject* pObj)
    {
        return pObj->mCurHealth;
    }

    float GetMaxHealth(const GameObject* pObj)
    {
        return pObj->mMaxHealth;
    }

    const CollisionObject* GetCollisionObject(const GameObject* pObj)
    {
        return pObj->GetCollisionObject();
    }

    const Controllable* GetControllable(const GameObject* pObj)
    {
        return pObj->GetControllable();
    }

    float GetSoldierInvisibilityFactor(const GameObject* pObj)
    {
        return static_cast<const EntitySoldier*>(pObj)->GetInvisibilityFactor();
    }

    int GetSoldierPlayerId(const GameObject* pObj)
    {
        return static_cast<const EntitySoldier*>(pObj)->mPlayerId;
    }

    void AddAidTypes(const float* pAddHealth, const float* pAddAmmo, unsigned int* pHealth, unsigned int* pAmmo)
    {
        for (int i = 0; i < NUM_HEALTH_TYPES; ++i)
        {
            if (pAddHealth[i] > 0.0f)
            {
                *pHealth |= 1 << i;
            }
            if (pAddAmmo[i] > 0.0f)
            {
                *pAmmo |= 1 << i;
            }
        }
    }
}

Controllable* RepairHelper::GetMe()
{
    return mAgent->mCtrl->mOwner;
}

UnitController* RepairHelper::GetCtrl()
{
    return mAgent->mCtrl;
}

// FUNCTION: SWBF2 0x0062e2c0
RepairHelper::RepairHelper(UnitAgent* pAgent)
    : mAgent(pAgent)
{
    mAidTarget.mObject = 0;
    mAidTarget.mSavedHandleId = 0;
    mCheckForRecipientTime = 0.0f;
}

// FUNCTION: SWBF2 0x0062e2f0
RepairHelper::~RepairHelper()
{
}

// SYNTHETIC: SWBF2 0x0062e290
// RepairHelper::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0062e300
bool RepairHelper::CheckForRecipientAndInit()
{
    int iSoldierType = GetMe()->GetGameObject()->GetGameObjectClass()->GetSoldierType();
    if (iSoldierType != GameObjectClass::SOLDIER_REPAIR && iSoldierType != GameObjectClass::SOLDIER_PILOT)
    {
        return false;
    }

    if (mCheckForRecipientTime > GameLoop::GetMissionTime())
    {
        return false;
    }
    mCheckForRecipientTime = GameLoop::GetMissionTime() + 5.0f;

    GameObject* pMe = GetMe()->GetGameObject();
    int iMyTeam = pMe->GetTeam();
    if (AIUtil::IsEndGame(iMyTeam))
    {
        return false;
    }

    unsigned int health = 0;
    unsigned int ammo = 0;
    CalcMyAidTypes(&health, &ammo);

    PblVector3 vMyPos = pMe->GetPosition();
    GameObject* objects[50];
    int iCount = TeamManager::GetObjectsInRange(vMyPos, 75.0f, objects, 50, pMe, AI::AFFIL_ALLYNEUTRAL);

    GameObject* pBest = 0;
    AidType bestAid;
    float fBestDistSq = FLT_MAX;
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObj = objects[i];
        if (pObj->GetGameObjectClass()->GetAINoRepair())
        {
            continue;
        }

        PblVector3 vHisPos = pObj->GetPosition();
        if (!pObj->IsAlive() && pObj->mHealthType != HEALTH_TYPE_BUILDING_DEAD &&
            pObj->mHealthType != HEALTH_TYPE_BUILDING_UNBUILT)
        {
            continue;
        }
        if (ReserveManager::GetOpenReservableSlots(pObj, iMyTeam, RT_REPAIR) < 1)
        {
            continue;
        }
        int iOwningTeam = pObj->mOwningTeam;
        if (iOwningTeam && Team::GetPtr(iOwningTeam)->GetAffiliation(iMyTeam) <= 0)
        {
            continue;
        }
        if (pObj->GetEntityType() == ENT_SOLDIER && GetSoldierInvisibilityFactor(pObj) > 0.0f)
        {
            continue;
        }

        AidType aid;
        if (!FindAidNeed(pObj, health, ammo, &aid, false))
        {
            continue;
        }

        PblVector3 vDiff(vMyPos.x - vHisPos.x, vMyPos.y - vHisPos.y, vMyPos.z - vHisPos.z);
        float fDistSq = vDiff.y * vDiff.y + vDiff.x * vDiff.x + vDiff.z * vDiff.z;
        PblVector3 vVel = pObj->GetVelocity();
        float fVelSq = vVel.y * vVel.y + vVel.x * vVel.x + vVel.z * vVel.z;
        if (pObj->GetControllable() && pObj->GetControllable()->GetPlayerId() >= 0 && fDistSq < 900.0f)
        {
            const PblMatrix& matrix = pObj->GetMatrix();
            PblVector3 vDir = AIUtil::GetSafeNormal(vDiff);
            if (fVelSq < 0.25f ||
                matrix.m[2][1] * vDir.y + matrix.m[2][0] * vDir.x + matrix.m[2][2] * vDir.z > (float)cos(15.0))
            {
                pBest = pObj;
                bestAid = aid;
                break;
            }
        }

        if (pObj->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) && GetSoldierPlayerId(pObj) < 0)
        {
            continue;
        }
        if (fVelSq < 0.25f && fDistSq < fBestDistSq)
        {
            pBest = pObj;
            bestAid = aid;
            fBestDistSq = fDistSq;
        }
    }

    if (!pBest)
    {
        return false;
    }

    bool bChanged = false;
    mAidType = bestAid;
    if (!SetAidWpn(&bChanged))
    {
        return false;
    }

    mAidTarget = pBest;
    ReserveManager::Reserve(pBest, pMe, RT_REPAIR);
    mContactTime = FLT_MAX;
    mSearchTime = GameLoop::GetMissionTime();
    mPathing = false;
    mPathTime = -100.0f;
    reinterpret_cast<AILowLevel*>(GetCtrl()->mLowLevel)->SetNavigator_Stationary();
    AI::VOHelper::Play(AI::VOT_RepairStart, GetMe()->GetGameObject(), 0, pBest, 0.0f);
    return true;
}

// FUNCTION: SWBF2 0x0062e7d0
bool RepairHelper::SetAidWpn(bool* pbChanged)
{
    for (int i = 0; i < GetMe()->GetWeaponCount(); ++i)
    {
        WeaponClass* pClass = GetMe()->GetWeaponPtr(i)->mClass;
        float fAdd;
        if (pClass->IsRtti(WeaponRepairClass::rttiHashWeaponRepairClass))
        {
            WeaponRepairClass* pRepair = static_cast<WeaponRepairClass*>(pClass);
            fAdd = mAidType.ammo ? pRepair->mAddAmmo[mAidType.type] : pRepair->mAddHealth[mAidType.type];
        }
        else if (pClass->IsRtti(WeaponDispenserClass::rttiHashWeaponDispenserClass))
        {
            EntityClass* pDispense = static_cast<WeaponDispenserClass*>(pClass)->mDispenseClass;
            if (!pDispense || !pDispense->IsRtti(PowerupItemClass::rttiHashPowerupItemClass))
            {
                continue;
            }
            PowerupItemClass* pPowerup = static_cast<PowerupItemClass*>(pDispense);
            fAdd = mAidType.ammo ? pPowerup->mAddAmmo[mAidType.type] : pPowerup->mAddHealth[mAidType.type];
        }
        else
        {
            continue;
        }

        if (fAdd > 0.0f)
        {
            if (GetMe()->GetWeaponIndex(GetMe()->GetWpnChannel()) != i)
            {
                reinterpret_cast<AILowLevel*>(UnitControllerPtr(GetMe()->mCtrl)->mLowLevel)->SetNextWeapon(i);
                *pbChanged = true;
                return true;
            }
            *pbChanged = false;
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0062e910
void RepairHelper::CalcMyAidTypes(unsigned int* pHealth, unsigned int* pAmmo)
{
    *pAmmo = 0;
    *pHealth = 0;
    for (int i = 0; i < GetMe()->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = GetMe()->GetWeaponPtr(i);
        if (pWeapon->m_pAmmoCounter->GetNumRounds() <= 0)
        {
            ++gOutOfAmmoWeaponCount;
            continue;
        }

        if (pWeapon->IsRtti(WeaponRepair::rttiHashWeaponRepair))
        {
            WeaponRepairClass* pClass = static_cast<WeaponRepairClass*>(pWeapon->mClass);
            AddAidTypes(pClass->mAddHealth, pClass->mAddAmmo, pHealth, pAmmo);
        }
        else if (pWeapon->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
        {
            EntityClass* pDispense = static_cast<WeaponDispenserClass*>(pWeapon->mClass)->mDispenseClass;
            if (pDispense && pDispense->IsRtti(PowerupItemClass::rttiHashPowerupItemClass))
            {
                PowerupItemClass* pPowerup = static_cast<PowerupItemClass*>(pDispense);
                AddAidTypes(pPowerup->mAddHealth, pPowerup->mAddAmmo, pHealth, pAmmo);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0062ec20
void RepairHelper::FindPowerupsNearby(const GameObject* pObj, unsigned int* pHealth, unsigned int* pAmmo)
{
    GameObject* objects[10];
    PblVector3 vPos = *reinterpret_cast<const PblVector3*>(&AsCollisionObject(pObj)->GetMatrix().m[3][0]);
    int iCount = CollisionManager::GetObjectsInRange(vPos, 15.0f, objects, 10, 0, 1);
    for (int i = 0; i < iCount; ++i)
    {
        Entity* pEntity = reinterpret_cast<Entity*>(objects[i]);
        if (!pEntity->IsRtti(PowerupItem::rttiHashPowerupItem))
        {
            continue;
        }
        const PowerupItemClass* pClass = static_cast<const PowerupItemClass*>(pEntity->GetEntityClass());
        AddAidTypes(pClass->mAddHealth, pClass->mAddAmmo, pHealth, pAmmo);
    }
}

// FUNCTION: SWBF2 0x0062edd0
void RepairHelper::FindAllAidNeeds(const GameObject* pObj, unsigned int health, unsigned int ammo,
                                   bool bUnused, bool* pbNeedHealth, bool* pbNeedAmmo)
{
    unsigned int nearHealth = 0;
    unsigned int nearAmmo = 0;
    HealthType type = GetHealthType(pObj);
    FindPowerupsNearby(pObj, &nearHealth, &nearAmmo);

    *pbNeedHealth = ((health >> type) & (~nearHealth >> type) & (GetMaxHealth(pObj) > GetCurHealth(pObj))) & 1;
    bool bCanAmmo = ((ammo >> type) & 1) != 0;
    bool bNoAmmoNear = ((~nearAmmo >> type) & 1) != 0;
    *pbNeedAmmo = false;

    if (GetControllable(pObj))
    {
        bool bLow = false;
        for (int i = 0; i < GetControllable(pObj)->GetWeaponCount(); ++i)
        {
            const Weapon* pWeapon = GetControllable(pObj)->GetWeaponPtr(i);
            if (pWeapon && pWeapon->m_pAmmoCounter->GetMaxClips() - 1.0f > pWeapon->m_pAmmoCounter->GetNumClips())
            {
                bLow = true;
            }
        }
        *pbNeedAmmo = bNoAmmoNear & bCanAmmo & bLow;
    }
}

// FUNCTION: SWBF2 0x0062eed0
bool RepairHelper::FindAidNeed(const GameObject* pObj, unsigned int health, unsigned int ammo,
                               AidType* pAid, bool b)
{
    bool bNeedHealth;
    bool bNeedAmmo;
    FindAllAidNeeds(pObj, health, ammo, b, &bNeedHealth, &bNeedAmmo);
    if (!bNeedHealth && !bNeedAmmo)
    {
        return false;
    }

    HealthType type = GetHealthType(pObj);
    if (bNeedHealth && bNeedAmmo)
    {
        *pAid = AidType(type, AIUtil::GetRandomInt(0, 1) == 1);
    }
    else
    {
        *pAid = AidType(type, bNeedAmmo);
    }
    return true;
}

// FUNCTION: SWBF2 0x0062ef50
bool RepairHelper::FireRepairWeapon(float dt, const GameObject* pTarget)
{
    const bool bLinedUp = CombatUtil::IsTargetLinedUpObj(GetMe(), pTarget);
    if (!bLinedUp)
    {
        return bLinedUp;
    }

    Weapon* pWeapon = GetMe()->GetCurWpn();
    if (pWeapon->IsRtti(WeaponRepair::rttiHashWeaponRepair))
    {
        if (mContactTime > GameLoop::GetMissionTime())
        {
            mContactTime = GameLoop::GetMissionTime();
            mHealth = GetCurHealth(pTarget);
        }
        UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().SetFireRequest(true, false);
    }
    else if (pWeapon->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
    {
        if (mContactTime > GameLoop::GetMissionTime())
        {
            mContactTime = GameLoop::GetMissionTime();
            mHealth = GetCurHealth(pTarget);
            UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().SetFireRequest(true, false);
            AI::VOHelper::Play(AI::VOT_GivePowerup, GetMe()->GetGameObject(), 0, pTarget, 0.0f);
        }
    }
    else
    {
        return false;
    }
    return bLinedUp;
}

// FUNCTION: SWBF2 0x0062f080
bool RepairHelper::Update(float dt)
{
    GameObject* pTarget = AIUtil::GetAlivePtr(mAidTarget, true);
    if (!pTarget)
    {
        return false;
    }
    ReserveManager::Reserve(pTarget, GetMe()->GetGameObject(), RT_REPAIR);

    bool bChanged = false;
    SetAidWpn(&bChanged);
    if (bChanged)
    {
        return true;
    }

    Controllable* pMe = GetMe();
    UnitControllerPtr(pMe->mCtrl)->GetLowLevel().SetTarget(pTarget);
    const PblVector3 vHisPos = *reinterpret_cast<const PblVector3*>(&AsCollisionObject(pTarget)->GetMatrix().m[3][0]);
    const PblVector3 vMyPos = *reinterpret_cast<const PblVector3*>(&pMe->GetMatrix().m[3][0]);
    const float fRange = CombatUtil::GetWeaponMaxRange(pMe);
    const float fDistSq = (vMyPos.x - vHisPos.x) * (vMyPos.x - vHisPos.x) + (vMyPos.z - vHisPos.z) * (vMyPos.z - vHisPos.z);
    const bool bTouching = pMe->GetCurrentCollision() == pTarget;
    const bool bTooClose = fDistSq < 1.0f;
    const bool bInRange = fDistSq < fRange * fRange;

    if ((bTouching || fDistSq < 25.0f) && bInRange && !bTooClose)
    {
        if (!bTouching ||
            AIUtil::DirectLineToTarget(GetMe()->GetGameObject(),
                                       *reinterpret_cast<const PblVector3*>(&AsCollisionObject(pTarget)->GetMatrix().m[3][0])))
        {
            GetCtrl()->GetLowLevel().SetNavigator_Stationary();
            mSearchTime = GameLoop::GetMissionTime();
            const bool bFired = FireRepairWeapon(dt, pTarget);

            unsigned int health = 0;
            unsigned int ammo = 0;
            CalcMyAidTypes(&health, &ammo);
            bool bNeedHealth;
            bool bNeedAmmo;
            FindAllAidNeeds(pTarget, health, ammo, false, &bNeedHealth, &bNeedAmmo);
            if (mAidType.ammo ? !bNeedAmmo : !bNeedHealth)
            {
                AI::VOHelper::Play(AI::VOT_RepairEnd, GetMe()->GetGameObject(), 0, pTarget, 0.0f);
                return false;
            }

            if (GameLoop::GetMissionTime() > mContactTime + 8.0f)
            {
                if (!(GetCurHealth(pTarget) > mHealth))
                {
                    return false;
                }
                mHealth = GetCurHealth(pTarget);
                mContactTime = GameLoop::GetMissionTime();
            }
            if (!bFired)
            {
                mContactTime = FLT_MAX;
            }
        }
        else
        {
            mContactTime = FLT_MAX;
        }
    }
    else
    {
        if (GameLoop::GetMissionTime() - mSearchTime > 20.0f)
        {
            if (!UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().HasPath())
            {
                return false;
            }
            if (UnitControllerPtr(GetMe()->mCtrl)->GetLowLevel().IsStuck())
            {
                return false;
            }
            ++gRepairSearchTimeoutCount;
        }
        AIUtil::Slide(GetMe()->GetGameObject(), pTarget, false, &mPathing, &mPathTime, 8.0f, 1.6f);
        mContactTime = FLT_MAX;
    }

    const PblMatrix& rMyMatrix = GetMe()->GetMatrix();
    const PblMatrix& rHisMatrix = AsCollisionObject(pTarget)->GetMatrix();
    const float fX = rHisMatrix.m[3][0] - rMyMatrix.m[3][0];
    const float fY = rHisMatrix.m[3][1] - rMyMatrix.m[3][1];
    const float fZ = rHisMatrix.m[3][2] - rMyMatrix.m[3][2];
    const float fReach = GetCollisionObject(pTarget)->GetCollisionRadius() + 8.0f;
    if (fReach * fReach > fX * fX + fY * fY + fZ * fZ)
    {
        GetCtrl()->GetLowLevel().SetHeading(pTarget);
        GetCtrl()->GetLowLevel().SetHeight(HEIGHT_STAND);
        return true;
    }
    mAgent->FaceTarget(HEIGHT_STAND);
    return true;
}
