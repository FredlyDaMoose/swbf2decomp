#include "sndparametergraph.h"

#include <stddef.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// GLOBAL: SWBF2 0x007e39b8
// SYNTHETIC: SWBF2 0x0040c3c0
// `dynamic initializer for 'Snd::ParameterGraph::ControlPoint::smFreeList''
// SYNTHETIC: SWBF2 0x0076a4d0
// `dynamic atexit destructor for 'Snd::ParameterGraph::ControlPoint::smFreeList''
FreeList ParameterGraph::ControlPoint::smFreeList(12, 1, 16, 1);

// GLOBAL: SWBF2 0x007e39d4
// SYNTHETIC: SWBF2 0x0040c3e0
// `dynamic initializer for 'Snd::ParameterGraph::smFreeList''
// SYNTHETIC: SWBF2 0x0076a4e0
// `dynamic atexit destructor for 'Snd::ParameterGraph::smFreeList''
FreeList ParameterGraph::smFreeList(16, 1, 8, 1);

// GLOBAL: SWBF2 0x007e39f0
// SYNTHETIC: SWBF2 0x0040c400
// `dynamic initializer for 'Snd::ParameterGraph::smParameterGraphs''
// SYNTHETIC: SWBF2 0x0076a4f0
// `dynamic atexit destructor for 'Snd::ParameterGraph::smParameterGraphs''
PblListSingle ParameterGraph::smParameterGraphs;

// FUNCTION: SWBF2 0x00748b30
ParameterGraph::~ParameterGraph()
{
    while (!mControlPoints.IsEmpty())
    {
        delete reinterpret_cast<ControlPoint*>(reinterpret_cast<char*>(mControlPoints.mHead.mNext) -
                                               offsetof(ControlPoint, mNode.mNode));
    }
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

}

// FUNCTION: SWBF2 0x00748c00
void PblListSingleSorted::Add(Node* pNode)
{
    if (mFlags & flags_sortOnAdd)
    {
        for (PblListSingle::Node* pLink = mHead.mNext; pLink != &mHead; pLink = pLink->mNext)
        {
            Node* const pCur = (Node*)((char*)pLink - offsetof(Node, mNode));
            if (pCur == GetTerminator())
            {
                break;
            }
            if (pCur->mSortValue > pNode->mSortValue)
            {
                pNode->mNode.mNext = pCur->mNode.mNext;
                pCur->mNode.mNext = &pNode->mNode;
                return;
            }
        }
    }
    AddLast(&pNode->mNode);
}

namespace Snd
{

// FUNCTION: SWBF2 0x00748d40
const ParameterGraph* __cdecl ParameterGraph::FindByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Node* pLink = smParameterGraphs.mHead.mNext; pLink != &smParameterGraphs.mHead;
         pLink = pLink->mNext)
    {
        const ParameterGraph* const pGraph = (const ParameterGraph*)((char*)pLink - offsetof(ParameterGraph, mNode));
        if (pGraph->mHashID == uiHashID)
        {
            return pGraph;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00748d70
void __cdecl ParameterGraph::Load(PblFileChunk& chunk, bool bOnce)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144u)
        {
            continue;
        }

        child._pFile->Read32();
        child = chunk.ReadNextChild();

        ParameterGraph* const pGraph = new ParameterGraph;
        pGraph->Read(child);
        if (bOnce)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00748e40
void ParameterGraph::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        child._pFile->Read8();
        if (uiID == 0x415B9110u)
        {
            const float fInput = child._pFile->ReadFloat();
            const float fOutput = child._pFile->ReadFloat();
            new ControlPoint(fInput, fOutput, mControlPoints);
        }
        else if (uiID == 0x8D39BDE6u)
        {
            mHashID = child._pFile->Read32();
        }
    }
}

// FUNCTION: SWBF2 0x00748c70
float ParameterGraph::CalculateOutputValue(float fInput) const
{
    const ControlPoint* pPrev = 0;
    for (const PblListSingle::Node* pLink = mControlPoints.mHead.mNext; pLink != &mControlPoints.mHead;
         pLink = pLink->mNext)
    {
        const PblListSingleSorted::Node* const pNode =
            (const PblListSingleSorted::Node*)((const char*)pLink - offsetof(PblListSingleSorted::Node, mNode));
        if (pNode == mControlPoints.GetTerminator())
        {
            break;
        }
        const ControlPoint* const pPoint = (const ControlPoint*)pNode;
        if (pPoint->mNode.mSortValue > fInput)
        {
            if (!pPrev)
            {
                return pPoint->mOutput;
            }
            const float fBlend = (pPoint->mNode.mSortValue - fInput)
                               / (pPoint->mNode.mSortValue - pPrev->mNode.mSortValue);
            return (1.0f - fBlend) * pPoint->mOutput + pPrev->mOutput * fBlend;
        }
        pPrev = pPoint;
    }
    return pPrev->mOutput;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00748d00
void __cdecl ParameterGraph::DestroyAll()
{
    while (!smParameterGraphs.IsEmpty())
    {
        ParameterGraph* const pGraph = reinterpret_cast<ParameterGraph*>(reinterpret_cast<char*>(smParameterGraphs.mHead.mNext) - offsetof(ParameterGraph, mNode));
        if (pGraph)
        {
            pGraph->~ParameterGraph();
            smFreeList.Free(pGraph);
        }
    }
}
}
