#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class Controllable;
class GameObject;
class UnitController;

namespace AI
{

    enum ThreatStatus
    {
        TS_UNKNOWN = 0,
        TS_UNSEEN = 1,
        TS_FORCEDSEEN = 2,
        TS_SEEN = 3,
        TS_LOCKED = 4
    };

    class Threat
    {
    public:

        Threat();

        Threat(GameObject* pObj, ThreatStatus eStatus);

        ~Threat();

        void Reset();

        unsigned char GetPriority(float fTime, Controllable* pMe) const;

        GameObject* GetThreatObject() const { return mThreatObject; }
        ThreatStatus GetThreatStatus() const { return mThreatStatus; }

        PblHandle<GameObject> mThreatObject;
        ThreatStatus mThreatStatus;
        float mLastStatusTime;
        float mLastSeenTime;
        PblVector3 mLastSeenPos;
    };

    class UnitThreatManager
    {
    public:
        enum { MAX_THREATS = 6 };

        UnitThreatManager();

        ~UnitThreatManager();
        void Update(float dt, UnitController* pCtrl);
        const GameObject* GetVisibleObject() const;
        const PblVector3& GetVisibleObjectPos() const;

        const Threat& GetThreat(int i) const { return mThreats[i]; }

        void RayhitResult(GameObject* pObj, bool bHit);

        void ForceThreat(GameObject* pObj);
        void LockThreat(GameObject* pObj);
        void AudibleResult(GameObject* pObj);

        bool ShouldRaytestUnit(const GameObject* pObj) const;

        void AddThreat(GameObject* pObj);

    private:

        int GetHighestPriorityIndex(bool b) const;

        void UpdateThreat(GameObject* pObj, ThreatStatus eStatus);
        float TimeSinceLastRaytest(const GameObject* pObj) const;

    public:
        UnitController* mMyCtrl;
        Threat mThreats[MAX_THREATS];
        int mVisibleObjectIndex;
        float mLastVisObjUpdateTime;
    };
}

 
 
 
 
 
