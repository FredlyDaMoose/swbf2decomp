#include <string.h>

#include "VoiceChatTransport.h"
#include "GameVoiceChat.h"

#include "VoiceChatUtil.h"

struct VoiceChatTransportFields
{

    void*          mLocalBitmapPointers[4];
    void*          mIncomingBitmapPointers[4];
    void*          mChangeTrackingPointers[4];
    void*          mHostUpdated;
    unsigned int   mClientChangedPckID;
    unsigned int   mSize;

    unsigned short mBitmapDimensions;
    unsigned char  mNumNodes;
    unsigned char  mNumNodesLog2;
    unsigned short mLocalNodeLimits;
    unsigned short mReservedNodeLimitBytes;
    unsigned char  mMethodOutgoing[8];
    unsigned char  mMethodIncoming[8];

    unsigned char  mClientUpdatedFlags;
};

namespace VoiceChat { void* Alloc(unsigned int uiSize); }

namespace VoiceChat { void Free(void* pBlock); }

namespace VoiceChat { int GetMSBIndex(); }

// FUNCTION: SWBF2 0x006745c0
VoiceChat::Transport::Method::Method(Type type, unsigned char numRelayNodes)
{
    SetType(type);
    SetNumRelayNodes(numRelayNodes);
}

// FUNCTION: SWBF2 0x00674620
unsigned char VoiceChat::Transport::Method::CalculateOptimalRelayNodes(int iNumNodes) const
{
    const unsigned char ucNodes = (unsigned char)iNumNodes;
    const unsigned char ucSelf = mNumRelayNodes;

    unsigned char ucRelay = (ucSelf > ucNodes) ? ucNodes : ucSelf;

    while (ucRelay > (unsigned char)((int)ucNodes / (int)(ucRelay + 1)))
    {
        --ucRelay;
    }

    return ucRelay ? ucRelay : (unsigned char)1;
}

// FUNCTION: SWBF2 0x006749b0
unsigned char VoiceChat::Transport::GetNumNodes() const
{
    return mNumNodes;
}

// FUNCTION: SWBF2 0x006749c0
unsigned char VoiceChat::Transport::GetNumNodesLog2() const
{
    return mNumNodesLog2;
}

// FUNCTION: SWBF2 0x00674f10
void VoiceChat::Transport::SetMethod(void* pSrc, int bBoth)
{
    const Method& method = *(const Method*)pSrc;
    mMethodOutgoing = method;

    if (bBoth)
    {
        mMethodIncoming = method;
    }
}

