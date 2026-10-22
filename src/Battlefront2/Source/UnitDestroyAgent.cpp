#include "UnitDestroyAgent.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntityBuilding.h"
#include "EventInfo.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "UnitController.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblRandom.h"

namespace AI
{

class UnitThreatManager
{
public:
    const GameObject* GetVisibleObject() const;
    void LockThreat(GameObject* pObj);

    void AddThreat(GameObject* pObj);
};
}

namespace AIUtil
{

bool CanSee(Controllable* pMe, const GameObject* pObj, PblVector3* pPos);
}

namespace CombatUtil
{
float GetWeaponMaxRange(const Controllable* pMe);
float GetWeaponMinRange(const Controllable* pMe);
}

namespace
{

    const CollisionObject* GetCollisionObject(const GameObject* pObject)
    {
        return static_cast<const CollisionObject*>(pObject);
    }
    const Damageable* GetDamageable(const GameObject* pObject)
    {
        return static_cast<const Damageable*>(pObject);
    }

    const GameObject* ResolveHandle(PblHandle<const GameObject>& handle)
    {
        const GameObject* pObject = handle.mObject;
        if (pObject)
        {
            if (pObject->mHandleId ==
                handle.mSavedHandleId)
            {
                return pObject;
            }
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return 0;
    }

    const WeaponClass* GetWeaponClass(const Weapon* pWeapon)
    {
        return pWeapon->GetClass();
    }
    bool IsMelee(Weapon* pWeapon)
    {
        return pWeapon->IsMelee();
    }

    AI::UnitThreatManager* GetThreatManager(UnitController* pCtrl)
    {
        return reinterpret_cast<AI::UnitThreatManager*>(pCtrl->mThreatManager);
    }
}

// FUNCTION: SWBF2 0x00666300
static float GetOptimalBuildingKillingRange(const Controllable* pMe)
{
    float fRange = 0.0f;
    for (int i = 0; i < pMe->GetWeaponCount(); ++i)
    {
        const WeaponClass* pClass = GetWeaponClass(pMe->GetWeaponPtr(i));
        if ((pClass->mAITargetTypes & 0x10) && fRange <= pClass->mOptimalRange)
        {
            fRange = pClass->mOptimalRange;
        }
    }
    return fRange;
}

// GLOBAL: SWBF2 0x01f993b8
// SYNTHETIC: SWBF2 0x00406cb0
// `dynamic initializer for 'UnitDestroyAgent::rttiHashUnitDestroyAgent''
PblHash UnitDestroyAgent::rttiHashUnitDestroyAgent("UnitDestroyAgent");
char UnitDestroyAgent::rttiNameUnitDestroyAgent[] = "UnitDestroyAgent";

// FUNCTION: SWBF2 0x00666000
bool UnitDestroyAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitDestroyAgent || UnitAgent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00666030
unsigned int UnitDestroyAgent::GetDerivedRtti() const
{
    return rttiHashUnitDestroyAgent;
}

// FUNCTION: SWBF2 0x00666040
const char* UnitDestroyAgent::GetDerivedRttiName() const
{
    return rttiNameUnitDestroyAgent;
}

// SYNTHETIC: SWBF2 0x00666050
// UnitDestroyAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00666080
UnitDestroyAgent::UnitDestroyAgent(UnitController* pCtrl, const AI::GoalTarget& target)
    : UnitAgent(pCtrl)
{
    mTarget = target;
    mCurState = 1;
    mNextState = 2;
}

// FUNCTION: SWBF2 0x006660f0
void UnitDestroyAgent::EnterState()
{
    if (mCurState != STATE_Goto)
    {
        return;
    }

    PblVector3 vPos;
    if (!mTarget.GetLocation(vPos))
    {
        if (mTarget.mType == AI::GTT_GAMEOBJECT)
        {
            SetNextState(100);
            return;
        }
        if (mTarget.mType == AI::GTT_CHARACTER)
        {
            SetNextState(STATE_WaitForSpawn);
            return;
        }
    }

    mLastSeekPos = vPos;
    PathFinder::GetClosestValidPoint(vPos, vPos, GetMe()->GetGameObject());

    Controllable* pMe = GetMe();
    float fRand = AIUtil::GetRandomFloat(3.0f, 10.0f);
    float fDist = CombatUtil::GetWeaponMinRange(pMe);
    float fMax = CombatUtil::GetWeaponMaxRange(pMe);
    if (fDist <= fRand)
    {
        fDist = fRand > fMax ? fMax : fRand;
    }
    if (GetMe()->GetWeaponCount() >= 1 && IsMelee(GetMe()->GetWeaponPtr(0)))
    {
        fDist = 0.0f;
    }

    PblVector3 vDest = PblRandom::GetGlobalVector3();
    PblVector3 vDir = AIUtil::GetSafeNormalXZ(vDest);
    vDest.x = vPos.x + vDir.x * fDist;
    vDest.y = vPos.y + vDir.y * fDist;
    vDest.z = vPos.z + vDir.z * fDist;
    PathFinder::PushValidPoint(vPos, vDest, GetMe()->GetGameObject());

    GetLowLevel().SetNavigator_Goto(vDest, 0, false, false);
}

// FUNCTION: SWBF2 0x00666290
void UnitDestroyAgent::ExitState()
{
}

// FUNCTION: SWBF2 0x006662a0
void UnitDestroyAgent::CheckTargetPosition(const PblVector3& vPos)
{
    float dx = vPos.x - mLastSeekPos.x;
    float dy = vPos.y - mLastSeekPos.y;
    float dz = vPos.z - mLastSeekPos.z;
    if (dx * dx + dy * dy + dz * dz > 15.0f * 15.0f)
    {
        SetNextState(2);
    }
}

// FUNCTION: SWBF2 0x00666370
void UnitDestroyAgent::UpdateStatePostVision(float dt)
{
    reinterpret_cast<AttackHelper*>(mAttackHelper)->Update();

    if (mCurState == STATE_Goto)
    {
        PblVector3 vPos;
        if (!mTarget.GetLocation(vPos))
        {
            if (mTarget.mType == AI::GTT_GAMEOBJECT)
            {
                SetNextState(100);
            }
            else if (mTarget.mType == AI::GTT_CHARACTER)
            {
                SetNextState(STATE_WaitForSpawn);
            }
            return;
        }

        CheckTargetPosition(vPos);

        GameObject* pMyObj = GetMe()->GetGameObject();
        const PblVector3& vMyPos = GetCollisionObject(pMyObj)->GetPosition();
        float dx = vPos.x - vMyPos.x;
        float dy = vPos.y - vMyPos.y;
        float dz = vPos.z - vMyPos.z;
        float fDistSq = dy * dy + dx * dx + dz * dz;

        bool bMelee = false;
        if (GetMe()->GetWeaponCount() >= 1 && IsMelee(GetMe()->GetWeaponPtr(0)))
        {
            bMelee = true;
        }

        const Damageable* pMyHealth = GetDamageable(pMyObj);
        bool bAttacking = false;
        if (pMyHealth->mCurHealth > pMyHealth->mMaxHealth * 0.5f &&
            mTarget.mType == AI::GTT_GAMEOBJECT && !bMelee && GetLowLevel().IsNavigatorInProgress())
        {
            const Entity* pTarget = reinterpret_cast<const Entity*>(ResolveHandle(mTarget.mGameObj));
            if (pTarget->IsRtti(EntityBuilding::rttiHashEntityBuilding))
            {
                if (GetThreatManager(mCtrl)->GetVisibleObject() == mTarget.GetGameObject())
                {
                    float fRange = GetOptimalBuildingKillingRange(GetMe());
                    if (fRange * fRange > fDistSq)
                    {
                        GetLowLevel().SetNavigator_Stationary();
                        mCtrl->ForceSetAttackHelperTarget(mTarget.GetGameObject());
                        reinterpret_cast<AI::UnitThreatManager*>(mCtrl->mThreatManager)->AddThreat(
                                   const_cast<GameObject*>(mTarget.GetGameObject()));
                        bAttacking = true;
                    }
                }
            }
        }

        if (pMyHealth->mCurHealth > pMyHealth->mMaxHealth * 0.5f &&
            mTarget.mType == AI::GTT_GAMEOBJECT)
        {
            float fNear = AIUtil::GetCollisionRadius(pMyObj) * 30.0f;
            if (!bAttacking && fNear * fNear > fDistSq)
            {
                vPos = GetCollisionObject(mTarget.GetGameObject())->GetPosition();
                if (GetThreatManager(mCtrl)->GetVisibleObject() == mTarget.GetGameObject() ||
                    AIUtil::CanSee(GetMe(), mTarget.GetGameObject(), &vPos))
                {
                    mCtrl->ForceSetAttackHelperTarget(mTarget.GetGameObject());
                    GetThreatManager(mCtrl)->LockThreat(const_cast<GameObject*>(mTarget.GetGameObject()));
                }
            }
        }

        FaceTarget(HEIGHT_STAND);
    }
    else if (mCurState == STATE_WaitForSpawn)
    {
        if (mTarget.mChr->GetControllable())
        {
            SetNextState(STATE_Goto);
        }
    }
}

// FUNCTION: SWBF2 0x00666660
int UnitDestroyAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_Grenade:
    case AI::EVT_Vision_CanSeeEnemy:
    case AI::EVT_Audible_CanHearEnemy:
    case AI::EVT_EndInvisibility:
        return AI::EVT_NotHandled;

    case AI::EVT_Damaged:
        DamagedEventPlayerCentricness(pInfo);
        PlayDamageEventVO(pInfo);
        return AI::EVT_NotHandled;

    default:
        return UnitAgent::EventHandler(pInfo);
    }
}

// FUNCTION: SWBF2 0x006666e0
const char* UnitDestroyAgent::NFINAL_GetStateString(int iState) const
{
    return 0;
}
