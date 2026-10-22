#pragma once

#include "PebbleFL/Common/PblRef.h"

namespace EventManager
{

class Node : public PblRef
{
public:

    virtual ~Node();

    virtual const char* GetName();

    Node() : mParent(0), mChild(0), mSibling(0)
    {
    }

    void AttachToParent(Node* pParent);
    void DetachFromParent();

    void Clear();

    Node* mParent;
    Node* mChild;
    Node* mSibling;
};

void Init();
void Cleanup();
}
