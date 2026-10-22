#ifndef VOICECHATNODE_H
#define VOICECHATNODE_H

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

class SoundDebugDisplay;

namespace VoiceChat
{
struct AudioPacket;
}

namespace VoiceChat
{

class Encoder
{
public:
    Encoder();
    AudioPacket* GetData();

    void FreeAudioPacket(AudioPacket* packet);
    void Play();

    bool Open();
    void Close();
    void Stop();
    unsigned char IsEnabled();
    unsigned int GetMaxOutputPacketSizeBytes() const;
    float GetMaxOutputPacketSizeMs() const;
    void SetGain(float fGain);

    void* mAppData;
};

class Decoder
{
public:
    Decoder();
    void Play();
    bool IsEnabled();

    bool ReceivePacket(unsigned int input, const AudioPacket* packet);
    bool Update();
    bool IsStarving(unsigned int input, bool force);

    bool Open(float fPacketMs, int iSlots);
    void Close();
    void Stop();

    void SetGain(float fGain, unsigned int uiOutput);
    float GetGain(unsigned int uiOutput) const;
    unsigned int GetNumOutputs() const;

    void* mAppData;
};

class Transport;
class NetPacketAudio;

class Node : public PblList<Node>::Node
{
public:
    class Header
    {
    public:

        enum Format
        {
            format_invalid = 0,
            format_direct = 1,
            format_relay = 2
        };

        Header();

        void SendSetup(const Transport& transport, unsigned char node, unsigned char receiver,
                       bool send, unsigned short sequence);
        void SetReceiveNode(const Transport& transport, unsigned char node, bool receive);
        bool GetReceiveNode(const Transport& transport, unsigned char node);
        unsigned int Write(NetPacketAudio* const pPacket) const;
        bool Read(const Transport& transport, NetPacketAudio* const pPacket);
        unsigned char GetSendNodeID(const Transport& transport) const;
        unsigned char GetNumReceiveNodes(const Transport& transport) const;
        bool GetDiscontinuity() const;
        unsigned short GetSeqNum() const;
        Format GetFormat() const;

    protected:
        void Reset();
        void WriteBits(unsigned int value, unsigned char count);
        unsigned int ReadBits(unsigned int bit, unsigned char count) const;
        unsigned int CalculateSize(const Transport& transport, Format format) const;

    public:
        unsigned char* mBitmap;
        unsigned int mSize;
        unsigned int mBits;
    };

    Node(Encoder* const encoder, Decoder* const decoder);

    static Header sReadHeader;
    static Header sWriteHeader;

    static bool IsOpenStatic();

    unsigned char GetID() const;

    Encoder& GetEncoder() const;
    Decoder& GetDecoder() const;

    static void Update(float fDeltaTime);

    float GetLastSendTime() const;
    bool IsReceiving() const;
    bool IsCapturing() const;
    static float GetTime();

    static float GetLastReceiveTime(unsigned char node);
    static float GetBufferSizeMs();

    static Node* FindByID(unsigned char uNodeID);

    bool Open();
    bool IsOpen() const;
    void Close();
    void SetID(unsigned char uChannel);
    unsigned int UpdateReceiveStreamLatency(unsigned char ucIndex);
    static bool OpenStatic(float fSeconds, float fA, float fB);
    static void CloseStatic();
    static void SetSkipOutOfSequence(float fRate);
    void Close(bool, bool);

    struct ReceiveStream
    {
        AudioPacket* packet;
        float bufferedData;
        float activationTime;
        unsigned int lastPacketSize;
        unsigned short numPackets;
        unsigned short writeIndex;
        unsigned short readIndex;
        unsigned char decoderInput;
        bool activated : 1;
        bool starving : 1;
    };

    const ReceiveStream& GetReceiveStream(unsigned char stream) const;
    unsigned int CalculateNumPackets(const ReceiveStream& stream) const;

    static bool __fastcall ReceiveAudioPacketAllLocalNodes(unsigned char uID, const AudioPacket* pPacket,
                                                          bool bDiscontinuity, unsigned short uDropped);
    friend class ::SoundDebugDisplay;

protected:
    void Send();
    void Decode(float fDeltaTime);
    void Flush(ReceiveStream& stream, AudioPacket& packet);
    bool DecoderReceiveAudioPacket(unsigned char stream, const AudioPacket* const packet);

    void DecodeQueue(unsigned char stream, bool force);
    void RepeatLastPacket(unsigned char stream);
    bool DecoderIsStarving(unsigned char stream, bool force);
    bool ReceiveAudioPacket(unsigned char stream, const AudioPacket* packet);

    void SendAudioPacket(const AudioPacket* const packet);
    static bool SendAudioPacket(const AudioPacket* const audio, const Header* const header,
                                unsigned char destination, float* const sendTime);
    static void SendAudioPacketPeerRelay(const AudioPacket* const packet, unsigned char first,
                                         unsigned char numNodes, unsigned char maxRelayNodes,
                                         unsigned short sequence, bool incoming,
                                         float* const sendTime);
    static void RelayAudioPacket(const AudioPacket* const packet, unsigned char node,
                                 unsigned short sequence);
    unsigned char DecoderInputAlloc(unsigned char stream);
    void DecoderInputFree(unsigned char decoderInput, bool flush);

public:

    Encoder* mEncoder;
    Decoder* mDecoder;
    float mEncoderPacketSizeSec;
    float mLastCaptureTime;
    float mLastSendTime;
    float mLastReceiveTime;
    float mBufferUpdateTime;
    ReceiveStream* mReceiveStreams;
    unsigned char* mAllocatedInputs;
    unsigned int mMaxReceiveSize;
    unsigned short mMaxReceivePackets;
    unsigned short mSendSeq;
    unsigned char mNodeID;
    unsigned char mNumDecoderInputs;
    bool mIsOpen : 1;

    struct ReceiveMetrics
    {
        float receiveLatency;
        unsigned int lastPacketSize;
        float lastReceiveTime;
        unsigned short receiveSeq;
        unsigned int starvedCount;
        float starvedFrequency;
        float lastStarveTime;
        unsigned int outOfOrderCount;
        bool activated : 1;
    };
    struct SendMetrics
    {
        float lastSendInterval;
        float lastSendTimeDirect;
        unsigned int sendCountDirect;
        float lastSendTime;
        unsigned int sendCount;
        unsigned int droppedCount;
        float bandwidth;
        float bandwidthAverage;
    };

    static const ReceiveMetrics& GetReceiveMetrics(unsigned char node);
    static const SendMetrics& GetSendMetrics(unsigned char node);
public:
    static void* sTransport;
    static unsigned char sSkipDropped;
    static float sBufferSizeMs;
    static unsigned short sRepeatMax;
    static void* sSendMetrics;
    static void* sReceiveMetrics;
    static int sTime;
    static void* sList[4];
    static unsigned short sSeqNumVariance;
    static float sReceiveBufferSize;
};

struct AudioPacket
{
    void* mData;
    unsigned int mSize;
    void* mAppData;
    bool mDiscontinuity : 1;
};

 
 
 
 
 
 
 
 
 
 
 
 
 
}

 
 
 
 
 
 
 
 
 
 

#endif
