#pragma once

#include <stddef.h>
#include "sndsourcetransport.h"

namespace Snd
{

struct StreamPacket;

class StreamNodeSource
{
public:
    virtual ~StreamNodeSource();
    virtual StreamPacket* GetPacket(unsigned int uiOutput);
};

 

struct StreamNodeConnection
{

    // FUNCTION: SWBF2 0x0073d0f0
    StreamNodeConnection() : mInputNode(0), mInputNodeOutputIndex(0) {}

    // FUNCTION: SWBF2 0x0053c650
    ~StreamNodeConnection() {}

    StreamNodeSource* mInputNode;
    unsigned int mInputNodeOutputIndex;
};

class StreamNodeSink
{
public:
    void Connect(StreamNodeSource* pSource, unsigned int uiOutput)
    {
        mConnection.mInputNode = pSource;
        mConnection.mInputNodeOutputIndex = uiOutput;
    }

    StreamPacket* ConnectionGetPacket() const
    {
        StreamNodeSource* const pNode = mConnection.mInputNode;
        if (pNode == 0)
        {
            return 0;
        }
        return pNode->GetPacket(mConnection.mInputNodeOutputIndex);
    }

    StreamNodeConnection mConnection;
};

class StreamNodeSinkNInput
{
public:

    StreamNodeSinkNInput();
    ~StreamNodeSinkNInput();

    void Connect(StreamNodeSource* const pSource, unsigned int uiOutput,
                 unsigned int uiInput);

    StreamNodeSource* ConnectionGetNode(unsigned int uiInput) const;

    unsigned int GetNumInputs() const;

    StreamPacket* ConnectionGetPacket(unsigned int uiInput) const;

    void SetConnectionArray(StreamNodeConnection* const pArray,
                            unsigned int uiCount);

    StreamNodeConnection* mConnections;
    unsigned int mNumConnections;
};

class StreamNode : public SourceTransport
{
public:
    StreamNode() : mFlags(0)
    {
        SourceTransport::mPlaybackPosition = 0;
        SourceTransport::mFlags = (unsigned short)SourceTransport::flags_stopped;
    }

    bool IsStarving() const;

    bool IsFlushing() const;

protected:

    void SetStarved(bool bStarved);

    void StartFlush();
    void EndFlush();

    bool UpdateNode(bool bAllowPausing);

public:
    unsigned char mFlags;
};

 
 
 

 
 

}
