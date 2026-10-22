#pragma once

#if !defined(DIRECTSOUND_VERSION) || DIRECTSOUND_VERSION < 0x0800
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800
#endif

#include <stddef.h>
#include <windows.h>
#include <dsound.h>

#include "PebbleFL/Common/PblListSingle.h"
#include "SoundFL/source/sndstreamcapturesourcebase.h"
#include "SoundFL/source/sndstreamformat.h"
#include "SoundFL/source/sndstreampacketio.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "sndsemaphore.h"

namespace Snd
{

// VTABLE: SWBF2 0x007869c0
class StreamCaptureSource : public StreamCaptureSourceBase,
                            public ThreadUpdate::Object
{
public:
    StreamCaptureSource();
    virtual ~StreamCaptureSource();
    void Close();
    void Flush(bool bAlreadyLocked);
    bool Open(unsigned char ucDeviceIndex, StreamFormat const& cFormat,
              unsigned int uiPacketSize, unsigned char ucNumPackets,
              StreamPacket* pPackets);

private:
    virtual void UpdateObject();

public:
    PblListSingle::Node mNode;
    StreamFormat mFormat;
    Semaphore mLock;
    IDirectSoundCapture* mCaptureDevice;
    IDirectSoundCaptureBuffer8* mCaptureBuffer;
    unsigned int mCaptureBufferSize;
    unsigned int mNextLockPos;
    StreamPacketIOLite mOutputPacket;
    unsigned int mFrameSize;
};
 
 
 
 
 
 
 
 
 
 

}