// FUNCTION: SWBF2 0x00674700
bool VoiceChat::Transport::Open()
{
    VoiceChatTransportFields* t = (VoiceChatTransportFields*)this;

    char* p = (char*)VoiceChat::Alloc(0x8BC);
    t->mLocalBitmapPointers[0] = p;
    if (!p)
    {
        return false;
    }

    p += 0x188; t->mClientChangedPckID = 0;      t->mLocalBitmapPointers[1] = p;
    p += 0x188;                            t->mLocalBitmapPointers[2] = p;
    p += 0x10;                             t->mLocalBitmapPointers[3] = p;
    p += 0x08;                             t->mIncomingBitmapPointers[0] = p;
    p += 0x188;                            t->mIncomingBitmapPointers[1] = p;
    p += 0x188;                            t->mIncomingBitmapPointers[2] = p;
    p += 0x10;                             t->mIncomingBitmapPointers[3] = p;
    p += 0x08;                             t->mChangeTrackingPointers[0] = p;
    p += 0x08;                             t->mChangeTrackingPointers[1] = p;
    p += 0x188;                            t->mChangeTrackingPointers[2] = p;
    p += 0xC8;                             t->mChangeTrackingPointers[3] = p;
    p += 0x04;                             t->mHostUpdated = p;

    t->mSize = 0x188;
    t->mBitmapDimensions = 0x738;
    t->mNumNodes = 0x32;
    t->mNumNodesLog2 = (unsigned char)(VoiceChat::GetMSBIndex() + 1);
    t->mClientUpdatedFlags &= 0xFE;
    t->mLocalNodeLimits = 0x100;

    ((VoiceChat::Transport::Method*)t->mMethodOutgoing)->SetType((VoiceChat::Transport::Method::Type)1);
    ((VoiceChat::Transport::Method*)t->mMethodOutgoing)->SetNumRelayNodes(0);
    *(unsigned int*)&t->mMethodIncoming[0] = *(unsigned int*)&t->mMethodOutgoing[0];
    *(unsigned int*)&t->mMethodIncoming[4] = *(unsigned int*)&t->mMethodOutgoing[4];

    memset(t->mLocalBitmapPointers[0], 0, t->mSize);
    memset(t->mLocalBitmapPointers[1], 0, t->mSize);

    unsigned char* q = (unsigned char*)t->mLocalBitmapPointers[2];
    *(unsigned int*)(q + 0) = 0;
    *(unsigned int*)(q + 4) = 0;
    *(unsigned int*)(q + 8) = 0;
    *(unsigned short*)(q + 0xC) = 0;

    q = (unsigned char*)t->mLocalBitmapPointers[3];
    *(unsigned int*)(q + 0) = 0;
    *(unsigned short*)(q + 4) = 0;
    *(unsigned char*)(q + 6) = 0;

    memset(t->mIncomingBitmapPointers[0], 0, t->mSize);
    memset(t->mIncomingBitmapPointers[1], 0, t->mSize);

    q = (unsigned char*)t->mIncomingBitmapPointers[2];
    *(unsigned int*)(q + 0) = 0;
    *(unsigned int*)(q + 4) = 0;
    *(unsigned int*)(q + 8) = 0;
    *(unsigned short*)(q + 0xC) = 0;

    q = (unsigned char*)t->mIncomingBitmapPointers[3];
    *(unsigned int*)(q + 0) = 0;
    *(unsigned short*)(q + 4) = 0;
    *(unsigned char*)(q + 6) = 0;

    q = (unsigned char*)t->mChangeTrackingPointers[0];
    *(unsigned int*)(q + 0) = 0;
    *(unsigned short*)(q + 4) = 0;
    *(unsigned char*)(q + 6) = 0;

    memset(t->mChangeTrackingPointers[1], 0, t->mSize);

    memset(t->mChangeTrackingPointers[2], 0, (unsigned int)t->mNumNodes * 4);

    *(unsigned char*)t->mChangeTrackingPointers[3] = 0;

    q = (unsigned char*)t->mHostUpdated;
    *(unsigned int*)(q + 0) = 0;
    *(unsigned short*)(q + 4) = 0;
    *(unsigned char*)(q + 6) = 0;

    return true;
}

// FUNCTION: SWBF2 0x00674920
void VoiceChat::Transport::Close()
{
    VoiceChatTransportFields* t = (VoiceChatTransportFields*)this;

    VoiceChat::Free(t->mLocalBitmapPointers[0]);

    t->mClientUpdatedFlags = (unsigned char)(t->mClientUpdatedFlags & 0xFE);

    t->mLocalBitmapPointers[0] = 0;
    t->mLocalBitmapPointers[1] = 0;
    t->mLocalBitmapPointers[2] = 0;
    t->mLocalBitmapPointers[3] = 0;
    t->mIncomingBitmapPointers[0] = 0;
    t->mIncomingBitmapPointers[1] = 0;
    t->mIncomingBitmapPointers[2] = 0;
    t->mIncomingBitmapPointers[3] = 0;
    t->mChangeTrackingPointers[0] = 0;
    t->mChangeTrackingPointers[1] = 0;
    t->mChangeTrackingPointers[2] = 0;
    t->mChangeTrackingPointers[3] = 0;

    t->mHostUpdated = 0;
    t->mClientChangedPckID = 0;
    t->mSize = 0;

    *(unsigned int*)&t->mBitmapDimensions = 0;

    t->mLocalNodeLimits = 0;

    ((VoiceChat::Transport::Method*)t->mMethodOutgoing)->SetType((VoiceChat::Transport::Method::Type)0);
    ((VoiceChat::Transport::Method*)t->mMethodOutgoing)->SetNumRelayNodes(0);

    *(unsigned int*)&t->mMethodIncoming[0] = *(unsigned int*)&t->mMethodOutgoing[0];
    *(unsigned int*)&t->mMethodIncoming[4] = *(unsigned int*)&t->mMethodOutgoing[4];
}

// FUNCTION: SWBF2 0x00674670
VoiceChat::Transport::Transport()
    : mClientChangedNodes(0), mHostChangedNodes(0), mHostChangedPckID(0), mLocalNodes(0),
      mHostUpdated(0), mClientChangedPckID(0), mSize(0), mWidthHeight(0), mWidthBytes(0),
      mNumNodes(0), mNumNodesLog2(0), mMethodOutgoing(Method::type_invalid, 0),
      mMethodIncoming(Method::type_invalid, 0)
{
    mClientUpdated = false;
}

