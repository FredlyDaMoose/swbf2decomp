#include <math.h>

#include "AIUtil.h"
#include "CommandFsm.h"
#include "Controllable.h"
#include "Controller.h"
#include "EntitySoldier.h"
#include "EventInfo.h"
#include "GameObject.h"
#include "UnitController.h"
#include "UnitThreatManager.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

#include "AIGoalManager.h"
#include "ListPool.h"
#include "TreeGrid.h"
#include "PebbleFL/Common/PblHeap.h"
#include "NetGame.h"
#include "TeamManager.h"

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

int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
                      const GameObject* pObject, AI::AIAffil eAffil);
}

namespace AudibleManager
{
extern GameObject** sPVRect;
void Initialize();
void UpdateAudience(Controller* pController);
}

// FUNCTION: SWBF2 0x00447fc0
void AudibleManager::Initialize()
{
    sPVRect = new GameObject*[1200];
}

// FUNCTION: SWBF2 0x00447ff0
void AudibleManager::UpdateAudience(Controller* pController)
{
    const Controllable* pMe = pController->GetMe();
    const GameObject* pObject = pMe->GetGameObject();
    float fRadius = pMe->GetSoundRadius();
    if (pController->mForceAudible)
    {
        fRadius = 100.0f;
    }
    pController->mForceAudible = false;
    if (fRadius > 30.0f)
    {
        fRadius = 30.0f;
    }

    if (!pObject->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
        static_cast<const EntitySoldier*>(pObject)->GetInvisibilityFactor() < 0.1f)
    {
        if (fRadius < 8.0f)
        {
            fRadius = 8.0f;
        }
    }
    if (fRadius <= 0.0f)
    {
        return;
    }

    const PblVector3 vPos = pMe->GetMatrix().GetTranslation();
    const int iCount = TeamManager::GetObjectsInRange(vPos, fRadius, sPVRect, 1200, pObject, AI::AFFIL_ENEMY);
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pListener = sPVRect[i];
        const PblList<Controllable>& list = pListener->GetControllableList();
        for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
        {
            if (!UnitControllerPtr(it->mCtrl))
            {
                continue;
            }

            const PblVector3& vListener = pListener->GetMatrix().GetTranslation();
            PblVector3 vDelta;
            vDelta.x = vPos.x - vListener.x;
            vDelta.y = vPos.y - vListener.y;
            vDelta.z = vPos.z - vListener.z;
            const PblVector3 vDir = AIUtil::GetSafeNormal(vDelta);
            if (fabs(vDir.x * gVectorUnitY.x + vDir.y * gVectorUnitY.y + vDir.z * gVectorUnitY.z) >= 0.87f &&
                vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z >= 12.25f)
            {
                continue;
            }

            UnitController* pUnit = static_cast<UnitController*>(it->mCtrl);
            AI::UnitThreatManager& threats = *reinterpret_cast<AI::UnitThreatManager*>(pUnit->mThreatManager);
            if (vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z > 9.0f)
            {
                threats.AudibleResult(pController->mOwner->GetGameObject());
                if (!pUnit->IsNewCommand())
                {
                    AI::EventInfo info(AI::EVT_Audible_CanHearEnemy, pController->mOwner->GetGameObject());
                    pUnit->SendEvent(&info);
                }
            }
            else
            {
                threats.ForceThreat(pController->mOwner->GetGameObject());
                AI::EventInfo info(AI::EVT_Vision_CanSeeEnemy, pController->mOwner->GetGameObject());
                pUnit->SendEvent(&info);
            }
        }
    }
}

namespace AudibleManager {

// FUNCTION: SWBF2 0x00447fe0
void Reset()
{
    sPVRect = 0;
}
}

namespace AudibleManager
{

// GLOBAL: SWBF2 0x01e302e8
GameObject** sPVRect;
}
