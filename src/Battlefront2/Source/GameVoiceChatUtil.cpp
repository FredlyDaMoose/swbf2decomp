#include "GameVoiceChatUtil.h"

#include "NetGame.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatTransport.h"
#include "VoiceChatNode.h"

#include "NetPacket.h"
#include "SoundFL/source/sndqueue.h"

namespace GameVoiceChat { void* GetNode(); }

// FUNCTION: SWBF2 0x0053e300
void* VoiceChat::NetPacketAudio::Receive()
{
    NetPacket* pPacket = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (!GameVoiceChat::gPacketQueue[i].IsEmpty())
        {
            GameVoiceChat::gPacketQueue[i].Get(pPacket);
            break;
        }
    }

    if (pPacket == 0)
    {
        pPacket = NetComm::Recv(PKT_VOICE, PKT_NONE);
        if (pPacket == 0)
        {
            return 0;
        }
    }

    pPacket->BeginRead();
    return pPacket;
}

// FUNCTION: SWBF2 0x0053e170
VoiceChat::NetPacketAudio::NetPacketAudio(void* packet) : mAppPacket(packet) {}

// FUNCTION: SWBF2 0x0053e180
unsigned int VoiceChat::NetPacketAudio::Write(const void* pData, unsigned int uiSize)
{
    return ((NetPacket*)mAppPacket)->Write((const char*)pData, (int)uiSize);
}

// FUNCTION: SWBF2 0x0053e190
unsigned int VoiceChat::NetPacketAudio::Read(void* pData, unsigned int uiSize)
{
    return ((NetPacket*)mAppPacket)->Read((char*)pData, (int)uiSize);
}

// FUNCTION: SWBF2 0x0053e1a0
void* VoiceChat::NetPacketAudio::Read(unsigned int& length)
{
    NetPacket* current = (NetPacket*)mAppPacket;
    unsigned int available = (unsigned int)((int)current->mMaxIndex - (int)current->mCurIndex);
    char* data = current->mData + current->mCurIndex;
    if (length > available)
    {
        length = available;
    }
    current->mCurIndex += (short)length;
    return data;
}

// FUNCTION: SWBF2 0x0053e1d0
void* VoiceChat::NetPacketAudio::Alloc()
{
    NetPacket* packet = NetPacket::Alloc(false);
    if (packet)
    {
        packet->BeginWrite();
    }
    return packet;
}

// FUNCTION: SWBF2 0x0053e380
void VoiceChat::NetPacketAudio::Free(void* pPacket)
{
    ((NetPacket*)pPacket)->Free();
}

// TEMPLATE: SWBF2 0x0053e3e0
// Snd::Queue<NetPacket *,4>::~Queue<NetPacket *,4>
template Snd::Queue<NetPacket*, 4>::~Queue();

namespace GameVoiceChat {

// FUNCTION: SWBF2 0x0053e1f0
bool __fastcall SendVoice(NetPacket* packet, unsigned char destination, unsigned short sequence)
{
    unsigned int node = destination == 0xff ? 64 : destination;
    if (netInShell ? netEnabled : netEnabledNext)
    {
        if (NetComm::IsConnected(node))
        {
            NetComm::SendVoice(node, packet, sequence);
            return true;
        }
    }
    else if (destination != 0xff)
    {
        GameVoiceChat::Transport* transport = GameVoiceChat::GetTransport();
        if (transport)
        {
            if (transport->IsLocal((unsigned char)node))
            {
                unsigned int count = GameVoiceChat::GetNumNodes();
                unsigned int i = 0;
                if (count)
                {
                    VoiceChat::Node* voiceNode = (VoiceChat::Node*)GameVoiceChat::GetNode();
                    while (i < count)
                    {
                        if (voiceNode && voiceNode->GetID() == node)
                        {
                            break;
                        }
                        ++i;
                    }
                }
                Snd::Queue<NetPacket*,4>& queue = GameVoiceChat::gPacketQueue[i];
                if (!queue.IsFull())
                {
                    queue.Add(packet);
                    return true;
                }
            }
        }
    }
    VoiceChat::NetPacketAudio::Free(packet);
    return false;
}

}

// FUNCTION: SWBF2 0x0053e2e0
bool __fastcall VoiceChat::NetPacketAudio::Send(NetPacket** packet, unsigned char destination,
                            unsigned short sequence)
{
    (*packet)->EndWrite();
    return GameVoiceChat::SendVoice(*packet, destination, sequence);
}

// TEMPLATE: SWBF2 0x0053e3f0
// Snd::Queue<NetPacket*,4>::Add(NetPacket* const&)
template class Snd::Queue<NetPacket*, 4>;

// TEMPLATE: SWBF2 0x0053e3d0
// Snd::Queue<NetPacket *,4>::Queue

class NetPacket;

namespace GameVoiceChat
{

// GLOBAL: SWBF2 0x01ec6020
// SYNTHETIC: SWBF2 0x00403ef0
// `dynamic initializer for 'GameVoiceChat::gPacketQueue''
// SYNTHETIC: SWBF2 0x00768af0
// `dynamic atexit destructor for 'GameVoiceChat::gPacketQueue''
Snd::Queue<NetPacket*, 4> gPacketQueue[4];
}
