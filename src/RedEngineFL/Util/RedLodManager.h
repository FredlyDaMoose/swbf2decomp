#pragma once

#include "PebbleFL/Common/PblHeap.h"

class RedSceneObject;

struct LodClassData
{
    int mCostHint;
    int mMaxCost;
};

struct RedLodClass
{
    float mFadeSpeed;
    LodClassData mData[4];
    PblHeap<RedSceneObject*, float>* mHeap;
};

namespace RedLodManager
{

    void Add(RedSceneObject* pObject, float fPriority);

    void Cleanup();
    void Init(int iNumClass);
    float GetMinScreenSize();
    bool IsFull(int iClass);
    void Render(unsigned int uiFlags);
    void SetClassLodHint(int iClass, int iHint0, int iHint1, int iHint2);
    void SetClassMaxCost(int iClass, int iHeapSize, int iCost0, int iCost1,
                         int iCost2, int iCost3);
    void SetFadeSpeed(int iClass, float fSpeed);

    extern RedLodClass* mClass;
}