VoiceChat::Transport::~Transport()
{
}

// FUNCTION: SWBF2 0x00674f50
unsigned char* VoiceChat::Transport::GetBitmapElementAndMask(unsigned char* const bitmap,
                                                             unsigned char x, unsigned char y,
                                                             unsigned char& mask)
{
    return VoiceChat::GetBitmapElementAndMask(bitmap, y * mWidthHeight + x, mask);
}

// FUNCTION: SWBF2 0x00674f80
bool VoiceChat::Transport::SetBit(unsigned char* const bitmap, unsigned char x, unsigned char y,
                                  bool value, unsigned char* const changed, bool changedByX)
{
    unsigned char mask;
    unsigned char* element = GetBitmapElementAndMask(bitmap, x, y, mask);
    const unsigned char old = *element;
    if (value)
    {
        *element = old | mask;
    }
    else
    {
        *element = ~mask & old;
    }

    const bool bChanged = *element != old;
    if (bChanged && changed)
    {
        VoiceChat::SetBit(changed, changedByX ? x : y, true);
    }
    return bChanged;
}

// FUNCTION: SWBF2 0x00674ff0
bool VoiceChat::Transport::GetBit(const unsigned char* const bitmap, unsigned char x,
                                  unsigned char y) const
{
    unsigned char mask;
    const unsigned char* element = const_cast<Transport*>(this)->GetBitmapElementAndMask(
        const_cast<unsigned char*>(bitmap), x, y, mask);
    return (*element & mask) != 0;
}

// FUNCTION: SWBF2 0x00674e00
bool VoiceChat::Transport::GetSendEnable(unsigned char localNode, unsigned char node) const
{
    return GetBit(mLocal.mSend, node, localNode);
}

// FUNCTION: SWBF2 0x00674e50
bool VoiceChat::Transport::GetReceiveEnable(unsigned char localNode, unsigned char node) const
{
    return GetBit(mLocal.mReceive, node, localNode);
}

// FUNCTION: SWBF2 0x00675020
void VoiceChat::Transport::SetColumnBits(unsigned char* const bitmap, unsigned char x, bool value,
                                         unsigned char except)
{
    for (unsigned char i = 0; i < mNumNodes; ++i)
    {
        if (i != except)
        {
            SetBit(bitmap, x, i, value, 0, true);
        }
    }
}

// FUNCTION: SWBF2 0x00675070
void VoiceChat::Transport::SetRowBits(unsigned char* const bitmap, unsigned char y, bool value,
                                      unsigned char except)
{
    for (unsigned char i = 0; i < mNumNodes; ++i)
    {
        if (i != except)
        {
            SetBit(bitmap, i, y, value, 0, true);
        }
    }
}

// FUNCTION: SWBF2 0x00674c80
void VoiceChat::Transport::UpdateLocalState(unsigned char node, bool)
{
    if (!IsLocal(node))
    {
        unsigned char mask;
        memcpy(GetBitmapElementAndMask(mLocal.mSend, 0, node, mask),
               GetBitmapElementAndMask(mIncoming.mSend, 0, node, mask), mWidthBytes);
        memcpy(GetBitmapElementAndMask(mLocal.mReceive, 0, node, mask),
               GetBitmapElementAndMask(mIncoming.mReceive, 0, node, mask), mWidthBytes);

        const bool bReceive = GetBit(mIncoming.mConnected, node, 0);
        const bool bSend = GetBit(mIncoming.mConnected, node, 1);
        const bool bChanged = SetBit(mLocal.mConnected, node, 1, bSend, 0, true) |
                              SetBit(mLocal.mConnected, node, 0, bReceive, 0, true);
        const bool bConnected = bChanged && (bReceive || bSend);

        VoiceChat::SetBit(mLocal.mSending, node, VoiceChat::GetBit(mIncoming.mSending, node));
        if (bConnected)
        {
            SetRowBits(mHostChangedNodes, node, true, node);
            VoiceChat::SetBit(mHostUpdated, node, true);
        }
    }
    SetColumnBits(mHostChangedNodes, node, true, node);
    SetRowBits(mHostUpdated, 0, true, node);
}

