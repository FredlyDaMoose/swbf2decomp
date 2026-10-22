#pragma once

#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblHandle.h"

class Controllable;
class GameObject;
class UnitController;

namespace VisionManager
{
    class RayRequest
    {
    public:

        static MemoryPool sMemoryPool;

        static void operator delete(void* p) { sMemoryPool.Free(p); }

        static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }

        RayRequest(UnitController* pCtrl, GameObject* pHim, int iPriority);

        PblHandle<UnitController> mCtrl;
        PblHandle<GameObject> mHim;
        int mPriority;
    };

    void Initialize();
    void SetGlobalVisModifier(float fModifier);

    extern GameObject** sPVRect;
    extern PblHeap<RayRequest*, int>* sHeap1;
    extern PblHeap<RayRequest*, int>* sHeap2;
    extern PblHeap<RayRequest*, int>* sHeap;
    extern PblHeap<RayRequest*, int>* sTmpHeap;
    extern int mCastHistory[8];
    extern float mGlobalVisibilityModifier;
    extern int mCurCast;

    bool VisionTest(UnitController* pController, GameObject* pTarget);

    float MaxVisibleDist(const GameObject* pSeer, const GameObject* pTarget);
    void UpdatePotentiallyVisible(UnitController* pController);
    void RemoveRequests(UnitController* pController);

    void AddRayRequest(UnitController* pController, GameObject* pObj, int iPriority);

    int GetVisualPriority(const Controllable* pMe, const GameObject* pObj);

    int GetRelativePriority(const GameObject* pMe, const GameObject* pObj);
}

namespace VisionManager { void __fastcall AddFakeRayhitResult(UnitController* pUnit, const GameObject* pObject); }
