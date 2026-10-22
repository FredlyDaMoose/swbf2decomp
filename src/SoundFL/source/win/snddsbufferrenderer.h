#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "SoundFL/source/sndallocator.h"
#include "SoundFL/source/sndqueue.h"
#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndsource2d3d.h"
#include "SoundFL/source/sndsource3d.h"
#include "SoundFL/source/sndsourcei3dl2.h"
#include "SoundFL/source/sndstreamnode.h"
#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "snddsbuffer.h"
#include "sndsemaphore.h"

namespace Snd
{

class DSBufferRenderer : public ThreadUpdate::Object,
                         public Allocator<DSBufferRenderer>,
                         public StreamNode,
                         public StreamNodeSink,
                         public Source2D3D,
                         public Source3D,
                         public SourceI3DL2Interpolated,
                         public DSBuffer
{
public:
    DSBufferRenderer();

    virtual ~DSBufferRenderer();

    virtual void UpdateObject();

    void Play(bool bRestart);

    bool SetFormat(SampleInfo::Format eFormat, unsigned int uiNumChannels,
                   unsigned int uiFrequency, bool b3D, float fBufferMs,
                   bool bRestart, IDirectSound* pDirectSound,
                   unsigned long ulFlags, bool bPrimary);

    void Flush();

    float GetBufferSizeMs() const;
    void Set3DMode(bool b3D, unsigned long ulFlags);

    void SetFrequency(unsigned int uiFrequency) { mFrequency = uiFrequency; }
    void SetGain(float fGain) { Source::SetGain(fGain); }
    void SetPan(float fPan) { Source2D::SetPan(fPan); }

    void SetLowTideMark(float fMilliseconds);

    static void UpdateParameters();

private:

    void UpdateBufferParameters(IDirectSoundBuffer* const pBuffer,
                                bool bUnusedArg4, bool bImmediate);

    void UpdateGain(bool bEnable, float fGain);

    void QueuePackets();

    void ResetGain();

    void StopBuffer();

    unsigned int WriteData(void* pData, unsigned int uiSize,
                           SourceTransport::PlayState ePlayState);

    const StreamPacket* PeekPacket() const;

public:
    PblListSingle::Node mNode;
    unsigned int mFrequency;
    unsigned int mNumChannels;
    SampleInfo::Format mFormat;
    Semaphore mLock;
    unsigned int mBufferSize;
    unsigned int mPrevWritePos;
    unsigned int mPrevPlayPos;
    unsigned int mNextLockPos;
    unsigned int mBufferedData;
    unsigned int mLeftToPlay;
    unsigned int mPacketRead;
    unsigned int mFrameSize;
    Queue<StreamPacket*, 4> mPacketQueue;
    unsigned int mBufferFillSize;
    unsigned char mFlags;
    int mGainTarget[2];
    int mGainCurrent[2];

    static PblListSingle smRendererList;
};

 

}
