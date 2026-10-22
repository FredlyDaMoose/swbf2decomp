#pragma once

class PblFileChunk;

#include "sndfreelist.h"
#include "PebbleFL/Common/PblListSingle.h"
#include "PebbleFL/Common/PblListSingleSorted.h"

namespace Snd
{

class ParameterGraph
{
public:

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);

    ParameterGraph() : mControlPoints(true), mHashID(0)
    {
        smParameterGraphs.AddLast(&mNode);
    }

    static void* operator new(unsigned int) { return smFreeList.Alloc(); }

    static void operator delete(void* p) { smFreeList.Free(p); }

    ~ParameterGraph();

    void Read(PblFileChunk& chunk);

    static void __cdecl DestroyAll();

    static const ParameterGraph* __cdecl FindByHashID(unsigned int uiHashID);

    float CalculateOutputValue(float fInput) const;

    class ControlPoint
    {
    public:

        ControlPoint(float fInput, float fOutput, PblListSingleSorted& rList)
            : mNode(fInput), mOutput(fOutput)
        {
            rList.Add(&mNode);
        }

        static void* operator new(unsigned int) { return smFreeList.Alloc(); }

        static void operator delete(void* p) { smFreeList.Free(p); }
        ~ControlPoint()
        {
            if (mNode.mNode.mNext)
            {
                mNode.mNode.Remove();
            }
        }

        PblListSingleSorted::Node mNode;
        float mOutput;

        static FreeList smFreeList;
    };

    static FreeList smFreeList;
    static PblListSingle smParameterGraphs;

    PblListSingleSorted mControlPoints;
    unsigned int mHashID;
    PblListSingle::Node mNode;
};

 
 

}
