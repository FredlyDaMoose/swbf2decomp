#pragma once

#include "CmdInfo.h"

enum UnitControllerCommand
{
    CMD_NONE = 0,
    CMD_WAIT = 1,
    CMD_COMBAT = 2,
    CMD_WAIT_SECONDARY = 3,
    CMD_BOARD = 4,
    CMD_UNBOARD = 5,
    CMD_REPAIR = 6,
    CMD_RANDOM = 7,
    CMD_CAPTURECP = 8,
    CMD_CTFOFFENSE = 9,
    CMD_DEATHMATCH = 10,
    CMD_DEFEND = 11,
    CMD_DESTROY = 12,
    CMD_SC_FOLLOW = 13,
    CMD_SC_START = 13,
    CMD_SC_END = 14,
    CMD_SC_HOLD = 14
};

class UnitCmdInfo : public CmdInfo
{
public:

    UnitCmdInfo(UnitControllerCommand eCmd, PblHandle<GameObject> who,
                const PblVector3& pos, PblHandle<GameObject> owner);
    UnitCmdInfo(UnitControllerCommand eCmd, ConstCommandPostHandle post);

    UnitControllerCommand GetCommand() const;
};

class UnitCombatInfo : public UnitCmdInfo
{
public:

    enum State
    {
        TURN = 0,
        ENGAGE = 1,
        FLEE_EVENT = 2,
        INVESTIGATE = 3
    };

    UnitCombatInfo(State eState);
    UnitCombatInfo(State eState, const PblVector3& targetPos, bool bAllowRoll);

    UnitCombatInfo(State eState, PblHandle<const GameObject> who);

    State GetState() const;
    const PblVector3& GetTargetPos() const;
    const GameObject* GetWho() const;
    bool GetAllowRoll() const;

protected:
    void SetState(State eState);
};

class UnitFollowInfo : public UnitCmdInfo
{
public:
    UnitFollowInfo(PblHandle<GameObject> who, PblHandle<GameObject> owner, bool bParam);

    PblHandle<GameObject> GetWho() const;
};

class UnitWaitSecondaryInfo : public UnitCmdInfo
{
public:
    UnitWaitSecondaryInfo(float fTime, bool bParam);
};

class UnitBoardInfo : public UnitCmdInfo
{
public:
    UnitBoardInfo(PblHandle<GameObject> who, int iParam);

    GameObject* GetVehicle() const;
    int GetVOResponseHandle() const;
};

class UnitCaptureCPInfo : public UnitCmdInfo
{
public:
    UnitCaptureCPInfo(ConstCommandPostHandle post);

    ConstCommandPostHandle GetCommandPost() const;
};
 

class FlagItem;
namespace AI { struct GoalTarget; }

class UnitGoalAgentInfo : public UnitCmdInfo
{
public:

    UnitGoalAgentInfo(UnitControllerCommand eCmd, const AI::GoalTarget* pTarget);

    UnitGoalAgentInfo(UnitControllerCommand eCmd, const FlagItem* pFlag, const PblVector3& vPos);

    const PblVector3 GetPos() const;
    const FlagItem* GetFlag() const;
    void GetGoalTarget(AI::GoalTarget& target) const;
};

class UnitUnBoardInfo : public UnitCmdInfo
{
public:
    UnitUnBoardInfo();
};

class UnitRandomInfo : public UnitCmdInfo
{
public:
    UnitRandomInfo();
};

class DroidRepairInfo : public UnitCmdInfo
{
public:
    DroidRepairInfo(PblHandle<const GameObject> who, PblHandle<const GameObject> owner);
};
