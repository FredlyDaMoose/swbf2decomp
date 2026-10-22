#pragma once

#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "ConnectivityGraph.h"

class ConnectivityArc;
class ConnectivityGraph;
class ConnectivityNode;
class UnitController;

class ConnectivityGraphFollower
{
public:
    bool CanGetNewRandomPoint() const;

    static float GetNodeBufferRadius(const UnitController* pCtrl);

    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }
    static MemoryPool sMemoryPool;

    ConnectivityGraphFollower(UnitController* pCtrl);
    bool SetDestination(const ConnectivityGraph* pGraph, const PblVector3& vFrom,
                        const PblVector3& vTo,
                        ConnectivityGraph::ProhibitFlag eType);
    bool GotoNextLeg();
    const PblVector3& GetCurrentDestination() const;

    bool GetNewValidRandomPoint(float fRadius);

    const ConnectivityArc* GetCurrentArc() { return mCurArc; }
    const unsigned int GetCurrentArcFlags() { return mCurArc ? mCurArc->mFlags : 0; }

    ConnectivityGraph* mGraph;
    unsigned int mType;
    UnitController* mCtrl;
    ConnectivityNode* mDestNode;
    ConnectivityNode* mDestNode2;
    bool mUseDest2ForWeights;
    PblVector3 mDestPos;
    int mDestClassification;
    ConnectivityNode* mCurDestNode;
    PblVector3 mCurDestPos;
    ConnectivityArc* mCurArc;
};

 
