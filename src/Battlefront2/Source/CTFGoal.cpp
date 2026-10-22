#include "CTFGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

namespace
{
inline unsigned int GameObjectHandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

inline int GameObjectTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}

inline Character* ItemOwner(const GameObject* pItem)
{
    return static_cast<const FlagItem*>(pItem)->mOwner;
}

inline const GameObject* ReadHandle(const PblHandle<const GameObject>& handle)
{
    PblHandle<const GameObject>& h = const_cast<PblHandle<const GameObject>&>(handle);
    if (h.mObject && GameObjectHandleId(h.mObject) != h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

inline void SetHandle(PblHandle<const GameObject>& handle, const GameObject* pObject)
{
    handle.mObject = pObject;
    handle.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
}
}

// GLOBAL: SWBF2 0x01eb97ac
// SYNTHETIC: SWBF2 0x004027a0
// `dynamic initializer for 'AI::CTFGoal::rttiHashCTFGoal''
PblHash AI::CTFGoal::rttiHashCTFGoal("CTFGoal");
char AI::CTFGoal::rttiNameCTFGoal[] = "CTFGoal";

// FUNCTION: SWBF2 0x00487c20
bool AI::CTFGoal::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashCTFGoal || AIGoal::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00487c50
unsigned int AI::CTFGoal::GetDerivedRtti() const
{
    return rttiHashCTFGoal;
}

// FUNCTION: SWBF2 0x00487c60
const char* AI::CTFGoal::GetDerivedRttiName() const
{
    return rttiNameCTFGoal;
}

// SYNTHETIC: SWBF2 0x00487c70
// AI::CTFGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00487ca0
AI::CTFGoal::CTFGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_CTFOFFENSE, uTeam, fWeight)
{
    mCapturePos = target.mPos;
    SetHandle(mFlag, ReadHandle(target.mGameObj));
    if (!ReadHandle(mFlag))
    {
        for (PblList<FlagItem>::Itor it(*FlagItem::sList); it; ++it)
        {
            const GameObject* pFlag = reinterpret_cast<const GameObject*>(static_cast<FlagItem*>(it));
            if (GameObjectTeam(pFlag) != uTeam)
            {
                SetHandle(mFlag, pFlag);
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00487d80
void AI::CTFGoal::Assign(Character* pChr)
{
    if (!ReadHandle(mFlag) || !pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (pCtrl)
    {
        pCtrl->ForceCommand(UnitGoalAgentInfo(
            CMD_CTFOFFENSE, reinterpret_cast<const FlagItem*>(ReadHandle(mFlag)), mCapturePos));
    }
}

// FUNCTION: SWBF2 0x00487e40
void AI::CTFGoal::Update()
{
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pChr = Character::GetPtr(i);
        if (!pChr || pChr->mTeamNumber != mTeam)
        {
            continue;
        }
        const GameObject* pFlag = ReadHandle(mFlag);
        if (!pFlag || ItemOwner(pFlag) != pChr)
        {
            continue;
        }
        Controllable* pControllable = pChr->GetControllable();
        if (!pControllable)
        {
            continue;
        }
        UnitController* pCtrl = UnitControllerPtr(pControllable->mCtrl);
        if (pCtrl && (pCtrl->GetCurCmdType() != CMD_CTFOFFENSE || pCtrl->IsAgentDone()) &&
            !pCtrl->IsNewCommand())
        {
            AddUnit(pChr);
            Assign(pChr);
        }
    }
    AIGoal::Update();
}
