#include "HUDObjectiveList.h"
#include "PebbleFL/Common/PblList.h"

extern bool netIsOneWorld;

namespace ObjectiveManager
{

// GLOBAL: SWBF2 0x007ebbcc
PblList<HUD::ObjectiveList> gHostObjectiveLists;
// GLOBAL: SWBF2 0x007ebbf8
PblList<HUD::ObjectiveList> gClientObjectiveLists;
// GLOBAL: SWBF2 0x01eaed58
PblList<HUD::ObjectiveList>* gObjectiveLists;

// FUNCTION: SWBF2 0x005f6940
void CleanList(PblList<HUD::ObjectiveList>& list)
{
    while (list.GetCount() > 0)
    {
        PblList<HUD::ObjectiveList>::Node* pNode = list.GetFirst();
        if (pNode->_pObject)
        {
            delete pNode->_pObject;
        }
        list.Remove(pNode);
    }
}

// FUNCTION: SWBF2 0x005f6990
void Init()
{
    CleanList(gClientObjectiveLists);
    CleanList(gHostObjectiveLists);
    gObjectiveLists = &gHostObjectiveLists;
}

// FUNCTION: SWBF2 0x005f69d0
void CreateClient()
{
    if (!netIsOneWorld)
    {
        CleanList(gClientObjectiveLists);
    }
}

// FUNCTION: SWBF2 0x005f69f0
void EnterClient()
{
    if (!netIsOneWorld)
    {
        gObjectiveLists = &gClientObjectiveLists;
    }
}

// FUNCTION: SWBF2 0x005f6a10
void LeaveClient()
{
    if (!netIsOneWorld)
    {
        gObjectiveLists = &gHostObjectiveLists;
    }
}

}

namespace ObjectiveManager {

// FUNCTION: SWBF2 0x005f69b0
void DestroyAll()
{
    CleanList(gClientObjectiveLists);
    CleanList(gHostObjectiveLists);
}
}

namespace ObjectiveManager {

// FUNCTION: SWBF2 0x005f6760
void AddObjectiveList(HUD::ObjectiveList* pList)
{
    PblList<HUD::ObjectiveList>* pLists = ObjectiveManager::gObjectiveLists;
    if (!pLists)
    {
        return;
    }

    for (PblList<HUD::ObjectiveList>::Itor it = pLists->Begin(); it != pLists->End(); ++it)
    {
        if (it->_viewportId == pList->_viewportId)
        {
            return;
        }
    }

    pLists->Add(new PblList<HUD::ObjectiveList>::Node, pList);
}

}
