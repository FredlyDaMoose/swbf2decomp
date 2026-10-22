#pragma once

class PblListDouble
{
public:
    class Node
    {
    public:

        Node()
        {
            mNext = 0;
            mPrev = 0;
        }

        bool IsAttached() const
        {
            return mNext != 0 && mPrev != 0;
        }

        // FUNCTION: SWBF2 0x004d0d30
        void Remove()
        {
            mNext->mPrev = mPrev;
            mPrev->mNext = mNext;
            mPrev = 0;
            mNext = 0;
        }

        Node* mNext;
        Node* mPrev;
    };

    PblListDouble()
    {
        mTerminator.mNext = &mTerminator;
        mTerminator.mPrev = &mTerminator;
    }

    ~PblListDouble()
    {
        mTerminator.mNext = 0;
        mTerminator.mPrev = 0;
    }

    void AddFirst(Node* pNode)
    {
        Node* pAt = mTerminator.mNext;
        pNode->mNext = pAt;
        pNode->mPrev = pAt->mPrev;
        pAt->mPrev->mNext = pNode;
        pAt->mPrev = pNode;
    }

    // FUNCTION: SWBF2 0x004d0d60
    void Clear()
    {
        while (mTerminator.mNext != &mTerminator)
        {
            mTerminator.mNext->Remove();
        }
    }

    unsigned int GetCount() const
    {
        unsigned int uCount = 0;
        for (const Node* pNode = mTerminator.mNext; pNode != &mTerminator; pNode = pNode->mNext)
        {
            ++uCount;
        }
        return uCount;
    }

    void AddLast(Node* pNode)
    {
        pNode->mNext = &mTerminator;
        pNode->mPrev = mTerminator.mPrev;
        mTerminator.mPrev->mNext = pNode;
        mTerminator.mPrev = pNode;
    }

    Node mTerminator;
};
 
