#include "TriggerRegionManager.h"

#include <float.h>
#include <string.h>

#include "CollisionObject.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Util/RedRegion.h"

class CollisionManager
{
public:
    static int GetObjectsInRegion(const RedRegion& region, GameObject** const ppObjects, int iMaxObjects,
                                  const GameObject* pIgnore, int iMask);
};

static CollisionObject* GetCollisionObject(GameObject* pObject)
{
    return &static_cast<CollisionObject&>(*pObject);
}

static Damageable* GetDamageable(GameObject* pObject)
{
    return &static_cast<Damageable&>(*pObject);
}

// GLOBAL: SWBF2 0x01eafcf0
TriggerRegionManager* TriggerRegionManager::sInstance;

// FUNCTION: SWBF2 0x0065ce00
TriggerRegionManager::TriggerRegionManager()
{
    ActivateThread(-10, PER_TURN);
    memset(mTriggerRegions, 0, sizeof(mTriggerRegions));
    mSinkHeight = -FLT_MAX;
    mLandingRegionsExist = false;
}

// SYNTHETIC: SWBF2 0x0065ce70
// TriggerRegionManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0065cea0
TriggerRegionManager::~TriggerRegionManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0065cef0
bool TriggerRegionManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = new TriggerRegionManager;
    return true;
}

// FUNCTION: SWBF2 0x0065cf60
bool TriggerRegionManager::Destroy()
{
    if (!sInstance)
    {
        return false;
    }
    delete sInstance;
    sInstance = 0;
    return true;
}

// FUNCTION: SWBF2 0x0065cf80
void TriggerRegionManager::CreateClient()
{
    if (sInstance && NetGame::IsOneWorld())
    {
        sInstance->DeactivateThread();
        sInstance->ActivateThread(-10, PER_TURN);
    }
}

// FUNCTION: SWBF2 0x0065cfb0
void TriggerRegionManager::DestroyClient()
{
}

void TriggerRegionManager::InitialUpdate()
{
}

// FUNCTION: SWBF2 0x0065cfc0
int TriggerRegionManager::GetObjectsInRegion(const RedRegion* pRegion, GameObject** const ppObjects,
                                             int iMaxObjects)
{
    if (!pRegion)
    {
        return 0;
    }
    return CollisionManager::GetObjectsInRegion(*pRegion, ppObjects, iMaxObjects, 0,
                                                COLL_ITEM | COLL_SOFT | COLL_RIGID);
}

// FUNCTION: SWBF2 0x0065cfe0
void TriggerRegionManager::TriggerDeath(GameObject* pObject)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    GetDamageable(pObject)->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
}

// FUNCTION: SWBF2 0x0065d050
void TriggerRegionManager::ApplyTempDeathRegion(const RedRegion* pRegion)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    GameObject* apObjects[64];
    int iCount = GetObjectsInRegion(pRegion, apObjects, 64);
    for (int i = 0; i < iCount; ++i)
    {
        TriggerDeath(apObjects[i]);
    }
}

// FUNCTION: SWBF2 0x0065d0c0
bool TriggerRegionManager::Update(float)
{
    mLandingRegionsExist = false;
    for (int i = 0; i < 32; ++i)
    {
        TriggerRegionData& data = mTriggerRegions[i];
        if (!data.mRegion)
        {
            continue;
        }
        if (data.mType == TRIGGER_LANDING)
        {
            mLandingRegionsExist = true;
        }

        GameObject* apObjects[64];
        int iCount = GetObjectsInRegion(data.mRegion, apObjects, 64);
        for (int j = 0; j < iCount; ++j)
        {
            switch (data.mType)
            {
            case TRIGGER_DEATH:
            case TRIGGER_SINK:
                TriggerDeath(apObjects[j]);
                break;
            case TRIGGER_LANDING:
                if (((Entity*)apObjects[j])->IsRtti(EntityFlyer::rttiHashEntityFlyer))
                {
                    CollisionObject* pCollision = GetCollisionObject(apObjects[j]);
                    ((EntityFlyer*)apObjects[j])->SetInLandingRegion(data.mRegion->GetIntersectionFactor(
                        pCollision->GetCollisionCenter(), pCollision->GetCollisionRadius()));
                }
                break;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0065d1e0
void TriggerRegionManager::AddDeathRegion(const char* pcName)
{
    if (!NetGame::IsEnabled() || NetGame::IsHost())
    {
        AddRegion(pcName, TRIGGER_DEATH);
    }
}

// FUNCTION: SWBF2 0x0065d220
void TriggerRegionManager::AddLandingRegion(const char* pcName)
{
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        AddRegion(pcName, TRIGGER_LANDING);
    }
}

// FUNCTION: SWBF2 0x0065d260
void TriggerRegionManager::AddSinkRegion(const char* pcName)
{
    if (!NetGame::IsEnabled() || NetGame::IsHost())
    {
        AddRegion(pcName, TRIGGER_SINK);
    }
}

// FUNCTION: SWBF2 0x0065d2a0
void TriggerRegionManager::AddRegion(const char* pcName, TriggerType eType)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    RedRegion* pRegion = RedRegion::Find(PblHash(pcName));
    if (!pRegion)
    {
        return;
    }
    if (pRegion->mType == PblRegion::type_plane && eType == TRIGGER_SINK)
    {
        mSinkHeight = pRegion->mMatrix.m[3][1];
        return;
    }
    for (int i = 0; i < 32; ++i)
    {
        if (!mTriggerRegions[i].mRegion)
        {
            mTriggerRegions[i].mRegion = pRegion;
            mTriggerRegions[i].mType = eType;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0065d350
void TriggerRegionManager::RemoveRegion(const char* pcName)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < 32; ++i)
    {
        if (mTriggerRegions[i].mRegion && mTriggerRegions[i].mRegion->mId == PblHash(pcName))
        {
            mTriggerRegions[i].mRegion = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0065d3c0
TriggerRegionManager::TriggerRegionData* TriggerRegionManager::GetTriggerRegionData(int iIndex)
{
    if ((unsigned int)iIndex < 32 && mTriggerRegions[iIndex].mRegion)
    {
        return &mTriggerRegions[iIndex];
    }
    return 0;
}
