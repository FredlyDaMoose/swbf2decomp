#include <float.h>
#include <math.h>
#include <string.h>

#include "ConnectivityGraph.h"
#include "AIUtil.h"
#include "ConnectivityGraphFollower.h"
#include "Controllable.h"
#include "ControllerManager.h"
#include "EventInfo.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include "RedEngineFL/Memory/RedMemory.h"

#include "ZlibFL/zlib.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// GLOBAL: SWBF2 0x01f9c264

static char sDiscardedRecordLabel[16];

// GLOBAL: SWBF2 0x01e3089c
ConnectivityGraph* ConnectivityGraphManager::mGraph;

// FUNCTION: SWBF2 0x004804c0
ConnectivityNode::ConnectivityNode()
{
    memset(mBranchingTable, 0, sizeof(mBranchingTable));
}

// FUNCTION: SWBF2 0x004804e0
ConnectivityNode::~ConnectivityNode()
{
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (mBranchingTable[i][j])
            {
                delete[] mBranchingTable[i][j];
                return;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00480520
PblVector3 ConnectivityNode::GetRandomPointInside(float fBuffer) const
{
    float fRadius = w - fBuffer;
    if (fRadius < 0.0f)
    {
        fRadius = 0.0f;
    }
    const PblVector3 vRandom = PblRandom::GetSim().GetVector3();
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vRandom);
    return PblVector3(x + vDir.x * fRadius, y + vDir.y * fRadius, z + vDir.z * fRadius);
}

// FUNCTION: SWBF2 0x004805b0
const ConnectivityArc* ConnectivityNode::GetRandomArc(const ConnectivityArc* pExclude,
                                                      const ConnectivityGraph& graph,
                                                      unsigned int uFlags) const
{
    int iCount = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (mArcs[i] == 0xFF)
        {
            break;
        }
        if ((uFlags & graph.GetArc(mArcs[i])->mAllowTypes) && graph.GetArc(mArcs[i]) != pExclude)
        {
            ++iCount;
        }
    }
    if (iCount == 0)
    {
        return pExclude;
    }

    int iPick = PblRandom::GetSim().GetInt() % iCount;
    for (int i = 0; i < 8; ++i)
    {
        const ConnectivityArc* pArc = graph.GetArc(mArcs[i]);
        if ((uFlags & pArc->mAllowTypes) && pArc != pExclude && --iPick < 0)
        {
            return pArc;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00480680
bool ConnectivityNode::HasClosedDynamicArc(const ConnectivityGraph& graph, unsigned int uFlags)
{
    for (int i = 0; i < 8; ++i)
    {
        if (mArcs[i] == 0xFF)
        {
            return false;
        }
        const ConnectivityArc* pArc = graph.GetArc(mArcs[i]);
        if ((uFlags & pArc->mAllowTypes) && !graph.GetNextArc(0, *pArc->n1, *pArc->n2, uFlags)
            && !graph.GetNextArc(0, *pArc->n2, *pArc->n1, uFlags))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00480700
void ConnectivityNode::Read(PblFileChunk& chunk, int iNumNodes)
{
    chunk._pFile->ReadData(sDiscardedRecordLabel, 16);
    const float fX = chunk._pFile->ReadFloat();
    const float fY = chunk._pFile->ReadFloat();
    const float fZ = chunk._pFile->ReadFloat();
    x = fX;
    y = fY;
    z = fZ;
    w = chunk._pFile->ReadFloat();
    chunk._pFile->ReadData(mArcs, 8);
    memset(mBranchingTable, 0, sizeof(mBranchingTable));

    unsigned char aCounts[5];
    chunk._pFile->ReadData(aCounts, 5);
    const int iSize = iNumNodes * (aCounts[0] + aCounts[1] + aCounts[2] + aCounts[3] + aCounts[4]);
    if (iSize > 0)
    {
        unsigned char* pData = new unsigned char[iSize];
        chunk._pFile->ReadData(pData, iSize);
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < aCounts[i]; ++j)
            {
                mBranchingTable[i][j] = pData;
                pData += iNumNodes;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00480800
void ConnectivityArc::Read(PblFileChunk& chunk, ConnectivityGraph& graph)
{
    chunk._pFile->ReadData(sDiscardedRecordLabel, 16);
    n1 = &graph.mNodes[(unsigned char)chunk._pFile->Read8()];
    n2 = &graph.mNodes[(unsigned char)chunk._pFile->Read8()];
    mAllowTypes = chunk._pFile->Read32();
    mFlags = chunk._pFile->Read32();
}

// FUNCTION: SWBF2 0x00480860
bool ConnectivityNode::IsPointInside(const PblVector3& v) const
{
    return w * w > (x - v.x) * (x - v.x) + (z - v.z) * (z - v.z);
}

// FUNCTION: SWBF2 0x004808a0
bool ConnectivityArc::IsPointInside(const PblVector3& v) const
{
    if ((v.x - mBoundingCenter.x) * (v.x - mBoundingCenter.x)
            + (v.z - mBoundingCenter.z) * (v.z - mBoundingCenter.z)
        > mBoundingRadius * mBoundingRadius)
    {
        return false;
    }
    const PblVector3 vA = n1->GetCenter();
    const PblVector3 vAB(n2->x - vA.x, n2->y - vA.y, n2->z - vA.z);
    const float fT = ((v.x - vA.x) * vAB.x + (v.y - vA.y) * vAB.y + (v.z - vA.z) * vAB.z)
                     / (vAB.x * vAB.x + vAB.y * vAB.y + vAB.z * vAB.z);
    if (fT < 0.0f || fT > 1.0f)
    {
        return false;
    }
    const float fDX = v.x - (vAB.x * fT + vA.x);
    const float fDY = v.y - (vAB.y * fT + vA.y);
    const float fDZ = v.z - (vAB.z * fT + vA.z);
    const float fDist = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
    return (1.0f - fT) * n1->w + n2->w * fT > fDist;
}

// FUNCTION: SWBF2 0x00480a10
float ConnectivityArc::GetDistFrom(const PblVector3& v) const
{
    const PblVector3 vA = n1->GetCenter();
    const PblVector3 vAB(n2->x - vA.x, n2->y - vA.y, n2->z - vA.z);
    const float fT = ((v.x - vA.x) * vAB.x + (v.y - vA.y) * vAB.y + (v.z - vA.z) * vAB.z)
                     / (vAB.x * vAB.x + vAB.y * vAB.y + vAB.z * vAB.z);
    if (fT >= 0.0f && fT <= 1.0f)
    {
        const float fRadius = (1.0f - fT) * n1->w + n2->w * fT;
        const float fDX = v.x - (vAB.x * fT + vA.x);
        const float fDY = v.y - (vAB.y * fT + vA.y);
        const float fDZ = v.z - (vAB.z * fT + vA.z);
        const float fDist = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
        if (fRadius > fDist)
        {
            return 0.0f;
        }
        return fDist - fRadius;
    }

    float fDist;
    if (fT < 0.0f)
    {
        fDist = sqrtf((v.x - n1->x) * (v.x - n1->x) + (v.y - n1->y) * (v.y - n1->y)
                      + (v.z - n1->z) * (v.z - n1->z))
                - n1->w;
    }
    else
    {
        fDist = sqrtf((v.x - n2->x) * (v.x - n2->x) + (v.y - n2->y) * (v.y - n2->y)
                      + (v.z - n2->z) * (v.z - n2->z))
                - n2->w;
    }
    return fDist > 0.0f ? fDist : 0.0f;
}

// FUNCTION: SWBF2 0x00480bf0
ConnectivityGraph::ConnectivityGraph()
    : mNodes(0),
      mMaxNodes(0),
      mArcs(0),
      mMaxArcs(0),
      mAllowTypes(0),
      mDynamicGroupCnt(0),
      mPatchList(0),
      mDynamicArcListCnt(0),
      mDynamicArcList(0),
      mDynamicArcState(0),
      mPatchData(0),
      mDynamicArcUnzipBuffer(0),
      mDynamicArcUnzipBufferSize(0)
{
    memset(mDynamicArcsInGroup, 0, sizeof(mDynamicArcsInGroup));
}

// FUNCTION: SWBF2 0x00480c60
ConnectivityGraph::~ConnectivityGraph()
{
    delete[] mDynamicArcUnzipBuffer;
    delete[] mPatchData;
    delete[] mPatchList;
    delete[] mDynamicArcList;
    delete[] mArcs;
    delete[] mNodes;
}

// FUNCTION: SWBF2 0x00480cc0
int GetLowestBitIndex(unsigned int uBits)
{
    int iIndex = 0;
    if (uBits)
    {
        while (!(uBits & 1))
        {
            uBits >>= 1;
            ++iIndex;
        }
    }
    return iIndex;
}

// FUNCTION: SWBF2 0x00480ce0
int ConnectivityGraph::GetIndexFromNode(const ConnectivityNode& node) const
{
    for (int i = 0; i < mMaxNodes; ++i)
    {
        if (&mNodes[i] == &node)
        {
            return i;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00480d10
int ConnectivityGraph::GetIndexFromArc(const ConnectivityArc& arc) const
{
    for (int i = 0; i < mMaxArcs; ++i)
    {
        if (&mArcs[i] == &arc)
        {
            return i;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00480d40
const ConnectivityArc* ConnectivityGraph::GetNextArc(const ConnectivityArc* pCurArc,
                                                     const ConnectivityNode& from,
                                                     const ConnectivityNode& to,
                                                     unsigned int uFlags) const
{
    if (!(uFlags & mAllowTypes))
    {
        return 0;
    }
    const int iType = GetLowestBitIndex(uFlags);
    const int iTo = GetIndexFromNode(to);
    int iTotal = 31;
    unsigned int uCurSlot = 8;
    if (pCurArc)
    {
        const int iArc = GetIndexFromArc(*pCurArc);
        for (uCurSlot = 0; uCurSlot < 8; ++uCurSlot)
        {
            if (from.mArcs[uCurSlot] == iArc)
            {
                break;
            }
        }
        for (int i = 0; i < 4; ++i)
        {
            const unsigned char* pTable = from.mBranchingTable[iType][i];
            if (!pTable || !pTable[iTo])
            {
                break;
            }
            const unsigned int uEntry = pTable[iTo];
            if ((uEntry & 7) == uCurSlot)
            {
                iTotal = 31 - (uEntry >> 3);
                if (iTotal > 0)
                {
                    break;
                }
                return from.GetRandomArc(pCurArc, *this, uFlags);
            }
        }
    }

    unsigned char* const* ppTables = from.mBranchingTable[iType];
    bool bAny = false;
    for (int i = 0; i < 4; ++i)
    {
        if (ppTables[i] && ppTables[i][iTo])
        {
            bAny = true;
            break;
        }
    }
    if (bAny)
    {
        int iPick = PblRandom::GetSim().GetInt() % iTotal;
        for (int i = 0; i < 4; ++i)
        {
            const unsigned char* pTable = ppTables[i];
            if (!pTable || !pTable[iTo])
            {
                break;
            }
            const unsigned int uEntry = pTable[iTo];
            iPick -= uEntry >> 3;
            if (iPick <= 0 && (uEntry & 7) != uCurSlot)
            {
                return &mArcs[from.mArcs[uEntry & 7]];
            }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00480ef0
bool ConnectivityGraph::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4F464E49:
            mMaxNodes = child._pFile->Read16();
            mMaxArcs = child._pFile->Read16();
            child._pFile->Read16();
            mNodes = new ConnectivityNode[mMaxNodes];
            mArcs = new ConnectivityArc[mMaxArcs];
            break;
        case 0x45444F4E:
            for (int i = 0; i < mMaxNodes; ++i)
            {
                mNodes[i].Read(child, mMaxNodes);
            }
            break;
        case 0x53435241:
            for (int i = 0; i < mMaxArcs; ++i)
            {
                mArcs[i].Read(child, *this);
                mAllowTypes |= mArcs[i].mAllowTypes;
            }
            break;
        }
    }

    for (int i = 0; i < mMaxNodes; ++i)
    {
        mNodes[i].mAllowTypes = 0;
        for (int j = 0; j < 8; ++j)
        {
            if (mNodes[i].mArcs[j] != 0xFF)
            {
                mNodes[i].mAllowTypes |= mArcs[mNodes[i].mArcs[j]].mAllowTypes;
            }
        }
    }

    for (int i = 0; i < mMaxArcs; ++i)
    {
        const ConnectivityNode* pN1 = mArcs[i].n1;
        const ConnectivityNode* pN2 = mArcs[i].n2;
        const PblVector3 vDelta(pN2->x - pN1->x, pN2->y - pN1->y, pN2->z - pN1->z);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vDelta);

        pN1 = mArcs[i].n1;
        const PblVector3 vStart(pN1->x - vDir.x * pN1->w, pN1->y - vDir.y * pN1->w,
                                pN1->z - vDir.z * pN1->w);
        pN2 = mArcs[i].n2;
        const PblVector3 vEnd(pN2->x + vDir.x * pN2->w, pN2->y + vDir.y * pN2->w,
                              pN2->z + vDir.z * pN2->w);
        mArcs[i].mBoundingCenter = PblVector3((vEnd.x + vStart.x) * 0.5f,
                                              (vEnd.y + vStart.y) * 0.5f,
                                              (vEnd.z + vStart.z) * 0.5f);

        const PblVector3& vCenter = mArcs[i].mBoundingCenter;
        float fRadius = sqrtf((vStart.x - vCenter.x) * (vStart.x - vCenter.x)
                              + (vStart.y - vCenter.y) * (vStart.y - vCenter.y)
                              + (vStart.z - vCenter.z) * (vStart.z - vCenter.z));
        if (!(fRadius > mArcs[i].n1->w))
        {
            fRadius = mArcs[i].n1->w;
        }
        if (!(fRadius > mArcs[i].n2->w))
        {
            fRadius = mArcs[i].n2->w;
        }
        mArcs[i].mBoundingRadius = fRadius;
    }

    return mMaxArcs && mMaxNodes;
}

// FUNCTION: SWBF2 0x00481380
bool ConnectivityGraph::ReadPatch(PblFileChunk& chunk)
{
    mDynamicGroupCnt = chunk._pFile->Read8();
    if (mDynamicGroupCnt > 8)
    {
        mDynamicGroupCnt = 0;
        return false;
    }
    chunk._pFile->ReadData(mDynamicArcsInGroup, mDynamicGroupCnt);
    mDynamicArcListCnt = chunk._pFile->Read8();
    mDynamicArcList = new unsigned int[mDynamicArcListCnt];
    chunk._pFile->ReadData(mDynamicArcList, mDynamicArcListCnt * 4);

    const int iNumPatches = 1 << mDynamicGroupCnt;
    mPatchList = new ConnectivityGraphPatch*[iNumPatches];
    char* pData = new char[chunk.GetSize() + (iNumPatches * 4 - mDynamicArcListCnt) * 4
                           - mDynamicGroupCnt - 2];
    mPatchData = pData;
    mDynamicArcUnzipBufferSize = 0;
    for (int i = 0; i < iNumPatches; ++i)
    {
        ConnectivityGraphPatch* pPatch = (ConnectivityGraphPatch*)(((unsigned int)pData + 15) & ~15u);
        mPatchList[i] = pPatch;
        mPatchList[i]->mUncompressedSize = chunk._pFile->Read32();
        if (mPatchList[i]->mUncompressedSize > mDynamicArcUnzipBufferSize)
        {
            mDynamicArcUnzipBufferSize = mPatchList[i]->mUncompressedSize;
        }
        const int iCompressed = chunk._pFile->Read32();
        mPatchList[i]->mCompressedSize = iCompressed;
        chunk._pFile->ReadData(mPatchList[i]->mCompressedData, iCompressed);
        pData = (char*)pPatch + iCompressed + 8;
    }
    mDynamicArcUnzipBuffer = new unsigned char[mDynamicArcUnzipBufferSize];
    return true;
}

// FUNCTION: SWBF2 0x004814e0
int ConnectivityGraph::ClassifyPoint(const PblVector3& v, const ConnectivityNode** ppNode,
                                     const ConnectivityArc** ppArc, unsigned int uFlags) const
{
    if (!(uFlags & mAllowTypes))
    {
        return 0;
    }
    float fBest = 10000.0f;
    for (int i = 0; i < mMaxNodes; ++i)
    {
        const ConnectivityNode* pNode = &mNodes[i];
        if ((uFlags & pNode->mAllowTypes) && fBest > pNode->w && pNode->IsPointInside(v))
        {
            fBest = pNode->w;
            *ppNode = pNode;
        }
    }
    if (fBest < 10000.0f)
    {
        return 1;
    }

    fBest = 10000.0f;
    for (int i = 0; i < mMaxArcs; ++i)
    {
        ConnectivityArc* pArc = &mArcs[i];
        if (!(pArc->mFlags & 7) && (uFlags & pArc->mAllowTypes)
            && fBest > pArc->GetNodeRadiusSum() && pArc->IsPointInside(v))
        {
            fBest = pArc->GetNodeRadiusSum();
            *ppArc = pArc;
        }
    }
    if (fBest < 10000.0f)
    {
        return 2;
    }
    *ppNode = 0;
    *ppArc = 0;
    return 0;
}

// FUNCTION: SWBF2 0x00481630
const ConnectivityArc* ConnectivityGraph::FindClosestArc(const PblVector3& v, unsigned int uFlags) const
{
    if (!(uFlags & mAllowTypes))
    {
        return 0;
    }
    const ConnectivityArc* pBest = 0;
    float fBest = FLT_MAX;
    for (int i = 0; i < mMaxArcs; ++i)
    {
        const ConnectivityArc* pArc = &mArcs[i];
        if ((uFlags & pArc->mAllowTypes) && !(pArc->mFlags & 1))
        {
            const PblVector3& vC = pArc->mBoundingCenter;
            if (!((vC.x - v.x) * (vC.x - v.x) + (vC.y - v.y) * (vC.y - v.y) + (vC.z - v.z) * (vC.z - v.z)
                > (pArc->mBoundingRadius + fBest) * (pArc->mBoundingRadius + fBest)))
            {
                const float fDist = pArc->GetDistFrom(v);
                if (fBest > fDist)
                {
                    fBest = fDist;
                    pBest = pArc;
                }
            }
        }
    }
    return pBest;
}

// FUNCTION: SWBF2 0x00481720
const ConnectivityArc* ConnectivityGraph::GetArc(int iIndex) const
{
    return &mArcs[iIndex];
}

// FUNCTION: SWBF2 0x00481730
const ConnectivityNode* ConnectivityGraph::GetNode(int iIndex) const
{
    return &mNodes[iIndex];
}

// FUNCTION: SWBF2 0x00481740
int ConnectivityGraph::_GetGroupFromName(unsigned int uName) const
{
    int iInGroup = 0;
    int iGroup = 0;
    for (int i = 0; i < mDynamicArcListCnt; ++i)
    {
        if (mDynamicArcList[i] == uName)
        {
            return iGroup;
        }
        if (++iInGroup >= mDynamicArcsInGroup[iGroup])
        {
            ++iGroup;
            iInGroup = 0;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x00481790
unsigned int ConnectivityGraph::_GetDynamicArcBits(unsigned int* puNames, int iNumNames)
{
    unsigned int uBits = 0;
    for (int i = 0; i < iNumNames; ++i)
    {
        const int iGroup = _GetGroupFromName(puNames[i]);
        if (iGroup >= 0)
        {
            uBits |= 1 << iGroup;
        }
    }
    return uBits;
}

// FUNCTION: SWBF2 0x004817e0
void ConnectivityGraph::ToggleDynamicArcs(unsigned int* puNames, int iNumNames,
                                          unsigned int* puGroups, int iNumGroups)
{
    unsigned int uBits = _GetDynamicArcBits(puNames, iNumNames);
    for (int i = iNumGroups - 1; i >= 0; --i)
    {
        uBits |= 1 << (puGroups[i] - 1);
    }
    _SetDynamicArcState(mDynamicArcState ^ uBits);
}

// FUNCTION: SWBF2 0x00481830
void ConnectivityGraph::BlockDynamicArcs(unsigned int* puNames, int iNumNames,
                                         unsigned int* puGroups, int iNumGroups)
{
    unsigned int uBits = _GetDynamicArcBits(puNames, iNumNames);
    for (int i = iNumGroups - 1; i >= 0; --i)
    {
        uBits |= 1 << (puGroups[i] - 1);
    }
    _SetDynamicArcState(mDynamicArcState | uBits);
}

// FUNCTION: SWBF2 0x00481880
void ConnectivityGraph::UnblockDynamicArcs(unsigned int* puNames, int iNumNames,
                                           unsigned int* puGroups, int iNumGroups)
{
    unsigned int uBits = _GetDynamicArcBits(puNames, iNumNames);
    for (int i = iNumGroups - 1; i >= 0; --i)
    {
        uBits |= 1 << (puGroups[i] - 1);
    }
    _SetDynamicArcState(mDynamicArcState & ~uBits);
}

// FUNCTION: SWBF2 0x004818d0
bool ConnectivityGraph::IsDynamicArc(unsigned int uName) const
{
    for (int i = 0; i < mDynamicArcListCnt; ++i)
    {
        if (mDynamicArcList[i] == uName)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00481900
bool ConnectivityGraph::CanGetThereFromHere_QuickButNotSoPerfectlyAccurateTest(
    UnitController* pCtrl, const PblVector3& vDest) const
{
    if (!pCtrl || !pCtrl->mOwner || !pCtrl->mOwner->GetGameObject())
    {
        return false;
    }

    const ProhibitFlag eType = (ProhibitFlag)AIUtil::GetAIType(pCtrl->mOwner->GetGameObject());
    if (eType == CGT_FLAG_FLYER)
    {
        return true;
    }
    const PblVector3& vFrom = *(const PblVector3*)&pCtrl->mOwner->GetMatrix().m[3][0];

    float fFromDist = FLT_MAX;
    float fToDist = FLT_MAX;
    ConnectivityNode* pFrom = 0;
    ConnectivityNode* pTo = 0;
    for (int i = 0; i < mMaxNodes; ++i)
    {
        ConnectivityNode* pNode = &mNodes[i];
        if (eType & pNode->mAllowTypes)
        {
            float fDist = sqrtf((vFrom.z - pNode->z) * (vFrom.z - pNode->z)
                                + (vFrom.x - pNode->x) * (vFrom.x - pNode->x))
                          - pNode->w;
            if (fDist < 0.0f)
            {
                fDist = 0.0f;
            }
            if (fFromDist > fDist)
            {
                fFromDist = fDist;
                pFrom = pNode;
            }
            const float fDist2 = (vDest.z - pNode->z) * (vDest.z - pNode->z)
                                 + (vDest.x - pNode->x) * (vDest.x - pNode->x);
            if (fToDist > fDist2)
            {
                fToDist = fDist2;
                pTo = pNode;
            }
        }
    }
    if (pFrom == pTo)
    {
        return true;
    }
    if ((fFromDist > pFrom->w * pFrom->w && pFrom->HasClosedDynamicArc(*this, eType))
        || (fToDist > pTo->w * pTo->w && pTo->HasClosedDynamicArc(*this, eType)))
    {
        return CanGetThereFromHere(pCtrl, vDest);
    }
    return GetNextArc(0, *pFrom, *pTo, eType) != 0;
}

// FUNCTION: SWBF2 0x00481ae0
bool ConnectivityGraph::CanGetThereFromHere(UnitController* pCtrl, const PblVector3& vDest) const
{
    if (!pCtrl || !pCtrl->mOwner || !pCtrl->mOwner->GetGameObject())
    {
        return false;
    }

    const ProhibitFlag eType = (ProhibitFlag)AIUtil::GetAIType(pCtrl->mOwner->GetGameObject());
    if (eType == CGT_FLAG_FLYER)
    {
        return true;
    }
    ConnectivityGraphFollower follower(pCtrl);
    if (!follower.SetDestination(this, *(const PblVector3*)&pCtrl->mOwner->GetMatrix().m[3][0],
                                 vDest, eType))
    {
        return false;
    }
    int iLegs = 100;
    while (follower.GetCurrentDestination() != vDest)
    {
        if (--iLegs <= 0 || !follower.GotoNextLeg())
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00481bb0
const ConnectivityNode* ConnectivityGraph::GetRandomAccessibleNode(const UnitController* pCtrl) const
{
    if (!pCtrl || !pCtrl->mOwner || !((const Controllable*)pCtrl->mOwner)->GetGameObject())
    {
        return 0;
    }
    const ProhibitFlag eType =
        (ProhibitFlag)AIUtil::GetAIType(((const Controllable*)pCtrl->mOwner)->GetGameObject());
    if (eType == CGT_FLAG_FLYER)
    {
        return 0;
    }
    float fBest = FLT_MAX;
    const ConnectivityNode* pNearest = 0;
    const PblVector3& vPos = *(const PblVector3*)&pCtrl->mOwner->GetMatrix().m[3][0];
    for (int i = 0; i < mMaxNodes; ++i)
    {
        const ConnectivityNode* pNode = &mNodes[i];
        if (eType & pNode->mAllowTypes)
        {
            float fDist = sqrtf((vPos.x - pNode->x) * (vPos.x - pNode->x)
                                + (vPos.z - pNode->z) * (vPos.z - pNode->z))
                          - pNode->w;
            if (fDist < 0.0f)
            {
                fDist = 0.0f;
            }
            if (fBest > fDist)
            {
                fBest = fDist;
                pNearest = pNode;
            }
        }
    }
    for (int i = 0; i < 2 * mMaxNodes; ++i)
    {
        const ConnectivityNode* pNode = GetNode(PblRandom::GetGlobalInt() % mMaxNodes);
        if (pNode != pNearest && GetNextArc(0, *pNearest, *pNode, eType))
        {
            return pNode;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00481d40
void ConnectivityGraph::_SetDynamicArcState(unsigned int uState)
{
    uState &= (1 << mDynamicGroupCnt) - 1;
    if (mDynamicArcState == uState)
    {
        return;
    }
    if (mDynamicArcState)
    {
        _ApplyUnzipBufferPatch();
    }
    mDynamicArcState = uState;
    if (uState)
    {
        _UncompressPatch(uState);
        _ApplyUnzipBufferPatch();
    }
    AI::EventInfo info(AI::EVT_Path_DynamicCongraphChange);
    ControllerManager::BroadcastEvent(&info, 1000);
}

// FUNCTION: SWBF2 0x00481db0
void* ConnectivityGraph::_zlibAlloc(void* pOpaque, unsigned int uItems, unsigned int uSize)
{
    const unsigned int uBytes = uItems * uSize;
    void* pMem = RedAllocFromHeap(__RedCurrHeap, uBytes, 0);
    if (pMem)
    {
        memset(pMem, 0, uBytes);
    }
    return pMem;
}

// FUNCTION: SWBF2 0x00481df0
void ConnectivityGraph::_zlibFree(void* pOpaque, void* pAddress)
{
    RedFreeToHeap(__RedCurrHeap, pAddress, 0);
}

// FUNCTION: SWBF2 0x00481e10
void ConnectivityGraph::_UncompressPatch(unsigned int uState)
{
    ConnectivityGraphPatch* pPatch = mPatchList[uState];
    z_stream stream;
    stream.zalloc = _zlibAlloc;
    stream.zfree = _zlibFree;
    stream.opaque = 0;
    if (inflateInit(&stream) != Z_OK)
    {
        return;
    }
    stream.next_in = (Bytef*)pPatch->mCompressedData;
    stream.avail_in = pPatch->mCompressedSize;
    stream.total_in = 0;
    stream.next_out = mDynamicArcUnzipBuffer;
    stream.avail_out = mDynamicArcUnzipBufferSize;
    stream.total_out = 0;
    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
}

// FUNCTION: SWBF2 0x00481ea0
void ConnectivityGraph::_ApplyUnzipBufferPatch()
{
    const unsigned char* pData = mDynamicArcUnzipBuffer;
    for (int i = 0; i < mMaxNodes; ++i)
    {
        if (*pData++)
        {
            ConnectivityNode& node = mNodes[i];
            for (int t = 0; t < CGT_NUMTYPES; ++t)
            {
                const int iNumTables = *pData++;
                for (int j = 0; j < iNumTables; ++j)
                {
                    for (int k = 0; k < mMaxNodes; ++k)
                    {
                        node.mBranchingTable[t][j][k] ^= *pData++;
                    }
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x00481f50
void ConnectivityGraphManager::Init()
{
    mGraph = 0;
}

// FUNCTION: SWBF2 0x00481f60
void ConnectivityGraphManager::Clean()
{
    mGraph = 0;
}

// FUNCTION: SWBF2 0x00481f70
void ConnectivityGraphManager::ReadWorldGraph(PblFileChunk& chunk)
{
    if (mGraph)
    {
        delete mGraph;
        mGraph = 0;
    }
    mGraph = new ConnectivityGraph;
    if (!mGraph->Read(chunk))
    {
        delete mGraph;
        mGraph = 0;
    }
}

// FUNCTION: SWBF2 0x00481ff0
void ConnectivityGraphManager::ReadWorldPatch(PblFileChunk& chunk)
{
    if (mGraph)
    {
        mGraph->ReadPatch(chunk);
    }
}

// FUNCTION: SWBF2 0x00482000
ConnectivityGraph* ConnectivityGraphManager::GetWorldGraph(ConnectivityGraph::ProhibitFlag eFlag)
{
    if (mGraph && !(mGraph->mAllowTypes & eFlag))
    {
        return 0;
    }
    return mGraph;
}
