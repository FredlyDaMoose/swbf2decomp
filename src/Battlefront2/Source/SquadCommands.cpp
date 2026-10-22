#include "SquadCommands.h"
#include "Character.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PebbleFL/Common/PblRandom.h"
#include "AIUtil.h"
#include "EventInfo.h"
#include "EventManager_Character.h"
#include "NetGame.h"
#include "UnitController.h"
#include "EntitySoldier.h"
#include "MessageDisplay.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include <math.h>
#include <stdlib.h>

namespace SquadCommands
{

// FUNCTION: SWBF2 0x006507a0
bool IsSquadCommand(int iCmd) { return (unsigned int)(iCmd - 13) <= 1; }
}

namespace AI
{
enum VOType { VOT_SC_GetIn = 28, VOT_SC_GetInResponseRepeat = 30,
              VOT_SC_GetOut = 31, VOT_SC_GetOutResponse = 32 };
enum AIAffil { AFFIL_ENEMY = 1, AFFIL_NEUTRAL = 2, AFFIL_ALLY = 4,
               AFFIL_ALLYNEUTRAL = 6, AFFIL_ALL = 7 };
namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

namespace ReserveManager
{
int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);
int GetPlayerOccupiedSlots(const GameObject* pObj);
const GameObject* __fastcall GetFirstReserver(const GameObject* pObj);
}
namespace TeamManager
{
int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects,
                      int iMax, const GameObject* pObject, AI::AIAffil eAffil);
}

namespace SquadCommands
{
int VehicleDistCallback(const void* pA, const void* pB);
void VehicleGetIn(GameObject* pObject, Character* pCharacter);
void VehicleGetOut(GameObject* pObject, Character* pCharacter);
void VehicleCommand(GameObject* pObject, Character* pCharacter);
}

// FUNCTION: SWBF2 0x00650020
int SquadCommands::VehicleDistCallback(const void* pA, const void* pB)
{
    const float a = ((const float*)pA)[1];
    const float b = ((const float*)pB)[1];
    if (a > b)
    {
        return 1;
    }
    return (b <= a) - 1;
}

// FUNCTION: SWBF2 0x00650720
void SquadCommands::VehicleCommand(GameObject* pObject, Character* pCharacter)
{
    const PblList<Controllable>& list = pObject->GetControllableList();
    if (list.GetCount() <= 1)
    {
        return;
    }
    PblList<Controllable>::Node* pNode = list.GetFirst();
    while (pNode->_pObject && pNode->_pObject->mCtrl)
    {
        pNode = pNode->_pNext;
    }
    if (pNode->_pObject)
    {
        SquadCommands::VehicleGetIn(pObject, pCharacter);
    }
    else
    {
        SquadCommands::VehicleGetOut(pObject, pCharacter);
    }
}

