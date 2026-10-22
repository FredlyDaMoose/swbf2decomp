#include "AIVisRegion.h"

#include <stdio.h>
#include <string.h>

#include "CollisionObject.h"
#include "Controllable.h"
#include "Controller.h"
#include "GameObject.h"
#include "TriggerRegionManager.h"

namespace
{

    Controllable* GetControllable(GameObject* pObject)
    {
        return pObject->GetControllable();
    }

    const PblMatrix& GetMatrix(GameObject* pObject)
    {
        return static_cast<CollisionObject&>(*pObject).GetMatrix();
    }
}

// GLOBAL: SWBF2 0x007eb76c
// SYNTHETIC: SWBF2 0x00402230
// `dynamic initializer for 'AIVisRegionManager::sRegions''
// SYNTHETIC: SWBF2 0x00766d60
// `dynamic atexit destructor for 'AIVisRegionManager::sRegions''
ListPool<AIVisRegion*, int> AIVisRegionManager::sRegions(20);

// GLOBAL: SWBF2 0x01e2fdd4
int AIVisRegionManager::mNextUpdateRegion;

// FUNCTION: SWBF2 0x004433f0
float AIVisRegion::GetVisStand()
{
    return mEnabled ? mVisStand : 1.0f;
}

// FUNCTION: SWBF2 0x00443410
float AIVisRegion::GetVisCrouch()
{
    return mEnabled ? mVisCrouch : 1.0f;
}

// FUNCTION: SWBF2 0x00443430
AIVisRegionManager::AIVisRegionManager()
    : RedRegionFactory("aivis")
{
}

// FUNCTION: SWBF2 0x00443480
void AIVisRegionManager::Reset()
{
    sRegions.Clear();
    mNextUpdateRegion = 0;
}

// FUNCTION: SWBF2 0x00443570
void* AIVisRegionManager::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    AIVisRegion* pRegion = new AIVisRegion(desc, 1.0f, 1.0f);
    if (pRegion)
    {
        ProcessRegionName(pcName, pRegion);
        sRegions.Append(pRegion);
    }
    return pRegion;
}

// FUNCTION: SWBF2 0x00443620
void AIVisRegionManager::ReadParameter(const char* pcRegionName, const char* pcParameter,
                                       unsigned int uiIndex, unsigned int uiCount,
                                       const char* pcValue, unsigned int uiValueIndex,
                                       void* pDesc) const
{
    AIVisRegion* pRegion = (AIVisRegion*)pDesc;
    float* pfValue;
    if (_stricmp(pcParameter, "stand") == 0)
    {
        pfValue = &pRegion->mVisStand;
    }
    else if (_stricmp(pcParameter, "crouch") == 0)
    {
        pfValue = &pRegion->mVisCrouch;
    }
    else
    {
        return;
    }
    sscanf(pcValue, "%f", pfValue);
}

// FUNCTION: SWBF2 0x004434a0
void AIVisRegionManager::UpdateNextRegion()
{
    if (!sRegions.GetLength())
    {
        return;
    }

    AIVisRegion* pRegion = sRegions[mNextUpdateRegion];
    GameObject* apObjects[40];
    int iCount = TriggerRegionManager::GetObjectsInRegion(pRegion, apObjects, 40);
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObject = apObjects[i];
        if (pObject && GetControllable(pObject))
        {
            Controller* pController = GetControllable(pObject)->mCtrl;
            if (pController && pRegion->IsPointInside(*(const PblVector3*)&GetMatrix(pObject).m[3][0]))
            {
                pController->SetRegionVisFactor(pRegion->GetVisCrouch(), pRegion->GetVisStand());
            }
        }
    }
    mNextUpdateRegion = (mNextUpdateRegion + 1) % sRegions.GetLength();
}
