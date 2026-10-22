#pragma once

#include "PebbleFL/Common/PblList.h"

template <class T>
class FLList
{
public:

    FLList(int iSize);

    virtual ~FLList();

    void AddLast(const T& item)
    {
        typename PblList<T>::Node* pNode = mFreeList.GetLast();
        *pNode->_pObject = item;
        T* pObject = pNode->_pObject;
        mFreeList.Remove(pNode);
        mUsedList.AddLast(pNode, pObject);
    }

    void RemoveFirst()
    {
        if (mUsedList.GetCount() > 0)
        {
            typename PblList<T>::Node* pNode = mUsedList.GetFirst();
            T* pObject = pNode->_pObject;
            mUsedList.Remove(pNode);
            mFreeList.AddLast(pNode, pObject);
        }
    }

    const PblList<T>& GetList() const { return mUsedList; }

    PblList<T> mUsedList;
    PblList<T> mFreeList;
    typename PblList<T>::Node* mNodes;
    T* mData;
};

// FUNCTION: SWBF2 0x00529900
template <class T>
FLList<T>::FLList(int iSize)
{
    mNodes = new typename PblList<T>::Node[iSize];
    mData = new T[iSize];
    for (int i = 0; i < iSize; ++i)
    {
        mFreeList.AddLast(&mNodes[i], &mData[i]);
    }
}

// FUNCTION: SWBF2 0x00529a00
template <class T>
FLList<T>::~FLList()
{
    mUsedList.RemoveAll();
    mFreeList.RemoveAll();
    if (mNodes)
    {
        delete[] mNodes;
    }
    if (mData)
    {
        delete[] mData;
    }
}

// SYNTHETIC: SWBF2 0x00529a90
// FLList<GuiManager::Event *>::`scalar deleting destructor'
