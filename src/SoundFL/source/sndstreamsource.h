#pragma once

#include "sndfile.h"
#include "sndstreamnode.h"
#include "sndstreampacket.h"
#include "win/sndsemaphore.h"

namespace Snd
{

class StreamSource : public StreamNodeSource, public StreamNode
{
public:

    enum Flags
    {
        flags_Reading = 1,
        flags_Looping = 2,
        flags_LastPacket = 4,
        flags_SegmentQueued = 8,
        flags_SegmentSetup = 16,
        flags_SeekPending = 32,
        flags_SeekReady = 64,
        flags_MediaSecBase2 = 128
    };

    class SegmentProperties
    {
    public:

        SegmentProperties() : mOffset(0), mLength(0) {}

        unsigned int mOffset;
        unsigned int mLength;
        unsigned int mPadding;
    };

    StreamSource();

    void Close();
    void Flush();

    virtual StreamPacket* GetPacket(unsigned int uiOutput);

    void Update();

    bool Open(File& rFile, unsigned int uiPacketSize, unsigned char uiNumPackets,
              StreamPacket* pPackets, unsigned int uiMediaSectorSize);

    bool GetLooping() const { return (mFlags & flags_Looping) != 0; }

    void SetLooping(bool bLooping);
    void SetSegmentOffsetLength(unsigned int uiOffset, unsigned int uiLength,
                                unsigned int uiPadding);

    virtual ~StreamSource();

private:

    static void __cdecl FileOpComplete(File* pFile, void* pCallbackData,
                                       bool bSuccess);

    void SeekSegment(unsigned int uiPos);

public:
    SegmentProperties mSegment;
    SegmentProperties mSeekSegment;
    SegmentProperties mNextSegment;
    File mFile;
    Semaphore mLock;
    unsigned char mFlags;
    unsigned char mPacketReadPos;
    unsigned char mPacketLockPos;
    unsigned char mNumPackets;
    unsigned int mPacketSize;
    StreamPacket* mPackets;
    unsigned int mSeekPos;
    unsigned int mReadSize;
    unsigned int mMediaSectorSize;
};

 
 

}
