#pragma once

template <class T, class KEY>
class PblHeap
{
public:
    struct RECORD
    {
        KEY mKey;
        T mObj;
    };

    PblHeap(int iMaxCount, KEY maxKey, void* pMem)
    {
        if (pMem)
        {
            mPool = (RECORD*)pMem;
            mAllocated = false;
        }
        else
        {
            mPool = new RECORD[iMaxCount + 1];
            mAllocated = true;
        }
        mMaxCount = iMaxCount;
        mPool[0].mKey = maxKey;
        mCount = 0;
    }

    void Flush() { mCount = 0; }

    KEY Value(int i) const { return mPool[i + 1].mKey; }
    bool IsEmpty() { return mCount == 0; }

    bool IsFull() { return mCount == mMaxCount; }

    T Get(int i) { return mPool[i + 1].mObj; }

    void Pop()
    {
        mPool[1] = mPool[mCount];
        --mCount;
        RECORD rec = mPool[1];
        int i = 1;
        while (i <= (mCount >> 1))
        {
            int iChild = i + i;
            if (iChild < mCount && mPool[iChild + 1].mKey > mPool[iChild].mKey)
            {
                ++iChild;
            }
            if (rec.mKey >= mPool[iChild].mKey)
            {
                break;
            }
            mPool[i] = mPool[iChild];
            i = iChild;
        }
        mPool[i] = rec;
    }

    ~PblHeap()
    {
        if (mAllocated)
        {
            delete[] mPool;
        }
    }

    void Push(T& obj, KEY key) { SortDown(++mCount, obj, key); }

    int GetCount() const { return mCount; }
    int GetMaxSize() const { return mMaxCount; }

    int FindLowest()
    {
        int lowest = mCount >> 1;
        for (int i = lowest + 1; i <= mCount; ++i)
        {
            if (mPool[i].mKey < mPool[lowest].mKey)
            {
                lowest = i;
            }
        }
        return lowest - 1;
    }

    void ReplaceLowest(T& obj, KEY key, int iLowest) { SortDown(iLowest + 1, obj, key); }

private:
    void SortDown(int i, T& obj, KEY key)
    {
        int iParent = i >> 1;
        while (key > mPool[iParent].mKey)
        {
            mPool[i] = mPool[iParent];
            i = iParent;
            iParent >>= 1;
        }
        mPool[i].mKey = key;
        mPool[i].mObj = obj;
    }

public:

    int mCount;
    int mMaxCount;
    RECORD* mPool;
    bool mAllocated;
};
