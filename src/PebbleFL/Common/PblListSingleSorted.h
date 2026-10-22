#pragma once

#include "PblListSingle.h"

class PblListSingleSorted : public PblListSingle
{
public:
    enum Flags
    {
        flags_sortOnAdd = 1
    };

    class Node
    {
    public:

        explicit Node(float fSortValue) : mSortValue(fSortValue) {}

        float mSortValue;
        PblListSingle::Node mNode;
    };

    explicit PblListSingleSorted(bool bSortOnAdd)
    {
        mFlags = bSortOnAdd ? (unsigned char)flags_sortOnAdd : (unsigned char)0;
    }

    const Node* GetTerminator() const { return (const Node*)this; }

    void Add(Node* pNode);

    unsigned char mFlags;
};

 
 