// FUNCTION: SWBF2 0x00674b00
void VoiceChat::Transport::DisconnectNode(unsigned char node)
{
    const bool bSend = SetBit(mIncoming.mConnected, node, 1, false, 0, true);
    const bool bReceive = SetBit(mIncoming.mConnected, node, 0, false, 0, true);
    if (bSend | bReceive)
    {
        unsigned char mask;
        memset(GetBitmapElementAndMask(mIncoming.mSend, 0, node, mask), 0, mWidthBytes);
        memset(GetBitmapElementAndMask(mIncoming.mReceive, 0, node, mask), 0, mWidthBytes);
        SetBit(mIncoming.mSending, node, 0, false, 0, true);
        UpdateLocalState(node, false);
    }
}

// FUNCTION: SWBF2 0x00674ac0
bool VoiceChat::Transport::IsLocal(unsigned char node) const
{
    for (unsigned char i = 0; i < mNumLocalNodes; ++i)
    {
        if (mLocalNodes[i] == node)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00674bd0
bool VoiceChat::Transport::IsConnectedSend(unsigned char node) const
{
    return GetBit(mLocal.mConnected, node, 1);
}

// FUNCTION: SWBF2 0x00674bf0
bool VoiceChat::Transport::IsConnectedReceive(unsigned char node) const
{
    return GetBit(mLocal.mConnected, node, 0);
}

// FUNCTION: SWBF2 0x00674c10
void VoiceChat::Transport::SetSending(unsigned char node, bool sending)
{
    mClientUpdated |= SetBit(mLocal.mSending, node, 0, sending, mClientChangedNodes, true);
}

// FUNCTION: SWBF2 0x00674dd0
void VoiceChat::Transport::SetSendEnable(unsigned char localNode, unsigned char node, bool enable,
                                         bool)
{
    mClientUpdated |= SetBit(mLocal.mSend, node, localNode, enable, mClientChangedNodes, false);
}

// FUNCTION: SWBF2 0x00674e20
void VoiceChat::Transport::SetReceiveEnable(unsigned char localNode, unsigned char node,
                                            bool enable, bool)
{
    mClientUpdated |= SetBit(mLocal.mReceive, node, localNode, enable, mClientChangedNodes, false);
}

// FUNCTION: SWBF2 0x006749d0
void VoiceChat::Transport::SetLocalNode(unsigned char node, bool local, bool receive, bool send)
{
    if (local)
    {
        unsigned char i;
        for (i = 0; i < mNumLocalNodes; ++i)
        {
            if (mLocalNodes[i] == node)
            {
                break;
            }
        }
        if (i == mNumLocalNodes)
        {
            mLocalNodes[mNumLocalNodes++] = node;
        }
    }
    else
    {
        for (unsigned char i = 0; i < mNumLocalNodes; ++i)
        {
            if (mLocalNodes[i] == node)
            {
                for (; i < mNumLocalNodes - 1; ++i)
                {
                    mLocalNodes[i] = mLocalNodes[i + 1];
                }
                --mNumLocalNodes;
                break;
            }
        }
    }

    const bool bReceive = SetBit(mLocal.mConnected, node, 0, local && receive,
                                 mClientChangedNodes, true);
    const bool bSend = SetBit(mLocal.mConnected, node, 1, local && send, mClientChangedNodes, true);
    mClientUpdated |= bReceive | bSend;
}

// FUNCTION: SWBF2 0x00674c40
bool VoiceChat::Transport::GetSending(unsigned char node) const
{
    return VoiceChat::GetBit(mLocal.mSending, node) && IsConnectedSend(node);
}

// FUNCTION: SWBF2 0x00674610
unsigned char VoiceChat::Transport::Method::GetNumRelayNodes() const
{
    return mNumRelayNodes;
}

// FUNCTION: SWBF2 0x00674ba0
bool VoiceChat::Transport::IsConnectedNet(unsigned char node) const
{
    return IsConnectedSend(node) || IsConnectedReceive(node);
}

// FUNCTION: SWBF2 0x00674e70
bool VoiceChat::Transport::GetSendPossible(unsigned char local, unsigned char node) const
{
    return GetSendEnable(local, node) && GetReceiveEnable(node, local) &&
           IsConnectedSend(local) && IsConnectedReceive(node);
}

// FUNCTION: SWBF2 0x00674ec0
bool VoiceChat::Transport::GetReceivePossible(unsigned char node, unsigned char stream) const
{
    return GetReceiveEnable(node, stream) && GetSendEnable(stream, node) &&
           IsConnectedReceive(node) && IsConnectedSend(stream);
}
