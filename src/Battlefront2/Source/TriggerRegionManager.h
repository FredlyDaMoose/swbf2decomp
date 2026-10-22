#pragma once

#include <stddef.h>

#include "Thread.h"

class GameObject;
class RedRegion;

// VTABLE: SWBF2 0x007af610
class TriggerRegionManager : public Thread
{
public:
    enum TriggerType
    {
        TRIGGER_DEATH = 0,
        TRIGGER_SINK = 1,
        TRIGGER_LANDING = 2
    };

    struct TriggerRegionData
    {
        RedRegion* mRegion;
        TriggerType mType;
    };

    TriggerRegionManager();
    virtual ~TriggerRegionManager();
    virtual bool Update(float fDeltaTime);

    static bool Create();
    static bool Destroy();
    static void CreateClient();
    static void DestroyClient();

    void InitialUpdate();

    static int GetObjectsInRegion(const RedRegion* pRegion, GameObject** const ppObjects, int iMaxObjects);

    void RemoveRegion(const char* pcName);
    TriggerRegionData* GetTriggerRegionData(int iIndex);
    void ApplyTempDeathRegion(const RedRegion* pRegion);

    void AddDeathRegion(const char* pcName);
    void AddLandingRegion(const char* pcName);
    void AddSinkRegion(const char* pcName);

    void TriggerDeath(GameObject* pObject);

private:
    void AddRegion(const char* pcName, TriggerType eType);

public:
    TriggerRegionData mTriggerRegions[32];
    float mSinkHeight;
    bool mLandingRegionsExist;

    static TriggerRegionManager* sInstance;
};
 
 
 
 
