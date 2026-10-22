#include "sndqueue.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00745050
void QueueLockBase::Empty(QueueBase* const pQueue, unsigned int uiElementSize,
                          void* const pElements, unsigned short usCapacity,
                          bool(__cdecl* const pfnCallback)(void*, void*), void* const pData)
{
    pQueue->Empty(uiElementSize, pElements, usCapacity, pfnCallback, pData);
    mLock = pQueue->mTail;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00745040
QueueLockBase::~QueueLockBase()
{
}
}
