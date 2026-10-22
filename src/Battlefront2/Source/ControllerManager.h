#pragma once

#include "PebbleFL/Common/PblList.h"

class Controller;
class PblVector3;
class PlayerController;
class UnitController;
namespace AI { class EventInfo; }

template <class TYPE, class KEY> class PblOrderedList;

namespace ControllerManager
{

void Reset();

void EnterClient();
void LeaveClient();

void Init();
void AddController(Controller* pCtrl);
void RemoveController(Controller* pCtrl);
void RequestControllerUpdateASAP(UnitController* pCtrl);
void BroadcastEvent(AI::EventInfo* pInfo, int iMaxCount);

void LogVODeath(int iVictimTeam, int iKillerTeam, const PblVector3& vPos);

void UpdatePilotSelf();
void UpdateCrowd(Controller* pCtrl);
void DoPeriodicUpdate(float fDeltaTime);
void Update(float fDeltaTime);

}
