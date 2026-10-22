#include "RedMoviePlayerBase.h"

#include <stdio.h>
#include <windows.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include <new>

PblList<RedMoviePlayerBase> RedMoviePlayerBase::sMovieList;

// GLOBAL: SWBF2 0x007e272c
PblList<RedMoviePlayerBase::Info> RedMoviePlayerBase::Info::sInfoList;

// FUNCTION: SWBF2 0x00725220
RedMoviePlayerBase::RedMoviePlayerBase(unsigned int uiMediaSectorSize)
{
    mTextures = 0;
    mNumTextures = 0;
    mNotifyCallback = 0;
    mNotifyCallbackData = 0;
    mBufferingCallback = 0;
    mBufferingCallbackData = 0;
    mInfo = 0;
    mCurrentSegment = 0;

    mCurrentPlayState = playState_stopped;
    mDesiredPlayState = playState_stopped;

    mInvalidStartData = 0;
    mReadBuffer = 0;

    mMediaSectorSize = uiMediaSectorSize;
    mFileReadSize = 0;
    mFileScheduledReadSize = 0;
    mNumReallocTextures = 0;
    mFlags = 0;

    sMovieList.AddLast(&mNode, this);
}

// FUNCTION: SWBF2 0x007263f0
void RedMoviePlayerBase::SetNotifyCallback(NotifyCallback pfnCallback, void* pData)
{
    mNotifyCallback = pfnCallback;
    mNotifyCallbackData = pData;
}

// FUNCTION: SWBF2 0x00725590
void RedMoviePlayerBase::TextureQueue::Add(RedTexture** const ppTextures,
                                           RedTexture* const pTexture,
                                           unsigned int uiSize)
{
    ppTextures[mHead] = pTexture;
    ++mHead;
    ++mUsed;
    if (mHead == uiSize)
    {
        mHead = 0;
    }
}

// FUNCTION: SWBF2 0x007255c0
void RedMoviePlayerBase::Info::AddRef()
{
    ++mRefCount;
}

// FUNCTION: SWBF2 0x007255d0
void* RedMoviePlayerBase::BufferGetBuffer() const
{
    return mBuffer.GetBuffer();
}

// FUNCTION: SWBF2 0x007255e0
unsigned int RedMoviePlayerBase::BufferGetUsed() const
{
    return mBuffer.GetUsed();
}

// FUNCTION: SWBF2 0x007255f0
unsigned int RedMoviePlayerBase::BufferReadData(void* pDest, unsigned int uiSize, bool bUpdate)
{
    unsigned int uiRead = mBuffer.ReadData(pDest, uiSize, bUpdate);
    if (mReadBuffer == &mBufferStart && bUpdate)
    {
        mBufferStart.UpdateReadPos(uiRead);
        mInvalidStartData -= uiRead;
    }
    return uiRead;
}

// FUNCTION: SWBF2 0x00725640
void RedMoviePlayerBase::Close()
{
    ResetStart();
    Flush();
    CloseHeader();
    mFile.Close();
    mBuffer.SetBuffer(0, 0);
    mBufferStart.SetBuffer(0, 0);
    mInvalidStartData = 0;
    mFreeTextures.Empty();
    mUsedTextures.Empty();
    mFlags &= ~flags_Reading;
    mReadBuffer = 0;
    mTextures = 0;
    mNumTextures = 0;
    mCurrentSegment = 0;
    mFileReadSize = 0;
    mFileScheduledReadSize = 0;
}

// FUNCTION: SWBF2 0x007256e0
void RedMoviePlayerBase::CloseHeader()
{
    Info::Release(mInfo);
    mInfo = 0;
}

// FUNCTION: SWBF2 0x00725700
unsigned int RedMoviePlayerBase::CreateTextureHash(unsigned int uiIndex) const
{
    char acName[128];
    _snprintf(acName, sizeof(acName), "RedMoviePlayer_0x%08x_0x%08x\n", this, uiIndex);
    return PblHash(acName);
}

