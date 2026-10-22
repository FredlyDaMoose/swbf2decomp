#include <stdio.h>

#include "ODFPropertySet.h"
#include "HUDElementMap.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "HUDObjectiveList.h"
#include "PebbleFL/Common/PblList.h"

namespace ObjectiveManager
{
extern PblList<HUD::ObjectiveList>* gObjectiveLists;
}

namespace ObjectiveManager { void AddObjective(unsigned int uiTextHashID, unsigned int uiPopupTextID, int iTeam); }

// FUNCTION: SWBF2 0x005f67f0
void ObjectiveManager::AddObjective(unsigned int uiTextHashID, unsigned int uiPopupTextID, int iTeam)
{
    if (!ObjectiveManager::gObjectiveLists)
    {
        return;
    }
    for (PblList<HUD::ObjectiveList>::Itor it = ObjectiveManager::gObjectiveLists->Begin();
         it != ObjectiveManager::gObjectiveLists->End(); ++it)
    {
        it->AddObjective(uiTextHashID, uiPopupTextID, iTeam);
    }
}

namespace ObjectiveManager { void SetObjectiveState(unsigned int uiTextHashID, int iState); }

// FUNCTION: SWBF2 0x005f6840
void ObjectiveManager::SetObjectiveState(unsigned int uiTextHashID, int iState)
{
    HUD::Objective::ObjectiveState state = (HUD::Objective::ObjectiveState)iState;
    for (PblList<HUD::ObjectiveList>::Itor it = ObjectiveManager::gObjectiveLists->Begin();
         it != ObjectiveManager::gObjectiveLists->End(); ++it)
    {
        HUD::ObjectiveList* pList = it;
        if (state == HUD::Objective::ACTIVE && pList->GetState(uiTextHashID) != state)
        {
            HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(pList->GetViewport());
            if (pMap && pMap->GetMode() == HUD::ElementMap::mode_Small)
            {
                HUD::Event event;
                event.mClass = HUD::GameEvents::gPlayerEvents[pList->GetViewport()].objectivesUpdated;
                event.mData.boolValue = true;
                event.SendDelayed(0.25f);
                HUD::Event eventOff;
                eventOff.mClass = HUD::GameEvents::gPlayerEvents[it->GetViewport()].objectivesUpdatedDisable;
                eventOff.mData.boolValue = true;
                eventOff.SendDelayed(6.0f);
            }
        }
        it->SetState(uiTextHashID, state);
    }
}

namespace ODFPropertySet
{

// GLOBAL: SWBF2 0x0078a70c
char* sScanfFormats[10] = {
    "%s", "%f", "%c", "%c", "%hd", "%hu", "%d", "%u", "%ld", "%lu",
};

// FUNCTION: SWBF2 0x005f6a50
bool SetProperty(const ODFProperty* const pProperties, unsigned int uNumProperties,
                 const void* const pObject, unsigned int uProperty,
                 const char* const pszValue)
{
    for (unsigned int i = 0; i < uNumProperties; ++i)
    {
        if (pProperties[i].id == uProperty)
        {
            sscanf(pszValue, sScanfFormats[pProperties[i].type - 1],
                   (char*)pObject + pProperties[i].memberOffset);
            return true;
        }
    }
    return false;
}

}