// FUNCTION: SWBF2 0x00650570
void SquadCommands::VehicleGetOut(GameObject* pObject, Character* pCharacter)
{
    AI::VOHelper::Play(AI::VOT_SC_GetOut, AIUtil::FindFirstPlayerInVehicle(pObject), 0,
                       AIUtil::FindFirstAIInVehicle(pObject), 0.0f);
    const PblList<Controllable>& list = pObject->GetControllableList();
    for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
    {
        UnitController* pUnit = UnitControllerPtr((Controllable*)it);
        if (!pUnit)
        {
            continue;
        }
        AI::EventInfo info(AI::EVT_SC_GetOutOfVehicle, pObject);
        pUnit->SendEvent(&info);
        const GameObject* pSpeaker = pUnit->mOwner->mCharacter->mUnit->GetGameObject();
        AI::VOHelper::Play(AI::VOT_SC_GetOutResponse, pSpeaker, 0, pObject,
                           AIUtil::GetRandomFloat(0.5f, 1.2f) + GameLoop::GetMissionTime());

        AI::EventType eType = AI::EVT_SC_GetOutOfVehicle;
        Controllable* pVehicle = pObject->GetControllable();
        if (!NetGame::IsEnabled() || !netEnabledNext || !netClientActive)
        {
            EventManager::CharacterEvent::mIssueAICommand.Trigger(*pVehicle->mCharacter, eType);
        }

        const unsigned short* pText = RedLocalizeDB::Find(0x7e3c4096u, false);
        for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); ++i)
        {
            if (NetGame::GetLocalPlayer(i) == pCharacter)
            {
                MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)->AddMessage(pText, RedColor::CYAN);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00650050
void SquadCommands::VehicleGetIn(GameObject* pObject, Character* pCharacter)
{
    const int iOpenSlots = ReserveManager::GetOpenReservableSlots(pObject, pObject->GetTeam(), RT_BOARD);
    if (iOpenSlots <= 0)
    {
        if (pObject->GetReservableSlots(RT_BOARD) - ReserveManager::GetPlayerOccupiedSlots(pObject) > 0)
        {
            const GameObject* pReserver = ReserveManager::GetFirstReserver(pObject);
            GameObject* pPlayer = AIUtil::FindFirstPlayerInVehicle(pObject);
            AI::VOHelper::Play(AI::VOT_SC_GetInResponseRepeat, pReserver, pPlayer, pObject,
                               AIUtil::GetRandomFloat(0.2f, 0.6f) + GameLoop::GetMissionTime());
        }
        return;
    }

    const PblMatrix& matrix = pObject->GetMatrix();
    const PblVector3& vVehicle = pObject->GetPosition();
    PblVector3 vSearch(vVehicle.x + matrix.m[2][0] * 15.000001f,
                       vVehicle.y + matrix.m[2][1] * 15.000001f,
                       vVehicle.z + matrix.m[2][2] * 15.000001f);
    GameObject* apNearby[30];
    int iCount = TeamManager::GetObjectsInRange(vSearch, 50.0f, apNearby, 30, pObject, AI::AFFIL_ALLY);
    for (int i = 0; i < iCount; ++i)
    {
        Controllable* pControl = apNearby[i]->GetControllable();
        if (!pControl || !pControl->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
            !UnitControllerPtr(pControl))
        {
            apNearby[i] = apNearby[--iCount];
            --i;
        }
    }
    if (!iCount)
    {
        return;
    }

    struct Candidate
    {
        GameObject* pObject;
        float fScore;
    };
    Candidate aCandidates[30];
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pCandidate = apNearby[i];
        aCandidates[i].pObject = pCandidate;
        if (pObject->GetControllable()->mReticuleTarget[0] == pCandidate)
        {
            const PblVector3& vTarget = pCandidate->GetPosition();
            const GameObject* pReticule = pObject->GetControllable()->mReticuleTarget[0];
            const PblVector3& vReticule = pReticule->GetPosition();
            const float dx = vReticule.x - vTarget.x;
            const float dy = vReticule.y - vTarget.y;
            const float dz = vReticule.z - vTarget.z;
            if (dx * dx + dy * dy + dz * dz < 3600.0f)
            {
                aCandidates[i].fScore = 0.0f;
                continue;
            }
        }

        const PblVector3& vCandidate = pCandidate->GetPosition();
        const float dx = vCandidate.x - vVehicle.x;
        const float dy = vCandidate.y - vVehicle.y;
        const float dz = vCandidate.z - vVehicle.z;
        const float fDistance = sqrtf(dx * dx + dy * dy + dz * dz);
        const float fInverseDistance = 1.0f / fDistance;
        const float fForward = matrix.m[2][0] * (dx * fInverseDistance) +
                               matrix.m[2][1] * (dy * fInverseDistance) +
                               matrix.m[2][2] * (dz * fInverseDistance);
        aCandidates[i].fScore = (1.5f - fForward) * fDistance;
    }
    qsort(aCandidates, iCount, sizeof(aCandidates[0]), SquadCommands::VehicleDistCallback);

    int iAccepted = 0;
    for (int i = 0; i < iCount && iAccepted < iOpenSlots; ++i)
    {
        UnitController* pUnit = UnitControllerPtr(aCandidates[i].pObject->GetControllable());
        AI::EventInfo info(AI::EVT_SC_GetIntoVehicle, pObject);
        if (pUnit->SendEvent(&info) == AI::EVT_Handled)
        {
            ++iAccepted;
        }
    }
    if (!iAccepted)
    {
        return;
    }

    AI::VOHelper::Play(AI::VOT_SC_GetIn, AIUtil::FindFirstPlayerInVehicle(pObject), 0,
                       pObject, 0.0f);
    AI::EventType eType = AI::EVT_SC_GetIntoVehicle;
    Controllable* pVehicle = pObject->GetControllable();
    if (!NetGame::IsEnabled() || !netEnabledNext || !netClientActive)
    {
        EventManager::CharacterEvent::mIssueAICommand.Trigger(*pVehicle->mCharacter, eType);
    }

    const unsigned short* pText = RedLocalizeDB::Find(0xf00699adu, false);
    for (unsigned int i = 0; i < NetGame::GetNumLocalPlayers(); ++i)
    {
        if (NetGame::GetLocalPlayer(i) == pCharacter)
        {
            MessageDisplay::Instance(i, MessageDisplay::DISPLAY_SYSTEM_MSG)->AddMessage(pText, RedColor::CYAN);
        }
    }
}

// FUNCTION: SWBF2 0x00650770
SquadCommands::SquadCommandOrder::SquadCommandOrder(SquadCommandType eOrder, Character* pWhoGaveIt,
                                                    PblHandle<GameObject> obj)
    : mWhoGaveIt(pWhoGaveIt), mWhatOrder(eOrder), mGameObj(obj)
{
    mTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x006507b0
void SquadCommands::AIShoutOrder(SquadCommandOrder& order, float fDelay)
{
    if (order.mWhatOrder == SC_FOLLOW && order.mWhoGaveIt->mVehicle)
    {
        order.mWhatOrder = SC_BOARDVEHICLE;
        order.mGameObj = PblHandle<GameObject>(static_cast<Trackable*>(order.mWhoGaveIt->mVehicle)->GetGameObject());
    }

    (void)fDelay;
    PblRandom::GetGlobalInt();
}
