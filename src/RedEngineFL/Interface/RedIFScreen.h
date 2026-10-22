#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblHashTable.h"

class RedScreenGroupElement;

class IFScreen
{
public:

    IFScreen(const char* pcName);
    void AddToScreen(RedScreenGroupElement* pElement);
    void Visible(bool bVisible);
    virtual ~IFScreen();

    // SYNTHETIC: SWBF2 0x006efc30
    // IFScreen::`scalar deleting destructor'

    static IFScreen* Find(unsigned int uiName);
    static IFScreen* Find(const char* pcName) { return Find(pcName ? (unsigned int)PblHash(pcName) : 0u); }
    const char* GetNameStr() const { return mNameStr; }
    void AddToRedScreen();
    void RemoveFromRedScreen();
    static void SetCurScreen(unsigned int uViewport, IFScreen* pScreen)
    {
        if (uViewport < 1)
        {
            spCurrentScreen[uViewport] = pScreen;
        }
    }

    static IFScreen* GetCurScreen(unsigned int uViewport)
    {
        return uViewport < 1 ? spCurrentScreen[uViewport] : 0;
    }
    void SetViewport(int iViewport) { mViewport = iViewport; }

    static void Reset()
    {
        sAllScreensList.RemoveAll();
        spCurrentScreen[0] = 0;
    }
    static IFScreen* spCurrentScreen[1];

    unsigned int mName;
    char mNameStr[64];
    int mViewport;
    RedScreenGroupElement* mElements[256];
    int mNumElements;
    PblList<IFScreen>::Node m_AllListNode;

    static PblList<IFScreen> sAllScreensList;
};
 
 
 
 
 
 
 
