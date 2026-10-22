#include <new>

#include "UnitCmdInfo.h"
#include "AIGoal.h"
#include "AIUtil.h"
#include "GameObject.h"

static unsigned int GetHandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

static const GameObject* GetObject(PblHandle<const GameObject>& handle)
{
    if (handle.mObject && GetHandleId(handle.mObject) != handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

static void SetObject(PblHandle<GameObject>& handle, const GameObject* pObject)
{
    handle.mObject = const_cast<GameObject*>(pObject);
    handle.mSavedHandleId = pObject ? GetHandleId(pObject) : 0;
}

static GameObject* GetObject(PblHandle<GameObject>& handle)
{
    if (handle.mObject && GetHandleId(handle.mObject) != handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

static void SetObject(PblHandle<const GameObject>& handle, const GameObject* pObject)
{
    handle.mObject = pObject;
    handle.mSavedHandleId = pObject ? GetHandleId(pObject) : 0;
}

// FUNCTION: SWBF2 0x00662a70
DroidRepairInfo::DroidRepairInfo(PblHandle<const GameObject> who,
                                 PblHandle<const GameObject> owner)
    : UnitCmdInfo(CMD_REPAIR, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                  PblHandle<GameObject>())
{
    SetObject(mWho, GetObject(who));
    SetObject(mOwner, GetObject(owner));
}

// FUNCTION: SWBF2 0x006626b0
UnitCmdInfo::UnitCmdInfo(UnitControllerCommand eCmd, PblHandle<GameObject> who,
                         const PblVector3& pos, PblHandle<GameObject> owner)
    : CmdInfo(eCmd, who, pos, owner)
{
}

// FUNCTION: SWBF2 0x006626d0
UnitCmdInfo::UnitCmdInfo(UnitControllerCommand eCmd, ConstCommandPostHandle post)
    : CmdInfo(eCmd, post)
{
}

// FUNCTION: SWBF2 0x006626f0
UnitControllerCommand UnitCmdInfo::GetCommand() const
{
    return (UnitControllerCommand)mWhat;
}

// FUNCTION: SWBF2 0x00662700
UnitFollowInfo::UnitFollowInfo(PblHandle<GameObject> who, PblHandle<GameObject> owner, bool bParam)
    : UnitCmdInfo(CMD_SC_FOLLOW, who, PblVector3(0, 0, 0), owner)
{
    mParam = bParam;
}

// FUNCTION: SWBF2 0x00662750
UnitCombatInfo::UnitCombatInfo(State eState, PblHandle<const GameObject> who)
    : UnitCmdInfo(CMD_COMBAT, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    SetState(eState);
    SetObject(mWho, GetObject(who));
}

// FUNCTION: SWBF2 0x006627d0
UnitCombatInfo::UnitCombatInfo(State eState, const PblVector3& targetPos, bool bAllowRoll)
    : UnitCmdInfo(CMD_COMBAT, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    SetState(eState);
    mPos = targetPos;
    mChr = (Character*)(unsigned int)bAllowRoll;
}

// FUNCTION: SWBF2 0x00662850
UnitCombatInfo::UnitCombatInfo(State eState)
    : UnitCmdInfo(CMD_COMBAT, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    SetState(eState);
}

// FUNCTION: SWBF2 0x00662a20
UnitRandomInfo::UnitRandomInfo()
    : UnitCmdInfo(CMD_RANDOM, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
}

// FUNCTION: SWBF2 0x006628b0
void UnitCombatInfo::SetState(State eState)
{
    mParam = eState;
}

// FUNCTION: SWBF2 0x006628c0
UnitCombatInfo::State UnitCombatInfo::GetState() const
{
    return (State)mParam;
}

// FUNCTION: SWBF2 0x006628d0
const PblVector3& UnitCombatInfo::GetTargetPos() const
{
    return mPos;
}

// FUNCTION: SWBF2 0x006628e0
const GameObject* UnitCombatInfo::GetWho() const
{
    return AIUtil::GetAlivePtr(mWho, false);
}

// FUNCTION: SWBF2 0x006628f0
bool UnitCombatInfo::GetAllowRoll() const
{
    return mChr != 0;
}

// FUNCTION: SWBF2 0x00662960
UnitBoardInfo::UnitBoardInfo(PblHandle<GameObject> who, int iParam)
    : UnitCmdInfo(CMD_BOARD, who, PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    mParam = iParam;
}

// FUNCTION: SWBF2 0x006629b0
GameObject* UnitBoardInfo::GetVehicle() const
{
    return AIUtil::GetAlivePtr(mWho, false);
}

// FUNCTION: SWBF2 0x006629c0
int UnitBoardInfo::GetVOResponseHandle() const
{
    return mParam;
}

// FUNCTION: SWBF2 0x006629d0
UnitUnBoardInfo::UnitUnBoardInfo()
    : UnitCmdInfo(CMD_UNBOARD, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
}

// FUNCTION: SWBF2 0x00662b40
UnitWaitSecondaryInfo::UnitWaitSecondaryInfo(float fTime, bool bParam)
    : UnitCmdInfo(CMD_WAIT_SECONDARY, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    mParam = (int)(fTime * 1000.0f) | 1;
}

// FUNCTION: SWBF2 0x00662900
UnitCaptureCPInfo::UnitCaptureCPInfo(ConstCommandPostHandle post)
    : UnitCmdInfo(CMD_CAPTURECP, post)
{
}

// FUNCTION: SWBF2 0x00662920
ConstCommandPostHandle UnitCaptureCPInfo::GetCommandPost() const
{
    return ConstCommandPostHandle(mWhoCommandPost);
}

// FUNCTION: SWBF2 0x00662bb0
UnitGoalAgentInfo::UnitGoalAgentInfo(UnitControllerCommand eCmd, const AI::GoalTarget* pTarget)
    : UnitCmdInfo(eCmd, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    mPos = pTarget->mPos;
    mParam = pTarget->mType;
    mChr = pTarget->mChr;
    SetObject(mWho, GetObject(const_cast<AI::GoalTarget*>(pTarget)->mGameObj));
}

// FUNCTION: SWBF2 0x00662c70
UnitGoalAgentInfo::UnitGoalAgentInfo(UnitControllerCommand eCmd, const FlagItem* pFlag, const PblVector3& vPos)
    : UnitCmdInfo(eCmd, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>())
{
    mPos = vPos;
    SetObject(mFlag, reinterpret_cast<const GameObject*>(pFlag));
    mParam = 4;
}

// FUNCTION: SWBF2 0x00662d10
const PblVector3 UnitGoalAgentInfo::GetPos() const
{
    if (mParam == 1 || mParam == 4)
    {
        return mPos;
    }
    return PblVector3(0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x00662d70
const FlagItem* UnitGoalAgentInfo::GetFlag() const
{
    return reinterpret_cast<const FlagItem*>(GetObject(const_cast<PblHandle<const GameObject>&>(mFlag)));
}

// FUNCTION: SWBF2 0x00662da0
void UnitGoalAgentInfo::GetGoalTarget(AI::GoalTarget& target) const
{
    target.mType = (AI::GoalTargetType)mParam;
    target.mPos = mPos;
    target.mChr = mChr;
    SetObject(target.mGameObj, GetObject(const_cast<PblHandle<GameObject>&>(mWho)));
}
