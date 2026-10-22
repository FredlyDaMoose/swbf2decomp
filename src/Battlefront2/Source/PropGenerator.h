#pragma once

#include <stddef.h>

#include "PropCluster.h"
#include "PropModel.h"
#include "Battlefront2Lod.h"
#include "Thread.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Util/ext/basevector.h"

class PropModel;
class PblFileChunk;

class PropLayerLodData : public RedLodData
{
public:
    PropLayerLodData(float fMaxDist)
    {
        _LodClass = -1;
        _LodBitmask = 7;
        _LodCost[0] = 0;
        _LodCost[1] = 0;
        _LodCost[2] = 0;
        _LodCost[3] = 0;
        mLowRezFlags = 0;
        mMaxDist = fMaxDist;
    }
};
 

class PropLayer
{
public:

    PropLayer();
    ~PropLayer();
    void Clear();
    void Read(PblFileChunk& chunk);
    void CreateCollisionObjects(int x, int z, int iOffset, float fParam);
    PropObject* CreateObject(int x, int z, int iOffset, float fParam);

    PropModel* mModels[32];
    int mNumModels;
    int mSpreadFactor;
    int mObjectsPerCell;
    float mMaxDistance;
    PropLayerLodData* mLodData;
};
 
 

class PropClusterInfo
{
public:
    void Setup(int x, int z);

    unsigned char mLayerInfo;
    unsigned char mOwner;
};

namespace PropGenerator
{
    // VTABLE: SWBF2 0x007ade90
    class PropGeneratorThread : public Thread
    {
    public:
        virtual ~PropGeneratorThread() {}
        virtual bool Update(float fDeltaTime);
    };

    void Init();
    void Cleanup();
    void Read(PblFileChunk& chunk);
    void SetupClusters();
    void SetRandSeed(unsigned int uiSeed);
    float GetRand();
    void Update();

    void FlushClusterObjects_();
    void SortClusters();
    void ResetClusterRenderedFlags();
    int GetClusterIndex(float x, float z);
    float CalcDistScore_(const PropCluster* pCluster, const PblVector3& vPos, bool bAllCameras);

    float GetLayerMaxDist(int iLayer);
    void AddObjectsFromClusterIndex(int iClusterIndex, int iLayer, PblList<PropObject>& objects);

    void GetTerrainY(float x, float z, float& y);

    const RedLodData* GetLodData(int iIndex);

    const PropObject* FindPropObject(const PblVector3& vPos, bool bParam);

    extern PropClusterInfo* mClusterMap;
    extern float mClusterCellWidth;
    extern PropCluster** mClusterObjectArray;
    extern int mNumClusterObjects;
    extern int mClusterMapWidth;
    extern bool mInitFlag;
    extern bool mLoaded;
    extern PropLayer mPropLayers[4];
    extern PblMatrix m_amCamera[1];
    extern PropGeneratorThread mThread;

    extern unsigned int mSeed;
    extern float mLayerCellWidthAspect;
    extern int mLayerMapWidth;
    extern float mLayerCellWidth;
    extern float mClusterCellWidthAspect;
}
