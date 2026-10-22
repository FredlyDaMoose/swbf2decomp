#include "VehicleManager.h"
#include "Controllable.h"
#include "AIUtil.h"
#include "Character.h"
#include "CommandFlyer.h"
#include "EntityBuildingArmed.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PathFinder.h"
#include "Team.h"
#include "UnitController.h"
#include <stdlib.h>
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class Character;
class FlagItem;

namespace
{
    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    GameObject* HandleObject(PblHandle<GameObject>& handle)
    {
        if (handle.mObject && GameObjectHandleId(handle.mObject) != handle.mSavedHandleId)
        {
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return handle.mObject;
    }

    const PblList<Controllable>& ControllableList(const GameObject* pObject)
    {
        return pObject->mControllableList;
    }

    const Controllable* GetControllable(const GameObject* pObject)
    {
        return pObject->GetControllable();
    }

    const FlagItem* CarriedFlag(const Character* pCharacter)
    {
        return pCharacter->mCarriedFlagPtr;
    }

    const PblVector3& GameObjectPosition(const GameObject* pObject)
    {
        return static_cast<const CollisionObject*>(pObject)->GetPosition();
    }
}

// GLOBAL: SWBF2 0x007ec3cc
// SYNTHETIC: SWBF2 0x00406db0
// `dynamic initializer for 'sHostVehicles''
// SYNTHETIC: SWBF2 0x00769700
// `dynamic atexit destructor for 'sHostVehicles''
ListPool<PblHandle<GameObject>, int> sHostVehicles(1200);

// GLOBAL: SWBF2 0x007ec3dc
// SYNTHETIC: SWBF2 0x00406e60
// `dynamic initializer for 'sClientVehicles''
// SYNTHETIC: SWBF2 0x00769730
// `dynamic atexit destructor for 'sClientVehicles''
ListPool<PblHandle<GameObject>, int> sClientVehicles(1200);

// GLOBAL: SWBF2 0x01eafcf4
ListPool<PblHandle<GameObject>, int>* VehicleManager::sVehicles;

// GLOBAL: SWBF2 0x01faa428
float sNotifyRadius;

// GLOBAL: SWBF2 0x01eafb43
bool sGameObjSortFlagPriority;

// GLOBAL: SWBF2 0x01f9a260
PblVector3 sGameObjSortFrom;

// FUNCTION: SWBF2 0x0066dca0
void VehicleManager::Initialize()
{
    sHostVehicles.Clear();
    sClientVehicles.Clear();
    sVehicles = &sHostVehicles;
    sNotifyRadius = 50.0f;
}

// FUNCTION: SWBF2 0x0066dce0
void VehicleManager::EnterClient()
{
    sVehicles = &sClientVehicles;
}

// FUNCTION: SWBF2 0x0066dcf0
void VehicleManager::LeaveClient()
{
    sVehicles = &sHostVehicles;
}

// FUNCTION: SWBF2 0x0066dd00
void VehicleManager::Add(GameObject* pObject)
{
    if (!pObject)
    {
        return;
    }

    PblHandle<GameObject> handle;
    handle.mObject = pObject;
    handle.mSavedHandleId = GameObjectHandleId(pObject);
    for (int i = 0; i < sVehicles->GetLength(); ++i)
    {
        if (HandleObject(handle) == HandleObject((*sVehicles)[i]))
        {
            return;
        }
    }
    sVehicles->Append(handle);
}

// FUNCTION: SWBF2 0x0066ddc0
void VehicleManager::Remove(GameObject* pObject)
{
    if (!pObject)
    {
        return;
    }

    sVehicles->RemoveObject(PblHandle<GameObject>(pObject));
}

// TEMPLATE: SWBF2 0x0066e460
// ListPool<PblHandle<GameObject>,int>::RemoveObject
template void ListPool<PblHandle<GameObject>, int>::RemoveObject(const PblHandle<GameObject>&);

// FUNCTION: SWBF2 0x0066ddf0
bool VehicleManager::sIsFull(const GameObject* pObject)
{
    for (PblList<Controllable>::Itor it(ControllableList(pObject)); (Controllable*)it; ++it)
    {
        const Controllable* pSeat = it;
        PilotType type = pSeat->mPilotType;
        if (type != PILOT_VEHICLE && type != PILOT_VEHICLESELF)
        {
            continue;
        }
        if (type == PILOT_SELF || (type == PILOT_VEHICLESELF && pSeat->mPilot == pSeat) || !pSeat->mPilot)
        {
            if (type != PILOT_SELF && type != PILOT_VEHICLESELF)
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0066de50
int VehicleManager::sGameObjSortByDistance(const void* pA, const void* pB)
{
    const GameObject* pObjectA = *(const GameObject* const*)pA;
    const GameObject* pObjectB = *(const GameObject* const*)pB;
    if (!pObjectA)
    {
        return 1;
    }
    if (!pObjectB)
    {
        return -1;
    }

    if (sGameObjSortFlagPriority)
    {
        const Controllable* pControllableA = GetControllable(pObjectA);
        if (pControllableA && pControllableA->mCharacter && CarriedFlag(pControllableA->mCharacter))
        {
            return 1;
        }
        const Controllable* pControllableB = GetControllable(pObjectB);
        if (pControllableB && pControllableB->mCharacter && CarriedFlag(pControllableB->mCharacter))
        {
            return -1;
        }
    }

    const PblVector3& posA = GameObjectPosition(pObjectA);
    float fDistA = (posA.x - sGameObjSortFrom.x) * (posA.x - sGameObjSortFrom.x) +
                   (posA.y - sGameObjSortFrom.y) * (posA.y - sGameObjSortFrom.y) +
                   (posA.z - sGameObjSortFrom.z) * (posA.z - sGameObjSortFrom.z);
    const PblVector3& posB = GameObjectPosition(pObjectB);
    float fDistB = (posB.x - sGameObjSortFrom.x) * (posB.x - sGameObjSortFrom.x) +
                   (posB.y - sGameObjSortFrom.y) * (posB.y - sGameObjSortFrom.y) +
                   (posB.z - sGameObjSortFrom.z) * (posB.z - sGameObjSortFrom.z);
    if (fDistB > fDistA)
    {
        return -1;
    }
    return fDistA > fDistB;
}

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
}

namespace TeamManager
{

int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);
}

// GLOBAL: SWBF2 0x007ec558
int sVehicleUpdateBucket = 30;

// FUNCTION: SWBF2 0x0066df90
void VehicleManager::Update()
{
    if (sVehicleUpdateBucket == 30)
    {
        sVehicleUpdateBucket = 0;
    }
    else
    {
        ++sVehicleUpdateBucket;
    }

    for (int i = 0; i < sVehicles->GetLength(); ++i)
    {
        if ((int)(*sVehicles)[i].GetHandleId() % 30 != sVehicleUpdateBucket)
        {
            continue;
        }

        GameObject* pObject = (*sVehicles)[i];
        if (pObject->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed) &&
            static_cast<EntityBuildingArmed*>(pObject)->mNoAIBoard)
        {
            continue;
        }
        if (pObject->IsRtti(CommandFlyer::rttiHashCommandFlyer) &&
            static_cast<CommandFlyer*>(pObject)->mbHasTakenOffBefore)
        {
            continue;
        }
        if (sIsFull(pObject))
        {
            continue;
        }

        const PblVector3& vVelocity = pObject->GetVelocity();
        if (vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z > 9.0f)
        {
            continue;
        }

        if (!pObject->GetEntity()->IsRtti(EntityBuilding::rttiHashEntityBuilding))
        {
            const Obstacle* pBarrier = AIUtil::GetGameObjectBarrier(pObject);
            if (!PathFinder::IsPointValid(*(const PblVector3*)&pObject->GetMatrix().m[3][0], 2.0f, 1, 0,
                                          &pBarrier, 1))
            {
                continue;
            }
        }
        if (pObject->GetGameObjectClass()->GetVehicleType() != GameObjectClass::VEHICLE_UNKNOWN)
        {
            const Obstacle* pBarrier = AIUtil::GetGameObjectBarrier(pObject);
            int eProhibit = pObject->GetGameObjectClass()->mAIType;
            if (!PathFinder::IsPointValid(*(const PblVector3*)&pObject->GetMatrix().m[3][0],
                                          AIUtil::GetCollisionRadius(pObject), eProhibit, 0, &pBarrier, 1))
            {
                continue;
            }
        }

        GameObject* apObjects[30];
        int iCount = TeamManager::GetObjectsInRange(*(const PblVector3*)&pObject->GetMatrix().m[3][0],
                                                    sNotifyRadius, apObjects, 30, pObject,
                                                    AI::AFFIL_ALLYNEUTRAL);
        if (iCount > 1)
        {
            sGameObjSortFlagPriority = pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer);
            sGameObjSortFrom = *(const PblVector3*)&pObject->GetMatrix().m[3][0];
            qsort(apObjects, iCount, sizeof(GameObject*), sGameObjSortByDistance);
        }

        for (int j = 0; j < iCount; ++j)
        {
            GameObject* pOther = apObjects[j];
            if (pObject->IsReservedForPlayer() &&
                Team::GetPtr(pOther->GetTeam())->GetAffiliation(AIUtil::IsPlayerTeam(1) ? 1 : 2) >= 0)
            {
                continue;
            }

            for (PblList<Controllable>::Itor it(pOther->GetControllableList()); (Controllable*)it; ++it)
            {
                Controllable* pSeat = it;
                const ControllableClass* pClass = pSeat->GetControllableClass();
                Controller* pController = pSeat->mCtrl;
                Character* pCharacter = pSeat->mCharacter;
                if (pCharacter && pCharacter->GetVehicleBlockTime() > GameLoop::GetMissionTime())
                {
                    continue;
                }
                if (!pController || !pClass || !pClass->mCanEnterVehicles)
                {
                    continue;
                }

                if (pObject->mOwningTeam &&
                    Team::GetPtr(pObject->mOwningTeam)->GetAffiliation(pOther->GetTeam()) <= 0)
                {
                    if (!pObject->GetControllable()->GetPilot())
                    {
                        continue;
                    }
                    if (pObject->GetControllable()->GetPilot()->GetPlayerId() < 0)
                    {
                        continue;
                    }
                }

                if (!AIUtil::IsEndGame(0) && pController->IsRtti(UnitController::rttiHashUnitController))
                {
                    AI::EventInfo info(AI::EVT_EmptyVehicle, pObject);
                    static_cast<UnitController*>(pController)->SendEvent(&info);
                }
            }
        }
    }
}
