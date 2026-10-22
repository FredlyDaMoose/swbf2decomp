#pragma once

#include "SoundFL/source/sndstreambase.h"
#include "SoundFL/source/sndstreamimaadpcmdecoder.h"
#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndstreamsource.h"
#include "SoundFL/source/sndsubstreamsplitter.h"
#include "sndstreamrenderer.h"

class PblFileChunk;

namespace Snd
{

class Stream : public StreamBase
{
public:
    Stream();

    static void EnableStreaming(bool bEnable);

    void Close();

    void Flush(bool bThreadPaused, bool bFlushRenderers);

    void Update(float fDeltaTime);

    bool Open(PblFileChunk& chunk, unsigned int uiSegment, char const* pcFileName,
              void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData,
              unsigned int uiArg);

    bool AppendSegments(PblFileChunk& chunk, unsigned int uiHashID, const char* pcFileName,
                        void (__cdecl* pfnIdle)(StreamBase*, void*), void* pIdleData);

protected:

    void Connect(StreamNodeSink** ppSinks);
    void Open(File& rFile);
    bool OpenFile(File& rFile, char const* pcFileName,
                  void (__cdecl* pfnIdle)(StreamBase*, void*),
                  void* pIdleData);

private:
    static void __cdecl FileOpenCallback(File* pFile, void* pData,
                                         bool bSuccess);

public:

    ~Stream();

    StreamPacket mPackets[8];
    unsigned char mStreamBuf[8][110592];
    StreamSource mSource;
    SubstreamSplitter mSplitter;
    StreamPacket mSplitterPackets[24];
    unsigned short mSplitterReadPos[3];
    unsigned short mSplitterLockPos[3];
    StreamIMAADPCMDecoder mADPCMDecoder[3];
    StreamPacket mADPCMPackets[3][8];
    unsigned char mADPCMPacketBuf[3][8][110592];
    StreamRenderer mRenderer[3];
    unsigned char mOpening;

    static bool smFileOpening;
    static bool smEnableStreaming;
};

 

}
