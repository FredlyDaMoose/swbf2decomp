#include "Music.h"

#include <float.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblFile.h"
#include "SoundFL/source/win/sndstream.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/sndenginebase.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// GLOBAL: SWBF2 0x01ecb1a8
// SYNTHETIC: SWBF2 0x00404350
// `dynamic initializer for 'Music::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768d30
// `dynamic atexit destructor for 'Music::sMemoryPool''
MemoryPool Music::sMemoryPool("Music", 0x48);

// GLOBAL: SWBF2 0x007ec23c
// SYNTHETIC: SWBF2 0x00404370
// `dynamic initializer for 'Music::sMusicInactive''
// SYNTHETIC: SWBF2 0x00768d40
// `dynamic atexit destructor for 'Music::sMusicInactive''
PblListDoubleSorted Music::sMusicInactive(true);
// GLOBAL: SWBF2 0x007ec230
// SYNTHETIC: SWBF2 0x00404380
// `dynamic initializer for 'Music::sMusicActive''
// SYNTHETIC: SWBF2 0x00768d60
// `dynamic atexit destructor for 'Music::sMusicActive''
PblListDoubleSorted Music::sMusicActive(true);
// GLOBAL: SWBF2 0x01e579ec
unsigned int Music::sMusicPlaying;
// GLOBAL: SWBF2 0x01e579e8
float Music::sMinInactiveTime;
// GLOBAL: SWBF2 0x01e579e4
float Music::sInactiveTime;
// GLOBAL: SWBF2 0x01e579c6
bool Music::sIgnoreInactiveTime;

// FUNCTION: SWBF2 0x005a9420
void PblListDoubleSorted::Sort()
{
    PblListDouble::Node* pLink = mTerminator.mNext;
    Node* pPrev = (pLink == &mTerminator) ? 0 : (Node*)((char*)pLink - offsetof(Node, mNode));

    for (pLink = pLink->mNext; pLink != &mTerminator; pLink = pLink->mNext)
    {
        Node* pNode = (Node*)((char*)pLink - offsetof(Node, mNode));
        if (pNode == 0)
        {
            break;
        }

        if (pNode->mSortValue > pPrev->mSortValue)
        {
            Node* pAfter;
            PblListDouble::Node* pScan = &pNode->mNode;
            for (;;)
            {
                pScan = pScan->mPrev;
                if (pScan == &mTerminator)
                {
                    pAfter = 0;
                    break;
                }
                pAfter = (Node*)((char*)pScan - offsetof(Node, mNode));
                if (pAfter == 0 || !(pNode->mSortValue > pAfter->mSortValue))
                {
                    break;
                }
            }

            pNode->mNode.Remove();

            PblListDouble::Node* pAt = pAfter ? pAfter->mNode.mNext : mTerminator.mNext;
            pNode->mNode.mNext = pAt;
            pNode->mNode.mPrev = pAt->mPrev;
            pAt->mPrev->mNext = &pNode->mNode;
            pAt->mPrev = &pNode->mNode;
        }

        pPrev = pNode;
        pLink = &pNode->mNode;
    }
}

// FUNCTION: SWBF2 0x005a94b0
Music::Music()
    : mHashID(0),
      mFadeInTime(1.0f),
      mFadeOutTime(1.0f),
      mStream(0),
      mMinPlaybackTime(0.0f),
      mMaxPlaybackTime(FLT_MAX),
      mPlaybackTime(0.0f),
      mMinInactiveTime(0.0f),
      mInactiveTime(0.0f),
      mPlaybackPosition(0),
      mPriority(0.0f),
      mNode(),
      mPlayStateCurrent(playState_stopped),
      mPlayStateDesired(playState_stopped)
{
    sMusicInactive.Add(&mNode);
}

// FUNCTION: SWBF2 0x005a9540
Music::~Music()
{
}

// FUNCTION: SWBF2 0x005a9570
void Music::Play()
{
    if (!GetProperties(false))
    {
        return;
    }

    switch (mPlayStateCurrent)
    {
    case playState_starting:
    case playState_playing:
        mPlaybackPosition = 0;
        break;

    case playState_stopped:
        mNode.mNode.Remove();
        UpdatePriority(false);
        sMusicActive.Add(&mNode);
        break;
    }
}

// FUNCTION: SWBF2 0x005a95d0
void Music::Stop()
{
    if (mPlayStateCurrent == playState_stopped)
    {
        mNode.mNode.Remove();
        sMusicInactive.Add(&mNode);
    }
    mPlayStateDesired = playState_stopped;
}

