#include "FoleyFX.h"
#include "FoleyFXOrdnance.h"
#include "FoleyFXSoldier.h"
#include "FoleyFXWalker.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// GLOBAL: SWBF2 0x01ebc2bc
// SYNTHETIC: SWBF2 0x00403a90
// `dynamic initializer for 'FoleyFX::rttiHashFoleyFX''
PblHash FoleyFX::rttiHashFoleyFX("FoleyFX");
char FoleyFX::rttiNameFoleyFX[] = "FoleyFX";

// GLOBAL: SWBF2 0x007eb8c8
// SYNTHETIC: SWBF2 0x00403a80
// `dynamic initializer for 'FoleyFX::smList''
// SYNTHETIC: SWBF2 0x007687c0
// `dynamic atexit destructor for 'FoleyFX::smList''
PblListSingle FoleyFX::smList;

// FUNCTION: SWBF2 0x0052c040
bool FoleyFX::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashFoleyFX;
}

// FUNCTION: SWBF2 0x0052c060
unsigned int FoleyFX::GetDerivedRtti() const
{
    return rttiHashFoleyFX;
}

// FUNCTION: SWBF2 0x0052c070
const char* FoleyFX::GetDerivedRttiName() const
{
    return rttiNameFoleyFX;
}

// FUNCTION: SWBF2 0x0052c090
FoleyFX::FoleyFX()
    : mID(0), mClass(0)
{
    smList.AddFirst(&mNode);
}

// FUNCTION: SWBF2 0x0052c0f0
FoleyFX::~FoleyFX()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// SYNTHETIC: SWBF2 0x0052c0c0
// FoleyFX::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052c120
unsigned int FoleyFX::GetID() const
{
    return mID;
}

// FUNCTION: SWBF2 0x0052c130
unsigned int FoleyFX::GetClassID() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x0052c140
void FoleyFX::ReadChunk(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (!Read(child, child._pFile->Read32()))
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0052c1a0
bool FoleyFX::Read(PblFileChunk& chunk, unsigned int uChunkID)
{
    if (uChunkID == 0x8d39bde6)
    {
        chunk._pFile->Read8();
        mID = chunk._pFile->Read32();
        return true;
    }
    if (uChunkID == 0xab3e0bff)
    {
        chunk._pFile->Read8();
        mClass = chunk._pFile->Read32();
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0052c200
void FoleyFX::Load(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        FoleyFX* pFX;
        switch ((unsigned int)child._pFile->Read32())
        {
        case 0xa1c9cbc5:
        {
            FoleyFXGroup* pGroup = new FoleyFXGroup;
            child = chunk.ReadNextChild();
            pGroup->Read(child);
            continue;
        }
        case 0xd098618c:
            pFX = new FoleyFXSoldier;
            break;
        case 0x5e9b9948:
            pFX = new FoleyFXWalker;
            break;
        case 0xfd545f86:
            pFX = new FoleyFXImpact;
            break;
        default:
            continue;
        }
        child = chunk.ReadNextChild();
        pFX->ReadChunk(child);
    }
}

// FUNCTION: SWBF2 0x0052c390
FoleyFX* FoleyFX::FindByID(unsigned int uID)
{
    for (PblListSingle::Node* pNode = smList.mHead.mNext; pNode != &smList.mHead;
         pNode = pNode->mNext)
    {
        FoleyFX* pFX = (FoleyFX*)((char*)pNode - offsetof(FoleyFX, mNode));
        if (pFX->GetID() == uID)
        {
            return pFX;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0052c3c0
void FoleyFX::DestroyAll()
{
    while (smList.mHead.mNext != &smList.mHead)
    {
        FoleyFX* pFX = (FoleyFX*)((char*)smList.mHead.mNext - offsetof(FoleyFX, mNode));
        delete pFX;
    }
    smList.mHead.mNext = &smList.mHead;
}
