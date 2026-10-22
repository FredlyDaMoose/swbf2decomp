#pragma once

class PblListSingle
{
public:
    class Node
    {
    public:

        Node()
        {
            mNext = 0;
        }

        void Remove()
        {
            Node* pPrev = this;
            while (pPrev->mNext != this)
            {
                pPrev = pPrev->mNext;
            }
            pPrev->mNext = mNext;
            mNext = 0;
        }

        Node* mNext;
    };

    PblListSingle()
    {
        mHead.mNext = &mHead;
    }

    ~PblListSingle()
    {
        mHead.mNext = 0;
    }

    bool IsEmpty() const
    {
        return mHead.mNext == &mHead;
    }

    void Clear()
    {
        mHead.mNext = &mHead;
    }

    void AddFirst(Node* pNode)
    {
        pNode->mNext = mHead.mNext;
        mHead.mNext = pNode;
    }
    void AddLast(Node* pNode)
    {
        Node* pPrev = &mHead;
        while (pPrev->mNext != &mHead)
        {
            pPrev = pPrev->mNext;
        }
        pNode->mNext = pPrev->mNext;
        pPrev->mNext = pNode;
    }

    class Itor
    {
    public:
        Itor(const PblListSingle& rList)
            : mpNode(rList.mHead.mNext), mpTerminator(&rList.mHead)
        {
        }
        Node* GetNode() { return mpNode; }
        bool IsDone() const { return mpNode == mpTerminator; }
        Itor& operator++()
        {
            mpNode = mpNode->mNext;
            return *this;
        }

        Node* mpNode;
        const Node* mpTerminator;
    };

    Node mHead;
};
 
