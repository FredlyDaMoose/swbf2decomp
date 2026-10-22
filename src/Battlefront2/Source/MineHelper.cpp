#include "MineHelper.h"

#include "AILowLevel.h"
#include "Controllable.h"
#include "EntityMine.h"
#include "GameLoop.h"
#include "HintManager.h"
#include "UnitAgent.h"
#include "UnitController.h"
#include "WeaponDispenser.h"
#include "GameObject.h"
#include "RedEngineFL/Application/RedTerrain.h"

namespace AI { enum AIAffil { AFFIL_ENEMY = 1, AFFIL_NEUTRAL = 2, AFFIL_ALLY = 4 }; }
namespace TeamManager {
int GetObjectsInRange(const PblVector3&, float, GameObject** const, int,
                      const GameObject*, AI::AIAffil);
}

class AmmoCounter
{
public:
    int GetNumRounds() const;
};

// FUNCTION: SWBF2 0x005a3c60
MineHelper::MineHelper(UnitAgent* pAgent)
    : HelperFsm(STATE_START, STATE_START)
{
    mAgent = pAgent;
    mNextMineCheckTime = 0.0f;
}

// FUNCTION: SWBF2 0x005a3c90
bool MineHelper::CheckForHintsAndInit()
{
    if (!HELPERIsValidMineLayer(GetMe()))
    {
        return false;
    }
    if (mNextMineCheckTime > GameLoop::GetMissionTime())
    {
        return false;
    }

    mNextMineCheckTime = GameLoop::GetMissionTime() + 5.0f;
    mHint = NULL;
    if (!HELPERIsMinePosNearby((const MinefieldHint**)&mHint, &mMinePos))
    {
        return false;
    }

    mHint->Reserve(GetMe()->GetGameObject());
    SetNextState(STATE_GOTO);
    FSMChangeState();
    return true;
}

// FUNCTION: SWBF2 0x005a3d70
void MineHelper::EnterState()
{
    switch (mCurState)
    {
    case STATE_GOTO:
        mFacePos = mMinePos;
        if (RedTerrain::_bEnable)
        {
            mFacePos.y = RedTerrain::GetInterpolatedHeight(mFacePos.x, mFacePos.z) + 1.5f;
        }
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(GetMe()->mCtrl)->mLowLevel)
            ->SetNavigator_Goto(mMinePos, NULL, false, false);
        break;

    case STATE_LAYMINE:
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(GetMe()->mCtrl)->mLowLevel)
            ->SetNavigator_Stationary();
        mMineTimer = -1.0f;
        mLaidMine = false;
        break;
    }
}

// FUNCTION: SWBF2 0x005a3e20
void MineHelper::ExitState()
{
}

