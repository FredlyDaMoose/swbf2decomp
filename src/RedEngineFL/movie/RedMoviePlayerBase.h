#pragma once

#include "PebbleFL/Common/PblList.h"
#include "SoundFL/source/sndqueue.h"
#include "RedCircularBuffer.h"
#include "RedFileAsync.h"

class PblFileChunk;
class RedTexture;

class RedMoviePlayerBase
{
public:

    enum PlayState
    {
        playState_invalid = 0,
        playState_stopped = 1,
        playState_pausing = 2,
        playState_paused = 3,
        playState_playing = 4
    };

    enum NotifyType
    {
        notifyType_Invalid = 0,
        notifyType_BufferingComplete = 1,
        notifyType_End = 2,
        notifyType_Discontinuity = 3
    };

    enum Flags
    {
        flags_Reading = 1,
        flags_Loop = 2,
        flags_BufferFull = 4,
        flags_End = 8,
        flags_Discontinuity = 16,
        flags_ResetSegment = 32,
        flags_BufferingCb = 64,
        flags_BufferingEnd = 128
    };

    typedef void(__cdecl* NotifyCallback)(RedMoviePlayerBase* pPlayer,
                                          NotifyType eType, void* pData);

    class Segment
    {
    public:

        Segment(unsigned int uiHashID, unsigned int uiStartPos, unsigned int uiLength)
            : mHashID(uiHashID), mStartPos(uiStartPos), mLength(uiLength) {}

        unsigned int GetHashID() const { return mHashID; }

        unsigned int mHashID;
        unsigned int mStartPos;
        unsigned int mLength;
    };

    class Info : public PblList<Info>::Node
    {
    public:

        Info(unsigned int uiHashID, Segment* const pSegments,
             unsigned int uiNumSegments, int iRedHeap);

        unsigned int GetNumSegments() const;

        static Info* __cdecl Read(PblFileChunk& chunk, int iHeap);

        bool ReadSegments(PblFileChunk& chunk, unsigned int uiNumSegments);

        static const Info* FindByHashID(unsigned int uiHashID);

        const Segment* FindSegmentByHashID(unsigned int uiHashID) const;

        const Segment* FindSegmentByIndex(unsigned int uiIndex) const;

        unsigned int GetHashID() const;
        int GetRedHeap() const;

        void AddRef();

        bool RemoveRef();

        ~Info();

        static void __cdecl Release(Info* const pInfo);

        static PblList<Info> sInfoList;

        unsigned int mHashID;
        Segment* mSegments;
        unsigned int mNumSegments;
        int mRedMemoryHeap;
        unsigned short mRefCount;
    };

    class TextureStack
    {
    public:
        // FUNCTION: SWBF2 0x007253e0
        TextureStack() { mHeight = 0; }

        void Empty();

        bool Exists(RedTexture** const ppTextures, const RedTexture* const pTexture) const;

        void Push(RedTexture** const ppTextures, RedTexture* const pTexture, unsigned int uiSize);

        void Remove(RedTexture** const ppTextures, RedTexture* const pTexture);

        RedTexture* Peek(RedTexture** const ppTextures);
        RedTexture* Pop(RedTexture** const ppTextures);

        unsigned int mHeight;
    };

    class TextureQueue
    {
    public:
        // FUNCTION: SWBF2 0x007253d0
        TextureQueue() { mHead = 0; mUsed = 0; }

        void Add(RedTexture** const ppTextures, RedTexture* const pTexture, unsigned int uiSize);

        void Empty();

        RedTexture* Get(RedTexture** const ppTextures, unsigned int uiSize);

        unsigned int mHead;
        unsigned int mUsed;
    };

    class SegmentSequence
    {
    public:
        enum Flags { flags_playAtEnd = 1 };

        // FUNCTION: SWBF2 0x007253c0
        SegmentSequence() { mSegment = 0; mFlags = 0; }
        Segment* GetSegment() const;
        bool PlayAtEnd() const;
        void Setup(Segment* pSegment, bool bPlayAtEnd);

        Segment* mSegment;
        unsigned char mFlags;
    };

    // FUNCTION: SWBF2 0x00725220
    RedMoviePlayerBase(unsigned int uiMediaSectorSize);

