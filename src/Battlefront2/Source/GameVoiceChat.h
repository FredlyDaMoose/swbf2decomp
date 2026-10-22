#pragma once

namespace GameVoiceChat
{
    void Close();
    void Reset();

    class Node;
    Node* GetEnabledNode(unsigned int uiEnabledIndex, unsigned int& uiIndex);
    Node* GetEnabledNode(unsigned int uiEnabledIndex);
    unsigned int GetNumEnabledNodes();
    void SetNodeID(Node* const pNode, unsigned char uNodeID);
    void Update(float fDeltaTime);
    bool IsOpen();

    class Transport;
    Transport* GetTransport();
    extern Transport gTransport;

    bool IsSuspended();
    unsigned int GetNumNodes();
    void PlayerChange();
    bool IsBanned();
    unsigned int CalculatePlayCaptureBufferSizeSamples(float fMilliseconds);
    unsigned int CalculateEncodedPacketSize(unsigned int uiSamples);
}
