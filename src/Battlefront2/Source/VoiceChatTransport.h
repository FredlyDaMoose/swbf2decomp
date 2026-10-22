#ifndef VOICECHATTRANSPORT_H
#define VOICECHATTRANSPORT_H

#include <stddef.h>

namespace VoiceChat
{
class Transport
{
public:
    class Bitmaps
    {
    public:

        Bitmaps() : mSend(0), mReceive(0) {}

        unsigned char* mSend;
        unsigned char* mReceive;
        unsigned char* mConnected;
        unsigned char* mSending;
    };

    class Method
    {
    public:
        enum Type
        {
            type_invalid = 0,
            type_none = 1,
            type_direct = 2,
            type_relay = 3,
            type_serverRelay = 4,
            type_max = 5,
            type_max_log2 = 3
        };

        Method(Type type, unsigned char numRelayNodes);

        // FUNCTION: SWBF2 0x006745e0
        void SetType(Type type) { mType = type; }
        // FUNCTION: SWBF2 0x00674600
        void SetNumRelayNodes(unsigned char numRelayNodes) { mNumRelayNodes = numRelayNodes; }
        unsigned char GetNumRelayNodes() const;

        // FUNCTION: SWBF2 0x006745f0
        Type GetType() const { return mType; }

        unsigned char CalculateOptimalRelayNodes(int iNumNodes) const;

        Type mType;
        unsigned char mNumRelayNodes;
    };

    Transport();
    ~Transport();

    bool Open();
    void Close();
    void SetMethod(void* pSrc, int bBoth);

    // FUNCTION: SWBF2 0x00674f40
    const Method& GetMethod() const { return mMethodIncoming; }

    void SetLocalNode(unsigned char node, bool local, bool receive, bool send);
    bool IsConnectedSend(unsigned char node) const;
    bool IsConnectedReceive(unsigned char node) const;
    bool IsConnectedNet(unsigned char node) const;

    unsigned char GetNumNodes() const;
    unsigned char GetNumNodesLog2() const;
    bool IsLocal(unsigned char node) const;
    bool GetSendPossible(unsigned char localNode, unsigned char node) const;
    bool GetSendEnable(unsigned char localNode, unsigned char node) const;
    bool GetReceiveEnable(unsigned char localNode, unsigned char node) const;
    void DisconnectNode(unsigned char node);
    bool GetSending(unsigned char node) const;
    void SetSending(unsigned char node, bool sending);
    void SetSendEnable(unsigned char localNode, unsigned char node, bool enable, bool unused);
    void SetReceiveEnable(unsigned char localNode, unsigned char node, bool enable, bool unused);

    bool GetReceivePossible(unsigned char node, unsigned char stream) const;

private:
    unsigned char* GetBitmapElementAndMask(unsigned char* const bitmap, unsigned char x,
                                           unsigned char y, unsigned char& mask);
    bool SetBit(unsigned char* const bitmap, unsigned char x, unsigned char y, bool value,
                unsigned char* const changed, bool changedByX);
    bool GetBit(const unsigned char* const bitmap, unsigned char x, unsigned char y) const;
    void SetColumnBits(unsigned char* const bitmap, unsigned char x, bool value,
                       unsigned char except);
    void SetRowBits(unsigned char* const bitmap, unsigned char y, bool value,
                    unsigned char except);
    void UpdateLocalState(unsigned char node, bool);

public:

    Bitmaps mLocal;
    Bitmaps mIncoming;
    unsigned char* mClientChangedNodes;
    unsigned char* mHostChangedNodes;
    unsigned int* mHostChangedPckID;
    unsigned char* mLocalNodes;
    unsigned char* mHostUpdated;
    unsigned int mClientChangedPckID;
    unsigned int mSize;
    unsigned char mWidthHeight;
    unsigned char mWidthBytes;
    unsigned char mNumNodes;
    unsigned char mNumNodesLog2;
    unsigned char mNumLocalNodes;
    unsigned char mMaxLocalNodes;
    Method mMethodOutgoing;
    Method mMethodIncoming;
    bool mClientUpdated : 1;
};

 
 
 
 
 
 
 
 
 
}

#endif