// FUNCTION: SWBF2 0x005a9610
void Music::Update(float fDeltaTime)
{
    switch (mPlayStateCurrent)
    {
    case playState_starting:
    case playState_playing:
        if (mStream)
        {
            mPlaybackPosition = mStream->GetPlaybackPosition();
            if (mPlaybackTime >= mMaxPlaybackTime)
            {
                Stop();
            }
            if (mStream->GetPlayState() != Snd::SourceTransport::playState_stopped)
            {
                mPlaybackTime += fDeltaTime;
                break;
            }
        }
        mPlayStateDesired = playState_stopped;
        mPlayStateCurrent = playState_stopping;

    case playState_stopping:
        mPlaybackTime += fDeltaTime;
        break;

    case playState_stopped:
        mInactiveTime += fDeltaTime;
        UpdatePriority(true);
        break;
    }

    switch (mPlayStateDesired)
    {
    case playState_playing:
        switch (mPlayStateCurrent)
        {
        case playState_starting:
            if (!mStream || !mStream->GetBus()->IsActive())
            {
                mPlayStateCurrent = playState_playing;
            }
            break;

        case playState_stopped:
        case playState_paused:
            if (mInactiveTime >= mMinInactiveTime || sIgnoreInactiveTime)
            {
                const Snd::SoundStream::Properties* pProps = GetProperties(false);
                if (!mStream)
                {
                    mStream = Snd::EngineBase::GetOpenStream(pProps->mStreamID);
                }

                if (!mStream)
                {
                    mPlayStateCurrent = playState_starting;
                }
                else if (!mStream->GetLocked())
                {
                    mStream->Lock(true);
                    pProps->SetupStream(mStream, 0);
                    mStream->SetPitch(pProps->SourceVariation::CalculatePitch(pProps->mPitch));
                    mStream->SetGain(pProps->SourceVariation::CalculateGain(pProps->mGain));
                    mStream->GetBus()->Fade(mFadeInTime, 1.0f, 0.0f);
                    mStream->Play(false);
                    mStream->SetPlaybackPosition(mPlaybackPosition);
                    ++sMusicPlaying;
                    mPlayStateCurrent = playState_starting;
                    mPlaybackTime = 0.0f;
                }
            }
            break;
        }
        break;

    case playState_stopped:
        switch (mPlayStateCurrent)
        {
        case playState_starting:
        case playState_playing:
            if (mStream)
            {
                if (mPlaybackTime < mMinPlaybackTime)
                {
                    break;
                }
                mStream->GetBus()->Fade(mFadeOutTime, 0.0f, -1.0f);
            }
            mPlayStateCurrent = playState_stopping;
            break;

        case playState_stopping:
            if (mStream)
            {
                if (mStream->GetBus()->IsActive())
                {
                    break;
                }
                mPlayStateCurrent = playState_stopped;
                mStream->Stop();
                if (mStream->GetLocked())
                {
                    mStream->Lock(false);
                }
                --sMusicPlaying;
                mStream = 0;
            }
            else
            {
                --sMusicPlaying;
                mPlayStateCurrent = playState_stopped;
            }
            mInactiveTime = 0.0f;
            mPlaybackPosition = 0;
            mNode.mNode.Remove();
            sMusicInactive.Add(&mNode);
            break;

        case playState_stopped:
            if (mStream)
            {
                if (mStream->GetLocked())
                {
                    mStream->Lock(false);
                }
                mStream = 0;
            }
            break;

        case playState_pausing:
        case playState_paused:
            mPlayStateCurrent = playState_stopping;
            break;
        }
        break;

    case playState_paused:
        switch (mPlayStateCurrent)
        {
        case playState_starting:
        case playState_playing:
            if (mPlaybackTime >= mMinPlaybackTime)
            {
                if (mStream)
                {
                    mStream->GetBus()->Fade(mFadeOutTime, 0.0f, -1.0f);
                }
                mPlayStateCurrent = playState_pausing;
            }
            break;

        case playState_stopping:
        case playState_stopped:
            mPlayStateDesired = playState_stopped;
            break;

        case playState_pausing:
            if (mStream)
            {
                if (mStream->GetBus()->IsActive())
                {
                    break;
                }
                mPlayStateCurrent = playState_paused;
                mPlaybackPosition = mStream->GetPlaybackPosition();
                mStream->Stop();
                mStream->Lock(false);
                --sMusicPlaying;
            }
            else
            {
                mPlayStateCurrent = playState_paused;
            }
            break;
        }
        break;
    }
}

