#include "GameVoiceChat.h"
#include "GameVoiceChatTransport.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatDecoder.h"
#include "GameVoiceChatEncoder.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "SoundFL/source/win/sndengine.h"
#include "NetGame.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"

// GLOBAL: SWBF2 0x01e56bc8

int sVoiceChatHeap;

// GLOBAL: SWBF2 0x007e68f2
namespace GameVoiceChat { bool gPlayerChange = true; }

// GLOBAL: SWBF2 0x01e563cc

void* sVoiceChatEncoders[1];
// GLOBAL: SWBF2 0x01e5665c
namespace GameVoiceChat { void* gNodes[1]; }
// GLOBAL: SWBF2 0x01e566b4

void* sVoiceChatDecoders[1];

// GLOBAL: SWBF2 0x01e563d0
namespace GameVoiceChat { char _gDecoderStorage[0x288 + 3]; }
// GLOBAL: SWBF2 0x01e56660
namespace GameVoiceChat { char _gNodeStorage[0x50 + 3]; }
// GLOBAL: SWBF2 0x01e566b8
namespace GameVoiceChat { char _gEncoderStorage[0x50C + 3]; }

// GLOBAL: SWBF2 0x01ec5fb8

// SYNTHETIC: SWBF2 0x00403ec0
// `dynamic initializer for 'sEncoderStorage''
void* sEncoderStorage = (void*)(((unsigned int)GameVoiceChat::_gEncoderStorage + 3) & ~3u);
// GLOBAL: SWBF2 0x01ec5fbc

// SYNTHETIC: SWBF2 0x00403ed0
// `dynamic initializer for 'sDecoderStorage''
void* sDecoderStorage = (void*)(((unsigned int)GameVoiceChat::_gDecoderStorage + 3) & ~3u);
// GLOBAL: SWBF2 0x01ec6018

// SYNTHETIC: SWBF2 0x00403ee0
// `dynamic initializer for 'sNodeStorage''
void* sNodeStorage = (void*)(((unsigned int)GameVoiceChat::_gNodeStorage + 3) & ~3u);

// FUNCTION: SWBF2 0x0053c500
unsigned int GameVoiceChat::GetNumNodes()
{
    return 1;
}

namespace VoiceChat { void* Alloc(unsigned int uiSize); void Free(void* pBlock); }

// FUNCTION: SWBF2 0x0053e390
void* VoiceChat::Alloc(unsigned int uiSize)
{
    return RedAllocFromHeap(sVoiceChatHeap, uiSize, 0);
}

// FUNCTION: SWBF2 0x0053e3b0
void VoiceChat::Free(void* pBlock)
{
    RedFreeToHeap(sVoiceChatHeap, pBlock, 0);
}

// FUNCTION: SWBF2 0x0053c370
void GameVoiceChat::Reset()
{
    for (unsigned int i = (unsigned int)GameVoiceChat::GetNumNodes(); i; --i)
    {
        void* pNode = GameVoiceChat::gNodes[0];
        if (!pNode)
        {
            continue;
        }
        ((GameVoiceChat::Node*)pNode)->SetSendMask(1);
        ((GameVoiceChat::Node*)pNode)->SetReceiveMask(4);
        ((GameVoiceChat::Node*)pNode)->SetID(0xFF);
    }
}

// FUNCTION: SWBF2 0x0053c2c0
void GameVoiceChat::Close()
{
    unsigned int uiNodes = (unsigned int)GameVoiceChat::GetNumNodes();
    int iOldHeap = RedSetCurrentHeap(sVoiceChatHeap);

    for (unsigned int i = 0; i < uiNodes; ++i)
    {
        if (!GameVoiceChat::gNodes[i])
        {
            continue;
        }

        if (((VoiceChat::Node*)GameVoiceChat::gNodes[i])->IsOpen())
        {
            ((GameVoiceChat::Node*)GameVoiceChat::gNodes[i])->SetID(0xFF);
            ((VoiceChat::Node*)GameVoiceChat::gNodes[i])->Close();
        }
        GameVoiceChat::gNodes[i] = 0;

        if (sVoiceChatEncoders[i])
        {
            ((GameVoiceChat::Encoder*)sVoiceChatEncoders[i])->~Encoder();
            sVoiceChatEncoders[i] = 0;
        }

        if (sVoiceChatDecoders[i])
        {
            ((GameVoiceChat::Decoder*)sVoiceChatDecoders[i])->~Decoder();
            sVoiceChatDecoders[i] = 0;
        }
    }

    VoiceChat::Node::CloseStatic();
    GameVoiceChat::gTransport.Close();
    GameVoiceChat::gPlayerChange = false;
    RedSetCurrentHeap(iOldHeap);
}

// FUNCTION: SWBF2 0x0053c3e0
bool GameVoiceChat::IsSuspended()
{
    return false;
}

