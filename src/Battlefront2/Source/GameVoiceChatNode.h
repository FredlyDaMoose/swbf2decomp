#ifndef BATTLEFRONT2_GAMEVOICECHATNODE_H
#define BATTLEFRONT2_GAMEVOICECHATNODE_H

#include "VoiceChatNode.h"

#include <stddef.h>

#include "GameVoiceChatTransport.h"

namespace GameVoiceChat
{
class Encoder;
class Decoder;

class Node : public VoiceChat::Node
{
public:

    class Mask
    {
    public:
        Mask() : mMask(Transport::mask_Custom), mEnable(true), mMaskExisting(false) {}

        Transport::Mask mMask;
        bool mEnable;
        bool mMaskExisting;
    };

    Node(Encoder* const encoder, Decoder* const decoder);

    static void Update(float fDeltaTime);

    void Update();

    void SetID(unsigned int uChannel);
    void SetSendMask(int iMask);
    void SetReceiveMask(int iMask);

    unsigned char* GetSendMask(unsigned int* pMask, unsigned char* pFirst, unsigned char* pSecond) const;
    unsigned char* GetReceiveMask(unsigned int* pMask, unsigned char* pFirst, unsigned char* pSecond) const;

    Mask mSendMask;
    Mask mReceiveMask;
};
}

 
 
 
 
 
 

#endif
