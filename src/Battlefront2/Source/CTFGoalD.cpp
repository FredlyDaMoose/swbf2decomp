#include "CTFGoalD.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "RedEngineFL/Util/RedRegion.h"

// FUNCTION: SWBF2 0x00488290
AI::GoalTarget& AI::GoalTarget::operator=(const GoalTarget& other)
{
    mType = other.mType;
    mPos = other.mPos;
    mChr = other.mChr;
    mGameObj = other.mGameObj;
    return *this;
}

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

inline const RedRegion* FlagHomeRegion(const GameObject* pFlag)
{
    return static_cast<const FlagItem*>(pFlag)->mHomeRegion;
}

inline const PblVector3& RegionPosition(const RedRegion* pRegion)
{
    return *reinterpret_cast<const PblVector3*>(&pRegion->mMatrix.m[3][0]);
}
}

// GLOBAL: SWBF2 0x01eb97b0
// SYNTHETIC: SWBF2 0x004027b0
// `dynamic initializer for 'AI::CTFGoalD::rttiHashCTFGoalD''
PblHash AI::CTFGoalD::rttiHashCTFGoalD("CTFGoalD");
char AI::CTFGoalD::rttiNameCTFGoalD[] = "CTFGoalD";

// FUNCTION: SWBF2 0x00488040
bool AI::CTFGoalD::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashCTFGoalD || AIGoal::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00488070
unsigned int AI::CTFGoalD::GetDerivedRtti() const
{
    return rttiHashCTFGoalD;
}

// FUNCTION: SWBF2 0x00488080
const char* AI::CTFGoalD::GetDerivedRttiName() const
{
    return rttiNameCTFGoalD;
}

// SYNTHETIC: SWBF2 0x00488090
// AI::CTFGoalD::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00487fb0
AI::GoalTarget::GoalTarget(const PblVector3& vPos)
    : mType(GTT_POS), mPos(vPos), mChr(0)
{
}

// FUNCTION: SWBF2 0x00487ff0
AI::GoalTarget::GoalTarget(const GameObject* pObject)
    : mType(GTT_GAMEOBJECT), mPos(0, 0, 0), mChr(0)
{
    SetHandle(mGameObj, pObject);
}

// FUNCTION: SWBF2 0x004880c0
AI::CTFGoalD::CTFGoalD(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_CTFDEFENSE, uTeam, fWeight)
{
    SetHandle(mFlag, ReadHandle(target.mGameObj));
    if (!ReadHandle(mFlag))
    {
        for (PblList<FlagItem>::Itor it(*FlagItem::sList); it; ++it)
        {
            const GameObject* pFlag = reinterpret_cast<const GameObject*>(static_cast<FlagItem*>(it));
            if (GameObjectTeam(pFlag) == uTeam)
            {
                SetHandle(mFlag, pFlag);
                break;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00488190
void AI::CTFGoalD::Assign(Character* pChr)
{
    if (!ReadHandle(mFlag) || !pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (!pCtrl)
    {
        return;
    }

    GoalTarget target(RegionPosition(FlagHomeRegion(ReadHandle(mFlag))));
    if (pCtrl->GetRandomSeed() % 2 == 1)
    {
        target = GoalTarget(ReadHandle(mFlag));
    }
    pCtrl->ForceCommand(UnitGoalAgentInfo(CMD_DEFEND, &target));
}
