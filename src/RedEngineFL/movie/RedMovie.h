#pragma once

#include "SoundFL/source/sndqueue.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "pc/pcRedMoviePlayer.h"
#include "redpipe.h"

#include "PebbleFL/Common/PblRandom.h"

class PblFileChunk;
class RedTexture;

namespace Snd { class Bus; }

class RedMovie : public RedMoviePlayer
{
public:

    class Properties;
    class SegmentInfo;
    class SegmentSubtitles;
    class Subtitle;

    class Subtitle
    {
    public:
        Subtitle() { mStringID = 0; mEnableTime = 0.0f; mDisableTime = 0.0f; }

        void Init(unsigned int uiStringID, float fEnableTime, float fDisableTime)
        {
            mStringID = uiStringID;
            mEnableTime = fEnableTime;
            mDisableTime = fDisableTime;
        }
        unsigned int GetStringID() const { return mStringID; }

        static int __cdecl CompareSearch(const void* pKey, const void* pElement);
        static int __cdecl CompareSort(const void* pElement1, const void* pElement2);

        unsigned int mStringID;
        float mEnableTime;
        float mDisableTime;
    };

    class SegmentInfo : public PblList<SegmentInfo>::Node
    {
    public:
        SegmentInfo(unsigned int uiHashID, float fPlayProbability, int iNumLoops,
                     PblList<SegmentInfo>& list);
        ~SegmentInfo();

        unsigned int GetHashID() const;
        int GetNumLoops() const;
        float GetPlayProbability() const { return mPlayProbability; }

        unsigned int mHashID;
        float mPlayProbability;
        int mNumLoops;
    };

    class Properties : public PblList<Properties>::Node
    {
    public:
        Properties();
        ~Properties();

        static void __cdecl DestroyAll();
        static const Properties* __cdecl FindByHashID(unsigned int uiHashID);

        unsigned int GetHashID() const;
        unsigned int GetMovieID() const;
        float GetFadeInTime() const;
        float GetFadeOutTime() const;
        const SegmentInfo* GetSegmentInfo() const;

        void SetMovieID(unsigned int uiMovieID);
        void SetFadeInTime(float fTime);
        void SetFadeOutTime(float fTime);
        void SetCyclePlayback(bool bCycle);

        void Read(PblFileChunk& chunk);
        void ReadSegments(PblFileChunk& chunk);

        static PblList<Properties> sPropertyList;
        static PblRandom sRandom;

        unsigned int mHashID;
        unsigned int mMovieID;
        float mFadeInTime;
        float mFadeOutTime;
        PblList<SegmentInfo> mList;
        PblList<SegmentInfo>* mSelList;

        mutable SegmentInfo* mPrevSegmentInfo;
        unsigned char mFlags;
    };

    class SegmentSubtitles : public PblList<SegmentSubtitles>::Node
    {
    public:
        SegmentSubtitles();
        ~SegmentSubtitles();

        static void __cdecl DestroyAll();
        static const SegmentSubtitles* __cdecl FindBySegmentID(unsigned int uiSegmentID);

        unsigned int GetSegmentID() const;
        unsigned int GetFontID() const;
        RedColor GetColor() const;
        const Subtitle* GetSubtitle(float fTime) const;

        void Read(PblFileChunk& chunk);
        void ReadSubtitles(PblFileChunk& chunk);
        void FreeSubtitles();

        static PblList<SegmentSubtitles> sList;

        unsigned int mSegmentID;
        unsigned int mFontID;
        Subtitle* mSubtitles;
        Subtitle* mSelSubtitles;
        unsigned int mNumSubtitles;
        RedColor mColor;
    };

    enum Flags
    {
        flags_Playing = 1,
        flags_Loop = 2,
        flags_Rendering = 4
    };

    // FUNCTION: SWBF2 0x006f4cd0
    RedMovie();

    void SetAudioBus(Snd::Bus* pBus);

    bool Open(const char* const pFileName, int iHeap, int iUnused, unsigned int uiArg4,
              unsigned int uiBufferSize, unsigned int uiReadSize, unsigned int uiNumTextures);
    bool IsOpen() const;
    void Close();
    static void DestroyAll();

    virtual ~RedMovie();
    static void __cdecl Load(PblFileChunk& chunk, bool bOne);
    static void __cdecl SubtitlesEnable(bool bEnable);
    void Play(const Properties* const pProperties, bool bPlayAtEnd);
    void Stop(bool bUpdate);
    void Flush(bool bFlushPlayer);
    void Update(float fDeltaTime);
    void Render();
    void SetDimensions(int iX, int iY, unsigned int uiWidth, unsigned int uiHeight,
                       bool bSafeArea);
    void SetLooping(bool bLoop);
    bool IsPlaying() const;
    const Properties* GetProperties() const;
    bool GetPropertiesScheduled(const Properties* const pProperties) const;

    void Play(const Properties* const pProperties, const SegmentInfo* const pSegmentInfo,
              bool bResetLoops, bool bPlayAtEnd);
    void Loop(const SegmentInfo* const pSegmentInfo);
    void UpdateCurrent();
    void FlushPipe(RedPipePointer<const RedTexture*, 1>& pipe);
    bool PipeTextureExists(const RedTexture* pTexture) const;
    void UnlockTexture(const RedTexture* const pTexture);

    static bool sSubtitlesEnabled;

    static void __cdecl MovieNotification(RedMoviePlayerBase* pPlayer,
                                          NotifyType eType, void* pData);

    float mPlayTime;
    unsigned int mNumLoops;
    RedPipePointer<const RedTexture*, 1> mTextures;
    RedPipePointer<const RedTexture*, 1> mFadeTextures;
    Snd::QueueCircular<const Properties*, 11> mPropertyQueue;
    Snd::QueueCircular<const SegmentInfo*, 11> mSegmentInfoQueue;
    Properties* mPropertiesCur;
    SegmentInfo* mSegmentCur;
    Properties* mPropertiesPrev;
    Properties* mPropertiesNext;
    SegmentInfo* mSegmentNext;
    int mPosX;
    int mPosY;
    unsigned int mWidth;
    unsigned int mHeight;
    Snd::Bus* mAudioBus;
    float mChangedSegmentTime;
    RedScreenGroupElement mSubtitleGroup;
    RedTextElement mSubtitles;
    SegmentSubtitles* mCurSubtitles;
    Subtitle* mDisplaySubtitle;
    unsigned char mFlags;
};

 
 
 
 
 
 
 
 
 
