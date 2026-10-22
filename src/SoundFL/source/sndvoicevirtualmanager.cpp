#include "sndvoicevirtualmanager.h"

#include <stddef.h>

#include "sndvoicebase.h"
#include "sndvoicevirtual.h"

namespace Snd
{

static inline PblListDoubleSorted::Node* NodeFromLink(const PblListDoubleSorted& list,
                                                      PblListDouble::Node* pLink)
{
    if (pLink == &list.mTerminator)
    {
        return (PblListDoubleSorted::Node*)list.GetTerminator();
    }
    return (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
}

// FUNCTION: SWBF2 0x00748900
VoiceBase* VoiceVirtualManager::GetFreeVoice()
{
    for (PblListSingle::Node* pLink = mVoiceList.mHead.mNext; pLink != &mVoiceList.mHead;
         pLink = pLink->mNext)
    {
        VoiceBase* pVoice = (VoiceBase*)((char*)pLink - offsetof(VoiceBase, mNode));
        if (!pVoice->GetLocked())
        {
            return pVoice;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00748920
void VoiceVirtualManager::InsertVoice(VoiceVirtual* pVoice)
{
    pVoice->mNode.mNode.Remove();
    pVoice->DetermineClosestListener();
    pVoice->CalculatePriority(false);
    mActiveList.Add(&pVoice->mNode);
}

// FUNCTION: SWBF2 0x00748990
void VoiceVirtualManager::RemoveVoice(VoiceVirtual* pVoice)
{
    if (pVoice->mVoice)
    {
        pVoice->mVoice->SourceTransport::Stop();
        pVoice->mVoice->SetSample(0);
        pVoice->SetVoice(0);
    }
    pVoice->mNode.mNode.Remove();
    mInactiveList.Add(&pVoice->mNode);
}

// FUNCTION: SWBF2 0x00748a10
void VoiceVirtualManager::Update(float fDeltaTime)
{
    unsigned int uiNumActive = 0;

    PblListDouble::Node* pLink = mInactiveList.mTerminator.mNext;
    for (;;)
    {
        PblListDoubleSorted::Node* pNode = NodeFromLink(mInactiveList, pLink);
        if (pNode == mInactiveList.GetTerminator())
        {
            break;
        }
        VoiceVirtual* pVoice = pNode == mInactiveList.GetTerminator()
            ? 0
            : (VoiceVirtual*)((char*)pNode - offsetof(VoiceVirtual, mNode));
        pLink = pLink->mNext;
        pVoice->Update(fDeltaTime);
    }

    for (pLink = mActiveList.mTerminator.mNext; pLink != &mActiveList.mTerminator;
         pLink = pLink->mNext)
    {
        PblListDoubleSorted::Node* pNode = NodeFromLink(mActiveList, pLink);
        if (pNode == mActiveList.GetTerminator())
        {
            break;
        }
        VoiceVirtual* pVoice = (VoiceVirtual*)((char*)pNode - offsetof(VoiceVirtual, mNode));
        pVoice->DetermineClosestListener();
        pVoice->CalculatePriority(false);
        ++uiNumActive;
    }

    if (uiNumActive > mNumManagedVoices)
    {
        mActiveList.Sort();
    }

    for (pLink = &NodeFromLink(mActiveList, mActiveList.mTerminator.mPrev)->mNode;
         pLink != &mActiveList.mTerminator;)
    {
        PblListDoubleSorted::Node* pNode = NodeFromLink(mActiveList, pLink);
        if (pNode == mActiveList.GetTerminator())
        {
            break;
        }
        --uiNumActive;
        VoiceVirtual* pVoice = pNode == mActiveList.GetTerminator()
            ? 0
            : (VoiceVirtual*)((char*)pNode - offsetof(VoiceVirtual, mNode));
        pLink = pLink->mPrev;
        if (uiNumActive < mNumManagedVoices)
        {
            if (!pVoice->mVoice)
            {
                pVoice->SetVoice(GetFreeVoice());
            }
        }
        else if (pVoice->mVoice)
        {
            pVoice->mVoice->SourceTransport::Stop();
            pVoice->SetVoice(0);
        }
        pVoice->Update(fDeltaTime);
    }
}

}

// FUNCTION: SWBF2 0x00748bf0
extern "C" void __fastcall ClearUnsignedWord(unsigned int* pValue)
{
    *pValue = 0;
}
