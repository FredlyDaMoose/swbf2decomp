#pragma once

#include "Fsm.h"
#include "CmdInfo.h"

namespace AI { class CTFGoal; }
namespace ControllerManager { void UpdatePilotSelf(); }
class Controller;
namespace AudibleManager { void UpdateAudience(Controller* pController); }
class GameObject;
namespace AIUtil { void TellAIToExit(GameObject* pObject); }
class UnitController;
namespace VisionManager { bool VisionTest(UnitController* pController, GameObject* pTarget); }

// VTABLE: SWBF2 0x007981b0
class CommandFsm : public Fsm
{
public:
    CommandFsm();

    virtual void FSMUpdate(float dt, bool b);

    int GetCurCmdType() { return mCurCmd.mWhat; }

protected:
    virtual ~CommandFsm();

    virtual void ApplyNextCommand();
    virtual void ApplyNextState() = 0;

public:
    virtual void ForceCommand(const CmdInfo& cmd);
    virtual void QueueCommand(const CmdInfo& cmd);
    virtual void RemoveQueuedCommands(int what);
    virtual void PrependQueueCommand(const CmdInfo& cmd);
    virtual void PrependQueueCommands(const CmdInfo& cmdA, const CmdInfo& cmdB);

protected:

    friend class AI::CTFGoal;

    friend void ControllerManager::UpdatePilotSelf();

    friend void AudibleManager::UpdateAudience(Controller* pController);

    friend void AIUtil::TellAIToExit(GameObject* pObject);

    friend bool VisionManager::VisionTest(UnitController* pController, GameObject* pTarget);
    bool IsNewCommand();

    CmdInfo mCurCmd;
    CmdInfo mCmdQueue[5];
    int mCmdCount;
    bool mForceCommand;
    bool mCommandDone;
};

 
