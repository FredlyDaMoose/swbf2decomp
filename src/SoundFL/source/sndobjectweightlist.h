#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "sndfreelist.h"
#include "sndsourcetransportcontrol.h"

namespace Snd
{

class ObjectWeight : public SourceTransportControl
{
public:

    ObjectWeight(PblListSingle& list)
    {
        list.AddLast(&mNode);
    }

    ~ObjectWeight()
    {
        if (mNode.mNext)
        {
            mNode.Remove();
        }
    }

    static void* operator new(unsigned int)
    {
        return smFreeList.Alloc();
    }
    static void __cdecl operator delete(void* p);

    static FreeList smFreeList;

    PblListSingle::Node mNode;
    void* mObject;
};

 

class ObjectWeightList : public PblListSingle
{
public:
    ObjectWeightList();

    ~ObjectWeightList();

    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);
    bool ReadObjectWeights(PblFileChunk& chunk, unsigned int uiObjectID,
                           void const* (__cdecl* pfnFind)(unsigned int));

    void const* GetObjectA() const;

protected:

    const ObjectWeight* GetObjectNode(const ObjectWeight* const pExclude) const;

    const ObjectWeight* GetNextObjectNode(bool bRandomStart) const;

public:

    ObjectWeight* mPrevObject;
    unsigned char mSelectMode;
};

 

}
