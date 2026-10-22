#include "sndi3dl2reverbpreset.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// GLOBAL: SWBF2 0x007e3890
// SYNTHETIC: SWBF2 0x0040c190
// `dynamic initializer for 'Snd::I3DL2ReverbPreset::smFreeList''
// SYNTHETIC: SWBF2 0x0076a4b0
// `dynamic atexit destructor for 'Snd::I3DL2ReverbPreset::smFreeList''
FreeList I3DL2ReverbPreset::smFreeList(sizeof(I3DL2ReverbPreset), 1, 4, 1);

// GLOBAL: SWBF2 0x007e38ac
// SYNTHETIC: SWBF2 0x0040c1b0
// `dynamic initializer for 'Snd::I3DL2ReverbPreset::smList''
// SYNTHETIC: SWBF2 0x0076a4c0
// `dynamic atexit destructor for 'Snd::I3DL2ReverbPreset::smList''
PblListSingle I3DL2ReverbPreset::smList;

// FUNCTION: SWBF2 0x00744e10
void __cdecl I3DL2ReverbPreset::operator delete(void* p)
{
    smFreeList.Free(p);
}

// FUNCTION: SWBF2 0x00744e80
I3DL2ReverbPreset* __cdecl I3DL2ReverbPreset::Find(unsigned int uiHashID)
{
    for (PblListSingle::Node* pNode = smList.mHead.mNext; pNode != &smList.mHead;
         pNode = pNode->mNext)
    {
        I3DL2ReverbPreset* const pPreset = reinterpret_cast<I3DL2ReverbPreset*>(reinterpret_cast<char*>(pNode) - offsetof(I3DL2ReverbPreset, mNode));
        if (pPreset->mHashID == uiHashID)
        {
            return pPreset;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00744fb0
void I3DL2ReverbPreset::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uiID = child._pFile->Read32();
        if (uiID == 0x8D39BDE6u)
        {
            child._pFile->Read8();
            mHashID = child._pFile->Read32();
        }
        else
        {
            I3DL2Reverb::Read(child, uiID);
        }
    }
}

// FUNCTION: SWBF2 0x00744eb0
void __cdecl I3DL2ReverbPreset::Load(PblFileChunk& chunk, bool bOnce)
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

        I3DL2ReverbPreset* const pPreset = new I3DL2ReverbPreset;
        pPreset->Read(child);
        if (bOnce)
        {
            break;
        }
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00744e30
void __cdecl I3DL2ReverbPreset::DestroyAll()
{
    for (PblListSingle::Node* pNode = smList.mHead.mNext; pNode != &smList.mHead;)
    {
        I3DL2ReverbPreset* const pPreset =
            (I3DL2ReverbPreset*)((char*)pNode - offsetof(I3DL2ReverbPreset, mNode));
        pNode = pNode->mNext;
        delete pPreset;
    }
}
}

namespace Snd
{

I3DL2ReverbPreset::~I3DL2ReverbPreset()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}
}
