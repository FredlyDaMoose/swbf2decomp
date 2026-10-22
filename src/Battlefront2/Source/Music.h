#pragma once

#include <stddef.h>

#include "GameSoundStream.h"
#include "PebbleFL/Common/PblListDoubleSorted.h"

namespace Snd { class Stream; }
class MemoryPool;
class PblFileChunk;

class Music : public GameSoundStream
{
public:
    enum PlayState
    {
        playState_invalid = 0,
        playState_starting = 1,
        playState_playing = 2,
        playState_stopping = 3,
        playState_stopped = 4,
        playState_pausing = 5,
        playState_paused = 6,
    };

    static Music* FindByID(const char* pcID);

    static Music* FindByHashID(PblListDoubleSorted* pList, unsigned int uHashID);

    static void IgnoreInactiveTime(bool bIgnore);

    Music();
    ~Music();

    static void* operator new(unsigned int uSize);

    void Play();
    void Stop();
    void Update(float fDeltaTime);

    static void StopAll();

    static void UpdateAll(float fDeltaTime);

    static void DestroyAll()
    {
        DestroyAll(&sMusicInactive);
        DestroyAll(&sMusicActive);
        sInactiveTime = 0.0f;
        sMinInactiveTime = 0.0f;
        sMusicPlaying = 0;
    }

private:

    static void DestroyAll(PblListDoubleSorted* pList);

public:

    void SetHashID(unsigned int uHashID) { mHashID = uHashID; }
    void SetFadeInTime(float fTime) { mFadeInTime = fTime; }
    void SetFadeOutTime(float fTime) { mFadeOutTime = fTime; }
    void SetMinPlaybackTime(float fTime)
    {
        mMinPlaybackTime = fTime;
        if (fTime > mMaxPlaybackTime)
        {
            mMaxPlaybackTime = fTime;
        }
        mPlaybackTime = fTime;
    }
    void SetMaxPlaybackTime(float fTime)
    {
        mMaxPlaybackTime = fTime;
        if (mMinPlaybackTime > fTime)
        {
            mMinPlaybackTime = fTime;
        }
    }
    void SetMinInactiveTime(float fTime)
    {
        mMinInactiveTime = fTime;
        mInactiveTime = fTime;
    }
    void SetPriority(float fPriority)
    {
        mPriority = fPriority;
        mNode.mSortValue = 0.0f;
    }

    PlayState GetCurrentPlayState() const { return mPlayStateCurrent; }
    float GetInactiveTimeLeft() const { return mMinInactiveTime - mInactiveTime; }

    unsigned int mHashID;
    float mFadeInTime;
    float mFadeOutTime;
    Snd::Stream* mStream;
    float mMinPlaybackTime;
    float mMaxPlaybackTime;
    float mPlaybackTime;
    float mMinInactiveTime;
    float mInactiveTime;
    unsigned int mPlaybackPosition;
    float mPriority;
    PblListDoubleSorted::Node mNode;
    PlayState mPlayStateCurrent;
    PlayState mPlayStateDesired;

    void UpdatePriority(bool bResort);
    void Read(PblFileChunk& chunk);

public:

    static void Load(PblFileChunk& chunk);

    static PblListDoubleSorted sMusicInactive;
    static PblListDoubleSorted sMusicActive;
    static unsigned int sMusicPlaying;
    static MemoryPool sMemoryPool;
    static float sMinInactiveTime;
    static float sInactiveTime;
    static bool sIgnoreInactiveTime;
};
 
 
 
 
 
 
