#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"

class PblFileChunk;
class UnitController;
class ConnectivityArc;
class ConnectivityGraph;

class ConnectivityNode : public PblSphere
{
public:
    ConnectivityNode();
    ~ConnectivityNode();

    PblVector3 GetRandomPointInside(float fBuffer) const;
    const ConnectivityArc* GetRandomArc(const ConnectivityArc* pExclude,
                                        const ConnectivityGraph& graph,
                                        unsigned int uFlags) const;
    bool HasClosedDynamicArc(const ConnectivityGraph& graph, unsigned int uFlags);
    void Read(PblFileChunk& chunk, int iNumNodes);
    bool IsPointInside(const PblVector3& v) const;

    unsigned int mAllowTypes;
    unsigned char mArcs[8];

    unsigned char* mBranchingTable[5][4];
};
 
 

class ConnectivityArc
{
public:

    ConnectivityArc() : n1(0), n2(0), mFlags(0) {}

    void Read(PblFileChunk& chunk, ConnectivityGraph& graph);
    bool IsPointInside(const PblVector3& v) const;
    float GetDistFrom(const PblVector3& v) const;

    float GetNodeRadiusSum() { return n2->w + n1->w; }

    const ConnectivityNode* GetOtherNode(const ConnectivityNode* pNode) const
    {
        return n1 == pNode ? n2 : n1;
    }

    ConnectivityNode* n1;
    ConnectivityNode* n2;
    unsigned int mAllowTypes;
    unsigned int mFlags;
    PblVector3 mBoundingCenter;
    float mBoundingRadius;
};
 

struct ConnectivityGraphPatch
{
    int mUncompressedSize;
    int mCompressedSize;
    char mCompressedData[1];
};

class ConnectivityGraph
{
public:

    enum ProhibitFlag
    {
        CGT_FLAG_SOLDIER = 1,
        CGT_FLAG_HOVER = 2,
        CGT_FLAG_SMALL = 4,
        CGT_FLAG_MEDIUM = 8,
        CGT_FLAG_HUGE = 16,
        CGT_FLAG_FLYER = 32
    };
    enum
    {
        CGT_SOLDIER = 0,
        CGT_HOVER = 1,
        CGT_SMALL = 2,
        CGT_MEDIUM = 3,
        CGT_HUGE = 4,
        CGT_NUMTYPES = 5
    };

    ConnectivityGraph();
    ~ConnectivityGraph();

    bool Read(PblFileChunk& chunk);
    bool ReadPatch(PblFileChunk& chunk);

    const ConnectivityArc* GetNextArc(const ConnectivityArc* pCurArc,
                                      const ConnectivityNode& from,
                                      const ConnectivityNode& to,
                                      unsigned int uFlags) const;
    int ClassifyPoint(const PblVector3& v, const ConnectivityNode** ppNode,
                      const ConnectivityArc** ppArc, unsigned int uFlags) const;
    const ConnectivityArc* FindClosestArc(const PblVector3& v, unsigned int uFlags) const;
    const ConnectivityArc* GetArc(int iIndex) const;
    const ConnectivityNode* GetNode(int iIndex) const;
    const ConnectivityNode* GetRandomAccessibleNode(const UnitController* pCtrl) const;
    bool CanGetThereFromHere(UnitController* pCtrl, const PblVector3& vDest) const;
    bool CanGetThereFromHere_QuickButNotSoPerfectlyAccurateTest(UnitController* pCtrl,
                                                                const PblVector3& vDest) const;
    bool IsDynamicArc(unsigned int uName) const;
    void ToggleDynamicArcs(unsigned int* puNames, int iNumNames, unsigned int* puGroups,
                           int iNumGroups);
    void BlockDynamicArcs(unsigned int* puNames, int iNumNames, unsigned int* puGroups,
                          int iNumGroups);
    void UnblockDynamicArcs(unsigned int* puNames, int iNumNames, unsigned int* puGroups,
                            int iNumGroups);

private:
    int GetIndexFromNode(const ConnectivityNode& node) const;
    int GetIndexFromArc(const ConnectivityArc& arc) const;
    int _GetGroupFromName(unsigned int uName) const;
    unsigned int _GetDynamicArcBits(unsigned int* puNames, int iNumNames);
    void _SetDynamicArcState(unsigned int uState);
    void _UncompressPatch(unsigned int uState);
    void _ApplyUnzipBufferPatch();

    static void* _zlibAlloc(void* pOpaque, unsigned int uItems, unsigned int uSize);
    static void _zlibFree(void* pOpaque, void* pAddress);

public:
    ConnectivityNode* mNodes;
    int mMaxNodes;
    ConnectivityArc* mArcs;
    int mMaxArcs;
    unsigned int mAllowTypes;
    int mDynamicGroupCnt;
    ConnectivityGraphPatch** mPatchList;
    unsigned char mDynamicArcsInGroup[8];
    int mDynamicArcListCnt;
    unsigned int* mDynamicArcList;
    unsigned int mDynamicArcState;
    char* mPatchData;
    unsigned char* mDynamicArcUnzipBuffer;
    int mDynamicArcUnzipBufferSize;
};
 

int GetLowestBitIndex(unsigned int uBits);

namespace ConnectivityGraphManager
{
void Init();
void Clean();
void ReadWorldGraph(PblFileChunk& chunk);
void ReadWorldPatch(PblFileChunk& chunk);
ConnectivityGraph* GetWorldGraph(ConnectivityGraph::ProhibitFlag eFlag);

extern ConnectivityGraph* mGraph;
}
