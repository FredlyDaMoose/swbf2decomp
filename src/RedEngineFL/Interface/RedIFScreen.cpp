#include "RedIFScreen.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedInterfaceScreen.h"

// GLOBAL: SWBF2 0x007e1410
PblList<IFScreen> IFScreen::sAllScreensList;

// GLOBAL: SWBF2 0x009ace70
IFScreen* IFScreen::spCurrentScreen[1];

// FUNCTION: SWBF2 0x006efbe0
IFScreen::~IFScreen()
{
    sAllScreensList.Remove(&m_AllListNode);
}

// FUNCTION: SWBF2 0x006efb20
IFScreen::IFScreen(const char* pcName)
{
    mName = pcName ? (unsigned int)PblHash(pcName) : 0;
    memset(mNameStr, 0, sizeof(mNameStr));
    strlcpy(mNameStr, pcName, sizeof(mNameStr));
    memset(mElements, 0, sizeof(mElements));
    mNumElements = 0;
    mViewport = -1;
    sAllScreensList.AddLast(&m_AllListNode, this);
}

// FUNCTION: SWBF2 0x006efc60
void IFScreen::AddToRedScreen()
{
    for (int i = 0; i < mNumElements; i++)
    {
        RedScreenGroupElement* pElement = mElements[i];
        pElement->Visible(true);
        if (mViewport == -1)
        {
            FLRenderer::AddInterfaceElement(pElement, pElement->m_fScreenRelativeX,
                                            pElement->m_fScreenRelativeY, pElement->m_bUseSafeZone);
        }
        else
        {
            FLRenderer::AddInterfaceElement(mViewport, pElement, pElement->m_fScreenRelativeX,
                                            pElement->m_fScreenRelativeY, pElement->m_bUseSafeZone);
        }
    }
}

// FUNCTION: SWBF2 0x006efce0
void IFScreen::AddToScreen(RedScreenGroupElement* pElement)
{
    if (mNumElements < 256)
    {
        mElements[mNumElements++] = pElement;
    }
}

// FUNCTION: SWBF2 0x006efd10
IFScreen* IFScreen::Find(unsigned int uiName)
{
    for (PblList<IFScreen>::Itor it = sAllScreensList.Begin(); it != sAllScreensList.End(); ++it)
    {
        if (it->mName == uiName)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006efd50
void IFScreen::RemoveFromRedScreen()
{
    for (int i = 0; i < mNumElements; i++)
    {
        mElements[i]->Visible(false);
        if (mViewport == -1)
        {
            FLRenderer::RemoveInterfaceElement(mElements[i]);
        }
        else
        {
            FLRenderer::RemoveInterfaceElement(mViewport, mElements[i]);
        }
    }
}

// FUNCTION: SWBF2 0x006efda0
void IFScreen::Visible(bool bVisible)
{
    for (int i = 0; i < mNumElements; i++)
    {
        mElements[i]->Visible(bVisible);
    }
}
