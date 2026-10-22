#include "sndobjectlist.h"

namespace Snd
{

// GLOBAL: SWBF2 0x007e3634
// SYNTHETIC: SWBF2 0x0040bae0
// `dynamic initializer for 'Snd::ObjectList::Node::smFreeList''
// SYNTHETIC: SWBF2 0x0076a360
// `dynamic atexit destructor for 'Snd::ObjectList::Node::smFreeList''
FreeList ObjectList::Node::smFreeList(8, 1, 16, 1);

// FUNCTION: SWBF2 0x007388d0
void ObjectList::Remove(void* pObject)
{

    Node* pNode = (Node*)mHead.mNext;
    for (;;)
    {
        if (pNode == (Node*)&mHead)
        {
            return;
        }

        void* const pStored = pNode->mObject;
        if (pStored == 0)
        {
            return;
        }
        if (pStored == pObject)
        {
            break;
        }

        pNode = (Node*)pNode->mNode.mNext;
    }

    if (pNode == 0)
    {
        return;
    }

    if (pNode->mNode.mNext != 0)
    {
        pNode->mNode.Remove();
    }

    Node::smFreeList.Free(pNode);
}

// FUNCTION: SWBF2 0x0052c6c0
ObjectList::~ObjectList()
{
    for (;;)
    {
        Node* const pNode = (Node*)mHead.mNext;

        if (pNode == (Node*)&mHead)
        {
            break;
        }

        if (pNode == 0)
        {
            continue;
        }

        if (pNode->mNode.mNext != 0)
        {
            pNode->mNode.Remove();
        }

        Node::smFreeList.Free(pNode);
    }

}

// FUNCTION: SWBF2 0x007388a0
ObjectList::Node* ObjectList::FindByObject(void* pObject)
{
    for (Node* pNode = (Node*)mHead.mNext; pNode != (Node*)&mHead;
         pNode = (Node*)pNode->mNode.mNext)
    {
        if (pNode->mObject == 0)
        {
            break;
        }
        if (pNode->mObject == pObject)
        {
            return pNode;
        }
    }
    return 0;
}

}
