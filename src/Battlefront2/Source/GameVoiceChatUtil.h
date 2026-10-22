#pragma once

#include "NetComm.h"
#include "SoundFL/source/sndqueue.h"

namespace GameVoiceChat
{

extern Snd::Queue<NetPacket*, 4> gPacketQueue[4];
}

namespace VoiceChat
{

void* Alloc(unsigned int uiSize);
void Free(void* pBlock);

class NetPacketAudio
{
public:

    static void* Receive();

    NetPacketAudio(void* pAppPacket);
    void* Read(unsigned int& uiSize);
    static void Free(void* pAppPacket);

    unsigned int Write(const void* pData, unsigned int uiSize);
    unsigned int Read(void* pData, unsigned int uiSize);
    static void* Alloc();

    static bool __fastcall Send(NetPacket** ppPacket, unsigned char uDestination, unsigned short uSequence);

    void* mAppPacket;
};
}

 