// GLOBAL: SWBF2 0x01ec5fc0
// SYNTHETIC: SWBF2 0x00403ea0
// `dynamic initializer for 'GameVoiceChat::gTransport''
// SYNTHETIC: SWBF2 0x00768ae0
// `dynamic atexit destructor for 'GameVoiceChat::gTransport''
GameVoiceChat::Transport GameVoiceChat::gTransport;

// FUNCTION: SWBF2 0x0053c590
GameVoiceChat::Transport* GameVoiceChat::GetTransport()
{
    return &gTransport;
}

// FUNCTION: SWBF2 0x0053c5b0
bool GameVoiceChat::IsBanned()
{
    return false;
}

// FUNCTION: SWBF2 0x0053c5a0
void GameVoiceChat::PlayerChange()
{
    GameVoiceChat::gPlayerChange = true;
}

namespace GameVoiceChat { extern void* gNodes[1]; }
namespace GameVoiceChat { unsigned int GetNumNodes(); }

// FUNCTION: SWBF2 0x0053c510
GameVoiceChat::Node* GameVoiceChat::GetEnabledNode(unsigned int uiEnabledIndex, unsigned int& uiIndex)
{
    const unsigned int uiNumNodes = GameVoiceChat::GetNumNodes();
    uiIndex = 0;
    for (unsigned int i = 0; i < uiNumNodes; ++i)
    {
        GameVoiceChat::Node* pNode = (GameVoiceChat::Node*)GameVoiceChat::gNodes[i];
        if (pNode->mNodeID == 0xFF)
        {
            continue;
        }
        if (uiIndex == uiEnabledIndex)
        {
            return pNode;
        }
        ++uiIndex;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0053c560
GameVoiceChat::Node* GameVoiceChat::GetEnabledNode(unsigned int uiEnabledIndex)
{
    unsigned int uiIndex;
    return GetEnabledNode(uiEnabledIndex, uiIndex);
}

// FUNCTION: SWBF2 0x0053c570
unsigned int GameVoiceChat::GetNumEnabledNodes()
{
    unsigned int uiCount;
    GetEnabledNode(GameVoiceChat::GetNumNodes(), uiCount);
    return uiCount;
}

namespace GameVoiceChat
{
    struct SuspendState
    {
        unsigned char mNodeID;
        unsigned char pad[3];
        unsigned int mCaptureDeviceIndex;
        unsigned int mRenderDeviceIndex;
    };
     
    // GLOBAL: SWBF2 0x007ec214
    SuspendState gSuspend[1] = { { 0xff, { 0, 0, 0 }, 0xffffffffu, 0xffffffffu } };
}

// FUNCTION: SWBF2 0x0053c3b0
void GameVoiceChat::SetNodeID(Node* const pNode, unsigned char uNodeID)
{
    unsigned int count = (unsigned int)GameVoiceChat::GetNumNodes();
    for (unsigned int i = 0; i < count; ++i)
    {
        if (GameVoiceChat::gNodes[i] == pNode)
        {
            GameVoiceChat::gSuspend[i].mNodeID = uNodeID;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0053c2b0
bool GameVoiceChat::IsOpen()
{
    return VoiceChat::Node::IsOpenStatic();
}

// FUNCTION: SWBF2 0x0053c3f0
void GameVoiceChat::Update(float fDeltaTime)
{
    if (!IsOpen())
    {
        return;
    }

    Snd::Engine::GetThreadUpdate()->Pause(false);

    SpawnDisplay* pSpawnDisplay = SpawnDisplay::Instance(0);
    bool abNotSelectingTeam[1];
    abNotSelectingTeam[0] = !pSpawnDisplay || !pSpawnDisplay->IsSelectingTeam();

    const unsigned int uiNodes = (unsigned int)GameVoiceChat::GetNumNodes();
    for (unsigned int i = 0; i < uiNodes; ++i)
    {
        VoiceChat::Node* pNode = (VoiceChat::Node*)GameVoiceChat::gNodes[i];
        if (pNode && pNode->GetID() != 0xFF)
        {
            const int iLocal = NetGame::GetJoystickIndex(pNode->GetID());
            if (GameVoiceChat::gPlayerChange && iLocal >= 0 && abNotSelectingTeam[iLocal])
            {
                static_cast<GameVoiceChat::Node*>(pNode)->Update();
            }
        }
    }

    GameVoiceChat::gPlayerChange = false;
    for (unsigned int i = 0; i < uiNodes; ++i)
    {
        if (!abNotSelectingTeam[i])
        {
            GameVoiceChat::gPlayerChange = true;
        }
    }

    GameVoiceChat::Node::Update(fDeltaTime);
    Snd::Engine::GetThreadUpdate()->Resume();
}

namespace GameVoiceChat {

// FUNCTION: SWBF2 0x0053c4e0
void* GetNode()
{
    return GameVoiceChat::gNodes[0];
}

}

namespace GameVoiceChat {

// FUNCTION: SWBF2 0x0053c4f0
void* FindNodeByDeviceID()
{
    return GameVoiceChat::gNodes[0];
}

}

// FUNCTION: SWBF2 0x0053c030
GameVoiceChat::Transport::~Transport()
{
}