// FUNCTION: SWBF2 0x005a9a60
void Music::UpdatePriority(bool bResort)
{
    float fSortValue;
    if (mInactiveTime >= mMinInactiveTime || sIgnoreInactiveTime)
    {
        fSortValue = mPriority + 0.0001f;
    }
    else
    {
        fSortValue = 0.0f;
    }

    if (mNode.mSortValue != fSortValue)
    {
        mNode.mSortValue = fSortValue;
        if (bResort)
        {
            sMusicActive.Sort();
        }
    }
}

// FUNCTION: SWBF2 0x005a9ab0
void Music::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk data = chunk.ReadNextChild();
        const unsigned int uTag = data._pFile->Read32();
        data._pFile->Read8();

        switch (uTag)
        {
        case 0x8d39bde6:
            SetHashID(data._pFile->Read32());
            break;
        case 0x2e93ef4c:
            SetID(data._pFile->Read32(), false, false);
            break;
        case 0x5083ecd9:
            SetFadeInTime(data._pFile->ReadFloat());
            break;
        case 0x798dc484:
            SetFadeOutTime(data._pFile->ReadFloat());
            break;
        case 0x6813be83:
            SetMinPlaybackTime(data._pFile->ReadFloat());
            break;
        case 0x319a9661:
            SetMaxPlaybackTime(data._pFile->ReadFloat());
            break;
        case 0x35724887:
            SetMinInactiveTime(data._pFile->ReadFloat());
            break;
        case 0x94e4e309:
            SetPriority(data._pFile->ReadFloat());
            break;
        }
    }
}

// FUNCTION: SWBF2 0x005a9d00
void Music::IgnoreInactiveTime(bool bIgnore)
{
    sIgnoreInactiveTime = bIgnore;
}

// FUNCTION: SWBF2 0x005a9eb0
void Music::StopAll()
{
    PblListDouble::Node* pLink = sMusicActive.mTerminator.mNext;
    while (pLink != &sMusicActive.mTerminator)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == sMusicActive.GetTerminator())
        {
            break;
        }

        Music* pMusic = (Music*)((char*)pNode - offsetof(Music, mNode));
        if (pMusic->mStream)
        {
            Snd::SourceTransport* pTransport = pMusic->mStream;
            pMusic->mStream->Snd::SourceBus::Stop(pTransport);
            pTransport->Stop();
        }

        pLink = pLink->mNext;
        pMusic->mPlayStateCurrent = playState_stopped;
        pMusic->Stop();
    }

    sInactiveTime = 0.0f;
    sMinInactiveTime = 0.0f;
    sMusicPlaying = 0;
}

// FUNCTION: SWBF2 0x005a9e20
Music* Music::FindByID(const char* pcID)
{
    unsigned int uHashID = PblHash(pcID);
    Music* pMusic = FindByHashID(&sMusicActive, uHashID);
    if (!pMusic)
    {
        pMusic = FindByHashID(&sMusicInactive, uHashID);
    }
    return pMusic;
}

// FUNCTION: SWBF2 0x005a9f40
Music* Music::FindByHashID(PblListDoubleSorted* pList, unsigned int uHashID)
{
    for (PblListDouble::Node* pLink = pList->mTerminator.mNext; pLink != &pList->mTerminator;
         pLink = pLink->mNext)
    {
        PblListDoubleSorted::Node* pNode =
            (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
        if (pNode == pList->GetTerminator())
        {
            break;
        }

        Music* pMusic = (Music*)((char*)pNode - offsetof(Music, mNode));
        if (pMusic->mHashID == uHashID)
        {
            return pMusic;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005a9e60
void Music::DestroyAll(PblListDoubleSorted* pList)
{
    StopAll();
    while (pList->GetFirst() != pList->GetTerminator())
    {

        Music* const pMusic = (Music*)pList->GetFirst()->GetObjectA(pList, offsetof(Music, mNode));
        if (pMusic)
        {

            pMusic->~Music();
            sMemoryPool.Free(pMusic);
        }
    }
}

// FUNCTION: SWBF2 0x005a9c20
void Music::Load(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }
        unsigned int uTag = child._pFile->Read32();
        child._pFile->Read8();
        if (uTag != 0x9F9C4FD4)
        {
            continue;
        }
        if (!Music::sMemoryPool.GetCount())
        {
            Music::sMemoryPool.Setup(0x20, Music::sMemoryPool.GetSize(), Music::sMemoryPool.GetGrow());
        }
        Music* pMusic = new Music;
        PblFileChunk data = chunk.ReadNextChild();
        pMusic->Read(data);
    }
    Music::sMusicInactive.Sort();
}

void* Music::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}
