#pragma once

#include <stddef.h>

#include "PblListDouble.h"

class PblListDoubleSorted : public PblListDouble
{
public:
    class Node
    {
    public:

        Node(float fSortValue = 0.0f)
        {
            mSortValue = fSortValue;
        }

        // FUNCTION: SWBF2 0x004d26d0
        ~Node()
        {
            if (mNode.IsAttached())
            {
                mNode.Remove();
            }
        }

        void* GetObjectA(PblListDoubleSorted const* pList, unsigned int uOffset) const;

        float mSortValue;
        PblListDouble::Node mNode;
    };

    PblListDoubleSorted(bool bSortOnAdd = false)
    {
        mFlags = bSortOnAdd ? 1 : 0;
    }

    void SetSortOnAdd(bool bSortOnAdd)
    {
        if (bSortOnAdd)
        {
            mFlags |= 1;
        }
        else
        {
            mFlags = (unsigned char)(mFlags & ~1u);
        }
    }

    // FUNCTION: SWBF2 0x004d26c0
    ~PblListDoubleSorted()
    {
    }

    Node const* GetTerminator() const
    {
        return (Node const*)this;
    }

    // FUNCTION: SWBF2 0x004fbe10
    Node* GetFirst() const
    {
        if (mTerminator.mNext == &mTerminator)
        {
            return (Node*)GetTerminator();
        }
        return (Node*)((char*)mTerminator.mNext - offsetof(Node, mNode));
    }

    // FUNCTION: SWBF2 0x004d2640
    void Add(Node* pNode)
    {
        if (mFlags & 1)
        {
            for (PblListDouble::Node* pLink = mTerminator.mNext; pLink != &mTerminator;
                 pLink = pLink->mNext)
            {
                Node* pCur = (Node*)((char*)pLink - offsetof(Node, mNode));
                if (pCur == GetTerminator())
                {
                    break;
                }

                if (pNode->mSortValue > pCur->mSortValue)
                {
                    pNode->mNode.mNext = pLink;
                    pNode->mNode.mPrev = pLink->mPrev;
                    pLink->mPrev->mNext = &pNode->mNode;
                    pLink->mPrev = &pNode->mNode;
                    return;
                }
            }
        }

        AddLast(&pNode->mNode);
    }

    void Sort();

    unsigned char mFlags;
};
 
 
