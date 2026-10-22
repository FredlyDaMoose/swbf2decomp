#include "sndobjectweightlist.h"

#include "sndglobals.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblRandom.h"

namespace Snd {
extern PblRandom sndRandom;
}

namespace Snd
{

// GLOBAL: SWBF2 0x007e3b5c
// SYNTHETIC: SWBF2 0x0040c610
// `dynamic initializer for 'Snd::ObjectWeight::smFreeList''
// SYNTHETIC: SWBF2 0x0076a500
// `dynamic atexit destructor for 'Snd::ObjectWeight::smFreeList''
FreeList ObjectWeight::smFreeList(sizeof(ObjectWeight), 1, 32, 1);

// FUNCTION: SWBF2 0x0074b880
void __cdecl ObjectWeight::operator delete(void* p)
{
    smFreeList.Free(p);
}

// FUNCTION: SWBF2 0x0074b7e0
ObjectWeightList::ObjectWeightList()
    : mPrevObject(0),
      mSelectMode(1)
{
}

// FUNCTION: SWBF2 0x0074ba20
bool ObjectWeightList::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    if (uiPropertyID != 0x4D5AF670u)
    {
        return false;
    }
    PblFile* const pFile = chunk._pFile;
    pFile->Read8();
    mSelectMode = (unsigned char)((int)Clamp(pFile->ReadFloat(), 0.0f, 3.0f) + 1);
    return true;
}

// FUNCTION: SWBF2 0x0074ba80
bool ObjectWeightList::ReadObjectWeights(PblFileChunk& chunk, unsigned int uiObjectID,
                                         void const* (__cdecl* pfnFind)(unsigned int))
{
    (void)uiObjectID;
    float fTotal = 0.0f;
    while (chunk.HasMoreChildren())
    {
        float fInterval = 0.0f;
        float fIntervalDev = 0.0f;
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* const pFile = child._pFile;
        pFile->Read32();
        const int iType = pFile->Read8();
        const unsigned int uiHash = (unsigned int)pFile->Read32();
        const float fWeight = pFile->ReadFloat();
        if (iType == 4)
        {
            fInterval = pFile->ReadFloat();
            fIntervalDev = pFile->ReadFloat();
        }

        void const* pObject = (void const*)uiHash;
        if (pfnFind)
        {
            pObject = pfnFind(uiHash);
        }

        ObjectWeight* const pWeight = new ObjectWeight(*this);
        if (!pWeight)
        {
            return false;
        }
        pWeight->mObject = (void*)pObject;
        pWeight->mPlayInterval = fInterval;
        pWeight->mPlayProbability = fWeight;
        fTotal += fWeight;
        pWeight->mPlayIntervalDev = fIntervalDev;
    }

    unsigned int uiCount = 0;
    for (PblListSingle::Node* pNode = mHead.mNext; pNode != &mHead; pNode = pNode->mNext)
    {
        ++uiCount;
    }
    if (!uiCount)
    {
        return false;
    }

    float fRemaining = fTotal;
    for (PblListSingle::Node* pNode = mHead.mNext; pNode != &mHead; pNode = pNode->mNext)
    {
        ObjectWeight* const pWeight = reinterpret_cast<ObjectWeight*>(reinterpret_cast<char*>(pNode) - offsetof(ObjectWeight, mNode));
        fRemaining -= pWeight->mPlayProbability;
        pWeight->mPlayProbability = fRemaining / fTotal;
    }
    return true;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074b7f0
ObjectWeightList::~ObjectWeightList()
{
    for (PblListSingle::Node* pNode = mHead.mNext; pNode != &mHead;)
    {
        ObjectWeight* const pWeight = (ObjectWeight*)((char*)pNode - offsetof(ObjectWeight, mNode));
        pNode = pNode->mNext;
        delete pWeight;
    }
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074b8a0
void const* ObjectWeightList::GetObjectA() const
{
    ObjectWeightList* const pThis = const_cast<ObjectWeightList*>(this);
    const ObjectWeight* pWeight = 0;
    switch (mSelectMode)
    {
    case 1:
        pWeight = GetObjectNode(0);
        break;
    case 2:
        pWeight = GetNextObjectNode(true);
        break;
    case 3:
        pWeight = GetNextObjectNode(false);
        break;
    case 4:
        pWeight = GetObjectNode(0);
        if (pWeight == mPrevObject)
        {
            pWeight = GetNextObjectNode(true);
        }
        break;
    }
    pThis->mPrevObject = const_cast<ObjectWeight*>(pWeight);

    const ObjectWeight* const pFirst = pWeight;
    if (!pWeight)
    {
        return 0;
    }
    while (!pWeight->DeterminePlaySource(true))
    {
        pWeight = GetNextObjectNode(false);
        pThis->mPrevObject = const_cast<ObjectWeight*>(pWeight);
        if (pFirst == pWeight || !pWeight)
        {
            return 0;
        }
    }
    return pWeight->mObject;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x0074b990
const ObjectWeight* ObjectWeightList::GetObjectNode(const ObjectWeight* const pExclude) const
{
    const float fRandom = sndRandom.GetFloat();
    const ObjectWeight* pPrev = 0;
    const ObjectWeight* pWeight = 0;
    for (const PblListSingle::Node* pNode = mHead.mNext; pNode != &mHead; pNode = pNode->mNext)
    {
        pPrev = pWeight;
        pWeight = pNode == &mHead ? 0 : reinterpret_cast<const ObjectWeight*>(reinterpret_cast<const char*>(pNode) - offsetof(ObjectWeight, mNode));
        if (fRandom >= pWeight->mPlayProbability && pWeight != pExclude)
        {
            return pWeight;
        }
    }
    return pWeight == pExclude ? pPrev : pWeight;
}
}

namespace Snd
{

const ObjectWeight* ObjectWeightList::GetNextObjectNode(bool bRandomStart) const
{
    const PblListSingle::Node* pNode;
    if (mPrevObject)
    {
        pNode = mPrevObject->mNode.mNext;
    }
    else if (bRandomStart)
    {
        const ObjectWeight* pWeight = GetObjectNode(0);
        pNode = pWeight ? &pWeight->mNode : 0;
    }
    else
    {
        pNode = mHead.mNext;
    }

    if (pNode == &mHead)
    {
        pNode = mHead.mNext;
    }
    if (!pNode)
    {
        return 0;
    }
    return pNode == &mHead ? 0 : reinterpret_cast<const ObjectWeight*>(reinterpret_cast<const char*>(pNode) - offsetof(ObjectWeight, mNode));
}
}
