#pragma once

#include <stddef.h>

#include "VoiceChatNode.h"
#include "SoundFL/source/sndstreamagc.h"
#include "SoundFL/source/sndstreamgsmencoder.h"
#include "SoundFL/source/sndstreamvad.h"
#include "SoundFL/source/win/sndstreamcapturesource.h"

namespace Snd { struct StreamPacket; }

namespace GameVoiceChat
{

class Encoder : public VoiceChat::Encoder
{
public:
    Encoder();
    ~Encoder();

    VoiceChat::AudioPacket* GetData();
    void Play();

    bool Open(float fMaxPacketMs);
    void FreeBuffers();
    void Close();
    void FreeAudioPacket(VoiceChat::AudioPacket* packet);
    void Flush();
    void Stop();
    bool IsPlaying();
    unsigned char IsEnabled();
    void SetGain(float fGain);
    unsigned int GetMaxOutputPacketSizeBytes() const;
    float GetMaxOutputPacketSizeMs() const;

    Snd::StreamCaptureSource mSource;
    Snd::StreamGSMEncoder mEncoder;
    Snd::StreamVAD mVAD;
    Snd::StreamAGC mAGC;
    float mPacketSizeMs;
    unsigned int mPacketSizeBytes;
    float mSourceGain;
    unsigned int mNumBuffers;
    unsigned char* mCaptureBuffer;
    Snd::StreamPacket* mCapturePackets;
    unsigned char* mEncoderBuffer;
    Snd::StreamPacket* mEncoderPackets;
    VoiceChat::AudioPacket* mAudioPacket;
    unsigned char mAudioPacketIndex;
    unsigned char mSourceDeviceIndex;
    bool mPlaying : 1;
};
}

 
 
 
 
 
 
 
 
 
