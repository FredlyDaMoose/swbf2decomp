#pragma once

#include <stddef.h>

#include "RedInterfaceElement.h"

// VTABLE: SWBF2 0x007730e4

class RedGroupElement : public RedInterfaceElement
{
public:

    // FUNCTION: SWBF2 0x006d68f0
    RedGroupElement();

    // FUNCTION: SWBF2 0x006d68b0
    RedGroupElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x006d6990
    // RedGroupElement::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006d6930
    virtual ~RedGroupElement();

    // FUNCTION: SWBF2 0x006d6a60
    virtual void Enable(bool bEnable);

    // FUNCTION: SWBF2 0x006d6c10
    virtual void Visible(bool bVisible);

    // FUNCTION: SWBF2 0x006d6be0
    virtual void SetBlendMode(BlendModeT eBlendMode);

    // FUNCTION: SWBF2 0x00543eb0
    virtual bool IsGroup() const { return true; }

    // FUNCTION: SWBF2 0x006d6a30
    virtual void Cleanup();

    // FUNCTION: SWBF2 0x006d69c0
    virtual bool AddElement(RedInterfaceElement* pElement);

    // FUNCTION: SWBF2 0x006d6ae0
    virtual bool RemoveElement(RedInterfaceElement* pElement);

protected:
    // FUNCTION: SWBF2 0x006d6b20
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:

    // FUNCTION: SWBF2 0x006d6a90
    void RemoveAllElements();

    void PropagateAlpha(bool bPropagate) { mPropagateAlpha = bPropagate; }
    bool GetPropagateAlpha() const { return mPropagateAlpha; }
    PblList<RedInterfaceElement>& GetChildrenList() { return m_children; }

    PblList<RedInterfaceElement> m_children;
    bool mPropagateAlpha : 1;
};

 
 

 
