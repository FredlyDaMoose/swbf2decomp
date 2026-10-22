#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "sndfreelist.h"
#include "sndi3dl2reverb.h"

class PblFileChunk;

namespace Snd
{

class I3DL2ReverbPreset : public I3DL2Reverb
{
public:

    I3DL2ReverbPreset()
    {
        smList.AddLast(&mNode);
    }

    static void* operator new(unsigned int)
    {
        return smFreeList.Alloc();
    }
    static void __cdecl operator delete(void* p);

    ~I3DL2ReverbPreset();
    static void __cdecl DestroyAll();

    static void __cdecl Load(PblFileChunk& chunk, bool bOnce);
    static I3DL2ReverbPreset* __cdecl Find(unsigned int uiHashID);
    void Read(PblFileChunk& chunk);

    static FreeList smFreeList;

    static PblListSingle smList;

    unsigned int mHashID;
    PblListSingle::Node mNode;
};

 

}
