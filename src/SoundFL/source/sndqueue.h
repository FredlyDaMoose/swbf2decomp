#pragma once

namespace Snd
{

class QueueBase
{
public:

    void* Get(unsigned int uStride, char* pData, unsigned int uCapacity);

    bool IsFull() const;

    bool IsEmpty() const;

    const void* Peek(unsigned int uiElementSize, const void* const pElements,
                     unsigned short usCapacity,
                     unsigned short usIndex) const;

    void Empty(unsigned int uiElementSize, void* const pElements, unsigned short usCapacity,
               bool(__cdecl* const pfnCallback)(void*, void*), void* const pData);
    unsigned short GetNumItems(unsigned short usCapacity) const;
    void* Add(unsigned int uiElementSize, void* const pElements,
              unsigned short usCapacity);

    unsigned short mHead;
    unsigned short mTail;
    bool mFull : 1;
};
 

template <class T, int N>
class Queue : public QueueBase
{
public:

    // FUNCTION: SWBF2 0x00738930
    Queue()
    {
        mFull = false;
        mHead = 0;
        mTail = 0;
    }

    // FUNCTION: SWBF2 0x0073d940
    ~Queue()
    {
    }

    // FUNCTION: SWBF2 0x0073da80
    bool Add(const T& rElement)
    {
        if (IsFull())
        {
            return false;
        }

        mElements[mHead] = rElement;
        ++mHead;

        if (mHead == N)
        {
            mHead = 0;
        }

        mFull = (mHead == mTail);
        return true;
    }

    const T* Peek(unsigned short usIndex) const
    {
        return (const T*)QueueBase::Peek((unsigned int)sizeof(T), mElements,
                                         (unsigned short)N, usIndex);
    }

    void Get(T& rElement)
    {
        rElement = mElements[mTail];
        ++mTail;

        if (mTail == N)
        {
            mTail = 0;
        }

        mFull = false;
    }

    void Empty(bool(__cdecl* const pfnCallback)(void*, void*), void* const pData)
    {
        QueueBase::Empty((unsigned int)sizeof(T), mElements, (unsigned short)N, pfnCallback,
                         pData);
    }
    unsigned short GetNumItems() const { return QueueBase::GetNumItems((unsigned short)N); }

    T mElements[N];
};

class QueueLockBase
{
public:

    // FUNCTION: SWBF2 0x00745030
    QueueLockBase() { mLock = 0; }

    ~QueueLockBase();

    void Empty(QueueBase* const pQueue, unsigned int uiElementSize, void* const pElements,
               unsigned short usCapacity, bool(__cdecl* const pfnCallback)(void*, void*),
               void* const pData);
    void* Get(QueueBase* const pQueue, unsigned int uiElementSize,
              void* const pElements, unsigned int uiCapacity);
    void Unlock(QueueBase* const pQueue, const void* const pElement,
                unsigned int uiElementSize, void* const pElements,
                unsigned short usCapacity);

    unsigned short mLock;
};
 

template <class T, int N>
class QueueLock : public Queue<T, N>, public QueueLockBase
{
public:

    void Empty(bool(__cdecl* const pfnCallback)(void*, void*), void* const pData)
    {
        QueueLockBase::Empty(this, (unsigned int)sizeof(T), this->mElements, (unsigned short)N,
                             pfnCallback, pData);
    }

    T* Lock()
    {
        return (T*)QueueLockBase::Get(this, (unsigned int)sizeof(T), this->mElements, N);
    }

    void Unlock(const T* const pElement)
    {
        QueueLockBase::Unlock(this, pElement, (unsigned int)sizeof(T), this->mElements,
                              (unsigned short)N);
    }
};

template <class T, int N>
class QueueCircular : public Queue<T, N>
{
public:

    bool Add(const T& rElement)
    {
        if (this->IsFull())
        {
            this->mElements[this->mHead] = rElement;
            ++this->mHead;
            if (this->mHead == N)
            {
                this->mHead = 0;
            }
            this->mTail = this->mHead;
            return true;
        }

        this->mElements[this->mHead] = rElement;
        ++this->mHead;
        if (this->mHead == N)
        {
            this->mHead = 0;
        }
        this->mFull = (this->mHead == this->mTail);
        return false;
    }
};

}