    virtual ~RedMoviePlayerBase();

    virtual RedTexture* AllocTexture(const unsigned int uiWidth,
                                     const unsigned int uiHeight,
                                     const unsigned int uiFormat,
                                     const unsigned int uiLevels,
                                     const unsigned int uiPool,
                                     const unsigned char ucFlags) = 0;
    virtual void FreeTexture(RedTexture* pTexture, const unsigned int uiIndex,
                             const unsigned char ucFlags) = 0;

    // FUNCTION: SWBF2 0x007263f0
    void SetNotifyCallback(NotifyCallback pfnCallback, void* pData);

    void Close();

    void Flush();

    void Update();

    // SYNTHETIC: SWBF2 0x00725560
    // RedMoviePlayerBase::`scalar deleting destructor'

    void* BufferGetBuffer() const;

    unsigned int BufferGetUsed() const;

    unsigned int BufferReadData(void* pDest, unsigned int uiSize, bool bUpdate);

    void ResetStart();
    bool SetupStart();

    bool Open(const char* const pFileName, RedTexture** ppTextures,
              unsigned int uiNumTextures, int iHeap, unsigned int uiArg4,
              void* pBuffer, unsigned int uiBufferSize, unsigned int uiReadSize,
              unsigned int uiArg8);
    void SeekSegment(const Segment* const pSegment, bool bFlag);
    void Stop();
    void FlushSegmentQueue();
    const Info* GetInfo() const;
    PlayState GetPlayState() const;
    PlayState GetDesiredPlayState() const;
    void SetCurrentPlayState(PlayState eState);
    unsigned int GetNumReallocTextures() const;
    void ReallocTextures(unsigned int uiWidth, unsigned int uiHeight,
                         unsigned int uiFormat, unsigned char ucFlags);
    unsigned int GetNumUsedTextures() const;
    bool GetLooping() const;

    void SetLooping(bool bLoop);
    const Segment* GetCurrentSegment() const;
    unsigned int GetDataLeft() const;
    bool IsOpen() const;
    const RedTexture* LockTexture();
    void UnlockTexture(const RedTexture* const pTexture);
    void Pause();
    void Play();
    void Notify(NotifyType eType);

    RedTexture* GetTexture();
    void TextureComplete(RedTexture* const pTexture);
    bool IsEnd() const;
    bool IsDiscontinuity() const;
    bool GetReadBufferFull() const;

    bool CreateTextures(unsigned int uiWidth, unsigned int uiHeight,
                        unsigned int uiFormat, unsigned char ucFlags);

    void DestroyTextures(bool bForce, unsigned char ucFlags);

    bool TextureUsed(const RedTexture* const pTexture) const;

    unsigned int CreateTextureHash(unsigned int uiIndex) const;

    void CloseHeader();

    bool OpenHeader(PblFileChunk& chunk, int iHeap, unsigned int uiArg4);

    RedTexture** GetAllocTextures() const;

    RedTexture** GetFreeTextures() const;

    RedTexture** GetUsedTextures() const;

    static PblList<RedMoviePlayerBase> sMovieList;

    PblList<RedMoviePlayerBase>::Node mNode;
    RedTexture** mTextures;
    unsigned int mNumTextures;
    NotifyCallback mNotifyCallback;
    void* mNotifyCallbackData;
    void(__cdecl* mBufferingCallback)();
    void* mBufferingCallbackData;
    TextureStack mFreeTextures;
    TextureQueue mUsedTextures;
    Info* mInfo;
    Segment* mCurrentSegment;
    Snd::QueueLock<SegmentSequence, 10> mSegmentQueue;
    PlayState mCurrentPlayState;
    PlayState mDesiredPlayState;
    RedCircularBuffer mBuffer;
    RedCircularBuffer mBufferStart;
    unsigned int mInvalidStartData;
    RedCircularBuffer* mReadBuffer;
    RedFileAsync mFile;
    unsigned int mFileReadSize;
    unsigned int mFileScheduledReadSize;
    unsigned int mMediaSectorSize;
    unsigned int mNumReallocTextures;
    unsigned char mFlags;
};

 
 
 
