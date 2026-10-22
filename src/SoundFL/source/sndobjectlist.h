#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "sndfreelist.h"

namespace Snd
{

class ObjectList : public PblListSingle
{
public:

    class Node
    {
    public:

        static FreeList smFreeList;

        PblListSingle::Node mNode;
        void* mObject;
    };

    void Remove(void* pObject);

    Node* FindByObject(void* pObject);

    // FUNCTION: SWBF2 0x0052c740
    void Add(void* pObject)
    {
        Node* const pNode = (Node*)Node::smFreeList.Alloc();
        if (pNode)
        {
            pNode->mNode.mNext = 0;
            pNode->mObject = pObject;
            AddLast(&pNode->mNode);
        }
    }

    void AddBefore(Node* pBefore, void* pObject)
    {
        Node* pNode = (Node*)Node::smFreeList.Alloc();
        if (pNode)
        {
            pNode->mNode.mNext = 0;
            pNode->mObject = pObject;
        }
        PblListSingle::Node* pPrev = &pBefore->mNode;
        while (pPrev->mNext != &pBefore->mNode)
        {
            pPrev = pPrev->mNext;
        }
        pNode->mNode.mNext = pPrev->mNext;
        pPrev->mNext = &pNode->mNode;
    }

    ObjectList() {}

    ~ObjectList();

    Node* FindByIndex(unsigned int uiIndex) const;
    unsigned int GetCount() const;
};

 
 

}
