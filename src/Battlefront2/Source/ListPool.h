#pragma once

template <class TYPE, class INDEX>
class ListPool
{
public:
    ListPool(INDEX iPoolSize)
        : mElements(new TYPE[iPoolSize]), mLength(0), mPeak(0), mPoolSize(iPoolSize)
    {
    }

    ~ListPool() { delete[] mElements; }

    void Clear()
    {
        mLength = 0;
        mPeak = 0;
    }

    INDEX GetLength() const { return mLength; }
    bool IsFull() const { return mLength == mPoolSize; }
    TYPE& operator[](INDEX i) { return mElements[i]; }

    void Append(const TYPE& item)
    {
        ++mPeak;
        if (!IsFull())
        {
            mElements[mLength++] = item;
        }
    }

    void Remove(INDEX i)
    {
        --mLength;
        --mPeak;
        if (i < mLength)
        {
            mElements[i] = mElements[mLength];
        }
    }

    void RemoveObject(const TYPE& item)
    {
        for (INDEX i = 0; i < mLength; ++i)
        {
            if (mElements[i] == item)
            {
                Remove(i);
                return;
            }
        }
    }

    TYPE* mElements;
    INDEX mLength;
    INDEX mPeak;
    INDEX mPoolSize;
};