// FUNCTION: SWBF2 0x005a3e30
void MineHelper::UpdateState(float)
{
    switch (mCurState)
    {
    case STATE_GOTO:
        mHint->Reserve(GetMe()->GetGameObject());
        if (reinterpret_cast<AILowLevel*>(UnitControllerPtr(GetMe()->mCtrl)->mLowLevel)->IsAtDest())
        {
            SetNextState(STATE_LAYMINE);
        }
        break;

    case STATE_LAYMINE:
    {
        mHint->Reserve(GetMe()->GetGameObject());
        bool bSwitching;
        if (!SetMineWpn(&bSwitching))
        {
            SetNextState(100);
        }
        else if (!bSwitching)
        {
            if (!mLaidMine)
            {
                reinterpret_cast<AILowLevel*>(UnitControllerPtr(GetMe()->mCtrl)->mLowLevel)
                    ->SetFireRequest(true, false);
                mMineTimer = GameLoop::GetMissionTime();
                mLaidMine = true;
            }
            else if (GameLoop::GetMissionTime() > mMineTimer + 3.0f)
            {
                if (!CheckForHintsAndInit())
                {
                    SetNextState(100);
                }
            }
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x005a3d30
static bool sHasMines(const WeaponDispenser* pWpn)
{
    if (pWpn->m_pAmmoCounter->GetNumRounds() == 0)
    {
        return false;
    }

    int iMaxItems = static_cast<const WeaponDispenserClass*>(pWpn->mClass)->mMaxItems;
    if (iMaxItems && *pWpn->GetItemCount() >= iMaxItems)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x005a4170
bool MineHelper::IsValidMineLayer(const Controllable* pMe) const
{
    for (int i = 0; i < pMe->GetWeaponCount(); ++i)
    {
        Weapon* pWpn = pMe->GetWeaponPtr(i);
        if (!pWpn || !pWpn->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
        {
            continue;
        }
        EntityClass* pDispense =
            static_cast<const WeaponDispenserClass*>(pWpn->mClass)->mDispenseClass;
        if (pDispense && pDispense->IsRtti(EntityMineClass::rttiHashEntityMineClass)
            && sHasMines(static_cast<const WeaponDispenser*>(pWpn)))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005a4200
bool sContainsEnemyMines(const MinefieldHint* pHint, const GameObject* pMe)
{
    PblVector3 vPos = pHint->GetCollisionCenter();
    if (RedTerrain::_bEnable)
    {
        vPos.y = RedTerrain::GetInterpolatedHeight(vPos.x, vPos.z);
    }

    GameObject* apObjects[1];
    const int iCount = TeamManager::GetObjectsInRange(vPos, 6.1f, apObjects, 1,
                                                       pMe, AI::AFFIL_ENEMY);
    for (int i = 0; i < iCount; ++i)
    {
        if (apObjects[i]->GetGameObject()->IsRtti(EntityMine::rttiHashEntityMine))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005a4000
bool MineHelper::FindMinePos(const MinefieldHint* pHint, PblVector3* pPos) const
{
    PblVector3 vPos = pHint->GetCollisionCenter();
    if (RedTerrain::_bEnable)
    {
        vPos.y = RedTerrain::GetInterpolatedHeight(vPos.x, vPos.z);
    }

    GameObject* apObjects[30];
    const int iCount = TeamManager::GetObjectsInRange(vPos, 6.1f, apObjects, 30,
                                                       GetMe()->GetGameObject(), AI::AFFIL_ALLY);
    const PblVector3* apMinePositions[30];
    int iMineCount = 0;
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObject = apObjects[i];
        if (pObject->GetGameObject()->IsRtti(EntityMine::rttiHashEntityMine))
        {
            apMinePositions[iMineCount++] = &pObject->GetMatrix().GetTranslation();
        }
    }
    bool bFound = false;
    for (int i = 0; i < iMineCount && !bFound; ++i)
    {
        const PblVector3& vMine = *apMinePositions[i];
        const float fX = vPos.x - vMine.x;
        const float fZ = vPos.z - vMine.z;
        bFound = fX * fX + fZ * fZ < 5.0625f;
    }
    if (bFound)
    {
        return false;
    }
    *pPos = vPos;
    return true;
}

// FUNCTION: SWBF2 0x005a42b0
bool MineHelper::HELPERIsMinePosNearby(const MinefieldHint** ppHint, PblVector3* pPos) const
{
    const BaseHint* apHints[3];
    int iCount = HintManager::RequestMinefieldHints(mAgent->mCtrl, GetMe()->GetMatrix().GetTranslation(),
                                                    100, 3, apHints);
    for (int i = 0; i < iCount; ++i)
    {
        const MinefieldHint* pHint = static_cast<const MinefieldHint*>(apHints[i]);
        if (pHint->IsReserved(GetMe()->GetGameObject()->GetTeam()))
        {
            continue;
        }
        if (sContainsEnemyMines(pHint, GetMe()->GetGameObject()))
        {
            continue;
        }

        PblVector3 vPos;
        if (FindMinePos(pHint, &vPos))
        {
            *ppHint = pHint;
            *pPos = vPos;
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x005a41f0
bool MineHelper::HELPERIsValidMineLayer(const Controllable* pMe) const
{
    return IsValidMineLayer(pMe);
}

// FUNCTION: SWBF2 0x005a4370
const Controllable* MineHelper::GetMe() const
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x005a4380
Controllable* MineHelper::GetMe()
{
    return mAgent->mCtrl->mOwner;
}

// FUNCTION: SWBF2 0x005a3d20
void MineHelper::HELPERClean()
{
    ExitState();
}

// FUNCTION: SWBF2 0x005a4390
bool MineHelper::HELPERGrenadeEventInterrupt() const
{
    return true;
}

// FUNCTION: SWBF2 0x005a43a0
bool MineHelper::HELPERDamagedInterrupt(bool, const PblVector3&) const
{
    return true;
}

// FUNCTION: SWBF2 0x005a43b0
bool MineHelper::HELPERSeeEnemyInterrupt(const GameObject*) const
{
    return true;
}

// FUNCTION: SWBF2 0x005a43c0
bool MineHelper::HELPERHearEnemyInterrupt(const GameObject*) const
{
    return true;
}

// FUNCTION: SWBF2 0x005a43d0
bool MineHelper::HELPERVehicleNearbyInterrupt(const GameObject*) const
{
    return true;
}

// FUNCTION: SWBF2 0x005a3f30
bool MineHelper::SetMineWpn(bool* pbSwitching)
{
    *pbSwitching = false;
    Weapon* pCurrentWeapon = GetMe()->GetCurWpn();
    int iWeapon = 0;
    if (GetMe()->GetWeaponCount() <= 0)
    {
        return false;
    }
    for (;;)
    {
        Weapon* pWeapon = GetMe()->GetWeaponPtr(iWeapon);
        if (pWeapon->IsRtti(WeaponDispenser::rttiHashWeaponDispenser))
        {
            EntityClass* pDispenseClass = static_cast<WeaponDispenserClass*>(pWeapon->mClass)->mDispenseClass;
            if (pDispenseClass->IsRtti(EntityMineClass::rttiHashEntityMineClass) &&
                sHasMines(static_cast<WeaponDispenser*>(pWeapon)))
            {
                break;
            }
        }
        if (++iWeapon >= GetMe()->GetWeaponCount())
        {
            return false;
        }
    }
    if (pCurrentWeapon != GetMe()->GetWeaponPtr(iWeapon))
    {
        reinterpret_cast<AILowLevel*>(UnitControllerPtr(GetMe()->mCtrl)->mLowLevel)->SetNextWeapon(iWeapon);
        *pbSwitching = true;
    }
    return true;
}