// FUNCTION: SWBF2 0x00725760
bool RedMoviePlayerBase::CreateTextures(unsigned int uiWidth, unsigned int uiHeight,
                                        unsigned int uiFormat, unsigned char ucFlags)
{
    RedTexture** ppAlloc = GetAllocTextures();
    RedTexture** ppFree = GetFreeTextures();

    for (unsigned int i = 0; i < mNumTextures; ++i)
    {
        if (ppAlloc[i] == 0)
        {
            RedTexture* pTexture = AllocTexture((ucFlags & 1) ? CreateTextureHash(i) : 0,
                                                uiWidth, uiHeight, uiFormat, i, ucFlags);
            if (pTexture == 0)
            {
                return false;
            }

            if (mTextures[i] == 0)
            {
                mTextures[i] = pTexture;
                mFreeTextures.Push(ppFree, mTextures[i], mNumTextures);
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00725800
void RedMoviePlayerBase::DestroyTextures(bool bForce, unsigned char ucFlags)
{
    RedTexture** ppAlloc = GetAllocTextures();
    mNumReallocTextures = 0;

    for (unsigned int i = 0; i < mNumTextures; ++i)
    {
        RedTexture* pTexture = mTextures[i];
        bool bUsed = TextureUsed(pTexture);

        if (!bForce && bUsed)
        {
            ppAlloc[i] = pTexture;
            ++mNumReallocTextures;
        }
        else
        {
            FreeTexture(pTexture, i, ucFlags);
            if (ucFlags & 1)
            {
                if (!bUsed)
                {
                    mFreeTextures.Remove(GetFreeTextures(), mTextures[i]);
                }
                mTextures[i] = 0;
            }
            ppAlloc[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x007258a0
void RedMoviePlayerBase::TextureQueue::Empty()
{
    mUsed = 0;
}

// FUNCTION: SWBF2 0x007258b0
void RedMoviePlayerBase::TextureStack::Empty()
{
    mHeight = 0;
}

// FUNCTION: SWBF2 0x007258c0
bool RedMoviePlayerBase::TextureStack::Exists(RedTexture** const ppTextures,
                                              const RedTexture* const pTexture) const
{
    for (unsigned int i = 0; i < mHeight; ++i)
    {
        if (ppTextures[i] == pTexture)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x007258f0
const RedMoviePlayerBase::Info* RedMoviePlayerBase::Info::FindByHashID(unsigned int uiHashID)
{
    for (PblList<Info>::Itor it(sInfoList); it != sInfoList._head._pObject; ++it)
    {
        if (it->GetHashID() == uiHashID)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00725940
const RedMoviePlayerBase::Segment* RedMoviePlayerBase::Info::FindSegmentByHashID(unsigned int uiHashID) const
{
    for (unsigned int i = 0; i < mNumSegments; ++i)
    {
        if (mSegments[i].mHashID == uiHashID)
        {
            return &mSegments[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00725980
const RedMoviePlayerBase::Segment* RedMoviePlayerBase::Info::FindSegmentByIndex(unsigned int uiIndex) const
{
    return &mSegments[uiIndex];
}

// FUNCTION: SWBF2 0x007259a0
void RedMoviePlayerBase::Flush()
{
    if (mFlags & flags_Reading)
    {
        unsigned int uiTimeout = 1500;
        while (!mFile.FileOperationComplete() && uiTimeout-- != 0)
        {
            Sleep(1);
        }
        mFlags &= ~flags_Reading;
        mFileScheduledReadSize = 0;
    }

    ResetStart();
    mBuffer.Flush();
    mFlags &= ~flags_BufferFull;

    RedTexture* pTexture;
    while ((pTexture = mUsedTextures.Get(GetUsedTextures(), mNumTextures)) != 0)
    {
        mFreeTextures.Push(GetFreeTextures(), pTexture, mNumTextures);
    }
}

// FUNCTION: SWBF2 0x00725a60
RedTexture* RedMoviePlayerBase::TextureQueue::Get(RedTexture** const ppTextures,
                                                  unsigned int uiSize)
{
    if (mUsed == 0)
    {
        return 0;
    }

    unsigned int uiIndex;
    if (mHead < mUsed)
    {
        uiIndex = mHead - mUsed + uiSize;
    }
    else
    {
        uiIndex = mHead - mUsed;
    }
    --mUsed;
    return ppTextures[uiIndex];
}

// FUNCTION: SWBF2 0x00726330
void RedMoviePlayerBase::ResetStart()
{
    if (mReadBuffer == &mBufferStart)
    {
        mBufferStart.UpdateReadPos(mInvalidStartData);
        mInvalidStartData = 0;
        mBuffer = mBufferStart;
        mFlags &= ~(flags_End | flags_Discontinuity | flags_BufferingEnd);
        mReadBuffer = &mBuffer;
        mBufferStart.Flush();
    }
}

// FUNCTION: SWBF2 0x00725aa0
RedTexture** RedMoviePlayerBase::GetAllocTextures() const
{
    return mTextures + mNumTextures * 3;
}

// FUNCTION: SWBF2 0x00725b00
RedTexture** RedMoviePlayerBase::GetFreeTextures() const
{
    return mTextures + mNumTextures;
}

// FUNCTION: SWBF2 0x00725b10
unsigned int RedMoviePlayerBase::Info::GetHashID() const
{
    return mHashID;
}

// FUNCTION: SWBF2 0x00725b20
const RedMoviePlayerBase::Info* RedMoviePlayerBase::GetInfo() const
{
    return mInfo;
}

// FUNCTION: SWBF2 0x00725b80
RedMoviePlayerBase::PlayState RedMoviePlayerBase::GetPlayState() const
{
    return mCurrentPlayState;
}

// FUNCTION: SWBF2 0x00725ba0
int RedMoviePlayerBase::Info::GetRedHeap() const
{
    return mRedMemoryHeap;
}

// FUNCTION: SWBF2 0x00725f50
void RedMoviePlayerBase::TextureStack::Push(RedTexture** const ppTextures,
                                            RedTexture* const pTexture, unsigned int uiSize)
{
    ppTextures[mHeight++] = pTexture;
}

// FUNCTION: SWBF2 0x007262c0
void RedMoviePlayerBase::TextureStack::Remove(RedTexture** const ppTextures,
                                              RedTexture* const pTexture)
{
    unsigned int i = mHeight;
    while (i != 0)
    {
        --i;
        if (ppTextures[i] == pTexture)
        {
            for (; i < mHeight - 1; ++i)
            {
                ppTextures[i] = ppTextures[i + 1];
            }
            --mHeight;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00726520
bool RedMoviePlayerBase::TextureUsed(const RedTexture* const pTexture) const
{
    return !mFreeTextures.Exists(GetFreeTextures(), pTexture);
}

// FUNCTION: SWBF2 0x00725ab0
const RedMoviePlayerBase::Segment* RedMoviePlayerBase::GetCurrentSegment() const
{
    return mCurrentSegment;
}

// FUNCTION: SWBF2 0x00725b30
bool RedMoviePlayerBase::GetLooping() const
{
    return (mFlags & flags_Loop) != 0;
}

// FUNCTION: SWBF2 0x00725b50
unsigned int RedMoviePlayerBase::GetNumReallocTextures() const
{
    return mNumReallocTextures;
}

// FUNCTION: SWBF2 0x00725b70
unsigned int RedMoviePlayerBase::GetNumUsedTextures() const
{
    return mUsedTextures.mUsed;
}

// FUNCTION: SWBF2 0x00725f10
void RedMoviePlayerBase::Play()
{
    mDesiredPlayState = playState_playing;
}

// FUNCTION: SWBF2 0x00725ac0
unsigned int RedMoviePlayerBase::GetDataLeft() const
{
    unsigned int uiEnd = mCurrentSegment->mStartPos + mCurrentSegment->mLength;
    unsigned int uiPos = mFile.mFile.GetPos();
    return uiEnd > uiPos ? uiEnd - mFile.mFile.GetPos() : 0;
}

// FUNCTION: SWBF2 0x00725bb0
RedMoviePlayerBase::Segment* RedMoviePlayerBase::SegmentSequence::GetSegment() const
{
    return mSegment;
}

// FUNCTION: SWBF2 0x00725f20
bool RedMoviePlayerBase::SegmentSequence::PlayAtEnd() const
{
    return (mFlags & flags_playAtEnd) != 0;
}

// FUNCTION: SWBF2 0x00726410
void RedMoviePlayerBase::SegmentSequence::Setup(
    Segment* pSegment, bool bPlayAtEnd)
{
    mSegment = pSegment;
    if (bPlayAtEnd)
    {
        mFlags |= flags_playAtEnd;
    }
    else
    {
        mFlags &= ~flags_playAtEnd;
    }
}

// FUNCTION: SWBF2 0x00725c40
const RedTexture* RedMoviePlayerBase::LockTexture()
{
    return mUsedTextures.Get(GetUsedTextures(), mNumTextures);
}

// FUNCTION: SWBF2 0x00725c60
void RedMoviePlayerBase::Notify(NotifyType eType)
{
    if (mNotifyCallback)
    {
        mNotifyCallback(this, eType, mNotifyCallbackData);
    }
}

// FUNCTION: SWBF2 0x00725b90
bool RedMoviePlayerBase::GetReadBufferFull() const
{
    return (mFlags >> 2) & 1;
}

// FUNCTION: SWBF2 0x00725bc0
RedTexture* RedMoviePlayerBase::GetTexture()
{
    return mFreeTextures.Peek(GetFreeTextures());
}

// FUNCTION: SWBF2 0x00725bf0
bool RedMoviePlayerBase::IsDiscontinuity() const
{
    return (mFlags >> 4) & 1;
}

// FUNCTION: SWBF2 0x00725c00
bool RedMoviePlayerBase::IsEnd() const
{
    return (mFlags >> 3) & 1;
}

// FUNCTION: SWBF2 0x00725ee0
void RedMoviePlayerBase::Pause()
{
    mDesiredPlayState = playState_paused;
}

// FUNCTION: SWBF2 0x00725ef0
RedTexture* RedMoviePlayerBase::TextureStack::Peek(RedTexture** const ppTextures)
{
    if (!mHeight)
    {
        return 0;
    }
    return ppTextures[mHeight - 1];
}

// FUNCTION: SWBF2 0x00725f30
RedTexture* RedMoviePlayerBase::TextureStack::Pop(RedTexture** const ppTextures)
{
    if (!mHeight)
    {
        return 0;
    }
    return ppTextures[--mHeight];
}

// FUNCTION: SWBF2 0x007264f0
void RedMoviePlayerBase::TextureComplete(RedTexture* const pTexture)
{
    (void)pTexture;
    mUsedTextures.Add(GetUsedTextures(), mFreeTextures.Pop(GetFreeTextures()), mNumTextures);
}

// FUNCTION: SWBF2 0x00726570
void RedMoviePlayerBase::UnlockTexture(const RedTexture* const pTexture)
{
    mFreeTextures.Push(GetFreeTextures(), const_cast<RedTexture*>(pTexture),
                       mNumTextures);
}

// TEMPLATE: SWBF2 0x00725c20
// Snd::QueueLock<RedMoviePlayerBase::SegmentSequence,10>::Lock
template RedMoviePlayerBase::SegmentSequence*
Snd::QueueLock<RedMoviePlayerBase::SegmentSequence, 10>::Lock();

// FUNCTION: SWBF2 0x00726390
void RedMoviePlayerBase::SeekSegment(const Segment* const pSegment, bool bPlayAtEnd)
{
    SegmentSequence* pSequence = (SegmentSequence*)mSegmentQueue.Snd::QueueBase::Add(
        sizeof(SegmentSequence), mSegmentQueue.mElements, 10);
    if (pSequence)
    {
        pSequence->Setup(const_cast<Segment*>(pSegment), bPlayAtEnd);
    }
}

// TEMPLATE: SWBF2 0x00726550
// Snd::QueueLock<RedMoviePlayerBase::SegmentSequence,10>::Unlock
template void Snd::QueueLock<RedMoviePlayerBase::SegmentSequence, 10>::Unlock(
    const RedMoviePlayerBase::SegmentSequence* const);

// FUNCTION: SWBF2 0x007261c0
void RedMoviePlayerBase::ReallocTextures(unsigned int uiWidth,
                                         unsigned int uiHeight,
                                         unsigned int uiFormat,
                                         unsigned char ucFlags)
{
    RedTexture** ppAlloc = GetAllocTextures();
    for (unsigned int i = 0; i < mNumTextures; ++i)
    {
        RedTexture* pOldAlloc = ppAlloc[i];
        if (!pOldAlloc || TextureUsed(pOldAlloc))
        {
            continue;
        }

        RedTexture* pCurrent = mTextures[i];
        if (pCurrent)
        {
            mFreeTextures.Remove(GetFreeTextures(), pCurrent);
            FreeTexture(pCurrent, i, ucFlags);
        }

        pCurrent = AllocTexture(CreateTextureHash(i), uiWidth, uiHeight,
                                uiFormat, i, ucFlags);
        mTextures[i] = pCurrent;
        if (pCurrent)
        {
            mFreeTextures.Push(GetFreeTextures(), pCurrent, mNumTextures);
            ppAlloc[i] = 0;
            --mNumReallocTextures;
        }
    }
}

// FUNCTION: SWBF2 0x00726430
bool RedMoviePlayerBase::SetupStart()
{
    if (mReadBuffer == &mBufferStart)
    {
        return false;
    }

    const unsigned int uiAlign = mFileReadSize;
    if (mBuffer.GetFreeAligned(uiAlign) < uiAlign)
    {
        return false;
    }
    const unsigned int uiWriteOffset = mBuffer.GetWriteOffset();
    const unsigned int uiReadAligned = uiAlign * (mBuffer.mPos / uiAlign);
    const unsigned int uiWriteAligned =
        uiAlign * ((uiAlign + uiWriteOffset - 1) / uiAlign);
    const unsigned int uiPadding = uiWriteAligned - uiWriteOffset;
    if (uiPadding > mBuffer.GetFree())
    {
        return false;
    }

    mBufferStart = mBuffer;
    mReadBuffer = &mBufferStart;
    mInvalidStartData = mBufferStart.mUsed;
    mBufferStart.UpdateWritePos(uiPadding);
    mInvalidStartData += uiPadding;
    return true;
}

// FUNCTION: SWBF2 0x00726590
void RedMoviePlayerBase::Update()
{
    PlayState state = mCurrentPlayState;
stopped_or_playing:
    if (state == playState_stopped)
    {
        mFlags = (mFlags & 0xdb) | flags_ResetSegment;
        if ((mFlags & flags_Reading) && mFile.mFileOpComplete)
        {
            mFlags &= ~flags_Reading;
            mFileScheduledReadSize = 0;
        }
    }
    else
    {
        while (state > playState_stopped && state <= playState_playing)
        {
            if (mFlags & flags_Reading)
            {
                if (!mFile.mFileOpComplete)
                {
                    break;
                }
                if (!(mFlags & flags_ResetSegment))
                {
                    mReadBuffer->UpdateWritePos(mFileScheduledReadSize);
                }
                mFlags &= ~flags_Reading;
                mFileScheduledReadSize = 0;
                break;
            }
            if (mFlags & flags_ResetSegment)
            {
                break;
            }

            unsigned int uiFreeBlock = mReadBuffer->GetFreeBlock();
            unsigned int uiDataLeft = GetDataLeft();
            const SegmentSequence* pNext = 0;
            if (!mSegmentQueue.IsEmpty())
            {
                pNext = mSegmentQueue.Peek(0);
            }
            Segment* pNextSegment = pNext ? pNext->GetSegment() : 0;
            bool bPlayAtEnd = pNext && pNext->PlayAtEnd();

            if (!(uiDataLeft && (!pNextSegment || bPlayAtEnd)))
            {
                bool bContinue = (mFlags & flags_BufferingEnd) != 0;
                if (!bContinue)
                {
                    Notify(notifyType_BufferingComplete);
                    mFlags |= flags_BufferingEnd;
                    bContinue = mSegmentQueue.IsEmpty();
                }
                if (!bContinue && !pNextSegment)
                {
                    state = mCurrentPlayState;
                    if (state == playState_stopped)
                    {
                        goto stopped_or_playing;
                    }
                    continue;
                }

                if ((GetLooping() || pNextSegment) &&
                    !(mFlags & (flags_End | flags_Discontinuity)))
                {
                    if (!SetupStart())
                    {
                        return;
                    }
                    Segment* pSegment = pNextSegment ? pNextSegment : mCurrentSegment;
                    mFile.Seek(pSegment->mStartPos);
                    if (pNextSegment)
                    {
                        SegmentSequence* pLocked = mSegmentQueue.Lock();
                        mSegmentQueue.Unlock(pLocked);
                    }
                    mCurrentSegment = pSegment;
                    mFlags |= uiDataLeft ? flags_Discontinuity : flags_End;
                    uiFreeBlock = mReadBuffer->GetFreeBlock();
                    uiDataLeft = GetDataLeft();
                }
            }

            unsigned int uiReadSize = uiFreeBlock;
            if (uiReadSize > uiDataLeft)
            {
                uiReadSize = uiDataLeft;
            }
            if (uiReadSize > mFileReadSize)
            {
                uiReadSize = mFileReadSize;
            }
            const unsigned int uiAlignedSize = mMediaSectorSize *
                ((uiReadSize + mMediaSectorSize - 1) / mMediaSectorSize);
            if ((uiReadSize >= mFileReadSize || uiReadSize == uiDataLeft) &&
                uiAlignedSize && mReadBuffer->GetFreeBlock() >= uiAlignedSize)
            {
                mFlags &= ~flags_BufferFull;
                mFile.Read(mReadBuffer->GetWritePos(), uiAlignedSize);
                mFlags |= flags_Reading;
                mFileScheduledReadSize = uiReadSize;
            }
            else
            {
                mFlags |= flags_BufferFull;
            }
            break;
        }
    }

    if ((mFlags & flags_ResetSegment) && !(mFlags & flags_Reading))
    {
        unsigned int uiStart = mCurrentSegment->mStartPos;
        if (mFile.GetPos() != uiStart)
        {
            mFile.Seek(uiStart);
            Flush();
        }
        mFlags &= ~flags_ResetSegment;
    }
}

// FUNCTION: SWBF2 0x006f86f0
RedCircularBuffer& RedCircularBuffer::operator=(const RedCircularBuffer& other)
{
    mBuffer = other.mBuffer;
    mPos = other.mPos;
    mUsed = other.mUsed;
    mSize = other.mSize;
    return *this;
}

// FUNCTION: SWBF2 0x006f8740
unsigned int RedCircularBuffer::GetFree() const
{
    return mSize - mUsed;
}

// FUNCTION: SWBF2 0x006f8750
unsigned int RedCircularBuffer::GetFreeAligned(unsigned int uiAlign) const
{
    unsigned int uiRead = uiAlign * (mPos / uiAlign);
    unsigned int uiWrite = uiAlign * ((uiAlign + GetWriteOffset() - 1) / uiAlign);
    unsigned int uiDistance = uiWrite < uiRead ? mSize - uiRead + uiWrite : uiWrite - uiRead;
    return mSize <= uiDistance ? 0 : mSize - uiDistance;
}

// FUNCTION: SWBF2 0x006f87a0
unsigned int RedCircularBuffer::GetFreeBlock() const
{
    unsigned int uiWrite = GetWriteOffset();
    if (!GetFree())
    {
        return 0;
    }
    return (mPos <= uiWrite ? mSize : mPos) - uiWrite;
}

// FUNCTION: SWBF2 0x006f87e0
unsigned int RedCircularBuffer::GetWriteOffset() const
{
    unsigned int uiWrite = mPos + mUsed;
    if (uiWrite >= mSize)
    {
        uiWrite -= mSize;
    }
    return uiWrite;
}

// FUNCTION: SWBF2 0x006f87f0
void* RedCircularBuffer::GetWritePos() const
{
    return mBuffer + GetWriteOffset();
}

// FUNCTION: SWBF2 0x006f8900
void RedCircularBuffer::UpdateWritePos(unsigned int uiSize)
{
    mUsed += uiSize;
}

// FUNCTION: SWBF2 0x00726c70
unsigned int RedFileAsync::GetPos() const
{
    return mFile.GetPos();
}

// FUNCTION: SWBF2 0x00726ce0
void RedFileAsync::Read(void* pBuffer, unsigned int uiSize)
{
    mFileOpComplete = false;
    mFile.Read(pBuffer, uiSize);
}

// FUNCTION: SWBF2 0x00726cf0
void RedFileAsync::Seek(unsigned int uiPos)
{
    mFile.Seek(uiPos);
}

namespace Snd
{

// FUNCTION: SWBF2 0x00738950
void* QueueBase::Add(unsigned int uiElementSize, void* const pElements,
                     unsigned short usCapacity)
{
    if (mFull)
    {
        return 0;
    }
    unsigned short i = mHead++;
    void* pResult = (char*)pElements + i * uiElementSize;
    if (mHead == usCapacity)
    {
        mHead = 0;
    }
    mFull = mHead == mTail;
    return pResult;
}

// FUNCTION: SWBF2 0x00745080
void* QueueLockBase::Get(QueueBase* const pQueue, unsigned int uiElementSize,
                         void* const pElements, unsigned int uiCapacity)
{
    if (mLock == pQueue->mHead)
    {
        return 0;
    }
    unsigned short i = mLock++;
    void* pResult = (char*)pElements + i * uiElementSize;
    if (mLock == uiCapacity)
    {
        mLock = 0;
    }
    return pResult;
}

// FUNCTION: SWBF2 0x007450c0
void QueueLockBase::Unlock(QueueBase* const pQueue, const void* const pElement,
                           unsigned int uiElementSize, void* const pElements,
                           unsigned short usCapacity)
{
    (void)pElement;
    (void)uiElementSize;
    (void)pElements;
    ++pQueue->mTail;
    if (pQueue->mTail == usCapacity)
    {
        pQueue->mTail = 0;
    }
}
}

// TEMPLATE: SWBF2 0x007253f0
// Snd::Queue<RedMoviePlayerBase::SegmentSequence,10>::~Queue<RedMoviePlayerBase::SegmentSequence,10>

// TEMPLATE: SWBF2 0x00725400
// Snd::QueueLock<RedMoviePlayerBase::SegmentSequence,10>::~QueueLock<RedMoviePlayerBase::SegmentSequence,10>

// FUNCTION: SWBF2 0x007251b0
RedMoviePlayerBase::Info::Info(unsigned int uiHashID, Segment* const pSegments,
                               unsigned int uiNumSegments, int iRedHeap)
    : mHashID(uiHashID),
      mSegments(pSegments),
      mNumSegments(uiNumSegments),
      mRedMemoryHeap(iRedHeap),
      mRefCount(0)
{
    sInfoList.Add(this, this);
}

// FUNCTION: SWBF2 0x00725460
RedMoviePlayerBase::Info::~Info()
{
    sInfoList.Remove(this);
}

// FUNCTION: SWBF2 0x00725490
RedMoviePlayerBase::~RedMoviePlayerBase()
{
    if (IsOpen())
    {
        Close();
    }
    mNode.GetList()->Remove(&mNode);
}

// FUNCTION: SWBF2 0x00725a40
void RedMoviePlayerBase::FlushSegmentQueue()
{
    mSegmentQueue.Empty(0, 0);
}

// FUNCTION: SWBF2 0x00725af0
RedMoviePlayerBase::PlayState RedMoviePlayerBase::GetDesiredPlayState() const
{
    return mDesiredPlayState;
}

// FUNCTION: SWBF2 0x00725b60
unsigned int RedMoviePlayerBase::Info::GetNumSegments() const
{
    return mNumSegments;
}

// FUNCTION: SWBF2 0x00725be0
RedTexture** RedMoviePlayerBase::GetUsedTextures() const
{
    return mTextures + mNumTextures * 2;
}

// FUNCTION: SWBF2 0x00725c10
bool RedMoviePlayerBase::IsOpen() const
{
    return mInfo != 0;
}

// FUNCTION: SWBF2 0x00725c80
bool RedMoviePlayerBase::Open(const char* const pFileName, RedTexture** ppTextures,
                              unsigned int uiNumTextures, int iHeap, unsigned int uiArg4,
                              void* pBuffer, unsigned int uiBufferSize, unsigned int uiReadSize,
                              unsigned int uiArg8)
{
    PblFile file;
    if (!file.Open(pFileName, true, false, true))
    {
        return false;
    }

    PblFileChunk chunk(&file);
    bool bHeader = OpenHeader(chunk, iHeap, uiArg4);
    file.Close();
    if (!bHeader)
    {
        return false;
    }

    if (!mFile.Open(pFileName, uiArg8, true))
    {
        return false;
    }

    mBuffer.SetBuffer(pBuffer, uiBufferSize);
    mBufferStart.SetBuffer(pBuffer, uiBufferSize);
    mCurrentSegment = const_cast<Segment*>(mInfo->FindSegmentByIndex(0));
    mFileReadSize = uiReadSize;
    mNumTextures = uiNumTextures;
    mInvalidStartData = 0;
    mReadBuffer = &mBuffer;
    mTextures = ppTextures;
    mFlags = (mFlags & ~(flags_Reading | flags_BufferFull)) | flags_ResetSegment;
    memset(ppTextures, 0, uiNumTextures * 4 * sizeof(RedTexture*));
    return true;
}

// FUNCTION: SWBF2 0x00725de0
bool RedMoviePlayerBase::OpenHeader(PblFileChunk& chunk, int iHeap, unsigned int uiArg4)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x0fb40705:
            mInfo = Info::Read(child, iHeap);
            break;

        case 0xd872e2a5:
        {
            unsigned int uiDataPos = child._pFile->Tell();
            if (mInfo)
            {
                for (unsigned int i = 0; i < mInfo->GetNumSegments(); ++i)
                {
                    mInfo->mSegments[i].mStartPos += uiDataPos;
                }
            }
            break;
        }

        case 0x2f1f7060:
        case 0xf9f40518:
            if (OpenHeader(child, iHeap, uiArg4))
            {
                return true;
            }
            break;
        }
    }

    if (!mInfo)
    {
        return false;
    }
    mCurrentSegment = const_cast<Segment*>(mInfo->FindSegmentByIndex(0));
    return true;
}

// FUNCTION: SWBF2 0x00725f70
RedMoviePlayerBase::Info* RedMoviePlayerBase::Info::Read(PblFileChunk& chunk, int iHeap)
{
    unsigned int uiHashID = 0;
    unsigned int uiNumSegments = 0;

    while (chunk.HasMoreChildren())
    {
        switch ((unsigned int)chunk._pFile->Read32())
        {
        case 0x40fbdebd:
            uiNumSegments = chunk._pFile->Read32();
            break;

        case 0x8d39bde6:
        {
            uiHashID = chunk._pFile->Read32();
            Info* pInfo = const_cast<Info*>(FindByHashID(uiHashID));
            if (pInfo)
            {
                pInfo->AddRef();
                return pInfo;
            }
            break;
        }

        case 0xb969be96:
        {
            void* pMem = RedAllocFromHeap(iHeap, sizeof(Info) + uiNumSegments * sizeof(Segment), 0);
            if (!pMem)
            {
                return 0;
            }
            Info* pInfo = new (pMem) Info(uiHashID, (Segment*)((Info*)pMem + 1), uiNumSegments, iHeap);
            pInfo->AddRef();
            if (!pInfo->ReadSegments(chunk, uiNumSegments))
            {
                Release(pInfo);
                return 0;
            }
            return pInfo;
        }
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00726070
bool RedMoviePlayerBase::Info::ReadSegments(PblFileChunk& chunk, unsigned int uiNumSegments)
{
    unsigned int uiIndex = 0;
    unsigned int uiStartPos = 0;

    while (chunk.HasMoreChildren() && uiNumSegments)
    {
        if ((unsigned int)chunk._pFile->Read32() != 0xf3c342e6)
        {
            continue;
        }

        unsigned int uiHashID = 0;
        unsigned int uiGap = 0;
        unsigned int uiLength = 0;
        bool bReading = true;
        while (chunk.HasMoreChildren() && bReading)
        {
            switch ((unsigned int)chunk._pFile->Read32())
            {
            case 0x8d39bde6:
                uiHashID = chunk._pFile->Read32();
                break;
            case 0x83d03615:
                uiLength = chunk._pFile->Read32();
                break;
            case 0x809608b6:
                uiGap = chunk._pFile->Read32();
                break;
            case 0x7db1d043:
                new (&mSegments[uiIndex++]) Segment(uiHashID, uiStartPos, uiLength);
                uiStartPos += uiGap + uiLength;
                bReading = false;
                --uiNumSegments;
                break;
            }
        }
    }
    return uiNumSegments == 0;
}

// FUNCTION: SWBF2 0x00726290
void __cdecl RedMoviePlayerBase::Info::Release(Info* const pInfo)
{
    if (pInfo->RemoveRef())
    {
        int iHeap = pInfo->mRedMemoryHeap;
        pInfo->~Info();
        RedFreeToHeap(iHeap, pInfo, 0);
    }
}

// FUNCTION: SWBF2 0x00726310
bool RedMoviePlayerBase::Info::RemoveRef()
{
    return --mRefCount == 0;
}

// FUNCTION: SWBF2 0x007263c0
void RedMoviePlayerBase::SetCurrentPlayState(PlayState eState)
{
    mCurrentPlayState = eState;
}

// FUNCTION: SWBF2 0x007263d0
void RedMoviePlayerBase::SetLooping(bool bLoop)
{
    if (bLoop)
    {
        mFlags |= flags_Loop;
    }
    else
    {
        mFlags &= ~flags_Loop;
    }
}

// FUNCTION: SWBF2 0x007264e0
void RedMoviePlayerBase::Stop()
{
    mDesiredPlayState = playState_stopped;
    FlushSegmentQueue();
}
